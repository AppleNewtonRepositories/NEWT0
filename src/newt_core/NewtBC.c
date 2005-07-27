/*------------------------------------------------------------------------*/
/**
 * @file	NewtBC.c
 * @brief   �o�C�g�R�[�h�̐���
 *
 * @author  M.Nukui
 * @date	2003-11-07
 *
 * Copyright (C) 2003-2004 M.Nukui All rights reserved.
 */


/* �w�b�_�t�@�C�� */
#include <stdio.h>
#include <stdlib.h>

#include "NewtCore.h"
#include "NewtBC.h"
#include "NewtVM.h"
#include "NewtIO.h"
#include "NewtMem.h"


/* �^�錾 */

/// �o�C�g�R�[�h��
typedef struct nbc_env_t	nbc_env_t;

/// �o�C�g�R�[�h���i�\���̒�`�j
struct nbc_env_t {
    nbc_env_t *	parent;			///< �ďo������

	newtStack   bytecode;		///< �o�C�g�R�[�h�o�b�t�@
	newtStack   breakstack;		///< �u���[�N�X�^�b�N
	newtStack   onexcpstack;	///< ��O�X�^�b�N

    newtRefVar	func;			///< �֐��I�u�W�F�N�g
    newtRefVar	literals;		///< �֐��I�u�W�F�N�g�̃��e�����t���[��
    newtRefVar	argFrame;		///< �֐��I�u�W�F�N�g�̃t���[��
    newtRefVar	constant;		///< �萔�t���[��
};


/// �֐����߃e�[�u���\����
typedef struct {
    char *		name;		///< �֐���
    int32_t		numArgs;	///< �����̐�
    int16_t		b;			///< �o�C�g�R�[�h
    newtRefVar	sym;		///< �V���{��
} freq_func_t;


/* �֐��v���g�^�C�v */
#define	ENV_BC(env)				((uint8_t*)env->bytecode.stackp)				///< �o�C�g�R�[�h
#define	ENV_CX(env)				(env->bytecode.sp)								///< �R�[�h�C���f�b�N�X�i�v���O�����J�E���^�j
#define	BC						ENV_BC(newt_bc_env)								///< �쐬���̃o�C�g�R�[�h
#define	CX						ENV_CX(newt_bc_env)								///< �쐬���̃R�[�h�C���f�b�N�X�i�v���O�����J�E���^�j
#define	BREAKSTACK				((uint32_t*)newt_bc_env->breakstack.stackp)		///< �u���[�N�X�^�b�N
#define BREAKSP					(newt_bc_env->breakstack.sp)					///< �u���[�N�X�^�b�N�̃X�^�b�N�|�C���^
#define	ONEXCPSTACK				((uint32_t*)newt_bc_env->onexcpstack.stackp)	///< ��O�X�^�b�N
#define ONEXCPSP				(newt_bc_env->onexcpstack.sp)					///< ��O�X�^�b�N�̃X�^�b�N�|�C���^
#define	LITERALS				(newt_bc_env->literals)							///< �쐬���֐��I�u�W�F�N�g�̃��e����
#define	ARGFRAME				(newt_bc_env->argFrame)							///< �쐬���֐��I�u�W�F�N�g�̈����t���[��
#define	CONSTANT				(newt_bc_env->constant)							///< �萔�t���[��

#define NBCAddLiteral(r)		NBCAddLiteralEnv(newt_bc_env, r)				///< ���e�������X�g�ɃI�u�W�F�N�g��ǉ�
#define NBCGenCode(a, b)		NBCGenCodeEnv(newt_bc_env, a, b)				///< �o�C�g�R�[�h�𐶐�
#define NBCGenCodeL(a, r)		NBCGenCodeEnvL(newt_bc_env, a, r)				///< ���e�����ȃI�y�f�[�^�̃o�C�g�R�[�h�𐶐�
#define NBCGenPushLiteral(r)	NBCGenPushLiteralEnv(newt_bc_env, r)			///< ���e�������v�b�V������o�C�g�R�[�h�𐶐�

#define NBCGenBC_op(stree, r)   NBCGenBC_stmt(stree, r, true)					///< �����̃o�C�g�R�[�h�𐶐�����
#define NBCGenFreq(b)			NBCGenCode(kNBCFreqFunc, b)						///< �֐����߂̃o�C�g�R�[�h�𐶐�����


#pragma mark -
#pragma mark ���[�J���ϐ�
/* ���[�J���ϐ� */

/// �o�C�h�R�[�h��
static nbc_env_t *	newt_bc_env;

/// �֐����߃e�[�u��
static freq_func_t freq_func_tb[] =
    {
//		{"aref",			2,	kNBCAref,			0},
//		{"setAref",			3,	kNBCSetAref,		0},
        {"BAnd",			2,	kNBCBitAnd,			0},
        {"BOr",				2,	kNBCBitOr,			0},
        {"BNot",			1,	kNBCBitNot,			0},
        {"Length",			1,	kNBCLength,			0},
        {"Clone",			1,	kNBCClone,			0},
        {"SetClass",		2,	kNBCSetClass,		0},
        {"AddArraySlot",	2,	kNBCAddArraySlot,	0},
        {"Stringer",		1,	kNBCStringer,		0},
        {"ClassOf",			1,	kNBCClassOf,		0},

        // end
        {NULL,				0,	0,					0}
    };


#pragma mark -
/* �֐��v���g�^�C�v */
static int16_t			NBCAddLiteralEnv(nbc_env_t * env, newtRefArg r);
static void				NBCGenCodeEnv(nbc_env_t * env, uint8_t a, int16_t b);
static void				NBCGenCodeEnvL(nbc_env_t * env, uint8_t a, newtRefArg r);
static int16_t			NBCGenPushLiteralEnv(nbc_env_t * env, newtRefArg r);

static void				NBCGenPUSH(newtRefArg r);
static void				NBCGenGetVar(nps_syntax_node_t * stree, newtRefArg r);
static void				NBCGenCallFn(newtRefArg fn, int16_t numArgs);
static int16_t			NBCMakeFnArgFrame(newtRefArg argFrame, nps_syntax_node_t * stree, nps_node_t r, bool * indefiniteP);
static int16_t			NBCMakeFnArgs(newtRefArg fn, nps_syntax_node_t * stree, nps_node_t r);
static nbc_env_t *		NBCMakeFnEnv(nps_syntax_node_t * stree, nps_node_t args);
static uint32_t			NBCGenBranch(uint8_t a);
static void				NBCDefLocal(newtRefArg type, newtRefArg r, bool init);
static void				NBCBackPatch(uint32_t cx, int16_t b);
static void				NBCPushBreakStack(uint32_t cx);
static void				NBCBreakBackPatchs(uint32_t loop_head, uint32_t cx);
static void				NBCPushOnexcpStack(uint32_t cx);
static void				NBCOnexcpBackPatchs(uint32_t try_head, uint32_t cx);
static void				NBCGenOnexcpPC(int32_t pc);
static void				NBCGenOnexcpBranch(void);
static void				NBCOnexcpBackPatchL(uint32_t sp, int32_t pc);

static newtRef			NBCMakeFn(nbc_env_t * env);
static void				NBCInitFreqFuncTable(void);
static nbc_env_t *		NBCEnvNew(nbc_env_t * parent);
static void				NBCEnvFree(nbc_env_t * env);
static newtRef			NBCFnDone(nbc_env_t ** envP);

static void				NBCInit(void);
static void				NBCCleanup(void);

static void				NBCGenBC_stmt(nps_syntax_node_t * stree, nps_node_t r, bool ret);
static void				NBCGenConstant(nps_syntax_node_t * stree, nps_node_t r);
static void				NBCGenGlobalVar(nps_syntax_node_t * stree, nps_node_t r);
static void				NBCGenLocalVar(nps_syntax_node_t * stree, nps_node_t type, nps_node_t r);
static bool				NBCTypeValid(nps_node_t node);
static int16_t			NBCGenTryPre(nps_syntax_node_t * stree, nps_node_t r);
static int16_t			NBCGenTryPost(nps_syntax_node_t * stree, nps_node_t r, uint32_t * onexcpspP);
static void				NBCGenTry(nps_syntax_node_t * stree, nps_node_t expr, nps_node_t onexception_list);
static void				NBCGenIfThenElse(nps_syntax_node_t * stree, nps_node_t cond, nps_node_t thenelse, bool ret);
static void				NBCGenAnd(nps_syntax_node_t * stree, nps_node_t op1, nps_node_t op2);
static void				NBCGenOr(nps_syntax_node_t * stree, nps_node_t op1, nps_node_t op2);
static void				NBCGenLoop(nps_syntax_node_t * stree, nps_node_t expr);
static newtRef			NBCMakeTempSymbol(newtRefArg index, newtRefArg val, char * s);
static void				NBCGenFor(nps_syntax_node_t * stree, nps_node_t r, nps_node_t expr);
static void				NBCGenForeach(nps_syntax_node_t * stree, nps_node_t r, nps_node_t expr);
static void				NBCGenWhile(nps_syntax_node_t * stree, nps_node_t cond, nps_node_t expr);
static void				NBCGenRepeat(nps_syntax_node_t * stree, nps_node_t expr, nps_node_t cond);
static void				NBCGenBreak(nps_syntax_node_t * stree, nps_node_t expr);
static void				NBCGenStringer(nps_syntax_node_t * stree, nps_node_t op1, nps_node_t op2, char * dlmt);
static void				NBCGenAsign(nps_syntax_node_t * stree, nps_node_t lvalue, nps_node_t expr, bool ret);
static void				NBCGenExists(nps_syntax_node_t * stree, nps_node_t r);
static void				NBCGenReceiver(nps_syntax_node_t * stree, nps_node_t r);
static void				NBCGenMethodExists(nps_syntax_node_t * stree, nps_node_t receiver, nps_node_t name);
static void				NBCGenFn(nps_syntax_node_t * stree, nps_node_t args, nps_node_t expr);
static void				NBCGenGlobalFn(nps_syntax_node_t * stree, nps_node_t name, nps_node_t fn);
static void				NBCGenCall(nps_syntax_node_t * stree, nps_node_t name, nps_node_t args);
static void				NBCGenInvoke(nps_syntax_node_t * stree, nps_node_t fn, nps_node_t args);
static void				NBCGenFunc2(nps_syntax_node_t * stree, newtRefArg name, nps_node_t op1, nps_node_t op2);
static void				NBCGenSend(nps_syntax_node_t * stree, uint32_t code, nps_node_t receiver, nps_node_t r);
static void				NBCGenResend(nps_syntax_node_t * stree, uint32_t code, nps_node_t name, nps_node_t args);
static void				NBCGenMakeArray(nps_syntax_node_t * stree, nps_node_t klass, nps_node_t r);
static void				NBCGenMakeFrame(nps_syntax_node_t * stree, nps_node_t r);
static void				NVCGenNoResult(bool ret);
static void				NBCGenSyntaxCode(nps_syntax_node_t * stree, nps_syntax_node_t * node, bool ret);
static int16_t			NBCCountNumArgs(nps_syntax_node_t * stree, nps_node_t r);
static newtRef			NBCGenMakeFrameSlots_sub(nps_syntax_node_t * stree, nps_node_t r);
static newtRef			NBCGenMakeFrameSlots(nps_syntax_node_t * stree, nps_node_t r);
static void				NBCGenBC_sub(nps_syntax_node_t * stree, uint32_t n, bool ret);


#pragma mark -
/*------------------------------------------------------------------------*/
/** ���e�������X�g�ɃI�u�W�F�N�g��ǉ�����
 *
 * @param env		[in] �o�C�g�R�[�h��
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�ǉ����ꂽ�ʒu
 */

int16_t NBCAddLiteralEnv(nbc_env_t * env, newtRefArg r)
{
    int16_t	b;

    b = NewtArrayLength(env->literals);
    NcAddArraySlot(env->literals, r);

    return b;
}


/*------------------------------------------------------------------------*/
/** �o�C�g�R�[�h�𐶐�
 *
 * @param env		[in] �o�C�g�R�[�h��
 * @param a			[in] ����
 * @param b			[in] �I�y�f�[�^
 *
 * @return			�Ȃ�
 */

void NBCGenCodeEnv(nbc_env_t * env, uint8_t a, int16_t b)
{
    uint8_t *	bc;
    uint32_t	cx;

	cx = ENV_CX(env);

	if (! NewtStackExpand(&env->bytecode, cx + 3))
		return;

	bc = ENV_BC(env);

    if (a == kNBCFieldMask)
        b = 1;

    if (a != kNBCFieldMask &&
        ((a & kNBCFieldMask) == a ||
        (b != kNBCFieldMask && (b & kNBCFieldMask) == b)))
    {
        bc[cx++] = a | b;
    }
    else
    {
        bc[cx++] = a | kNBCFieldMask;
        bc[cx++] = b >> 8;
        bc[cx++] = b & 0xff;
    }

	ENV_CX(env) = cx;
}


/*------------------------------------------------------------------------*/
/** ���e�����ȃI�y�f�[�^�̃o�C�g�R�[�h�𐶐�
 *
 * @param env		[in] �o�C�g�R�[�h��
 * @param a			[in] ����
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�Ȃ�
 */

void NBCGenCodeEnvL(nbc_env_t * env, uint8_t a, newtRefArg r)
{
    newtRefVar	obj;
    int16_t	b = 0;

    obj = NewtPackLiteral(r);

    // ���e����������
    b = NewtFindArrayIndex(env->literals, obj, 0);

    if (b == -1) // ���e�����ɒǉ�
        b = NBCAddLiteralEnv(env, obj);

    NBCGenCodeEnv(env, a, b);
}


/*------------------------------------------------------------------------*/
/** ���e�������v�b�V������o�C�g�R�[�h�𐶐�
 *
 * @param env		[in] �o�C�g�R�[�h��
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			���e�������X�g�̈ʒu
 */

int16_t NBCGenPushLiteralEnv(nbc_env_t * env, newtRefArg r)
{
    newtRefVar	obj;
    int16_t	b;

    obj = NewtPackLiteral(r);
    b = NBCAddLiteralEnv(env, obj);

    NBCGenCodeEnv(env, kNBCPush, b);

    return b;
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g���v�b�V������o�C�g�R�[�h�𐶐�
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�Ȃ�
 */

void NBCGenPUSH(newtRefArg r)
{
    switch (NewtGetRefType(r, false))
    {
        case kNewtInt30:
            {
                int32_t	n;

                n = NewtRefToInteger(r);

                if (-8192 <= n && n <= 8191)
                    NBCGenCode(kNBCPushConstant, r);
                else
                    NBCGenCodeL(kNBCPush, r);
            }
            break;

        case kNewtNil:
        case kNewtTrue:
        case kNewtUnbind:
            NBCGenCode(kNBCPushConstant, r);
            break;

        case kNewtCharacter:
        case kNewtSpecial:
        case kNewtMagicPointer:
            if ((r & 0xffff0000) == 0)
                NBCGenCode(kNBCPushConstant, r);
            else
                NBCGenCodeL(kNBCPush, r);
            break;

        case kNewtPointer:
        default:
            NBCGenCodeL(kNBCPush, r);
            break;
    }
}


/*------------------------------------------------------------------------*/
/** �ϐ����擾����o�C�g�R�[�h�𐶐�
 *
 * @param stree		[in] �\����
 * @param r			[in] �ϐ����I�u�W�F�N�g
 *
 * @return			�Ȃ�
 */

void NBCGenGetVar(nps_syntax_node_t * stree, newtRefArg r)
{
    if (NewtHasSlot(CONSTANT, r))
    {
        // �萔�̏ꍇ
        newtRefVar	c;

        c = NcGetSlot(CONSTANT, r);

        if (! NPSRefIsSyntaxNode(c) && NewtRefIsLiteral(c))
            NBCGenPUSH(c);
        else
            NBCGenBC_op(stree, c);
    }
    else
    {
        int16_t	b;

        // ���[�J���ϐ�������
        b = NewtFindSlotIndex(ARGFRAME, r);
    
        if (b != -1)
            NBCGenCode(kNBCGetVar, b);
        else
            NBCGenCodeL(kNBCFindVar, r);
    }
}


/*------------------------------------------------------------------------*/
/** �֐��ďo���̃o�C�g�R�[�h�𐶐�
 *
 * @param fn		[in] �֐��I�u�W�F�N�g
 * @param numArgs	[in] �����̐�
 *
 * @return			�Ȃ�
 */

void NBCGenCallFn(newtRefArg fn, int16_t numArgs)
{
    int	i;

    // freq-func �̏ꍇ
    for (i = 0; freq_func_tb[i].name != NULL; i++)
    {
        if (NewtRefEqual(fn, freq_func_tb[i].sym))
        {
            if (numArgs != freq_func_tb[i].numArgs)
            {
                NBError(kNErrWrongNumberOfArgs);
                return;
            }

            NBCGenFreq(freq_func_tb[i].b);
            return;
        }
    }

    // call function
    NBCGenPUSH(fn);
    NBCGenCode(kNBCCall, numArgs);
}


/*------------------------------------------------------------------------*/
/** �֐��̈����t���[�����쐬����
 *
 * @param argFrame		[in] �����t���[��
 * @param stree			[in] �\����
 * @param r				[in] �\���؃m�[�h
 * @param indefiniteP	[out]�s�蒷�t���O
 *
 * @return			�����̐�
 */

int16_t NBCMakeFnArgFrame(newtRefArg argFrame, nps_syntax_node_t * stree, nps_node_t r, bool * indefiniteP)
{
    int16_t	numArgs = 1;

    if (r == kNewtRefUnbind)
        return 0;

	if (*indefiniteP == true)
	{
		NBError(kNErrSyntaxError);
        return 0;
	}

    if (NPSRefIsSyntaxNode(r))
    {
        nps_syntax_node_t * node;

        node = &stree[NPSRefToSyntaxNode(r)];

        switch (node->code)
        {
            case kNPSCommaList:
                numArgs = NBCMakeFnArgFrame(argFrame, stree, node->op1, indefiniteP)
                            + NBCMakeFnArgFrame(argFrame, stree, node->op2, indefiniteP);
                break;

            case kNPSArg:
                // type (node->op1) �͂Ƃ肠��������
                NcSetSlot(argFrame, node->op2, kNewtRefUnbind);
                break;

			case kNPSIndefinite:
                // �s�蒷
				NcSetSlot(argFrame, node->op1, kNewtRefUnbind);
				*indefiniteP = true;
                numArgs = 0;
				break;

            default:
                NBError(kNErrSyntaxError);
                break;
        }
    }
    else
    {
        NcSetSlot(argFrame, r, kNewtRefUnbind);
    }

    return numArgs;
}


/*------------------------------------------------------------------------*/
/** �֐��I�u�W�F�N�g�̈������쐬����
 *
 * @param fn		[in] �֐��I�u�W�F�N�g
 * @param stree		[in] �\����
 * @param r			[in] �\���؃m�[�h
 *
 * @return			�����̐�
 */

int16_t NBCMakeFnArgs(newtRefArg fn, nps_syntax_node_t * stree, nps_node_t r)
{
    int16_t	numArgs = 0;

    if (r != kNewtRefUnbind)
    {
        newtRefVar	argFrame;
		bool		indefinite = false;

        argFrame = NcGetSlot(fn, NSSYM0(argFrame));
        numArgs = NBCMakeFnArgFrame(argFrame, stree, r, &indefinite);

        if (0 < numArgs)
            NcSetSlot(fn, NSSYM0(numArgs), NewtMakeInteger(numArgs));
 
		if (indefinite)
            NcSetSlot(fn, NSSYM0(indefinite), kNewtRefTRUE);
	}

    return numArgs;
}


/*------------------------------------------------------------------------*/
/** �֐��̃o�C�g�R�[�h�����쐬����
 *
 * @param stree		[in] �\����
 * @param args		[in] ����
 *
 * @return			�o�C�g�R�[�h��
 */

nbc_env_t * NBCMakeFnEnv(nps_syntax_node_t * stree, nps_node_t args)
{
    newt_bc_env = NBCEnvNew(newt_bc_env);

	if (newt_bc_env != NULL)
		NBCMakeFnArgs(newt_bc_env->func, stree, args);

    return newt_bc_env;
}


/*------------------------------------------------------------------------*/
/** ���򖽗߂̃o�C�g�R�[�h�𐶐�
 *
 * @param a			[in] ����
 *
 * @return			�o�C�g�R�[�h�̈ʒu
 */

uint32_t NBCGenBranch(uint8_t a)
{
    uint32_t	cx;

    cx = CX;
    NBCGenCode(a, 0xffff);	// 0xffff is dummy

    return cx;
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** ���[�J���ϐ����`����o�C�g�R�[�h�𐶐�
 *
 * @param type		[in] �ϐ��̌^
 * @param r			[in] �ϐ����V���{��
 * @param init		[in] ������
 *
 * @return			�Ȃ�
 *
 * @note			���� init �� true �Ȃ�Ώ���������o�C�g�R�[�h�𐶐�����
 *					���݂̃o�[�W�����ł� type �͊��S�ɖ��������
 */

void NBCDefLocal(newtRefArg type, newtRefArg r, bool init)
{
    NcSetSlot(ARGFRAME, r, kNewtRefUnbind);

    if (init)
    {
        int16_t	b;
    
        // ���[�J���ϐ�������
        b = NewtFindSlotIndex(ARGFRAME, r);

        if (b != -1)
            NBCGenCode(kNBCSetVar, b);
        else
            NewtFprintf(stderr, "Not inpriment");
    }
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �o�C�g�R�[�h���o�b�N�p�b�`����
 *
 * @param cx		[in] �o�b�N�p�b�`����ʒu
 * @param b			[in] �o�b�N�p�b�`����I�y�f�[�^
 *
 * @return			�Ȃ�
 *
 *�@@note				���򖽗߂⃋�[�v���߂Ȃǂ����ɃI�y�f�[�^�����肵�Ȃ��ꍇ�Ɏg��
 */

void NBCBackPatch(uint32_t cx, int16_t b)
{
    BC[cx + 1] = b >> 8;
    BC[cx + 2] = b & 0xff;
}


/*------------------------------------------------------------------------*/
/** �u���[�N���߂̈ʒu���X�^�b�N����
 *
 * @param cx		[in] �u���[�N���߂̈ʒu
 *
 * @return			�Ȃ�
 *
 *�@@note				�o�b�N�p�b�`�̂��߂Ɋo���Ă���
 */

void NBCPushBreakStack(uint32_t cx)
{
    if (BREAKSTACK == NULL)
		NewtStackSetup(&newt_bc_env->breakstack, NEWT_POOL, sizeof(uint32_t), NEWT_NUM_BREAKSTACK);

	if (! NewtStackExpand(&newt_bc_env->breakstack, BREAKSP + 1))
		return;

    BREAKSTACK[BREAKSP] = cx;
    BREAKSP++;
}


/*------------------------------------------------------------------------*/
/** ���[�v���̃u���[�N���߂��o�b�N�p�b�`����
 *
 * @param loop_head	[in] ���[�v�̊J�n�ʒu
 * @param cx		[in] ���[�v�̏I���ʒu
 *
 * @return			�Ȃ�
 */

void NBCBreakBackPatchs(uint32_t loop_head, uint32_t cx)
{
    uint32_t	branch;

    for (; 0 < BREAKSP; BREAKSP--)
    {
        branch = BREAKSTACK[BREAKSP - 1];

        if (branch < loop_head)
            break;

        NBCBackPatch(branch, cx);	// �u�����`���o�b�N�p�b�`
    }
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** ��O�������߂̈ʒu���X�^�b�N����
 *
 * @param cx		[in] ��O�������߂̈ʒu
 *
 * @return			�Ȃ�
 *
 *�@@note				�o�b�N�p�b�`�̂��߂Ɋo���Ă���
 */

void NBCPushOnexcpStack(uint32_t cx)
{
    if (ONEXCPSTACK == NULL)
		NewtStackSetup(&newt_bc_env->onexcpstack, NEWT_POOL, sizeof(uint32_t), NEWT_NUM_ONEXCPSTACK);

	if (! NewtStackExpand(&newt_bc_env->onexcpstack, ONEXCPSP + 1))
		return;

    ONEXCPSTACK[ONEXCPSP] = cx;
    ONEXCPSP++;
}


/*------------------------------------------------------------------------*/
/** TRY�����̃u���[�N���߂��o�b�N�p�b�`����
 *
 * @param try_head	[in] TRY���̊J�n�ʒu
 * @param cx		[in] TRY���̏I���ʒu
 *
 * @return			�Ȃ�
 */

void NBCOnexcpBackPatchs(uint32_t try_head, uint32_t cx)
{
    uint32_t	branch;

    for (; 0 < ONEXCPSP; ONEXCPSP--)
    {
        branch = ONEXCPSTACK[ONEXCPSP - 1];

        if (branch < try_head)
            break;

        NBCBackPatch(branch, cx);	// �u�����`���o�b�N�p�b�`
    }
}


/*------------------------------------------------------------------------*/
/** ��O�������߂̃o�C�g�R�[�h�𐶐�����
 *
 * @param pc		[in] ��O�������߂̃v���O�����J�E���^
 *
 * @return			�Ȃ�
 */

void NBCGenOnexcpPC(int32_t pc)
{
    newtRefVar	r;
    int16_t	b;

    r = NewtMakeInteger(pc);
    b = NBCGenPushLiteral(r);

    NBCPushOnexcpStack(b);	// �o�b�N�p�b�`�̂��߂ɃX�^�b�N�Ƀv�b�V������
}


/*------------------------------------------------------------------------*/
/** ��O�������̃u���[�N���߂̃o�C�g�R�[�h�𐶐�����
 *
 * @return			�Ȃ�
 */

void NBCGenOnexcpBranch(void)
{
    uint32_t	cx;
    
    cx = NBCGenBranch(kNBCBranch);	// �u�����`
    NBCPushOnexcpStack(cx);			// �o�b�N�p�b�`�̂��߂ɃX�^�b�N�Ƀv�b�V������
}


/*------------------------------------------------------------------------*/
/** �o�b�N�p�b�`���ɗ�O�����V���{�������e�������X�g�ɓo�^����
 *
 * @param sp		[in] ��O�������߃X�^�b�N�̃X�^�b�N�|�C���^
 * @param pc		[in] ��O�������߂̃v���O�����J�E���^
 *
 * @return			�Ȃ�
 */

void NBCOnexcpBackPatchL(uint32_t sp, int32_t pc)
{
    newtRefVar	r;

    if (ONEXCPSTACK == NULL || ONEXCPSP <= sp)
        return;

    r = NewtMakeInteger(pc);
    NewtSetArraySlot(LITERALS, ONEXCPSTACK[sp], r);
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �֐��I�u�W�F�N�g���쐬����
 *
 * @param env		[in] �o�C�g�R�[�h��
 *
 * @return			�֐��I�u�W�F�N�g
 */

newtRef NBCMakeFn(nbc_env_t * env)
{
    newtRefVar	fnv[] = {
                            NS_CLASS,				NSSYM0(CodeBlock),
                            NSSYM0(instructions),	kNewtRefNIL,
                            NSSYM0(literals),		kNewtRefNIL,
                            NSSYM0(argFrame),		kNewtRefNIL,
                            NSSYM0(numArgs),		kNewtRefNIL
                        };

    newtRefVar	afv[] = {
                            NSSYM0(_nextArgFrame),	kNewtRefNIL,
                            NSSYM0(_parent),		kNewtRefNIL,
                            NSSYM0(_implementor),	kNewtRefNIL
                        };

    newtRefVar	fn;
    int32_t	numArgs = 0;

    // literals
    env->literals = NewtMakeArray(NSSYM0(literals), 0);

    // argFrame
    env->argFrame = NewtMakeFrame2(sizeof(afv) / (sizeof(newtRefVar) * 2), afv);

    // function
    fn = NewtMakeFrame2(sizeof(fnv) / (sizeof(newtRefVar) * 2), fnv);

//    NcSetClass(fn, NSSYM0(CodeBlock));
    NcSetSlot(fn, NSSYM0(instructions), kNewtRefNIL);
    NcSetSlot(fn, NSSYM0(literals), env->literals);
    NcSetSlot(fn, NSSYM0(argFrame), env->argFrame);
    NcSetSlot(fn, NSSYM0(numArgs), NewtMakeInteger(numArgs));

    env->func = fn;

    // constant
	if (env->parent)
	{	// �e������Β萔�t���[�������L����
		env->constant = env->parent->constant;
	}
	else
	{	// �e���Ȃ���ΐV�K�ɒ萔�t���[�����쐬
		env->constant = NcMakeFrame();
	}

    return fn;
}


/*------------------------------------------------------------------------*/
/** �֐����߃e�[�u��������������
 *
 * @return			�Ȃ�
 */

void NBCInitFreqFuncTable(void)
{
    int	i;

    for (i = 0; freq_func_tb[i].name != NULL; i++)
    {
        freq_func_tb[i].sym = NewtMakeSymbol(freq_func_tb[i].name);
    }
}


/*------------------------------------------------------------------------*/
/** �o�C�g�R�[�h�����쐬����
 *
 * @param parent	[in] �ďo�����̃o�C�g�R�[�h��
 *
 * @return			�o�C�g�R�[�h��
 */

nbc_env_t * NBCEnvNew(nbc_env_t * parent)
{
    nbc_env_t *	env;

    env = (nbc_env_t *)NewtMemCalloc(NEWT_POOL, 1, sizeof(nbc_env_t));

	if (env != NULL)
	{
		env->parent = parent;
		NewtStackSetup(&env->bytecode, NEWT_POOL, sizeof(uint8_t), NEWT_NUM_BYTECODE);

		NBCMakeFn(env);
	}

    return env;
}


/*------------------------------------------------------------------------*/
/** �o�C�g�R�[�h�����������
 *
 * @param env		[in] �o�C�g�R�[�h��
 *
 * @return			�Ȃ�
 */

void NBCEnvFree(nbc_env_t * env)
{
    if (env != NULL)
    {
		NewtStackFree(&env->bytecode);
		NewtStackFree(&env->breakstack);
		NewtStackFree(&env->onexcpstack);

        NewtMemFree(env);
    }
}


/*------------------------------------------------------------------------*/
/** �֐��I�u�W�F�N�g�̐������I������
 *
 * @param envP		[in] �o�C�g�R�[�h���ւ̃|�C���^
 *
 * @return			�֐��I�u�W�F�N�g
 */

newtRef NBCFnDone(nbc_env_t ** envP)
{
    nbc_env_t *	env = *envP;
    newtRefVar	fn = kNewtRefNIL;

    if (env != NULL)
    {
        newtRefVar	instr;
        newtRefVar	literals;

        NBCGenCodeEnv(env, kNBCReturn, 0);

        fn = env->func;
        instr = NewtMakeBinary(kNewtRefNIL, ENV_BC(env), ENV_CX(env), true);
        NcSetSlot(fn, NSSYM0(instructions), instr);
    
        literals = NcGetSlot(fn, NSSYM0(literals));

        if (NewtRefIsNotNIL(literals) && NcLength(literals) == 0)
            NcSetSlot(fn, NSSYM0(literals), kNewtRefNIL);

        *envP = env->parent;
        NBCEnvFree(env);
    }

    return fn;
}


/*------------------------------------------------------------------------*/
/** �o�C�g�R�[�h�����̂��߂̏�����
 *
 * @return			�Ȃ�
 */

void NBCInit(void)
{
    NBCInitFreqFuncTable();
}


/*------------------------------------------------------------------------*/
/** �o�C�g�R�[�h�����̌�n��
 *
 * @return			�Ȃ�
 */

void NBCCleanup(void)
{
    nbc_env_t *	env;

    while (newt_bc_env != NULL)
    {
        env = newt_bc_env;
        newt_bc_env = env->parent;
        NBCEnvFree(env);
    }
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** ���̃o�C�g�R�[�h�𐶐�����
 *
 * @param stree		[in] �\����
 * @param r			[in] �\���؃m�[�h
 * @param ret		[in] �߂�l�̗L��
 *
 * @return			�Ȃ�
 */

void NBCGenBC_stmt(nps_syntax_node_t * stree, nps_node_t r, bool ret)
{
    if (NPSRefIsSyntaxNode(r))
    {
        NBCGenBC_sub(stree, NPSRefToSyntaxNode(r), ret);
        return;
    }

    if (r != kNewtRefUnbind)
        NBCGenPUSH(r);
}


/*------------------------------------------------------------------------*/
/** �萔�̃o�C�g�R�[�h�𐶐�����
 *
 * @param stree		[in] �\����
 * @param r			[in] �\���؃m�[�h
 *
 * @return			�Ȃ�
 */

void NBCGenConstant(nps_syntax_node_t * stree, nps_node_t r)
{
    if (NPSRefIsSyntaxNode(r))
    {
        nps_syntax_node_t * node;

        node = &stree[NPSRefToSyntaxNode(r)];

        switch (node->code)
        {
            case kNPSCommaList:
                NBCGenConstant(stree, node->op1);
                NBCGenConstant(stree, node->op2);
                break;

            case kNPSAsign:
                // node->op2 ���I�u�W�F�N�g�łȂ��ꍇ�̏������s������
                NcSetSlot(CONSTANT, node->op1, node->op2);
                break;
        }
    }
    else
    {
        NBError(kNErrSyntaxError);
    }
}


/*------------------------------------------------------------------------*/
/** �O���[�o���ϐ��̃o�C�g�R�[�h�𐶐�����
 *
 * @param stree		[in] �\����
 * @param r			[in] �\���؃m�[�h
 *
 * @return			�Ȃ�
 */

void NBCGenGlobalVar(nps_syntax_node_t * stree, nps_node_t r)
{
    if (NPSRefIsSyntaxNode(r))
    {
        nps_syntax_node_t * node;

        node = &stree[NPSRefToSyntaxNode(r)];

        switch (node->code)
        {
            case kNPSCommaList:
                NBCGenGlobalVar(stree, node->op1);
                NBCGenGlobalVar(stree, node->op2);
                break;

            case kNPSAsign:
                NBCGenPUSH(node->op1);
                NBCGenBC_op(stree, node->op2);

                // defGlobalVar ���ďo��
                NBCGenCallFn(NSSYM0(defGlobalVar), 2);
                break;
        }
    }
    else if (NewtRefIsSymbol(r))
    {
        NBCGenPUSH(r);
        NBCGenPUSH(kNewtRefUnbind);

        // defGlobalVar ���ďo��
        NBCGenCallFn(NSSYM0(defGlobalVar), 2);
    }
    else
    {
        NBError(kNErrSyntaxError);
    }
}


/*------------------------------------------------------------------------*/
/** ���[�J���ϐ��̃o�C�g�R�[�h�𐶐�����
 *
 * @param stree		[in] �\����
 * @param type		[in] �^
 * @param r			[in] �\���؃m�[�h
 *
 * @return			�Ȃ�
 *
 * @note			���݂̃o�[�W�����ł� type �͊��S�ɖ��������
 */

void NBCGenLocalVar(nps_syntax_node_t * stree, nps_node_t type, nps_node_t r)
{
    if (NPSRefIsSyntaxNode(r))
    {
        nps_syntax_node_t * node;

        node = &stree[NPSRefToSyntaxNode(r)];

        switch (node->code)
        {
            case kNPSCommaList:
                NBCGenLocalVar(stree, type, node->op1);
                NBCGenLocalVar(stree, type, node->op2);
                break;

            case kNPSAsign:
                NBCGenBC_op(stree, node->op2);
                NBCDefLocal(type, node->op1, true);
                break;
        }
    }
    else if (NewtRefIsSymbol(r))
    {
        NBCDefLocal(type, r, false);
    }
    else
    {
        NBError(kNErrSyntaxError);
    }
}


/*------------------------------------------------------------------------*/
/** �^�����������`�F�b�N����
 *
 * @param type		[in] �^
 *
 * @retval			true	�������^
 * @retval			false	�������Ȃ��^
 */

bool NBCTypeValid(nps_node_t node)
{
    if (node == kNewtRefUnbind)
        return true;

    if (node == NS_INT)
        return true;

    if (node == NSSYM0(array))
        return true;

    NBError(kNErrSyntaxError);

    return false;
}


/*------------------------------------------------------------------------*/
/** TRY���̐擪�̃o�C�g�R�[�h�𐶐�����
 *
 * @param stree		[in] �\����
 * @param r			[in] �\���؃m�[�h
 *
 * @return			��O�����̐�
 */

int16_t NBCGenTryPre(nps_syntax_node_t * stree, nps_node_t r)
{
    int16_t	numExcps = 0;

    if (NPSRefIsSyntaxNode(r))
    {
        nps_syntax_node_t * node;

        node = &stree[NPSRefToSyntaxNode(r)];

        switch (node->code)
        {
            case kNPSOnexception:
                NBCGenPUSH(node->op1);	// �V���{��
                NBCGenOnexcpPC(-1);	// PC�i�_�~�[�j

                numExcps = 1;
                break;

            case kNPSOnexceptionList:
                numExcps = NBCGenTryPre(stree, node->op1)
                            + NBCGenTryPre(stree, node->op2);
                break;
        }
    }

    return numExcps;
}


/*------------------------------------------------------------------------*/
/** TRY���̏I���̃o�C�g�R�[�h�𐶐�����
 *
 * @param stree		[in] �\����
 * @param r			[in] �\���؃m�[�h
 * @param onexcpspP	[in] ��O�������߂̏��Ԃւ̃|�C���^
 *
 * @return			��O�����̐�
 */

int16_t NBCGenTryPost(nps_syntax_node_t * stree, nps_node_t r,
            uint32_t * onexcpspP)
{
    int16_t	numExcps = 0;

    if (NPSRefIsSyntaxNode(r))
    {
        nps_syntax_node_t * node;

        node = &stree[NPSRefToSyntaxNode(r)];

        switch (node->code)
        {
            case kNPSOnexception:
                // new-handler �̈������o�b�N�p�b�`
                NBCOnexcpBackPatchL(*onexcpspP, CX);
                (*onexcpspP)++;

                // onexception �̃R�[�h����
                NBCGenBC_stmt(stree, node->op2, true);
                NBCGenOnexcpBranch();

                numExcps = 1;
                break;

            case kNPSOnexceptionList:
                numExcps = NBCGenTryPost(stree, node->op1, onexcpspP)
                            + NBCGenTryPost(stree, node->op2, onexcpspP);
                break;
        }
    }

    return numExcps;
}


/*------------------------------------------------------------------------*/
/** TRY���̃o�C�g�R�[�h�𐶐�����
 *
 * @param stree		[in] �\����
 * @param expr		[in] ���̍\���؃m�[�h
 * @param onexception_list	[in] ��O�����̍\���؃m�[�h
 *
 * @return			�Ȃ�
 */

void NBCGenTry(nps_syntax_node_t * stree, nps_node_t expr,
        nps_node_t onexception_list)
{
    uint32_t	onexcp_cx;
    uint32_t	branch_cx;
    uint32_t	onexcpsp;
    int16_t	numExcps = 0;

    onexcpsp = ONEXCPSP;
    numExcps = NBCGenTryPre(stree, onexception_list);
    NBCGenCode(kNBCNewHandlers, numExcps);

    // ���s��
    NBCGenBC_op(stree, expr);
    NBCGenCode(kNBCPopHandlers, 0);

    branch_cx = CX;
    branch_cx = NBCGenBranch(kNBCBranch);

    // onexception
    onexcp_cx = CX;
    NBCGenTryPost(stree, onexception_list, &onexcpsp);

    // onexception �̏I��
    NBCOnexcpBackPatchs(onexcp_cx, CX);	// onexception �̏I�����o�b�N�p�b�`
    NBCGenCode(kNBCPopHandlers, 0);

    // ONEXCPSP ��߂�
    ONEXCPSP = onexcpsp;

    // �o�b�N�p�b�`
    NBCBackPatch(branch_cx, CX);	// branch ���o�b�N�p�b�`
}


/*------------------------------------------------------------------------*/
/** IF���̃o�C�g�R�[�h�𐶐�����
 *
 * @param stree		[in] �\����
 * @param cond		[in] �������̍\���؃m�[�h
 * @param thenelse	[in] THEN, ELSE �̍\���؃m�[�h
 * @param ret		[in] �߂�l�̗L��
 *
 * @return			�Ȃ�
 */

void NBCGenIfThenElse(nps_syntax_node_t * stree, nps_node_t cond,
        nps_node_t thenelse, bool ret)
{
    nps_node_t	ifthen;
    nps_node_t	ifelse = kNewtRefUnbind;
    uint32_t	cond_cx;

    NBCGenBC_op(stree, cond);
    cond_cx = NBCGenBranch(kNBCBranchIfFalse);

    if (NewtRefIsArray(thenelse))
    {
        ifthen = NewtGetArraySlot(thenelse, 0);
        ifelse = NewtGetArraySlot(thenelse, 1);
    }
    else
    {
        ifthen = thenelse;
    }

    // THEN ��
    NBCGenBC_stmt(stree, ifthen, ret);

    if (ifelse == kNewtRefUnbind)
    {
        NBCBackPatch(cond_cx, CX);				// ���������o�b�N�p�b�`
    }
    else
    {
        uint32_t	then_done;

        then_done = NBCGenBranch(kNBCBranch);	// THEN ���̏I��
        NBCBackPatch(cond_cx, CX);				// ���������o�b�N�p�b�`

        // ELSE ��
        NBCGenBC_stmt(stree, ifelse, ret);

        NBCBackPatch(then_done, CX);			// THEN ���I���̃u�����`���o�b�N�p�b�`
    }
}


/*------------------------------------------------------------------------*/
/** �_��AND �̃o�C�g�R�[�h�𐶐�����
 *
 * @param stree		[in] �\����
 * @param code		[in] �\���R�[�h�ikNPSAnd or kNPSOr�j
 * @param op1		[in] �I�y�����h�P�̍\���؃m�[�h
 * @param op2		[in] �I�y�����h�Q�̍\���؃m�[�h
 *
 * @return			�Ȃ�
 */

void NBCGenAnd(nps_syntax_node_t * stree, nps_node_t op1, nps_node_t op2)
{
    uint32_t	cx1;
    uint32_t	cx2;

	// �I�y�����h�P
    NBCGenBC_op(stree, op1);

	// NIL �Ȃ番��
    cx1 = NBCGenBranch(kNBCBranchIfFalse);

    // �I�y�����h�Q
    NBCGenBC_op(stree, op2);
	// ���̍Ō�֕���
    cx2 = NBCGenBranch(kNBCBranch);

	// �߂�l���v�b�V��
	NBCBackPatch(cx1, CX);		// ������o�b�N�p�b�`
    NBCGenPUSH(kNewtRefNIL);	// �߂�l�� NIL

	// ���̍Ō�
	NBCBackPatch(cx2, CX);		// ������o�b�N�p�b�`
}


/*------------------------------------------------------------------------*/
/** �_��OR �̃o�C�g�R�[�h�𐶐�����
 *
 * @param stree		[in] �\����
 * @param op1		[in] �I�y�����h�P�̍\���؃m�[�h
 * @param op2		[in] �I�y�����h�Q�̍\���؃m�[�h
 *
 * @return			�Ȃ�
 */

void NBCGenOr(nps_syntax_node_t * stree, nps_node_t op1, nps_node_t op2)
{
    uint32_t	cx1;
    uint32_t	cx2;

	// �I�y�����h�P
    NBCGenBC_op(stree, op1);

	// TRUE �Ȃ番��
    cx1 = NBCGenBranch(kNBCBranchIfTrue);

    // �I�y�����h�Q
    NBCGenBC_op(stree, op2);
	// ���̍Ō�֕���
    cx2 = NBCGenBranch(kNBCBranch);

	// �߂�l���v�b�V��
	NBCBackPatch(cx1, CX);		// ������o�b�N�p�b�`

	if (NPSRefIsSyntaxNode(op1))
		NBCGenPUSH(kNewtRefTRUE);
	else
		NBCGenPUSH(op1);

	// ���̍Ō�
	NBCBackPatch(cx2, CX);		// ������o�b�N�p�b�`
}


/*------------------------------------------------------------------------*/
/** LOOP���̃o�C�g�R�[�h�𐶐�����
 *
 * @param stree		[in] �\����
 * @param expr		[in] ���s���̍\���؃m�[�h
 *
 * @return			�Ȃ�
 */

void NBCGenLoop(nps_syntax_node_t * stree, nps_node_t expr)
{
    uint32_t	loop_head;

    // loop �̐擪
    loop_head = CX;

    // ���s��
    NBCGenBC_stmt(stree, expr, false);

    NBCGenCode(kNBCBranch, loop_head);	// loop �̐擪��

    // �o�b�N�p�b�`
    NBCBreakBackPatchs(loop_head, CX);	// break ���o�b�N�p�b�`
}


/*------------------------------------------------------------------------*/
/** �C�e���[�^�Ŏg�p����ꎞ�I�ȃV���{�����쐬����
 *
 * @param index		[in] �C���f�b�N�X�ϐ��V���{��
 * @param val		[in] �o�����[�ϐ��V���{��
 * @param s			[in] ������
 *
 * @return			�V���{��
 */

newtRef NBCMakeTempSymbol(newtRefArg index, newtRefArg val, char * s)
{
    newtRefVar	str;

    str = NSSTR("");

    NcStrCat(str, index);
    NcStrCat(str, val);
    NewtStrCat(str, "|");
    NewtStrCat(str, s);

    return NcMakeSymbol(str);
}


/*------------------------------------------------------------------------*/
/** FOR���̃o�C�g�R�[�h�𐶐�����
 *
 * @param stree		[in] �\����
 * @param r			[in] �\���؃m�[�h
 * @param expr		[in] ���s���̍\���؃m�[�h
 *
 * @return			�Ȃ�
 */

void NBCGenFor(nps_syntax_node_t * stree, nps_node_t r, nps_node_t expr)
{
    nps_node_t	index;
    nps_node_t	to;
    nps_node_t	by;
    nps_node_t	v;
    newtRefVar	_limit;
    newtRefVar	_incr;
    uint32_t	loop_head;
    uint32_t	branch_cx;

    index = NewtGetArraySlot(r, 0);
    v = NewtGetArraySlot(r, 1);
    to = NewtGetArraySlot(r, 2);
    by = NewtGetArraySlot(r, 3);
 
    if (by == kNewtRefUnbind)
        by = NSINT(1);

    // index ��������
    NBCGenBC_op(stree, v);
    NBCDefLocal(NS_INT, index, true);

    // index|limit ��������
    _limit = NBCMakeTempSymbol(index, kNewtRefUnbind, "limit");
    NBCGenBC_op(stree, to);
    NBCDefLocal(NS_INT, _limit, true);

    // index|limit ��������
    _incr = NBCMakeTempSymbol(index, kNewtRefUnbind, "incr");
    NBCGenBC_op(stree, by);
    NBCDefLocal(NS_INT, _incr, true);

    // �������փu�����`
    NBCGenGetVar(stree, _incr);
    NBCGenGetVar(stree, index);

    branch_cx = NBCGenBranch(kNBCBranch);

    // loop �̐擪
    loop_head = CX;

    // ���s��
    NBCGenBC_stmt(stree, expr, false);

    // �ϐ��� by �𑝕�
    {
        int16_t	b;

        b = NewtFindSlotIndex(ARGFRAME, index);

        NBCGenGetVar(stree, _incr);
        NBCGenCode(kNBCIncrVar, b);
    }

    // ������
    NBCBackPatch(branch_cx, CX);			// branch ���o�b�N�p�b�`
    NBCGenGetVar(stree, _limit);
    NBCGenCode(kNBCBranchIfLoopNotDone, loop_head);	// loop �̐擪��

    // �߂�l
    NBCGenPUSH(kNewtRefNIL);

    // �o�b�N�p�b�`
    NBCBreakBackPatchs(loop_head, CX);	// break ���o�b�N�p�b�`
}


/*------------------------------------------------------------------------*/
/** FOREACH���̃o�C�g�R�[�h�𐶐�����
 *
 * @param stree		[in] �\����
 * @param r			[in] �\���؃m�[�h
 * @param expr		[in] ���s���̍\���؃m�[�h
 *
 * @return			�Ȃ�
 */

void NBCGenForeach(nps_syntax_node_t * stree, nps_node_t r, nps_node_t expr)
{
    nps_node_t	index;
    nps_node_t	val;
    nps_node_t	obj;
    nps_node_t	deeply;
    nps_node_t	op;
    newtRefVar	_iter;
    newtRefVar	_index = kNewtRefUnbind;
    newtRefVar	_result = kNewtRefUnbind;
    uint32_t	loop_head;
    uint32_t	branch_cx;
    bool	collect;

    index = NewtGetArraySlot(r, 0);
    val = NewtGetArraySlot(r, 1);
    obj = NewtGetArraySlot(r, 2);
    deeply = NewtGetArraySlot(r, 3);
    op = NewtGetArraySlot(r, 4);

    collect = (op == NSSYM0(collect));

    NBCDefLocal(kNewtRefUnbind, val, false);
    if (index != kNewtRefUnbind) NBCDefLocal(kNewtRefUnbind, index, false);

    _iter = NBCMakeTempSymbol(index, val, "iter");

    // new-iterator
    NBCGenBC_op(stree, obj);
    NBCGenPUSH(deeply);
    NBCGenFreq(kNBCNewIterator);
    NBCDefLocal(NSSYM0(array), _iter, true);

    if (collect)
    {
        int32_t	lenIndex;

        _index = NBCMakeTempSymbol(index, val, "index");
        _result = NBCMakeTempSymbol(index, val, "result");

        NBCDefLocal(NSSYM0(array), _index, false);
        NBCDefLocal(NSSYM0(array), _result, false);

        // �߂�l�� array ���쐬
        if (NewtRefIsNIL(deeply))
            lenIndex = kIterMax;
        else
            lenIndex = kIterDeeply;

        // length
        NBCGenGetVar(stree, _iter);
        NBCGenPUSH(NewtMakeInteger(lenIndex));
        NBCGenFreq(kNBCAref);
        // make-array
        NBCGenPUSH(kNewtRefUnbind);
        NBCGenCode(kNBCMakeArray, -1);
        NBCDefLocal(NSSYM0(array), _result, true);

        // index �̏�����
        NBCGenPUSH(NSINT(0));
        NBCDefLocal(NS_INT, _index, true);
    }

    // �������փu�����`
    branch_cx = NBCGenBranch(kNBCBranch);

    // loop �̐擪
    loop_head = CX;

    // val �̃Z�b�g
    NBCGenGetVar(stree, _iter);
    NBCGenPUSH(NewtMakeInteger(kIterValue));
    NBCGenFreq(kNBCAref);
    NBCDefLocal(kNewtRefUnbind, val, true);

    // index �̃Z�b�g
    if (index != kNewtRefUnbind)
    {
        NBCGenGetVar(stree, _iter);
        NBCGenPUSH(NewtMakeInteger(kIterIndex));
        NBCGenFreq(kNBCAref);
        NBCDefLocal(kNewtRefUnbind, index, true);
    }

    // ���s��
    if (collect)
    {
        int16_t	b;

        NBCGenGetVar(stree, _result);
        NBCGenGetVar(stree, _index);
        NBCGenBC_stmt(stree, expr, true);
        NBCGenFreq(kNBCSetAref);

        NBCGenCode(kNBCPop, 0);

        //
        b = NewtFindSlotIndex(ARGFRAME, _index);
        NBCGenPUSH(NSINT(1));
        NBCGenCode(kNBCIncrVar, b);

        NBCGenCode(kNBCPop, 0);
        NBCGenCode(kNBCPop, 0);
    }
    else
    {
        NBCGenBC_stmt(stree, expr, false);
    }

    // iter-next
    NBCGenGetVar(stree, _iter);
    NBCGenCode(kNBCIterNext, 0);

    // ������
    NBCBackPatch(branch_cx, CX);		// branch ���o�b�N�p�b�`
    NBCGenGetVar(stree, _iter);
    NBCGenCode(kNBCIterDone, 0);		// iter-done
    NBCGenCode(kNBCBranchIfFalse, loop_head);	// loop �̐擪��

    // �߂�l
    if (collect)
        NBCGenGetVar(stree, _result);
    else
        NBCGenPUSH(kNewtRefNIL);

    // �o�b�N�p�b�`
    NBCBreakBackPatchs(loop_head, CX);	// break ���o�b�N�p�b�`

    // iterator �̌�n��
    if (collect)
    {
        NBCGenPUSH(kNewtRefNIL);
        NBCDefLocal(kNewtRefUnbind, _result, true);
    }

    NBCGenPUSH(kNewtRefNIL);
    NBCDefLocal(kNewtRefUnbind, _iter, true);
}


/*------------------------------------------------------------------------*/
/** WHILE���̃o�C�g�R�[�h�𐶐�����
 *
 * @param stree		[in] �\����
 * @param cond		[in] �������̍\���؃m�[�h
 * @param expr		[in] ���s���̍\���؃m�[�h
 *
 * @return			�Ȃ�
 */

void NBCGenWhile(nps_syntax_node_t * stree, nps_node_t cond, nps_node_t expr)
{
    uint32_t	loop_head;
    uint32_t	cond_cx;

    // loop �̐擪
    loop_head = CX;

    // ������
    NBCGenBC_op(stree, cond);
    cond_cx = NBCGenBranch(kNBCBranchIfFalse);

    // ���s��
    NBCGenBC_stmt(stree, expr, false);

    NBCGenCode(kNBCBranch, loop_head);	// loop �̐擪��

    // �o�b�N�p�b�`
    NBCBackPatch(cond_cx, CX);		// ���������o�b�N�p�b�`

    // �߂�l
    NBCGenPUSH(kNewtRefNIL);

    NBCBreakBackPatchs(loop_head, CX);	// break ���o�b�N�p�b�`
}


/*------------------------------------------------------------------------*/
/** REPEAT���̃o�C�g�R�[�h�𐶐�����
 *
 * @param stree		[in] �\����
 * @param expr		[in] ���s���̍\���؃m�[�h
 * @param cond		[in] �������̍\���؃m�[�h
 *
 * @return			�Ȃ�
 */

void NBCGenRepeat(nps_syntax_node_t * stree, nps_node_t expr, nps_node_t cond)
{
    uint32_t	loop_head;

    // loop �̐擪
    loop_head = CX;

    // ���s��
    NBCGenBC_stmt(stree, expr, false);

    // ������
    NBCGenBC_op(stree, cond);
    NBCGenCode(kNBCBranchIfFalse, loop_head);	// loop �̐擪��

    // �߂�l
    NBCGenPUSH(kNewtRefNIL);

    // �o�b�N�p�b�`
    NBCBreakBackPatchs(loop_head, CX);		// break ���o�b�N�p�b�`
}


/*------------------------------------------------------------------------*/
/** BREAK���̃o�C�g�R�[�h�𐶐�����
 *
 * @param stree		[in] �\����
 * @param expr		[in] ���s���̍\���؃m�[�h
 *
 * @return			�Ȃ�
 */

void NBCGenBreak(nps_syntax_node_t * stree, nps_node_t expr)
{
    uint32_t	cx;

    // �߂�l
    if (expr == kNewtRefUnbind)
        NBCGenPUSH(kNewtRefNIL);
    else
        NBCGenBC_op(stree, expr);

    // �u�����`
    cx = NBCGenBranch(kNBCBranch);	// loop �̏I����
    NBCPushBreakStack(cx);		// �o�b�N�p�b�`�̂��߂ɃX�^�b�N�Ƀv�b�V������
}


/*------------------------------------------------------------------------*/
/** ������̌������߂̃o�C�g�R�[�h�𐶐�����
 *
 * @param stree		[in] �\����
 * @param op1		[in] �����P
 * @param op2		[in] �����Q
 * @param dlmt		[in] ��؂蕶����
 *
 * @return			�Ȃ�
 */

void NBCGenStringer(nps_syntax_node_t * stree, nps_node_t op1, nps_node_t op2,
        char * dlmt)
{
    int16_t numArgs = 2;

    NBCGenBC_op(stree, op1);

    if (dlmt != NULL)
    {
        NBCGenPUSH(NSSTRCONST(dlmt));
        numArgs++;
    }

    NBCGenBC_op(stree, op2);
    NBCGenPUSH(kNewtRefUnbind);
    NBCGenCode(kNBCMakeArray, numArgs);

    NBCGenFreq(kNBCStringer);
}


/*------------------------------------------------------------------------*/
/** ������̃o�C�g�R�[�h�𐶐�����
 *
 * @param stree		[in] �\����
 * @param lvalue	[in] ����
 * @param expr		[in] ��
 * @param ret		[in] �߂�l�̗L��
 *
 * @return			�Ȃ�
 */

void NBCGenAsign(nps_syntax_node_t * stree,
        nps_node_t lvalue, nps_node_t expr, bool ret)
{
    if (NewtRefIsSymbol(lvalue))
    {
        if (NewtHasSlot(CONSTANT, lvalue))
        {
            // �萔�̏ꍇ
            NBError(kNErrAssignToConstant);
            return;
        }

        NBCGenBC_op(stree, expr);
        NBCGenCodeL(kNBCFindAndSetVar, lvalue);

        if (ret)
            NBCGenCodeL(kNBCFindVar, lvalue);
    }
	else if (NewtRefIsMagicPointer(lvalue))
	{
#ifdef __NAMED_MAGIC_POINTER__
		newtRefVar	sym;

		sym = NewtMPToSymbol(lvalue);
		NBCGenFunc2(stree, NSSYM0(defMagicPointer), sym, expr);
		NVCGenNoResult(ret);
#else
		int32_t	index;

		index = NewtMPToIndex(lvalue);
		NBCGenFunc2(stree, NSSYM0(defMagicPointer), NewtMakeInteger(index), expr);
		NVCGenNoResult(ret);
#endif
	}
    else if (NPSRefIsSyntaxNode(lvalue))
    {
        nps_syntax_node_t * node;

        node = &stree[NPSRefToSyntaxNode(lvalue)];

        switch (node->code)
        {
            case kNPSGetPath:
                NBCGenBC_op(stree, node->op1);
                NBCGenBC_op(stree, node->op2);
                NBCGenBC_op(stree, expr);
                NBCGenCode(kNBCSetPath, 1);
                break;

            case kNPSAref:
                NBCGenBC_op(stree, node->op1);
                NBCGenBC_op(stree, node->op2);
                NBCGenBC_op(stree, expr);
                NBCGenFreq(kNBCSetAref);
                break;

            default:
                NBError(kNErrSyntaxError);
                break;
        }
    }
    else
    {
        NBError(kNErrSyntaxError);
    }
}


/*------------------------------------------------------------------------*/
/** EXISTS���̃o�C�g�R�[�h�𐶐�����
 *
 * @param stree		[in] �\����
 * @param r			[in] �\���؃m�[�h
 *
 * @return			�Ȃ�
 */

void NBCGenExists(nps_syntax_node_t * stree, nps_node_t r)
{
    if (NewtRefIsSymbol(r))
    {
        if (NewtFindArrayIndex(LITERALS, r, 0) != -1)
        {
            // ���[�J���ϐ����錾����Ă���ꍇ
            NBCGenPUSH(kNewtRefTRUE);
        }
        else
        {
            NBCGenPUSH(r);
            NBCGenCallFn(NSSYM0(hasVar), 1);
        }
    }
    else if (NPSRefIsSyntaxNode(r))
    {
        nps_syntax_node_t * node;

        node = &stree[NPSRefToSyntaxNode(r)];

        switch (node->code)
        {
            case kNPSGetPath:
                NBCGenBC_op(stree, node->op1);
                NBCGenBC_op(stree, node->op2);
                NBCGenFreq(kNBCHasPath);
                break;

            default:
                NBError(kNErrSyntaxError);
                break;
        }
    }
    else
    {
        NBError(kNErrSyntaxError);
    }
}


/*------------------------------------------------------------------------*/
/** ���V�[�o�̃o�C�g�R�[�h�𐶐�����
 *
 * @param stree		[in] �\����
 * @param r			[in] �\���؃m�[�h
 *
 * @return			�Ȃ�
 */

void NBCGenReceiver(nps_syntax_node_t * stree, nps_node_t r)
{
    if (r == kNewtRefUnbind)
        NBCGenCode(kNBCPushSelf, 0);
    else
        NBCGenBC_op(stree, r);
}


/*------------------------------------------------------------------------*/
/** ���\�b�hEXISTS���̃o�C�g�R�[�h�𐶐�����
 *
 * @param stree		[in] �\����
 * @param receiver  [in] ���V�[�o
 * @param name		[in] ���\�b�h��
 *
 * @return			�Ȃ�
 */

void NBCGenMethodExists(nps_syntax_node_t * stree,
        nps_node_t receiver, nps_node_t name)
{
    // receiver �̐���
    NBCGenReceiver(stree, receiver);

    // name
    NBCGenBC_op(stree, name);

    // hasVariable ���ďo��
    NBCGenCallFn(NSSYM0(hasVariable), 2);
}


/*------------------------------------------------------------------------*/
/** �֐���`�̃o�C�g�R�[�h�𐶐�����
 *
 * @param stree		[in] �\����
 * @param args		[in] ����
 * @param expr		[in] ��
 *
 * @return			�Ȃ�
 */

void NBCGenFn(nps_syntax_node_t * stree, nps_node_t args, nps_node_t expr)
{
    nbc_env_t * env;
    newtRefVar	fn;

    env = NBCMakeFnEnv(stree, args);
    NBCGenBC_op(stree, expr);
    fn = NBCFnDone(&newt_bc_env);
    NBCGenPUSH(fn);
    NBCGenCode(kNBCSetLexScope, 0);
}


/*------------------------------------------------------------------------*/
/** �O���[�o���֐���`�̃o�C�g�R�[�h�𐶐�����
 *
 * @param stree		[in] �\����
 * @param name		[in] �O���[�o���֐���
 * @param fn		[in] �֐�
 *
 * @return			�Ȃ�
 */

void NBCGenGlobalFn(nps_syntax_node_t * stree, nps_node_t name, nps_node_t fn)
{
    NBCGenCodeL(kNBCPush, name);
    NBCGenBC_op(stree, fn);
    NBCGenPUSH(NSSYM0(defGlobalFn));
    NBCGenCode(kNBCCall, 2);
}


/*------------------------------------------------------------------------*/
/** �֐��ďo���̃o�C�g�R�[�h�𐶐�����
 *
 * @param stree		[in] �\����
 * @param name		[in] �O���[�o���֐���
 * @param args		[in] ����
 *
 * @return			�Ȃ�
 */

void NBCGenCall(nps_syntax_node_t * stree, nps_node_t name, nps_node_t args)
{
    int16_t numArgs;

    NBCGenBC_op(stree, args);
    numArgs = NBCCountNumArgs(stree, args);

    NBCGenCallFn(name, numArgs);
}


/*------------------------------------------------------------------------*/
/** �֐��I�u�W�F�N�g���s�̃o�C�g�R�[�h�𐶐�����
 *
 * @param stree		[in] �\����
 * @param name		[in] �֐��I�u�W�F�N�g
 * @param args		[in] ����
 *
 * @return			�Ȃ�
 */

void NBCGenInvoke(nps_syntax_node_t * stree, nps_node_t fn, nps_node_t args)
{
    int16_t numArgs;

    NBCGenBC_op(stree, args);
    numArgs = NBCCountNumArgs(stree, args);

    NBCGenBC_op(stree, fn);
//    NBCGenCode(kNBCSetLexScope, 0);
    NBCGenCode(kNBCInvoke, numArgs);
}


/*------------------------------------------------------------------------*/
/** �Q���֐��̌ďo���o�C�g�R�[�h�𐶐�����
 *
 * @param stree		[in] �\����
 * @param op1		[in] �����P
 * @param op2		[in] �����Q
 *
 * @return			�Ȃ�
 */

void NBCGenFunc2(nps_syntax_node_t * stree,
        newtRefArg name, nps_node_t op1, nps_node_t op2)
{
    NBCGenBC_op(stree, op1);
    NBCGenBC_op(stree, op2);
    NBCGenPUSH(name);
    NBCGenCode(kNBCCall, 2);
}


/*------------------------------------------------------------------------*/
/** ���\�b�h���M�̃o�C�g�R�[�h�𐶐�����
 *
 * @param stree		[in] �\����
 * @param code		[in] ���M�^�C�v
 * @param receiver	[in] ���V�[�o
 * @param r			[in] ���\�b�h���{����
 *
 * @return			�Ȃ�
 */

void NBCGenSend(nps_syntax_node_t * stree, uint32_t code,
        nps_node_t receiver, nps_node_t r)
{
    nps_syntax_node_t * node;
    int16_t numArgs;

    node = &stree[NPSRefToSyntaxNode(r)];

    // �����̐���
    NBCGenBC_op(stree, node->op2);
    numArgs = NBCCountNumArgs(stree, node->op2);

    // message �̐���
    NBCGenPUSH(node->op1);

    // receiver �̐���
    NBCGenReceiver(stree, receiver);

    // ���b�Z�[�W�ďo���̐���
    NBCGenCode(code, numArgs);
}


/*------------------------------------------------------------------------*/
/** ���\�b�h�đ��M�̃o�C�g�R�[�h�𐶐�����
 *
 * @param stree		[in] �\����
 * @param code		[in] ���M�^�C�v
 * @param name		[in] ���\�b�h��
 * @param args		[in] ����
 *
 * @return			�Ȃ�
 */

void NBCGenResend(nps_syntax_node_t * stree, uint32_t code,
        nps_node_t name, nps_node_t args)
{
    int16_t numArgs;

    NBCGenBC_op(stree, args);
    numArgs = NBCCountNumArgs(stree, args);

    NBCGenPUSH(name);
    NBCGenCode(code, numArgs);
}


/*------------------------------------------------------------------------*/
/** �z��쐬�̃o�C�g�R�[�h�𐶐�����
 *
 * @param stree		[in] �\����
 * @param klass		[in] �N���X
 * @param r			[in] �������f�[�^
 *
 * @return			�Ȃ�
 */

void NBCGenMakeArray(nps_syntax_node_t * stree, nps_node_t klass, nps_node_t r)
{
    int16_t n;

    NBCGenBC_op(stree, r);
    n = NBCCountNumArgs(stree, r);

    NBCGenPUSH(klass);
    NBCGenCode(kNBCMakeArray, n);
}


/*------------------------------------------------------------------------*/
/** �t���[���쐬�̃o�C�g�R�[�h�𐶐�����
 *
 * @param stree		[in] �\����
 * @param r			[in] �������f�[�^
 *
 * @return			�Ȃ�
 */

void NBCGenMakeFrame(nps_syntax_node_t * stree, nps_node_t r)
{
    newtRefVar	map;
    uint32_t	n;

    map = NBCGenMakeFrameSlots(stree, r);
    n = NewtMapLength(map);

    NBCGenPUSH(map);
    NBCGenCode(kNBCMakeFrame, n);
}


/*------------------------------------------------------------------------*/
/** �߂�l���s�p�̏ꍇ�̃o�C�g�R�[�h�𐶐�����
 *
 * @param ret		[in] �߂�l�̗L��
 *
 * @return			�Ȃ�
 */

void NVCGenNoResult(bool ret)
{
    if (! ret)
        NBCGenCode(kNBCPop, 0);
}


/*------------------------------------------------------------------------*/
/** �\���R�[�h�̃o�C�g�R�[�h�𐶐�����
 *
 * @param stree		[in] �\����
 * @param node		[in] �\���؃m�[�h
 * @param ret		[in] �߂�l�̗L��
 *
 * @return			�Ȃ�
 */

void NBCGenSyntaxCode(nps_syntax_node_t * stree, nps_syntax_node_t * node, bool ret)
{
    switch (node->code)
    {
        case kNPSConstituentList:
            if (NewtRefIsNIL(node->op2))
            {
                NBCGenBC_stmt(stree, node->op1, ret);
            }
            else
            {
                NBCGenBC_stmt(stree, node->op1, false);
                NBCGenBC_stmt(stree, node->op2, ret);
            }
            break;

        case kNPSCommaList:
            NBCGenBC_op(stree, node->op1);
            NBCGenBC_op(stree, node->op2);
            break;

        case kNPSConstant:
            NBCGenConstant(stree, node->op1);
            break;

        case kNPSGlobal:
            NBCGenGlobalVar(stree, node->op1);
            NVCGenNoResult(ret);
            break;

        case kNPSLocal:
            if (! NBCTypeValid(node->op1))
                return;

            NBCGenLocalVar(stree, node->op1, node->op2);
            break;

        case kNPSFunc:
            NBCGenFn(stree, node->op1, node->op2);
            NVCGenNoResult(ret);
            break;

        case kNPSGlobalFn:
            NBCGenGlobalFn(stree, node->op1, node->op2);
            NVCGenNoResult(ret);
            break;

        case kNPSLvalue:
            if (NewtRefIsSymbol(node->op1))
                NBCGenGetVar(stree, node->op1);
            else
                NBCGenBC_op(stree, node->op1);

            NVCGenNoResult(ret);
            break;

        case kNPSAsign:
            NBCGenAsign(stree, node->op1, node->op2, ret);
            break;

        case kNPSExists:
            NBCGenExists(stree, node->op1);
            NVCGenNoResult(ret);
            break;

        case kNPSMethodExists:
            NBCGenMethodExists(stree, node->op1, node->op2);
            NVCGenNoResult(ret);
            break;

        case kNPSTry:
            NBCGenTry(stree, node->op1, node->op2);
            NVCGenNoResult(ret);
            break;

        case kNPSIf:
            NBCGenIfThenElse(stree, node->op1, node->op2, ret);
//            NVCGenNoResult(ret);
            break;

        case kNPSLoop:
            NBCGenLoop(stree, node->op1);
            NVCGenNoResult(ret);
            break;

        case kNPSFor:
            NBCGenFor(stree, node->op1, node->op2);
            NVCGenNoResult(ret);
            break;

        case kNPSForeach:
            NBCGenForeach(stree, node->op1, node->op2);
            NVCGenNoResult(ret);
            break;

        case kNPSWhile:
            NBCGenWhile(stree, node->op1, node->op2);
            NVCGenNoResult(ret);
            break;

        case kNPSRepeat:
            NBCGenRepeat(stree, node->op1, node->op2);
            NVCGenNoResult(ret);
            break;

        case kNPSBreak:
            NBCGenBreak(stree, node->op1);
            break;

        case kNPSAnd:
            NBCGenAnd(stree, node->op1, node->op2);
            NVCGenNoResult(ret);
            break;

        case kNPSOr:
            NBCGenOr(stree, node->op1, node->op2);
            NVCGenNoResult(ret);
            break;

        case kNPSMod:
            NBCGenFunc2(stree, NSSYM0(mod), node->op1, node->op2);
            NVCGenNoResult(ret);
            break;

		case kNPSShiftLeft:
            NBCGenFunc2(stree, NSSYM0(shiftLeft), node->op1, node->op2);
            NVCGenNoResult(ret);
            break;

		case kNPSShiftRight:
            NBCGenFunc2(stree, NSSYM0(shiftRight), node->op1, node->op2);
            NVCGenNoResult(ret);
            break;

        case kNPSConcat:
            NBCGenStringer(stree, node->op1, node->op2, NULL);
            NVCGenNoResult(ret);
            break;

        case kNPSConcat2:
            NBCGenStringer(stree, node->op1, node->op2, " ");
            NVCGenNoResult(ret);
            break;

		case kNPSObjectEqual:
            NBCGenFunc2(stree, NSSYM0(objectEqual), node->op1, node->op2);
            NVCGenNoResult(ret);
            break;

		case kNPSMakeRegex:
            NBCGenFunc2(stree, NSSYM0(makeRegex), node->op1, node->op2);
            NVCGenNoResult(ret);
            break;
    }
}


/*------------------------------------------------------------------------*/
/** �����̐����J�E���g����
 *
 * @param stree		[in] �\����
 * @param r			[in] �\���؃m�[�h
 *
 * @return			�Ȃ�
 */

int16_t NBCCountNumArgs(nps_syntax_node_t * stree, nps_node_t r)
{
    int16_t	numArgs = 1;

    if (r == kNewtRefUnbind)
        return 0;

    if (NPSRefIsSyntaxNode(r))
    {
        nps_syntax_node_t * node;

        node = &stree[NPSRefToSyntaxNode(r)];

        switch (node->code)
        {
            case kNPSCommaList:
                numArgs = NBCCountNumArgs(stree, node->op1)
                            + NBCCountNumArgs(stree, node->op2);
                break;
        }
    }

    return numArgs;
}


/*------------------------------------------------------------------------*/
/** �t���[���܂��͔z��̏������o�C�g�R�[�h�𐶐�����i�ċA�ďo���p�j
 *
 * @param stree		[in] �\����
 * @param r			[in] �\���؃m�[�h
 *
 * @return			�Ȃ�
 */

newtRef NBCGenMakeFrameSlots_sub(nps_syntax_node_t * stree, nps_node_t r)
{
    newtRefVar	obj = kNewtRefUnbind;

    if (r == kNewtRefUnbind)
        return NewtMakeMap(kNewtRefNIL, 0, NULL);

    if (NPSRefIsSyntaxNode(r))
    {
        nps_syntax_node_t * node;

        node = &stree[NPSRefToSyntaxNode(r)];

        switch (node->code)
        {
            case kNPSSlot:
                obj = node->op1;
                NBCGenBC_op(stree, node->op2);
                break;

            case kNPSCommaList:
                {
                    newtRefVar	obj1;
                    newtRefVar	obj2;

                    obj1 = NBCGenMakeFrameSlots_sub(stree, node->op1);
                    obj2 = NBCGenMakeFrameSlots_sub(stree, node->op2);

                    if (NewtRefIsArray(obj1))
                    {
                        obj = obj1;
                    }
                    else
                    {
                        newtRefVar	initMap[1];
                
                        initMap[0] = obj1;
                        obj = NewtMakeMap(kNewtRefNIL, 1, initMap);
                    }

                    if (obj2 == NSSYM0(_proto))
                        NewtSetMapFlags(obj, kNewtMapProto);

                    NcAddArraySlot(obj, obj2);
                }
                break;
        }
    }

    return obj;
}


/*------------------------------------------------------------------------*/
/** �t���[���܂��͔z��̏������o�C�g�R�[�h�𐶐�����
 *
 * @param stree		[in] �\����
 * @param r			[in] �\���؃m�[�h
 *
 * @return			�Ȃ�
 */

newtRef NBCGenMakeFrameSlots(nps_syntax_node_t * stree, nps_node_t r)
{
    newtRefVar	obj;

    obj = NBCGenMakeFrameSlots_sub(stree, r);

    if (NewtRefIsArray(obj))
    {
        obj = NewtPackLiteral(obj);
    }
    else
    {
		newtRefVar	initMap[1];

        initMap[0] = obj;
        obj = NewtMakeMap(kNewtRefNIL, 1, initMap);
    }

    return obj;
}


/*------------------------------------------------------------------------*/
/** �o�C�g�R�[�h�̐����i�ċA�ďo���p�j
 *
 * @param stree		[in] �\����
 * @param size		[in] �\���؂̒���
 * @param ret		[in] �߂�l�̗L��
 *
 * @return			�Ȃ�
 */

void NBCGenBC_sub(nps_syntax_node_t * stree, uint32_t n, bool ret)
{
    nps_syntax_node_t *	node;
    bool	handled = true;

    node = stree + n;

    if (kNPSConstituentList <= node->code)
    {
        NBCGenSyntaxCode(stree, node, ret);
        return;
    }

    switch (node->code)
    {
        case kNPSCall:
            NBCGenCall(stree, node->op1, node->op2);
            break;

        case kNPSInvoke:
            NBCGenInvoke(stree, node->op1, node->op2);
            break;

        case kNPSSend:
            NBCGenSend(stree, kNBCSend, node->op1, node->op2);
			break;

        case kNPSSendIfDefined:
            NBCGenSend(stree, kNBCSendIfDefined, node->op1, node->op2);
            break;

        case kNPSResend:
            NBCGenResend(stree, kNBCResend, node->op1, node->op2);
            break;

        case kNPSResendIfDefined:
            NBCGenResend(stree, kNBCResendIfDefined, node->op1, node->op2);
            break;

        case kNPSMakeArray:
            NBCGenMakeArray(stree, node->op1, node->op2);
            break;

        case kNPSMakeFrame:
            NBCGenMakeFrame(stree, node->op1);
            break;

        case kNPSGetPath:
            NBCGenBC_op(stree, node->op1);
            NBCGenBC_op(stree, node->op2);
            NBCGenCode(node->code, 2);
            break;

        default:
            handled = false;
            break;
    }

    if (handled)
    {
        NVCGenNoResult(ret);
    }
    else
    {
        NBCGenBC_op(stree, node->op1);
        NBCGenBC_op(stree, node->op2);
    
        if (node->code <= kNPSPopHandlers)
            NBCGenCode(0, node->code);
        else if (kNPSAdd <= node->code)
            NBCGenFreq(node->code - kNPSAdd);
        else
            NBCGenCode(node->code, 0);
    }
}


/*------------------------------------------------------------------------*/
/** �o�C�g�R�[�h�̐���
 *
 * @param stree		[in] �\����
 * @param size		[in] �\���؂̒���
 * @param ret		[in] �߂�l�̗L��
 *
 * @return			�֐��I�u�W�F�N�g
 */

newtRef NBCGenBC(nps_syntax_node_t * stree, uint32_t size, bool ret)
{
    newtRefVar	fn;

    NBCInit();

    newt_bc_env = NBCEnvNew(NULL);
    NBCGenBC_sub(stree, size - 1, ret);
    fn = NBCFnDone(&newt_bc_env);

    NBCCleanup();

    if (NewtRefIsNotNIL(fn))
    {
		fn = NewtPackLiteral(fn);

        if (NEWT_DUMPBC)
        {
            NewtFprintf(stderr, "*** byte code ***\n");
            NVMDumpFn(stderr, fn);
            NewtFprintf(stderr, "\n");
        }
    }

    return fn;
}


/*------------------------------------------------------------------------*/
/** �\�[�X�t�@�C�����R���p�C��
 *
 * @param s			[in] �\�[�X�t�@�C���̃p�X
 * @param ret		[in] �߂�l�̗L��
 *
 * @return			�֐��I�u�W�F�N�g
 */

newtRef NBCCompileFile(char * s, bool ret)
{
    nps_syntax_node_t *	stree = NULL;
    uint32_t	numStree = 0;
    newtRefVar	fn = kNewtRefUnbind;
    newtErr	err;

    err = NPSParseFile(s, &stree, &numStree);

    if (stree != NULL)
    {
        fn = NBCGenBC(stree, numStree, ret);
        NPSCleanup();
    }

    return fn;
}


/*------------------------------------------------------------------------*/
/** ��������R���p�C��
 *
 * @param s			[in] �X�N���v�g������
 * @param ret		[in] �߂�l�̗L��
 *
 * @return			�֐��I�u�W�F�N�g
 */

newtRef NBCCompileStr(char * s, bool ret)
{
    nps_syntax_node_t *	stree;
    uint32_t	numStree;
    newtRefVar	fn = kNewtRefUnbind;
    newtErr	err;

    err = NPSParseStr(s, &stree, &numStree);

    if (stree != NULL)
    {
        fn = NBCGenBC(stree, numStree, ret);
        NPSCleanup();
    }
    
    if (err)
    {
    	return NewtThrow(err, kNewtRefNIL);
    }

    return fn;
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �G���[���b�Z�[�W�̕\��
 *
 * @param err		[in] �G���[�R�[�h
 *
 * @return			�Ȃ�
 */

void NBError(int32_t err)
{
	char *  msg;

	switch (err)
	{
		case kNErrWrongNumberOfArgs:
			msg = "Wrong number of args";
			break;

		case kNErrAssignToConstant:
			msg = "Assign to constant";
			break;

		case kNErrSyntaxError:
			msg = "Syntax error";
			break;

		default:
			msg = "Unknown error";
			break;
	}

    NewtFprintf(stderr, "%s", msg);
}

