/**
 * @file key2pho.c
 * @brief Map bopomofos to uint16_t type according to different keyboard type.
 *
 * Copyright (c) 1999, 2000, 2001
 *	Lu-chuan Kung and Kang-pen Chen.
 *	All rights reserved.
 *
 * Copyright (c) 2004-2006, 2008, 2010, 2012-2014
 *	libchewing Core Team. See ChangeLog for details.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file.
 */

/* This file is encoded in UTF-8 */
#include "key2pho-private.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "chewing-utf8-util.h"

/* NOTE:
 * The reason why we convert string literal to hex representation is for the
 * sake of portability, that avoid some buggy or faulty environment like
 * Microsoft VC9 to misinterpret the string.
 */
#if 0
const char *const zhuin_tab[] = {               /* number of bits */
    "\xE3\x84\x85\xE3\x84\x86\xE3\x84\x87\xE3\x84\x88\xE3\x84\x89"
    "\xE3\x84\x8A\xE3\x84\x8B\xE3\x84\x8C\xE3\x84\x8D\xE3\x84\x8E"
    "\xE3\x84\x8F\xE3\x84\x90\xE3\x84\x91\xE3\x84\x92\xE3\x84\x93"
    "\xE3\x84\x94\xE3\x84\x95\xE3\x84\x96\xE3\x84\x97\xE3\x84\x98"
    "\xE3\x84\x99",                             /* 5 */
    /* ㄅㄆㄇㄈㄉㄊㄋㄌㄍㄎㄏㄐㄑㄒㄓㄔㄕㄖㄗㄘㄙ */
    "\xE3\x84\xA7\xE3\x84\xA8\xE3\x84\xA9",   /* 2 */
    /* ㄧㄨㄩ */
    "\xE3\x84\x9A\xE3\x84\x9B\xE3\x84\x9C\xE3\x84\x9D\xE3\x84\x9E"
    "\xE3\x84\x9F\xE3\x84\xA0\xE3\x84\xA1\xE3\x84\xA2\xE3\x84\xA3"
    "\xE3\x84\xA4\xE3\x84\xA5\xE3\x84\xA6",     /* 4 */
    /* ㄚㄛㄜㄝㄞㄟㄠㄡㄢㄣㄤㄥㄦ */
    "\xCB\x99\xCB\x8A\xCB\x87\xCB\x8B"        /* 3 */
    /* ˙ˊˇˋ */
};
#endif
const char *const zhuin_tab[] = {               /* number of bits */
    "abeghijklmnopstuc",
    "123456789"
};
static const phone_num = 17;
static const tone_num = 9;

static const int zhuin_tab_num[] = { 22, 4, 14, 5 };
static const int shift[] = { 9, 7, 3, 0 };
static const int mask[] = { 0x1F, 0x3, 0xF, 0x7 };

static const char *const ph_str =
    "\xE3\x84\x85\xE3\x84\x86\xE3\x84\x87\xE3\x84\x88"
    /* ㄅㄆㄇㄈ */
    "\xE3\x84\x89\xE3\x84\x8A\xE3\x84\x8B\xE3\x84\x8C"
    /* ㄉㄊㄋㄌ */
    "\xE3\x84\x8D\xE3\x84\x8E\xE3\x84\x8F\xE3\x84\x90"
    /* ㄍㄎㄏㄐ */
    "\xE3\x84\x91\xE3\x84\x92\xE3\x84\x93\xE3\x84\x94"
    /* ㄑㄒㄓㄔ */
    "\xE3\x84\x95\xE3\x84\x96\xE3\x84\x97\xE3\x84\x98"
    /* ㄕㄖㄗㄘ */
    "\xE3\x84\x99\xE3\x84\xA7\xE3\x84\xA8\xE3\x84\xA9"
    /* ㄙㄧㄨㄩ */
    "\xE3\x84\x9A\xE3\x84\x9B\xE3\x84\x9C\xE3\x84\x9D"
    /* ㄚㄛㄜㄝ */
    "\xE3\x84\x9E\xE3\x84\x9F\xE3\x84\xA0\xE3\x84\xA1"
    /* ㄞㄟㄠㄡ */
    "\xE3\x84\xA2\xE3\x84\xA3\xE3\x84\xA4\xE3\x84\xA5\xE3\x84\xA6"
    /* ㄢㄣㄤㄥㄦ */
    "\xCB\x99\xCB\x8A\xCB\x87\xCB\x8B";
    /* ˙ˊˇˋ */

static const char *const key_str[KBTYPE_COUNT] = {
    "1qaz2wsxedcrfv5tgbyhnujm8ik,9ol.0p;/-7634",        /* standard kb */
    "bpmfdtnlgkhjvcjvcrzasexuyhgeiawomnkllsdfj",        /* hsu */
    "1234567890-qwertyuiopasdfghjkl;zxcvbn/m,.",        /* IBM */
    "2wsx3edcrfvtgb6yhnujm8ik,9ol.0p;/-['=1qaz",        /* Gin-yieh */
    "bpmfdtnlvkhg7c,./j;'sexuaorwiqzy890-=1234",        /* ET  */
    "bpmfdtnlvkhgvcgycjqwsexuaorwiqzpmntlhdfjk",        /* ET26 */
    "1'a;2,oq.ejpuk5yixfdbghm8ctw9rnv0lsz[7634",        /* Dvorak */
    "bpmfdtnlgkhjvcjvcrzasexuyhgeiawomnkllsdfj",        /* Dvorak Hsu */
    "qqazwwsxedcrfvttgbyhnujmuikbiolmoplnpyerd",        /* DACHEN-CP26 */
    "1qaz2wsxedcrfv5tgbyhnujm8ik,9ol.0p;/-7634",        /* Hanyu Pinyin */
    "1qaz2wsxedcrfv5tgbyhnujm8ik,9ol.0p;/-7634",        /* Luoma Pinyin */
    "1qaz2wsxedcrfv5tgbyhnujm8ik,9ol.0p;/-7634",        /* secondary Bopomofo Pinyin */
    "1qdz2gsxmtclnv5wrjyikfap8ue,9bo.0;h/-7634",        /* Carpalx */
};

/*
 * Read one zhuin string,
 *
 * return the number it means. 0 means error.
 */
uint16_t UintFromPhone(const char *zhuin)
{
    const char *iter;
    char *pos;
    char buf[MAX_UTF8_SIZE + 1];
    int len, result = 0;
    int zhuin_index = 0;

    iter = zhuin;
    memset(buf, 0x0, MAX_UTF8_SIZE);

    /* 0x20: space character */
    while (*iter && *iter != 0x20) {
        buf[0] = iter[0];

        for (; zhuin_index < 2; ++zhuin_index) {
		pos = strchr(zhuin_tab[ zhuin_index ], buf[0]);
		if (pos) {
	//		printf("Got %c\n", pos[0]);
			break;
		}
        }

        if (zhuin_index >= 2) {
            return 0;
        }
	if (zhuin_index == 0) {
		int offset = (int) (pos - zhuin_tab[0]);

	//	printf("offset=%d\n", offset);
		result = result * 17 + offset + 1;

	} else if (zhuin_index == 1) {
		int offset = (int) (pos - zhuin_tab[1]);
	//	printf("offset=%d\n", offset);
		result = result * 10 + offset + 1;
	}
        ++iter;
	//printf("buf=%c, zhuin_index=%d, result=%d\n", buf[0], zhuin_index, result); 
    }
    printf("%d\t%s\n", result, zhuin);
    return result;
}

int PhoneFromKey(char *pho, const char *inputkey, KBTYPE kbtype, int searchTimes)
{
    int len;
    int i;
    int s;
    const char *pTarget;

    //printf("\t\t\t\t%s, inputkey=%s\n", __func__, inputkey);
    len = strlen(inputkey);
	
    strncpy(pho, inputkey, 20);
#if 0
    pho[0] = '\0';
    for (i = 0; i < len; i++) {
        char *findptr = NULL;
        int _index;

        pTarget = key_str[kbtype];
        for (s = 0; s < searchTimes; s++) {
            findptr = strchr(pTarget, inputkey[i]);
            if (!findptr) {
		printf("\t\t\t\tfindptr is NULL\n");
                return 0;
            }
	    printf("%c ", pTarget);
            pTarget = findptr + 1;
        }
        _index = findptr - key_str[kbtype];
        ueStrNCpy(ueStrSeek(pho, i), ueConstStrSeek(ph_str, _index), 1, STRNCPY_NOT_CLOSE);
    }
    pho = ueStrSeek(pho, len);
    pho[0] = '\0';
    printf("\t\t\t\t%s, get pho=%s\n", __func__, pho);
#endif
    return 1;
}

int PhoneFromUint(char *phone, size_t phone_len, uint16_t phone_num)
{
    int i;
    int index;
    const char *pos;
    char tmp[MAX_UTF8_SIZE + 1];
    char buffer[MAX_UTF8_SIZE * BOPOMOFO_SIZE + 1] = { 0 };

    for (i = 0; i < BOPOMOFO_SIZE; ++i) {
        /* The first two characters in zhuin_tab are space, so we need
           to add 1 here. */
        index = ((phone_num >> shift[i]) & mask[i]);
        if (index >= 1) {
            pos = ueConstStrSeek(zhuin_tab[i], index - 1);
            ueStrNCpy(tmp, pos, 1, STRNCPY_CLOSE);
            strcat(buffer, tmp);
        }
    }
    strncpy(phone, buffer, phone_len);
    phone[phone_len - 1] = 0;
    return 0;
}

int PhoneInxFromKey(int key, int type, KBTYPE kbtype, int searchTimes)
{
    char keyStr[2];
    char rtStr[10];
    char *p;

    keyStr[0] = key;
    keyStr[1] = '\0';
    printf("\t\t\t%s, key=%d, type=%d\n", __func__, key, type);
    if (!PhoneFromKey(rtStr, keyStr, kbtype, searchTimes))
        return 0;

    p = strstr(zhuin_tab[type], rtStr);
    if (!p)
        return 0;
    {
	    int i;

	    printf("rtStr:\n");
	    for (i=0; i<10;++i) {
		    printf("%x ", (unsigned char) rtStr[i]);
	    }
	    printf("\n");
	    for (i=0; i<10;++i) {
		    printf("%c ", (unsigned char) rtStr[i]);
	    }
	    printf("\n");
    }
    return zhuin_tab_num[type] - ueStrLen(p);
}

uint16_t UintFromPhoneInx(const int ph_inx[])
{
    int i;
    uint16_t result = 0;

    for (i = 0; i < BOPOMOFO_SIZE; i++) {
        if (ph_inx[i] < 0 || ph_inx[i] >= zhuin_tab_num[i])
            return 0;

        result |= ph_inx[i] << shift[i];
    }

    return result;
}

size_t GetPhoneLen(const uint16_t *phoneSeq)
{
    size_t len = 0;

    assert(phoneSeq);

    while (phoneSeq[len])
        ++len;
    return len;
}

static size_t GetBopomofoCount(const char *bopomofo_buf)
{
    size_t count = 0;

    assert(bopomofo_buf);

    while ((bopomofo_buf = strpbrk(bopomofo_buf, " ")) != NULL) {
        ++count;
        bopomofo_buf += 1;
    }

    return count;
}

size_t BopomofoFromUintArray(char *const bopomofo_buf, const size_t bopomofo_len, const uint16_t *phoneSeq)
{
    size_t i;
    size_t len;
    size_t buf_len;
    size_t shift = 0;

    assert(phoneSeq);

    len = GetPhoneLen(phoneSeq);
    buf_len = GetBopomofoBufLen(len);

    if (bopomofo_buf && bopomofo_len >= buf_len) {
        for (i = 0; i < len; ++i) {
            PhoneFromUint(bopomofo_buf + shift, bopomofo_len - shift, phoneSeq[i]);
            strcat(bopomofo_buf + shift, " ");
            shift += strlen(bopomofo_buf + shift);
        }
        if (shift)
            bopomofo_buf[shift - 1] = 0;
    }
    return buf_len;
}

ssize_t UintArrayFromBopomofo(uint16_t *phone_seq, const size_t phone_len, const char *bopomofo_buf)
{
    size_t i;
    size_t len;

    assert(bopomofo_buf);

    len = GetBopomofoCount(bopomofo_buf) + 1;
    if (!phone_seq)
        return len;

    if (phone_len <= len)
        return -1;

    for (i = 0; i < len; ++i) {
        phone_seq[i] = UintFromPhone(bopomofo_buf);
        if (phone_seq[i] == 0)
            return -1;
        bopomofo_buf = strpbrk(bopomofo_buf, " ") + 1;
    }
    phone_seq[len] = 0;

    return len;
}

size_t GetBopomofoBufLen(size_t len)
{
    return (MAX_UTF8_SIZE * BOPOMOFO_SIZE + 1) * len;
}

size_t GetPhoneLenFromUint(uint16_t phone_num)
{
    int i;
    size_t len = 0 ;

    for (i = 0; i < BOPOMOFO_SIZE; ++i) {
        /* The first two characters in zhuin_tab are space, so we need
           to add 1 here. */
        int index = ((phone_num >> shift[i]) & mask[i]);
        if (index >= 1) {
            const char *pos = ueConstStrSeek(zhuin_tab[i], index - 1);
            len += ueStrNBytes(pos, 1) + 1;
        }
    }
    return len;
}
