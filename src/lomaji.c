/**
 * lomaji.c
 */

/**
 * @file lomaji.c
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

    TRACX("##### %s, %d Start\n", __func__, __LINE__);
    /* Only for the tone 1~8 */
    pho_inx = PhoneInxFromKey(key, 1, pBopomofo->kbtype, searchTimes);

    int len = 0;
    while(pBopomofo->pho_inx[len] != 0) {
	    printf("pBopomofo->pho_inx[%d]=%c\n", len, pBopomofo->pho_inx[len]);
	    ++len;
    };
    pBopomofo->pho_inx[len] = pho_inx;
    TRACX("pBopomofo->pho_inx[%d]=%c\n", len, pBopomofo->pho_inx[len]);

    u32Pho = UintFromPhoneInx(pBopomofo->pho_inx);
    if (GetCharFirst(pgdata, &tempword, u32Pho) == 0) {
	TRACX("%s, %d, u32Pho=%d\n", __func__, __LINE__, u32Pho);
        BopomofoRemoveAll(pBopomofo);
        return BOPOMOFO_NO_WORD;
    }

    TRACX("%s, %d, get tempword: phrase=%s, freq=%d\n", __func__ , __LINE__, tempword.phrase, tempword.freq);
    pBopomofo->phone = u32Pho;
    pBopomofo->phoneAlt = u32Pho;
    memset(pBopomofo->pho_inx, 0, sizeof(pBopomofo->pho_inx));
    TRACX("##### %s, %d, END #####\n", __func__, __LINE__);
    return BOPOMOFO_COMMIT;
}

static int DefPhoInput(ChewingData *pgdata, int key)
{
    BopomofoData *pBopomofo = &(pgdata->bopomofoData);
    int type = 0, inx = 0;
    int i;

    TRACX("\t\t%s, key=%d\n", __func__, key);
    if (IsDefPhoEndKey(key, pBopomofo->kbtype)) {
        for (i = 0; i < BOPOMOFO_SIZE; ++i)
            if (pBopomofo->pho_inx[i] == 0)
                break;
	if (i == 0) {
		// The first key is Num, should be key error
		return BOPOMOFO_KEY_ERROR;
	}
	TRACX("This is END key is %c at:%d\n", key, i);
        if (i < BOPOMOFO_SIZE) {
	    pBopomofo->pho_inx_n = 0;
            return EndKeyProcess(pgdata, key, 1);
	}
    } else {
        pBopomofo->pho_inx[pBopomofo->pho_inx_n] = 0;
    }
   TRACX("<<<<< %s, %d >>>>>\n", __func__, __LINE__);
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
    TRACX("%s, %d, pBopomofo->pho_inx[%d]=%d\n", __func__, __LINE__, pBopomofo->pho_inx_n, inx);
    pBopomofo->pho_inx[pBopomofo->pho_inx_n++] = inx;
    return BOPOMOFO_ABSORB;
}


/* copy the idea from hsu */

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


/* key: ascii code of input, including space */
int LomajiInput(ChewingData *pgdata, int key)
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
    case KB_ET26:
    case KB_DACHEN_CP26:
    case KB_HANYU_PINYIN:
    case KB_THL_PINYIN:
    case KB_MPS2_PINYIN:
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
