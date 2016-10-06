/**
 * bopomofo.c
 *
 * Copyright (c) 1999, 2000, 2001
 *      Lu-chuan Kung and Kang-pen Chen.
 *      All rights reserved.
 *
 * Copyright (c) 2004-2006, 2008-2010, 2012-2014
 *      libchewing Core Team. See ChangeLog for details.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file.
 */

/**
 * @file bopomofo.c
 *
 * control keyboard mapping
 * include the definition of BopomofoData structure
 */

#include <ctype.h>
#include <string.h>

#include "bopomofo-private.h"
#include "taigiutil.h"
#include "dict-private.h"
#include "key2pho-private.h"
#include "pinyin-private.h"
#include "private.h"

/*
 * process a key input
 * return value:
 *      BOPOMOFO_ABSORB
 *      BOPOMOFO_COMMIT
 *      BOPOMOFO_KEY_ERROR
 *      BOPOMOFO_ERROR
 */
static int IsHsuPhoEndKey(const int pho_inx[], int key)
{
    switch (key) {
    case 's':
    case 'd':
    case 'f':
    case 'j':
    case ' ':
        return (pho_inx[0] || pho_inx[1] || pho_inx[2]);
    default:
        return 0;
    }
}

/* copy the idea from HSU keyboard */
static int IsET26PhoEndKey(const int pho_inx[], int key)
{
    switch (key) {
    case 'd':
    case 'f':
    case 'j':
    case 'k':
    case ' ':
        return (pho_inx[0] || pho_inx[1] || pho_inx[2]);
    default:
        return 0;
    }
}

/* copy the idea from HSU keyboard */
static int IsDACHENCP26PhoEndKey(const int pho_inx[], int key)
{
    switch (key) {
    case 'e':
    case 'r':
    case 'd':
    case 'y':
    case ' ':
        return (pho_inx[0] || pho_inx[1] || pho_inx[2]);
    default:
        return 0;
    }
}

static int IsDefPhoEndKey(int key, int kbtype)
{
    if (PhoneInxFromKey(key, 1, kbtype, 1))
        return 1;

    if (key == ' ')
        return 1;
    return 0;
}

static int EndKeyProcess(ChewingData *pgdata, int key, int searchTimes)
{
    BopomofoData *pBopomofo = &(pgdata->bopomofoData);
    uint32_t u32Pho, u32PhoAlt;
    Phrase tempword;
    int pho_inx;

    printf("##### %s, %d Start\n", __func__, __LINE__);
    /* Only for the tone 1~8 */
    pho_inx = PhoneInxFromKey(key, 1, pBopomofo->kbtype, searchTimes);

    int len = 0;
    while(pBopomofo->pho_inx[len] != 0) {
	    printf("pBopomofo->pho_inx[%d]=%c\n", len, pBopomofo->pho_inx[len]);
	    ++len;
    };
    pBopomofo->pho_inx[len] = pho_inx;
    printf("pBopomofo->pho_inx[%d]=%c\n", len, pBopomofo->pho_inx[len]);

    u32Pho = UintFromPhoneInx(pBopomofo->pho_inx);
    if (GetCharFirst(pgdata, &tempword, u32Pho) == 0) {
	printf("%s, %d, u32Pho=%d\n", __func__, __LINE__, u32Pho);
        BopomofoRemoveAll(pBopomofo);
        return BOPOMOFO_NO_WORD;
    }

    printf("%s, %d, get tempword: phrase=%s, freq=%d\n", __func__ , __LINE__, tempword.phrase, tempword.freq);
    pBopomofo->phone = u32Pho;
    pBopomofo->phoneAlt = u32Pho;
    memset(pBopomofo->pho_inx, 0, sizeof(pBopomofo->pho_inx));
    printf("##### %s, %d, END #####\n", __func__, __LINE__);
    return BOPOMOFO_COMMIT;
}

static int DefPhoInput(ChewingData *pgdata, int key)
{
    BopomofoData *pBopomofo = &(pgdata->bopomofoData);
    int type = 0, inx = 0;
    int i;

    printf("\t\t%s, key=%d\n", __func__, key);
    if (IsDefPhoEndKey(key, pBopomofo->kbtype)) {
	printf("This is END keyn\n");
        for (i = 0; i < BOPOMOFO_SIZE; ++i)
            if (pBopomofo->pho_inx[i] != 0)
                break;
        if (i < BOPOMOFO_SIZE) {
	    pBopomofo->pho_inx_n = 0;
            return EndKeyProcess(pgdata, key, 1);
	}
    } else {
        pBopomofo->pho_inx[pBopomofo->pho_inx_n] = 0;
    }
   printf("<<<<< %s, %d >>>>>\n", __func__, __LINE__);
#define TAIGI_TYPES 2
    /* decide if the key is a phone */
    for (type = 0; type < TAIGI_TYPES; type++) {
        inx = PhoneInxFromKey(key, type, pBopomofo->kbtype, 1);
        if (inx)
            break;
    }

    /* the key is NOT a phone */
    if (type >= TAIGI_TYPES) {
        return BOPOMOFO_KEY_ERROR;
    }

    /* fill the key into the phone buffer */
    printf("%s, %d, pBopomofo->pho_inx[%d]=%d\n", __func__, __LINE__, pBopomofo->pho_inx_n, inx);
    pBopomofo->pho_inx[pBopomofo->pho_inx_n++] = inx;
    return BOPOMOFO_ABSORB;
}

static int HsuPhoInput(ChewingData *pgdata, int key)
{
    BopomofoData *pBopomofo = &(pgdata->bopomofoData);
    int type = 0, searchTimes = 0, inx = 0;
	
    printf("%s, %d\n", __func__, __LINE__);
#if 0
    /* Dvorak Hsu key has already converted to Hsu */
    if (IsHsuPhoEndKey(pBopomofo->pho_inx, key)) {
        if (pBopomofo->pho_inx[1] == 0 && pBopomofo->pho_inx[2] == 0) {
            /* convert "ㄐㄑㄒ" to "ㄓㄔㄕ" */
            if (12 <= pBopomofo->pho_inx[0] && pBopomofo->pho_inx[0] <= 14) {
                pBopomofo->pho_inx[0] += 3;
            }
            /* convert "ㄏ" to "ㄛ" */
            else if (pBopomofo->pho_inx[0] == 11) {
                pBopomofo->pho_inx[0] = 0;
                pBopomofo->pho_inx[2] = 2;
            }
            /* convert "ㄍ" to "ㄜ" */
            else if (pBopomofo->pho_inx[0] == 9) {
                pBopomofo->pho_inx[0] = 0;
                pBopomofo->pho_inx[2] = 3;
            }
            /* convert "ㄇ" to "ㄢ" */
            else if (pBopomofo->pho_inx[0] == 3) {
                pBopomofo->pho_inx[0] = 0;
                pBopomofo->pho_inx[2] = 9;
            }
            /* convert "ㄋ" to "ㄣ" */
            else if (pBopomofo->pho_inx[0] == 7) {
                pBopomofo->pho_inx[0] = 0;
                pBopomofo->pho_inx[2] = 10;
            }
            /* convert "ㄎ" to "ㄤ" */
            else if (pBopomofo->pho_inx[0] == 10) {
                pBopomofo->pho_inx[0] = 0;
                pBopomofo->pho_inx[2] = 11;
            }
            /* convert "ㄌ" to "ㄦ" */
            else if (pBopomofo->pho_inx[0] == 8) {
                pBopomofo->pho_inx[0] = 0;
                pBopomofo->pho_inx[2] = 13;
            }
        }

        /* fuzzy ㄍㄧ to ㄐㄧ and ㄍㄩ to ㄐㄩ */
        if ((pBopomofo->pho_inx[0] == 9) && ((pBopomofo->pho_inx[1] == 1) || (pBopomofo->pho_inx[1] == 3))) {
            pBopomofo->pho_inx[0] = 12;
        }

        searchTimes = (key == 'j') ? 3 : 2;

        return EndKeyProcess(pgdata, key, searchTimes);
    } else {
        /* decide if the key is a phone */
        for (type = 0, searchTimes = 1; type < 3; type++) {
            inx = PhoneInxFromKey(key, type, pBopomofo->kbtype, searchTimes);
            if (!inx)
                continue;       /* if inx == 0, next type */
            else if (type == 0) {
                /**
                 * Hsu maps multiple bopomofo into one single key.
                 * Therefore, if a consonant or a medial already exists
                 * in buffer, and the user presses a key with consonant
                 * and rhyme, libchewing should consider that the user
                 * wants to input the rhyme.
                 */
                if ((inx == 3 || (7 <= inx && inx <= 11) || inx == 20)
                    && (pBopomofo->pho_inx[0] || pBopomofo->pho_inx[1])) {
                    /* if inx !=0 */
                    searchTimes = 2;    /* possible infinite loop here */
                } else
                    break;
            } else if (type == 1 && inx == 1) { /* handle i and e */
                if (pBopomofo->pho_inx[1]) {
                    searchTimes = 2;
                } else
                    break;
            } else
                break;
        }

        /* fuzzy ㄍㄧ to ㄐㄧ and ㄍㄩ to ㄐㄩ */
        if ((pBopomofo->pho_inx[0] == 9) && ((pBopomofo->pho_inx[1] == 1) || (pBopomofo->pho_inx[1] == 3))) {
            pBopomofo->pho_inx[0] = 12;
        }

        /* ㄐㄑㄒ must be followed by ㄧㄩ, if not, convert them to ㄓㄔㄕ */
        if (type == 1 && inx == 2 && 12 <= pBopomofo->pho_inx[0] && pBopomofo->pho_inx[0] <= 14) {
            /* followed by ㄨ */
            pBopomofo->pho_inx[0] += 3;
        }

        if (type == 2 && pBopomofo->pho_inx[1] == 0 && 12 <= pBopomofo->pho_inx[0] && pBopomofo->pho_inx[0] <= 14) {
            /* followed by other phones */
            pBopomofo->pho_inx[0] += 3;
        }

        if (type == 3) {        /* the key is NOT a phone */
            if (isalpha(key))
                return BOPOMOFO_NO_WORD;
            return BOPOMOFO_KEY_ERROR;
        }
        /* fill the key into the phone buffer */
        pBopomofo->pho_inx[type] = inx;
#endif
        return BOPOMOFO_ABSORB;
}

/* copy the idea from hsu */
static int ET26PhoInput(ChewingData *pgdata, int key)
{
    BopomofoData *pBopomofo = &(pgdata->bopomofoData);
    int type = 0, searchTimes = 0, inx = 0;

    if (IsET26PhoEndKey(pBopomofo->pho_inx, key)) {
        if (pBopomofo->pho_inx[1] == 0 && pBopomofo->pho_inx[2] == 0) {
            /* convert "ㄐㄒ" to "ㄓㄕ" */
            if (pBopomofo->pho_inx[0] == 12 || pBopomofo->pho_inx[0] == 14) {
                pBopomofo->pho_inx[0] += 3;
            }
            /* convert "ㄆ" to "ㄡ" */
            else if (pBopomofo->pho_inx[0] == 2) {
                pBopomofo->pho_inx[0] = 0;
                pBopomofo->pho_inx[2] = 8;
            }
            /* convert "ㄇ" to "ㄢ" */
            else if (pBopomofo->pho_inx[0] == 3) {
                pBopomofo->pho_inx[0] = 0;
                pBopomofo->pho_inx[2] = 9;
            }
            /* convert "ㄋ" to "ㄣ" */
            else if (pBopomofo->pho_inx[0] == 7) {
                pBopomofo->pho_inx[0] = 0;
                pBopomofo->pho_inx[2] = 10;
            }
            /* convert "ㄊ" to "ㄤ" */
            else if (pBopomofo->pho_inx[0] == 6) {
                pBopomofo->pho_inx[0] = 0;
                pBopomofo->pho_inx[2] = 11;
            }
            /* convert "ㄌ" to "ㄥ" */
            else if (pBopomofo->pho_inx[0] == 8) {
                pBopomofo->pho_inx[0] = 0;
                pBopomofo->pho_inx[2] = 12;
            }
            /* convert "ㄏ" to "ㄦ" */
            else if (pBopomofo->pho_inx[0] == 11) {
                pBopomofo->pho_inx[0] = 0;
                pBopomofo->pho_inx[2] = 13;
            }
        }
        searchTimes = 2;
        return EndKeyProcess(pgdata, key, searchTimes);
    } else {
        /* decide if the key is a phone */
        for (type = 0, searchTimes = 1; type < 3; type++) {
            inx = PhoneInxFromKey(key, type, pBopomofo->kbtype, searchTimes);
            if (!inx)
                continue;       /* if inx == 0, next type */
            else if (type == 0) {
                 /**
                  * Same as Hsu: If a consonant or a medial already exists
                  * in buffer, and the user presses a key with consonant
                  * and rhyme, libchewing should consider that the user
                  * wants to input the rhyme.
                  */
                if ((inx == 2 || inx == 3 || inx == 11 || inx == 19 || inx == 20 ||
                    (6 <= inx && inx <= 8)) && (pBopomofo->pho_inx[0] || pBopomofo->pho_inx[1])) {
                    /* if inx !=0 */
                    searchTimes = 2;    /* possible infinite loop here */
                } else
                    break;
            } else
                break;
        }
        /* convert "ㄐㄒ" to "ㄓㄕ" */
        if (type == 1) {
            if (inx == 2) {
                if (pBopomofo->pho_inx[0] == 12 || pBopomofo->pho_inx[0] == 14) {
                    pBopomofo->pho_inx[0] += 3;
                }
            } else {
                /* convert "ㄍ" to "ㄑ" */
                if (pBopomofo->pho_inx[0] == 9) {
                    pBopomofo->pho_inx[0] = 13;
                }
            }
        }

        if (type == 2 && pBopomofo->pho_inx[1] == 0 && (pBopomofo->pho_inx[0] == 12 || pBopomofo->pho_inx[0] == 14)) {
            pBopomofo->pho_inx[0] += 3;
        }

        if (type == 3) {        /* the key is NOT a phone */
            if (isalpha(key))
                return BOPOMOFO_NO_WORD;
            return BOPOMOFO_KEY_ERROR;
        }
        /* fill the key into the phone buffer */
        pBopomofo->pho_inx[type] = inx;
        return BOPOMOFO_ABSORB;
    }
}

static int SwitchingBetween(int *pho_idx, int a, int b)
{
    if (*pho_idx == a) {
        *pho_idx = b;
        return 1;
    } else if (*pho_idx == b) {
        *pho_idx = a;
        return 1;
    }
    return 0;
}

static int DACHENCP26PhoInput(ChewingData *pgdata, int key)
{
    BopomofoData *pBopomofo = &(pgdata->bopomofoData);
    int type = 0, searchTimes = 0, inx = 0;

    if (IsDACHENCP26PhoEndKey(pBopomofo->pho_inx, key)) {
        searchTimes = 2;
        return EndKeyProcess(pgdata, key, searchTimes);
    } else {
        /* decide if the key is a phone */
        for (type = 0, searchTimes = 1; type < 3; type++) {
            inx = PhoneInxFromKey(key, type, pBopomofo->kbtype, searchTimes);
            if (!inx)
                continue;       /* if inx == 0, next type */
            else if (type == 0) {
                break;
                if (pBopomofo->pho_inx[0] || pBopomofo->pho_inx[1]) {
                    /* if inx !=0 */
                    searchTimes = 2;    /* possible infinite loop here */
                } else
                    break;
            } else
                break;
        }
        /* switching between "ㄅ" and "ㄆ" */
        if (key == 'q') {
            if (SwitchingBetween(&pBopomofo->pho_inx[0], 1, 2)) {
                return BOPOMOFO_ABSORB;
            }
        }
        /* switching between "ㄉ" and "ㄊ" */
        else if (key == 'w') {
            if (SwitchingBetween(&pBopomofo->pho_inx[0], 5, 6)) {
                return BOPOMOFO_ABSORB;
            }
        }
        /* switching between "ㄓ" and "ㄔ" */
        else if (key == 't') {
            if (SwitchingBetween(&pBopomofo->pho_inx[0], 15, 16)) {
                return BOPOMOFO_ABSORB;
            }
        }
        /* converting "ㄖ" to "ㄝ" */
        else if (key == 'b') {
            if (pBopomofo->pho_inx[0] != 0 || pBopomofo->pho_inx[1] != 0) {
                pBopomofo->pho_inx[2] = 4;
                return BOPOMOFO_ABSORB;
            }
        }
        /* converting "ㄙ" to "ㄥ" */
        else if (key == 'n') {
            if (pBopomofo->pho_inx[0] != 0 || pBopomofo->pho_inx[1] != 0) {
                pBopomofo->pho_inx[2] = 12;
                return BOPOMOFO_ABSORB;
            }
        }
        /* switching between "ㄧ", "ㄚ", and "ㄧㄚ" */
        else if (key == 'u') {
            if (pBopomofo->pho_inx[1] == 1 && pBopomofo->pho_inx[2] != 1) {
                pBopomofo->pho_inx[1] = 0;
                pBopomofo->pho_inx[2] = 1;
                return BOPOMOFO_ABSORB;
            } else if (pBopomofo->pho_inx[1] != 1 && pBopomofo->pho_inx[2] == 1) {
                pBopomofo->pho_inx[1] = 1;
                return BOPOMOFO_ABSORB;
            } else if (pBopomofo->pho_inx[1] == 1 && pBopomofo->pho_inx[2] == 1) {
                pBopomofo->pho_inx[1] = 0;
                pBopomofo->pho_inx[2] = 0;
                return BOPOMOFO_ABSORB;
            } else if (pBopomofo->pho_inx[1] != 0) {
                pBopomofo->pho_inx[2] = 1;
                return BOPOMOFO_ABSORB;
            }
        }
        /* switching between "ㄩ" and "ㄡ" */
        else if (key == 'm') {
            if (pBopomofo->pho_inx[1] == 3 && pBopomofo->pho_inx[2] != 8) {
                pBopomofo->pho_inx[1] = 0;
                pBopomofo->pho_inx[2] = 8;
                return BOPOMOFO_ABSORB;
            } else if (pBopomofo->pho_inx[1] != 3 && pBopomofo->pho_inx[2] == 8) {
                pBopomofo->pho_inx[1] = 3;
                pBopomofo->pho_inx[2] = 0;
                return BOPOMOFO_ABSORB;
            } else if (pBopomofo->pho_inx[1] != 0) {
                pBopomofo->pho_inx[2] = 8;
                return BOPOMOFO_ABSORB;
            }
        }
        /* switching between "ㄛ" and "ㄞ" */
        else if (key == 'i') {
            if (SwitchingBetween(&pBopomofo->pho_inx[2], 2, 5)) {
                return BOPOMOFO_ABSORB;
            }
        }
        /* switching between "ㄟ" and "ㄢ" */
        else if (key == 'o') {
            if (SwitchingBetween(&pBopomofo->pho_inx[2], 6, 9)) {
                return BOPOMOFO_ABSORB;
            }
        }
        /* switching between "ㄠ" and "ㄤ" */
        else if (key == 'l') {
            if (SwitchingBetween(&pBopomofo->pho_inx[2], 7, 11)) {
                return BOPOMOFO_ABSORB;
            }
        }
        /* switching between "ㄣ" and "ㄦ" */
        else if (key == 'p') {
            if (SwitchingBetween(&pBopomofo->pho_inx[2], 10, 13)) {
                return BOPOMOFO_ABSORB;
            }
        }
        if (type == 3) {        /* the key is NOT a phone */
            if (isalpha(key))
                return BOPOMOFO_NO_WORD;
            return BOPOMOFO_KEY_ERROR;
        }
        /* fill the key into the phone buffer */
        pBopomofo->pho_inx[type] = inx;
        return BOPOMOFO_ABSORB;
    }
}

static int IsPinYinEndKey(int key)
{
    if ((key == ' ') || (key == '1') || (key == '2') || (key == '3') || (key == '4') || (key == '5')) {
        return 1;
    }
    return 0;
}

static int IsSymbolKey(int key)
{
    if ((key < 97) || (key > 122)) {
        return 1;
    }

    return 0;
}

static int PinYinInput(ChewingData *pgdata, int key)
{
    BopomofoData *pBopomofo = &(pgdata->bopomofoData);
    int err = 0;
    unsigned int i;
    char bopomofoKeySeq[5], bopomofoKeySeqAlt[5], buf[2];
    size_t len;

    DEBUG_CHECKPOINT();

    if (pBopomofo->pinYinData.keySeq[0] == 0 && IsSymbolKey(key)) {
        return BOPOMOFO_KEY_ERROR;
    }

    buf[0] = key;
    buf[1] = '\0';
    strcat(pBopomofo->pinYinData.keySeq, buf);

    DEBUG_OUT("PinYin Seq: %s\n", pBopomofo->pinYinData.keySeq);

    return BOPOMOFO_ABSORB;
}

/* key: ascii code of input, including space */
int BopomofoPhoInput(ChewingData *pgdata, int key)
{
    BopomofoData *pBopomofo = &(pgdata->bopomofoData);

    DEBUG_OUT("\n\t%s, key=%d\n", __func__, key);
    /* open symbol table */
    if (key == '`') {
        pgdata->bSelect = 1;
        pgdata->choiceInfo.oldChiSymbolCursor = pgdata->chiSymbolCursor;
        HaninSymbolInput(pgdata);
        return BOPOMOFO_OPEN_SYMBOL_TABLE;
    }
    switch (pBopomofo->kbtype) {
    case KB_HSU:
    case KB_DVORAK_HSU:
        return HsuPhoInput(pgdata, key);
        break;
    case KB_ET26:
        return ET26PhoInput(pgdata, key);
        break;
    case KB_DACHEN_CP26:
        return DACHENCP26PhoInput(pgdata, key);
        break;
    case KB_HANYU_PINYIN:
    case KB_THL_PINYIN:
    case KB_MPS2_PINYIN:
        return PinYinInput(pgdata, key);
        break;
    default:
        return DefPhoInput(pgdata, key);
    }
    return BOPOMOFO_ERROR;
}

/* remove the latest key */
int BopomofoRemoveLast(BopomofoData *pBopomofo)
{
    int i;

    if (pBopomofo->kbtype >= KB_HANYU_PINYIN) {
        i = strlen(pBopomofo->pinYinData.keySeq);
        pBopomofo->pinYinData.keySeq[i - 1] = '\0';
    } else {
        for (i = BOPOMOFO_SIZE; i >= 0; i--) {
            if (pBopomofo->pho_inx[i]) {
                pBopomofo->pho_inx[i] = 0;
		if (pBopomofo->pho_inx_n)
			pBopomofo->pho_inx_n--;
                return 0;
            }
        }
    }
    return 0;
}

/* remove all the key entered */
int BopomofoRemoveAll(BopomofoData *pBopomofo)
{
    memset(pBopomofo->pho_inx, 0, sizeof(pBopomofo->pho_inx));
    memset(pBopomofo->pinYinData.keySeq, 0, sizeof(pBopomofo->pinYinData.keySeq));
    return 0;
}

int BopomofoIsEntering(BopomofoData *pBopomofo)
{
    int i;

    if (pBopomofo->kbtype >= KB_HANYU_PINYIN) {
        if (pBopomofo->pinYinData.keySeq[0])
            return 1;
    } else {
        for (i = 0; i < BOPOMOFO_SIZE; i++)
            if (pBopomofo->pho_inx[i])
                return 1;
    }
    return 0;
}
