/*------------------------------------------------------------------------*/
/**
 * @file	NewtGC.h
 * @brief   �K�x�[�W�R���N�V����
 *
 * @author  M.Nukui
 * @date	2003-11-07
 *
 * Copyright (C) 2003-2004 M.Nukui All rights reserved.
 */


#ifndef	NEWTGC_H
#define	NEWTGC_H

/* �w�b�_�t�@�C�� */
#include "NewtMem.h"


/* �}�N�� */
#define NewtGCHint(r, hint)									///< GC �������ǂ��s�����߂̃q���g��^����


/* �֐��v���g�^�C�v */

#ifdef __cplusplus
extern "C" {
#endif


void		NewtCheckGC(newtPool pool, size_t size);
newtObjRef	NewtObjChainAlloc(newtPool pool, size_t size, size_t dataSize);
void		NewtPoolRelease(newtPool pool);

void		NewtGC(void);

newtRef		NsGC(newtRefArg rcvr);


#ifdef __cplusplus
}
#endif


#endif /* NEWTGC_H */
