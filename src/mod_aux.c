/*
 * mod_aux.c
 *
 * Copyright (c) 2005, 2006, 2008, 2009, 2012-2014
 *      libchewing Core Team. See ChangeLog for details.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file.
 */

/**
 * @file mod_aux.c
 * @brief Auxiliary module
 */

#include <string.h>
#include <stdlib.h>

#include "global.h"
#include "taigi-private.h"
#include "bopomofo-private.h"
#include "taigiio.h"
#include "taigi-utf8-util.h"
#include "private.h"

#ifndef LOG_API_MOD_AUX
#undef LOG_API
#define LOG_API(fmt, ...) 
#endif
/**
 * @param ctx handle to Chewing IM context
 * @retval TRUE if it currnet input state is at the "end-of-a-char"
 */
CHEWING_API int taigi_commit_Check(const ChewingContext *ctx)
{
    const ChewingData *pgdata;

    if (!ctx) {
        return -1;
    }
    pgdata = ctx->data;

    LOG_API("");

    return ! !(ctx->output->keystrokeRtn & KEYSTROKE_COMMIT);
}

/**
 * @param ctx handle to Chewing IM context
 *
 * retrun current commit string, regardless current input state.
 * Always returns a char pointer, caller must free it.
 */
CHEWING_API char *taigi_commit_String(const ChewingContext *ctx)
{
    const ChewingData *pgdata;

    if (!ctx) {
        return strdup("");
    }
    pgdata = ctx->data;

    LOG_API("%s", ctx->output->commitBuf);

    char *p = strdup(ctx->output->commitBuf);

    memset(ctx->output->commitBuf, 0x0, sizeof(ctx->output->commitBuf));
    ctx->output->commitBufLen = 0;
    return p;
}

/**
 * @param ctx handle to Chewing IM context
 * retrun current commit string, regardless current input state.
 * Always returns a const char pointer, you have to clone them immediately,
 * if you need.
 */
CHEWING_API const char *taigi_commit_String_static(const ChewingContext *ctx)
{
    const ChewingData *pgdata;

    if (!ctx) {
        return "";
    }
    pgdata = ctx->data;

    LOG_API("");

    return ctx->output->commitBuf;
}

CHEWING_API int taigi_buffer_Check(const ChewingContext *ctx)
{
    const ChewingData *pgdata;

    if (!ctx) {
        return -1;
    }
    pgdata = ctx->data;

    LOG_API("");

    return (ctx->output->chiSymbolBufLen != 0);
}

CHEWING_API int taigi_buffer_Len(const ChewingContext *ctx)
{
    const ChewingData *pgdata;

    if (!ctx) {
        return -1;
    }
    pgdata = ctx->data;

    LOG_API("");

    return ctx->output->chiSymbolBufLen;
}

CHEWING_API char *taigi_buffer_String(const ChewingContext *ctx)
{
    const ChewingData *pgdata;

    if (!ctx) {
        return strdup("");
    }
    pgdata = ctx->data;

    LOG_API("%s", ctx->output->preeditBuf);

    return strdup(ctx->output->preeditBuf);
}

CHEWING_API const char *taigi_buffer_String_static(const ChewingContext *ctx)
{
    const ChewingData *pgdata;

    if (!ctx) {
        return "";
    }
    pgdata = ctx->data;

    LOG_API("%s", ctx->output->preeditBuf);

    return ctx->output->preeditBuf;
}

/**
 * @param ctx handle to Chewing IM context
 *
 * Always returns a const char pointer, you have to clone them immediately,
 * if you need.
 */
CHEWING_API const char *taigi_bopomofo_String_static(const ChewingContext *ctx)
{
    const ChewingData *pgdata;

    if (!ctx) {
        return "";
    }
    pgdata = ctx->data;

    LOG_API("");

    return ctx->output->bopomofoBuf;
}

CHEWING_API int taigi_bopomofo_Check(const ChewingContext *ctx)
{
    const ChewingData *pgdata;

    if (!ctx) {
        return -1;
    }
    pgdata = ctx->data;

    LOG_API("");

    return ctx->output->bopomofoBuf[0] != 0;
}

CHEWING_API int taigi_cursor_Current(const ChewingContext *ctx)
{
    const ChewingData *pgdata;
    int i;

    if (!ctx) {
        return -1;
    }
    pgdata = ctx->data;
    LOG_API("ctx->output->chiSymbolCursor=%d", ctx->output->chiSymbolCursor);

    return (ctx->output->chiSymbolCursor);
}

CHEWING_API int taigi_cursor_Raw(const ChewingContext *ctx)
{
    const ChewingData *pgdata;
    int i;
    int raw_cursor = 0;
    int cursor = ctx->output->chiSymbolCursor;
    PreeditBuf *preedit = ctx->data->preeditBuf;

    if (!ctx) {
        return -1;
    }
    pgdata = ctx->data;
    for (i=0; i < cursor; ++i) {
	    if (i > 0) {
		    if(preedit[i-1].type == TYPE_TAILO && preedit[i].type == TYPE_TAILO)
			raw_cursor += 1;  //Add the '-' len
	    }
	    raw_cursor += strlen(preedit[i].char_);
    }
    /* The cursor would fall on the '-' between 2 Lomaji */
    if (cursor < ctx->output->chiSymbolBufLen) {
	    if(preedit[cursor - 1].type == TYPE_TAILO &&
			    preedit[cursor].type == TYPE_TAILO) {
		    ++raw_cursor;
	    }
    }
    return raw_cursor;
}

CHEWING_API int taigi_cand_CheckDone(const ChewingContext *ctx)
{
    const ChewingData *pgdata;

    if (!ctx) {
        return -1;
    }
    pgdata = ctx->data;

    LOG_API("");

    return (!ctx->output->pci);
}

CHEWING_API int taigi_cand_TotalPage(const ChewingContext *ctx)
{
    const ChewingData *pgdata;

    if (!ctx) {
        return -1;
    }
    pgdata = ctx->data;

    LOG_API("");

    return (ctx->output->pci ? ctx->output->pci->nPage : 0);
}

CHEWING_API int taigi_cand_ChoicePerPage(const ChewingContext *ctx)
{
    const ChewingData *pgdata;

    if (!ctx) {
        return -1;
    }
    pgdata = ctx->data;

    LOG_API("");

    return (ctx->output->pci ? ctx->output->pci->nChoicePerPage : 0);
}

CHEWING_API int taigi_cand_TotalChoice(const ChewingContext *ctx)
{
    const ChewingData *pgdata;

    if (!ctx) {
        return -1;
    }
    pgdata = ctx->data;

    LOG_API("");

    return (ctx->output->pci ? ctx->output->pci->nTotalChoice : 0);
}

CHEWING_API int taigi_cand_CurrentPage(const ChewingContext *ctx)
{
    const ChewingData *pgdata;

    if (!ctx) {
        return -1;
    }
    pgdata = ctx->data;

    LOG_API("");

    return (ctx->output->pci ? ctx->output->pci->pageNo : -1);
}

CHEWING_API void taigi_cand_Enumerate(ChewingContext *ctx)
{
    ChewingData *pgdata;

    if (!ctx) {
        return;
    }
    pgdata = ctx->data;

    LOG_API("");

    ctx->cand_no = ctx->output->pci->pageNo * ctx->output->pci->nChoicePerPage;
}

CHEWING_API int taigi_cand_hasNext(ChewingContext *ctx)
{
    ChewingData *pgdata;

    if (!ctx) {
        return -1;
    }
    pgdata = ctx->data;

    LOG_API("");

    return (ctx->cand_no < ctx->output->pci->nTotalChoice);
}

CHEWING_API const char *taigi_cand_String_static(ChewingContext *ctx)
{
    ChewingData *pgdata;
    const char *s = "";

    if (!ctx) {
        return s;
    }
    pgdata = ctx->data;

    LOG_API("");

    if (taigi_cand_hasNext(ctx)) {
        s = ctx->output->pci->totalChoiceStr[ctx->cand_no];
	LOG_API("%s, Get: %s\n", __func__, s);
        ctx->cand_no++;
    }

    return s;
}

CHEWING_API char *taigi_cand_String(ChewingContext *ctx)
{
    return strdup(taigi_cand_String_static(ctx));
}

CHEWING_API void taigi_interval_Enumerate(ChewingContext *ctx)
{
    ChewingData *pgdata;

    if (!ctx) {
        return;
    }
    pgdata = ctx->data;

    LOG_API("");

    ctx->it_no = 0;
}

CHEWING_API int taigi_interval_hasNext(ChewingContext *ctx)
{
    ChewingData *pgdata;

    if (!ctx) {
        return -1;
    }
    pgdata = ctx->data;

    LOG_API("");

    return (ctx->it_no < ctx->output->nDispInterval);
}

CHEWING_API void taigi_interval_Get(ChewingContext *ctx, IntervalType * it)
{
    ChewingData *pgdata;

    if (!ctx) {
        return;
    }
    pgdata = ctx->data;

    LOG_API("");

    if (taigi_interval_hasNext(ctx)) {
        if (it) {
            it->from = ctx->output->dispInterval[ctx->it_no].from;
            it->to = ctx->output->dispInterval[ctx->it_no].to;
        }
        ctx->it_no++;
    }
}

CHEWING_API int taigi_aux_Check(const ChewingContext *ctx)
{
    const ChewingData *pgdata;

    if (!ctx) {
        return -1;
    }
    pgdata = ctx->data;

    LOG_API("");

    return (ctx->data->bShowMsg);
}

CHEWING_API int taigi_aux_Length(const ChewingContext *ctx)
{
    const ChewingData *pgdata;

    if (!ctx) {
        return -1;
    }
    pgdata = ctx->data;

    LOG_API("");

    return (ctx->data->bShowMsg ? ctx->data->showMsgLen : 0);
}

CHEWING_API const char *taigi_aux_String_static(const ChewingContext *ctx)
{
    const ChewingData *pgdata;

    if (!ctx) {
        return "";
    }
    pgdata = ctx->data;

    LOG_API("");

    return ctx->data->showMsg;
}

CHEWING_API char *taigi_aux_String(const ChewingContext *ctx)
{
    const ChewingData *pgdata;

    if (!ctx) {
        return strdup("");
    }
    pgdata = ctx->data;

    LOG_API("");

    return strdup(taigi_aux_String_static(ctx));
}

CHEWING_API int taigi_keystroke_CheckIgnore(const ChewingContext *ctx)
{
    const ChewingData *pgdata;

    if (!ctx) {
        return -1;
    }
    pgdata = ctx->data;

    LOG_API("");

    return ! !(ctx->output->keystrokeRtn & KEYSTROKE_IGNORE);
}

CHEWING_API int taigi_keystroke_CheckAbsorb(const ChewingContext *ctx)
{
    const ChewingData *pgdata;

    if (!ctx) {
        return -1;
    }
    pgdata = ctx->data;

    LOG_API("");

    return ! !(ctx->output->keystrokeRtn & KEYSTROKE_ABSORB);
}

CHEWING_API int taigi_kbtype_Total(const ChewingContext *ctx UNUSED)
{
    return KB_TYPE_NUM;
}

CHEWING_API void taigi_kbtype_Enumerate(ChewingContext *ctx)
{
    ChewingData *pgdata;

    if (!ctx) {
        return;
    }
    pgdata = ctx->data;

    LOG_API("");

    ctx->kb_no = 0;
}

CHEWING_API int taigi_kbtype_hasNext(ChewingContext *ctx)
{
    ChewingData *pgdata;

    if (!ctx) {
        return -1;
    }
    pgdata = ctx->data;

    LOG_API("");

    return ctx->kb_no < KB_TYPE_NUM;
}

extern const char *const kb_type_str[];

CHEWING_API const char *taigi_kbtype_String_static(ChewingContext *ctx)
{
    ChewingData *pgdata;
    const char *s = "";

    if (!ctx) {
        return s;
    }
    pgdata = ctx->data;

    LOG_API("");

    if (taigi_kbtype_hasNext(ctx)) {
        s = kb_type_str[ctx->kb_no];
        ctx->kb_no++;
    }

    return s;
}

CHEWING_API char *taigi_kbtype_String(ChewingContext *ctx)
{
    ChewingData *pgdata;

    if (!ctx) {
        return strdup("");
    }
    pgdata = ctx->data;

    LOG_API("");

    return strdup(taigi_kbtype_String_static(ctx));
}
