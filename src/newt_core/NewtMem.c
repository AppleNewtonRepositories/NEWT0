/*------------------------------------------------------------------------*/
/**
 * @file	NewtMem.c
 * @brief   �������Ǘ�
 *
 * @author  M.Nukui
 * @date	2003-11-07
 *
 * Copyright (C) 2003-2004 M.Nukui All rights reserved.
 */


/* �w�b�_�t�@�C�� */
#include <string.h>
#include "NewtMem.h"


/*------------------------------------------------------------------------*/
/** �������v�[���̊m��
 *
 * @param expandspace   [in] �u���b�N�̊g���T�C�Y
 *
 * @return				�������v�[��
 */

newtPool NewtPoolAlloc(int32_t expandspace)
{
    newtPool	pool;

    pool = (newtPool)calloc(1, sizeof(newtpool_t));

    if (pool != NULL)
    {
        pool->maxspace = pool->expandspace = expandspace;
    }

    return pool;
}


/*------------------------------------------------------------------------*/
/** �������v�[������w�肳�ꂽ�T�C�Y�̃��������m�ۂ���
 *
 * @param pool		[in] �������v�[��
 * @param size		[in] �f�[�^�T�C�Y
 *
 * @return			�m�ۂ����������ւ̃|�C���^
 *
 * @note			���݂͂܂��Ǝ��������Ǘ��͍s���Ă��Ȃ�
 */

void * NewtMemAlloc(newtPool pool, size_t size)
{
    if (pool != NULL)
    {
    }

    return malloc(size);
}


/*------------------------------------------------------------------------*/
/** �������v�[������w�肳�ꂽ�T�C�Y�̃��������m�ۂ���
 *
 * @param pool		[in] �������v�[��
 * @param number	[in] �f�[�^��
 * @param size		[in] �f�[�^�T�C�Y
 *
 * @return			�m�ۂ����������ւ̃|�C���^
 *
 * @note			���݂͂܂��Ǝ��������Ǘ��͍s���Ă��Ȃ�
 */

void * NewtMemCalloc(newtPool pool, size_t number, size_t size)
{
    if (pool != NULL)
    {
    }

    return calloc(number, size);
}


/*------------------------------------------------------------------------*/
/** �������v�[������w�肳�ꂽ�T�C�Y�̃��������Ċm�ۂ���
 *
 * @param pool		[in] �������v�[��
 * @param ptr		[in] �������ւ̃|�C���^
 * @param size		[in] �f�[�^�T�C�Y
 *
 * @return			�Ċm�ۂ����������ւ̃|�C���^
 *
 * @note			���݂͂܂��Ǝ��������Ǘ��͍s���Ă��Ȃ�
 */

void * NewtMemRealloc(newtPool pool, void * ptr, size_t size)
{
    if (pool != NULL)
    {
    }

    return realloc(ptr, size);
}


/*------------------------------------------------------------------------*/
/** ���������������
 *
 * @param ptr		[in] �������ւ̃|�C���^
 *
 * @return			�Ȃ�
 *
 * @note			���݂͂܂��Ǝ��������Ǘ��͍s���Ă��Ȃ�
 */

void NewtMemFree(void * ptr)
{
    free(ptr);
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �X�^�b�N�����Z�b�g�A�b�v
 *
 * @param stackinfo	[out]�X�^�b�N���
 * @param pool		[in] �������v�[��
 * @param datasize	[in] �f�[�^�T�C�Y
 * @param blocksize	[in] �u���b�N�T�C�Y
 *
 * @return			�Ȃ�
 */

void NewtStackSetup(newtStack * stackinfo,
        newtPool pool, uint32_t datasize, uint32_t blocksize)
{
    memset(stackinfo, 0, sizeof(newtStack));

    stackinfo->pool = pool;
    stackinfo->datasize = datasize;
    stackinfo->blocksize = blocksize;
}


/*------------------------------------------------------------------------*/
/** �X�^�b�N�����
 *
 * @param stackinfo	[in] �X�^�b�N���
 *
 * @return			�Ȃ�
 */

void NewtStackFree(newtStack * stackinfo)
{
    if (stackinfo->stackp != NULL)
    {
        NewtMemFree(stackinfo->stackp);
        stackinfo->stackp = NULL;
    }

    stackinfo->nums = 0;
    stackinfo->sp = 0;
}


/*------------------------------------------------------------------------*/
/** �X�^�b�N���g��
 *
 * @param stackinfo	[in] �X�^�b�N���
 * @param n			[in] �K�v�Ƃ���Ă���X�^�b�N��
 *
 * @retval			true	�K�v�Ȃ����m�ۂ���Ă���
 * @retval			false   �m�ۂł��Ȃ�����
 */

bool NewtStackExpand(newtStack * stackinfo, uint32_t n)
{
    if (stackinfo->nums < n)
    {
        uint32_t	newsize;
        uint32_t	nums;
        void *		newp;
    
        nums = stackinfo->nums + stackinfo->blocksize;
        newsize = stackinfo->datasize * nums;

        newp = NewtMemRealloc(stackinfo->pool, stackinfo->stackp, newsize);

        if (newp == NULL) return false;
    
        stackinfo->stackp = newp;
        stackinfo->nums = nums;
    }

    return true;
}


/*------------------------------------------------------------------------*/
/** �X�^�b�N���X������
 *
 * @param stackinfo	[in] �X�^�b�N���
 * @param n			[in] �K�v�Ƃ���Ă���X�^�b�N��
 *
 * @return			�Ȃ�
 */

void NewtStackSlim(newtStack * stackinfo, uint32_t n)
{
    if (0 < n)
    {
        uint32_t	newsize;
		void *		newp;

        newsize = stackinfo->datasize * n;
		newp = NewtMemRealloc(stackinfo->pool, stackinfo->stackp, newsize);

		if (newp != NULL)
		{
			stackinfo->stackp = newp;
			stackinfo->nums = n;
		}
    }
    else
    {
        NewtStackFree(stackinfo);
    }
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �A���C�����v�Z
 *
 * @param n			[in] �A���C������l
 * @param byte		[in] �A���C�������P��
 *
 * @return			�A���C�����ꂽ�l
 */

uint32_t NewtAlign(uint32_t n, uint16_t byte)
{
    uint32_t	mod;

    mod = n % byte;
    if (0 < mod) n += byte - mod;

    return n;
}
