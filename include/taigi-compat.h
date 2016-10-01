/*
 * taigi-compat.h
 *
 * Copyright (c) 2014
 *      libchewing Core Team. See ChangeLog for details.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file.
 */

/* *INDENT-OFF* */
#ifndef _CHEWING_COMPAT_
#define _CHEWING_COMPAT_
/* *INDENT-ON* */

/** @brief indicate the internal encoding of data processing.
 *  @since 0.3.0
 */
#define LIBCHEWING_ENCODING "UTF-8"

/* deprecated function. for API compatibility */
CHEWING_API int taigi_zuin_Check(const ChewingContext *ctx)
    DEPRECATED_FOR(taigi_bopomofo_Check);
CHEWING_API char *taigi_zuin_String(const ChewingContext *, int *zuin_count)
    DEPRECATED_FOR(taigi_bopomofo_String_static);

CHEWING_API int taigi_Init(const char *dataPath, const char *hashPath)
    DEPRECATED;
CHEWING_API void taigi_Terminate() DEPRECATED;
CHEWING_API int taigi_Configure(ChewingContext *ctx, ChewingConfigData * pcd)
    DEPRECATED_FOR(taigi_set_*);
CHEWING_API void taigi_set_hsuSelKeyType(ChewingContext *ctx, int mode)
    DEPRECATED_FOR(taigi_set_selKey);
CHEWING_API int taigi_get_hsuSelKeyType(ChewingContext *ctx)
    DEPRECATED_FOR(taigi_get_selKey);

/* *INDENT-OFF* */
#endif
/* *INDENT-ON* */
