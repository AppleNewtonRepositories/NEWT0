/*------------------------------------------------------------------------*/
/**
 * @file	NewtGC.c
 * @brief   �K�x�[�W�R���N�V����
 *
 * @author  M.Nukui
 * @date	2003-11-07
 *
 * Copyright (C) 2003-2004 M.Nukui All rights reserved.
 */


/* �w�b�_�t�@�C�� */
#include <memory.h>

#include "NewtGC.h"
#include "NewtObj.h"
#include "NewtMem.h"
#include "NewtEnv.h"
#include "NewtVM.h"
#include "NewtIO.h"
#include "NewtPrint.h"


/* �֐��v���g�^�C�v */
static void		NewtPoolSnap(const char * title, newtPool pool, int32_t usesize);

static void		NewtObjChain(newtObjRef * objp, newtObjRef obj);
static void		NewtPoolChain(newtPool pool, newtObjRef obj, bool literal);
static void		NewtObjFree(newtPool pool, newtObjRef obj);
static void		NewtObjChainFree(newtPool pool, newtObjRef * objp);

#if 0
static void		NewtPoolMarkClean(newtPool pool);
#endif

static void		NewtPoolSweep(newtPool pool, bool mark);

static void		NewtGCRefMark(newtRefArg r, bool mark);
static void		NewtGCRegMark(vm_reg_t * reg, bool mark);
static void		NewtGCStackMark(vm_env_t * env, bool mark);
static void		NewtGCMark(vm_env_t * env, bool mark);


#pragma mark -
/*------------------------------------------------------------------------*/
/** �������v�[���̃X�i�b�v�V���b�g���o�͂���
 *
 * @param title		[in] �^�C�g��
 * @param pool		[in] �������v�[��
 * @param usesize	[in] GC�O�̎g�p�T�C�Y
 *
 * @return			�Ȃ�
 */

void NewtPoolSnap(const char * title, newtPool pool, int32_t usesize)
{
    NewtDebugMsg(title, "mem = %d(%d)\n", pool->usesize, pool->usesize - usesize);
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�f�[�^���`�F�C������
 *
 * @param objp		[in] �`�F�C�������I�u�W�F�N�g�f�[�^�ւ̃|�C���^
 * @param obj		[in] �`�F�C������I�u�W�F�N�g�f�[�^
 *
 * @return			�Ȃ�
 *
 * @note			objp �̎Q�Ɛ悪 NULL �̏ꍇ�� obj ���Z�b�g���ĕԂ�
 */

void NewtObjChain(newtObjRef * objp, newtObjRef obj)
{
    if (*objp == NULL)
    {
        *objp = obj;
    }
    else
    {
        obj->header.nextp = *objp;
        *objp = obj;
    }
}


/*------------------------------------------------------------------------*/
/** �������v�[�����ŃI�u�W�F�N�g�f�[�^���`�F�C������
 *
 * @param pool		[in] �������v�[��
 * @param obj		[in] �`�F�C������I�u�W�F�N�g�f�[�^
 * @param literal	[in] ���e�������ǂ���
 *
 * @return			�Ȃ�
 */

void NewtPoolChain(newtPool pool, newtObjRef obj, bool literal)
{
    if (obj != NULL && pool != NULL)
    {
        if (literal)
            NewtObjChain(&pool->literal, obj);
        else
            NewtObjChain(&pool->obj, obj);
    }
}


/*------------------------------------------------------------------------*/
/** GC���K�v���`�F�b�N����
 *
 * @param pool		[in] �������v�[��
 * @param size		[in] �ǉ��T�C�Y
 *
 * @return			�Ȃ�
 */

void NewtCheckGC(newtPool pool, size_t size)
{
    if (pool->maxspace < pool->usesize + size)
    {
        NEWT_NEEDGC = true;
    }
}


/*------------------------------------------------------------------------*/
/** �������v�[�����ŃI�u�W�F�N�g���������m�ۂ��ă`�F�C������
 *
 * @param pool		[in] �������v�[��
 * @param size		[in] �I�u�W�F�N�g�T�C�Y
 * @param dataSize	[in] �f�[�^�T�C�Y
 *
 * @return			�I�u�W�F�N�g�f�[�^
 */

newtObjRef NewtObjChainAlloc(newtPool pool, size_t size, size_t dataSize)
{
    newtObjRef	obj;

    NewtCheckGC(pool, size + dataSize);

    obj = (newtObjRef)NewtMemAlloc(pool, size);
    if (obj == NULL) return NULL;

	memset(&obj->header, 0, sizeof(obj->header));

    if (0 < dataSize)
    {
        uint8_t *	data;
    
        data = NewtMemAlloc(pool, dataSize);
    
        if (data == NULL)
        {
            NewtMemFree(obj);
            return NULL;
        }

        *((uint8_t **)(obj + 1)) = data;
    }
    else
    {
        obj->header.h = kNewtObjLiteral;
    }

    if (pool != NULL)
    {
        NewtPoolChain(pool, obj, dataSize == 0);
        pool->usesize += size + dataSize;
    }

    return obj;
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g���������
 *
 * @param pool		[in] �������v�[��
 * @param obj		[in] �I�u�W�F�N�g
 *
 * @return			�Ȃ�
 */

void NewtObjFree(newtPool pool, newtObjRef obj)
{
    uint32_t	datasize;

    if (NewtObjIsLiteral(obj))
    {
        datasize = NewtAlign(sizeof(newtObj) + NewtObjSize(obj), 4);
    }
    else
    {
        datasize = sizeof(newtObj) + sizeof(uint8_t *);
        datasize += NewtObjCalcDataSize(NewtObjSize(obj));

        NewtMemFree(NewtObjData(obj));
    }

    NewtMemFree(obj);

    if (pool != NULL)
        pool->usesize -= datasize;
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�f�[�^�Ƀ`�F�C������Ă���S�ẴI�u�W�F�N�g�f�[�^���������
 *
 * @param pool		[in] �������v�[��
 * @param objp		[i/o]�I�u�W�F�N�g�f�[�^�ւ̃|�C���^
 *
 * @return			�Ȃ�
 */

void NewtObjChainFree(newtPool pool, newtObjRef * objp)
{
    newtObjRef	nextp;
    newtObjRef	obj;

    for (obj = *objp; obj != NULL; obj = nextp)
    {
        nextp = obj->header.nextp;
        NewtObjFree(pool, obj);
    }

    *objp = NULL;
}


/*------------------------------------------------------------------------*/
/** �������v�[���̉��
 *
 * @param pool		[in] �������v�[��
 *
 * @return			�Ȃ�
 */

void NewtPoolRelease(newtPool pool)
{
    if (pool != NULL)
    {
        int32_t		usesize;

        usesize = pool->usesize;

        NewtObjChainFree(pool, &pool->obj);
        NewtObjChainFree(pool, &pool->literal);

        if (NEWT_DEBUG)
            NewtPoolSnap("RELEASE", pool, usesize);
    }
}


#pragma mark -
#if 0
/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̃}�[�N���N���A����
 *
 * @param pool		[in] �������v�[��
 *
 * @return			�Ȃ�
 */

void NewtPoolMarkClean(newtPool pool)
{
    if (pool != NULL)
    {
        newtObjRef	nextp;
        newtObjRef	obj;
        newtObjRef *	prevp = &pool->obj;
        int32_t		usesize;

        usesize = pool->usesize;

        for (obj = pool->obj; obj != NULL; obj = nextp)
        {
            nextp = obj->header.nextp;

            if (NewtObjIsLiteral(obj))
            {
                NewtObjChain(&pool->literal, obj);
                *prevp = nextp;

                continue;
            }

            obj->header.h &= ~ (uint32_t)kNewtObjSweep;
            prevp = &obj->header.nextp;
        }
    }
}

#endif

/*------------------------------------------------------------------------*/
/** �������v�[�����̃I�u�W�F�N�g���X�E�B�[�v�i�|���j����
 *
 * @param pool		[in] �������v�[��
 * @param mark		[in] �}�[�N�t���O
 *
 * @return			�Ȃ�
 */

void NewtPoolSweep(newtPool pool, bool mark)
{
    if (pool != NULL)
    {
        newtObjRef	nextp;
        newtObjRef	obj;
        newtObjRef *	prevp = &pool->obj;
        int32_t		usesize;

        usesize = pool->usesize;

        for (obj = pool->obj; obj != NULL; obj = nextp)
        {
            nextp = obj->header.nextp;

            if (NewtObjIsLiteral(obj))
            {
                NewtObjChain(&pool->literal, obj);
                *prevp = nextp;

                continue;
            }

            if (NewtObjIsSweep(obj, mark))
            {
                *prevp = nextp;
                NewtObjFree(pool, obj);

                continue;
            }

            prevp = &obj->header.nextp;
        }

        if (NEWT_DEBUG)
            NewtPoolSnap("GC", pool, usesize);
    }

    if (pool->maxspace < pool->usesize)
    {
        pool->maxspace = ((pool->usesize + pool->expandspace - 1) / pool->expandspace)
                            * pool->expandspace;
    }
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g���}�[�N����
 *
 * @param r			[in] �I�u�W�F�N�g
 * @param mark		[in] �}�[�N�t���O
 *
 * @return			�Ȃ�
 */

void NewtGCRefMark(newtRefArg r, bool mark)
{
    if (NewtRefIsPointer(r))
    {
        newtObjRef	obj;
    
        obj = NewtRefToPointer(r);
    
        if (! NewtObjIsLiteral(obj) && NewtObjIsSweep(obj, mark))
        {
            if (mark)
                obj->header.h &= ~ (uint32_t)kNewtObjSweep;
            else
                obj->header.h |= kNewtObjSweep;

            if (NewtObjIsSlotted(obj))
            {
                newtRef *	slots;
                uint32_t	len;
                uint32_t	i;
    
                len = NewtObjSlotsLength(obj);
                slots = NewtObjToSlots(obj);
    
                for (i = 0; i < len; i++)
                {
                    NewtGCRefMark(slots[i], mark);
                }
            }
    
            if (NewtObjIsFrame(obj))
                NewtGCRefMark(obj->as.map, mark);
        }
    }
}


/*------------------------------------------------------------------------*/
/** ���W�X�^���̃I�u�W�F�N�g���}�[�N����
 *
 * @param reg		[in] ���W�X�^
 * @param mark		[in] �}�[�N�t���O
 *
 * @return			�Ȃ�
 */

void NewtGCRegMark(vm_reg_t * reg, bool mark)
{
    NewtGCRefMark(reg->func, mark);
    NewtGCRefMark(reg->locals, mark);
    NewtGCRefMark(reg->rcvr, mark);
    NewtGCRefMark(reg->impl, mark);
}


/*------------------------------------------------------------------------*/
/** �X�^�b�N���̃I�u�W�F�N�g���}�[�N����
 *
 * @param env		[in] ���s��
 * @param mark		[in] �}�[�N�t���O
 *
 * @return			�Ȃ�
 */

void NewtGCStackMark(vm_env_t * env, bool mark)
{
    newtRef *	stack;
    vm_reg_t *	callstack;
    uint32_t	i;

    // �X�^�b�N
    stack = (newtRef *)env->stack.stackp;

    for (i = 0; i < env->reg.sp; i++)
    {
        NewtGCRefMark(stack[i], mark);
    }

    // �֐��ďo���X�^�b�N
    callstack = (vm_reg_t *)env->callstack.stackp;

    for (i = 0; i < env->callstack.sp; i++)
    {
        NewtGCRegMark(&callstack[i], mark);
    }

    // ��O�n���h���E�X�^�b�N
    NewtGCRefMark(env->currexcp, mark);

/*
    {
        vm_excp_t *	excpstack;
        vm_excp_t *	excp;

        excpstack = (vm_excp_t *)env->excpstack.stackp;

        for (i = 0; i < env->excpsp; i++)
        {
            excp = &excpstack[i];
            NewtGCRefMark(excp->sym, mark);
        }
    }
*/
}


/*------------------------------------------------------------------------*/
/** �Q�Ƃ���Ă���I�u�W�F�N�g���}�[�N����
 *
 * @param env		[in] ���s��
 * @param mark		[in] �}�[�N�t���O
 *
 * @return			�Ȃ�
 */

void NewtGCMark(vm_env_t * env, bool mark)
{
    NewtGCRefMark(NcGetRoot(), mark);
//    NewtGCRefMark(NSGetGlobals(), mark);
//    NewtGCRefMark(NSGetGlobalFns(), mark);
//    NewtGCRefMark(NSGetMagicPointers(), mark);

    // ���W�X�^
    NewtGCRegMark(&env->reg, mark);

    // �X�^�b�N
    NewtGCStackMark(env, mark);
}


/*------------------------------------------------------------------------*/
/** �K�x�[�W�R���N�V�����̎��s
 *
 * @return			�Ȃ�
 */

void NewtGC(void)
{
//    NewtPoolMarkClean(NEWT_POOL);
    NewtGCMark(&vm_env, NEWT_SWEEP);
	NewtPoolSweep(NEWT_POOL, NEWT_SWEEP);

    NEWT_SWEEP = ! NEWT_SWEEP;
    NEWT_NEEDGC = false;
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �X�N���v�g���� GC�@���ďo���i���ۂɂ� GC ��\�񂷂邾���j
 *
 * @param rcvr		[in] ���V�[�o
 *
 * @return			NIL
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef	NsGC(newtRefArg rcvr)
{
	NEWT_NEEDGC = true;
    return kNewtRefNIL;
}
