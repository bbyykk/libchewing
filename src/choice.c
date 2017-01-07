/**
 * choice.c
 *
 * Copyright (c) 1999, 2000, 2001
 *      Lu-chuan Kung and Kang-pen Chen.
 *      All rights reserved.
 *
 * Copyright (c) 2004-2008, 2010-2014
 *      libchewing Core Team. See ChangeLog for details.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file.
 */

/**
 * @file choice.c
 * @brief Choice module
 */

#include <string.h>
#include <assert.h>

#include "taigi-utf8-util.h"
#include "global.h"
#include "dict-private.h"
#include "taigiutil.h"
#include "tree-private.h"
#include "userphrase-private.h"
#include "choice-private.h"
#include "bopomofo-private.h"
#include "key2pho-private.h"
#include "private.h"

static void ChangeSelectIntervalAndBreakpoint(ChewingData *pgdata, int from, int to, const char *str, int type)
{
    int i;
    int user_alloc;

    IntervalType inte;

    
    inte.from = from;
    inte.to = to;
    for (i = 0; i < pgdata->nSelect; i++) {
        if (IsIntersect(inte, pgdata->selectInterval[i])) {
            RemoveSelectElement(i, pgdata);
            i--;
        }
    }

    pgdata->selectInterval[pgdata->nSelect].from = from;
    pgdata->selectInterval[pgdata->nSelect].to = to;
    pgdata->selectInterval[pgdata->nSelect].type = type;

    /* No available selection */
    if ((user_alloc = (to - from)) == 0)
        return;
    
    if(pgdata->selectInterval[pgdata->nSelect].type == TYPE_TAILO)
	    strncpy(pgdata->selectStr[pgdata->nSelect], str, 32);
    else
	    ueStrNCpy(pgdata->selectStr[pgdata->nSelect], str, 16, 1);
    pgdata->nSelect++;

    if (user_alloc > 1) {
        memset(&pgdata->bUserArrBrkpt[from + 1], 0, sizeof(int) * (user_alloc - 1));
        memset(&pgdata->bUserArrCnnct[from + 1], 0, sizeof(int) * (user_alloc - 1));
    }
}

/** @brief Loading all possible phrases after the cursor from long to short into AvailInfo structure.*/
static void SetAvailInfo(ChewingData *pgdata, int begin, int end)
{
    AvailInfo *pai = &(pgdata->availInfo);
    const uint32_t *phoneSeq = pgdata->phoneSeq;
    int nPhoneSeq = pgdata->nPhoneSeq;
    const int *bSymbolArrBrkpt = pgdata->bSymbolArrBrkpt;
    int symbolArrBrkpt[ARRAY_SIZE(pgdata->bSymbolArrBrkpt)] = { 0 };

    const TreeType *tree_pos;
    int diff;
    uint32_t userPhoneSeq[MAX_PHONE_SEQ_LEN];

    int i, head, head_tmp;
    int tail, tail_tmp;
    int pos;

    head = tail = 0;

    pai->nAvail = 0;

    /*
     * XXX: The phoneSeq, nPhoneSeq skip any symbol in preedit buffer,
     * while bSymbolArrBrkpt, does not skip any symbol in preedit
     * buffer. So we need to do some translate here.
     */
    for (i = 0; i < pgdata->chiSymbolBufLen; ++i) {
        if (bSymbolArrBrkpt[i]) {
            /*
             * XXX: If preedit buffer starts with symbol, the pos
             * will become negative. In this case, we just ignore
             * this symbol because it does not create any break
             * point.
             */
            pos = i - CountSymbols(pgdata, i + 1);
            if (pos >= 0)
                symbolArrBrkpt[pos] = 1;
        }
    }

    if (pgdata->config.bPhraseChoiceRearward) {
        for (i = end; i >= begin; i--) {
            if (symbolArrBrkpt[i])
                break;
            head = i;
        }
        head_tmp = end;
    } else {
        head_tmp = head = begin;
    }

    if (pgdata->config.bPhraseChoiceRearward) {
        tail_tmp = tail = end;
    } else {
        for (i = begin; i < nPhoneSeq; i++) {
            tail = i;
            if (symbolArrBrkpt[i])
                break;
        }
        tail_tmp = begin;
    }

    while (head <= head_tmp && tail_tmp <= tail) {
        diff = tail_tmp - head_tmp;
        tree_pos = TreeFindPhrase(pgdata, head_tmp, tail_tmp, phoneSeq);

        if (tree_pos) {
            /* save it! */
            pai->avail[pai->nAvail].len = diff + 1;
            pai->avail[pai->nAvail].id = tree_pos;
            pai->nAvail++;
        } else {
            memcpy(userPhoneSeq, &phoneSeq[head_tmp], sizeof(uint32_t) * (diff + 1));
            userPhoneSeq[diff + 1] = 0;
            if (UserGetPhraseFirst(pgdata, userPhoneSeq)) {
                /* save it! */
                pai->avail[pai->nAvail].len = diff + 1;
                pai->avail[pai->nAvail].id = NULL;
                pai->nAvail++;
            } else {
                pai->avail[pai->nAvail].len = 0;
                pai->avail[pai->nAvail].id = NULL;
            }
            UserGetPhraseEnd(pgdata, userPhoneSeq);
        }

        if (pgdata->config.bPhraseChoiceRearward) {
            head_tmp--;
        } else {
            tail_tmp++;
        }
    }
}

/* FIXME: Improper use of len parameter */
static int ChoiceTheSame(ChoiceInfo *pci, const char *str, int len)
{
    int i;

    for (i = 0; i < pci->nTotalChoice; i++)
        if (!memcmp(pci->totalChoiceStr[i], str, len))
            return 1;
    return 0;
}

static void ChoiceInfoAppendChi(ChewingData *pgdata, ChoiceInfo *pci, uint32_t phone)
{
    Phrase tempWord;
    int len;

    TRACX("---- %s, %d -----\n", __func__, __LINE__);
    if (GetCharFirst(pgdata, &tempWord, phone)) {
        do {
            //len = ueBytesFromChar(tempWord.phrase[0]);
            len = strlen(tempWord.phrase);
	    TRACX("---- string len=%d xxxxx\n", strlen(tempWord.phrase));
	    {
		    int j;
		    for (j=0;j< 10;j++) {
			    TRACX("%02X ", (unsigned char) tempWord.phrase[j]);
		    }
	    }
            if (ChoiceTheSame(pci, tempWord.phrase, len))
                continue;
            assert(pci->nTotalChoice < MAX_CHOICE);
            memcpy(pci->totalChoiceStr[pci->nTotalChoice], tempWord.phrase, len);
            pci->totalChoiceStr[pci->nTotalChoice]
                [len] = '\0';
	    pci->totalChoiceType[pci->nTotalChoice] = tempWord.type;
	    TRACX("---- %s, %d: totalChoiceStr[%d]=%s, type=%d\n", __func__, __LINE__,
			    pci->nTotalChoice, pci->totalChoiceStr[pci->nTotalChoice],
			    pci->totalChoiceType[pci->nTotalChoice]);
            pci->nTotalChoice++;
        } while (GetVocabNext(pgdata, &tempWord));
    }
}

/** @brief Loading all possible phrases of certain length.
 *
 * Loading all possible phrases of certain length into ChoiceInfo structure
 * from static and dynamic dictionaries, including number of total pages and
 * the number of current page.
 */
static void SetChoiceInfo(ChewingData *pgdata)
{
    Phrase tempPhrase;
    int len;
    UserPhraseData *pUserPhraseData;
    uint32_t userPhoneSeq[MAX_PHONE_SEQ_LEN];

    ChoiceInfo *pci = &(pgdata->choiceInfo);
    AvailInfo *pai = &(pgdata->availInfo);
    uint32_t *phoneSeq = pgdata->phoneSeq;
    uint32_t *phoneSeqAlt = pgdata->phoneSeqAlt;
    int cursor = PhoneSeqCursor(pgdata);
    int candPerPage = pgdata->config.candPerPage;

    /* Clears previous candidates. */
    memset(pci->totalChoiceStr, '\0', MAX_CHOICE * MAX_PHRASE_LEN * MAX_UTF8_SIZE + 1);

    pci->nTotalChoice = 0;
    len = pai->avail[pai->currentAvail].len;
    assert(len);

    /* secondly, read tree phrase */
    if (len == 1) {             /* single character */
        ChoiceInfoAppendChi(pgdata, pci, phoneSeq[cursor]);

        if (phoneSeq[cursor] != phoneSeqAlt[cursor]) {
            ChoiceInfoAppendChi(pgdata, pci, phoneSeqAlt[cursor]);
        }
    }
    /* phrase */
    else {
        if (pai->avail[pai->currentAvail].id) {
            GetPhraseFirst(pgdata, &tempPhrase, pai->avail[pai->currentAvail].id);
            do {
                if (ChoiceTheSame(pci, tempPhrase.phrase, len * ueBytesFromChar(tempPhrase.phrase[0]))) {
                    continue;
                }
                ueStrNCpy(pci->totalChoiceStr[pci->nTotalChoice], tempPhrase.phrase, len, 1);
                pci->nTotalChoice++;
		printf("---- %s, %d -----\n", __func__, __LINE__);
            } while (GetVocabNext(pgdata, &tempPhrase));
        }

        memcpy(userPhoneSeq, &phoneSeq[cursor], sizeof(uint32_t) * len);
        userPhoneSeq[len] = 0;

	/* Get User Phrase here */
        pUserPhraseData = UserGetPhraseFirst(pgdata, userPhoneSeq);
        if (pUserPhraseData) {
            do {
                /* check if the phrase is already in the choice list */
                if (ChoiceTheSame(pci, pUserPhraseData->wordSeq, len * ueBytesFromChar(pUserPhraseData->wordSeq[0])))
                    continue;
                /* otherwise store it */
                ueStrNCpy(pci->totalChoiceStr[pci->nTotalChoice], pUserPhraseData->wordSeq, len, 1);
		pci->totalChoiceType[pci->nTotalChoice] = TYPE_HAN;
		printf("\tCopying User: len=%d\t, (%s)\n", len, pUserPhraseData->wordSeq);
                pci->nTotalChoice++;
            } while ((pUserPhraseData = UserGetPhraseNext(pgdata, userPhoneSeq)) != NULL);
        }
        UserGetPhraseEnd(pgdata, userPhoneSeq);

	/* Get Tailo Phrase here */
        pUserPhraseData = TailoGetPhraseFirst(pgdata, userPhoneSeq);
        if (pUserPhraseData) {
            do {
                /* check if the phrase is already in the choice list */
                //if (ChoiceTheSame(pci, pUserPhraseData->wordSeq, strlen(pUserPhraseData->wordSeq[0])))
                 //   continue;
                /* otherwise store it */
                strncpy(pci->totalChoiceStr[pci->nTotalChoice], pUserPhraseData->wordSeq, MAX_PHRASE_LEN * MAX_UTF8_SIZE + 1);
		pci->totalChoiceType[pci->nTotalChoice] = TYPE_TAILO;
		printf("\tCopying Tailo: len=%d\t, (%s)\n", len, pUserPhraseData->wordSeq);
                pci->nTotalChoice++;
            } while ((pUserPhraseData = TailoGetPhraseNext(pgdata, userPhoneSeq)) != NULL);
        }
        TailoGetPhraseEnd(pgdata, userPhoneSeq);
    }

    /* magic number */
    pci->nChoicePerPage = candPerPage;
    assert(pci->nTotalChoice > 0);
    pci->nPage = CEIL_DIV(pci->nTotalChoice, pci->nChoicePerPage);
    pci->pageNo = 0;
    pci->isSymbol = WORD_CHOICE;
}

/*
 * Seek the start of the phrase (English characters are skipped.)
 */
static int SeekPhraseHead(ChewingData *pgdata)
{
    int i;
    int phoneSeq = PhoneSeqCursor(pgdata);

    for (i = pgdata->nPrefer - 1; i >= 0; i--) {
        if (pgdata->preferInterval[i].from > phoneSeq || pgdata->preferInterval[i].to < phoneSeq)
            continue;
        return pgdata->preferInterval[i].from;
    }
    return 0;
}

/** @brief Enter choice mode and relating initialisations. */
int ChoiceInitAvail(ChewingData *pgdata)
{
    int end, begin;

    /* save old cursor position */
    pgdata->choiceInfo.oldChiSymbolCursor = pgdata->chiSymbolCursor;

    /* see if there is some word in the cursor position */
    if (pgdata->chiSymbolBufLen == pgdata->chiSymbolCursor) {
        pgdata->chiSymbolCursor--;
    }

    end = PhoneSeqCursor(pgdata);

    if (pgdata->config.bPhraseChoiceRearward) {
        pgdata->chiSymbolCursor = SeekPhraseHead(pgdata) + CountSymbols(pgdata, pgdata->chiSymbolCursor);
	TRACZ("<<<< %s, %d, chiSymbolCursor=%d, SeekPhraseHead(pgdata)=%d, end=%d >>>>\n", __func__, __LINE__,
			SeekPhraseHead(pgdata), CountSymbols(pgdata, pgdata->chiSymbolCursor));
    }
    begin = PhoneSeqCursor(pgdata);

    pgdata->bSelect = 1;

    TRACZ("<<<< %s, %d, chiSymbolCursor=%d, begin=%d, end=%d >>>>\n", __func__, __LINE__,
		    pgdata->chiSymbolCursor, begin, end);
    SetAvailInfo(pgdata, begin, end);

    if (!pgdata->availInfo.nAvail)
        return ChoiceEndChoice(pgdata);

    pgdata->availInfo.currentAvail = pgdata->availInfo.nAvail - 1;
    SetChoiceInfo(pgdata);
    printf("XXX %s, %d XXX\n", __func__, __LINE__);
    return 0;
}

static void ChangeCurrentAvailInfo(ChewingData *pgdata, int current)
{
    assert(pgdata);

    if (pgdata->config.bPhraseChoiceRearward) {
        pgdata->chiSymbolCursor = pgdata->choiceInfo.oldChiSymbolCursor - pgdata->availInfo.avail[current].len + 1;

        /*
         * When oldChiSymbolCursor == chiSymbolBufLen, the old cursor is at the
         * end of buffer. This mean the end of phrase is in (oldChiSymbolCursor
         * - 1) instead of oldChiSymbolCursor. In this case, we need to adjust
         * chiSymbolCursor.
         */
        assert(pgdata->choiceInfo.oldChiSymbolCursor <= pgdata->chiSymbolBufLen);
        if (pgdata->choiceInfo.oldChiSymbolCursor == pgdata->chiSymbolBufLen) {
            --pgdata->chiSymbolCursor;
        }
    }

    pgdata->availInfo.currentAvail = current;
}

int ChoiceFirstAvail(ChewingData *pgdata)
{
    assert(pgdata);
    assert(pgdata->bSelect);

    if (pgdata->choiceInfo.isSymbol != WORD_CHOICE)
        return 0;

    ChangeCurrentAvailInfo(pgdata, pgdata->availInfo.nAvail - 1);

    SetChoiceInfo(pgdata);

    return 0;
}

int ChoiceLastAvail(ChewingData *pgdata)
{
    assert(pgdata);
    assert(pgdata->bSelect);

    if (pgdata->choiceInfo.isSymbol != WORD_CHOICE)
        return 0;

    ChangeCurrentAvailInfo(pgdata, 0);

    SetChoiceInfo(pgdata);

    return 0;
}

int ChoiceHasNextAvail(ChewingData *pgdata)
{
    assert(pgdata);
    assert(pgdata->bSelect);

    if (pgdata->choiceInfo.isSymbol != WORD_CHOICE)
        return 0;

    return pgdata->availInfo.currentAvail > 0;
}


int ChoiceHasPrevAvail(ChewingData *pgdata)
{
    assert(pgdata);
    assert(pgdata->bSelect);

    if (pgdata->choiceInfo.isSymbol != WORD_CHOICE)
        return 0;

    return pgdata->availInfo.currentAvail < pgdata->availInfo.nAvail - 1;
}

int ChoicePrevAvail(ChewingData *pgdata)
{
    assert(pgdata);
    assert(pgdata->bSelect);

    if (!ChoiceHasPrevAvail(pgdata)) {
        return -1;
    }

    ChangeCurrentAvailInfo(pgdata, pgdata->availInfo.currentAvail + 1);

    SetChoiceInfo(pgdata);

    return 0;
}

int ChoiceNextAvail(ChewingData *pgdata)
{
    assert(pgdata);
    assert(pgdata->bSelect);

    if (!ChoiceHasNextAvail(pgdata)) {
        return -1;
    }

    ChangeCurrentAvailInfo(pgdata, pgdata->availInfo.currentAvail - 1);

    SetChoiceInfo(pgdata);

    return 0;
}

int ChoiceEndChoice(ChewingData *pgdata)
{
    pgdata->bSelect = 0;
    pgdata->choiceInfo.nTotalChoice = 0;
    pgdata->choiceInfo.nPage = 0;

    if (pgdata->choiceInfo.isSymbol != WORD_CHOICE || pgdata->choiceInfo.isSymbol != SYMBOL_CHOICE_INSERT) {
        /* return to the old chiSymbolCursor position */
        pgdata->chiSymbolCursor = pgdata->choiceInfo.oldChiSymbolCursor;
        assert(pgdata->chiSymbolCursor <= pgdata->chiSymbolBufLen);
    }
    pgdata->choiceInfo.isSymbol = WORD_CHOICE;
    return 0;
}

static void ChangeUserData(ChewingData *pgdata, int selectNo, int type)
{
    uint32_t userPhoneSeq[MAX_PHONE_SEQ_LEN];
    int len;
    char *p = NULL;

    /* This function is used to determine how many word there, len is Number of word */
    if (pgdata->choiceInfo.totalChoiceType[selectNo] == TYPE_TAILO) {
	    p = pgdata->choiceInfo.totalChoiceStr[selectNo];
	    len = 1;
	    while (p = strchr(p, '-')) {
		    ++p;
		    ++len;
	    }
    }  else
	    len = ueStrLen(pgdata->choiceInfo.totalChoiceStr[selectNo]);


    TRACX("<<<<<---- %s, %d, selectNo=%d, type=%d, str=%s, len=%d ----->>>>\n", __func__, __LINE__, selectNo, type, pgdata->choiceInfo.totalChoiceStr[selectNo], len);
    memcpy(userPhoneSeq, &(pgdata->phoneSeq[PhoneSeqCursor(pgdata)]), len * sizeof(uint32_t));
    userPhoneSeq[len] = 0;
    UserUpdatePhrase(pgdata, userPhoneSeq, pgdata->choiceInfo.totalChoiceStr[selectNo], type);
}

/** @brief commit the selected phrase. */
int ChoiceSelect(ChewingData *pgdata, int selectNo)
{
    ChoiceInfo *pci = &(pgdata->choiceInfo);
    AvailInfo *pai = &(pgdata->availInfo);

    TRACX("---- %s, %d -----\n", __func__, __LINE__);
    ChangeUserData(pgdata, selectNo, pci->totalChoiceType[selectNo]);
    ChangeSelectIntervalAndBreakpoint(pgdata,
                                      PhoneSeqCursor(pgdata),
                                      PhoneSeqCursor(pgdata) + pai->avail[pai->currentAvail].len,
                                      pci->totalChoiceStr[selectNo],
				      pci->totalChoiceType[selectNo]);
    ChoiceEndChoice(pgdata);
    return 0;
}
