/*------------------------------------------------------------------------*/
/**
 * @file	NewtPrint.h
 * @brief   �v�����g�֌W
 *
 * @author  M.Nukui
 * @date	2005-04-11
 *
 * Copyright (C) 2003-2005 M.Nukui All rights reserved.
 */


#ifndef	NEWTPRINT_H
#define	NEWTPRINT_H


/* �w�b�_�t�@�C�� */
#include <stdio.h>
#include "NewtType.h"


/* �֐��v���g�^�C�v */

#ifdef __cplusplus
extern "C" {
#endif


void		NewtPrintObj(FILE * f, newtRefArg r);
void		NewtPrintObject(FILE * f, newtRefArg r);
void		NewtPrint(FILE * f, newtRefArg r);
void		NewtInfo(newtRefArg r);
void		NewtInfoGlobalFns(void);


#ifdef __cplusplus
}
#endif


#endif /* NEWTPRINT_H */

