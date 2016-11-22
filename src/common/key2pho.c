/**
 * @file key2pho.c
 * @brief Map bopomofos to uint32_t type according to different keyboard type.
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
#include "taigi-utf8-util.h"

const char taigi_tone[] = "0123456789";
const char taigi_pho[] =  "abeghijklmnopstuc";
const char taigi_pho_cap[] =  "ABEGHIJKLMNOPSTUC";
#define TAIGI_PHO_SPEC_NUM 98
const char *taigi_pho_spec[ TAIGI_PHO_SPEC_NUM ] =  { 
	"á", "à", "â", "ǎ", "ā", "a̍", "a̋",
	"é", "è", "ê", "ě", "ē", "e̍", "e̋",
	"í", "ì", "î", "ǐ", "ī", "i̍", "i̋",
	"ó", "ò", "ô", "ǒ", "ō", "o̍", "ő",
	"ú", "ù", "û", "ǔ", "ū", "u̍", "ű",
	"ḿ", "m̀", "m̂", "m̆", "m̄", "m̍", "m",
	"ńg","ǹg","n̂g","n̆g","n̄g","n̍g","ng",
	"Á", "À", "Â", "Ǎ", "Ā", "A̍", "A̋",
        "É", "È", "Ê", "Ě", "Ē", "E̍", "E",
        "Í", "Ì", "Î", "Ǐ", "Ī", "I̍", "I",
        "Ó", "Ò", "Ô", "Ǒ", "Ō", "O̍", "Ő",
        "Ú", "Ù", "Û", "Ǔ", "Ū", "U̍", "Ű",
        "Ḿ", "M̀", "M̂", "M̆", "M̄", "M̍", "M",
        "Ńg","Ǹg","N̂g","N̆g","N̄g","N̍g", "Ng"
};
const char *const lomaji_tab[] = {               /* number of bits */
    taigi_pho,
    taigi_tone
};
static const int phone_num = 17;
static const int tone_num = 9;

//static const int lomaji_tab_num[] = { 22, 4, 14, 5 };
//static const int shift[] = { 9, 7, 3, 0 };
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
uint32_t IsThePhone(const char c)
{
	if (strchr(taigi_pho, c) || strchr(taigi_pho_cap, c))
		return 1;
	return 0;
}

uint32_t IsTheTaiLoPhone(const char *s)
{
	int i;
	for (i = 0; i < TAIGI_PHO_SPEC_NUM; ++i) {
		if(!strncmp(taigi_pho_spec[i], s, strlen(taigi_pho_spec[i])))
			return 1;
	}
	return 0;
}


uint32_t UintFromPhone(const char *zhuin)
{
    const char *iter;
    char *pos;
    char buf[MAX_UTF8_SIZE + 1];
    uint32_t len, result = 0;
    int zhuin_index = 0;
    int offset = 0;

    iter = zhuin;
    memset(buf, 0x0, MAX_UTF8_SIZE);

    /* 0x20: space character */
    while (*iter && *iter != 0x20) {
        buf[0] = iter[0];

        for (; zhuin_index < 2; ++zhuin_index) {
		pos = strchr(lomaji_tab[ zhuin_index ], buf[0]);
		if (pos) {
	//		printf("Got %c\n", pos[0]);
			break;
		}
        }

        if (zhuin_index >= 2) {
            return 0;
        }
	offset = (int) (pos - lomaji_tab[zhuin_index]);
	if (zhuin_index == 1) {
	    break;
	}
	result = result * 17 + offset + 1;
        ++iter;
	//printf("buf=%c, zhuin_index=%d, result=%d\n", buf[0], zhuin_index, result); 
    }
    result = result << 4;
    result |= offset;
    return result;
}

int PhoneFromKey(char *pho, const char *inputkey, KBTYPE kbtype, int searchTimes)
{
    int len;

    len = strlen(inputkey);
	
    strncpy(pho, inputkey, 20);
    return 1;
}

int PhoneFromUint(char *phone, size_t phone_len, uint32_t phone_num)
{
    int i;
    int index;
    const char *pos;
    char tmp[16];
    char buffer[MAX_UTF8_SIZE * BOPOMOFO_SIZE + 1] = { 0 };
    char tone;

    //printf("%s, phone_num=0x%x\n", __func__, phone_num);
    tone = phone_num & 0xf;
    //printf("tone=%d\n", tone);
    phone_num = phone_num >> 4;

    memset(tmp, 0x0, 16);

    for (i = 0; i < BOPOMOFO_SIZE, phone_num > 0; ++i) {
        /* The first two characters in lomaji_tab are space, so we need
           to add 1 here. */
#if 0
        index = ((phone_num >> shift[i]) & mask[i]);
        if (index >= 1) {
            pos = ueConstStrSeek(lomaji_tab[i], index - 1);
            ueStrNCpy(tmp, pos, 1, STRNCPY_CLOSE);
            strcat(buffer, tmp);
        }
#endif
	index = phone_num % 17 - 1;
	tmp[i] = taigi_pho[index];
	//printf("tmp[%d]=%c\n", i, tmp[i]);
	phone_num /= 17;
    }
    int len = strlen(tmp);

    //printf("string len=%d\n", len);
    memset(phone, 0x0, len + 2);
    phone[len + 1] = tone;
    for (i = 0; i < len; ++i) {
	 phone[len - i - 1] = tmp[i]; 
	// printf("phone[%d]=%c\n", len - i, phone[len - i]);
    }
    phone[i] = taigi_tone[tone];
    //printf("phone=%s\n", phone);
    return 0;
}

int PhoneInxFromKey(int key, int type, KBTYPE kbtype, int searchTimes)
{
    char *p;

    TRACX("%s: %d, type=%d, key=%d\n", __func__, __LINE__, type, key);
    p = strchr(lomaji_tab[type], key);
    if (!p)
        return 0;
    TRACX("%s: %d\n", __func__, __LINE__);
    return *p;
}

uint32_t UintFromPhoneInx(const int ph_inx[])
{
    int i;
    uint32_t result = 0;
    char *pos;
    uint32_t offset = 0;

    for (i = 0; i < BOPOMOFO_SIZE - 1; i++) {
	    if(ph_inx[ i + 1 ] == 0)
		    break;
	    pos = strchr(lomaji_tab[0], ph_inx[i]);
	    TRACX("%s, got=%c\n", __func__, *pos);
	    result = result * 17 + (uint32_t) (pos - lomaji_tab[0]) + 1;
    }
    pos = strchr(lomaji_tab[1], ph_inx[i]);
    offset = (uint32_t) (pos - lomaji_tab[1]);
    if (pos) {
	    result = (result << 4) + offset;
    } else {
	    TRACX("No tone\n");
	    return result;
    }
    TRACX("%s END result=%d\n", __func__, result);
    return result;
}

size_t GetPhoneLen(const uint32_t *phoneSeq)
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

size_t BopomofoFromUintArray(char *const bopomofo_buf, const size_t bopomofo_len, const uint32_t *phoneSeq)
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

ssize_t UintArrayFromBopomofo(uint32_t *phone_seq, const size_t phone_len, const char *bopomofo_buf)
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

size_t GetPhoneLenFromUint(uint32_t phone_num)
{
    int i;
    size_t len = 0 ;

#if 0
    for (i = 0; i < BOPOMOFO_SIZE; ++i) {
        /* The first two characters in lomaji_tab are space, so we need
           to add 1 here. */
        int index = ((phone_num >> shift[i]) & mask[i]);
        if (index >= 1) {
            const char *pos = ueConstStrSeek(lomaji_tab[i], index - 1);
            len += ueStrNBytes(pos, 1) + 1;
        }
    }
#endif
    return len;
}
