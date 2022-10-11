/*
 * Copyright (C) 2020 Siara Logics (cc)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @author Arundale Ramanathan
 *
 */
/**
 * @file test_unishox3.c
 * @author Arundale Ramanathan, James Z. M. Gao
 * @brief Demo / Test program for Unishox2 Compression and Decompression
 *
 * This file run comprehensive tests on the Unishox2 API \n
 * It also provides command line options for demonstration \n
 * of its features.
 */
#include "src/unishox3.h"

#ifdef _MSC_VER
#include <windows.h>
#else
#include <sys/time.h>
#endif

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>

unishox3 usx3;

int test_ushx_cd_with_len(const char *input, int len) {

  char cbuf[200];
  char dbuf[251];
  int clen = usx3.compress(input, len, USX3_API_OUT_AND_LEN(cbuf, sizeof cbuf));
  if (clen > (int)sizeof cbuf) {
    printf("Compress Overflow\n");
    return 0;
  }
  printf("\n\n");
  int dlen = usx3.decompress(cbuf, clen, USX3_API_OUT_AND_LEN(dbuf, sizeof dbuf));
  if (dlen > (int)sizeof dbuf) {
    printf("Decompress Overflow\n");
    return 0;
  } else if (dlen < (int)sizeof dbuf)
    dbuf[dlen] = '\0';
  if (dlen != len) {
    dbuf[sizeof dbuf - 1] = '\0';
    printf("Fail len: %d, %d:\n%s\n%s\n", len, dlen, input, dbuf);
    return 0;
  }
  if (memcmp(input, dbuf, len)) {
    printf("Fail cmp:\n%s\n%s\n", input, dbuf);
    return 0;
  }
  float perc = (float)(len - clen);
  perc /= len;
  perc *= 100;
  printf("%s: %d/%d=", input, clen, len);
  printf("%.2f%%\n", perc);

#if (USX3_API_OUT_AND_LEN(0,1)) == 1
  // check compress overflow
  for (int i = 1; i <= 16 && clen - i >= 0; ++i) {
    char cbuf_cut[sizeof cbuf];
    const int clen_cut = usx3.compress(input, len, cbuf_cut, clen - i, USX3_TEMPLATES);
    if (clen_cut != clen - i + 1) { // should overflow
      printf("Fail compress len overflow: %d, %d\n", clen - i, clen_cut);
      return 0;
    }
    if (memcmp(cbuf, cbuf_cut, clen - i)) {
      printf("Fail compress overflow cmp\n");
      return 0;
    }
  }

  // check decompress overflow
  for (int i = 1; i <= 16 && len - i >= 0; ++i) {
    memset(dbuf, 0, sizeof dbuf);
    dlen = unishox3_decompress(cbuf, clen, dbuf, len - i, USX3_TEMPLATES);
    if (dlen != len - i + 1) { // should overflow
      dbuf[sizeof dbuf - 1] = '\0';
      printf("Fail decompress len overflow: %d, %d:\n%s\n%s\n", len, dlen, input, dbuf);
      return 0;
    }
    dbuf[len - i] = '\0';
    if (strncmp(input, dbuf, len - i)) {
      printf("Fail decompress overflow cmp:\n%s\n%s\n", input, dbuf);
      return 0;
    }
  }

  // test terminator, only valid when olen parameter is used in *_compress api
  char cbuf_term[sizeof cbuf + 6];
  const int clen_term_raw = usx3.compress(input, len, USX3_API_OUT_AND_LEN(cbuf_term, -(int)(sizeof cbuf_term)), USX3_TEMPLATES);
  int clen_term = clen_term_raw / 4;
  const int clen_term_size = clen_term_raw % 4;
  if (clen_term > (int)sizeof cbuf_term) {
      printf("Fail, overflow for full term codes\n");
      return 0;
  }
  if (clen != clen_term - clen_term_size) {
    printf("Fail compress len with term codes: %d, %d\n", clen, clen_term - clen_term_size);
    return 0;
  }
  if (memcmp(cbuf, cbuf_term, clen)) {
    printf("Fail compress with term codes cmp\n");
    return 0;
  }
  if ((unsigned char)cbuf_term[clen_term-1] != 0xFF) {
    printf("term size = %d, last byte is not 0 or 0xFF: %X\n", clen_term_size, (unsigned char)cbuf[clen-1]);
    return 0;
  }
  cbuf_term[clen_term++] = cbuf_term[0];
  cbuf_term[clen_term++] = cbuf_term[1];
  cbuf_term[clen_term++] = cbuf_term[2];

  for (int i = 1; i <= 7 && clen + i <= (int)sizeof cbuf_term; ++i) {
    memset(dbuf, 0, sizeof(dbuf));
    dlen = unishox3_decompress(cbuf_term, clen + i, USX3_API_OUT_AND_LEN(dbuf, sizeof dbuf), USX3_TEMPLATES);
    if (dlen > (int)sizeof dbuf) {
      printf("Decompress Overflow for testing terminator\n");
      return 0;
    } else if (dlen < (int)sizeof dbuf)
      dbuf[dlen] = '\0';
    if (dlen != len) {
      dbuf[sizeof dbuf - 1] = '\0';
      printf("Fail len (term+%d): %d, %d:\n%s\n%s\n", i, len, dlen, input, dbuf);
      return 0;
    }
    if (strncmp(input, dbuf, len)) {
      printf("Fail cmp (term+%d):\n%s\n%s\n", i, input, dbuf);
      return 0;
    }
  }
#endif

  return 1;

}

/// Helper function for unit tests
int test_ushx_cd(const char *input) {
  int len = (int)strlen(input);
  return test_ushx_cd_with_len(input, len);
}

int is_empty(const char *s) {
  while (*s != '\0') {
    if (!isspace((unsigned char)*s))
      return 0;
    s++;
  }
  return 1;
}

// From https://stackoverflow.com/questions/19758270/read-varint-from-linux-sockets#19760246
// Encode an unsigned 64-bit varint.  Returns number of encoded bytes.
// 'buffer' must have room for up to 10 bytes.
/// Internal helper function
int encode_unsigned_varint(uint8_t *buffer, uint64_t value) {
  int encoded = 0;
  do {
    uint8_t next_byte = value & 0x7F;
    value >>= 7;
    if (value)
      next_byte |= 0x80;
    buffer[encoded++] = next_byte;
  } while (value);
  return encoded;
}

/// Internal helper function
uint64_t decode_unsigned_varint(const uint8_t *data, int *decoded_bytes) {
  int i = 0;
  uint64_t decoded_value = 0;
  int shift_amount = 0;
  do {
    decoded_value |= (uint64_t)(data[i] & 0x7F) << shift_amount;     
    shift_amount += 7;
  } while ((data[i++] & 0x80) != 0);
  *decoded_bytes = i;
  return decoded_value;
}

/// Internal helper function
void print_string_as_hex(char *in, int len) {

  int l;
  printf("String in hex:\n");
  for (l=0; l<len; l++) {
    printf("%02x, ", (unsigned char) in[l]);
  }
  printf("\n");
}

/// Internal helper function
void print_bytes(char *in, int len, const char *title) {

  int l;
  uint8_t bit;
  printf("%s %d bytes\n", title, len);
  printf("Bytes in decimal:\n");
  for (l=0; l<len; l++)
    printf("%u, ", (unsigned char) in[l]);
  printf("\nBytes in hex:\n");
  for (l=0; l<len; l++)
    printf("\\x%02x", (unsigned char) in[l]);
  printf("\nBytes in binary:\n");
  for (l=0; l<len*8; l++) {
    bit = (in[l/8]>>(7-l%8))&0x01;
    printf("%d", bit);
    if (l%8 == 7) printf(" ");
  }
  printf("\n");

}


/// Internal helper function
uint32_t getTimeVal() {
#ifdef _MSC_VER
    return GetTickCount() * 1000;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000000) + tv.tv_usec;
#endif
}

/// Internal helper function
double timedifference(uint32_t t0, uint32_t t1) {
    double ret = t1;
    ret -= t0;
    ret /= 1000;
    return ret;
}

/// This is the unit-test function
int run_unit_tests(int argc, char *argv[]) {

    // Basic
    if (!test_ushx_cd("Hello")) return 1;
    if (!test_ushx_cd("Hello World")) return 1;
    if (!test_ushx_cd("The quick brown fox jumped over the lazy dog")) return 1;
    if (!test_ushx_cd("HELLO WORLD")) return 1;
    if (!test_ushx_cd("HELLO WORLD HELLO WORLD")) return 1;

    // Numbers
    if (!test_ushx_cd("Hello1")) return 1;
    if (!test_ushx_cd("Hello1 World2")) return 1;
    if (!test_ushx_cd("Hello123")) return 1;
    if (!test_ushx_cd("12345678")) return 1;
    if (!test_ushx_cd("12345678 12345678")) return 1;
    if (!test_ushx_cd("HELLO WORLD 1234 hello world12")) return 1;
    if (!test_ushx_cd("HELLO 234 WORLD")) return 1;
    if (!test_ushx_cd("9 HELLO, WORLD")) return 1;
    if (!test_ushx_cd("H1e2l3l4o5 w6O7R8L9D")) return 1;
    if (!test_ushx_cd("8+80=88")) return 1;

    // Symbols
    if (!test_ushx_cd("~!@#$%^&*()_+=-`;'\\|\":,./?><")) return 1;
    if (!test_ushx_cd("if (!test_ushx_cd(\"H1e2l3l4o5 w6O7R8L9D\")) return 1;")) return 1;
    if (!test_ushx_cd("Hello\tWorld\tHow\tare\tyou?")) return 1;
    if (!test_ushx_cd("Hello~World~How~are~you?")) return 1;
    if (!test_ushx_cd("Hello\rWorld\rHow\rare\ryou?")) return 1;

    // Repeat
    if (!test_ushx_cd("-----------------///////////////")) return 1;
    if (!test_ushx_cd("-----------------Hello World1111111111112222222abcdef12345abcde1234_////////Hello World///////")) return 1;
    if (!test_ushx_cd("-----------------///////////////")) return 1;
    if (!test_ushx_cd("------------------------------------")) return 1;

    // Nibbles
    if (!test_ushx_cd("fa01b51e-7ecc-4e3e-be7b-918a4c2c891c")) return 1;
    if (!test_ushx_cd("Fa01b51e-7ecc-4e3e-be7b-918a4c2c891c")) return 1;
    if (!test_ushx_cd("fa01b51e-7ecc-4e3e-be7b-9182c891c")) return 1;
    if (!test_ushx_cd("760FBCA3-272E-4F1A-BF88-8472DF6BD994")) return 1;
    if (!test_ushx_cd("760FBCA3-272E-4F1A-BF88-8472DF6Bd994")) return 1;
    if (!test_ushx_cd("760FBCA3-272E-4F1A-BF88-8472DF6Bg994")) return 1;
    if (!test_ushx_cd("FBCA3-272E-4F1A-BF88-8472DF6BD994")) return 1;
    if (!test_ushx_cd("Hello 1 5347a688-d8bf-445d-86d1-b470f95b007fHello World")) return 1;
    if (!test_ushx_cd("01234567890123")) return 1;

    // Templates
    if (!test_ushx_cd("2020-12-31")) return 1;
    if (!test_ushx_cd("1934-02")) return 1;
    if (!test_ushx_cd("2020-12-31T12:23:59.234Z")) return 1;
    if (!test_ushx_cd("1899-05-12T23:59:59.23434")) return 1;
    if (!test_ushx_cd("1899-05-12T23:59:59")) return 1;
    if (!test_ushx_cd("2020-12-31T12:23:59.234Zfa01b51e-7ecc-4e3e-be7b-918a4c2c891c")) return 1;
    if (!test_ushx_cd("顔に(993) 345-3495あり")) return 1;
    if (!test_ushx_cd("HELLO(993) 345-3495WORLD")) return 1;
    if (!test_ushx_cd("顔に1899-05-12T23:59:59あり")) return 1;
    if (!test_ushx_cd("HELLO1899-05-12T23:59:59WORLD")) return 1;

    if (!test_ushx_cd("Cada buhonero alaba sus agujas. - A peddler praises his needles (wares).")) return 1;
    if (!test_ushx_cd("Cada gallo canta en su muladar. - Each rooster sings on its dung-heap.")) return 1;
    if (!test_ushx_cd("Cada martes tiene su domingo. - Each Tuesday has its Sunday.")) return 1;
    if (!test_ushx_cd("Cada uno habla de la feria como le va en ella. - Our way of talking about things reflects our relevant experience, good or bad.")) return 1;
    if (!test_ushx_cd("Dime con quien andas y te diré quién eres.. - Tell me who you walk with, and I will tell you who you are.")) return 1;
    if (!test_ushx_cd("Donde comen dos, comen tres. - You can add one person more in any situation you are managing.")) return 1;
    if (!test_ushx_cd("El amor es ciego. - Love is blind")) return 1;
    if (!test_ushx_cd("El amor todo lo iguala. - Love smoothes life out.")) return 1;
    if (!test_ushx_cd("El tiempo todo lo cura. - Time cures all.")) return 1;
    if (!test_ushx_cd("La avaricia rompe el saco. - Greed bursts the sack.")) return 1;
    if (!test_ushx_cd("La cara es el espejo del alma. - The face is the mirror of the soul.")) return 1;
    if (!test_ushx_cd("La diligencia es la madre de la buena ventura. - Diligence is the mother of good fortune.")) return 1;
    if (!test_ushx_cd("La fe mueve montañas. - Faith moves mountains.")) return 1;
    if (!test_ushx_cd("La mejor palabra siempre es la que queda por decir. - The best word is the one left unsaid.")) return 1;
    if (!test_ushx_cd("La peor gallina es la que más cacarea. - The worst hen is the one that clucks the most.")) return 1;
    if (!test_ushx_cd("La sangre sin fuego hierve. - Blood boils without fire.")) return 1;
    if (!test_ushx_cd("La vida no es un camino de rosas. - Life is not a path of roses.")) return 1;
    if (!test_ushx_cd("Las burlas se vuelven veras. - Bad jokes become reality.")) return 1;
    if (!test_ushx_cd("Las desgracias nunca vienen solas. - Misfortunes never come one at a time.")) return 1;
    if (!test_ushx_cd("Lo comido es lo seguro. - You can only be really certain of what is already in your belly.")) return 1;
    if (!test_ushx_cd("Los años no pasan en balde. - Years don't pass in vain.")) return 1;
    if (!test_ushx_cd("Los celos son malos consejeros. - Jealousy is a bad counsellor.")) return 1;
    if (!test_ushx_cd("Los tiempos cambian. - Times change.")) return 1;
    if (!test_ushx_cd("Mañana será otro día. - Tomorrow will be another day.")) return 1;
    if (!test_ushx_cd("Ningún jorobado ve su joroba. - No hunchback sees his own hump.")) return 1;
    if (!test_ushx_cd("No cantan dos gallos en un gallinero. - Two roosters do not crow in a henhouse.")) return 1;
    if (!test_ushx_cd("No hay harina sin salvado. - No flour without bran.")) return 1;
    if (!test_ushx_cd("No por mucho madrugar, amanece más temprano.. - No matter if you rise early because it does not sunrise earlier.")) return 1;
    if (!test_ushx_cd("No se puede hacer tortilla sin romper los huevos. - One can't make an omelette without breaking eggs.")) return 1;
    if (!test_ushx_cd("No todas las verdades son para dichas. - Not every truth should be said.")) return 1;
    if (!test_ushx_cd("No todo el monte es orégano. - The whole hillside is not covered in spice.")) return 1;
    if (!test_ushx_cd("Nunca llueve a gusto de todos. - It never rains to everyone's taste.")) return 1;
    if (!test_ushx_cd("Perro ladrador, poco mordedor.. - A dog that barks often seldom bites.")) return 1;
    if (!test_ushx_cd("Todos los caminos llevan a Roma. - All roads lead to Rome.")) return 1;

    // Unicode
    if (!test_ushx_cd("案ずるより産むが易し。 - Giving birth to a baby is easier than worrying about it.")) return 1;
    if (!test_ushx_cd("出る杭は打たれる。 - The stake that sticks up gets hammered down.")) return 1;
    if (!test_ushx_cd("知らぬが仏。 - Not knowing is Buddha. - Ignorance is bliss.")) return 1;
    if (!test_ushx_cd("見ぬが花。 - Not seeing is a flower. - Reality can't compete with imagination.")) return 1;
    if (!test_ushx_cd("花は桜木人は武士 - Of flowers, the cherry blossom; of men, the warrior.")) return 1;

    if (!test_ushx_cd("小洞不补，大洞吃苦 - A small hole not mended in time will become a big hole much more difficult to mend.")) return 1;
    if (!test_ushx_cd("读万卷书不如行万里路 - Reading thousands of books is not as good as traveling thousands of miles")) return 1;
    if (!test_ushx_cd("福无重至,祸不单行 - Fortune does not come twice. Misfortune does not come alone.")) return 1;
    if (!test_ushx_cd("风向转变时,有人筑墙,有人造风车 - When the wind changes, some people build walls and have artificial windmills.")) return 1;
    if (!test_ushx_cd("父债子还 - Father's debt, son to give back.")) return 1;
    if (!test_ushx_cd("害人之心不可有 - Do not harbour intentions to hurt others.")) return 1;
    if (!test_ushx_cd("今日事，今日毕 - Things of today, accomplished today.")) return 1;
    if (!test_ushx_cd("空穴来风,未必无因 - Where there's smoke, there's fire.")) return 1;
    if (!test_ushx_cd("良药苦口 - Good medicine tastes bitter.")) return 1;
    if (!test_ushx_cd("人算不如天算 - Man proposes and God disposes")) return 1;
    if (!test_ushx_cd("师傅领进门，修行在个人 - Teachers open the door. You enter by yourself.")) return 1;
    if (!test_ushx_cd("授人以鱼不如授之以渔 - Teach a man to take a fish is not equal to teach a man how to fish.")) return 1;
    if (!test_ushx_cd("树倒猢狲散 - When the tree falls, the monkeys scatter.")) return 1;
    if (!test_ushx_cd("水能载舟，亦能覆舟 - Not only can water float a boat, it can sink it also.")) return 1;
    if (!test_ushx_cd("朝被蛇咬，十年怕井绳 - Once bitten by a snake for a snap dreads a rope for a decade.")) return 1;
    if (!test_ushx_cd("一分耕耘，一分收获 - If one does not plow, there will be no harvest.")) return 1;
    if (!test_ushx_cd("有钱能使鬼推磨 - If you have money you can make the devil push your grind stone.")) return 1;
    if (!test_ushx_cd("一失足成千古恨，再回头已百年身 - A single slip may cause lasting sorrow.")) return 1;
    if (!test_ushx_cd("自助者天助 - Those who help themselves, God will help.")) return 1;
    if (!test_ushx_cd("早起的鸟儿有虫吃 - Early bird gets the worm.")) return 1;
    if (!test_ushx_cd("This is first line,\r\nThis is second line")) return 1;
    if (!test_ushx_cd("{\"menu\": {\n  \"id\": \"file\",\n  \"value\": \"File\",\n  \"popup\": {\n    \"menuitem\": [\n      {\"value\": \"New\", \"onclick\": \"CreateNewDoc()\"},\n      {\"value\": \"Open\", \"onclick\": \"OpenDoc()\"},\n      {\"value\": \"Close\", \"onclick\": \"CloseDoc()\"}\n    ]\n  }\n}}")) return 1;
    if (!test_ushx_cd("{\"menu\": {\r\n  \"id\": \"file\",\r\n  \"value\": \"File\",\r\n  \"popup\": {\r\n    \"menuitem\": [\r\n      {\"value\": \"New\", \"onclick\": \"CreateNewDoc()\"},\r\n      {\"value\": \"Open\", \"onclick\": \"OpenDoc()\"},\r\n      {\"value\":\"Close\", \"onclick\": \"CloseDoc()\"}\r\n    ]\r\n  }\r\n}}")) return 1;
    if (!test_ushx_cd("https://siara.cc")) return 1;

    if (!test_ushx_cd("符号\"δ\"表")) return 1;
    if (!test_ushx_cd("学者地”[3]。学者")) return 1;
    if (!test_ushx_cd("한데......아무")) return 1;

    // English
    if (!test_ushx_cd("Beauty is not in the face. Beauty is a light in the heart.")) return 1;
    // Spanish
    if (!test_ushx_cd("La belleza no está en la cara. La belleza es una luz en el corazón.")) return 1;
    // French
    if (!test_ushx_cd("La beauté est pas dans le visage. La beauté est la lumière dans le coeur.")) return 1;
    // Portugese
    if (!test_ushx_cd("A beleza não está na cara. A beleza é a luz no coração.")) return 1;
    // Dutch
    if (!test_ushx_cd("Schoonheid is niet in het gezicht. Schoonheid is een licht in het hart.")) return 1;

    // German
    if (!test_ushx_cd("Schönheit ist nicht im Gesicht. Schönheit ist ein Licht im Herzen.")) return 1;
    // Spanish
    if (!test_ushx_cd("La belleza no está en la cara. La belleza es una luz en el corazón.")) return 1;
    // French
    if (!test_ushx_cd("La beauté est pas dans le visage. La beauté est la lumière dans le coeur.")) return 1;
    // Italian
    if (!test_ushx_cd("La bellezza non è in faccia. La bellezza è la luce nel cuore.")) return 1;
    // Swedish
    if (!test_ushx_cd("Skönhet är inte i ansiktet. Skönhet är ett ljus i hjärtat.")) return 1;
    // Romanian
    if (!test_ushx_cd("Frumusețea nu este în față. Frumusețea este o lumină în inimă.")) return 1;
    // Ukranian
    if (!test_ushx_cd("Краса не в особі. Краса - це світло в серці.")) return 1;
    // Greek
    if (!test_ushx_cd("Η ομορφιά δεν είναι στο πρόσωπο. Η ομορφιά είναι ένα φως στην καρδιά.")) return 1;
    // Turkish
    if (!test_ushx_cd("Güzellik yüzünde değil. Güzellik, kalbin içindeki bir ışıktır.")) return 1;
    // Polish
    if (!test_ushx_cd("Piękno nie jest na twarzy. Piękno jest światłem w sercu.")) return 1;

    // Africans
    if (!test_ushx_cd("Skoonheid is nie in die gesig nie. Skoonheid is 'n lig in die hart.")) return 1;
    // Swahili
    if (!test_ushx_cd("Beauty si katika uso. Uzuri ni nuru moyoni.")) return 1;
    // Zulu
    if (!test_ushx_cd("Ubuhle abukho ebusweni. Ubuhle bungukukhanya enhliziyweni.")) return 1;
    // Somali
    if (!test_ushx_cd("Beauty ma aha in wajiga. Beauty waa iftiin ah ee wadnaha.")) return 1;

    // Russian
    if (!test_ushx_cd("Красота не в лицо. Красота - это свет в сердце.")) return 1;
    // Arabic
    if (!test_ushx_cd("الجمال ليس في الوجه. الجمال هو النور الذي في القلب.")) return 1;
    // Persian
    if (!test_ushx_cd("زیبایی در چهره نیست. زیبایی نور در قلب است.")) return 1;
    // Pashto
    if (!test_ushx_cd("ښکلا په مخ کې نه ده. ښکلا په زړه کی یوه رڼا ده.")) return 1;
    // Azerbaijani
    if (!test_ushx_cd("Gözəllik üzdə deyil. Gözəllik qəlbdə bir işıqdır.")) return 1;
    // Uzbek
    if (!test_ushx_cd("Go'zallik yuzida emas. Go'zallik - qalbdagi nur.")) return 1;
    // Kurdish
    if (!test_ushx_cd("Bedewî ne di rû de ye. Bedewî di dil de ronahiyek e.")) return 1;
    // Urdu
    if (!test_ushx_cd("خوبصورتی چہرے میں نہیں ہے۔ خوبصورتی دل میں روشنی ہے۔")) return 1;

    // Hindi
    if (!test_ushx_cd("सुंदरता चेहरे में नहीं है। सौंदर्य हृदय में प्रकाश है।")) return 1;
    // Bangla
    if (!test_ushx_cd("সৌন্দর্য মুখে নেই। সৌন্দর্য হৃদয় একটি আলো।")) return 1;
    // Punjabi
    if (!test_ushx_cd("ਸੁੰਦਰਤਾ ਚਿਹਰੇ ਵਿੱਚ ਨਹੀਂ ਹੈ. ਸੁੰਦਰਤਾ ਦੇ ਦਿਲ ਵਿਚ ਚਾਨਣ ਹੈ.")) return 1;
    // Telugu
    if (!test_ushx_cd("అందం ముఖంలో లేదు. అందం హృదయంలో ఒక కాంతి.")) return 1;
    // Tamil
    if (!test_ushx_cd("அழகு முகத்தில் இல்லை. அழகு என்பது இதயத்தின் ஒளி.")) return 1;
    // Marathi
    if (!test_ushx_cd("सौंदर्य चेहरा नाही. सौंदर्य हे हृदयातील एक प्रकाश आहे.")) return 1;
    // Kannada
    if (!test_ushx_cd("ಸೌಂದರ್ಯವು ಮುಖದ ಮೇಲೆ ಇಲ್ಲ. ಸೌಂದರ್ಯವು ಹೃದಯದಲ್ಲಿ ಒಂದು ಬೆಳಕು.")) return 1;
    // Gujarati
    if (!test_ushx_cd("સુંદરતા ચહેરા પર નથી. સુંદરતા હૃદયમાં પ્રકાશ છે.")) return 1;
    // Malayalam
    if (!test_ushx_cd("സൗന്ദര്യം മുഖത്ത് ഇല്ല. സൗന്ദര്യം ഹൃദയത്തിലെ ഒരു പ്രകാശമാണ്.")) return 1;
    // Nepali
    if (!test_ushx_cd("सौन्दर्य अनुहारमा छैन। सौन्दर्य मुटुको उज्यालो हो।")) return 1;
    // Sinhala
    if (!test_ushx_cd("රූපලාවන්ය මුහුණේ නොවේ. රූපලාවන්ය හදවත තුළ ඇති ආලෝකය වේ.")) return 1;

    // Chinese
    if (!test_ushx_cd("美是不是在脸上。 美是心中的亮光。")) return 1;
    // Javanese
    if (!test_ushx_cd("Beauty ora ing pasuryan. Kaendahan iku cahya ing sajroning ati.")) return 1;
    // Japanese
    if (!test_ushx_cd("美は顔にありません。美は心の中の光です。")) return 1;
    // Filipino
    if (!test_ushx_cd("Ang kagandahan ay wala sa mukha. Ang kagandahan ay ang ilaw sa puso.")) return 1;
    // Korean
    if (!test_ushx_cd("아름다움은 얼굴에 없습니다。아름다움은 마음의 빛입니다。")) return 1;
    // Vietnam
    if (!test_ushx_cd("Vẻ đẹp không nằm trong khuôn mặt. Vẻ đẹp là ánh sáng trong tim.")) return 1;
    // Thai
    if (!test_ushx_cd("ความงามไม่ได้อยู่ที่ใบหน้า ความงามเป็นแสงสว่างในใจ")) return 1;
    // Burmese
    if (!test_ushx_cd("အလှအပမျက်နှာပေါ်မှာမဟုတ်ပါဘူး။ အလှအပစိတ်နှလုံးထဲမှာအလင်းကိုဖြစ်ပါတယ်။")) return 1;
    // Malay
    if (!test_ushx_cd("Kecantikan bukan di muka. Kecantikan adalah cahaya di dalam hati.")) return 1;

    // Emoji
    if (!test_ushx_cd("🤣🤣🤣🤣🤣🤣🤣🤣🤣🤣🤣")) return 1;
    if (!test_ushx_cd("😀😃😄😁😆😅🤣😂🙂🙃😉😊😇🥰😍🤩😘😗😚😙😋😛😜🤪😝🤑🤗🤭🤫🤔🤐🤨😐😑😶😏😒🙄😬🤥😌😔😪🤤😴😷🤒🤕🤢")) return 1;

    // Binary
    if (!test_ushx_cd("Hello\x80\x83\xAE\xBC\xBD\xBE")) return 1;
    if (!test_ushx_cd_with_len("Hello world\x0 with nulls\x0", 24)) return 1;

    // check template
    {
      char cbuf[128];
      char dbuf[128];
      const char *hex = ":AAAAAA-bbbbbb";
      const int len = strlen(hex);
      usx3.setTemplates((const char *[]){":FFFFFF", "-ffffff", 0, 0, 0});
      const int clen = usx3.compress(hex, len, USX3_API_OUT_AND_LEN(cbuf, sizeof cbuf));
      const int dlen = usx3.decompress(cbuf, clen, USX3_API_OUT_AND_LEN(dbuf, sizeof dbuf));
      if (dlen != len) {
        printf("Fail len (template): %d, %d:\n%s\n%s\n", len, dlen, hex, dbuf);
        return 1;
      }
      if (strncmp(hex, dbuf, len)) {
        printf("Fail cmp (template):\n%s\n%s\n", hex, dbuf);
        return 0;
      }
    }

    return 0;
}
extern int short_count;
extern int long_count;
/**
 * <pre>
 * Usage: test_unishox3 \"string\"
 *               (or)
 *        test_unishox3 [action] [in_file] [out_file]
 *
 *          action:
 *          -t    run tests
 *          -c    compress
 *          -d    decompress
 *
 * </pre>
 */
int main(int argc, char *argv[]) {

char cbuf[65536];
char dbuf[1038576];
long len, tot_len, clen, ctot=0;
size_t dlen;
float perc=0.F;
FILE *fp, *wfp;
int bytes_read;
uint32_t tStart;

tStart = getTimeVal();

if (argc >= 4 && strcmp(argv[1], "-c") == 0) {
   tot_len = 0;
   fp = fopen(argv[2], "rb");
   if (fp == NULL) {
      perror(argv[2]);
      return 1;
   }
   wfp = fopen(argv[3], "wb");
   if (wfp == NULL) {
      perror(argv[3]);
      return 1;
   }
   do {
     bytes_read = (int)fread(cbuf, 1, sizeof(cbuf), fp);
     if (bytes_read > 0) {
        clen = usx3.compress(cbuf, bytes_read, USX3_API_OUT_AND_LEN(dbuf, sizeof dbuf));
        ctot += clen;
        tot_len += bytes_read;
        if (clen > 0) {
           fputc(clen >> 8, wfp);
           fputc(clen & 0xFF, wfp);
           if (clen != (long)fwrite(dbuf, 1, clen, wfp)) {
              perror("fwrite");
              return 1;
           }
        }
     }
   } while (bytes_read > 0);
   perc = (float)(tot_len-ctot);
   perc /= tot_len;
   perc *= 100;
   printf("\nBytes (Compressed/Original=Savings%%): %ld/%ld=", ctot, tot_len);
   printf("%.2f%%\n", perc);
   printf("Short count: %d\n", short_count);
   printf("Long count: %d\n", long_count);
} else
if (argc >= 4 && strcmp(argv[1], "-d") == 0) {
   fp = fopen(argv[2], "rb");
   if (fp == NULL) {
      perror(argv[2]);
      return 1;
   }
   wfp = fopen(argv[3], "wb");
   if (wfp == NULL) {
      perror(argv[3]);
      return 1;
   }
   do {
     //memset(dbuf, 0, sizeof(dbuf));
     int len_to_read = fgetc(fp) << 8;
     len_to_read += fgetc(fp);
     bytes_read = (int)fread(dbuf, 1, len_to_read, fp);
     if (bytes_read > 0) {
        dlen = usx3.decompress(dbuf, bytes_read, USX3_API_OUT_AND_LEN(cbuf, sizeof cbuf));
        if (dlen > 0) {
           if (dlen != fwrite(cbuf, 1, dlen, wfp)) {
              perror("fwrite");
              return 1;
           }
        }
     }
   } while (bytes_read > 0);
} else
if (argc >= 2 && strcmp(argv[1], "-t") == 0) {
  usx3.setTemplates(USX_TEMPLATES);
  return run_unit_tests(argc, argv);
} else
if (argc == 4 && strcmp(argv[1], "-di") == 0) {
  char *input = argv[2];
  int clen = atoi(argv[3]);
  //char *input = "\252!\355\347;멠<\322\336\346\070\205X\200v\367b\002\332l\213\022\n\003P\374\267\002\266e\207.\210r:\021\225\224\243\353\204\305\352\255\017L/(HH4i\223~\270-\223\206\221\246\212\261\221e\254\375\341\350\037\240X\211ǉ\325\330u\365\303ʂ\200гM\236&\375\377\071%'?V\025\070\374\026\346s\323$\276\350F\224\r-\226\347ɋ\317\344\214\v\032U\303\353\215\335GX\202\371B\302\355\a\247\273\356C\372\a-\262\006\\\343\"ZH|\357\034\001";
  //clen = strlen(input);
  //print_bytes(argv[2], clen, "Input:");
  print_bytes(input, clen, "Input:");
  dlen = usx3.decompress_simple(input, clen, dbuf);
  dbuf[dlen] = '\0';
  printf("Decompressed: [%s], len: %zu", dbuf, dlen);
} else
if (argc == 2 || (argc == 3 && atoi(argv[2]) > 0)) {
   len = (long)strlen(argv[1]);
   printf("String: %s, Len:%ld\n", argv[1], len);
   //print_string_as_hex(argv[1], len);
   memset(cbuf, 0, sizeof(cbuf));
   ctot = usx3.compress(argv[1], len, USX3_API_OUT_AND_LEN(cbuf, sizeof cbuf));
   print_bytes(cbuf, ctot, "Compressed:");
   memset(dbuf, 0, sizeof(dbuf));
   dlen = usx3.decompress(cbuf, ctot, USX3_API_OUT_AND_LEN(dbuf, sizeof dbuf - 1));
   dbuf[dlen] = 0;
   printf("\nDecompressed: %s\n", dbuf);
   if (strncmp(dbuf, argv[1], len))
     printf("\nERROR: DECOMPRESSED STRING DOES NOT MATCH ORIGINAL");
   //print_compressed(dbuf, dlen);
   perc = (float)(len-ctot);
   perc /= len;
   perc *= 100;
   printf("\nBytes (Compressed/Original=Savings%%): %ld/%ld=", ctot, len);
   printf("%.2f%%\n", perc);
} else {
   printf("Unishox (byte format version: 3.0)\n");
   printf("----------------------------------\n");
   printf("Usage: unishox3 \"string\"\n");
   printf("              (or)\n");
   printf("       unishox3 [action] [in_file] [out_file]\n");
   printf("\n");
   printf("         [action]:\n");
   printf("         -t    run tests\n");
   printf("         -c    compress\n");
   printf("         -d    decompress\n");
   printf("\n");
   return 1;
}

printf("\nElapsed: %0.3lf ms\n", timedifference(tStart, getTimeVal()));

return 0;

}
