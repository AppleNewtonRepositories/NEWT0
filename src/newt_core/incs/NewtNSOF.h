/*------------------------------------------------------------------------*/
/**
 * @file	NewtNSOF.h
 * @brief   Newton Streamed Object Format
 *
 * @author  M.Nukui
 * @date	2005-04-01
 *
 * Copyright (C) 2005 M.Nukui All rights reserved.
 */


#ifndef	NEWTNSOF_H
#define	NEWTNSOF_H

/* �w�b�_�t�@�C�� */
#include "NewtType.h"


/* �}�N�� */


/* �֐��v���g�^�C�v */

#ifdef __cplusplus
extern "C" {
#endif


newtRef		NsMakeNSOF(newtRefArg rcvr, newtRefArg r, newtRefArg ver);
newtRef		NsReadNSOF(newtRefArg rcvr, newtRefArg r);


#ifdef __cplusplus
}
#endif


#endif /* NEWTNSOF_H */
