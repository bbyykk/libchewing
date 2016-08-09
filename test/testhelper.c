/**
 * testhelper.c
 *
 * Copyright (c) 2012
 *      libchewing Core Team. See ChangeLog for details.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file.
 */
#include "testhelper.h"

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chewing-private.h"
#include "chewing-utf8-util.h"
#include "key2pho-private.h"
#include "plat_path.h"
#include "userphrase-private.h"

static unsigned int test_run;
static unsigned int test_ok;

/* We cannot use designated initializer here due to Visual Studio */
BufferType COMMIT_BUFFER = {
    "commit buffer",
    taigi_commit_Check,
    0,
    0,
    taigi_commit_String,
    0,
    taigi_commit_String_static
};

BufferType PREEDIT_BUFFER = {
    "preedit buffer",
    taigi_buffer_Check,
    0,
    taigi_buffer_Len,
    taigi_buffer_String,
    0,
    taigi_buffer_String_static
};

BEGIN_IGNORE_DEPRECATIONS

BufferType BOPOMOFO_BUFFER = {
    "bopomofo buffer",
    taigi_bopomofo_Check,
    taigi_zuin_Check,
    0,
    0,
    taigi_zuin_String,
    taigi_bopomofo_String_static
};

END_IGNORE_DEPRECATIONS

BufferType AUX_BUFFER = {
    "aux buffer",
    taigi_aux_Check,
    0,
    taigi_aux_Length,
    taigi_aux_String,
    0,
    0,
};

int get_keystroke(get_char_func get_char, void *param)
{
    int ch;
    int result = END;
    int flag = 0;

    assert(get_char);

    while ((ch = get_char(param)) != END) {
        if ((ch != '<') && (flag != 1))
            return (int) ch;
        else if (ch == '>') {
            flag = 0;
            return result;
        } else {
            flag = 1;
            ch = get_char(param);
            switch (ch) {
            case '<':
            case '>':
                if (get_char(param) == '>')
                    return result = ch;
                break;
            case 'L':
                result = KEY_LEFT;
                break;
            case 'R':
                result = KEY_RIGHT;
                break;
            case 'U':
                result = KEY_UP;
                break;
            case 'D':
                if ((ch = get_char(param)) == '>')
                    return result = KEY_DOWN;
                else {
                    get_char(param);
                    return result = KEY_DELETE;
                }
                break;
            case 'E':
                if ((ch = get_char(param)) == '>')
                    return result = KEY_ENTER;
                else if (ch == 'E')
                    result = KEY_ESC;
                else
                    result = KEY_END;
                break;
            case 'C':
                if ((ch = get_char(param)) != '>') {
                    if (ch == 'B')
                        result = (KEY_CAPSLOCK);
                    else
                        result = (KEY_CTRL_BASE + ch);
                }
                break;
            case 'B':
                result = KEY_BACKSPACE;
                break;
            case 'H':
                result = KEY_HOME;
                break;
            case 'S':
                if ((ch = get_char(param)) == 'L')
                    result = KEY_SLEFT;
                else if (ch == 'R')
                    result = KEY_SRIGHT;
                else
                    result = KEY_SSPACE;
                break;
            case 'T':
                if ((ch = get_char(param)) == '>')
                    return result = KEY_TAB;
                else
                    result = KEY_DBLTAB;
                break;
            case 'P':
                if ((ch = get_char(param)) == 'D')
                    result = KEY_NPAGE;
                else
                    result = KEY_PPAGE;
                break;
            case 'N':
                ch = get_char(param);
                result = KEY_NUMPAD_BASE + ch;
                break;
            }
        }
    }
    return result = END;
}

void type_single_keystroke(ChewingContext *ctx, int ch)
{
    switch (ch) {
    case KEY_LEFT:
        taigi_handle_Left(ctx);
        break;
    case KEY_SLEFT:
        taigi_handle_ShiftLeft(ctx);
        break;
    case KEY_RIGHT:
        taigi_handle_Right(ctx);
        break;
    case KEY_SRIGHT:
        taigi_handle_ShiftRight(ctx);
        break;
    case KEY_UP:
        taigi_handle_Up(ctx);
        break;
    case KEY_DOWN:
        taigi_handle_Down(ctx);
        break;
    case KEY_SPACE:
        taigi_handle_Space(ctx);
        break;
    case KEY_ENTER:
        taigi_handle_Enter(ctx);
        break;
    case KEY_BACKSPACE:
        taigi_handle_Backspace(ctx);
        break;
    case KEY_ESC:
        taigi_handle_Esc(ctx);
        break;
    case KEY_DELETE:
        taigi_handle_Del(ctx);
        break;
    case KEY_HOME:
        taigi_handle_Home(ctx);
        break;
    case KEY_END:
        taigi_handle_End(ctx);
        break;
    case KEY_TAB:
        taigi_handle_Tab(ctx);
        break;
    case KEY_CAPSLOCK:
        taigi_handle_Capslock(ctx);
        break;
    case KEY_NPAGE:
        taigi_handle_PageDown(ctx);
        break;
    case KEY_PPAGE:
        taigi_handle_PageUp(ctx);
        break;
    case KEY_SSPACE:
        taigi_handle_ShiftSpace(ctx);
        break;
    case KEY_DBLTAB:
        taigi_handle_DblTab(ctx);
        break;
    default:
        if (KEY_CTRL_BASE <= ch && ch < KEY_NUMPAD_BASE)
            taigi_handle_CtrlNum(ctx, ch - KEY_CTRL_BASE);
        else if (KEY_NUMPAD_BASE <= ch)
            taigi_handle_Numlock(ctx, ch - KEY_NUMPAD_BASE);
        else
            taigi_handle_Default(ctx, (char) ch);
        break;
    }
}

static void type_keystroke(ChewingContext *ctx, get_char_func get_char, void *param)
{
    int ch;

    while ((ch = get_keystroke(get_char, param)) != END)
        type_single_keystroke(ctx, ch);
}

static int get_char_by_string(void *param)
{
    const char **ptr = param;
    char ch;

    assert(param);

    if (**ptr == 0) {
        return END;
    }

    ch = **ptr;
    ++*ptr;
    return ch;
}

void internal_ok(const char *file, int line, int test, const char *test_txt, const char *fmt, ...)
{
    va_list ap;

    ++test_run;
    if (test) {
        ++test_ok;
        printf("ok %u ", test_run);

        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);

        printf("\n");
    } else {
        printf("not ok %u ", test_run);

        va_start(ap, fmt);
        vprintf(fmt, ap);
        va_end(ap);

        printf("\n# %s failed in %s:%d\n", test_txt, file, line);
    }
}

void type_keystroke_by_string(ChewingContext *ctx, const char *keystroke)
{
    type_keystroke(ctx, get_char_by_string, &keystroke);
}

void internal_ok_buffer(const char *file, int line, ChewingContext *ctx, const char *expected, const BufferType *buffer)
{
    char *buf;
    const char *const_buf;
    int actual_ret;
    int expected_ret;
    int expected_len;

    assert(ctx);
    assert(expected);
    assert(buffer);

    expected_len = ueStrLen(expected);

    if (buffer->check) {
        actual_ret = buffer->check(ctx);
        expected_ret = ! !expected_len;
        internal_ok(file, line, actual_ret == expected_ret,
                    "actual_ret == expected_ret",
                    "%s check function returned `%d' shall be `%d'", buffer->name, actual_ret, expected_ret);
    }

    if (buffer->check_alt) {
        actual_ret = buffer->check_alt(ctx);
        expected_ret = !expected_len;
        internal_ok(file, line, actual_ret == expected_ret,
                    "actual_ret == expected_ret",
                    "%s check function returned `%d' shall be `%d'", buffer->name, actual_ret, expected_ret);
    }

    if (buffer->get_length) {
        actual_ret = buffer->get_length(ctx);
        expected_ret = expected_len;
        internal_ok(file, line, actual_ret == expected_ret,
                    "actual_ret == expected_ret",
                    "%s get length function returned `%d' shall be `%d'", buffer->name, actual_ret, expected_ret);
    }

    if (buffer->get_string) {
        buf = buffer->get_string(ctx);
        internal_ok(file, line, !strcmp(buf, expected), "!strcmp( buf, expected )",
                    "%s string function returned `%s' shall be `%s'", buffer->name, buf, expected);
        taigi_free(buf);
    }

    if (buffer->get_string_alt) {
        buf = buffer->get_string_alt(ctx, &actual_ret);
        expected_ret = expected_len;
        internal_ok(file, line, actual_ret == expected_ret,
                    "actual_ret == expected_ret",
                    "%s string function returned parameter `%d' shall be `%d'", buffer->name, actual_ret, expected_ret);
        internal_ok(file, line, !strcmp(buf, expected), "!strcmp( buf, expected )",
                    "%s string function returned `%s' shall be `%s'", buffer->name, buf, expected);
        taigi_free(buf);
    }

    if (buffer->get_string_static) {
        const_buf = buffer->get_string_static(ctx);
        internal_ok(file, line, !strcmp(const_buf, expected), "!strcmp( const_buf, expected )",
                    "%s string function returned `%s' shall be `%s'", buffer->name, const_buf, expected);
    }
}

void internal_ok_candidate(const char *file, int line, ChewingContext *ctx, const char *cand[], size_t cand_len)
{
    size_t i;
    char *buf;
    const char *const_buf;

    assert(ctx);

    taigi_cand_Enumerate(ctx);
    for (i = 0; i < cand_len; ++i) {
        internal_ok(file, line, taigi_cand_hasNext(ctx), __func__, "shall has next candidate");

        buf = taigi_cand_String(ctx);
        internal_ok(file, line, strcmp(buf, cand[i]) == 0, __func__, "candidate `%s' shall be `%s'", buf, cand[i]);
        taigi_free(buf);

        const_buf = taigi_cand_string_by_index_static(ctx, i);
        internal_ok(file, line, strcmp(const_buf, cand[i]) == 0, __func__,
                    "candndate `%s' shall be `%s'", const_buf, cand[i]);
    }

    internal_ok(file, line, !taigi_cand_hasNext(ctx), __func__, "shall not have next candidate");
    buf = taigi_cand_String(ctx);

    internal_ok(file, line, strcmp(buf, "") == 0, __func__, "candndate `%s' shall be `%s'", buf, "");

    const_buf = taigi_cand_string_by_index_static(ctx, i);
    internal_ok(file, line, strcmp(const_buf, "") == 0, __func__, "candndate `%s' shall be `%s'", const_buf, "");

    taigi_free(buf);
}

void internal_ok_candidate_len(const char *file, int line, ChewingContext *ctx, size_t expected_len)
{
    const char *buf;
    size_t actual_len;

    assert(ctx);

    buf = taigi_cand_string_by_index_static(ctx, 0);
    actual_len = ueStrLen(buf);
    internal_ok(file, line, actual_len == expected_len, __func__,
                "candidate length `%d' shall be `%d'", actual_len, expected_len);
}

void internal_ok_keystroke_rtn(const char *file, int line, ChewingContext *ctx, int rtn)
{
    const struct {
        int rtn;
        int (*func) (const ChewingContext *ctx);
    } TABLE[] = {
        {KEYSTROKE_IGNORE, taigi_keystroke_CheckIgnore},
        {KEYSTROKE_COMMIT, taigi_commit_Check},
        // No function to check KEYSTROKE_BELL
        {KEYSTROKE_ABSORB, taigi_keystroke_CheckAbsorb},
    };
    size_t i;
    int actual;
    int expected;

    assert(ctx);

    for (i = 0; i < ARRAY_SIZE(TABLE); ++i) {
        actual = TABLE[i].func(ctx);
        expected = ! !(rtn & TABLE[i].rtn);

        internal_ok(file, line, actual == expected, __func__, "keystroke rtn `%d' shall be `%d'", actual, expected);
    }
}

void logger(void *data, int level UNUSED, const char *fmt, ...)
{
    va_list ap;
    FILE *fd = (FILE *) data;

    va_start(ap, fmt);
    vfprintf(fd, fmt, ap);
    va_end(ap);
}

void internal_start_testcase(const char *func, ChewingContext *ctx, FILE * file)
{
    assert(func);

    printf("#\n# %s\n#\n", func);
    fprintf(file, "#\n# %s\n#\n", func);
    taigi_set_logger(ctx, logger, file);
}

int exit_status()
{
    return test_run == test_ok ? 0 : -1;
}

void clean_userphrase()
{
    if(remove(TEST_HASH_DIR PLAT_SEPARATOR DB_NAME) != 0 && errno != ENOENT)
        fprintf(stderr, "remove fails at %s:%d\n", __FILE__, __LINE__);
}
