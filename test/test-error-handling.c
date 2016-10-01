/**
 * test-error-handling.c
 *
 * Copyright (c) 2013
 *      libchewing Core Team. See ChangeLog for details.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file.
 */
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "testhelper.h"
#include "taigi.h"

FILE *fd;

void test_null()
{
    int ret;
    char *buf;
    const char *const_buf;
    int *key;
    unsigned short *phone;

    start_testcase(NULL, fd);

    taigi_Reset(NULL);        // shall not crash

    ret = taigi_set_KBType(NULL, 0);
    ok(ret == -1, "taigi_set_KBType() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_get_KBType(NULL);
    ok(ret == -1, "taigi_get_KBType() returns `%d' shall be `%d'", ret, -1);

    buf = taigi_get_KBString(NULL);
    ok(strcmp(buf, "") == 0, "taigi_get_KBString() returns `%s' shall be `%s'", buf, "");
    taigi_free(buf);

    taigi_delete(NULL);       // shall not crash

    taigi_free(NULL);         // shall not crash

    taigi_set_candPerPage(NULL, 0);   // shall not crash

    ret = taigi_get_candPerPage(NULL);
    ok(ret == -1, "taigi_get_candPerPage() returns `%d' shall be `%d'", ret, -1);

    taigi_set_maxChiSymbolLen(NULL, 0);       // shall not crash

    ret = taigi_get_maxChiSymbolLen(NULL);
    ok(ret == -1, "taigi_get_maxChiSymbolLen() returns `%d' shall be `%d'", ret, -1);

    taigi_set_selKey(NULL, NULL, 0);  // shall not crash

    key = taigi_get_selKey(NULL);
    ok(key == NULL, "taigi_get_selKey() returns NULL");
    taigi_free(key);

    taigi_set_addPhraseDirection(NULL, 0);    // shall not crash

    ret = taigi_get_addPhraseDirection(NULL);
    ok(ret == -1, "taigi_get_addPhraseDirection() returns `%d' shall be `%d'", ret, -1);

    taigi_set_spaceAsSelection(NULL, 0);      // shall not crash

    ret = taigi_get_spaceAsSelection(NULL);
    ok(ret == -1, "taigi_get_spaceAsSelection() returns `%d' shall be `%d'", ret, -1);

    taigi_set_escCleanAllBuf(NULL, 0);        // shall not crash

    ret = taigi_get_escCleanAllBuf(NULL);
    ok(ret == -1, "taigi_get_escCleanAllBuf() returns `%d' shall be `%d'", ret, -1);

    taigi_set_autoShiftCur(NULL, 0);  // shall not crash

    ret = taigi_get_autoShiftCur(NULL);
    ok(ret == -1, "taigi_get_autoShiftCur() returns `%d' shall be `%d'", ret, -1);

    taigi_set_easySymbolInput(NULL, 0);       // shall not crash

    ret = taigi_get_easySymbolInput(NULL);
    ok(ret == -1, "taigi_get_easySymbolInput() returns `%d' shall be `%d'", ret, -1);

    taigi_set_phraseChoiceRearward(NULL, 0);

    ret = taigi_get_phraseChoiceRearward(NULL);
    ok(ret == -1, "taigi_get_phraseChoiceRearward() returns `%d' shall be `%d'", ret, -1);

    taigi_set_ChiEngMode(NULL, 0);    // shall not crash

    ret = taigi_get_ChiEngMode(NULL);
    ok(ret == -1, "taigi_get_ChiEngMode() returns `%d' shall be `%d'", ret, -1);

    taigi_set_ShapeMode(NULL, 0);     // shall not crash

    ret = taigi_handle_Space(NULL);
    ok(ret == -1, "taigi_handle_Space() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_handle_Esc(NULL);
    ok(ret == -1, "taigi_handle_Esc() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_handle_Enter(NULL);
    ok(ret == -1, "taigi_handle_Enter() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_handle_Del(NULL);
    ok(ret == -1, "taigi_handle_Del() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_handle_Backspace(NULL);
    ok(ret == -1, "taigi_handle_Backspace() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_handle_Up(NULL);
    ok(ret == -1, "taigi_handle_Up() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_handle_Down(NULL);
    ok(ret == -1, "taigi_handle_Down() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_handle_ShiftLeft(NULL);
    ok(ret == -1, "taigi_handle_ShiftLeft() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_handle_Left(NULL);
    ok(ret == -1, "taigi_handle_Left() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_handle_ShiftRight(NULL);
    ok(ret == -1, "taigi_handle_ShiftRight() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_handle_Right(NULL);
    ok(ret == -1, "taigi_handle_Right() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_handle_Tab(NULL);
    ok(ret == -1, "taigi_handle_Tab() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_handle_DblTab(NULL);
    ok(ret == -1, "taigi_handle_DblTab() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_handle_Capslock(NULL);
    ok(ret == -1, "taigi_handle_Capslock() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_handle_Home(NULL);
    ok(ret == -1, "taigi_handle_Home() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_handle_PageUp(NULL);
    ok(ret == -1, "taigi_handle_PageUp() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_handle_PageDown(NULL);
    ok(ret == -1, "taigi_handle_PageDown() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_handle_Default(NULL, 0);
    ok(ret == -1, "taigi_handle_Default() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_handle_CtrlNum(NULL, 0);
    ok(ret == -1, "taigi_handle_CtrlNum() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_handle_ShiftSpace(NULL);
    ok(ret == -1, "taigi_handle_ShiftSpace() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_handle_Numlock(NULL, 0);
    ok(ret == -1, "taigi_handle_Numlock() returns `%d' shall be `%d'", ret, -1);

    phone = taigi_get_phoneSeq(NULL);
    ok(phone == NULL, "taigi_get_phoneSeq() returns NULL");
    taigi_free(phone);

    ret = taigi_get_phoneSeqLen(NULL);
    ok(ret == -1, "taigi_get_phoneSeqLen() returns `%d' shall be `%d'", ret, -1);

    taigi_set_logger(NULL, NULL, NULL);

    ret = taigi_userphrase_enumerate(NULL);
    ok(ret == -1, "taigi_userphrase_enumerate() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_userphrase_has_next(NULL, NULL, NULL);
    ok(ret == 0, "taigi_userphrase_has_next() returns `%d' shall be `%d'", ret, 0);

    ret = taigi_userphrase_get(NULL, NULL, 0, NULL, 0);
    ok(ret == -1, "taigi_userphrase_get() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_userphrase_add(NULL, NULL, NULL);
    ok(ret == -1, "taigi_userphrase_add() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_userphrase_remove(NULL, NULL, NULL);
    ok(ret == -1, "taigi_userphrase_remove() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_userphrase_lookup(NULL, NULL, NULL);
    ok(ret == 0, "taigi_userphrase_lookup() returns `%d' shall be `%d'", ret, 0);

    ret = taigi_cand_open(NULL);
    ok(ret == -1, "taigi_cand_open() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_cand_close(NULL);
    ok(ret == -1, "taigi_cand_open() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_cand_choose_by_index(NULL, 0);
    ok(ret == -1, "taigi_cand_choose_by_index() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_cand_list_first(NULL);
    ok(ret == -1, "taigi_cand_list_first() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_cand_list_last(NULL);
    ok(ret == -1, "taigi_cand_list_last() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_cand_list_has_next(NULL);
    ok(ret == 0, "taigi_cand_list_has_next() returns `%d' shall be `%d'", ret, 0);

    ret = taigi_cand_list_has_prev(NULL);
    ok(ret == 0, "taigi_cand_list_has_prev() returns `%d' shall be `%d'", ret, 0);

    ret = taigi_cand_list_next(NULL);
    ok(ret == -1, "taigi_cand_list_next() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_cand_list_prev(NULL);
    ok(ret == -1, "taigi_cand_list_prev() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_commit_preedit_buf(NULL);
    ok(ret == -1, "taigi_commit_preedit_buf() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_clean_preedit_buf(NULL);
    ok(ret == -1, "taigi_clean_preedit_buf() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_clean_bopomofo_buf(NULL);
    ok(ret == -1, "taigi_clean_bopomofo_buf() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_commit_Check(NULL);
    ok(ret == -1, "taigi_commit_Check() returns `%d' shall be `%d'", ret, -1);

    buf = taigi_commit_String(NULL);
    ok(strcmp(buf, "") == 0, "taigi_commit_String() returns `%s' shall be `%s'", buf, "");
    taigi_free(buf);

    const_buf = taigi_commit_String_static(NULL);
    ok(strcmp(const_buf, "") == 0, "taigi_commit_String() returns `%s' shall be `%s'", const_buf, "");

    ret = taigi_buffer_Check(NULL);
    ok(ret == -1, "taigi_buffer_Check() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_buffer_Len(NULL);
    ok(ret == -1, "taigi_buffer_Len() returns `%d' shall be `%d'", ret, -1);

    buf = taigi_buffer_String(NULL);
    ok(strcmp(buf, "") == 0, "taigi_buffer_String() returns `%s' shall be `%s'", buf, "");
    taigi_free(buf);

    const_buf = taigi_buffer_String_static(NULL);
    ok(strcmp(const_buf, "") == 0, "taigi_buffer_String_static() returns `%s' shall be `%s'", const_buf, "");

    const_buf = taigi_bopomofo_String_static(NULL);
    ok(strcmp(const_buf, "") == 0, "taigi_bopomofo_String_static() returns `%s' shall be `%s'", const_buf, "");

BEGIN_IGNORE_DEPRECATIONS
    buf = taigi_zuin_String(NULL, NULL);
END_IGNORE_DEPRECATIONS
    ok(strcmp(buf, "") == 0, "taigi_zuin_String() returns `%s' shall be `%s'", buf, "");
    taigi_free(buf);

    ret = taigi_bopomofo_Check(NULL);
    ok(ret == -1, "taigi_bopomofo_Check() returns `%d' shall be `%d'", ret, -1);

BEGIN_IGNORE_DEPRECATIONS
    taigi_zuin_Check(NULL); // shall not crash
END_IGNORE_DEPRECATIONS

    ret = taigi_cursor_Current(NULL);
    ok(ret == -1, "taigi_cursor_Current() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_cand_CheckDone(NULL);
    ok(ret == -1, "taigi_cand_CheckDone() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_cand_TotalPage(NULL);
    ok(ret == -1, "taigi_cand_TotalPage() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_cand_ChoicePerPage(NULL);
    ok(ret == -1, "taigi_cand_ChoicePerPage() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_cand_TotalChoice(NULL);
    ok(ret == -1, "taigi_cand_TotalChoice() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_cand_CurrentPage(NULL);
    ok(ret == -1, "taigi_cand_CurrentPage() returns `%d' shall be `%d'", ret, -1);

    taigi_cand_Enumerate(NULL);       // shall not crash

    ret = taigi_cand_hasNext(NULL);
    ok(ret == -1, "taigi_cand_hasNext() returns `%d' shall be `%d'", ret, -1);

    const_buf = taigi_cand_String_static(NULL);
    ok(strcmp(const_buf, "") == 0, "taigi_cand_String_static() returns `%s' shall be `%s'", const_buf, "");

    buf = taigi_cand_String(NULL);
    ok(strcmp(buf, "") == 0, "taigi_cand_String() returns `%s' shall be `%s'", buf, "");
    taigi_free(buf);

    taigi_interval_Enumerate(NULL);   // shall not crash

    ret = taigi_interval_hasNext(NULL);
    ok(ret == -1, "taigi_interval_hasNext() returns `%d' shall be `%d'", ret, -1);

    taigi_interval_Get(NULL, NULL);   // shall not crash

    ret = taigi_aux_Check(NULL);
    ok(ret == -1, "taigi_aux_Check() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_aux_Length(NULL);
    ok(ret == -1, "taigi_aux_Length() returns `%d' shall be `%d'", ret, -1);

    const_buf = taigi_aux_String_static(NULL);
    ok(strcmp(const_buf, "") == 0, "taigi_aux_String_static() returns `%s' shall be `%s'", const_buf, "");

    buf = taigi_aux_String(NULL);
    ok(strcmp(buf, "") == 0, "taigi_aux_String() returns `%s' shall be `%s'", buf, "");
    taigi_free(buf);

    ret = taigi_keystroke_CheckIgnore(NULL);
    ok(ret == -1, "taigi_keystroke_CheckIgnore() returns `%d' shall be `%d'", ret, -1);

    ret = taigi_keystroke_CheckAbsorb(NULL);
    ok(ret == -1, "taigi_keystroke_CheckAbsorb() returns `%d' shall be `%d'", ret, -1);

    taigi_kbtype_Enumerate(NULL);     // shall not crash

    ret = taigi_kbtype_hasNext(NULL);
    ok(ret == -1, "taigi_kbtype_hasNext() returns `%d' shall be `%d'", ret, -1);

    const_buf = taigi_kbtype_String_static(NULL);
    ok(strcmp(const_buf, "") == 0, "taigi_kbtype_String_static() returns `%s' shall be `%s'", const_buf, "");

    buf = taigi_kbtype_String(NULL);
    ok(strcmp(buf, "") == 0, "taigi_kbtype_String() returns `%s' shall be `%s'", buf, "");
    taigi_free(buf);
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


    test_null();

    fclose(fd);

    return exit_status();
}
