/*
 * mod_aux.h
 *
 * Copyright (c) 2005, 2008
 *      libchewing Core Team. See ChangeLog for details.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file.
 */

/* *INDENT-OFF* */
#ifndef CHEWING_MOD_AUX_H
#define CHEWING_MOD_AUX_H
/* *INDENT-ON* */

/*! \file mod_aux.h
 *  \brief Auxiliary module
 *  \author libchewing Core Team
 */

#include "global.h"

/**
 * @brief Chewing the state for input context during commit process
 * @param ctx handle to Chewing IM context
 * @retval TRUE if its currnet input state is at the "end-of-a-char"
 */
CHEWING_API int taigi_commit_Check(const ChewingContext *ctx);

/**
 * @brief Get current commit string regardless of current input state
 * @param ctx handle to Chewing IM context
 *
 * Always returns a char pointer, caller must free it.
 */
CHEWING_API char *taigi_commit_String(const ChewingContext *ctx);
CHEWING_API const char *taigi_commit_String_static(const ChewingContext *ctx);


/*! \name Preedit string buffer
 */

/*@{*/
CHEWING_API char *taigi_buffer_String(const ChewingContext *ctx);
CHEWING_API const char *taigi_buffer_String_static(const ChewingContext *ctx);
CHEWING_API int taigi_buffer_Check(const ChewingContext *ctx);
CHEWING_API int taigi_buffer_Len(const ChewingContext *ctx);

/*@}*/


/*@{*/
CHEWING_API const char *taigi_bopomofo_String_static(const ChewingContext *ctx);
CHEWING_API int taigi_bopomofo_Check(const ChewingContext *ctx);

/*@}*/

CHEWING_API int taigi_cursor_Current(const ChewingContext *ctx);

/*@{*/
CHEWING_API int taigi_cand_CheckDone(const ChewingContext *ctx);
CHEWING_API int taigi_cand_TotalPage(const ChewingContext *ctx);
CHEWING_API int taigi_cand_ChoicePerPage(const ChewingContext *ctx);
CHEWING_API int taigi_cand_TotalChoice(const ChewingContext *ctx);
CHEWING_API int taigi_cand_CurrentPage(const ChewingContext *ctx);
CHEWING_API void taigi_cand_Enumerate(ChewingContext *ctx);
CHEWING_API int taigi_cand_hasNext(ChewingContext *ctx);
CHEWING_API char *taigi_cand_String(ChewingContext *ctx);
CHEWING_API const char *taigi_cand_String_static(ChewingContext *ctx);
CHEWING_API char *taigi_cand_string_by_index(ChewingContext *ctx, int index);
CHEWING_API const char *taigi_cand_string_by_index_static(ChewingContext *ctx, int index);
CHEWING_API int taigi_cand_choose_by_index(ChewingContext *ctx, int index);
CHEWING_API int taigi_cand_open(ChewingContext *ctx);
CHEWING_API int taigi_cand_close(ChewingContext *ctx);

/*@}*/


/*@{*/
CHEWING_API void taigi_interval_Enumerate(ChewingContext *ctx);
CHEWING_API int taigi_interval_hasNext(ChewingContext *ctx);
CHEWING_API void taigi_interval_Get(ChewingContext *ctx, IntervalType * it);

/*@}*/

/*@{*/
CHEWING_API int taigi_aux_Check(const ChewingContext *ctx);
CHEWING_API int taigi_aux_Length(const ChewingContext *ctx);
CHEWING_API char *taigi_aux_String(const ChewingContext *ctx);
CHEWING_API const char *taigi_aux_String_static(const ChewingContext *ctx);

/*@}*/


/*@{*/
CHEWING_API int taigi_keystroke_CheckIgnore(const ChewingContext *ctx);
CHEWING_API int taigi_keystroke_CheckAbsorb(const ChewingContext *ctx);

/*@}*/


/*@{*/
CHEWING_API int taigi_kbtype_Total(const ChewingContext *ctx);
CHEWING_API void taigi_kbtype_Enumerate(ChewingContext *ctx);
CHEWING_API int taigi_kbtype_hasNext(ChewingContext *ctx);
CHEWING_API char *taigi_kbtype_String(ChewingContext *ctx);
CHEWING_API const char *taigi_kbtype_String_static(ChewingContext *ctx);

/*@}*/

/* *INDENT-OFF* */
#endif                          /* CHEWING_MOD_AUX_H */
/* *INDENT-ON* */
