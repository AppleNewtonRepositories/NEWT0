/*------------------------------------------------------------------------*/
/**
 * @file	NewtMem.h
 * @brief   �������Ǘ�
 *
 * @author  M.Nukui
 * @date	2003-11-07
 *
 * Copyright (C) 2003-2004 M.Nukui All rights reserved.
 */


#ifndef	NEWTMEM_H
#define	NEWTMEM_H


/* �w�b�_�t�@�C�� */
#include "NewtType.h"


/* �^�錾 */

/// �������v�[��
typedef struct {
    void *		pool;			///< ���������v�[���ւ̃|�C���^

    int32_t		usesize;		///< �g�p�T�C�Y
    int32_t		maxspace;		///< ���݂̍ő�T�C�Y
    int32_t		expandspace;	///< ��x�Ɋg���ł��郁�����T�C�Y

    newtObjRef	obj;			///< �m�ۂ����I�u�W�F�N�g�ւ̃`�F�C���iGC �̑Ώہj
    newtObjRef	literal;		///< �m�ۂ������e�����ւ̃`�F�C��
} newtpool_t;

typedef newtpool_t *	newtPool;   ///< �������v�[���ւ̃|�C���^


/// �X�^�b�N
typedef struct {
    newtPool	pool;		///< �������v�[��

    void *		stackp;		///< �X�^�b�N�i�z�񃁃����j�ւ̃|�C���^
    uint32_t	sp;			///< �X�^�b�N�|�C���^

    uint32_t	datasize;	///< �f�[�^�T�C�Y
    uint32_t	nums;		///< �������m�ۂ���Ă���f�[�^��
    uint32_t	blocksize;	///< ���������ꊇ�m�ۂ���f�[�^��
} newtStack;


/* �֐��v���g�^�C�v */

#ifdef __cplusplus
extern "C" {
#endif


newtPool	NewtPoolAlloc(int32_t expandspace);

void *		NewtMemAlloc(newtPool pool, size_t size);
void *		NewtMemCalloc(newtPool pool, size_t number, size_t size);
void *		NewtMemRealloc(newtPool pool, void * ptr, size_t size);
void		NewtMemFree(void * ptr);

void		NewtStackSetup(newtStack * stackinfo,
                    newtPool pool, uint32_t datasize, uint32_t blocksize);

bool		NewtStackExpand(newtStack * stackinfo, uint32_t n);
void		NewtStackSlim(newtStack * stackinfo, uint32_t n);
void		NewtStackFree(newtStack * stackinfo);

uint32_t	NewtAlign(uint32_t n, uint16_t byte);


#ifdef __cplusplus
}
#endif


#endif /* NEWTMEM_H */

