/*------------------------------------------------------------------------*/
/**
 * @file	NewtVM.h
 * @brief   VM
 *
 * @author  M.Nukui
 * @date	2003-11-07
 *
 * Copyright (C) 2003-2004 M.Nukui All rights reserved.
 */


#ifndef	NEWTVM_H
#define	NEWTVM_H


/* �w�b�_�t�@�C�� */
#include <stdio.h>
#include "NewtType.h"
#include "NewtMem.h"


/* �萔 */

/// �C�e���[�^�̗v�f�ʒu
enum {
    kIterIndex		= 0,	///< �J��Ԃ����̈ʒu
    kIterValue,				///< �l
    kIterObj,				///< �I�u�W�F�N�g
    kIterDeeply,			///< deeply �t���O
    kIterPos,				///< �I�u�W�F�N�g�̈ʒu
    kIterMax,				///< �I�u�W�F�N�g�̒���
    kIterMap,				///< frame�I�u�W�F�N�g�̃}�b�v

    //
    kIterALength			///< �C�e���[�^�z��̒���
};


/// VM ���W�X�^
typedef struct {
    newtRefVar	func;	///< FUNC   ���s���̊֐��I�u�W�F�N�g
    uint32_t	pc;		///< PC     ���s���� instruction�I�u�W�F�N�g�̃C���f�b�N�X
    uint32_t	sp;		///< SP     �X�^�b�N�|�C���^
    newtRefVar	locals;	///< LOCALS ���s���̃��[�J���t���[��
    newtRefVar	rcvr;	///< RCVR   ���s���̃��V�[�o�ifor ���b�Z�[�W���M�j
    newtRefVar	impl;	///< IMPL   ���s���̃C���v�������^(for ���b�Z�[�W���M)
} vm_reg_t;


/// ��O�n���h��
typedef struct {
    uint32_t	callsp;		///< �ďo���X�^�b�N�̃X�^�b�N�|�C���^
    uint32_t	excppc;		///< ��O�n���h�����쐬�����Ƃ��̃v���O�����J�E���^

    newtRefVar	sym;		///< �V���{��
    uint32_t	pc;			///< �v���O�����J�E���^
} vm_excp_t;


/// VM ���s��
typedef struct {
    // �o�C�g�R�[�h
    uint8_t *	bc;			///< �o�C�g�R�[�h
    uint32_t	bclen;		///< �o�C�g�R�[�h�̒���

    // ���W�X�^
    vm_reg_t	reg;		///< ���W�X�^

    // �X�^�b�N
    newtStack	stack;		///< �X�^�b�N
    newtStack	callstack;	///< �֐��ďo���X�^�b�N
    newtStack	excpstack;	///< ��O�n���h���E�X�^�b�N

    // ��O
    newtRefVar	currexcp;   ///< ���݂̗�O

	// VM �Ǘ�
	uint16_t	level;		///< VM�ďo�����x��
} vm_env_t;


extern vm_env_t		vm_env; ///< VM ���s��


/* �֐��v���g�^�C�v */

#ifdef __cplusplus
extern "C" {
#endif


newtRef		NVMSelf(void);
newtRef		NVMCurrentFunction(void);
newtRef		NVMCurrentImplementor(void);
bool		NVMHasVar(newtRefArg name);
void		NVMThrowData(newtRefArg name, newtRefArg data);
void		NVMThrow(newtRefArg name, newtRefArg data);
void		NVMRethrow(void);
newtRef		NVMCurrentException(void);
void		NVMClearException(void);

bool		NVMFuncCheckNumArgs(newtRefArg fn, int16_t numArgs);

void		NVMDumpInstName(FILE * f, uint8_t a, int16_t b);
void		NVMDumpCode(FILE * f, uint8_t * bc, uint32_t len);
void		NVMDumpBC(FILE * f, newtRefArg instructions);
void		NVMDumpFn(FILE * f, newtRefArg fn);
void		NVMDumpStackTop(FILE * f, char * s);
void		NVMDumpStacks(FILE * f);

void		NVMFnCall(newtRefArg fn, int16_t numArgs);
newtRef		NVMInterpret(newtRefArg fn, newtErr * errP);
newtErr		NVMInfo(const char * name);

newtRef		NVMCall(newtRefArg fn, int16_t numArgs, newtErr * errP);
newtRef		NVMInterpretFile(const char * path, newtErr * errP);
newtRef		NVMInterpretStr(const char * s, newtErr * errP);

newtRef		NVMMessageSendWithArgArray(newtRefArg inImpl, newtRefArg inRcvr,
				newtRefArg inFunction, newtRefArg inArgs);

#ifdef __cplusplus
}
#endif


#endif /* NEWTVM_H */
