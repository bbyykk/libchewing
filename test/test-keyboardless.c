/**
 * test-keyboardless.c
 *
 * Copyright (c) 2013
 *      libchewing Core Team. See ChangeLog for details.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file.
 */
#include <assert.h>
#include <stdlib.h>

#include "testhelper.h"
#include "chewing.h"

FILE *fd;

void test_cand_open_word()
{
    ChewingContext *ctx;
    int ret;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    type_keystroke_by_string(ctx, "hk4" /* ㄘㄜˋ */ );

    ret = taigi_cand_open(ctx);
    ok(ret == 0, "taigi_cand_open() returns `%d' shall be `%d'", ret, 0);

    ret = taigi_cand_TotalChoice(ctx);
    ok(ret > 0, "taigi_cand_TotalChoice() returns `%d' shall be greater than `%d'", ret, 0);

    taigi_delete(ctx);
}

void test_cand_open_symbol()
{
    ChewingContext *ctx;
    int ret;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    type_keystroke_by_string(ctx, "`31" /* ， */ );

    ret = taigi_cand_open(ctx);
    ok(ret == 0, "taigi_cand_open() returns `%d' shall be `%d'", ret, 0);

    ret = taigi_cand_TotalChoice(ctx);
    ok(ret > 0, "taigi_cand_TotalChoice() returns `%d' shall be greater than `%d'", ret, 0);

    taigi_delete(ctx);
}

void test_cand_open_already_opened()
{
    ChewingContext *ctx;
    int ret;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    type_keystroke_by_string(ctx, "hk4" /* ㄘㄜˋ */ );

    ret = taigi_cand_open(ctx);
    ok(ret == 0, "taigi_cand_open() returns `%d' shall be `%d'", ret, 0);

    ret = taigi_cand_TotalChoice(ctx);
    ok(ret > 0, "taigi_cand_TotalChoice() returns `%d' shall be greater than `%d'", ret, 0);

    /* FIXME: Need to ensure the candidate windows does not change */
    ret = taigi_cand_open(ctx);
    ok(ret == 0, "taigi_cand_open() returns `%d' shall be `%d'", ret, 0);

    ret = taigi_cand_TotalChoice(ctx);
    ok(ret > 0, "taigi_cand_TotalChoice() returns `%d' shall be greater than `%d'", ret, 0);

    taigi_delete(ctx);
}

void test_cand_open_nothing_in_preedit()
{
    ChewingContext *ctx;
    int ret;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    ret = taigi_cand_open(ctx);
    ok(ret == -1, "taigi_cand_open() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_cand_TotalChoice(ctx);
    ok(ret == 0, "taigi_cand_TotalChoice() returns `%d' shall be `%d'", ret, 0);

    taigi_delete(ctx);
}

void test_cand_open_during_bopomofo()
{
    ChewingContext *ctx;
    int ret;

    /* FIXME: shall we clean bopomofo when taigi_cand_open is called? */

    ctx = taigi_new();
    start_testcase(ctx, fd);

    type_keystroke_by_string(ctx, "hk" /* ㄘㄜ */ );

    ret = taigi_cand_open(ctx);
    ok(ret == -1, "taigi_cand_open() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_cand_TotalChoice(ctx);
    ok(ret == 0, "taigi_cand_TotalChoice() returns `%d' shall be `%d'", ret, 0);

    taigi_Reset(ctx);

    type_keystroke_by_string(ctx, "hk4g" /* ㄘㄜˋ ㄕ */ );
    ret = taigi_cand_open(ctx);
    ok(ret == 0, "taigi_cand_open() returns `%d' shall be `%d'", ret, 0);

    ret = taigi_cand_TotalChoice(ctx);
    ok(ret > 0, "taigi_cand_TotalChoice() returns `%d' shall be greater than `%d'", ret, 0);

    taigi_delete(ctx);
}

void test_cand_open()
{
    test_cand_open_word();
    test_cand_open_symbol();
    test_cand_open_already_opened();
    test_cand_open_nothing_in_preedit();
    test_cand_open_during_bopomofo();
}

void test_cand_close_word()
{
    ChewingContext *ctx;
    int ret;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    type_keystroke_by_string(ctx, "hk4" /* ㄘㄜˋ */ );

    ret = taigi_cand_open(ctx);
    ok(ret == 0, "taigi_cand_open() returns `%d' shall be `%d'", ret, 0);

    ret = taigi_cand_TotalChoice(ctx);
    ok(ret > 0, "taigi_cand_TotalChoice() returns `%d' shall be greater than `%d'", ret, 0);

    ret = taigi_cand_close(ctx);
    ok(ret == 0, "taigi_cand_close() returns `%d' shall be `%d'", ret, 0);

    ret = taigi_cand_TotalChoice(ctx);
    ok(ret == 0, "taigi_cand_TotalChoice() returns `%d' shall be 0 than `%d'", ret, 0);

    ok_commit_buffer(ctx, "");

    taigi_delete(ctx);
}


void test_cand_close_symbol()
{
    ChewingContext *ctx;
    int ret;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    type_keystroke_by_string(ctx, "`31" /* ， */ );

    ret = taigi_cand_open(ctx);
    ok(ret == 0, "taigi_cand_open() returns `%d' shall be `%d'", ret, 0);

    ret = taigi_cand_TotalChoice(ctx);
    ok(ret > 0, "taigi_cand_TotalChoice() returns `%d' shall be greater than `%d'", ret, 0);

    ret = taigi_cand_close(ctx);
    ok(ret == 0, "taigi_cand_close() returns `%d' shall be `%d'", ret, 0);

    ret = taigi_cand_TotalChoice(ctx);
    ok(ret == 0, "taigi_cand_TotalChoice() returns `%d' shall be 0 than `%d'", ret, 0);

    ok_commit_buffer(ctx, "");

    taigi_delete(ctx);
}

void test_cand_close_already_closed()
{
    ChewingContext *ctx;
    int ret;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    type_keystroke_by_string(ctx, "hk4" /* ㄘㄜˋ */ );

    ret = taigi_cand_close(ctx);
    ok(ret == 0, "taigi_cand_close() returns `%d' shall be `%d'", ret, 0);

    ret = taigi_cand_TotalChoice(ctx);
    ok(ret == 0, "taigi_cand_TotalChoice() returns `%d' shall be 0 than `%d'", ret, 0);

    taigi_delete(ctx);
}

void test_cand_close_nothing_in_preedit()
{
    ChewingContext *ctx;
    int ret;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    ret = taigi_cand_close(ctx);
    ok(ret == 0, "taigi_cand_close() returns `%d' shall be `%d'", ret, 0);

    ret = taigi_cand_TotalChoice(ctx);
    ok(ret == 0, "taigi_cand_TotalChoice() returns `%d' shall be 0 than `%d'", ret, 0);

    taigi_delete(ctx);
}

void test_cand_close()
{
    test_cand_close_word();
    test_cand_close_symbol();
    test_cand_close_already_closed();
    test_cand_close_nothing_in_preedit();
}

void test_cand_choose_word()
{
    ChewingContext *ctx;
    int ret;

    clean_userphrase();

    ctx = taigi_new();
    start_testcase(ctx, fd);

    type_keystroke_by_string(ctx, "hk4" /* ㄘㄜˋ */ );

    ret = taigi_cand_open(ctx);
    ok(ret == 0, "taigi_cand_open() returns `%d' shall be `%d'", ret, 0);

    ret = taigi_cand_TotalChoice(ctx);
    ok(ret > 0, "taigi_cand_TotalChoice() returns `%d' shall be greater than `%d'", ret, 0);

    ret = taigi_cand_choose_by_index(ctx, 2);
    ok(ret == 0, "taigi_cand_choose_by_index() returns `%d' shall be `%d'", ret, 0);

    ok_preedit_buffer(ctx, "\xE6\xB8\xAC" /* 測 */ );

    taigi_delete(ctx);
}

void test_cand_choose_symbol()
{
    ChewingContext *ctx;
    int ret;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    type_keystroke_by_string(ctx, "`" /* ， */ );

    ret = taigi_cand_choose_by_index(ctx, 2);
    ok(ret == 0, "taigi_cand_choose_by_index() returns `%d' shall be `%d'", ret, 0);

    ret = taigi_cand_choose_by_index(ctx, 0);
    ok(ret == 0, "taigi_cand_choose_by_index() returns `%d' shall be `%d'", ret, 0);

    ok_preedit_buffer(ctx, "\xEF\xBC\x8C" /* ， */ );

    taigi_delete(ctx);
}

void test_cand_choose_out_of_range()
{
    ChewingContext *ctx;
    int ret;
    int total_choice;

    clean_userphrase();

    ctx = taigi_new();
    start_testcase(ctx, fd);

    type_keystroke_by_string(ctx, "hk4" /* ㄘㄜˋ */ );

    ret = taigi_cand_open(ctx);
    ok(ret == 0, "taigi_cand_open() returns `%d' shall be `%d'", ret, 0);

    total_choice = taigi_cand_TotalChoice(ctx);
    ok(total_choice > 0, "taigi_cand_TotalChoice() returns `%d' shall be greater than `%d'", total_choice, 0);

    ret = taigi_cand_choose_by_index(ctx, total_choice);
    ok(ret == -1, "taigi_cand_choose_by_index() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_cand_choose_by_index(ctx, -1);
    ok(ret == -1, "taigi_cand_choose_by_index() returns `%d' shall be `%d'", ret, -1);

    ok_preedit_buffer(ctx, "\xE5\x86\x8A" /* 冊 */ );

    taigi_delete(ctx);
}

void test_cand_choose_second_layer()
{
    ChewingContext *ctx;
    int ret;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    type_keystroke_by_string(ctx, "`");
    ret = taigi_cand_TotalChoice(ctx);
    ok(ret > 0, "taigi_cand_TotalChoice() returns `%d' shall be greater than `%d'", ret, 0);
    ret = taigi_cand_choose_by_index(ctx, 2);
    ok(ret == 0, "taigi_cand_choose_by_index() returns `%d' shall be `%d'", ret, 0);
    ret = taigi_cand_TotalChoice(ctx);
    ok(ret > 0, "taigi_cand_TotalChoice() returns `%d' shall be greater than `%d'", ret, 0);
    ret = taigi_cand_choose_by_index(ctx, 0);
    ok(ret == 0, "taigi_cand_choose_by_index() returns `%d' shall be `%d'", ret, 0);
    ok_preedit_buffer(ctx, "\xEF\xBC\x8C" /* ， */ );

    taigi_delete(ctx);
}

void test_cand_choose_not_in_select()
{
    ChewingContext *ctx;
    int ret;

    clean_userphrase();

    ctx = taigi_new();
    start_testcase(ctx, fd);

    type_keystroke_by_string(ctx, "hk4" /* ㄘㄜˋ */ );

    ret = taigi_cand_TotalChoice(ctx);
    ok(ret == 0, "taigi_cand_TotalChoice() returns `%d' shall be `%d'", ret, 0);

    ret = taigi_cand_choose_by_index(ctx, 2);
    ok(ret == -1, "taigi_cand_choose_by_index() returns `%d' shall be `%d'", ret, -1);

    ok_preedit_buffer(ctx, "\xE5\x86\x8A" /* 冊 */ );

    taigi_delete(ctx);
}

void test_cand_choose()
{
    test_cand_choose_word();
    test_cand_choose_symbol();
    test_cand_choose_second_layer();
    test_cand_choose_out_of_range();
    test_cand_choose_not_in_select();
}

void test_cand_list_word_no_rearward()
{
    ChewingContext *ctx;
    int ret;

    ctx = taigi_new();
    start_testcase(ctx, fd);
    taigi_set_phraseChoiceRearward(ctx, 0);

    type_keystroke_by_string(ctx, "hk4g4<H>" /* 測試 */ );

    ret = taigi_cand_open(ctx);
    ok(ret == 0, "taigi_cand_open() returns `%d' shall be `%d'", ret, 0);
    ok_candidate_len(ctx, 2);

    ret = taigi_cand_list_has_next(ctx);
    ok(ret == 1, "taigi_cand_list_has_next() returns `%d' shall be `%d'", ret, 1);
    ret = taigi_cand_list_next(ctx);
    ok(ret == 0, "taigi_cand_list_next() returns `%d' shall be `%d'", ret, 0);
    ok_candidate_len(ctx, 1);

    ret = taigi_cand_list_has_next(ctx);
    ok(ret == 0, "taigi_cand_list_has_next() returns `%d' shall be `%d'", ret, 0);
    ret = taigi_cand_list_next(ctx);
    ok(ret == -1, "taigi_cand_list_next() returns `%d' shall be `%d'", ret, -1);
    ok_candidate_len(ctx, 1);

    ret = taigi_cand_list_has_prev(ctx);
    ok(ret == 1, "taigi_cand_list_has_prev() returns `%d' shall be `%d'", ret, 1);
    ret = taigi_cand_list_prev(ctx);
    ok(ret == 0, "taigi_cand_list_prev() returns `%d' shall be `%d'", ret, 0);
    ok_candidate_len(ctx, 2);

    ret = taigi_cand_list_has_prev(ctx);
    ok(ret == 0, "taigi_cand_list_has_prev() returns `%d' shall be `%d'", ret, 0);
    ret = taigi_cand_list_prev(ctx);
    ok(ret == -1, "taigi_cand_list_prev() returns `%d' shall be `%d'", ret, -1);
    ok_candidate_len(ctx, 2);

    ret = taigi_cand_list_last(ctx);
    ok(ret == 0, "taigi_cand_list_last() returns `%d' shall be `%d'", ret, 0);
    ok_candidate_len(ctx, 1);

    ret = taigi_cand_list_first(ctx);
    ok(ret == 0, "taigi_cand_list_first() returns `%d' shall be `%d'", ret, 0);
    ok_candidate_len(ctx, 2);

    taigi_delete(ctx);
}

void test_cand_list_word_rearward()
{
    ChewingContext *ctx;
    int ret;

    ctx = taigi_new();
    start_testcase(ctx, fd);
    taigi_set_phraseChoiceRearward(ctx, 1);

    type_keystroke_by_string(ctx, "hk4g4" /* 測試 */ );

    ret = taigi_cand_open(ctx);
    ok(ret == 0, "taigi_cand_open() returns `%d' shall be `%d'", ret, 0);
    ok_candidate_len(ctx, 2);

    ret = taigi_cand_list_has_next(ctx);
    ok(ret == 1, "taigi_cand_list_has_next() returns `%d' shall be `%d'", ret, 1);
    ret = taigi_cand_list_next(ctx);
    ok(ret == 0, "taigi_cand_list_next() returns `%d' shall be `%d'", ret, 0);
    ok_candidate_len(ctx, 1);

    ret = taigi_cand_list_has_next(ctx);
    ok(ret == 0, "taigi_cand_list_has_next() returns `%d' shall be `%d'", ret, 0);
    ret = taigi_cand_list_next(ctx);
    ok(ret == -1, "taigi_cand_list_next() returns `%d' shall be `%d'", ret, -1);
    ok_candidate_len(ctx, 1);

    ret = taigi_cand_list_has_prev(ctx);
    ok(ret == 1, "taigi_cand_list_has_prev() returns `%d' shall be `%d'", ret, 1);
    ret = taigi_cand_list_prev(ctx);
    ok(ret == 0, "taigi_cand_list_prev() returns `%d' shall be `%d'", ret, 0);
    ok_candidate_len(ctx, 2);

    ret = taigi_cand_list_has_prev(ctx);
    ok(ret == 0, "taigi_cand_list_has_prev() returns `%d' shall be `%d'", ret, 0);
    ret = taigi_cand_list_prev(ctx);
    ok(ret == -1, "taigi_cand_list_prev() returns `%d' shall be `%d'", ret, -1);
    ok_candidate_len(ctx, 2);

    ret = taigi_cand_list_last(ctx);
    ok(ret == 0, "taigi_cand_list_last() returns `%d' shall be `%d'", ret, 0);
    ok_candidate_len(ctx, 1);

    ret = taigi_cand_list_first(ctx);
    ok(ret == 0, "taigi_cand_list_first() returns `%d' shall be `%d'", ret, 0);
    ok_candidate_len(ctx, 2);

    taigi_delete(ctx);
}


void test_cand_list_word_selection_next_no_rearward()
{
    ChewingContext *ctx;
    int ret;

    clean_userphrase();

    ctx = taigi_new();
    start_testcase(ctx, fd);

    type_keystroke_by_string(ctx, "u61o4y7<H>" /* ㄧˊ ㄅㄟˋ ㄗ˙ */ );

    ret = taigi_cand_open(ctx);
    ok(ret == 0, "taigi_cand_open() returns `%d' shall be `%d'", ret, 0);

    ret = taigi_cand_TotalChoice(ctx);
    ok(ret > 0, "taigi_cand_TotalChoice() returns `%d' shall be greater than `%d'", ret, 0);
    ok_candidate_len(ctx, 3);

    ret = taigi_cand_list_has_next(ctx);
    ok(ret == 1, "taigi_cand_list_has_next() returns `%d' shall be `%d'", ret, 1);
    ret = taigi_cand_list_next(ctx);
    ok(ret == 0, "taigi_cand_list_next() returns `%d' shall be `%d'", ret, 0);

    ret = taigi_cand_TotalChoice(ctx);
    ok(ret > 0, "taigi_cand_TotalChoice() returns `%d' shall be greater than `%d'", ret, 0);
    ok_candidate_len(ctx, 2);

    ret = taigi_cand_choose_by_index(ctx, 1);
    ok(ret == 0, "taigi_cand_choose_by_index() returns `%d' shall be `%d'", ret, 0);

    ok_preedit_buffer(ctx, "\xE4\xB8\x80\xE8\xBC\xA9\xE5\xAD\x90" /* 一輩子 */ );

    taigi_delete(ctx);
}

void test_cand_list_word_selection_next_rearward()
{
    ChewingContext *ctx;
    int ret;

    clean_userphrase();

    ctx = taigi_new();
    start_testcase(ctx, fd);

    taigi_set_phraseChoiceRearward(ctx, 1);

    type_keystroke_by_string(ctx, "u61o4y7" /* ㄧˊ ㄅㄟˋ ㄗ˙ */ );

    ret = taigi_cand_open(ctx);
    ok(ret == 0, "taigi_cand_open() returns `%d' shall be `%d'", ret, 0);

    ret = taigi_cand_TotalChoice(ctx);
    ok(ret > 0, "taigi_cand_TotalChoice() returns `%d' shall be greater than `%d'", ret, 0);
    ok_candidate_len(ctx, 3);

    ret = taigi_cand_list_has_next(ctx);
    ok(ret == 1, "taigi_cand_list_has_next() returns `%d' shall be `%d'", ret, 1);
    ret = taigi_cand_list_next(ctx);
    ok(ret == 0, "taigi_cand_list_next() returns `%d' shall be `%d'", ret, 0);

    ret = taigi_cand_TotalChoice(ctx);
    ok(ret > 0, "taigi_cand_TotalChoice() returns `%d' shall be greater than `%d'", ret, 0);
    ok_candidate_len(ctx, 2);

    ret = taigi_cand_choose_by_index(ctx, 0);
    ok(ret == 0, "taigi_cand_choose_by_index() returns `%d' shall be `%d'", ret, 0);

    ok_preedit_buffer(ctx, "\xE4\xB8\x80\xE8\xBC\xA9\xE5\xAD\x90" /* 一輩子 */ );

    taigi_delete(ctx);
}

void test_cand_list_word_selection_prev_no_rearward()
{
    ChewingContext *ctx;
    int ret;

    clean_userphrase();

    ctx = taigi_new();
    start_testcase(ctx, fd);

    type_keystroke_by_string(ctx, "u61o4y7<H>" /* ㄧˊ ㄅㄟˋ ㄗ˙ */ );

    ret = taigi_cand_open(ctx);
    ok(ret == 0, "taigi_cand_open() returns `%d' shall be `%d'", ret, 0);

    ret = taigi_cand_TotalChoice(ctx);
    ok(ret > 0, "taigi_cand_TotalChoice() returns `%d' shall be greater than `%d'", ret, 0);
    ok_candidate_len(ctx, 3);

    ret = taigi_cand_list_last(ctx);
    ok(ret == 0, "taigi_cand_list_last() returns `%d' shall be `%d'", ret, 1);
    ok_candidate_len(ctx, 1);

    ret = taigi_cand_list_has_prev(ctx);
    ok(ret == 1, "taigi_cand_list_has_prev() returns `%d' shall be `%d'", ret, 1);
    ret = taigi_cand_list_prev(ctx);
    ok(ret == 0, "taigi_cand_list_prev() returns `%d' shall be `%d'", ret, 0);

    ret = taigi_cand_TotalChoice(ctx);
    ok(ret > 0, "taigi_cand_TotalChoice() returns `%d' shall be greater than `%d'", ret, 0);
    ok_candidate_len(ctx, 2);

    ret = taigi_cand_choose_by_index(ctx, 1);
    ok(ret == 0, "taigi_cand_choose_by_index() returns `%d' shall be `%d'", ret, 0);

    ok_preedit_buffer(ctx, "\xE4\xB8\x80\xE8\xBC\xA9\xE5\xAD\x90" /* 一輩子 */ );

    taigi_delete(ctx);
}

void test_cand_list_word_selection_prev_rearward()
{
    ChewingContext *ctx;
    int ret;

    clean_userphrase();

    ctx = taigi_new();
    start_testcase(ctx, fd);

    taigi_set_phraseChoiceRearward(ctx, 1);

    type_keystroke_by_string(ctx, "u61o4y7" /* ㄧˊ ㄅㄟˋ ㄗ˙ */ );

    ret = taigi_cand_open(ctx);
    ok(ret == 0, "taigi_cand_open() returns `%d' shall be `%d'", ret, 0);

    ret = taigi_cand_TotalChoice(ctx);
    ok(ret > 0, "taigi_cand_TotalChoice() returns `%d' shall be greater than `%d'", ret, 0);
    ok_candidate_len(ctx, 3);

    ret = taigi_cand_list_last(ctx);
    ok(ret == 0, "taigi_cand_list_last() returns `%d' shall be `%d'", ret, 1);
    ok_candidate_len(ctx, 1);

    ret = taigi_cand_list_has_prev(ctx);
    ok(ret == 1, "taigi_cand_list_has_prev() returns `%d' shall be `%d'", ret, 1);
    ret = taigi_cand_list_prev(ctx);
    ok(ret == 0, "taigi_cand_list_prev() returns `%d' shall be `%d'", ret, 0);

    ret = taigi_cand_TotalChoice(ctx);
    ok(ret > 0, "taigi_cand_TotalChoice() returns `%d' shall be greater than `%d'", ret, 0);
    ok_candidate_len(ctx, 2);

    ret = taigi_cand_choose_by_index(ctx, 0);
    ok(ret == 0, "taigi_cand_choose_by_index() returns `%d' shall be `%d'", ret, 0);

    ok_preedit_buffer(ctx, "\xE4\xB8\x80\xE8\xBC\xA9\xE5\xAD\x90" /* 一輩子 */ );

    taigi_delete(ctx);
}

void test_cand_list_word()
{
    test_cand_list_word_no_rearward();
    test_cand_list_word_rearward();
    test_cand_list_word_selection_next_no_rearward();
    test_cand_list_word_selection_next_rearward();
    test_cand_list_word_selection_prev_no_rearward();
    test_cand_list_word_selection_prev_rearward();
}

void test_cand_list_symbol()
{
    ChewingContext *ctx;
    int ret;

    ctx = taigi_new();
    start_testcase(ctx, fd);
    type_keystroke_by_string(ctx, "`31" /* ， */ );

    ret = taigi_cand_open(ctx);
    ok(ret == 0, "taigi_cand_open() returns `%d' shall be `%d'", ret, 0);
    ok_candidate_len(ctx, 1);

    ret = taigi_cand_list_has_next(ctx);
    ok(ret == 0, "taigi_cand_list_has_next() returns `%d' shall be `%d'", ret, 0);
    ret = taigi_cand_list_next(ctx);
    ok(ret == -1, "taigi_cand_list_has_next() returns `%d' shall be `%d'", ret, -1);
    ok_candidate_len(ctx, 1);

    ret = taigi_cand_list_has_prev(ctx);
    ok(ret == 0, "taigi_cand_list_has_prev() returns `%d' shall be `%d'", ret, 0);
    ret = taigi_cand_list_prev(ctx);
    ok(ret == -1, "taigi_cand_list_prev() returns `%d' shall be `%d'", ret, -1);
    ok_candidate_len(ctx, 1);

    ret = taigi_cand_list_first(ctx);
    ok(ret == 0, "taigi_cand_list_first() returns `%d' shall be `%d'", ret, 0);
    ok_candidate_len(ctx, 1);

    ret = taigi_cand_list_last(ctx);
    ok(ret == 0, "taigi_cand_list_last() returns `%d' shall be `%d'", ret, 0);
    ok_candidate_len(ctx, 1);

    taigi_delete(ctx);
}

void test_cand_list_no_cand_windows()
{
    ChewingContext *ctx;
    int ret;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    type_keystroke_by_string(ctx, "hk4g4" /* 測試 */ );

    ret = taigi_cand_list_has_next(ctx);
    ok(ret == 0, "taigi_cand_list_has_next() returns `%d' shall be `%d'", ret, 0);
    ret = taigi_cand_list_next(ctx);
    ok(ret == -1, "taigi_cand_list_next() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_cand_list_has_prev(ctx);
    ok(ret == 0, "taigi_cand_list_has_prev() returns `%d' shall be `%d'", ret, 0);
    ret = taigi_cand_list_prev(ctx);
    ok(ret == -1, "taigi_cand_list_prev() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_cand_list_first(ctx);
    ok(ret == -1, "taigi_cand_list_first() returns `%d' shall be `%d'", ret, 0);

    ret = taigi_cand_list_last(ctx);
    ok(ret == -1, "taigi_cand_list_last() returns `%d' shall be `%d'", ret, 0);

    taigi_delete(ctx);
}

void test_cand_list()
{
    test_cand_list_word();
    test_cand_list_symbol();
    test_cand_list_no_cand_windows();
}

void test_commit_preedit_normal()
{
    static const char phrase[] = "\xE6\xB8\xAC\xE8\xA9\xA6" /* 測試 */ ;
    ChewingContext *ctx;
    int ret;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    type_keystroke_by_string(ctx, "hk4g4" /* 測試 */ );
    ret = taigi_commit_preedit_buf(ctx);
    ok(ret == 0, "taigi_commit_preedit_buf() returns `%d' shall be `%d'", ret, 0);

    ok_preedit_buffer(ctx, "");
    ok_commit_buffer(ctx, phrase);

    taigi_delete(ctx);
}

void test_commit_preedit_empty()
{
    ChewingContext *ctx;
    int ret;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    ret = taigi_commit_preedit_buf(ctx);
    ok(ret == -1, "taigi_commit_preedit_buf() returns `%d' shall be `%d'", ret, -1);

    ok_preedit_buffer(ctx, "");
    ok_commit_buffer(ctx, "");

    taigi_delete(ctx);
}

void test_commit_preedit_during_cand_selecting()
{
    static const char phrase[] = "\xE6\xB8\xAC\xE8\xA9\xA6" /* 測試 */ ;
    ChewingContext *ctx;
    int ret;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    type_keystroke_by_string(ctx, "hk4g4" /* 測試 */ );

    ret = taigi_cand_open(ctx);
    ok(ret == 0, "taigi_cand_open() returns `%d' shall be `%d'", ret, 0);

    ret = taigi_commit_preedit_buf(ctx);
    ok(ret == -1, "taigi_commit_preedit_buf() returns `%d' shall be `%d'", ret, -1);

    ok_preedit_buffer(ctx, phrase);
    ok_commit_buffer(ctx, "");

    taigi_delete(ctx);
}

void test_commit_preedit()
{
    test_commit_preedit_normal();
    test_commit_preedit_empty();
    test_commit_preedit_during_cand_selecting();
}

void test_clean_preedit_normal()
{
    ChewingContext *ctx;
    int ret;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    type_keystroke_by_string(ctx, "hk4g4" /* 測試 */ );
    ret = taigi_clean_preedit_buf(ctx);
    ok(ret == 0, "taigi_clean_preedit_buf() returns `%d' shall be `%d'", ret, 0);

    ok_preedit_buffer(ctx, "");
    ok_commit_buffer(ctx, "");

    taigi_delete(ctx);
}

void test_clean_preedit_empty()
{
    ChewingContext *ctx;
    int ret;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    ret = taigi_clean_preedit_buf(ctx);
    ok(ret == 0, "taigi_clean_preedit_buf() returns `%d' shall be `%d'", ret, 0);

    ok_preedit_buffer(ctx, "");
    ok_commit_buffer(ctx, "");

    taigi_delete(ctx);
}

void test_clean_preedit_during_cand_selecting()
{
    static const char phrase[] = "\xE6\xB8\xAC\xE8\xA9\xA6" /* 測試 */ ;
    ChewingContext *ctx;
    int ret;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    type_keystroke_by_string(ctx, "hk4g4" /* 測試 */ );

    ret = taigi_cand_open(ctx);
    ok(ret == 0, "taigi_cand_open() returns `%d' shall be `%d'", ret, 0);

    ret = taigi_clean_preedit_buf(ctx);
    ok(ret == -1, "taigi_commit_preedit_buf() returns `%d' shall be `%d'", ret, -1);

    ok_preedit_buffer(ctx, phrase);
    ok_commit_buffer(ctx, "");

    taigi_delete(ctx);
}

void test_clean_preedit()
{
    test_clean_preedit_normal();
    test_clean_preedit_empty();
    test_clean_preedit_during_cand_selecting();
}

void test_clean_bopomofo_normal()
{
    ChewingContext *ctx;
    int ret;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    type_keystroke_by_string(ctx, "hk" /* ㄘㄜ */ );
    ret = taigi_clean_bopomofo_buf(ctx);
    ok(ret == 0, "taigi_clean_bopomofo_buf() returns `%d' shall be `%d'", ret, 0);

    ok_bopomofo_buffer(ctx, "");

    taigi_delete(ctx);
}

void test_clean_bopomofo_empty()
{
    ChewingContext *ctx;
    int ret;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    ret = taigi_clean_bopomofo_buf(ctx);
    ok(ret == 0, "taigi_clean_bopomofo_buf() returns `%d' shall be `%d'", ret, 0);

    ok_bopomofo_buffer(ctx, "");

    taigi_delete(ctx);
}

void test_clean_bopomofo_after_complete()
{
    ChewingContext *ctx;
    int ret;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    type_keystroke_by_string(ctx, "hk4" /* ㄘㄜˋ */ );
    ret = taigi_clean_bopomofo_buf(ctx);
    ok(ret == 0, "taigi_clean_bopomofo_buf() returns `%d' shall be `%d'", ret, 0);

    ok_preedit_buffer(ctx, "\xE5\x86\x8A" /* 冊 */ );
    ok_bopomofo_buffer(ctx, "");

    taigi_delete(ctx);
}

void test_clean_bopomofo_during_cand_selecting()
{
    ChewingContext *ctx;
    int ret;

    ctx = taigi_new();
    start_testcase(ctx, fd);

    type_keystroke_by_string(ctx, "hk4g4" /* 測試 */ );

    ret = taigi_cand_open(ctx);
    ok(ret == 0, "taigi_cand_open() returns `%d' shall be `%d'", ret, 0);

    /* XXX: Shall return 0 in select mode? */
    ret = taigi_clean_bopomofo_buf(ctx);
    ok(ret == 0, "taigi_clean_bopomofo_buf() returns `%d' shall be `%d'", ret, 0);

    ok_bopomofo_buffer(ctx, "");

    taigi_delete(ctx);
}

void test_clean_bopomofo()
{
    test_clean_bopomofo_normal();
    test_clean_bopomofo_empty();
    test_clean_bopomofo_after_complete();
    test_clean_bopomofo_during_cand_selecting();
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


    test_cand_open();
    test_cand_close();
    test_cand_choose();
    test_cand_list();

    test_commit_preedit();
    test_clean_preedit();

    test_clean_bopomofo();

    fclose(fd);

    return exit_status();
}
