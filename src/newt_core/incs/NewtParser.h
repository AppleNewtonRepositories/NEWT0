/*------------------------------------------------------------------------*/
/**
 * @file	NewtParser.h
 * @brief   �\���؂̐���
 *
 * @author  M.Nukui
 * @date	2003-11-07
 *
 * Copyright (C) 2003-2004 M.Nukui All rights reserved.
 */


#ifndef	NEWTPARSE_H
#define	NEWTPARSE_H

/* �w�b�_�t�@�C�� */
#include <stdio.h>

#include "NewtType.h"
#include "NewtConf.h"


/* �}�N�� */

/*
#define kNPSSyntaxNodeMask			0x80000003											///< �I�u�W�F�N�g�Q�Ƃ̃}�X�N�i�\���؃m�[�h�p�j

#define NPSRefIsSyntaxNode(r)		((r & kNPSSyntaxNodeMask) == kNPSSyntaxNodeMask)	///< �I�u�W�F�N�g�Q�Ƃ��\���؃m�[�h���H
#define NPSRefToSyntaxNode(r)		(((uint32_t)r & 0x7fffffff) >> 2)					///< �I�u�W�F�N�g�Q�Ƃ��\���؃m�[�h�ɕϊ�
#define NPSMakeSyntaxNode(v)		((v << 2) | kNPSSyntaxNodeMask)						///< �\���؃m�[�h�̃I�u�W�F�N�g�Q�Ƃ��쐬
*/

#define kNPSSyntaxNodeMask			0x0000000e									///< �I�u�W�F�N�g�Q�Ƃ̃}�X�N�i�\���؃m�[�h�p�j

#define NPSRefIsSyntaxNode(r)		((r & 0x0000000f) == kNPSSyntaxNodeMask)	///< �I�u�W�F�N�g�Q�Ƃ��\���؃m�[�h���H
#define NPSRefToSyntaxNode(r)		((uint32_t)r >> 4)							///< �I�u�W�F�N�g�Q�Ƃ��\���؃m�[�h�ɕϊ�
#define NPSMakeSyntaxNode(v)		((v << 4) | kNPSSyntaxNodeMask)				///< �\���؃m�[�h�̃I�u�W�F�N�g�Q�Ƃ��쐬


/* �萔 */

/// �I�y���[�^�R�[�h
enum {
	kNPS_NOT				= 256,  ///< not
	kNPS_AND,						///< and
	kNPS_OR,						///< or
	kNPS_DIV,						///< div
	kNPS_MOD,						///< mod
	kNPS_SHIFT_LEFT,				///< <<
	kNPS_SHIFT_RIGHT,				///< >>
	kNPS_NOT_EQUAL,					///< <>
	kNPS_GREATER_EQUAL,				///< >=
	kNPS_LESS_EQUAL,				///< <=
	kNPS_OBJECT_EQUAL,				///< ==
	kNPS_CONCAT2,					///< &&
};

/// �p�[�T�p�^������
enum {
    kNPSPop					= 000,	///< 000 pop
    kNPSDup					= 001,	///< 001 dup
    kNPSReturn				= 002,	///< 002 return
    kNPSPushSelf			= 003,	///< 003 push-self
    kNPSSetLexScope			= 004,	///< 004 set-lex-scope
    kNPSIterNext			= 005,	///< 005 iter-next
    kNPSIterDone			= 006,	///< 006 iter-done
    kNPSPopHandlers			= 007,	///< 007 000 001 pop-handlers

    kNPSPush				= 0030,	///< 03x push
    kNPSPushConstant		= 0040,	///< 04x (B signed) push-constant
    kNPSCall				= 0050,	///< 05x call
    kNPSInvoke				= 0060,	///< 06x invoke
    kNPSSend				= 0070,	///< 07x send
    kNPSSendIfDefined		= 0100,	///< 10x send-if-defined
    kNPSResend				= 0110,	///< 11x resend
    kNPSResendIfDefined		= 0120,	///< 12x resend-if-defined
    kNPSBranch				= 0130,	///< 13x branch
    kNPSBranchIfTrue		= 0140,	///< 14x branch-if-true
    kNPSBranchIfFalse		= 0150,	///< 15x branch-if-false
    kNPSFindVar				= 0160,	///< 16x find-var
    kNPSGetVar				= 0170,	///< 17x get-var
    kNPSMakeFrame			= 0200,	///< 20x make-frame
    kNPSMakeArray			= 0210,	///< 21x make-array
    kNPSGetPath				= 0220,	///< 220/221 get-path
    kNPSSetPath				= 0230,	///< 230/231 set-path
    kNPSSetVar				= 0240,	///< 24x set-var
    kNPSFindAndSetVar		= 0250,	///< 25x find-and-set-var
    kNPSIncrVar				= 0260,	///< 26x incr-var
    kNPSBranchIfLoopNotDone	= 0270,	///< 27x branch-if-loop-not-done
    kNPSFreqFunc			= 0300,	///< 30x freq-func
    kNPSNewHandlers			= 0310,	///< 31x new-handlers

    // 30x freq-func
    kNPSAdd					= 03000,///<  0 add					|+|
    kNPSSubtract,					///<  1 subtract			|-|
    kNPSAref,						///<  2 aref				aref
    kNPSSetAref,					///<  3 set-aref			setAref
    kNPSEquals,						///<  4 equals				|=|
    kNPSNot,						///<  5 not					|not|
    kNPSNotEqual,					///<  6 not-equals			|<>|
    kNPSMultiply,					///<  7 multiply			|*|
    kNPSDivide,						///<  8 divide				|/|
    kNPSDiv,						///<  9 div					|div|
    kNPSLessThan,					///< 10 less-than			|<|
    kNPSGreaterThan,				///< 11 greater-than		|>|
    kNPSGreaterOrEqual,				///< 12 greater-or-equal	|>=|
    kNPSLessOrEqual,				///< 13 less-or-equal		|<=|
    kNPSBitAnd,						///< 14 bit-and				BAnd
    kNPSBitOr,						///< 15 bit-or				BOr
    kNPSBitNot,						///< 16 bit-not				BNot
    kNPSNewIterator,				///< 17 new-iterator		newIterator
    kNPSLength,						///< 18 length				Length
    kNPSClone,						///< 19 clone				Clone
    kNPSSetClass,					///< 20 set-class			SetClass
    kNPSAddArraySlot,				///< 21 add-array-slot		AddArraySlot
    kNPSStringer,					///< 22 stringer			Stringer
    kNPSHasPath,					///< 23 has-path			none
    kNPSClassOf,					///< 24 class-of			ClassOf

    // 40x syntax
    kNPSConstituentList		= 04000,
    kNPSCommaList,
    kNPSConstant,
    kNPSGlobal,
    kNPSLocal,
    kNPSGlobalFn,
    kNPSFunc,
    kNPSArg,
	kNPSIndefinite,
    kNPSMessage,
    kNPSLvalue,
    kNPSAsign,
    kNPSExists,
    kNPSMethodExists,
    kNPSTry,
    kNPSOnexception,
    kNPSOnexceptionList,
    kNPSIf,
    kNPSLoop,
    kNPSFor,
    kNPSForeach,
    kNPSWhile,
    kNPSRepeat,
    kNPSBreak,
    kNPSSlot,
    kNPSConcat,								///< &
    kNPSConcat2,							///< &&

    // function
    kNPSAnd,								///< and
    kNPSOr,									///< or
    kNPSMod,								///< mod
	kNPSShiftLeft,							///< <<
	kNPSShiftRight,							///< >>

	// �Ǝ��g��
	kNPSObjectEqual,						///< ==
	kNPSMakeRegex,							///< ���K�\���I�u�W�F�N�g�̐���

    // Unknown
    KNPSUnknownCode		= 0xffffffff		///< �s���Ȗ���
};


/// �\���؃f�[�^�̎��
enum {
    kNPSKindNone,		///< �f�[�^�Ȃ�
    kNPSKindLink,		///< �����N
    kNPSKindObject		///< �I�u�W�F�N�g
};


/* �^�錾 */

/// �\���؃m�[�h�ւ̃|�C���^
typedef newtRef		nps_node_t;


/// �\���؃m�[�h
typedef struct {
    uint32_t	code;   ///< ���߃R�[�h
    nps_node_t	op1;	///< �I�y�R�[�h1
    nps_node_t	op2;	///< �I�y�R�[�h2
} nps_syntax_node_t;


/// �p�[�T��
typedef struct {
	bool			first_time;			///< �����͂̏��񔻕ʗp�t���O

	uint16_t		numwarns;			///< �����������[�j���O��
	uint16_t		numerrs;			///< ���������G���[��

	const char *	fname;				///< ���͒��̃t�@�C����
	uint32_t		lineno;				///< �����͂̍s�ԍ�
	uint32_t		tokenpos;			///< �g�[�N���̈ʒu
	uint16_t		yyleng;				///< �g�[�N���̒���
	char			linebuf[NEWT_LEX_LINEBUFFSIZE];  ///< �s�o�b�t�@
} nps_env_t;


/* �O���[�o���ϐ� */
extern nps_env_t	nps_env;		///< �p�[�T��


/* �֐��v���g�^�C�v */

#ifdef __cplusplus
extern "C" {
#endif


int			nps_yyinput(FILE * yyin, char * buff, int max_size);
void		nps_yyinit(void);
int			nps_yycleanup(void);


newtErr		NPSParse(const char * path, nps_syntax_node_t ** streeP, uint32_t * sizeP, bool is_file);

newtErr		NPSParseFile(const char * path,
                    nps_syntax_node_t ** streeP, uint32_t * sizeP);

newtErr		NPSParseStr(const char * s,
                    nps_syntax_node_t ** streeP, uint32_t * sizeP);

void		NPSCleanup(void);
void		NPSDumpSyntaxTree(FILE * f, nps_syntax_node_t * stree, uint32_t size);

//nps_node_t	NPSGetCX(void);

nps_node_t	NPSGenNode0(uint32_t code);
nps_node_t	NPSGenNode1(uint32_t code, nps_node_t op1);
nps_node_t	NPSGenNode2(uint32_t code, nps_node_t op1, nps_node_t op2);

nps_node_t	NPSGenOP1(uint32_t op, nps_node_t op1);
nps_node_t	NPSGenOP2(uint32_t op, nps_node_t op1, nps_node_t op2);

nps_node_t	NPSGenSend(nps_node_t receiver,
                    uint32_t op, nps_node_t msg, nps_node_t args);
nps_node_t	NPSGenResend(uint32_t op, nps_node_t msg, nps_node_t args);

nps_node_t	NPSGenIfThenElse(nps_node_t cond, nps_node_t ifthen, nps_node_t ifelse);
nps_node_t	NPSGenForLoop(nps_node_t index, nps_node_t v,
                    nps_node_t to, nps_node_t by, nps_node_t expr);
nps_node_t	NPSGenForeach(nps_node_t index, nps_node_t val, nps_node_t obj,
                    nps_node_t deeply, nps_node_t op, nps_node_t expr);
nps_node_t	NPSGenGlobalFn(nps_node_t name, nps_node_t args, nps_node_t expr);

newtRef		NPSMakePathExpr(newtRefArg sym1, newtRefArg sym2);
newtRef		NPSMakeArray(newtRefArg v);
newtRef		NPSAddArraySlot(newtRefArg r, newtRefArg v);
newtRef		NPSInsertArraySlot(newtRefArg r, uint32_t p, newtRefArg v);

newtRef		NPSMakeMap(newtRefArg v);
newtRef		NPSMakeFrame(newtRefArg slot, newtRefArg v);
newtRef		NPSSetSlot(newtRefArg r, newtRefArg slot, newtRefArg v);

newtRef		NPSMakeBinary(newtRefArg v);
newtRef		NPSAddARef(newtRefArg r, newtRefArg v);

void		NPSErrorStr(char c, char * s);
void		NPSError(int32_t err);


#ifdef __cplusplus
}
#endif


#endif /* NEWTPARSE_H */
