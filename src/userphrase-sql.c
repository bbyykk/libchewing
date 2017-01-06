/**
 * userphrase-sql.c
 *
 * Copyright (c) 2014
 *      libchewing Core Team. See ChangeLog for details.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file.
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "private.h"
#include "taigi-utf8-util.h"
#include "dict-private.h"
#include "tree-private.h"
#include "userphrase-private.h"
#include "private.h"
#include "key2pho-private.h"

#ifndef LOG_USERPHRASE_SQL
#undef LOG_WARN
#undef LOG_ERROR
#undef LOG_INFO
#undef LOG_VERBOSE
#define LOG_WARN(fmt...)
#define LOG_ERROR(fmt...)
#define LOG_INFO(fmt...)
#define LOG_VERBOSE(fmt...)
#endif


static int TailoBindPhone(ChewingData *pgdata, int index, const uint32_t phoneSeq[], int len)
{
    int i;
    int ret;

    assert(pgdata);
    assert(phoneSeq);

    LOG_VERBOSE("%s, %d, len=%d\n", __func__, __LINE__, len);
    if (len > MAX_PHRASE_LEN) {
        LOG_WARN("phoneSeq length %d > MAX_PHRASE_LEN(%d)", len, MAX_PHRASE_LEN);
        return -1;
    }

    ret = sqlite3_bind_int(pgdata->static_data.stmt_tailophrase[index], BIND_TAILOPHRASE_LENGTH, len);
    if (ret != SQLITE_OK) {
        LOG_ERROR("sqlite3_bind_int returns %d", ret);
        return ret;
    }

    for (i = 0; i < len; ++i) {
        ret = sqlite3_bind_int(pgdata->static_data.stmt_tailophrase[index], BIND_TAILOPHRASE_PHONE_0 + i, phoneSeq[i]);
        if (ret != SQLITE_OK) {
            LOG_ERROR("sqlite3_bind_int returns %d, 2nd par=%d, phoneSeq[%d]=%d\n", ret, BIND_TAILOPHRASE_PHONE_0 + i, i, phoneSeq[i]);
            return ret;
        }
    }

    for (i = len; i < MAX_PHRASE_LEN; ++i) {
        ret = sqlite3_bind_int(pgdata->static_data.stmt_tailophrase[index], BIND_TAILOPHRASE_PHONE_0 + i, 0);
        if (ret != SQLITE_OK) {
            LOG_ERROR("sqlite3_bind_int returns %d, 2nd par=%d, phoneSeq[%d]=%d\n", ret, BIND_TAILOPHRASE_PHONE_0 + i, i, phoneSeq[i]);
            return ret;
        }
    }

    return SQLITE_OK;
}


static int UserBindPhone(ChewingData *pgdata, int index, const uint32_t phoneSeq[], int len)
{
    int i;
    int ret;

    assert(pgdata);
    assert(phoneSeq);

    LOG_VERBOSE("len=%d\n", len);
    if (len > MAX_PHRASE_LEN) {
        LOG_WARN("phoneSeq length %d > MAX_PHRASE_LEN(%d)", len, MAX_PHRASE_LEN);
        return -1;
    }

    ret = sqlite3_bind_int(pgdata->static_data.stmt_userphrase[index], BIND_USERPHRASE_LENGTH, len);
    if (ret != SQLITE_OK) {
        LOG_ERROR("sqlite3_bind_int returns %d", ret);
        return ret;
    }

    for (i = 0; i < len; ++i) {
        ret = sqlite3_bind_int(pgdata->static_data.stmt_userphrase[index], BIND_USERPHRASE_PHONE_0 + i, phoneSeq[i]);
        if (ret != SQLITE_OK) {
            LOG_ERROR("sqlite3_bind_int returns %d, 2nd par=%d, phoneSeq[%d]=%d\n", ret, BIND_USERPHRASE_PHONE_0 + i, i, phoneSeq[i]);
            return ret;
        }
    }

    for (i = len; i < MAX_PHRASE_LEN; ++i) {
        ret = sqlite3_bind_int(pgdata->static_data.stmt_userphrase[index], BIND_USERPHRASE_PHONE_0 + i, 0);
        if (ret != SQLITE_OK) {
            LOG_ERROR("sqlite3_bind_int returns %d, 2nd par=%d, phoneSeq[%d]=%d\n", ret, BIND_USERPHRASE_PHONE_0 + i, i, phoneSeq[i]);
            return ret;
        }
    }

    return SQLITE_OK;
}


/* load the original frequency from the static dict */
static int LoadOriginalFreq(ChewingData *pgdata, const uint32_t phoneSeq[], const char wordSeq[], int len)
{
    const TreeType *tree_pos;
    int retval;
    Phrase *phrase = ALC(Phrase, 1);


    LOG_VERBOSE("%s, %d, len=%d\n", __func__, __LINE__, len);
    tree_pos = TreeFindPhrase(pgdata, 0, len - 1, phoneSeq);
    if (tree_pos) {
        GetPhraseFirst(pgdata, phrase, tree_pos);
        do {
            /* find the same phrase */
            if (!strcmp(phrase->phrase, wordSeq)) {
                retval = phrase->freq;
                free(phrase);
                return retval;
            }
        } while (GetVocabNext(pgdata, phrase));
    }

    free(phrase);
    return FREQ_INIT_VALUE;
}

/* find the maximum frequency of the same phrase */
static int LoadMaxFreq(ChewingData *pgdata, const uint32_t phoneSeq[], int len)
{
    const TreeType *tree_pos;
    Phrase *phrase = ALC(Phrase, 1);
    int maxFreq = FREQ_INIT_VALUE;
    int max_userphrase_freq;
    int ret;

    LOG_VERBOSE("%s, %d, len=%d\n", __func__, __LINE__, len);
    tree_pos = TreeFindPhrase(pgdata, 0, len - 1, phoneSeq);
    if (tree_pos) {
        GetPhraseFirst(pgdata, phrase, tree_pos);
        do {
            if (phrase->freq > maxFreq)
                maxFreq = phrase->freq;
        } while (GetVocabNext(pgdata, phrase));
    }
    free(phrase);

    assert(pgdata->static_data.stmt_userphrase[STMT_USERPHRASE_GET_MAX_FREQ]);

    ret = UserBindPhone(pgdata, STMT_USERPHRASE_GET_MAX_FREQ, phoneSeq, len);
    if (ret != SQLITE_OK) {
        LOG_ERROR("UserBindPhone returns %d", ret);
        return maxFreq;
    }

    ret = sqlite3_step(pgdata->static_data.stmt_userphrase[STMT_USERPHRASE_GET_MAX_FREQ]);
    if (ret != SQLITE_ROW)
        return maxFreq;

    ret = sqlite3_reset(pgdata->static_data.stmt_userphrase[STMT_USERPHRASE_GET_MAX_FREQ]);
    if (ret != SQLITE_OK) {
        LOG_ERROR("sqlite3_reset returns %d", ret);
        return maxFreq;
    }

    max_userphrase_freq = sqlite3_column_int(pgdata->static_data.stmt_userphrase[STMT_USERPHRASE_GET_MAX_FREQ],
                                             SQL_STMT_USERPHRASE[STMT_USERPHRASE_GET_MAX_FREQ].column
                                             [COLUMN_USERPHRASE_USER_FREQ]);

    if (max_userphrase_freq > maxFreq)
        maxFreq = max_userphrase_freq;

    return maxFreq;
}

/* compute the new updated freqency */
static int UpdateFreq(int freq, int maxfreq, int origfreq, int deltatime)
{
    int delta;

    /* Short interval */
    if (deltatime < 4000) {
        delta = (freq >= maxfreq) ?
            min((maxfreq - origfreq) / 5 + 1,
                SHORT_INCREASE_FREQ) : max((maxfreq - origfreq) / 5 + 1, SHORT_INCREASE_FREQ);
        return min(freq + delta, MAX_ALLOW_FREQ);
    }
    /* Medium interval */
    else if (deltatime < 50000) {
        delta = (freq >= maxfreq) ?
            min((maxfreq - origfreq) / 10 + 1,
                MEDIUM_INCREASE_FREQ) : max((maxfreq - origfreq) / 10 + 1, MEDIUM_INCREASE_FREQ);
        return min(freq + delta, MAX_ALLOW_FREQ);
    }
    /* long interval */
    else {
        delta = max((freq - origfreq) / 5, LONG_DECREASE_FREQ);
        return max(freq - delta, origfreq);
    }
}

static int GetCurrentLifeTime(ChewingData *pgdata)
{
    return pgdata->static_data.new_lifetime;
}

static void LogUserPhrase(ChewingData *pgdata,
                          const uint32_t phoneSeq[],
                          const char wordSeq[], int orig_freq, int max_freq, int user_freq, int recent_time)
{
    /* Size of each phone is len("0x1234 ") = 7 */
    char buf[7 * MAX_PHRASE_LEN + 1] = { 0 };
    int i;

    for (i = 0; i < MAX_PHRASE_LEN; ++i) {
        if (phoneSeq[i] == 0)
            break;
        snprintf(buf + 7 * i, 7 + 1, "%#06x ", phoneSeq[i]);
    }

    LOG_INFO("userphrase %s, phone = %s, orig_freq = %d, max_freq = %d, user_freq = %d, recent_time = %d",
             wordSeq, buf, orig_freq, max_freq, user_freq, recent_time);
}

void UserUpdatePhraseBegin(ChewingData *pgdata)
{
    sqlite3_exec(pgdata->static_data.db, "BEGIN", 0, 0, 0);
}

/*
 * Note: Currently do NOT support the multiple Tailo word
 *
 */
static int UserUpdatePhrase_Tailo(ChewingData *pgdata, const uint32_t phoneSeq[], const char wordSeq[])
{
    int ret;
    int action;
    int phone_len;
    int word_len;

    int orig_freq;
    int max_freq;
    int user_freq;
    int recent_time;
    int orig_time;


    phone_len = GetPhoneLen(phoneSeq);
    
    /* TODO: Currnetly do not support multiple word */
    word_len = phone_len;
    printf("%s, Currently Not support multiple Tailo words\n", __func__);
    printf("<<<<< %s, %d, input word=%s, phone_len=%d, word_len=%d >>>> \n",
	    __func__, __LINE__, wordSeq, phone_len, word_len);

    if (word_len > MAX_PHRASE_LEN) {
        LOG_WARN("wordSeq length %d > MAX_PHRASE_LEN (%d)", word_len, MAX_PHRASE_LEN);
        return USER_UPDATE_FAIL;
    }

    ret = TailoBindPhone(pgdata, STMT_TAILOPHRASE_SELECT_BY_PHONE_PHRASE, phoneSeq, phone_len);
    if (ret != SQLITE_OK) {
        LOG_ERROR("TailoBindPhone returns %d", ret);
        action = USER_UPDATE_FAIL;
        goto end;
    }

    ret = sqlite3_bind_text(pgdata->static_data.stmt_tailophrase[STMT_TAILOPHRASE_SELECT_BY_PHONE_PHRASE],
                            BIND_TAILOPHRASE_PHRASE, wordSeq, -1, SQLITE_STATIC);
    if (ret != SQLITE_OK) {
        LOG_ERROR("sqlite3_bind_text returns %d", ret);
        action = USER_UPDATE_FAIL;
        goto end;
    }

    recent_time = GetCurrentLifeTime(pgdata);

    ret = sqlite3_step(pgdata->static_data.stmt_tailophrase[STMT_TAILOPHRASE_SELECT_BY_PHONE_PHRASE]);
    if (ret == SQLITE_ROW) {
        action = USER_UPDATE_MODIFY;

        orig_freq = sqlite3_column_int(pgdata->static_data.stmt_tailophrase[STMT_TAILOPHRASE_SELECT_BY_PHONE_PHRASE],
                                       SQL_STMT_TAILOPHRASE[STMT_TAILOPHRASE_SELECT_BY_PHONE_PHRASE].column
                                       [COLUMN_TAILOPHRASE_ORIG_FREQ]);

        max_freq = LoadMaxFreq(pgdata, phoneSeq, phone_len);

        user_freq = sqlite3_column_int(pgdata->static_data.stmt_tailophrase[STMT_TAILOPHRASE_SELECT_BY_PHONE_PHRASE],
                                       SQL_STMT_TAILOPHRASE[STMT_TAILOPHRASE_SELECT_BY_PHONE_PHRASE].column
                                       [COLUMN_TAILOPHRASE_USER_FREQ]);

        orig_time = sqlite3_column_int(pgdata->static_data.stmt_tailophrase[STMT_TAILOPHRASE_SELECT_BY_PHONE_PHRASE],
                                       SQL_STMT_TAILOPHRASE[STMT_TAILOPHRASE_SELECT_BY_PHONE_PHRASE].column
                                       [COLUMN_TAILOPHRASE_TIME]);

        user_freq = UpdateFreq(user_freq, max_freq, orig_freq, recent_time - orig_time);
    } else {
        action = USER_UPDATE_INSERT;

        orig_freq = LoadOriginalFreq(pgdata, phoneSeq, wordSeq, word_len);
        max_freq = LoadMaxFreq(pgdata, phoneSeq, phone_len);
        user_freq = orig_freq;
    }

    assert(pgdata->static_data.stmt_tailophrase[STMT_TAILOPHRASE_UPSERT]);

    ret = sqlite3_bind_int(pgdata->static_data.stmt_tailophrase[STMT_TAILOPHRASE_UPSERT],
                           BIND_TAILOPHRASE_TIME, recent_time);
    if (ret != SQLITE_OK) {
        LOG_ERROR("sqlite3_bind_int returns %d", ret);
        action = USER_UPDATE_FAIL;
        goto end;
    }

    ret = sqlite3_bind_int(pgdata->static_data.stmt_tailophrase[STMT_TAILOPHRASE_UPSERT],
                           BIND_TAILOPHRASE_USER_FREQ, user_freq);
    if (ret != SQLITE_OK) {
        LOG_ERROR("sqlite3_bind_int returns %d", ret);
        action = USER_UPDATE_FAIL;
        goto end;
    }

    ret = sqlite3_bind_int(pgdata->static_data.stmt_tailophrase[STMT_TAILOPHRASE_UPSERT],
                           BIND_TAILOPHRASE_MAX_FREQ, max_freq);
    if (ret != SQLITE_OK) {
        LOG_ERROR("sqlite3_bind_int returns %d", ret);
        action = USER_UPDATE_FAIL;
        goto end;
    }

    ret = sqlite3_bind_int(pgdata->static_data.stmt_tailophrase[STMT_TAILOPHRASE_UPSERT],
                           BIND_TAILOPHRASE_ORIG_FREQ, orig_freq);
    if (ret != SQLITE_OK) {
        LOG_ERROR("sqlite3_bind_int returns %d", ret);
        action = USER_UPDATE_FAIL;
        goto end;
    }

    ret = TailoBindPhone(pgdata, STMT_TAILOPHRASE_UPSERT, phoneSeq, phone_len);
    if (ret != SQLITE_OK) {
        LOG_ERROR("TailoBindPhone returns %d", ret);
        action = USER_UPDATE_FAIL;
        goto end;
    }

    ret = sqlite3_bind_text(pgdata->static_data.stmt_tailophrase[STMT_TAILOPHRASE_UPSERT],
                            BIND_TAILOPHRASE_PHRASE, wordSeq, -1, SQLITE_STATIC);
    if (ret != SQLITE_OK) {
        LOG_ERROR("sqlite3_bind_text returns %d", ret);
        action = USER_UPDATE_FAIL;
        goto end;
    }

    ret = sqlite3_bind_int(pgdata->static_data.stmt_tailophrase[STMT_TAILOPHRASE_UPSERT],
                           BIND_TAILOPHRASE_TYPE, TYPE_TAILO);
    if (ret != SQLITE_OK) {
        LOG_ERROR("sqlite3_bind_int returns %d", ret);
        action = USER_UPDATE_FAIL;
        goto end;
    }

    ret = sqlite3_step(pgdata->static_data.stmt_tailophrase[STMT_TAILOPHRASE_UPSERT]);
    if (ret != SQLITE_DONE) {
        LOG_ERROR("sqlite3_step returns %d", ret);
        action = USER_UPDATE_FAIL;
        goto end;
    }

    LogUserPhrase(pgdata, phoneSeq, wordSeq, orig_freq, max_freq, user_freq, recent_time);

  end:
    ret = sqlite3_reset(pgdata->static_data.stmt_tailophrase[STMT_TAILOPHRASE_UPSERT]);
    if (ret != SQLITE_OK) {
        LOG_ERROR("sqlite3_reset returns %d", ret);
    }

    ret = sqlite3_reset(pgdata->static_data.stmt_tailophrase[STMT_TAILOPHRASE_SELECT_BY_PHONE_PHRASE]);
    if (ret != SQLITE_OK) {
        LOG_ERROR("sqlite3_reset returns %d", ret);
    }

    LOG_ERROR("%s: %d, action=%d\n", __func__, __LINE__, action);
    return action;
}

static int UserUpdatePhrase_Han(ChewingData *pgdata, const uint32_t phoneSeq[], const char wordSeq[])
{
    int ret;
    int action;
    int phone_len;
    int word_len;

    int orig_freq;
    int max_freq;
    int user_freq;
    int recent_time;
    int orig_time;

    phone_len = GetPhoneLen(phoneSeq);
    word_len = ueStrLen(wordSeq);

    if (phone_len != word_len) {
        LOG_WARN("Do not update userphrase because phoneSeq length %d != wordSeq length %d", phone_len, word_len);
        return USER_UPDATE_FAIL;
    }

    if (word_len > MAX_PHRASE_LEN) {
        LOG_WARN("wordSeq length %d > MAX_PHRASE_LEN (%d)", word_len, MAX_PHRASE_LEN);
        return USER_UPDATE_FAIL;
    }

    ret = UserBindPhone(pgdata, STMT_USERPHRASE_SELECT_BY_PHONE_PHRASE, phoneSeq, phone_len);
    if (ret != SQLITE_OK) {
        LOG_ERROR("UserBindPhone returns %d", ret);
        action = USER_UPDATE_FAIL;
        goto end;
    }

    ret = sqlite3_bind_text(pgdata->static_data.stmt_userphrase[STMT_USERPHRASE_SELECT_BY_PHONE_PHRASE],
                            BIND_USERPHRASE_PHRASE, wordSeq, -1, SQLITE_STATIC);
    if (ret != SQLITE_OK) {
        LOG_ERROR("sqlite3_bind_text returns %d", ret);
        action = USER_UPDATE_FAIL;
        goto end;
    }

    recent_time = GetCurrentLifeTime(pgdata);

    ret = sqlite3_step(pgdata->static_data.stmt_userphrase[STMT_USERPHRASE_SELECT_BY_PHONE_PHRASE]);
    if (ret == SQLITE_ROW) {
        action = USER_UPDATE_MODIFY;

        orig_freq = sqlite3_column_int(pgdata->static_data.stmt_userphrase[STMT_USERPHRASE_SELECT_BY_PHONE_PHRASE],
                                       SQL_STMT_USERPHRASE[STMT_USERPHRASE_SELECT_BY_PHONE_PHRASE].column
                                       [COLUMN_USERPHRASE_ORIG_FREQ]);

        max_freq = LoadMaxFreq(pgdata, phoneSeq, phone_len);

        user_freq = sqlite3_column_int(pgdata->static_data.stmt_userphrase[STMT_USERPHRASE_SELECT_BY_PHONE_PHRASE],
                                       SQL_STMT_USERPHRASE[STMT_USERPHRASE_SELECT_BY_PHONE_PHRASE].column
                                       [COLUMN_USERPHRASE_USER_FREQ]);

        orig_time = sqlite3_column_int(pgdata->static_data.stmt_userphrase[STMT_USERPHRASE_SELECT_BY_PHONE_PHRASE],
                                       SQL_STMT_USERPHRASE[STMT_USERPHRASE_SELECT_BY_PHONE_PHRASE].column
                                       [COLUMN_USERPHRASE_TIME]);

        user_freq = UpdateFreq(user_freq, max_freq, orig_freq, recent_time - orig_time);
    } else {
        action = USER_UPDATE_INSERT;

        orig_freq = LoadOriginalFreq(pgdata, phoneSeq, wordSeq, word_len);
        max_freq = LoadMaxFreq(pgdata, phoneSeq, phone_len);
        user_freq = orig_freq;
    }

    assert(pgdata->static_data.stmt_userphrase[STMT_USERPHRASE_UPSERT]);

    ret = sqlite3_bind_int(pgdata->static_data.stmt_userphrase[STMT_USERPHRASE_UPSERT],
                           BIND_USERPHRASE_TIME, recent_time);
    if (ret != SQLITE_OK) {
        LOG_ERROR("sqlite3_bind_int returns %d", ret);
        action = USER_UPDATE_FAIL;
        goto end;
    }

    ret = sqlite3_bind_int(pgdata->static_data.stmt_userphrase[STMT_USERPHRASE_UPSERT],
                           BIND_USERPHRASE_USER_FREQ, user_freq);
    if (ret != SQLITE_OK) {
        LOG_ERROR("sqlite3_bind_int returns %d", ret);
        action = USER_UPDATE_FAIL;
        goto end;
    }

    ret = sqlite3_bind_int(pgdata->static_data.stmt_userphrase[STMT_USERPHRASE_UPSERT],
                           BIND_USERPHRASE_MAX_FREQ, max_freq);
    if (ret != SQLITE_OK) {
        LOG_ERROR("sqlite3_bind_int returns %d", ret);
        action = USER_UPDATE_FAIL;
        goto end;
    }

    ret = sqlite3_bind_int(pgdata->static_data.stmt_userphrase[STMT_USERPHRASE_UPSERT],
                           BIND_USERPHRASE_ORIG_FREQ, orig_freq);
    if (ret != SQLITE_OK) {
        LOG_ERROR("sqlite3_bind_int returns %d", ret);
        action = USER_UPDATE_FAIL;
        goto end;
    }

    ret = UserBindPhone(pgdata, STMT_USERPHRASE_UPSERT, phoneSeq, phone_len);
    if (ret != SQLITE_OK) {
        LOG_ERROR("UserBindPhone returns %d", ret);
        action = USER_UPDATE_FAIL;
        goto end;
    }

    ret = sqlite3_bind_text(pgdata->static_data.stmt_userphrase[STMT_USERPHRASE_UPSERT],
                            BIND_USERPHRASE_PHRASE, wordSeq, -1, SQLITE_STATIC);
    if (ret != SQLITE_OK) {
        LOG_ERROR("sqlite3_bind_text returns %d", ret);
        action = USER_UPDATE_FAIL;
        goto end;
    }

    ret = sqlite3_bind_int(pgdata->static_data.stmt_userphrase[STMT_USERPHRASE_UPSERT],
                           BIND_USERPHRASE_TYPE, TYPE_HAN);
    if (ret != SQLITE_OK) {
        LOG_ERROR("sqlite3_bind_int returns %d", ret);
        action = USER_UPDATE_FAIL;
        goto end;
    }

    ret = sqlite3_step(pgdata->static_data.stmt_userphrase[STMT_USERPHRASE_UPSERT]);
    if (ret != SQLITE_DONE) {
        LOG_ERROR("sqlite3_step returns %d", ret);
        action = USER_UPDATE_FAIL;
        goto end;
    }

    LogUserPhrase(pgdata, phoneSeq, wordSeq, orig_freq, max_freq, user_freq, recent_time);

  end:
    ret = sqlite3_reset(pgdata->static_data.stmt_userphrase[STMT_USERPHRASE_UPSERT]);
    if (ret != SQLITE_OK) {
        LOG_ERROR("sqlite3_reset returns %d", ret);
    }

    ret = sqlite3_reset(pgdata->static_data.stmt_userphrase[STMT_USERPHRASE_SELECT_BY_PHONE_PHRASE]);
    if (ret != SQLITE_OK) {
        LOG_ERROR("sqlite3_reset returns %d", ret);
    }

    return action;
}

int UserUpdatePhrase(ChewingData *pgdata, const uint32_t phoneSeq[], const char wordSeq[], int type)
{
    int phone_len;
    int word_len;

    assert(pgdata);
    assert(phoneSeq);
    assert(wordSeq);

    if (type == TYPE_TAILO)
	    return UserUpdatePhrase_Tailo(pgdata, phoneSeq, wordSeq);

    return UserUpdatePhrase_Han(pgdata, phoneSeq, wordSeq);
}

void UserUpdatePhraseEnd(ChewingData *pgdata)
{
    sqlite3_exec(pgdata->static_data.db, "END", 0, 0, 0);
}

int UserRemovePhrase(ChewingData *pgdata, const uint32_t phoneSeq[], const char wordSeq[])
{
    int ret;
    int len;
    int affected = 0;

    assert(pgdata);
    assert(phoneSeq);
    assert(wordSeq);

    assert(pgdata->static_data.stmt_userphrase[STMT_USERPHRASE_DELETE]);

    len = GetPhoneLen(phoneSeq);
    ret = UserBindPhone(pgdata, STMT_USERPHRASE_DELETE, phoneSeq, len);
    if (ret != SQLITE_OK) {
        LOG_ERROR("UserBindPhone returns %d", ret);
        goto end;
    }

    ret = sqlite3_bind_text(pgdata->static_data.stmt_userphrase[STMT_USERPHRASE_DELETE],
                            BIND_USERPHRASE_PHRASE, wordSeq, -1, SQLITE_STATIC);
    if (ret != SQLITE_OK) {
        LOG_ERROR("sqlite3_bind_text returns %d", ret);
        goto end;
    }

    ret = sqlite3_step(pgdata->static_data.stmt_userphrase[STMT_USERPHRASE_DELETE]);
    if (ret != SQLITE_DONE) {
        LOG_ERROR("sqlite3_step returns %d", ret);
        goto end;
    }

    affected = sqlite3_changes(pgdata->static_data.db);

  end:
    ret = sqlite3_reset(pgdata->static_data.stmt_userphrase[STMT_USERPHRASE_DELETE]);
    if (ret != SQLITE_OK) {
        LOG_ERROR("sqlite3_reset returns %d", ret);
    }

    return affected;
}

UserPhraseData *TailoGetPhraseFirst(ChewingData *pgdata, const uint32_t phoneSeq[])
{
    int ret;
    int len;

    assert(pgdata);
    assert(phoneSeq);

    assert(pgdata->static_data.stmt_tailophrase[STMT_TAILOPHRASE_SELECT_BY_PHONE]);
    ret = sqlite3_reset(pgdata->static_data.stmt_tailophrase[STMT_TAILOPHRASE_SELECT_BY_PHONE]);
    if (ret != SQLITE_OK) {
        LOG_ERROR("sqlite3_reset returns %d", ret);
        return NULL;
    }

    len = GetPhoneLen(phoneSeq);
    LOG_INFO("len=%d", len);
    ret = TailoBindPhone(pgdata, STMT_TAILOPHRASE_SELECT_BY_PHONE, phoneSeq, len);
    if (ret != SQLITE_OK) {
        LOG_ERROR("TailoBindPhone returns %d", ret);
        return NULL;
    }
    
    return TailoGetPhraseNext(pgdata, phoneSeq);
}

UserPhraseData *TailoGetPhraseNext(ChewingData *pgdata, const uint32_t phoneSeq[])
{
    int ret;

    assert(pgdata);
    assert(phoneSeq);

    ret = sqlite3_step(pgdata->static_data.stmt_tailophrase[STMT_TAILOPHRASE_SELECT_BY_PHONE]);
    if (ret != SQLITE_ROW)
        return NULL;

    /* FIXME: shall not remove const here. */
    pgdata->tailophrase_data.wordSeq =
        (char *) sqlite3_column_text(pgdata->static_data.stmt_tailophrase[STMT_TAILOPHRASE_SELECT_BY_PHONE],
                                     SQL_STMT_TAILOPHRASE[STMT_TAILOPHRASE_SELECT_BY_PHONE].column
                                     [COLUMN_TAILOPHRASE_PHRASE]);
    pgdata->tailophrase_data.phoneSeq = (uint32_t *) phoneSeq;

    pgdata->tailophrase_data.recentTime =
        sqlite3_column_int(pgdata->static_data.stmt_tailophrase[STMT_TAILOPHRASE_SELECT_BY_PHONE],
                           SQL_STMT_TAILOPHRASE[STMT_TAILOPHRASE_SELECT_BY_PHONE].column[COLUMN_TAILOPHRASE_TIME]);

    pgdata->tailophrase_data.userfreq =
        sqlite3_column_int(pgdata->static_data.stmt_tailophrase[STMT_TAILOPHRASE_SELECT_BY_PHONE],
                           SQL_STMT_TAILOPHRASE[STMT_TAILOPHRASE_SELECT_BY_PHONE].column[COLUMN_TAILOPHRASE_USER_FREQ]);

    pgdata->tailophrase_data.maxfreq =
        sqlite3_column_int(pgdata->static_data.stmt_tailophrase[STMT_TAILOPHRASE_SELECT_BY_PHONE],
                           SQL_STMT_TAILOPHRASE[STMT_TAILOPHRASE_SELECT_BY_PHONE].column[COLUMN_TAILOPHRASE_MAX_FREQ]);

    pgdata->tailophrase_data.origfreq =
        sqlite3_column_int(pgdata->static_data.stmt_tailophrase[STMT_TAILOPHRASE_SELECT_BY_PHONE],
                           SQL_STMT_TAILOPHRASE[STMT_TAILOPHRASE_SELECT_BY_PHONE].column[COLUMN_TAILOPHRASE_ORIG_FREQ]);

    pgdata->tailophrase_data.type =
        sqlite3_column_int(pgdata->static_data.stmt_tailophrase[STMT_TAILOPHRASE_SELECT_BY_PHONE],
                           SQL_STMT_TAILOPHRASE[STMT_TAILOPHRASE_SELECT_BY_PHONE].column[COLUMN_TAILOPHRASE_TYPE]);

    return &(pgdata->tailophrase_data);
}

void TailoGetPhraseEnd(ChewingData *pgdata UNUSED, const uint32_t phoneSeq[] UNUSED)
{
    /* FIXME: Remove this */
}


UserPhraseData *UserGetPhraseFirst(ChewingData *pgdata, const uint32_t phoneSeq[])
{
    int ret;
    int len;

    assert(pgdata);
    assert(phoneSeq);

    assert(pgdata->static_data.stmt_userphrase[STMT_USERPHRASE_SELECT_BY_PHONE]);
    ret = sqlite3_reset(pgdata->static_data.stmt_userphrase[STMT_USERPHRASE_SELECT_BY_PHONE]);
    if (ret != SQLITE_OK) {
        LOG_ERROR("sqlite3_reset returns %d", ret);
        return NULL;
    }

    len = GetPhoneLen(phoneSeq);
    LOG_INFO("len=%d", len);
    ret = UserBindPhone(pgdata, STMT_USERPHRASE_SELECT_BY_PHONE, phoneSeq, len);
    if (ret != SQLITE_OK) {
        LOG_ERROR("UserBindPhone returns %d", ret);
        return NULL;
    }

    return UserGetPhraseNext(pgdata, phoneSeq);
}

UserPhraseData *UserGetPhraseNext(ChewingData *pgdata, const uint32_t phoneSeq[])
{
    int ret;

    assert(pgdata);
    assert(phoneSeq);

    ret = sqlite3_step(pgdata->static_data.stmt_userphrase[STMT_USERPHRASE_SELECT_BY_PHONE]);
    if (ret != SQLITE_ROW)
        return NULL;

    /* FIXME: shall not remove const here. */
    pgdata->userphrase_data.wordSeq =
        (char *) sqlite3_column_text(pgdata->static_data.stmt_userphrase[STMT_USERPHRASE_SELECT_BY_PHONE],
                                     SQL_STMT_USERPHRASE[STMT_USERPHRASE_SELECT_BY_PHONE].column
                                     [COLUMN_USERPHRASE_PHRASE]);
    pgdata->userphrase_data.phoneSeq = (uint32_t *) phoneSeq;

    pgdata->userphrase_data.recentTime =
        sqlite3_column_int(pgdata->static_data.stmt_userphrase[STMT_USERPHRASE_SELECT_BY_PHONE],
                           SQL_STMT_USERPHRASE[STMT_USERPHRASE_SELECT_BY_PHONE].column[COLUMN_USERPHRASE_TIME]);

    pgdata->userphrase_data.userfreq =
        sqlite3_column_int(pgdata->static_data.stmt_userphrase[STMT_USERPHRASE_SELECT_BY_PHONE],
                           SQL_STMT_USERPHRASE[STMT_USERPHRASE_SELECT_BY_PHONE].column[COLUMN_USERPHRASE_USER_FREQ]);

    pgdata->userphrase_data.maxfreq =
        sqlite3_column_int(pgdata->static_data.stmt_userphrase[STMT_USERPHRASE_SELECT_BY_PHONE],
                           SQL_STMT_USERPHRASE[STMT_USERPHRASE_SELECT_BY_PHONE].column[COLUMN_USERPHRASE_MAX_FREQ]);

    pgdata->userphrase_data.origfreq =
        sqlite3_column_int(pgdata->static_data.stmt_userphrase[STMT_USERPHRASE_SELECT_BY_PHONE],
                           SQL_STMT_USERPHRASE[STMT_USERPHRASE_SELECT_BY_PHONE].column[COLUMN_USERPHRASE_ORIG_FREQ]);

    pgdata->userphrase_data.type =
        sqlite3_column_int(pgdata->static_data.stmt_userphrase[STMT_USERPHRASE_SELECT_BY_PHONE],
                           SQL_STMT_USERPHRASE[STMT_USERPHRASE_SELECT_BY_PHONE].column[COLUMN_USERPHRASE_TYPE]);
    TRACX("%s ZZZZZZZZ %d\n", __func__, pgdata->userphrase_data.type);
    return &pgdata->userphrase_data;
}

void UserGetPhraseEnd(ChewingData *pgdata UNUSED, const uint32_t phoneSeq[] UNUSED)
{
    /* FIXME: Remove this */
}

void IncreaseLifeTime(ChewingData *pgdata)
{
    ++pgdata->static_data.new_lifetime;
}
