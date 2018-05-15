/*****************************************************************************
 *   type.h:  Type definition Header file for NXP LPC17xx Family 
 *   Microprocessors
 *
 *   Copyright(C) 2009, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2009.05.25  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#include <stdint.h>

#ifndef __TYPE_H__
#define __TYPE_H__

#ifndef NULL
#define NULL    ((void *)0)
#endif

#ifndef NUL
#define NUL     '0'
#endif

#if !defined(SUCCESS) && !defined(FAILURE) 
#define SUCCESS     1
#define FAILURE     !(SUCCESS)
#endif

#if !defined(TRUE) && !defined(FALSE)
enum {FALSE = 0, TRUE = !FALSE};
#endif //END_OF_!TRUE_&&_!FALSE

typedef enum {RESET = 0, SET = !RESET} FlagStatus, ITStatus;
typedef enum {DISABLE = 0, ENABLE = !DISABLE} FunctionalState;

typedef unsigned int u32;
typedef signed int i32;
typedef unsigned char u8;
typedef signed char i8;
typedef unsigned short int u16;
typedef signed short int i16;

#endif  /* __TYPE_H__ */
