/*------------------------------------------------------------------------*/
/**
 * @file	endian_utils.c
 * @brief   �G���f�B�A���E���[�e�B���e�B
 *
 * @author  M.Nukui
 * @date	2005-06-04
 *
 * Copyright (C) 2005 M.Nukui All rights reserved.
 */


/* �w�b�_�t�@�C�� */
#include <string.h>
#include "utils/endian_utils.h"


#pragma mark -
/*------------------------------------------------------------------------*/
/** ���������_���̃o�C�g�I�[�_�i�G���f�B�A���j��ϊ�����
 *
 * @param d			[in] ���������_��
 *
 * @return			�o�C�g�I�[�_��ϊ��������������_��
 */

double swapd(double d)
{
	uint64_t	tmp = 0;

    memcpy(&tmp, &d, sizeof(d));
	tmp = (tmp >> 32) | (tmp << 32);
	tmp = ((tmp & 0xff00ff00ff00ff00ULL) >> 8) | ((tmp & 0x00ff00ff00ff00ffULL) << 8);
	tmp = ((tmp & 0xffff0000ffff0000ULL) >> 16) | ((tmp & 0x0000ffff0000ffffULL) << 16);
    memcpy(&d, &tmp, sizeof(d));

	return d;
}
