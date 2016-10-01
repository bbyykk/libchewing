/**
 * genkeystroke.c
 *
 * Copyright (c) 2004, 2005, 2015
 *      libchewing Core Team. See ChangeLog for details.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file.
 */

#ifdef HAVE_CONFIG_H
#    include <config.h>
#endif

#include "taigi.h"

/* Only used by calculating char position */
#include "internal/taigi-utf8-util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined HAVE_NCURSESW_CURSES_H
#    include <ncursesw/curses.h>
#elif defined HAVE_NCURSESW_H
#    include <ncursesw.h>
#elif defined HAVE_NCURSES_CURSES_H
#    include <ncurses/curses.h>
#elif defined HAVE_NCURSES_H
#    include <ncurses.h>
#elif defined HAVE_CURSES_H
#    include <curses.h>
#else
#    error "SysV or X/Open-compatible Curses header file required"
#endif
#include <locale.h>

/* Avoid incorrect KEY_ENTER definition */
#ifdef KEY_ENTER
#    undef KEY_ENTER
#endif

/* Key list */
#define KEY_ENTER	'\n'
#define KEY_TAB		'\t'
#define KEY_ESC		(27)
#define KEY_CTRL_A	(1)
#define KEY_CTRL_(n)	(KEY_CTRL_A + (n) - 'A')
#define KEY_SPACE	' '

#define CTRL_0		KEY_F0
#define CTRL_1		KEY_F(1)
#define CTRL_2		KEY_F(2)
#define CTRL_3		KEY_F(3)
#define CTRL_4		KEY_F(4)
#define CTRL_5		KEY_F(5)
#define CTRL_6		KEY_F(6)
#define CTRL_7		KEY_F(7)
#define CTRL_8		KEY_F(8)
#define CTRL_9		KEY_F(9)

/* Spacing */
#define FILL_LINE  "--------------------------------------------------------"
#define FILL_BLANK "                                                               "

#define LOGNAME "genkeystroke.log"

static int hasColor = 0;
static int selKey_define[11] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 0 }; /* Default */

void drawline(int x, int y)
{
    move(x, y);
    addstr(FILL_LINE);
}

void show_edit_buffer(int x, int y, ChewingContext *ctx)
{
    int i, cursor, count;
    const char *buffer_string;
    const char *p;

    move(x, y);
    addstr(FILL_BLANK);
    if (!taigi_buffer_Check(ctx)) {
        move(x, y);
        return;
    }
    buffer_string = taigi_buffer_String_static(ctx);
    mvaddstr(x, y, buffer_string);
    cursor = taigi_cursor_Current(ctx);
    p = buffer_string;
    count = 0;
    for (i = 0; i < cursor; i++) {
        count += ueBytesFromChar(*p) <= 1 ? 1 : 2;
        p += ueBytesFromChar(*p);
    }
    move(x, count);
}

void show_interval_buffer(int x, int y, ChewingContext *ctx)
{
    const char *buf;
    const char *p;
    int buf_len;
    char out_buf[100];
    int i, count;
    int arrPos[50];
    IntervalType it;

    move(x, y);
    addstr(FILL_BLANK);
    move(x, y);

    /* Check if buffer is available. */
    if (!taigi_buffer_Check(ctx)) {
        return;
    }

    buf = taigi_buffer_String_static(ctx);
    buf_len = taigi_buffer_Len(ctx);

    p = buf;
    count = 0;
    for (i = 0; i < buf_len; i++) {
        arrPos[i] = count;
        count += ueBytesFromChar(*p) <= 1 ? 1 : 2;
        p += ueBytesFromChar(*p);
    }
    arrPos[i] = count;

    memset(out_buf, ' ', count * (sizeof(char)));
    out_buf[count] = '\0';

    taigi_interval_Enumerate(ctx);
    while (taigi_interval_hasNext(ctx)) {
        taigi_interval_Get(ctx, &it);
        out_buf[arrPos[it.from]] = '[';
        out_buf[arrPos[it.to] - 1] = ']';
        memset(&out_buf[arrPos[it.from] + 1], '-', arrPos[it.to] - arrPos[it.from] - 2);
    }
    addstr(out_buf);
}

void showBopomofo(ChewingContext *ctx)
{
    if (taigi_get_ChiEngMode(ctx))
        addstr("[中]");
    else
        addstr("[英]");
    addstr("        ");
    addstr(taigi_bopomofo_String_static(ctx));
}

void show_bopomofo_buffer(int x, int y, ChewingContext *ctx)
{
    move(x, y);
    addstr(FILL_BLANK);
    move(x, y);
    if (hasColor)
        attron(COLOR_PAIR(1));
    showBopomofo(ctx);
    if (hasColor)
        attroff(COLOR_PAIR(1));
}

void show_full_shape(int x, int y, ChewingContext *ctx)
{
    move(x, y);
    addstr("[");
    if (hasColor)
        attron(COLOR_PAIR(2));
    if (taigi_get_ShapeMode(ctx) == FULLSHAPE_MODE)
        addstr("全形");
    else
        addstr("半形");
    if (hasColor)
        attroff(COLOR_PAIR(2));
    addstr("]");
}

void show_userphrase(int x, int y, ChewingContext *ctx)
{
    const char *aux_string;

    if (taigi_aux_Length(ctx) == 0)
        return;

    move(x, y);
    addstr(FILL_BLANK);
    move(x, y);
    if (hasColor)
        attron(COLOR_PAIR(2));
    aux_string = taigi_aux_String_static(ctx);
    addstr(aux_string);
    if (hasColor)
        attroff(COLOR_PAIR(2));
}

void show_choose_buffer(int x, int y, ChewingContext *ctx)
{
    int i = 1;
    int currentPageNo;
    char str[20];
    const char *cand_string;

    move(x, y);
    addstr(FILL_BLANK);
    move(x, y);

    if (taigi_cand_TotalPage(ctx) == 0)
        return;

    taigi_cand_Enumerate(ctx);
    while (taigi_cand_hasNext(ctx)) {
        if (i > taigi_cand_ChoicePerPage(ctx))
            break;
        snprintf(str, sizeof(str), "%d.", i);
        if (hasColor)
            attron(COLOR_PAIR(3));
        addstr(str);
        if (hasColor)
            attroff(COLOR_PAIR(3));
        cand_string = taigi_cand_String_static(ctx);
        addstr(cand_string);
        i++;
    }
    currentPageNo = taigi_cand_CurrentPage(ctx);
    if (taigi_cand_TotalPage(ctx) != 1) {
        if (currentPageNo == 0)
            addstr("  >");
        else if (currentPageNo == (taigi_cand_TotalPage(ctx) - 1))
            addstr("<  ");
        else
            addstr("< >");
    }
}

void show_commit_string(int x, int y, ChewingContext *ctx)
{
    const char *commit_string;

#if 0
    if (pgo->keystrokeRtn & KEYSTROKE_COMMIT) {
        for (i = 0; i < pgo->CommitBufLen; i++) {
            mvaddstr(x, y, (const char *) pgo->commitStr[i].s);
            y = (y >= 54) ? 0 : (y + strlen((const char *) pgo->commitStr[i].s) - 3 < 0 ? y + 1 : y + 2);
            x = (y == 0) ? (x + 1) : x;
        }
    }
#endif
    if (taigi_commit_Check(ctx)) {
        commit_string = taigi_commit_String_static(ctx);
        mvaddstr(x, y, FILL_BLANK);
        mvaddstr(x, y, commit_string);
    }
}

static void logger(void *data, int level, const char *fmt, ...)
{
    va_list ap;
    FILE *fd = (FILE *) data;

    va_start(ap, fmt);
    vfprintf(fd, fmt, ap);
    va_end(ap);
}

int main(int argc, char *argv[])
{
    ChewingContext *ctx = NULL;
    FILE *fout = NULL;
    FILE *log = NULL;
    int ch;
    int add_phrase_length;

    if (argc < 2) {
        fprintf(stderr, "usage: genkeystroke filename\n");
        exit(1);
    } else {
        fout = fopen(argv[1], "w");
        if (!fout) {
            fprintf(stderr, "Error: failed to open %s\n", argv[1]);
            exit(1);
        }
    }

    log = fopen(LOGNAME, "w");
    if (!log) {
        fprintf(stderr, "Error: failed to open %s\n", LOGNAME);
        goto end;
    }

    /* Initialize curses library */
    setlocale(LC_CTYPE, "");
    initscr();
    if (has_colors() == TRUE) {
        start_color();
        init_pair(1, COLOR_WHITE, COLOR_BLUE);
        init_pair(2, COLOR_RED, COLOR_YELLOW);
        init_pair(3, COLOR_WHITE, COLOR_RED);
        hasColor = 1;
    }
    cbreak();
    noecho();
    keypad(stdscr, 1);
    start_color();
    clear();
    refresh();

    /* Initialize libchewing */
    putenv("CHEWING_PATH=" CHEWING_DATA_PREFIX);
    /* for the sake of testing, we should not change existing hash data */
    putenv("CHEWING_USER_PATH=" TEST_HASH_DIR);

    /* Request handle to ChewingContext */
    ctx = taigi_new2(NULL, NULL, logger, log);

    /* Set keyboard type */
    taigi_set_KBType(ctx, taigi_KBStr2Num("KB_DEFAULT"));

    /* Fill configuration values */
    taigi_set_candPerPage(ctx, 9);
    taigi_set_maxChiSymbolLen(ctx, 16);
    taigi_set_addPhraseDirection(ctx, 1);
    taigi_set_selKey(ctx, selKey_define, 10);
    taigi_set_spaceAsSelection(ctx, 1);
    taigi_set_autoShiftCur(ctx, 1);
    taigi_set_phraseChoiceRearward(ctx, 1);

    clear();
    mvaddstr(0, 0, "Any key to start testing...");

    while (TRUE) {
        ch = getch();
        switch (ch) {
        case KEY_LEFT:
            taigi_handle_Left(ctx);
            fprintf(fout, "<L>");
            break;
        case KEY_SLEFT:
            taigi_handle_ShiftLeft(ctx);
            fprintf(fout, "<SL>");
            break;
        case KEY_RIGHT:
            taigi_handle_Right(ctx);
            fprintf(fout, "<R>");
            break;
        case KEY_SRIGHT:
            taigi_handle_ShiftRight(ctx);
            fprintf(fout, "<SR>");
            break;
        case KEY_UP:
            taigi_handle_Up(ctx);
            fprintf(fout, "<U>");
            break;
        case KEY_DOWN:
            taigi_handle_Down(ctx);
            fprintf(fout, "<D>");
            break;
        case KEY_SPACE:
            taigi_handle_Space(ctx);
            fprintf(fout, " ");
            break;
        case KEY_ENTER:
            taigi_handle_Enter(ctx);
            fprintf(fout, "<E>");
            break;
        case KEY_BACKSPACE:
            taigi_handle_Backspace(ctx);
            fprintf(fout, "<B>");
            break;
        case KEY_ESC:
            taigi_handle_Esc(ctx);
            fprintf(fout, "<EE>");
            break;
        case KEY_DC:
            taigi_handle_Del(ctx);
            fprintf(fout, "<DC>");
            break;
        case KEY_HOME:
            taigi_handle_Home(ctx);
            fprintf(fout, "<H>");
            break;
        case KEY_END:
            taigi_handle_End(ctx);
            fprintf(fout, "<EN>");
            break;
        case KEY_TAB:
            taigi_handle_Tab(ctx);
            fprintf(fout, "<T>");
            break;
        case CTRL_0:
        case CTRL_1:
        case CTRL_2:
        case CTRL_3:
        case CTRL_4:
        case CTRL_5:
        case CTRL_6:
        case CTRL_7:
        case CTRL_8:
        case CTRL_9:
            add_phrase_length = (ch - CTRL_0 + '0');
            taigi_handle_CtrlNum(ctx, add_phrase_length);
            fprintf(fout, "<C%c>", add_phrase_length);
            break;
        case KEY_CTRL_('B'):   /* emulate CapsLock */
            taigi_handle_Capslock(ctx);
            fprintf(fout, "<CB>");
            break;
        case KEY_CTRL_('D'):
            goto end;
        case KEY_CTRL_('H'):   /* emulate Shift */
            if (taigi_get_ShapeMode(ctx) == FULLSHAPE_MODE)
                taigi_set_ShapeMode(ctx, HALFSHAPE_MODE);
            else
                taigi_set_ShapeMode(ctx, FULLSHAPE_MODE);
            break;
        case KEY_NPAGE:
            taigi_handle_PageDown(ctx);
            fprintf(fout, "<PD>");
            break;
        case KEY_PPAGE:
            taigi_handle_PageUp(ctx);
            fprintf(fout, "<PU>");
            break;
        default:
            taigi_handle_Default(ctx, (char) ch);
            if (ch != '<' && ch != '>')
                fprintf(fout, "%c", (char) ch);
            else
                fprintf(fout, "<%c>", (char) ch);
            break;
        }
        drawline(0, 0);
        drawline(2, 0);
        show_interval_buffer(3, 0, ctx);
        drawline(4, 0);
        show_choose_buffer(5, 0, ctx);
        drawline(6, 0);
        show_bopomofo_buffer(7, 0, ctx);
        show_full_shape(7, 5, ctx);
        drawline(8, 0);
        mvaddstr(9, 0, "Ctrl + d : leave");
        mvaddstr(9, 20, "Ctrl + b : toggle Eng/Chi mode");
        mvaddstr(10, 0, "F1, F2, F3, ..., F9 : Add user defined phrase");
        mvaddstr(11, 0, "Ctrl + h : toggle Full/Half shape mode");
        show_commit_string(12, 0, ctx);
        show_userphrase(7, 12, ctx);
        show_edit_buffer(1, 0, ctx);
    }
  end:
    endwin();

    /* Release Chewing context */
    taigi_delete(ctx);

    /* Termate Chewing services */

    fprintf(fout, "\n");
    fclose(fout);
    return 0;
}
