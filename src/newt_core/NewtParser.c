//--------------------------------------------------------------------------
/**
 * @file	NewtParser.c
 * @brief   �\���؂̐���
 *
 * @author  M.Nukui
 * @date	2003-11-07
 *
 * Copyright (C) 2003-2004 M.Nukui All rights reserved.
 */


/* �w�b�_�t�@�C�� */
#include <stdlib.h>
#include <string.h>

#include "NewtParser.h"
#include "NewtErrs.h"
#include "NewtMem.h"
#include "NewtObj.h"
#include "NewtEnv.h"
#include "NewtFns.h"
#include "NewtVM.h"
#include "NewtIO.h"
#include "NewtPrint.h"

#include "yacc.h"


/* �^�錾 */

/// ���̓f�[�^
typedef struct {
	const char *	data;		///< ���̓f�[�^
	const char *	currp;		///< ���݂̓��͈ʒu
	const char *	limit;		///< ���̓f�[�^�̍Ō�
} nps_inputdata_t;


/* �O���[�o���ϐ� */
nps_env_t				nps_env;		///< �p�[�T��


/* ���[�J���ϐ� */
static newtStack		nps_stree;		///< �\���؃X�^�b�N���
static nps_inputdata_t  nps_inputdata;  ///< ���̓f�[�^


/* �}�N�� */
#define STREESTACK		((nps_syntax_node_t *)nps_stree.stackp)		///< �\���؃X�^�b�N
#define CX				(nps_stree.sp)								///< �\���؃X�^�b�N�|�C���^


/* �֐��v���g�^�C�v */
static void		NPSBindParserInput(const char * s);
static int		nps_yyinput_str(char * buff, int max_size);

static void		NPSInit(newtPool pool);

static void		NPSPrintNode(FILE * f, nps_node_t r);
static void		NPSPrintSyntaxCode(FILE * f, uint32_t code);


#pragma mark -
/*------------------------------------------------------------------------*/
/** �\����͂��镶������Z�b�g����
 *
 * @param s			[in] ������
 *
 * @return			�Ȃ�
 */

void NPSBindParserInput(const char * s)
{
    nps_inputdata.data = nps_inputdata.currp = s;

    if (s != NULL)
        nps_inputdata.limit = s + strlen(s);
    else
        nps_inputdata.limit = NULL;
}


/*------------------------------------------------------------------------*/
/** �\����͂���f�[�^�𕶎��񂩂��o��
 *
 * @param buff		[out]�o�b�t�@
 * @param max_size	[in] �ő咷
 *
 * @return			��o�����f�[�^�T�C�Y
 */

int nps_yyinput_str(char * buff, int max_size)
{
    int	n;

    n = nps_inputdata.limit - nps_inputdata.currp;
    if (max_size < n) n = max_size;

    if (0 < n)
    {
        memcpy(buff, nps_inputdata.currp, n);
        nps_inputdata.currp += n;
    }

    return n;
}


/*------------------------------------------------------------------------*/
/** �\����͂���f�[�^����o��
 *
 * @param yyin		[in] ���̓t�@�C��
 * @param buff		[out]�o�b�t�@
 * @param max_size	[in] �ő咷
 *
 * @return			��o�����f�[�^�T�C�Y
 */

int nps_yyinput(FILE * yyin, char * buff, int max_size)
{
    if (nps_inputdata.data != NULL)
        return nps_yyinput_str(buff, max_size);
    else
        return fread(buff, 1, max_size, yyin);
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �\����͂̂��߂̏�����
 *
 * @param pool		[in] �������v�[��
 *
 * @return			�Ȃ�
 */

void NPSInit(newtPool pool)
{
	nps_yyinit();

	nps_env.fname = NULL;
	nps_env.lineno = 1;
	nps_env.tokenpos = 0;
	nps_env.first_time = true;
	nps_env.linebuf[sizeof(nps_env.linebuf) - 1] = '\0';

    NewtStackSetup(&nps_stree, NEWT_POOL, sizeof(nps_syntax_node_t), NEWT_NUM_STREESTACK);
}


/*------------------------------------------------------------------------*/
/** �\����͂���
 *
 * @param path		[in] ���̓t�@�C���̃p�X
 * @param streeP	[out]�\����
 * @param sizeP		[out]�\���؂̃T�C�Y
 * @param is_file	[in] �t�@�C�����ǂ����i#! �����������Ȃ����ǂ����j
 *
 * @return			�G���[�R�[�h
 */

newtErr NPSParse(const char * path, nps_syntax_node_t ** streeP, uint32_t * sizeP, bool is_file)
{
    newtErr	err = kNErrNone;

    NPSInit(NULL);
	nps_env.fname = path;
	nps_env.first_time = is_file;

    if (yyparse() != 0)
        err = kNErrSyntaxError;

	nps_yycleanup();

    if (NEWT_DUMPSYNTAX)
    {
        NewtFprintf(stderr, "*** syntax tree ***\n");
        NPSDumpSyntaxTree(stderr, STREESTACK, CX);
        NewtFprintf(stderr, "\n");
    }

    if (err == kNErrNone && 0 < CX)
        NewtStackSlim(&nps_stree, CX);
    else
        NPSCleanup();

    *streeP = STREESTACK;
    *sizeP = CX;

    return err;
}


/*------------------------------------------------------------------------*/
/** �w�肳�ꂽ�t�@�C�����\�[�X�ɍ\����͂���
 *
 * @param path		[in] ���̓t�@�C���̃p�X
 * @param streeP	[out]�\����
 * @param sizeP		[out]�\���؂̃T�C�Y
 *
 * @return			�G���[�R�[�h
 */

newtErr NPSParseFile(const char * path,
        nps_syntax_node_t ** streeP, uint32_t * sizeP)
{
	newtErr err;

    if (path != NULL)
    {
        yyin = fopen(path, "r");

        if (yyin == NULL)
        {
            NewtFprintf(stderr, "not open file.\n");
            return kNErrFileNotOpen;
        }

		err = NPSParse(path, streeP, sizeP, true);
		fclose(yyin);
    }
	else
	{
		err = NPSParse(path, streeP, sizeP, true);
	}

	return err;
}


/*------------------------------------------------------------------------*/
/** ��������\�[�X�ɍ\����͂���
 *
 * @param s			[in] ���̓f�[�^
 * @param streeP	[out]�\����
 * @param sizeP		[out]�\���؂̃T�C�Y
 *
 * @return			�G���[�R�[�h
 */

newtErr NPSParseStr(const char * s,
        nps_syntax_node_t ** streeP, uint32_t * sizeP)
{
    newtErr	err;

    NPSBindParserInput(s);
    err = NPSParse(NULL, streeP, sizeP, false);
    NPSBindParserInput(NULL);

    return err;
}


/*------------------------------------------------------------------------*/
/** �\����͂̌�n��
 *
 * @return			�Ȃ�
 */

void NPSCleanup(void)
{
    NewtStackFree(&nps_stree);
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �\���؂̃m�[�h���󎚂���
 *
 * @param f			[in] �o�̓t�@�C��
 * @param r			[in] �m�[�h
 *
 * @return			�Ȃ�
 */

void NPSPrintNode(FILE * f, nps_node_t r)
{
    if (NPSRefIsSyntaxNode(r))
        NewtFprintf(f, "*%d", (int)NPSRefToSyntaxNode(r));
    else
        NewtPrintObj(f, (newtRef)r);
}


/*------------------------------------------------------------------------*/
/** �\���R�[�h���󎚂���
 *
 * @param f			[in] �o�̓t�@�C��
 * @param code		[in] �\���R�[�h
 *
 * @return			�Ȃ�
 */

void NPSPrintSyntaxCode(FILE * f, uint32_t code)
{
    char *	s = "Unknown";

    switch (code)
    {
        case kNPSConstituentList:
            s = ";";
            break;

        case kNPSCommaList:
            s = ",";
            break;

        case kNPSConstant:
            s = "constant";
            break;

        case kNPSGlobal:
            s = "global";
            break;

        case kNPSLocal:
            s = "local";
            break;

        case kNPSGlobalFn:
            s = "global func";
            break;

        case kNPSFunc:
            s = "func";
            break;

        case kNPSArg:
            s = "arg";
            break;

        case kNPSMessage:
            s = "message";
            break;

        case kNPSLvalue:
            s = "lvalue";
            break;

        case kNPSAsign:
            s = ":=";
            break;

        case kNPSExists:
            s = "exists";
            break;

        case kNPSMethodExists:
            s = "method exists";
            break;

        case kNPSTry:
            s = "try";
            break;

        case kNPSOnexception:
            s = "onexception";
            break;

        case kNPSOnexceptionList:
            s = "onexception list";
            break;

        case kNPSIf:
            s = "if";
            break;

        case kNPSLoop:
            s = "loop";
            break;

        case kNPSFor:
            s = "for";
            break;

        case kNPSForeach:
            s = "foreach";
            break;

        case kNPSWhile:
            s = "while";
            break;

        case kNPSRepeat:
            s = "repeat";
            break;

        case kNPSBreak:
            s = "break";
            break;

        case kNPSSlot:
            s = "slot";
            break;
    }

    NewtFprintf(f, "%24s\t", s);
}


/*------------------------------------------------------------------------*/
/** �\���؂��_���v����
 *
 * @param f			[in] �o�̓t�@�C��
 * @param stree		[in] �\����
 * @param size		[in] �\���؂̃T�C�Y
 *
 * @return			�Ȃ�
 */

void NPSDumpSyntaxTree(FILE * f, nps_syntax_node_t * stree, uint32_t size)
{
    nps_syntax_node_t *	node;
    uint32_t	i;

    for (i = 0, node = stree; i < size; i++, node++)
    {
        NewtFprintf(f, "%d\t", (int)i);

        if (kNPSConstituentList <= node->code)
        {
            NPSPrintSyntaxCode(f, node->code);
        }
        else
        {
            if (node->code <= kNPSPopHandlers)
                NVMDumpInstName(f, 0, node->code);
            else if (kNPSAdd <= node->code)
                NVMDumpInstName(f, kNPSFreqFunc >> 3, node->code - kNPSAdd);
            else
                NVMDumpInstName(f, node->code >> 3, 0);
        }

        NewtFprintf(f, "\t");
        NPSPrintNode(f, node->op1);
        NewtFprintf(f, "\t");
        NPSPrintNode(f, node->op2);
        NewtFprintf(f, "\n");
    }
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �����O�̃m�[�h���쐬
 *
 * @param code		[in] �\���R�[�h
 *
 * @return			�m�[�h
 */

nps_node_t NPSGenNode0(uint32_t code)
{
    return NPSGenNode2(code, kNewtRefUnbind, kNewtRefUnbind);
}


/*------------------------------------------------------------------------*/
/** �����P�̃m�[�h���쐬
 *
 * @param code		[in] �\���R�[�h
 * @param op1		[in] �����P
 *
 * @return			�m�[�h
 */

nps_node_t NPSGenNode1(uint32_t code, nps_node_t op1)
{
    return NPSGenNode2(code, op1, kNewtRefUnbind);
}


/*------------------------------------------------------------------------*/
/** �����Q�̃m�[�h���쐬
 *
 * @param code		[in] �\���R�[�h
 * @param op1		[in] �����P
 * @param op2		[in] �����Q
 *
 * @return			�m�[�h
 */

nps_node_t NPSGenNode2(uint32_t code, nps_node_t op1, nps_node_t op2)
{
    nps_syntax_node_t *	node;

    if (! NewtStackExpand(&nps_stree, CX + 1))
    {
        yyerror("Syntax tree overflow");
        return -1;
    }

    node = &STREESTACK[CX];

    node->code = code;
    node->op1 = op1;
    node->op2 = op2;

    CX++;

    return NPSMakeSyntaxNode(CX - 1);
}


/*------------------------------------------------------------------------*/
/** �����P�̃I�y���[�^�m�[�h���쐬
 *
 * @param op		[in] �I�y�R�[�h
 * @param op1		[in] �����P
 *
 * @return			�m�[�h
 */

nps_node_t NPSGenOP1(uint32_t op, nps_node_t op1)
{
    uint32_t	b = KNPSUnknownCode;

    switch (op)
    {
        case kNPS_NOT:
            b = kNPSNot;
            break;

        default:
            NPSError(kNErrSyntaxError);
            return kNewtRefUnbind;
    }

    return NPSGenNode1(b, op1);
}


/*------------------------------------------------------------------------*/
/** �����Q�̃I�y���[�^�m�[�h���쐬
 *
 * @param op		[in] �I�y�R�[�h
 * @param op1		[in] �����P
 * @param op2		[in] �����Q
 *
 * @return			�m�[�h
 */

nps_node_t NPSGenOP2(uint32_t op, nps_node_t op1, nps_node_t op2)
{
    uint32_t	b = KNPSUnknownCode;

    switch (op)
    {
        case '+':
            b = kNPSAdd;
            break;

        case '-':
            b = kNPSSubtract;
            break;

        case '=':
            b = kNPSEquals;
            break;

        case kNPS_NOT_EQUAL:
            b = kNPSNotEqual;
            break;

        case kNPS_OBJECT_EQUAL:
            b = kNPSObjectEqual;
            break;

        case '<':
            b = kNPSLessThan;
            break;

        case '>':
            b = kNPSGreaterThan;
            break;

        case kNPS_GREATER_EQUAL:
            b = kNPSGreaterOrEqual;
            break;

        case kNPS_LESS_EQUAL:
            b = kNPSLessOrEqual;
            break;

        case kNPS_AND:
            b = kNPSAnd;
            break;

        case kNPS_OR:
            b = kNPSOr;
            break;

        case '*':
            b = kNPSMultiply;
            break;

        case '/':
            b = kNPSDivide;
            break;

        case kNPS_DIV:
            b = kNPSDiv;
            break;

        case kNPS_MOD:
            b = kNPSMod;
            break;

		case kNPS_SHIFT_LEFT:
            b = kNPSShiftLeft;
            break;

		case kNPS_SHIFT_RIGHT:
            b = kNPSShiftRight;
            break;

        case '&':
            b = kNPSConcat;
            break;

        case kNPS_CONCAT2:
            b = kNPSConcat2;
            break;

        default:
            NPSError(kNErrSyntaxError);
            return kNewtRefUnbind;
    }

    return NPSGenNode2(b, op1, op2);
}


/*------------------------------------------------------------------------*/
/** ���b�Z�[�W���M�̃I�y�m�[�h���쐬
 *
 * @param receiver	[in] ���V�[�o
 * @param op		[in] �I�y�R�[�h
 * @param msg		[in] ���b�Z�[�W
 * @param args		[in] ���b�Z�[�W�̈���
 *
 * @return			�m�[�h
 */

nps_node_t NPSGenSend(nps_node_t receiver,
                uint32_t op, nps_node_t msg, nps_node_t args)
{
    nps_node_t	node;

    op = (op == ':')?kNPSSend:kNPSSendIfDefined;

    node = NPSGenNode2(kNPSMessage, msg, args);
    return NPSGenNode2(op, receiver, node);
}


/*------------------------------------------------------------------------*/
/** ���b�Z�[�W�đ��M�̃I�y�m�[�h���쐬
 *
 * @param op		[in] �I�y�R�[�h
 * @param msg		[in] ���b�Z�[�W
 * @param args		[in] ���b�Z�[�W�̈���
 *
 * @return			�m�[�h
 */

nps_node_t NPSGenResend(uint32_t op, nps_node_t msg, nps_node_t args)
{
    op = (op == ':')?kNPSResend:kNPSResendIfDefined;

    return NPSGenNode2(op, msg, args);
}


//--------------------------------------------------------------------------
/** �������̃I�y�m�[�h���쐬
 *
 * @param cond		[in] ������
 * @param ifthen	[in] THEN ��
 * @param ifelse	[in] ELSE ��
 *
 * @return			�m�[�h
 */

nps_node_t NPSGenIfThenElse(nps_node_t cond, nps_node_t ifthen, nps_node_t ifelse)
{
    nps_node_t	node;

    if (ifelse == kNewtRefUnbind)
    {
        node = ifthen;
    }
    else
    {
        node = NewtMakeArray(kNewtRefNIL, 2);
        NewtSetArraySlot(node, 0, ifthen);
        NewtSetArraySlot(node, 1, ifelse);
    }

    return NPSGenNode2(kNPSIf, cond, node);
}


//--------------------------------------------------------------------------
/** FOR ���̃I�y�m�[�h���쐬
 *
 * @param index		[in] �C���f�b�N�X�ϐ�
 * @param v			[in] �����l
 * @param to		[in] �I���l
 * @param by		[in] �X�e�b�v�l
 * @param expr		[in] �J��Ԃ���
 *
 * @return			�m�[�h
 */

nps_node_t NPSGenForLoop(nps_node_t index, nps_node_t v,
                nps_node_t to, nps_node_t by, nps_node_t expr)
{
    newtRefVar	r;

    r = NewtMakeArray(kNewtRefNIL, 4);
    NewtSetArraySlot(r, 0, index);
    NewtSetArraySlot(r, 1, v);
    NewtSetArraySlot(r, 2, to);
    NewtSetArraySlot(r, 3, by);

    return NPSGenNode2(kNPSFor, r, expr);
}


//--------------------------------------------------------------------------
/** FOREACH ���̃I�y�m�[�h���쐬
 *
 * @param index		[in] �C���f�b�N�X�ϐ�
 * @param val		[in] �l���i�[����ϐ�
 * @param obj		[in] ���[�v�̑ΏۂƂȂ�I�u�W�F�N�g
 * @param deeply	[in] deeply �t���O
 * @param op		[in] �I�y���[�V������ʁiDO or COLLECT�j
 * @param expr		[in] �J��Ԃ���
 *
 * @return			�m�[�h
 */

nps_node_t NPSGenForeach(nps_node_t index, nps_node_t val, nps_node_t obj,
                nps_node_t deeply, nps_node_t op, nps_node_t expr)
{
    newtRefVar	r;

    r = NewtMakeArray(kNewtRefNIL, 5);
    NewtSetArraySlot(r, 0, index);
    NewtSetArraySlot(r, 1, val);
    NewtSetArraySlot(r, 2, obj);
    NewtSetArraySlot(r, 3, deeply);
    NewtSetArraySlot(r, 4, op);

    return NPSGenNode2(kNPSForeach, r, expr);
}


//--------------------------------------------------------------------------
/** �O���[�o���֐��̃I�y�m�[�h���쐬
 *
 * @param name		[in] �֐���
 * @param args		[in] �֐��̈���
 * @param expr		[in] ���s��
 *
 * @return			�m�[�h
 */

nps_node_t NPSGenGlobalFn(nps_node_t name, nps_node_t args, nps_node_t expr)
{
    nps_node_t	node;

    node = NPSGenNode2(kNPSFunc, args, expr);
    return NPSGenNode2(kNPSGlobalFn, name, node);
}


#pragma mark -
//--------------------------------------------------------------------------
/** �Q�ƃp�X�I�u�W�F�N�g�̍쐬
 *
 * @param sym1		[in] �V���{���P
 * @param sym2		[in] �V���{���Q
 *
 * @return			�Q�ƃp�X�I�u�W�F�N�g
 */

newtRef NPSMakePathExpr(newtRefArg sym1, newtRefArg sym2)
{
    newtRefVar	r;

    r = NewtMakeArray(NSSYM0(pathExpr), 2);
    NewtSetArraySlot(r, 0, sym1);
    NewtSetArraySlot(r, 1, sym2);

    return r;
}


//--------------------------------------------------------------------------
/** �z��I�u�W�F�N�g�̍쐬
 *
 * @param v			[in] �����l
 *
 * @return			�z��I�u�W�F�N�g
 */

newtRef NPSMakeArray(newtRefArg v)
{
    newtRefVar	r;

    if (v == kNewtRefUnbind)
    {
        r = NewtMakeArray(kNewtRefUnbind, 0);
    }
    else
    {
        r = NewtMakeArray(kNewtRefUnbind, 1);
        NewtSetArraySlot(r, 0, v);
    }

    return r;
}


//--------------------------------------------------------------------------
/** �z��I�u�W�F�N�g�̍Ō�ɃI�u�W�F�N�g��ǉ�����
 *
 * @param r			[in] �z��I�u�W�F�N�g
 * @param v			[in] �ǉ�����I�u�W�F�N�g
 *
 * @return			�z��I�u�W�F�N�g
 */

newtRef NPSAddArraySlot(newtRefArg r, newtRefArg v)
{
    NcAddArraySlot(r, v);
    return r;
}


//--------------------------------------------------------------------------
/** �z��I�u�W�F�N�g�̃I�u�W�F�N�g��}������
 *
 * @param r			[in] �z��I�u�W�F�N�g
 * @param p			[in] �}������ʒu
 * @param v			[in] �}����I�u�W�F�N�g
 *
 * @return			�z��I�u�W�F�N�g
 */

newtRef NPSInsertArraySlot(newtRefArg r, uint32_t p, newtRefArg v)
{
    NewtInsertArraySlot(r, p, v);

    return (newtRef)r;
}


//--------------------------------------------------------------------------
/** �t���[���}�b�v�I�u�W�F�N�g�̍쐬
 *
 * @param v			[in] �����l
 *
 * @return			�t���[���}�b�v�I�u�W�F�N�g
 */

newtRef NPSMakeMap(newtRefArg v)
{
    newtRefVar	r;

    if (v == kNewtRefUnbind)
    {
        r = NewtMakeMap(kNewtRefNIL, 0, NULL);
    }
    else
    {
        r = NewtMakeMap(kNewtRefNIL, 1, NULL);
        NewtSetArraySlot(r, 1, v);
    }

    return r;
}


//--------------------------------------------------------------------------
/** �t���[���I�u�W�F�N�g�̍쐬
 *
 * @param slot		[in] �X���b�g�V���{��
 * @param v			[in] �����l
 *
 * @return			�t���[���I�u�W�F�N�g
 */

newtRef NPSMakeFrame(newtRefArg slot, newtRefArg v)
{
    newtRefVar	r;

    if (NewtRefIsNIL(slot))
    {
        r = NcMakeFrame();
    }
    else
    {
        newtRefVar	def[] = {slot, v};

        r = NewtMakeFrame2(1, def);
    }

    return r;
}


//--------------------------------------------------------------------------
/** �t���[���̃X���b�g�ɃI�u�W�F�N�g���Z�b�g����
 *
 * @param r			[in] �t���[���I�u�W�F�N�g
 * @param slot		[in] �X���b�g�V���{��
 * @param v			[in] �I�u�W�F�N�g
 *
 * @return			�t���[���I�u�W�F�N�g
 */

newtRef NPSSetSlot(newtRefArg r, newtRefArg slot, newtRefArg v)
{
    NcSetSlot(r, slot, v);
    return r;
}


//--------------------------------------------------------------------------
/** �o�C�i���I�u�W�F�N�g�̍쐬
 *
 * @param v			[in] �����l
 *
 * @return			�o�C�i���I�u�W�F�N�g
 */

newtRef NPSMakeBinary(newtRefArg v)
{
    newtRefVar	r;

    if (v == kNewtRefUnbind)
    {
        r = NewtMakeBinary(kNewtRefUnbind, NULL, 0, false);
    }
    else if (NewtRefIsString(v))
    {
        r = NcClone(v);
        NcSetClass(r, kNewtRefUnbind);
        NewtBinarySetLength(r, NewtStringLength(v));
    }
    else
    {
        r = NewtMakeBinary(kNewtRefUnbind, NULL, 1, false);
        NewtSetARef(r, 0, v);
    }

    return r;
}


//--------------------------------------------------------------------------
/** �o�C�i���I�u�W�F�N�g�̍Ō�Ƀf�[�^��ǉ�
 *
 * @param r			[in] �o�C�i���I�u�W�F�N�g
 * @param v			[in] �ǉ�����f�[�^
 *
 * @return			�o�C�i���I�u�W�F�N�g
 */

newtRef NPSAddARef(newtRefArg r, newtRefArg v)
{
    int32_t	len;

    len = NewtBinaryLength(r);
    NewtBinarySetLength(r, len + 1);
    NewtSetARef(r, len, v);

    return r;
}


#pragma mark -
//--------------------------------------------------------------------------
/** �G���[���b�Z�[�W�̕\��
 *
 * @param c			[in] �G���[���
 * @param s			[in] �G���[���b�Z�[�W
 *
 * @return			�Ȃ�
 */

void NPSErrorStr(char c, char * s)
{
    NewtFprintf(stderr, "%c:", c);

	if (nps_env.fname != NULL)
		NewtFprintf(stderr, "\"%s\" ", nps_env.fname);

    NewtFprintf(stderr, "lines %d: %s:\n%s\n", nps_env.lineno, s, nps_env.linebuf);
    NewtFprintf(stderr, "%*s\n", nps_env.tokenpos - nps_env.yyleng + 1, "^");
}


//--------------------------------------------------------------------------
/** �\���G���[
 *
 * @param err		[in] �G���[�R�[�h
 *
 * @return			�Ȃ�
 */

void NPSError(int32_t err)
{
    yyerror("E:Syntax error");
}
