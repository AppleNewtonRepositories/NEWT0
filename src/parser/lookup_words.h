/*------------------------------------------------------------------------*/
/**
 * @file	lookup_words.h
 * @brief   �P��̌���
 *
 * @author  M.Nukui
 * @date	2003-11-07
 *
 * Copyright (C) 2003-2004 M.Nukui All rights reserved.
 */


#ifndef	LOOKUP_WORDS_H
#define	LOOKUP_WORDS_H


/* �^�錾 */

/// �P��e�[�u���̗v�f�\����
typedef struct {
    char *	name;   ///< �P��
    int		tokn;   ///< �g�[�N��ID
} keyword_t;


/* �֐��v���g�^�C�v */

#ifdef __cplusplus
extern "C" {
#endif


int		lookup_words(keyword_t words[], int len, const char * s);
void	lookup_sorttable(keyword_t words[], int len);


#ifdef __cplusplus
}
#endif


#endif /* LOOKUP_WORDS_H */
