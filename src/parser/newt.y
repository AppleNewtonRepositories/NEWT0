/*------------------------------------------------------------------------*/
/**
 * @file	newt.y
 * @brief   �\�����
 *
 * @author  M.Nukui
 * @date	2003-11-07
 *
 * Copyright (C) 2003-2004 M.Nukui All rights reserved.
 */

%{

/* �w�b�_�t�@�C�� */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "yacc.h"

#include "NewtCore.h"
#include "NewtParser.h"


/* �}�N�� */
#define SYMCHECK(v, sym)	if (v != sym) NPSError(kNErrSyntaxError);
#define TYPECHECK(v)		if (! (v == NS_INT || v == NSSYM0(array))) NPSError(kNErrSyntaxError);
#define ERR_NOS2C(msg)		if (NEWT_MODE_NOS2) yyerror(msg);


%}

%union {
    newtRefVar	obj;	// �I�u�W�F�N�g
    uint32_t	op;		// ���Z�q
    nps_node_t	node;	// �m�[�h
}

   /* �^�C�v */

%type	<node>	constituent_list	constituent
%type	<node>	expr
%type	<node>	simple_expr
%type	<node>	compound_expr
%type	<node>	expr_sequence
%type	<node>	constructor
%type	<node>	lvalue
%type	<node>	exists_expr
%type	<node>	local_declaration	l_init_clause_list	init_clause
%type	<node>	c_init_clause_list	c_init_clause
%type	<node>	if_expr
%type	<node>	iteration
%type	<node>	function_call
%type	<node>	actual_argument_list
%type	<node>	message_send
%type	<node>	by_expr
%type	<node>	expr_list
%type	<node>	onexcp_list	onexcp
%type	<node>	frame_constructor_list	frame_slot_value	frame_slot_list
%type	<node>	frame_accessor	array_accessor
%type	<node>	formal_argument_list	formal_argument_list2	formal_argument	indefinite_argument
%type	<node>	global_declaration	global_function_decl	function_decl
%type	<node>	type

%type	<obj>	literal
%type	<obj>	simple_literal
%type	<obj>	object
%type	<obj>	array	array_item_list
%type	<obj>	frame
%type	<obj>	binary	binary_item_list
%type	<obj>	path_expr
%type	<obj>	foreach_operator	deeply


%token  <op>	kBINBG	kBINED	// �o�C�i���f�[�^�i�Ǝ��g���j


   /* �\��� */

%token		kBEGIN	kEND
			kFUNC	kNATIVE		kCALL	kWITH
			kFOR	kFOREACH	kTO		kBY
			kSELF	kINHERITED
			kIF
			kTRY	kONEXCEPTION
			kLOOP	kWHILE	kREPEAT kUNTIL  kDO		kIN
			kGLOBAL	kLOCAL	kCONSTANT
			k3READER		// �R�_���[�_�[�i�Ǝ��g���j
			kERROR			// �G���[

    /* �I�u�W�F�N�g */

%token		<obj>	kSYMBOL		kSTRING		kREGEX
%token		<obj>	kINTEGER	kREAL		kCHARACTER	kMAGICPOINTER
%token		<obj>	kTRUE		kNIL		kUNBIND

// �D�揇��

    /* �� */

%left		kEXPR2

    /* �\��� */

%left		kBREAK	kRETURN
%left		//kTRY  kONEXCEPTION
			kLOOP	kWHILE	kREPEAT kUNTIL	kDO	kIN
			kGLOBAL	kLOCAL	kCONSTANT
			kSYMBOL
%left		','		//  ';'

    /* IF �� */

%nonassoc	kTHEN
%nonassoc	kELSE

    /* ���Z�q */

/*
�\2-5 �ɂ́ANewtonScript �̑S���Z�q�̗D�揇�ʂƁA�������̂���Ⴂ����
�ցA�ォ�牺�ɕ��ׂĎ����B�ꏏ�̃O���[�v�ɓ����Ă��鉉�Z�q�́A������
�D�揇�ʂ������Ƃɒ��ӁB

.				�X���b�g�A�N�Z�X					��
: :?			(�����t)���b�Z�[�W���M			��
[]				�z��v�f						��
-				�P���}�C�i�X					��
<< >>			���V�t�g�E�V�t�g					��
* / div mod		��Z�A���Z�A�������Z�A�]��			��
+ -				���Z�A���Z						��
& &&			�����񍇐��A������X�y�[�X���荇��	��
exists			�ϐ��E�X���b�g�̑��݊m�F			�Ȃ�
< <= > >= = <>	��r							��
not				�_���ے�						��
and or			�_��AND, �_��OR				��
:=				���							��
*/

%right		<op>	kASNOP		// ���							��
%left		<op>	kANDOP		// �_��AND, �_��OR				��
%left		<op>	kNOTOP		// �_���ے�						��
%left		<op>	kRELOP		// ��r							��
%left		<op>	kEXISTS		// �ϐ��E�X���b�g�̑��݊m�F			�Ȃ�
%left		<op>	kSTROP		// �����񍇐��A������X�y�[�X���荇��	��
%left		<op>	kADDOP		// ���Z�A���Z						��
%left		<op>	kMULOP		// ��Z�A���Z�A�������Z�A�]��			��
%left		<op>	kSFTOP		// ���V�t�g�E�V�t�g					��
%nonassoc			kUMINUS		// �P���}�C�i�X					��
%left				'[' ']'		// �z��v�f						��
%left		<op>	kSNDOP		// (�����t)���b�Z�[�W���M			��
					':'
%left				'.'			// �X���b�g�A�N�Z�X					��

    /* ���� */

%left				'(' ')'

%%

input
		: constituent_list  {}
		;

constituent_list
		: /* empty */					{ $$ = kNewtRefUnbind; }
		| constituent					{ $$ = NPSGenNode1(kNPSConstituentList, $1); }
		| constituent_list ';'
//		| constituent ';' constituent_list	// �E�ċA
		| constituent_list ';' constituent	// ���ċA
										{ $$ = NPSGenNode2(kNPSConstituentList, $1, $3); }
		| constituent_list error ';'	{ yyerrok; }
		;

constituent
		: expr
		| global_declaration
		;

expr
		: simple_expr
		| compound_expr
		| literal						{ $$ = $1; }
		| constructor
		| lvalue						{ $$ = NPSGenNode1(kNPSLvalue, $1); }
		| lvalue kASNOP expr			{ $$ = NPSGenNode2(kNPSAsign, $1, $3); }
		| kMAGICPOINTER kASNOP expr		{	// �}�W�b�N�|�C���^�̒�`�i�Ǝ��g���j
											ERR_NOS2C("Assign Magic Pointer");	// NOS2 ��݊�
											$$ = NPSGenNode2(kNPSAsign, $1, $3);
										}
		| exists_expr
		| function_call
		| message_send
		| if_expr
		| iteration

		// break_expr
		| kBREAK						{ $$ = NPSGenNode0(kNPSBreak); }
		| kBREAK expr					{ $$ = NPSGenNode1(kNPSBreak, $2); }

		// try_expr
		| kTRY expr_sequence onexcp_list
										{ $$ = NPSGenNode2(kNPSTry, $2, $3); }

		| local_declaration
		| kCONSTANT c_init_clause_list	{ $$ = NPSGenNode1(kNPSConstant, $2); }

		// return_expr
		| kRETURN						{ $$ = NPSGenNode0(kNPSReturn); }
		| kRETURN expr					{ $$ = NPSGenNode1(kNPSReturn, $2); }
		;

lvalue
		: kSYMBOL						{ $$ = $1; }
		| frame_accessor
		| array_accessor
		;

simple_expr
		// binary_operator
		: expr kADDOP expr   { $$ = NPSGenOP2($2, $1, $3); }
		| expr kMULOP expr   { $$ = NPSGenOP2($2, $1, $3); }
		| expr kSFTOP expr   { $$ = NPSGenOP2($2, $1, $3); }
		| expr kRELOP expr   { $$ = NPSGenOP2($2, $1, $3); }
		| expr kANDOP expr   { $$ = NPSGenOP2($2, $1, $3); }
		| expr kSTROP expr   { $$ = NPSGenOP2($2, $1, $3); }

		// unary_operator
		| kADDOP expr %prec kUMINUS
							{
								if ($1 == '-')
								{
									if (NewtRefIsInteger($2))
										$$ = NewtMakeInteger(- NewtRefToInteger($2));
									else
										$$ = NPSGenOP2('-', NewtMakeInteger(0), $2);
								}
								else
								{
									$$ = $2;
								}
							}

		| kNOTOP expr		{ $$ = NPSGenOP1($1, $2); }

		| '(' expr ')'		{ $$ = $2; }
		| kSELF				{ $$ = NPSGenNode0(kNPSPushSelf); }
		;

compound_expr
		: kBEGIN expr_sequence kEND	{ $$ = $2; }
		;

expr_sequence
		: /* empty */				{ $$ = kNewtRefUnbind; }
		| expr
		| expr_sequence ';'
//		| expr ';' expr_sequence	// �E�ċA
		| expr_sequence ';' expr	// ���ċA
									{ $$ = NPSGenNode2(kNPSConstituentList, $1, $3); }
		| expr_sequence error ';'   { yyerrok; }
		;

literal
		: simple_literal
		| kSTRING		{ $$ = NPSGenNode1(kNPSClone, $1); }
		| binary		{	// �o�C�i���f�[�^�i�Ǝ��g���j
							ERR_NOS2C("Binary Syntax");	// NOS2 ��݊�
							$$ = NPSGenNode1(kNPSClone, $1);
						}
		| '\'' object   { $$ = NewtPackLiteral($2); }
		;

simple_literal
//		: kSTRING
		: kINTEGER
		| kREAL
		| kCHARACTER
		| kMAGICPOINTER
		| kTRUE			{ $$ = kNewtRefTRUE; }
		| kNIL			{ $$ = kNewtRefNIL; }
		| kUNBIND		{	// #UNBIND �i�Ǝ��g���j
							ERR_NOS2C("Unbind Ref");	// NOS2 ��݊�
							$$ = kNewtRefUnbind;
						}
//		| binaryy		{ ERR_NOS2C("Binary Syntax"); }	// �o�C�i���f�[�^�i�Ǝ��g���j
		;

object
		: simple_literal
		| kSTRING
		| kSYMBOL				// NewtonScript �̍\���}�ł͂��ꂪ�Ȃ��i�L�q�R��H�j
		| path_expr
		| array
		| frame
		| binary		{ ERR_NOS2C("Binary Syntax"); }	// �o�C�i���f�[�^�i�Ǝ��g���j
		;

path_expr
		: kSYMBOL '.' kSYMBOL		{ $$ = NPSMakePathExpr($1, $3); }
		| kSYMBOL '.' path_expr		{ $$ = NPSInsertArraySlot($3, 0, $1); }
		;

array
		: '[' kSYMBOL ':' array_item_list ']'	{ $$ = NcSetClass($4, $2); } 
		| '[' array_item_list ']'				{ $$ = $2; }
		;

array_item_list
		: /* empty */					{ $$ = NPSMakeArray(kNewtRefUnbind); }
		| object						{ $$ = NPSMakeArray($1); }
		| array_item_list ','
		| array_item_list ',' object	{ $$ = NPSAddArraySlot($1, $3); }
		;

frame
		: '{' frame_slot_list '}'		{ $$ = $2; }
		;

frame_slot_list
		: /* empty */					{ $$ = NPSMakeFrame(kNewtRefUnbind, kNewtRefUnbind); }
		| kSYMBOL ':' object			{ $$ = NPSMakeFrame($1, $3); }
		| frame_slot_list ','
		| frame_slot_list ',' kSYMBOL ':' object
                                        { $$ = NPSSetSlot($1, $3, $5); }
		;

binary	// �o�C�i���f�[�^�i�Ǝ��g���j
		: kBINBG kSYMBOL ':' binary_item_list kBINED	{ $$ = NcSetClass($4, $2); } 
		| kBINBG binary_item_list kBINED				{ $$ = $2; }
		;

binary_item_list
		: /* empty */					{ $$ = NPSMakeBinary(kNewtRefUnbind); }
		| object						{ $$ = NPSMakeBinary($1); }
		| binary_item_list ',' object	{ $$ = NPSAddARef($1, $3); }
		;

constructor
		// �z��̐���
		: '[' kSYMBOL ':' expr_list ']'		{ $$ = NPSGenNode2(kNPSMakeArray, $2, $4); }
		| '[' expr_list ']'					{ $$ = NPSGenNode2(kNPSMakeArray, kNewtRefUnbind, $2); }

		// �t���[���̐���
		| '{' frame_constructor_list '}'	{ $$ = NPSGenNode1(kNPSMakeFrame, $2); }

		// �֐��I�u�W�F�N�g�̐���
		| kFUNC func_keyword '(' formal_argument_list ')' expr %prec kEXPR2
                        { $$ = NPSGenNode2(kNPSFunc, $4, $6); }

		// ���K�\���I�u�W�F�N�g�̐���
		| kREGEX kSTRING			{ $$ = NPSGenNode2(kNPSMakeRegex, $1, $2); }
		| kREGEX					{ $$ = NPSGenNode2(kNPSMakeRegex, $1, kNewtRefNIL); }
		;

expr_list
		: /* empty */				{ $$ = kNewtRefUnbind; }
		| expr
		| expr_list ','
		| expr_list ',' expr
									{ $$ = NPSGenNode2(kNPSCommaList, $1, $3); }
		;

frame_constructor_list
		: /* empty */				{ $$ = kNewtRefUnbind; }
		| frame_slot_value
		| frame_constructor_list ','
		| frame_constructor_list ',' frame_slot_value
									{ $$ = NPSGenNode2(kNPSCommaList, $1, $3); }
		;

frame_slot_value
		: kSYMBOL ':' expr	{ $$ = NPSGenNode2(kNPSSlot, $1, $3); }
		;

func_keyword	// ����(not supported)
		: /* empty */
		| kNATIVE
		;

formal_argument_list
		: /* empty */				{ $$ = kNewtRefUnbind; }
		| formal_argument_list2
		| formal_argument_list2 ',' indefinite_argument
									{ $$ = NPSGenNode2(kNPSCommaList, $1, $3); }
		;

formal_argument_list2
		: formal_argument
		| formal_argument_list2 ',' formal_argument
									{ $$ = NPSGenNode2(kNPSCommaList, $1, $3); }
		;

formal_argument
		: kSYMBOL					{ $$ = $1; }
		| type kSYMBOL				{ $$ = NPSGenNode2(kNPSArg, $1, $2); }		// type �͖���(not supported)
		;

indefinite_argument
		: kSYMBOL k3READER			{	// �s�蒷�i�Ǝ��g���j
										ERR_NOS2C("Indefinite Argument");	// NOS2 ��݊�
										$$ = NPSGenNode1(kNPSIndefinite, $1);
									}
		;

type
		: kSYMBOL					{ TYPECHECK($1); $$ = $1; }
		;

frame_accessor
		: expr '.' kSYMBOL			{ $$ = NPSGenNode2(kNPSGetPath, $1, $3); }
		| expr '.' '(' expr ')'		{ $$ = NPSGenNode2(kNPSGetPath, $1, $4); }
		;

array_accessor
		: expr '[' expr ']'			{ $$ = NPSGenNode2(kNPSAref, $1, $3); }
		;

exists_expr
		: kSYMBOL kEXISTS			{ $$ = NPSGenNode1(kNPSExists, $1); }
		| frame_accessor kEXISTS	{ $$ = NPSGenNode1(kNPSExists, $1); }
		| ':' kSYMBOL kEXISTS		{ $$ = NPSGenNode2(kNPSMethodExists, kNewtRefUnbind, $2); }
		| expr ':' kSYMBOL kEXISTS	{ $$ = NPSGenNode2(kNPSMethodExists, $1, $3); }
		;

function_call
		: kSYMBOL '(' actual_argument_list ')'	{ $$ = NPSGenNode2(kNPSCall, $1, $3); }
		| kCALL expr kWITH '(' actual_argument_list ')'
												{ $$ = NPSGenNode2(kNPSInvoke, $2, $5); }
		;

actual_argument_list
		: /* emtpry */							{ $$ = kNewtRefUnbind; }
		| expr
		| actual_argument_list ',' expr	{ $$ = NPSGenNode2(kNPSCommaList, $1, $3); }
		;

message_send
		: ':' kSYMBOL '(' actual_argument_list ')'
                                    { $$ = NPSGenSend(kNewtRefUnbind, $1, $2, $4); }
		| kSNDOP kSYMBOL '(' actual_argument_list ')'
                                    { $$ = NPSGenSend(kNewtRefUnbind, $1, $2, $4); }

		| expr ':' kSYMBOL '(' actual_argument_list ')'
                                    { $$ = NPSGenSend($1, ':', $3, $5); }
		| expr kSNDOP kSYMBOL '(' actual_argument_list ')'
                                    { $$ = NPSGenSend($1, $2, $3, $5); }

		| kINHERITED ':' kSYMBOL '(' actual_argument_list ')'
                                    { $$ = NPSGenResend(':', $3, $5); }
		| kINHERITED kSNDOP kSYMBOL '(' actual_argument_list ')'
                                    { $$ = NPSGenResend($2, $3, $5); }
		;

if_expr
		: kIF expr kTHEN expr					{ $$ = NPSGenIfThenElse($2, $4, kNewtRefUnbind); }
		| kIF expr kTHEN expr kELSE expr		{ $$ = NPSGenIfThenElse($2, $4, $6); }
//		| kIF expr kTHEN expr ';' kELSE expr	{ $$ = NPSGenIfThenElse($2, $4, $7); }
		;

iteration
		: kLOOP expr						{ $$ = NPSGenNode1(kNPSLoop, $2); }
		| kFOR kSYMBOL kASNOP expr kTO expr by_expr kDO expr
											{ $$ = NPSGenForLoop($2, $4, $6, $7, $9); }
		| kFOREACH kSYMBOL deeply kIN expr foreach_operator expr
											{ $$ = NPSGenForeach(kNewtRefUnbind, $2, $5, $3, $6, $7); }
		| kFOREACH kSYMBOL ',' kSYMBOL deeply kIN expr foreach_operator expr
											{ $$ = NPSGenForeach($2, $4, $7, $5, $8, $9); }
		| kWHILE expr kDO expr				{ $$ = NPSGenNode2(kNPSWhile, $2, $4); }
		| kREPEAT expr_sequence kUNTIL expr { $$ = NPSGenNode2(kNPSRepeat, $2, $4); }
		;

by_expr
		: /* emtpry */		{ $$ = kNewtRefUnbind; }
		| kBY expr			{ $$ = $2; }
		;

foreach_operator
		: kDO				{ $$ = kNewtRefNIL; }
		| kSYMBOL			{ SYMCHECK($1, NSSYM0(collect)); $$ = $1; }		// collect
		;

deeply
		: /* emtpry */		{ $$ = kNewtRefNIL; }
		| kSYMBOL			{ SYMCHECK($1, NSSYM0(deeply)); $$ = kNewtRefTRUE; }	// deeply
		;

onexcp_list
		: onexcp
/*
		| onexcp ';' onexcp_list	{ $$ = NPSGenNode2(kNPSOnexceptionList, $1, $3); }
		| onexcp onexcp_list		{ $$ = NPSGenNode2(kNPSOnexceptionList, $1, $2); }
*/
//		| onexcp_list ';' onexcp	{ $$ = NPSGenNode2(kNPSOnexceptionList, $1, $3); }
		| onexcp_list onexcp		{ $$ = NPSGenNode2(kNPSOnexceptionList, $1, $2); }
		;

onexcp
		: kONEXCEPTION kSYMBOL kDO expr { $$ = NPSGenNode2(kNPSOnexception, $2, $4); }
		;

local_declaration
		: kLOCAL l_init_clause_list  { $$ = NPSGenNode2(kNPSLocal, kNewtRefUnbind, $2); }
		| kLOCAL kSYMBOL l_init_clause_list	// type �͖���(not supported)
			{
				TYPECHECK($2);
				$$ = NPSGenNode2(kNPSLocal, $2, $3);
			}
/*
		// ���ꂾ�ƃR���t���N�g����������
		| kLOCAL type l_init_clause_list	// type �͖���(not supported)
									{ $$ = NPSGenNode2(kNPSLocal, $2, $3); }
*/
		;

l_init_clause_list
		: init_clause
//		| init_clause ',' l_init_clause_list	// �E�ċA
		| l_init_clause_list ',' init_clause	// ���ċA
									{ $$ = NPSGenNode2(kNPSCommaList, $1, $3); }
		;

init_clause
		: kSYMBOL					{ $$ = $1; }
		| kSYMBOL kASNOP expr		{ $$ = NPSGenNode2(kNPSAsign, $1, $3); }
		;

c_init_clause_list
		: c_init_clause
		| c_init_clause_list ',' c_init_clause	// ���ċA
									{ $$ = NPSGenNode2(kNPSCommaList, $1, $3); }
		;

c_init_clause
		: kSYMBOL kASNOP expr		{ $$ = NPSGenNode2(kNPSAsign, $1, $3); }
		;

global_declaration
		: kGLOBAL init_clause		{ $$ = NPSGenNode1(kNPSGlobal, $2); }
		| global_function_decl
		;

global_function_decl
		: kGLOBAL function_decl		{ $$ = $2; }
		| kFUNC function_decl		{ $$ = $2; }
		;

function_decl
		: kSYMBOL '(' formal_argument_list ')' expr { $$ = NPSGenGlobalFn($1, $3, $5); }
		;

%%
