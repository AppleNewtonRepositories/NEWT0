/*------------------------------------------------------------------------*/
/**
 * @file	NewtType.h
 * @brief   �^��`
 *
 * @author  M.Nukui
 * @date	2003-11-07
 *
 * Copyright (C) 2003-2004 M.Nukui All rights reserved.
 */


#ifndef	NEWTTYPE_H
#define	NEWTTYPE_H


/* �w�b�_�t�@�C�� */
#if defined(__BEOS__) || defined(__FREEBSD__)
	#include <inttypes.h>
#else
	#include <stdint.h>
#endif

#include <stdbool.h>
#include <stdlib.h>

#include "platform.h"
#include "NewtConf.h"


/* �}�N�� */

// Newton Refs Constant
#define	kNewtRefNIL			0x0002		///< NIL
#define	kNewtRefTRUE		0x001A		///< TRUE
#define	kNewtSymbolClass	0x55552		///< �V���{���N���X

#define	kNewtRefUnbind		0xFFF2		///< #UNDEF�i�Ǝ��@�\�j


/* �萔 */

/// �I�u�W�F�N�g�^�C�v�i�����ł̂ݎg�p�j
enum {
    kNewtUnknownType		= 0,	///< �s���ȃ^�C�v
    kNewtInt30,						///< 30bit�����i���l�j
    kNewtPointer,					///< �|�C���^�Q��
    kNewtCharacter,					///< �����i���l�j
    kNewtSpecial,					///< ����Q�Ɓi���l�j
    kNewtNil,						///< NIL�i����Q�Ɓ^���l�j
    kNewtTrue,						///< TRUE�i����Q�Ɓ^���l�j
    kNewtUnbind,					///< ����`�i����Q�Ɓ^���l�j
    kNewtMagicPointer,				///< �}�W�b�N�|�C���^�i���l�j

    //�@�|�C���^�Q��
    kNewtBinary,					///< �o�C�i���I�u�W�F�N�g
    kNewtArray,						///< �z��
    kNewtFrame,						///< �t���[��

    //�@�o�C�i���I�u�W�F�N�g
    kNewtInt32,						///< 32bit����
    kNewtReal,						///< ���������_
    kNewtSymbol,					///< �V���{��
    kNewtString						///< ������
};


/// Newton Object Constant
enum {
    kNewtObjSlotted		= 0x01,		///< �X���b�g
    kNewtObjFrame		= 0x02,		///< �t���[��

    kNewtObjLiteral		= 0x40,		///< ���e����
    kNewtObjSweep		= 0x80		///< �S�~�|���iGC�p�j
};


/// Newton Map Constant
enum {
    kNewtMapSorted		= 0x01,		///< �X���b�g
    kNewtMapProto		= 0x04		///< �v���g
};


/// Newton Streamed Object Format (NSOF)
enum {
    kNSOFImmediate			= 0,	///< ���l
    kNSOFCharacter			= 1,	///< ASCII����
    kNSOFUnicodeCharacter	= 2,	///< UNICODE����
    kNSOFBinaryObject		= 3,	///< �o�C�i���I�u�W�F�N�g
    kNSOFArray				= 4,	///< �z��
    kNSOFPlainArray			= 5,	///< �v���C���z��
    kNSOFFrame				= 6,	///< �t���[��
    kNSOFSymbol				= 7,	///< �V���{��
    kNSOFString				= 8,	///< ������
    kNSOFPrecedent			= 9,	///< �o���ς݃f�[�^
    kNSOFNIL				= 10,   ///< NIL
    kNSOFSmallRect			= 11,   ///< ��������`
    kNSOFLargeBinary		= 12,	///< �傫���o�C�i��

    kNSOFNamedMagicPointer	= 0x10,	///< ���O�t�}�W�b�N�|�C���^�i�Ǝ��@�\�j
};


/* �^�錾 */

// Ref(Integer, Pointer, Charcter, Spatial, Magic pointer)
typedef uint32_t		newtRef;		///< �I�u�W�F�N�g�Q��
typedef newtRef			newtRefVar;		///< �I�u�W�F�N�g�Q�ƕϐ�
typedef const newtRef	newtRefArg;		///< �I�u�W�F�N�g�Q�ƈ���


/// �I�u�W�F�N�g�Q��
typedef struct newtObj *	newtObjRef;

/// �I�u�W�F�N�g�w�b�_
typedef struct {
    uint32_t	h;		///< �Ǘ����
    newtObjRef	nextp;	///< ���̃I�u�W�F�N�g�ւ̃|�C���^
} newtObjHeader;


/// �I�u�W�F�N�g
typedef struct newtObj {
    newtObjHeader	header; ///< �I�u�W�F�N�g�w�b�_

	/// as
    union {
        newtRef	klass;		///< �N���X
        newtRef	map;		///< �}�b�v
    } as;
} newtObj;


/// �V���{���f�[�^
typedef struct {
    uint32_t	hash;		///< �n�b�V���l
    char		name[1];	///< �e�L�X�g
} newtSymData;

/// �V���{���f�[�^�ւ̃|�C���^
typedef newtSymData *	newtSymDataRef;


/// �G���[�R�[�h
typedef int32_t		newtErr;


#endif /* NEWTTYPE_H */

