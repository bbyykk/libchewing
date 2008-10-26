/**
 * zuin.h
 *
 * Copyright (c) 1999, 2000, 2001
 *	Lu-chuan Kung and Kang-pen Chen.
 *	All rights reserved.
 *
 * Copyright (c) 2004, 2005, 2006, 2008
 *	libchewing Core Team. See ChangeLog for details.
 *
 * See the file "COPYING" for information on usage and redistribution
 * of this file.
 */

#ifndef _ZUIN_H
#define _ZUIN_H

/* Keyboard Layout */
#define KB_TYPE_NUM 10
#define KB_DEFAULT 0
#define KB_HSU 1
#define KB_IBM 2
#define KB_GIN_YIEH 3
#define KB_ET 4
#define KB_ET26 5
#define KB_DVORAK 6
#define KB_DVORAK_HSU 7
#define KB_DACHEN_CP26 8
#define KB_HANYU_PINYIN 9

#define ZUIN_IGNORE 0
#define ZUIN_ABSORB 1
#define ZUIN_COMMIT 2
#define ZUIN_KEY_ERROR 4
#define ZUIN_ERROR 8
#define ZUIN_NO_WORD 16

#endif
