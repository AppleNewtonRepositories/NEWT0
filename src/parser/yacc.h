/*------------------------------------------------------------------------*/
/**
 * @file	yacc.h
 * @brief   yacc �֘A�̐錾
 *
 * @author  M.Nukui
 * @date	2003-11-07
 *
 * Copyright (C) 2003-2004 M.Nukui All rights reserved.
 */


#ifndef	YACC_H
#define	YACC_H

/* �O���[�o���ϐ� */

/// �p�[�T�̓��̓t�@�C��
extern FILE *	yyin;


/* �֐��v���g�^�C�v */

#ifdef __cplusplus
extern "C" {
#endif


extern int		yylex();
extern void		yyerror(char * s);
extern int		yyparse();


#ifdef __cplusplus
}
#endif


#endif /* YACC_H */
