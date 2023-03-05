from wordlist_data import wordlist


def main():
    f0 = open("wordlist_index2.h", "wb")
    f1 = open("wordlist2.bin", "wb")

    f0.write(b"const int wordlist_index[] = {")

    total_n = 0
    str_max_len = 0
    total_bytes = 0
    lvl = 0
    
    for wl in wordlist:
        lvl += 1
        print("level", lvl, "len", len(wl))
        for w in wl:
            w = w.encode()
            f1.write(w)
            f0.write(b"%d," % (total_bytes))

            s_len = len(w)
            if s_len > str_max_len:
                str_max_len = s_len

            total_n += 1
            total_bytes += s_len

    f0.write(b"%d};" % (total_bytes))
    f0.close()
    f1.close()

    print("total_lvl={} total_n={} total_bytes={} str_max_len={}".format(
        len(wordlist), total_n, total_bytes, str_max_len))


if __name__ == "__main__":
    main()
