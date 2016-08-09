/**
 * compat.c
 *
 * Copyright (c) 2014
 *      libchewing Core Team. See ChangeLog for details.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file.
 */

#ifdef HAVE_CONFIG_H
#    include <config.h>
#endif

#include <string.h>

#include "chewing-utf8-util.h"

/* for compatibility */

#include "chewing.h"

CHEWING_API int taigi_zuin_Check(const ChewingContext *ctx)
{
    return !taigi_bopomofo_Check(ctx);
}

CHEWING_API char *taigi_zuin_String(const ChewingContext *ctx, int *bopomofo_count)
{
    char *s = strdup(taigi_bopomofo_String_static(ctx));

    if (bopomofo_count) {
        *bopomofo_count = ueStrLen(s);
    }

    return s;
}

CHEWING_API int taigi_Init(const char *dataPath UNUSED, const char *hashPath UNUSED)
{
    return 0;
}

CHEWING_API void taigi_Terminate()
{
}

CHEWING_API int taigi_Configure(ChewingContext *ctx, ChewingConfigData * pcd)
{
    taigi_set_candPerPage(ctx, pcd->candPerPage);
    taigi_set_maxChiSymbolLen(ctx, pcd->maxChiSymbolLen);
    taigi_set_selKey(ctx, pcd->selKey, MAX_SELKEY);
    taigi_set_addPhraseDirection(ctx, pcd->bAddPhraseForward);
    taigi_set_spaceAsSelection(ctx, pcd->bSpaceAsSelection);
    taigi_set_escCleanAllBuf(ctx, pcd->bEscCleanAllBuf);
    taigi_set_autoShiftCur(ctx, pcd->bAutoShiftCur);
    taigi_set_easySymbolInput(ctx, pcd->bEasySymbolInput);
    taigi_set_phraseChoiceRearward(ctx, pcd->bPhraseChoiceRearward);

    return 0;
}

CHEWING_API void taigi_set_hsuSelKeyType(ChewingContext *ctx UNUSED, int mode UNUSED)
{
}

CHEWING_API int taigi_get_hsuSelKeyType(ChewingContext *ctx UNUSED)
{
    return 0;
}
