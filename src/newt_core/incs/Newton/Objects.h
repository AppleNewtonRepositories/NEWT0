/*------------------------------------------------------------------------*/
/**
 * @file	Objects.h
 * @brief   ��{�I�u�W�F�N�g�C���^�t�F�[�X
 *			�iNewton C++ Tools / Newton.framework �݊��p�j
 *
 * @author  M.Nukui
 * @date	2005-03-10
 *
 * Copyright (C) 2005 M.Nukui All rights reserved.
 */

#ifndef __OBJECTS_H
#define __OBJECTS_H


/* �w�b�_�t�@�C�� */
#include "Newton/Newton.h"


/* �^��` */
typedef newtRef					Ref;
typedef newtRefVar				RefVar;
typedef newtRefArg				RefArg;


/* �}�N�� */
#define	MAKEINT(i)				NewtMakeInteger(i)
#define	MAKECHAR(c)				NewtMakeCharacter(c)
#define	MAKEBOOLEAN(b)			NewtMakeBoolean(b)
#define	MAKEPTR(p)				NewtMakePointer(p)
#define	MAKEMAGICPTR(index)		NewtMakeMagicPointer(index)

#define	NILREF					kNewtRefNIL
#define	TRUEREF					kNewtRefTRUE
#define	FALSEREF				NILREF
#define	INVALIDPTRREF			NewtMakeInt30(0)

// �c��͖������A��`���邱��


#endif	/* __OBJECTS_H */
