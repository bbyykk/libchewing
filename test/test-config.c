/**
 * test-config.c
 *
 * Copyright (c) 2012
 *      libchewing Core Team. See ChangeLog for details.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file.
 */

#ifdef HAVE_CONFIG_H
#    include <config.h>
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "chewing.h"
#include "testhelper.h"

static const int MIN_CAND_PER_PAGE = 1;
static const int MAX_CAND_PER_PAGE = 10;
static const int DEFAULT_CAND_PER_PAGE = 10;
static const int MIN_CHI_SYMBOL_LEN = 0;
static const int MAX_CHI_SYMBOL_LEN = 39;

static const int DEFAULT_SELECT_KEY[] = {
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0'
};

static const int ALTERNATE_SELECT_KEY[] = {
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';'
};

static const TestData DATA = { "`a", "\xE2\x80\xA6" /* … */  };

FILE *fd;

void test_default_value()
{
    int *select_key;
    ChewingContext *ctx;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    select_key = taigi_get_selKey(ctx);
    ok(select_key, "taigi_get_selKey shall not return NULL");
    ok(!memcmp(select_key, DEFAULT_SELECT_KEY,
               sizeof(DEFAULT_SELECT_KEY)), "default select key shall be default value");
    taigi_free(select_key);

    ok(taigi_get_candPerPage(ctx) == DEFAULT_CAND_PER_PAGE, "default candPerPage shall be %d", DEFAULT_CAND_PER_PAGE);

    ok(taigi_get_maxChiSymbolLen(ctx) == MAX_CHI_SYMBOL_LEN,
       "default maxChiSymbolLen shall be %d", MAX_CHI_SYMBOL_LEN);

    ok(taigi_get_addPhraseDirection(ctx) == 0, "default addPhraseDirection shall be 0");

    ok(taigi_get_spaceAsSelection(ctx) == 0, "default spaceAsSelection shall be 0");

    ok(taigi_get_escCleanAllBuf(ctx) == 0, "default escCleanAllBuf shall be 0");

BEGIN_IGNORE_DEPRECATIONS
    ok(taigi_get_hsuSelKeyType(ctx) == 0, "default hsuSelKeyType shall be 0");
END_IGNORE_DEPRECATIONS

    ok(taigi_get_autoShiftCur(ctx) == 0, "default autoShiftCur shall be 0");

    ok(taigi_get_easySymbolInput(ctx) == 0, "default easySymbolInput shall be 0");

    ok(taigi_get_phraseChoiceRearward(ctx) == 0, "default phraseChoiceRearward shall be 0");

    ok(taigi_get_ChiEngMode(ctx) == CHINESE_MODE, "default ChiEngMode shall be CHINESE_MODE");

    ok(taigi_get_ShapeMode(ctx) == HALFSHAPE_MODE, "default ShapeMode shall be HALFSHAPE_MODE");

    taigi_delete(ctx);
}

void test_set_candPerPage()
{
    const int VALUE[] = {
        MIN_CAND_PER_PAGE,
        MAX_CAND_PER_PAGE,
    };

    const int INVALID_VALUE[] = {
        MIN_CAND_PER_PAGE - 1,
        MAX_CAND_PER_PAGE + 1,
    };

    ChewingContext *ctx;
    size_t i;
    size_t j;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    for (i = 0; i < ARRAY_SIZE(VALUE); ++i) {
        taigi_set_candPerPage(ctx, VALUE[i]);
        ok(taigi_get_candPerPage(ctx) == VALUE[i], "candPerPage shall be `%d'", VALUE[i]);

        for (j = 0; j < ARRAY_SIZE(INVALID_VALUE); ++j) {
            // mode shall not change when set mode has invalid value.
            taigi_set_candPerPage(ctx, INVALID_VALUE[j]);
            ok(taigi_get_candPerPage(ctx) == VALUE[i], "candPerPage shall be `%d'", VALUE[i]);
        }
    }

    taigi_delete(ctx);
}

void test_set_maxChiSymbolLen()
{
    ChewingContext *ctx;
    int i;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    taigi_set_maxChiSymbolLen(ctx, 16);
    ok(taigi_get_maxChiSymbolLen(ctx) == 16, "maxChiSymbolLen shall be 16");

    taigi_set_maxChiSymbolLen(ctx, MIN_CHI_SYMBOL_LEN - 1);
    ok(taigi_get_maxChiSymbolLen(ctx) == 16,
       "maxChiSymbolLen shall not change when set to %d", MIN_CHI_SYMBOL_LEN - 1);

    taigi_set_maxChiSymbolLen(ctx, MAX_CHI_SYMBOL_LEN + 1);
    ok(taigi_get_maxChiSymbolLen(ctx) == 16,
       "maxChiSymbolLen shall not change when set to %d", MAX_CHI_SYMBOL_LEN + 1);


    // Test auto commit
    taigi_set_maxChiSymbolLen(ctx, MAX_CHI_SYMBOL_LEN);

    // In boundary
    for (i = 0; i < MAX_CHI_SYMBOL_LEN; ++i)
        type_keystroke_by_string(ctx, "hk4");
    ok(taigi_commit_Check(ctx) == 0,
       "auto commit shall not be triggered when entering %d symbols", MAX_CHI_SYMBOL_LEN);

    // Out of boundary
    type_keystroke_by_string(ctx, "hk4");
    ok(taigi_commit_Check(ctx) == 1,
       "auto commit shall be triggered when entering %d symbols", MAX_CHI_SYMBOL_LEN + 1);

    taigi_delete(ctx);
}

void test_maxChiSymbolLen()
{
    ChewingContext *ctx;
    int i;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    taigi_set_maxChiSymbolLen(ctx, MAX_CHI_SYMBOL_LEN);

    for (i = 0; i < MAX_CHI_SYMBOL_LEN; ++i) {
        type_keystroke_by_string(ctx, "hk4");
    }

    // Use easy symbol 'Orz' as last input for worst case scenario.
    taigi_set_easySymbolInput(ctx, 1);
    type_keystroke_by_string(ctx, "L");

    taigi_delete(ctx);
}

void test_set_selKey_normal()
{
    ChewingContext *ctx;
    int *select_key;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    // XXX: taigi_set_selKey shall accept const char *.
    taigi_set_selKey(ctx, ALTERNATE_SELECT_KEY, ARRAY_SIZE(ALTERNATE_SELECT_KEY));
    select_key = taigi_get_selKey(ctx);
    ok(select_key, "taigi_get_selKey shall not return NULL");
    ok(!memcmp(select_key, ALTERNATE_SELECT_KEY,
               sizeof(ALTERNATE_SELECT_KEY)), "select key shall be ALTERNATE_SELECT_KEY");

    type_keystroke_by_string(ctx, DATA.token);
    ok_preedit_buffer(ctx, DATA.expected);

    taigi_free(select_key);

    taigi_delete(ctx);
}

void test_set_selKey_error_handling()
{
    ChewingContext *ctx;
    int *select_key;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    taigi_set_selKey(NULL, ALTERNATE_SELECT_KEY, ARRAY_SIZE(ALTERNATE_SELECT_KEY));
    select_key = taigi_get_selKey(ctx);
    ok(select_key, "taigi_get_selKey shall not return NULL");
    ok(!memcmp(select_key, DEFAULT_SELECT_KEY, sizeof(DEFAULT_SELECT_KEY)), "select key shall be DEFAULT_SELECT_KEY");
    taigi_free(select_key);

    taigi_set_selKey(ctx, NULL, ARRAY_SIZE(ALTERNATE_SELECT_KEY));
    select_key = taigi_get_selKey(ctx);
    ok(select_key, "taigi_get_selKey shall not return NULL");
    ok(!memcmp(select_key, DEFAULT_SELECT_KEY, sizeof(DEFAULT_SELECT_KEY)), "select key shall be DEFAULT_SELECT_KEY");
    taigi_free(select_key);

    taigi_set_selKey(ctx, ALTERNATE_SELECT_KEY, 0);
    select_key = taigi_get_selKey(ctx);
    ok(select_key, "taigi_get_selKey shall not return NULL");
    ok(!memcmp(select_key, DEFAULT_SELECT_KEY, sizeof(DEFAULT_SELECT_KEY)), "select key shall be DEFAULT_SELECT_KEY");
    taigi_free(select_key);

    taigi_set_selKey(ctx, ALTERNATE_SELECT_KEY, 11);
    select_key = taigi_get_selKey(ctx);
    ok(select_key, "taigi_get_selKey shall not return NULL");
    ok(!memcmp(select_key, DEFAULT_SELECT_KEY, sizeof(DEFAULT_SELECT_KEY)), "select key shall be DEFAULT_SELECT_KEY");
    taigi_free(select_key);

    taigi_delete(ctx);
}

void test_set_selKey()
{
    test_set_selKey_normal();
    test_set_selKey_error_handling();
}

void test_set_addPhraseDirection()
{
    ChewingContext *ctx;
    int value;
    int mode;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    for (value = 0; value < 2; ++value) {
        taigi_set_addPhraseDirection(ctx, value);
        mode = taigi_get_addPhraseDirection(ctx);
        ok(mode == value, "addPhraseDirection `%d' shall be `%d'", mode, value);

        taigi_set_addPhraseDirection(ctx, -1);
        mode = taigi_get_addPhraseDirection(ctx);
        ok(mode == value, "addPhraseDirection `%d' shall be `%d'", mode, value);

        taigi_set_addPhraseDirection(ctx, 2);
        mode = taigi_get_addPhraseDirection(ctx);
        ok(mode == value, "addPhraseDirection `%d' shall be `%d'", mode, value);
    }

    taigi_delete(ctx);
}

void test_set_spaceAsSelection()
{
    ChewingContext *ctx;
    int value;
    int mode;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    for (value = 0; value < 2; ++value) {
        taigi_set_spaceAsSelection(ctx, value);
        mode = taigi_get_spaceAsSelection(ctx);
        ok(mode == value, "spaceAsSelection `%d' shall be `%d'", mode, value);

        taigi_set_spaceAsSelection(ctx, -1);
        mode = taigi_get_spaceAsSelection(ctx);
        ok(mode == value, "spaceAsSelection `%d' shall be `%d'", mode, value);

        taigi_set_spaceAsSelection(ctx, 2);
        mode = taigi_get_spaceAsSelection(ctx);
        ok(mode == value, "spaceAsSelection `%d' shall be `%d'", mode, value);
    }

    taigi_delete(ctx);
}

void test_set_escCleanAllBuf()
{
    ChewingContext *ctx;
    int value;
    int mode;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    for (value = 0; value < 2; ++value) {
        taigi_set_escCleanAllBuf(ctx, value);
        mode = taigi_get_escCleanAllBuf(ctx);
        ok(mode == value, "escCleanAllBuf shall be `%d'", value);

        taigi_set_escCleanAllBuf(ctx, -1);
        mode = taigi_get_escCleanAllBuf(ctx);
        ok(mode == value, "escCleanAllBuf shall be `%d'", value);

        taigi_set_escCleanAllBuf(ctx, 2);
        mode = taigi_get_escCleanAllBuf(ctx);
        ok(mode == value, "escCleanAllBuf shall be `%d'", value);
    }

    taigi_delete(ctx);
}

void test_set_autoShiftCur()
{
    ChewingContext *ctx;
    int value;
    int mode;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    for (value = 0; value < 2; ++value) {
        taigi_set_autoShiftCur(ctx, value);
        mode = taigi_get_autoShiftCur(ctx);
        ok(mode == value, "autoShiftCur shall be `%d'", value);

        taigi_set_autoShiftCur(ctx, -1);
        mode = taigi_get_autoShiftCur(ctx);
        ok(mode == value, "autoShiftCur shall be `%d'", value);

        taigi_set_autoShiftCur(ctx, 2);
        mode = taigi_get_autoShiftCur(ctx);
        ok(mode == value, "autoShiftCur shall be `%d'", value);
    }

    taigi_delete(ctx);
}

void test_set_easySymbolInput()
{
    ChewingContext *ctx;
    int value;
    int mode;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    for (value = 0; value < 2; ++value) {
        taigi_set_easySymbolInput(ctx, value);
        mode = taigi_get_easySymbolInput(ctx);
        ok(mode == value, "easySymbolInput `%d', shall be `%d'", mode, value);

        taigi_set_easySymbolInput(ctx, -1);
        mode = taigi_get_easySymbolInput(ctx);
        ok(mode == value, "easySymbolInput `%d', shall be `%d'", mode, value);

        taigi_set_easySymbolInput(ctx, 2);
        mode = taigi_get_easySymbolInput(ctx);
        ok(mode == value, "easySymbolInput `%d', shall be `%d'", mode, value);
    }

    taigi_delete(ctx);
}

void test_set_phraseChoiceRearward()
{
    ChewingContext *ctx;
    int value;
    int mode;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    for (value = 0; value < 2; ++value) {
        taigi_set_phraseChoiceRearward(ctx, value);
        mode = taigi_get_phraseChoiceRearward(ctx);
        ok(mode == value, "phraseChoiceRearward `%d' shall be `%d'", mode, value);

        taigi_set_phraseChoiceRearward(ctx, -1);
        mode = taigi_get_phraseChoiceRearward(ctx);
        ok(mode == value, "phraseChoiceRearward `%d' shall be `%d'", mode, value);

        taigi_set_phraseChoiceRearward(ctx, 2);
        mode = taigi_get_phraseChoiceRearward(ctx);
        ok(mode == value, "phraseChoiceRearward `%d' shall be `%d'", mode, value);
    }

    taigi_delete(ctx);
}

void test_set_ChiEngMode()
{
    const int VALUE[] = {
        CHINESE_MODE,
        SYMBOL_MODE,
    };

    const int INVALID_VALUE[] = {
        -1,
        2,
    };

    ChewingContext *ctx;
    size_t i;
    size_t j;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    for (i = 0; i < ARRAY_SIZE(VALUE); ++i) {
        taigi_set_ChiEngMode(ctx, VALUE[i]);
        ok(taigi_get_ChiEngMode(ctx) == VALUE[i], "ChiEngMode shall be `%d'", VALUE[i]);

        for (j = 0; j < ARRAY_SIZE(INVALID_VALUE); ++j) {
            // mode shall not change when set mode has invalid value.
            taigi_set_ChiEngMode(ctx, INVALID_VALUE[j]);
            ok(taigi_get_ChiEngMode(ctx) == VALUE[i], "ChiEngMode shall be `%d'", VALUE[i]);
        }
    }

    taigi_delete(ctx);
}

void test_set_ShapeMode()
{
    const int VALUE[] = {
        HALFSHAPE_MODE,
        FULLSHAPE_MODE,
    };

    const int INVALID_VALUE[] = {
        -1,
        2,
    };

    ChewingContext *ctx;
    size_t i;
    size_t j;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    for (i = 0; i < ARRAY_SIZE(VALUE); ++i) {
        taigi_set_ShapeMode(ctx, VALUE[i]);
        ok(taigi_get_ShapeMode(ctx) == VALUE[i], "ShapeMode shall be `%d'", VALUE[i]);

        for (j = 0; j < ARRAY_SIZE(INVALID_VALUE); ++j) {
            // mode shall not change when set mode has invalid value.
            taigi_set_ShapeMode(ctx, INVALID_VALUE[j]);
            ok(taigi_get_ShapeMode(ctx) == VALUE[i], "ShapeMode shall be `%d'", VALUE[i]);
        }
    }

    taigi_delete(ctx);
}

void test_deprecated()
{
    ChewingContext *ctx;
    int type;
    ChewingConfigData configure;

    memset(&configure, 0, sizeof(ChewingConfigData));

    ctx = taigi_new();
    start_testcase(ctx, fd);

BEGIN_IGNORE_DEPRECATIONS
    taigi_set_hsuSelKeyType(ctx, HSU_SELKEY_TYPE1);
    type = taigi_get_hsuSelKeyType(ctx);
    ok(type == 0, "`%d' shall be `%d'", type, 0);

    taigi_Configure(ctx, &configure);
END_IGNORE_DEPRECATIONS

    taigi_delete(ctx);
}

void test_new2_syspath_alternative()
{
    ChewingContext *ctx;

    printf("#\n# %s\n#\n", __func__);
    fprintf(fd, "#\n# %s\n#\n", __func__);

    ctx = taigi_new2(TEST_DATA_DIR, NULL, logger, fd);
    ok(ctx != NULL, "taigi_new2 returns `%#p' shall not be `%#p'", ctx, NULL);

    taigi_delete(ctx);
}

void test_new2_syspath_error()
{
    ChewingContext *ctx;

    printf("#\n# %s\n#\n", __func__);
    fprintf(fd, "#\n# %s\n#\n", __func__);

    ctx = taigi_new2("NoSuchPath", NULL, logger, fd);
    ok(ctx == NULL, "taigi_new2 returns `%#p' shall be `%#p'", ctx, NULL);
}

void test_new2_syspath()
{
    test_new2_syspath_alternative();
    test_new2_syspath_error();
}

void test_new2_userpath_alternative()
{
    ChewingContext *ctx;

    printf("#\n# %s\n#\n", __func__);
    fprintf(fd, "#\n# %s\n#\n", __func__);

    ctx = taigi_new2(NULL, TEST_HASH_DIR "/test.sqlite3", logger, fd);
    ok(ctx != NULL, "taigi_new2 returns `%#p' shall not be `%#p'", ctx, NULL);

    taigi_delete(ctx);
}

void test_new2_userpath_error()
{
    ChewingContext *ctx;

    printf("#\n# %s\n#\n", __func__);
    fprintf(fd, "#\n# %s\n#\n", __func__);

    ctx = taigi_new2(NULL, TEST_HASH_DIR, logger, fd);
    ok(ctx == NULL, "taigi_new2 returns `%#p' shall be `%#p'", ctx, NULL);
}

void test_new2_userpath()
{
    test_new2_userpath_alternative();
    test_new2_userpath_error();
}

void test_new2()
{
    test_new2_syspath();
    test_new2_userpath();
}

int main(int argc, char *argv[])
{
    char *logname;
    int ret;

    putenv("CHEWING_PATH=" CHEWING_DATA_PREFIX);
    putenv("CHEWING_USER_PATH=" TEST_HASH_DIR);

    ret = asprintf(&logname, "%s.log", argv[0]);
    if (ret == -1)
        return -1;
    fd = fopen(logname, "w");
    assert(fd);
    free(logname);


    test_default_value();

    test_set_candPerPage();
    test_set_maxChiSymbolLen();
    test_maxChiSymbolLen();
    test_set_selKey();
    test_set_addPhraseDirection();
    test_set_spaceAsSelection();
    test_set_escCleanAllBuf();
    test_set_autoShiftCur();
    test_set_easySymbolInput();
    test_set_phraseChoiceRearward();
    test_set_ChiEngMode();
    test_set_ShapeMode();

    test_deprecated();

    test_new2();

    fclose(fd);

    return exit_status();
}
