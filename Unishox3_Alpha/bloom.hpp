#ifndef CPP_BARRUST_BLOOM_FILTER_H__
#define CPP_BARRUST_BLOOM_FILTER_H__
/*******************************************************************************
***
***     Original Author of C Version: Tyler Barrus
***     email:  barrust@gmail.com
***
***     Version: 1.9.0
***     Purpose: Simple, yet effective, bloom filter implementation
***
***     License: MIT 2015
***
***     URL: https://github.com/barrust/bloom
***
***     C++ Version by: Arundale Ramanathan (arun@siara.cc)
***
*******************************************************************************/

#include <inttypes.h>       /* PRIu64 */
#include <stdlib.h>
#include <math.h>           /* pow, exp */
#include <stdio.h>          /* printf */
#include <string.h>         /* strlen */
#include <fcntl.h>          /* O_RDWR */
#include <sys/mman.h>       /* mmap, mummap */
#include <sys/types.h>      /* */
#include <sys/stat.h>       /* fstat */
#include <unistd.h>         /* close */
#include <iostream>

/* https://gcc.gnu.org/onlinedocs/gcc/Alternate-Keywords.html#Alternate-Keywords */
#ifndef __GNUC__
#define __inline__ inline
#endif

#ifdef __APPLE__
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#define CPP_BLOOM_FILTER_VERSION "1.9.0"
#define CPP_BLOOM_FILTER_MAJOR 1
#define CPP_BLOOM_FILTER_MINOR 9
#define CPP_BLOOM_FILTER_REVISION 0

#define BLOOM_SUCCESS 0
#define BLOOM_FAILURE -1

typedef uint64_t* (*BloomHashFunction) (int num_hashes, const uint8_t *str, const size_t str_len);

class bloom_filter {

  private:
    /* bloom parameters */
    uint64_t estimated_elements;
    uint64_t number_bits;
    /* bloom filter */
    BloomHashFunction hash_function;
    /* on disk handeling */
    short __is_on_disk;
    FILE *filepointer;
    uint64_t __filesize;

    int CHECK_BIT_CHAR(unsigned char c, uint8_t k) {
        return ((c) & (1 << (k)));
    }
    int CHECK_BIT(unsigned char *A, uint64_t k) {
        return (CHECK_BIT_CHAR(A[((k) / 8)], ((k) % 8)));
    }
    // #define set_bit(A,k)          (A[((k) / 8)] |=  (1 << ((k) % 8)))
    // #define clear_bit(A,k)        (A[((k) / 8)] &= ~(1 << ((k) % 8)))

    /* define some constant magic looking numbers */
    const int CHAR_LEN = 8;
    const double LOG_TWO_SQUARED = 0.480453013918201388143813800;   // 0.4804530143737792968750000
                                                            // 0.4804530143737792968750000
    const double LOG_TWO = 0.693147180559945286226764000;

    void __calculate_optimal_hashes() {
        // calc optimized values
        long n = estimated_elements;
        float p = false_positive_probability;
        uint64_t m = ceil((-n * logl(p)) / LOG_TWO_SQUARED);  // AKA pow(log(2), 2);
        unsigned int k = round(LOG_TWO * m / n);             // AKA log(2.0);
        // set paramenters
        number_hashes = k; // should check to make sure it is at least 1...
        number_bits = m;
        long num_pos = ceil(m / (CHAR_LEN * 1.0));
        bloom_length = num_pos;
    }

    int __sum_bits_set_char(unsigned char c) {
        const char *bits_set_table = "\x00\x01\x01\x02\x01\x02\x02\x03\x01\x02\x02\x03\x02\x03\x03\x04";
        return bits_set_table[c >> 4] + bits_set_table[c & 0x0F];
    }

    /* NOTE: this assumes that the file handler is open and ready to use */
    void __write_to_file(FILE *fp, short on_disk) {
        if (on_disk == 0) {
            fwrite(bloom, bloom_length, 1, fp);
        } else {
            // will need to write out everything by hand
            uint64_t i;
            for (i = 0; i < bloom_length; ++i) {
                fputc(0, fp);
            }
        }
        fwrite(&estimated_elements, sizeof(uint64_t), 1, fp);
        fwrite(&elements_added, sizeof(uint64_t), 1, fp);
        fwrite(&false_positive_probability, sizeof(float), 1, fp);
    }

    /* NOTE: this assumes that the file handler is open and ready to use */
    void __read_from_file(FILE *fp, short on_disk, const char *filename) {
        int offset = sizeof(uint64_t) * 2 + sizeof(float);
        fseek(fp, offset * -1, SEEK_END);

        fread(&estimated_elements, sizeof(uint64_t), 1, fp);
        fread(&elements_added, sizeof(uint64_t), 1, fp);
        fread(&false_positive_probability, sizeof(float), 1, fp);
        __calculate_optimal_hashes();
        rewind(fp);
        if(on_disk == 0) {
            bloom = (unsigned char*)calloc(bloom_length + 1, sizeof(char));
            size_t read;
            read = fread(bloom, sizeof(char), bloom_length, fp);
            if (read != bloom_length) {
                perror("__read_from_file: ");
                exit(1);
            }
        } else {
            struct stat buf;
            int fd = open(filename, O_RDWR);
            if (fd < 0) {
                perror("open: ");
                exit(1);
            }
            fstat(fd, &buf);
            __filesize = buf.st_size;
            bloom = (unsigned char*)mmap((caddr_t)0, __filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            if (bloom == (unsigned char*) - 1) {
                perror("mmap: ");
                exit(1);
            }
            // close the file descriptor
            close(fd);
        }
    }

    void __update_elements_added_on_disk() {
        if (__is_on_disk == 1) { // only do this if it is on disk!
            int offset = sizeof(uint64_t) + sizeof(float);
            #pragma omp critical (critical_on_disk)
            {
                fseek(filepointer, offset * -1, SEEK_END);
                fwrite(&elements_added, sizeof(uint64_t), 1, filepointer);
            }
        }
    }

    static int __check_if_union_or_intersection_ok(bloom_filter *res, bloom_filter *bf1, bloom_filter *bf2) {
        if (res->number_hashes != bf1->number_hashes || bf1->number_hashes != bf2->number_hashes) {
            return BLOOM_FAILURE;
        } else if (res->number_bits != bf1->number_bits || bf1->number_bits != bf2->number_bits) {
            return BLOOM_FAILURE;
        } else if (res->hash_function != bf1->hash_function || bf1->hash_function != bf2->hash_function) {
            return BLOOM_FAILURE;
        }
        return BLOOM_SUCCESS;
    }

    /* NOTE: The caller will free the results */
    static uint64_t* __default_hash(int num_hashes, const uint8_t *str, const size_t str_len) {
        uint64_t *results = (uint64_t*)calloc(num_hashes, sizeof(uint64_t));
        int i;
        for (i = 0; i < num_hashes; ++i) {
            results[i] = __fnv_1a(str, str_len, i);
        }
        return results;
    }

    static uint64_t __fnv_1a(const uint8_t *key, const size_t len, int seed) {
        // FNV-1a hash (http://www.isthe.com/chongo/tech/comp/fnv/)
        size_t i;
        uint64_t h = 14695981039346656037ULL + (31 * seed); // FNV_OFFSET 64 bit with magic number seed
        for (i = 0; i < len; ++i){
                h = h ^ (unsigned char) key[i];
                h = h * 1099511628211ULL; // FNV_PRIME 64 bit
        }
        return h;
    }

  public:
    uint64_t elements_added;
    float false_positive_probability;
    unsigned char *bloom;
    unsigned long bloom_length;
    unsigned int number_hashes;

    bloom_filter() {
    }

    bloom_filter(uint64_t _estimated_elements, float _false_positive_rate) {
        if (init(_estimated_elements, _false_positive_rate) != BLOOM_SUCCESS)
            throw BLOOM_FAILURE;
    }

    bloom_filter(uint64_t _estimated_elements, float _false_positive_rate, BloomHashFunction _hash_function) {
        if (init_alt(_estimated_elements, _false_positive_rate, _hash_function) != BLOOM_SUCCESS)
            throw BLOOM_FAILURE;
    }

    ~bloom_filter() {
        destroy();
    }

    static const char *get_version() {
        return (CPP_BLOOM_FILTER_VERSION);
    }

    /*  Initialize a standard bloom filter in memory; this will provide 'optimal' size and hash numbers.

        Estimated elements is 0 < x <= UINT64_MAX.
        False positive rate is 0.0 < x < 1.0 */
    int init_alt(uint64_t _estimated_elements, float _false_positive_rate, BloomHashFunction _hash_function) {
        if(_estimated_elements == 0 || _estimated_elements > UINT64_MAX || _false_positive_rate <= 0.0 || _false_positive_rate >= 1.0) {
            std::cout << "Invalid parameters passed to init_alt" << std::endl;
            return BLOOM_FAILURE;
        }
        estimated_elements = _estimated_elements;
        false_positive_probability = _false_positive_rate;
        __calculate_optimal_hashes();
        bloom = (unsigned char*)calloc(bloom_length + 1, sizeof(char)); // pad to ensure no running off the end
        elements_added = 0;
        set_hash_function(_hash_function);
        __is_on_disk = 0; // not on disk
        return BLOOM_SUCCESS;
    }
    int init(uint64_t _estimated_elements, float _false_positive_rate) {
        return init_alt(_estimated_elements, _false_positive_rate, NULL);
    }

    /* Initialize a bloom filter directly into file; useful if the bloom filter is larger than available RAM */
    int init_on_disk_alt(uint64_t _estimated_elements, float _false_positive_rate, const char *filepath, BloomHashFunction _hash_function) {
        if(_estimated_elements == 0 || _estimated_elements > UINT64_MAX || _false_positive_rate <= 0.0 || _false_positive_rate >= 1.0) {
            std::cout << "Invalid parameters passed to init_on_disk_alt" << std::endl;
            return BLOOM_FAILURE;
        }
        estimated_elements = _estimated_elements;
        false_positive_probability = _false_positive_rate;
        __calculate_optimal_hashes();
        elements_added = 0;
        FILE *fp;
        fp = fopen(filepath, "w+b");
        if (fp == NULL) {
            fprintf(stderr, "Can't open file %s!\n", filepath);
            return BLOOM_FAILURE;
        }
        __write_to_file(fp, 1);
        fclose(fp);
        // slightly ineffecient to redo some of the calculations...
        return import_on_disk_alt(filepath, _hash_function);
    }
    int init_on_disk(uint64_t _estimated_elements, float _false_positive_rate, const char *filepath) {
        return init_on_disk_alt(_estimated_elements, _false_positive_rate, filepath, NULL);
    }

    /* Set or change the hashing function */
    void set_hash_function(BloomHashFunction _hash_function) {
        hash_function = (_hash_function == NULL) ? __default_hash : _hash_function;
    }

    /* reset filter to unused state */
    int clear() {
        for (unsigned long i = 0; i < bloom_length; ++i) {
            bloom[i] = 0;
        }
        elements_added = 0;
        __update_elements_added_on_disk();
        return BLOOM_SUCCESS;
    }

    /* Print out statistics about the bloom filter */
    void stats() {
        const char *is_on_disk = (__is_on_disk == 0 ? "no" : "yes");
        uint64_t size_on_disk = export_size();

        printf("BloomFilter\n\
        bits: %" PRIu64 "\n\
        estimated elements: %" PRIu64 "\n\
        number hashes: %d\n\
        max false positive rate: %f\n\
        bloom length (8 bits): %ld\n\
        elements added: %" PRIu64 "\n\
        estimated elements added: %" PRIu64 "\n\
        current false positive rate: %f\n\
        export size (bytes): %" PRIu64 "\n\
        number bits set: %" PRIu64 "\n\
        is on disk: %s\n",
        number_bits, estimated_elements, number_hashes,
        false_positive_probability, bloom_length, elements_added,
        estimate_elements(),
        current_false_positive_rate(), size_on_disk,
        count_set_bits(), is_on_disk);
    }

    /* Add a string (or element) to the bloom filter */
    int add_string(const char *str) {
        return add_uint8_str((const uint8_t *) str, strlen(str));
    }

    /* Add a uint8_t string (or element) to the bloom filter */
    int add_uint8_str(const uint8_t *str, const size_t str_len) {
        uint64_t *hashes = calculate_hashes(str, str_len, number_hashes);
        int res = add_string_alt(hashes, number_hashes);
        free(hashes);
        return res;
    }

    /* Check to see if a string (or element) is or is not in the bloom filter */
    int check_string(const char *str) {
        return check_uint8_str((const uint8_t *) str, strlen(str));
    }

    /* Check to see if a uint8_t string (or element) is or is not in the bloom filter */
    int check_uint8_str(const uint8_t *str, const size_t str_len) {
        uint64_t *hashes = calculate_hashes(str, str_len, number_hashes);
        int res = check_string_alt(hashes, number_hashes);
        free(hashes);
        return res;
    }

    /*  Generate the desired number of hashes for the provided string
        NOTE: It is up to the caller to free the allocated memory */
    uint64_t* calculate_hashes(const uint8_t *str, const size_t str_len, unsigned int _number_hashes) {
        return hash_function(_number_hashes, str, str_len);
    }

    /* Add a string to a bloom filter using the defined hashes */
    int add_string_alt(uint64_t *_hashes, unsigned int number_hashes_passed) {
        if (number_hashes_passed < number_hashes) {
            fprintf(stderr, "Error: not enough hashes passed in to correctly check!\n");
            return BLOOM_FAILURE;
        }

        for (unsigned int i = 0; i < number_hashes; ++i) {
            unsigned long idx = (_hashes[i] % number_bits) / 8;
            int bit = (_hashes[i] % number_bits) % 8;

            #pragma omp atomic update
            bloom[idx] |= (1 << bit); // set the bit
        }

        #pragma omp atomic update
        elements_added++;
        __update_elements_added_on_disk();
        return BLOOM_SUCCESS;
    }

    /* Check if a string is in the bloom filter using the passed hashes */
    int check_string_alt(uint64_t *_hashes, unsigned int number_hashes_passed) {
        if (number_hashes_passed < number_hashes) {
            fprintf(stderr, "Error: not enough hashes passed in to correctly check!\n");
            return BLOOM_FAILURE;
        }

        unsigned int i;
        int r = BLOOM_SUCCESS;
        for (i = 0; i < number_hashes; ++i) {
            int tmp_check = CHECK_BIT(bloom, (_hashes[i] % number_bits));
            if (tmp_check == 0) {
                r = BLOOM_FAILURE;
                break; // no need to continue checking
            }
        }
        return r;
    }

    /* Calculates the current false positive rate based on the number of inserted elements */
    float current_false_positive_rate() {
        int num = number_hashes * elements_added;
        double d = -num / (float) number_bits;
        double e = exp(d);
        return pow((1 - e), number_hashes);
    }

    /* Export the current bloom filter to file */
    int bf_export(const char *filepath) {
        // if the bloom is initialized on disk, no need to export it
        if (__is_on_disk == 1) {
            return BLOOM_SUCCESS;
        }
        FILE *fp;
        fp = fopen(filepath, "w+b");
        if (fp == NULL) {
            fprintf(stderr, "Can't open file %s!\n", filepath);
            return BLOOM_FAILURE;
        }
        __write_to_file(fp, 0);
        fclose(fp);
        return BLOOM_SUCCESS;
    }

    /* Import a previously exported bloom filter from a file into memory */
    int import_alt(const char *filepath, BloomHashFunction _hash_function) {
        FILE *fp;
        fp = fopen(filepath, "r+b");
        if (fp == NULL) {
            fprintf(stderr, "Can't open file %s!\n", filepath);
            return BLOOM_FAILURE;
        }
        __read_from_file(fp, 0, NULL);
        fclose(fp);
        set_hash_function(_hash_function);
        __is_on_disk = 0; // not on disk
        return BLOOM_SUCCESS;
    }
    int import(const char *filepath) {
        return import_alt(filepath, NULL);
    }

    /*  Import a previously exported bloom filter from a file but do not pull the full bloom into memory.
        This is allows for the speed / storage trade off of not needing to put the full bloom filter
        into RAM. */
    int import_on_disk_alt(const char *filepath, BloomHashFunction _hash_function) {
        filepointer = fopen(filepath, "r+b");
        if (filepointer == NULL) {
            fprintf(stderr, "Can't open file %s!\n", filepath);
            return BLOOM_FAILURE;
        }
        __read_from_file(filepointer, 1, filepath);
        // don't close the file pointer here...
        set_hash_function(_hash_function);
        __is_on_disk = 1; // on disk
        return BLOOM_SUCCESS;
    }
    int import_on_disk(const char *filepath) {
        return import_on_disk_alt(filepath, NULL);
    }

    /*  Export and import as a hex string; not space effecient but allows for storing
        multiple blooms in a single file or in a database, etc.

        NOTE: It is up to the caller to free the allocated memory */
    char* export_hex_string() {
        uint64_t i, bytes = sizeof(uint64_t) * 2 + sizeof(float) + (bloom_length);
        char* hex = (char*)calloc((bytes * 2 + 1), sizeof(char));
        for (i = 0; i < bloom_length; ++i) {
            sprintf(hex + (i * 2), "%02x", bloom[i]); // not the fastest way, but works
        }
        i = bloom_length * 2;
        sprintf(hex + i, "%016" PRIx64 "", estimated_elements);
        i += 16; // 8 bytes * 2 for hex
        sprintf(hex + i, "%016" PRIx64 "", elements_added);

        unsigned int ui;
        memcpy(&ui, &false_positive_probability, sizeof (ui));
        i += 16; // 8 bytes * 2 for hex
        sprintf(hex + i, "%08x", ui);
        return hex;
    }
    int import_hex_string_alt(const char *hex, BloomHashFunction _hash_function) {
        uint64_t len = strlen(hex);
        if (len % 2 != 0) {
            fprintf(stderr, "Unable to parse; exiting\n");
            return BLOOM_FAILURE;
        }
        char fpr[9] = {0};
        char est_els[17] = {0};
        char ins_els[17] = {0};
        memcpy(fpr, hex + (len - 8), 8);
        memcpy(ins_els, hex + (len - 24), 16);
        memcpy(est_els, hex + (len - 40), 16);
        uint32_t t_fpr;

        estimated_elements = strtoull(est_els, NULL, 16);
        elements_added = strtoull(ins_els, NULL, 16);
        sscanf(fpr, "%x", &t_fpr);
        float f;
        memcpy(&f, &t_fpr, sizeof(float));
        false_positive_probability = f;
        set_hash_function(_hash_function);

        __calculate_optimal_hashes();
        bloom = (unsigned char*)calloc(bloom_length + 1, sizeof(char));  // pad
        __is_on_disk = 0; // not on disk

        uint64_t i;
        for (i = 0; i < bloom_length; ++i) {
            sscanf(hex + (i * 2), "%2hx", (short unsigned int*)&bloom[i]);
        }
        return BLOOM_SUCCESS;
    }
    int import_hex_string(char *hex) {
        return import_hex_string_alt(hex, NULL);
    }

    /* Calculate the size the bloom filter will take on disk when exported in bytes */
    uint64_t export_size() {
        return (uint64_t)(bloom_length * sizeof(unsigned char)) + (2 * sizeof(uint64_t)) + sizeof(float);
    }

    /* Count the number of bits set to 1 */
    uint64_t count_set_bits() {
        uint64_t i, res = 0;
        for (i = 0; i < bloom_length; ++i) {
            res += __sum_bits_set_char(bloom[i]);
        }
        return res;
    }

    /*  Estimate the number of unique elements in a Bloom Filter instead of using the overall count
        https://en.wikipedia.org/wiki/Bloom_filter#Approximating_the_number_of_items_in_a_Bloom_filter
        m = bits in Bloom filter
        k = number hashes
        X = count of flipped bits in filter */
    uint64_t estimate_elements() {
        return estimate_elements_by_values(number_bits, count_set_bits(), number_hashes);
    }

    uint64_t estimate_elements_by_values(uint64_t m, uint64_t X, int k) {
        /* m = number bits; X = count of flipped bits; k = number hashes */
        double log_n = log(1 - ((double) X / (double) m));
        return (uint64_t)-(((double) m / k) * log_n);
    }

    int destroy() {
        if (__is_on_disk == 0) {
            free(bloom);
        } else {
            fclose(filepointer);
            munmap(bloom, __filesize);
        }
        bloom = NULL;
        filepointer = NULL;
        elements_added = 0;
        estimated_elements = 0;
        false_positive_probability = 0;
        number_hashes = 0;
        number_bits = 0;
        hash_function = NULL;
        __is_on_disk = 0;
        __filesize = 0;
        return BLOOM_SUCCESS;
    }

    /*******************************************************************************
        Merging, Intersection, and Jaccard Index Functions
        NOTE: Requires that the bloom filters be of the same type: hash, estimated
        elements, etc.
    *******************************************************************************/

    /* Merge Bloom Filters - inserts information into res */
    int bf_union(bloom_filter *res, bloom_filter *bf1, bloom_filter *bf2) {
        // Ensure the bloom filters can be unioned
        if (__check_if_union_or_intersection_ok(res, bf1, bf2) == BLOOM_FAILURE) {
            //std::cout << "bf_union: Union or intersection not ok" << std::endl;
            return BLOOM_FAILURE;
        }
        uint64_t i;
        for (i = 0; i < bf1->bloom_length; ++i) {
            res->bloom[i] = bf1->bloom[i] | bf2->bloom[i];
        }
        set_elements_to_estimated();
        return BLOOM_SUCCESS;
    }
    uint64_t count_union_bits_set(bloom_filter *bf1, bloom_filter *bf2) {
        // Ensure the bloom filters can be unioned
        if (__check_if_union_or_intersection_ok(bf1, bf1, bf2) == BLOOM_FAILURE) {  // use bf1 as res
            //std::cout << "count_union_bits_set: Union or intersection not ok" << std::endl;
            return BLOOM_FAILURE;
        }
        uint64_t i, res = 0;
        for (i = 0; i < bf1->bloom_length; ++i) {
            res += __sum_bits_set_char(bf1->bloom[i] | bf2->bloom[i]);
        }
        return res;
    }

    /*  Find the intersection of Bloom Filters - insert into res with the intersection
        The number of inserted elements is updated to the estimated elements calculation */
    int intersect(bloom_filter *res, bloom_filter *bf1, bloom_filter *bf2) {
        // Ensure the bloom filters can be used in an intersection
        if (__check_if_union_or_intersection_ok(res, bf1, bf2) == BLOOM_FAILURE) {
            //std::cout << "intersect: Union or intersection not ok" << std::endl;
            return BLOOM_FAILURE;
        }
        uint64_t i;
        for (i = 0; i < bf1->bloom_length; ++i) {
            res->bloom[i] = bf1->bloom[i] & bf2->bloom[i];
        }
        set_elements_to_estimated();
        return BLOOM_SUCCESS;
    }
    uint64_t count_intersection_bits_set(bloom_filter *bf1, bloom_filter *bf2) {
        // Ensure the bloom filters can be used in an intersection
        if (__check_if_union_or_intersection_ok(bf1, bf1, bf2) == BLOOM_FAILURE) {  // use bf1 as res
            std::cout << "count_intersection_bits_set: Union or intersection not ok" << std::endl;
            return BLOOM_FAILURE;
        }
        uint64_t i, res = 0;
        for (i = 0; i < bf1->bloom_length; ++i) {
            res += __sum_bits_set_char(bf1->bloom[i] & bf2->bloom[i]);
        }
        return res;
    }

    /* Wrapper to set the inserted elements count to the estimated elements calculation */
    void set_elements_to_estimated() {
        elements_added = estimate_elements();
        __update_elements_added_on_disk();
    }

    /*  Calculate the Jacccard Index of the Bloom Filters
        NOTE: The closer to 1 the index, the closer in bloom filters. If it is 1, then
        the Bloom Filters contain the same elements, 0.5 would mean about 1/2 the same
        elements are in common. 0 would mean the Bloom Filters are completely different. */
    float jaccard_index(bloom_filter *bf1, bloom_filter *bf2) {
        // Ensure the bloom filters can be used in an intersection and union
        if (__check_if_union_or_intersection_ok(bf1, bf1, bf2) == BLOOM_FAILURE) {  // use bf1 as res
            return (float)BLOOM_FAILURE;
        }
        float set_union_bits = (float)count_union_bits_set(bf1, bf2);
        if (set_union_bits == 0) {  // check for divide by 0 error
            return 1.0; // they must be both empty for this to occur and are therefore the same
        }
        return (float)count_intersection_bits_set(bf1, bf2) / set_union_bits;
    }

};

#endif /* END BLOOM FILTER HEADER */
