/*------------------------------------------------------------------------*/
/**
 * @file	lookup_words.c
 * @brief   �P��̌���
 *
 * @author  M.Nukui
 * @date	2003-11-07
 *
 * Copyright (C) 2003-2004 M.Nukui All rights reserved.
 */


/* �w�b�_�t�@�C�� */
#include <string.h>
#include <stdlib.h>
#include "lookup_words.h"


/*------------------------------------------------------------------------*/
/** �P��̌���
 *
 * @param words		[in] �P��e�[�u��
 * @param len		[in] �P��e�[�u���̒���
 * @param s			[in] �������镶����
 *
 * @retval			-1�ȊO	�g�[�N��ID
 * @retval			-1		�P�ꂪ�݂���Ȃ�����
 */

int lookup_words(keyword_t words[], int len, const char * s)
{
    int	p, q, r, comp;

    p =	0;
    q =	len - 1;

    while (p <= q)
    {
        r = (p + q) / 2;
        comp = strcasecmp(s, words[r].name);

        if (comp == 0)
            return words[r].tokn;

        if (comp < 0)
            q = r - 1;
        else
            p = r + 1;
    }

    return -1;
}


/*------------------------------------------------------------------------*/
/** �P��e�[�u���̃\�[�g�p��r�֐�
 */

int lookup_membcompare(const void * a1, const void * a2)
{
	return strcasecmp(((keyword_t *)a1)->name, ((keyword_t *)a2)->name);
}


/*------------------------------------------------------------------------*/
/** �P��e�[�u���̃\�[�g
 *
 * @param words		[in] �P��e�[�u��
 * @param len		[in] �P��e�[�u���̒���
 *
 * @return			�Ȃ�
 */

void lookup_sorttable(keyword_t words[], int len)
{
	qsort((void *)words, len, sizeof(keyword_t), &lookup_membcompare);
}
