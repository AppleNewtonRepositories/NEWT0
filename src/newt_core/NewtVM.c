/*------------------------------------------------------------------------*/
/**
 * @file	NewtVM.c
 * @brief   VM
 *
 * @author  M.Nukui
 * @date	2003-11-07
 *
 * Copyright (C) 2003-2004 M.Nukui All rights reserved.
 */


/* �w�b�_�t�@�C�� */
#include <stdlib.h>

#include "NewtErrs.h"
#include "NewtVM.h"
#include "NewtBC.h"
#include "NewtGC.h"
#include "NewtMem.h"
#include "NewtEnv.h"
#include "NewtObj.h"
#include "NewtFns.h"
#include "NewtStr.h"
#include "NewtFile.h"
#include "NewtIO.h"
#include "NewtPrint.h"
#include "NewtNSOF.h"


/* �^�錾 */
typedef void(*instruction_t)(int16_t b);			///< ���߃Z�b�g
typedef void(*simple_instruction_t)(void);			///< �V���v������
typedef newtRef(*nvm_func_t)();						///< �l�C�e�B�u�֐�


/* �O���[�o���ϐ� */
vm_env_t	vm_env;


#pragma mark -
/* �}�N�� */

#define START_LOCALARGS		3										///< ���[�J�������̊J�n�ʒu


#define BC					(vm_env.bc)								///< �o�C�g�R�[�h
#define BCLEN				(vm_env.bclen)							///<�@�o�C�g�R�[�h��

#define CALLSTACK			((vm_reg_t *)vm_env.callstack.stackp)   ///< �ďo���X�^�b�N
#define CALLSP				(vm_env.callstack.sp)					///< �ďo���X�^�b�N�̃X�^�b�N�|�C���^
#define EXCPSTACK			((vm_excp_t *)vm_env.excpstack.stackp)  ///< ��O�X�^�b�N
#define EXCPSP				(vm_env.excpstack.sp)					///< ��O�X�^�b�N�̃X�^�b�N�|�C���^
#define CURREXCP			(vm_env.currexcp)						///< ���݂̗�O

#define	REG					(vm_env.reg)							///< ���W�X�^
#define STACK				((newtRef *)vm_env.stack.stackp)		///< �X�^�b�N

#define	FUNC				((REG).func)							///< ���s���̊֐�
#define	PC					((REG).pc)								///< �v���O�����J�E���^
#define	SP					((REG).sp)								///< �X�^�b�N�|�C���^
#define	LOCALS				((REG).locals)							///< ���[�J���t���[��
#define	RCVR				((REG).rcvr)							///< ���V�[�o
#define	IMPL				((REG).impl)							///< �C���v�������^


#pragma mark -
/* �֐��v���g�^�C�v */

static newtErr		NVMGetExceptionErrCode(newtRefArg r, bool dump);
static newtRef		NVMMakeExceptionFrame(newtRefArg name, newtRefArg data);
static void			NVMClearCurrException(void);

static void			NVMSetFn(newtRefArg fn);
static void			NVMNoStackFrameForReturn(void);

static void			reg_rewind(int32_t sp);
static void			reg_pop(void);
static void			reg_push(int32_t sp);
static void			reg_save(int32_t sp);

static newtRef		stk_pop0(void);
static newtRef		stk_pop(void);
static void			stk_pop_n(int32_t n, newtRef a[]);
static void			stk_remove(uint16_t n);
static newtRef		stk_top(void);
static void			stk_push(newtRefArg value);

static bool			excp_push(newtRefArg sym, newtRefArg pc);
static void			excp_pop(void);
static vm_excp_t *  excp_top(void);
static void			excp_pop_handlers(void);

static newtRef		liter_get(int16_t n);

static newtRef		iter_new(newtRefArg r, newtRefArg deeply);
static void			iter_next(newtRefArg iter);
static bool			iter_done(newtRefArg iter);

static newtRef		NVMMakeArgsArray(uint16_t numArgs);
static void			NVMBindArgs(uint16_t numArgs);
static void			NVMThrowBC(newtErr err, newtRefArg value, int16_t pop, bool push);
static newtErr		NVMFuncCheck(newtRefArg fn, int16_t numArgs);
static void			NVMCallNativeFn(newtRefArg fn, int16_t numArgs);
static void			NVMCallNativeFunc(newtRefArg fn, newtRefArg rcvr, int16_t numArgs);
static void			NVMFuncCall(newtRefArg fn, int16_t numArgs);
static void			NVMMessageSend(newtRefArg impl, newtRefArg receiver, newtRefArg fn, int16_t numArgs);

static newtRef		vm_send(int16_t b, newtErr * errP);
static newtRef		vm_resend(int16_t b, newtErr * errP);

static void			si_pop(void);
static void			si_dup(void);
static void			si_return(void);
static void			si_pushself(void);
static void			si_set_lex_scope(void);
static void			si_iternext(void);
static void			si_iterdone(void);
static void			si_pop_handlers(void);

static void			fn_add(void);
static void			fn_subtract(void);
static void			fn_aref(void);
static void			fn_set_aref(void);
static void			fn_equals(void);
static void			fn_not(void);
static void			fn_not_equals(void);
static void			fn_multiply(void);
static void			fn_divide(void);
static void			fn_div(void);
static void			fn_less_than(void);
static void			fn_greater_than(void);
static void			fn_greater_or_equal(void);
static void			fn_less_or_equal(void);
static void			fn_bit_and(void);
static void			fn_bit_or(void);
static void			fn_bit_not(void);
static void			fn_new_iterator(void);
static void			fn_length(void);
static void			fn_clone(void);
static void			fn_set_class(void);
static void			fn_add_array_slot(void);
static void			fn_stringer(void);
static void			fn_has_path(void);
static void			fn_classof(void);

static void			is_dummy(int16_t b);
static void			is_simple_instructions(int16_t b);
static void			is_push(int16_t b);
static void			is_push_constant(int16_t b);
static void			is_call(int16_t b);
static void			is_invoke(int16_t b);
static void			is_send(int16_t b);
static void			is_send_if_defined(int16_t b);
static void			is_resend(int16_t b);
static void			is_resend_if_defined(int16_t b);
static void			is_branch(int16_t b);
static void			is_branch_if_true(int16_t b);
static void			is_branch_if_false(int16_t b);
static void			is_find_var(int16_t b);
static void			is_get_var(int16_t b);
static void			is_make_frame(int16_t b);
static void			is_make_array(int16_t b);
static void			is_get_path(int16_t b);
static void			is_set_path(int16_t b);
static void			is_set_var(int16_t b);
static void			is_find_and_set_var(int16_t b);
static void			is_incr_var(int16_t b);
static void			is_branch_if_loop_not_done(int16_t b);
static void			is_freq_func(int16_t b);
static void			is_new_handlers(int16_t b);

static void			NVMDumpInstResult(FILE * f);
static void			NVMDumpInstCode(FILE * f, uint8_t * bc, uint32_t pc, uint16_t len);

static void			NVMInitREG(void);
static void			NVMInitSTACK(void);
static void			NVMCleanSTACK(void);

static void			NVMInitGlobalFns0(void);
static void			NVMInitGlobalFns1(void);
static void			NVMInitExGlobalFns(void);
static void			NVMInitDebugGlobalFns(void);
static void			NVMInitGlobalFns(void);

static void			NVMInitGlobalVars(void);

static void			NVMInit(void);
static void			NVMClean(void);
static void			NVMLoop(uint32_t callsp);

static newtRef		NVMInterpret2(nps_syntax_node_t * stree, uint32_t numStree, newtErr * errP);


/* ���[�J���ϐ� */

/// �V���v�����߃e�[�u��
static simple_instruction_t	simple_instructions[] =
            {
                si_pop,				// 000 pop
                si_dup,				// 001 dup
                si_return,			// 002 return
                si_pushself,		// 003 push-self
                si_set_lex_scope,	// 004 set-lex-scope
                si_iternext,		// 005 iter-next
                si_iterdone,		// 006 iter-done
                si_pop_handlers		// 007 000 001 pop-handlers
            };


/// �֐����߃e�[�u��
static simple_instruction_t	fn_instructions[] =
            {
                fn_add,					//  0 add				|+|
                fn_subtract,			//  1 subtract			|-|
                fn_aref,				//  2 aref				aref
                fn_set_aref,	        //  3 set-aref			setAref
                fn_equals,				//  4 equals			|=|
                fn_not,					//  5 not				|not|
                fn_not_equals,	        //  6 not-equals		|<>|
                fn_multiply,	        //  7 multiply			|*|
                fn_divide,				//  8 divide			|/|
                fn_div,					//  9 div				|div|
                fn_less_than,	        // 10 less-than			|<|
                fn_greater_than,		// 11 greater-than		|>|
                fn_greater_or_equal,	// 12 greater-or-equal	|>=|
                fn_less_or_equal,		// 13 less-or-equal		|<=|
                fn_bit_and,				// 14 bit-and			BAnd
                fn_bit_or,				// 15 bit-or			BOr
                fn_bit_not,				// 16 bit-not			BNot
                fn_new_iterator,		// 17 new-iterator		newIterator
                fn_length,				// 18 length			Length
                fn_clone,				// 19 clone				Clone
                fn_set_class,			// 20 set-class			SetClass
                fn_add_array_slot,		// 21 add-array-slot	AddArraySlot
                fn_stringer,			// 22 stringer			Stringer
                fn_has_path,			// 23 has-path			none
                fn_classof				// 24 class-of			ClassOf
            };

/// ���߃Z�b�g�e�[�u��
static instruction_t	is_instructions[] =
            {
                is_simple_instructions,		// 00x simple instructions
                is_dummy,					// 01x
                is_dummy,					// 02x
                is_push,					// 03x push
                is_push_constant,			// 04x (B signed) push-constant
                is_call,					// 05x call
                is_invoke,					// 06x invoke
                is_send,					// 07x send
                is_send_if_defined,			// 10x send-if-defined
                is_resend,					// 11x resend
                is_resend_if_defined,		// 12x resend-if-defined
                is_branch,					// 13x branch
                is_branch_if_true,			// 14x branch-if-true
                is_branch_if_false,			// 15x branch-if-false
                is_find_var,				// 16x find-var
                is_get_var,					// 17x get-var
                is_make_frame,				// 20x make-frame
                is_make_array,				// 21x make-array
                is_get_path,				// 220/221 get-path
                is_set_path,				// 230/231 set-path
                is_set_var,					// 24x set-var
                is_find_and_set_var,		// 25x find-and-set-var
                is_incr_var,				// 26x incr-var
                is_branch_if_loop_not_done,	// 27x branch-if-loop-not-done
                is_freq_func,				// 30x freq-func
                is_new_handlers				// 31x new-handlers
            };

/// �V���v�����ߖ��e�[�u��
static char *	simple_instruction_names[] =
            {
                "pop",				// 000 pop
                "dup",				// 001 dup
                "return",			// 002 return
                "push-self",		// 003 push-self
                "set-lex-scope",	// 004 set-lex-scope
                "iter-next",		// 005 iter-next
                "iter-done",		// 006 iter-done
                "pop-handlers"		// 007 000 001 pop-handlers
            };

/// �֐����ߖ��e�[�u��
static char *	fn_instruction_names[] =
            {
                "add",				//  0 add		|+|
                "subtract",			//  1 subtract		|-|
                "aref",				//  2 aref		aref
                "set-aref",			//  3 set-aref		setAref
                "equals",			//  4 equals		|=|
                "not",				//  5 not		|not|
                "not-equals",		//  6 not-equals	|<>|
                "multiply",			//  7 multiply		|*|
                "divide",			//  8 divide		|/|
                "div",				//  9 div		|div|
                "less-than",		// 10 less-than		|<|
                "greater-than",		// 11 greater-than	|>|
                "greateror-equal",	// 12 greater-or-equal	|>=|
                "lessor-equal",		// 13 less-or-equal	|<=|
                "bit-and",			// 14 bit-and		BAnd
                "bit-or",			// 15 bit-or		BOr
                "bit-not",			// 16 bit-not		BNot
                "new-iterator",		// 17 new-iterator	newIterator
                "length",			// 18 length		Length
                "clone",			// 19 clone		Clone
                "set-class",		// 20 set-class		SetClass
                "add-array-slot",	// 21 add-array-slot	AddArraySlot
                "stringer",			// 22 stringer		Stringer
                "has-path",			// 23 has-path		none
                "class-of"			// 24 class-of		ClassOf
            };

/// ���߃Z�b�g���e�[�u��
static char *	vm_instruction_names[] =
            {
                "simple-instructions",		// 00x simple instructions
                NULL,						// 01x
                NULL,						// 02x
                "push",						// 03x push
                "push-constant",			// 04x (B signed) push-constant
                "call",						// 05x call
                "invoke",					// 06x invoke
                "send",						// 07x send
                "send-if-defined",			// 10x send-if-defined
                "resend",					// 11x resend
                "resend-if-defined",		// 12x resend-if-defined
                "branch",					// 13x branch
                "branch-if-true",			// 14x branch-if-true
                "branch-if-false",			// 15x branch-if-false
                "find-var",					// 16x find-var
                "get-var",					// 17x get-var
                "make-frame",				// 20x make-frame
                "make-array",				// 21x make-array
                "get-path",					// 220/221 get-path
                "set-path",					// 230/231 set-path
                "set-var",					// 24x set-var
                "find-and-set-var",			// 25x find-and-set-var
                "incr-var",					// 26x incr-var
                "branch-if-loop-not-done",	// 27x branch-if-loop-not-done
                "freq-func",				// 30x freq-func
                "new-handlers"				// 31x new-handlers
            };


#pragma mark -
/*------------------------------------------------------------------------*/
/** self ���擾
 *
 * @return			self
 */

newtRef NVMSelf(void)
{
	return RCVR;
}


/*------------------------------------------------------------------------*/
/** ���݂̊֐��I�u�W�F�N�g���擾����
 *
 * @return		���݂̊֐��I�u�W�F�N�g
 */
newtRef NVMCurrentFunction(void)
{
    return FUNC;
}


/*------------------------------------------------------------------------*/
/** ���݂̃C���v�������^���擾���� 
 *
 * @return		���݂̃C���v�������^
 */
newtRef NVMCurrentImplementor(void)
{
    return IMPL;
}


/*------------------------------------------------------------------------*/
/** �ϐ��̑��݃`�F�b�N
 *
 * @param name		[in] �ϐ��V���{��
 *
 * @retval			true		�ϐ������݂���
 * @retval			false		�ϐ������݂��Ȃ�
 */

bool NVMHasVar(newtRefArg name)
{
    if (NewtHasLexical(LOCALS, name))
        return true;

    if (NewtHasVariable(RCVR, name))
        return true;

    if (NewtHasGlobalVar(name))
        return true;

    return false;
}


/*------------------------------------------------------------------------*/
/** ��O�t���[������G���[�R�[�h���擾����
 *
 * @param r		[in] �ϐ��V���{��
 * @param dump	[in] �_���v�t���O
 *
 * @return		�G���[�R�[�h
 */

newtErr NVMGetExceptionErrCode(newtRefArg r, bool dump)
{
    newtRefVar	err;

    if (NewtRefIsNIL(r))
        return kNErrNone;

    if (dump)
        NewtPrintObject(stderr, r);

    err = NcGetSlot(r, NSSYM0(error));

    if (NewtRefIsNotNIL(err))
        return NewtRefToInteger(err);

    return kNErrBadExceptionName;
}


/*------------------------------------------------------------------------*/
/** ��O�t���[�����쐬����
 *
 * @param name	[in] �V���{��
 * @param data	[in] �f�[�^
 *
 * @return		��O�t���[��
 */

newtRef NVMMakeExceptionFrame(newtRefArg name, newtRefArg data)
{
    newtRefVar	r;

    r = NcMakeFrame();
    NcSetSlot(r, NSSYM0(name), name);

    if (NewtHasSubclass(name, NSSYM0(type.ref)))
        NcSetSlot(r, NSSYM0(data), data);
    else if (NewtHasSubclass(name, NSSYM0(ext.ex.msg)))
        NcSetSlot(r, NSSYM0(message), data);
    else
        NcSetSlot(r, NSSYM0(error), data);

    return r;
}


/*------------------------------------------------------------------------*/
/** ��O�𔭐�������
 *
 * @param name	[in] �V���{��
 * @param data	[in] ��O�t���[��
 *
 * @return		�Ȃ�
 */

void NVMThrowData(newtRefArg name, newtRefArg data)
{
    vm_excp_t *	excp;
    uint32_t	i;

	// ��O�������Ȃ�N���A����
	NVMClearCurrException();

    CURREXCP = data;

    for (i = EXCPSP; 0 < i; i--)
    {
        excp = &EXCPSTACK[i - 1];

        if (NewtHasSubclass(name, excp->sym))
        {
            reg_rewind(excp->callsp);
            PC = excp->pc;
            return;
        }
    }

    NVMNoStackFrameForReturn();
}


/*------------------------------------------------------------------------*/
/** ��O�𔭐�������
 *
 * @param name	[in] �V���{��
 * @param data	[in] �f�[�^
 *
 * @return		�Ȃ�
 */

void NVMThrow(newtRefArg name, newtRefArg data)
{
    newtRefVar	r;

    r = NVMMakeExceptionFrame(name, data);
    NVMThrowData(name, r);
}


/*------------------------------------------------------------------------*/
/** rethrow ����
 *
 * @return		�Ȃ�
 */

void NVMRethrow(void)
{
    if (NewtRefIsNotNIL(CURREXCP))
    {
        newtRefVar	currexcp;
        newtRefVar	name;

		currexcp = CURREXCP;
        name = NcGetSlot(currexcp, NSSYM0(name));

//        excp_pop_handlers();
        NVMThrowData(name, currexcp);
    }
}


/*------------------------------------------------------------------------*/
/** ���݂̗�O���擾����
 *
 * @return		��O�t���[��
 */

newtRef NVMCurrentException(void)
{
    return CURREXCP;
}


/*------------------------------------------------------------------------*/
/** ��O���������Ă������O�X�^�b�N���N���A����
 *
 * @return		�Ȃ�
 */

void NVMClearCurrException(void)
{
    if (NewtRefIsNotNIL(CURREXCP))
	{
		excp_pop_handlers();
		CURREXCP = kNewtRefUnbind;
	}
}


/*------------------------------------------------------------------------*/
/** ���݂̗�O���N���A����
 *
 * @return		�Ȃ�
 *
 * @note		�l�C�e�B�u�֐��ŗ�O�������s�����߂Ɏg�p
 */

void NVMClearException(void)
{
	CURREXCP = kNewtRefUnbind;
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �֐��I�u�W�F�N�g�����݂̎��s�֐��ɂ���
 *
 * @param fn	[in] �֐��I�u�W�F�N�g
 *
 * @return		�Ȃ�
 */

void NVMSetFn(newtRefArg fn)
{
    FUNC = fn;

    if (NewtRefIsNIL(FUNC) || ! NewtRefIsCodeBlock(FUNC))
    {
        BC = NULL;
        BCLEN = 0;
    }
    else
    {
        newtRefVar	instr;

        instr = NcGetSlot(FUNC, NSSYM0(instructions));
        BC = NewtRefToBinary(instr);
        BCLEN = NewtLength(instr);
    }
}


/*------------------------------------------------------------------------*/
/** �߂�֐��X�^�b�N���Ȃ��ꍇ�̏���
 *
 * @return		�Ȃ�
 */

void NVMNoStackFrameForReturn(void)
{
    BC = NULL;
    BCLEN = 0;
    CALLSP = 0;
}


#pragma mark *** �ďo���X�^�b�N
/*------------------------------------------------------------------------*/
/** ���W�X�^�̊����߂�
 *
 * @param sp	[in] �ďo���X�^�b�N�̃X�^�b�N�|�C���^
 *
 * @return		�Ȃ�
 */

void reg_rewind(int32_t sp)
{
    if (sp == CALLSP)
        return;

    if (sp < CALLSP)
    {
        CALLSP = sp;
        REG = CALLSTACK[CALLSP];
        NVMSetFn(FUNC);
    }
    else
    {
        NVMNoStackFrameForReturn();
    }
}


/*------------------------------------------------------------------------*/
/** ���W�X�^�̃|�b�v
 *
 * @return			�Ȃ�
 */

void reg_pop(void)
{
	reg_rewind(CALLSP - 1);
}


/*------------------------------------------------------------------------*/
/** ���W�X�^�̃v�b�V��
 *
 * @param sp		[in] �X�^�b�N�|�C���^
 *
 * @return			�Ȃ�
 */

void reg_push(int32_t sp)
{
    if (! NewtStackExpand(&vm_env.callstack, CALLSP + 1))
        return;

    CALLSTACK[CALLSP] = REG;
    CALLSTACK[CALLSP].sp = sp;
    CALLSP++;
}


/*------------------------------------------------------------------------*/
/** ���W�X�^�̕ۑ�
 *
 * @param sp		[in] �X�^�b�N�|�C���^
 *
 * @return			�Ȃ�
 */

void reg_save(int32_t sp)
{
//    PC++;
    reg_push(sp);
}


#pragma mark *** �X�^�b�N
/*------------------------------------------------------------------------*/
/** �X�^�b�N�̃|�b�v
 *
 * @return			�I�u�W�F�N�g
 */

newtRef stk_pop0(void)
{
    newtRefVar	x = kNewtRefUnbind;

    if (0 < SP)
    {
        SP--;
        x = STACK[SP];
    }

    return x;
}


/*------------------------------------------------------------------------*/
/** �X�^�b�N�̃|�b�v�i�}�W�b�N�|�C���^�Q�Ƃ������j
 *
 * @return			�I�u�W�F�N�g
 */

newtRef stk_pop(void)
{
	return NcResolveMagicPointer(stk_pop0());
}


/*------------------------------------------------------------------------*/
/** �X�^�b�N�� n�|�b�v
 *
 * @param n			[in] �|�b�v���鐔
 * @param a			[out]�|�b�v�����I�u�W�F�N�g���i�[����z��
 *
 * @return			�Ȃ�
 */

void stk_pop_n(int32_t n, newtRef a[])
{
    for (n--; 0 <= n; n--)
    {
        a[n] = stk_pop();
    }
}


/*------------------------------------------------------------------------*/
/** �X�^�b�N�� n�폜
 *
 * @return			�Ȃ�
 */

void stk_remove(uint16_t n)
{
    if (n < SP)
        SP -= n;
    else
        SP = 0;
}


/*------------------------------------------------------------------------*/
/** �X�^�b�N�̐擪�f�[�^����o��
 *
 * @return			�I�u�W�F�N�g
 */

newtRef stk_top(void)
{
    if (0 < SP)
        return STACK[SP - 1];
    else
        return kNewtRefNIL;
}


/*------------------------------------------------------------------------*/
/** �X�^�b�N�ɃI�u�W�F�N�g���v�b�V��
 *
 * @param value		[in] �I�u�W�F�N�g
 *
 * @return			�Ȃ�
 */

void stk_push(newtRefArg value)
{
    if (! NewtStackExpand(&vm_env.stack, SP + 1))
        return;

    STACK[SP] = value;
    SP++;
}


#pragma mark *** ��O�n���h���X�^�b�N
/*------------------------------------------------------------------------*/
/** ��O�X�^�b�N�Ƀv�b�V��
 *
 * @param sym		[in] ��O�V���{��
 * @param pc		[in] �v���O�����J�E���^
 *
 * @retval			true	�X�^�b�N���ꂽ
 * @retval			false	�X�^�b�N����Ȃ�����
 */

bool excp_push(newtRefArg sym, newtRefArg pc)
{
    vm_excp_t *	excp;

    if (! NewtRefIsSymbol(sym))
        return false;

    if (! NewtRefIsInteger(pc))
        return false;

    if (! NewtStackExpand(&vm_env.excpstack, EXCPSP + 1))
        return false;

    excp = &EXCPSTACK[EXCPSP];

    excp->callsp = CALLSP;
    excp->excppc = PC;
    excp->sym = sym;
    excp->pc = NewtRefToInteger(pc);

    EXCPSP++;

    return true;
}

/*------------------------------------------------------------------------*/
/** ��O�X�^�b�N���|�b�v
 *
 * @return			�Ȃ�
 */

void excp_pop(void)
{
    if (0 < EXCPSP)
        EXCPSP--;
}

/*------------------------------------------------------------------------*/
/** ��O�X�^�b�N�̐擪���擾
 *
 * @return			��O�\���̂ւ̃|�C���^
 */

vm_excp_t * excp_top(void)
{
    if (0 < EXCPSP)
        return &EXCPSTACK[EXCPSP - 1];
    else
        return NULL;
}


/*------------------------------------------------------------------------*/
/** ��O�n���h�����|�b�v����
 *
 * @return			�Ȃ�
 */

void excp_pop_handlers(void)
{
    vm_excp_t *	excp;

    excp = excp_top();

    if (excp != NULL)
    {
        uint32_t	excppc;

        excppc = excp->excppc;

        while (excp != NULL)
        {
            if (excp->excppc != excppc)
                break;

            excp_pop();
            excp = excp_top();
        }
    }
}


#pragma mark *** Literals
/*------------------------------------------------------------------------*/
/** ���e��������o��
 *
 * @param n			[in] ���e�������X�g�̈ʒu
 *
 * @return			���e�����I�u�W�F�N�g
 */

newtRef liter_get(int16_t n)
{
    newtRefVar	literals;

    literals = NcGetSlot(FUNC, NSSYM0(literals));

    if (NewtRefIsNotNIL(literals))
        return NewtGetArraySlot(literals, n);
    else
        return kNewtRefNIL;
}


#pragma mark *** Iterator
/*------------------------------------------------------------------------*/
/** �C�e���[�^�I�u�W�F�N�g���쐬����
 *
 * @param r			[in] �I�u�W�F�N�g
 * @param deeply	[in] deeply �t���O
 *
 * @return			�C�e���[�^�I�u�W�F�N�g
 */

newtRef iter_new(newtRefArg r, newtRefArg deeply)
{
    newtRefVar	iter;

    iter = NewtMakeArray(NSSYM0(forEachState), kIterALength);

    NewtSetArraySlot(iter, kIterObj, r);

    if (NewtRefIsNIL(deeply))
        NewtSetArraySlot(iter, kIterDeeply, deeply);
    else
        NewtSetArraySlot(iter, kIterDeeply, NcDeeplyLength(r));

    NewtSetArraySlot(iter, kIterPos, NSINT(-1));
    NewtSetArraySlot(iter, kIterMax, NcLength(r));

    if (NewtRefIsFrame(r))
        NewtSetArraySlot(iter, kIterMap, NewtFrameMap(r));
    else
        NewtSetArraySlot(iter, kIterMap, kNewtRefNIL);

    iter_next(iter);

    return iter;
}


/*------------------------------------------------------------------------*/
/** �C�e���[�^�����ɐi�߂�
 *
 * @param iter		[in] �C�e���[�^�I�u�W�F�N�g
 *
 * @return			�Ȃ�
 */

void iter_next(newtRefArg iter)
{
    newtRefVar	deeply;
    newtRefVar	obj;
    newtRefVar	index = kNewtRefUnbind;
    newtRefVar	value = kNewtRefUnbind;
    newtRefVar	map;
    int32_t	pos;
    int32_t	len;

    obj = NewtGetArraySlot(iter, kIterObj);
    deeply = NewtGetArraySlot(iter, kIterDeeply);
    pos = NewtRefToInteger(NewtGetArraySlot(iter, kIterPos));
    len = NewtRefToInteger(NewtGetArraySlot(iter, kIterMax));
    map = NewtGetArraySlot(iter, kIterMap);

    if (NewtRefIsNIL(deeply) || NewtRefIsNIL(map))
    {
        pos++;

        if (pos < len)
        {
            if (NewtRefIsNIL(map))
            {
                index = NewtMakeInteger(pos);
                value = NewtARef(obj, pos);
            }
            else
            {
                index = NewtGetArraySlot(map, pos + 1);
                value = NewtGetFrameSlot(obj, pos);
            }
        }
    }
    else
    {
        while (true)
        {
            pos++;

            if (len <= pos)
            {
                obj = NcGetSlot(obj, NSSYM0(_proto));

                if (NewtRefIsNIL(obj))
                {
                    index = kNewtRefUnbind;
                    break;
                }

                map = NewtFrameMap(obj);
                len = NewtLength(obj);

                NewtSetArraySlot(iter, kIterObj, obj);
                NewtSetArraySlot(iter, kIterMap, map);
                NewtSetArraySlot(iter, kIterMax, NewtMakeInteger(len));

                pos = -1;
                continue;
            }

            index = NewtGetArraySlot(map, pos + 1);

            if (index != NSSYM0(_proto))
            {
                value = NewtGetFrameSlot(obj, pos);
                break;
            }
        }
    }

    NewtSetArraySlot(iter, kIterIndex, index);
    NewtSetArraySlot(iter, kIterPos, NewtMakeInteger(pos));
    NewtSetArraySlot(iter, kIterValue, value);
}


/*------------------------------------------------------------------------*/
/** �C�e���[�^�̏I�����`�F�b�N����
 *
 * @param iter		[in] �C�e���[�^�I�u�W�F�N�g
 *
 * @retval			true	�I��
 * @retval			false	�I�����Ă��Ȃ�
 */

bool iter_done(newtRefArg iter)
{
    int32_t	pos;
    int32_t	len;

    pos = NewtRefToInteger(NewtGetArraySlot(iter, kIterPos));
    len = NewtRefToInteger(NewtGetArraySlot(iter, kIterMax));

    return (len <= pos);
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �������X�^�b�N�����o���Ĕz��ɂ���
 *
 * @param numArgs	[in] �����̐�
 *
 * @return			�z��
 */

newtRef NVMMakeArgsArray(uint16_t numArgs)
{
	newtRefVar	args;
	int16_t		i;

	args = NewtMakeArray(kNewtRefUnbind, numArgs);

	for (i = numArgs - 1; 0 <= i; i--)
	{
		NewtSetArraySlot(args, i, stk_pop());
	}

	return args;
}


/*------------------------------------------------------------------------*/
/** �������X�^�b�N�����o���ă��[�J���t���[���ɑ�������
 *
 * @param numArgs	[in] �����̐�
 *
 * @return			�Ȃ�
 */

void NVMBindArgs(uint16_t numArgs)
{
    newtRefVar	indefinite;
	int32_t		minArgs;
	int16_t		i;
    newtRefVar	v;

	minArgs = NewtRefToInteger(NcGetSlot(FUNC, NSSYM0(numArgs)));
    indefinite = NcGetSlot(FUNC, NSSYM0(indefinite));

	if (NewtRefIsNotNIL(indefinite))
	{
		newtRefVar	args;

		args = NVMMakeArgsArray(numArgs - minArgs);
		NewtSetFrameSlot(LOCALS, START_LOCALARGS + minArgs, args);
	}

    for (i = START_LOCALARGS + minArgs - 1; START_LOCALARGS <= i; i--)
    {
        v = stk_pop();
        NewtSetFrameSlot(LOCALS, i, v);
    }
}


/*------------------------------------------------------------------------*/
/** ��O�𔭐�����
 *
 * @param err		[in] �G���[�ԍ�
 * @param value		[in] �l�I�u�W�F�N�g
 * @param pop		[in] �|�b�v���鐔
 * @param push		[in] �v�b�V���̗L��
 *
 * @return			�Ȃ�
 */

void NVMThrowBC(newtErr err, newtRefArg value, int16_t pop, bool push)
{
    stk_remove(pop);

    if (push)
        stk_push(kNewtRefUnbind);

	if (NewtRefIsSymbol(value))
		NewtThrowSymbol(err, value);
	else
		NewtThrow(err, value);
}


/*------------------------------------------------------------------------*/
/** �֐��̈����̐����`�F�b�N����
 *
 * @param fn		[in] �֐��I�u�W�F�N�g
 * @param numArgs	[in] �����̐�
 *
 * @retval			true	����
 * @retval			false	�s��
 */

bool NVMFuncCheckNumArgs(newtRefArg fn, int16_t numArgs)
{
    newtRefVar	indefinite;
    int32_t		minArgs;

    minArgs = NewtRefToInteger(NcGetSlot(fn, NSSYM0(numArgs)));
    indefinite = NcGetSlot(fn, NSSYM0(indefinite));

	if (NewtRefIsNIL(indefinite))
		return (minArgs == numArgs);
	else
		return (minArgs <= numArgs);
}


/*------------------------------------------------------------------------*/
/** �֐��I�u�W�F�N�g���`�F�b�N����
 *
 * @param fn		[in] �֐��I�u�W�F�N�g
 * @param numArgs	[in] �����̐�
 *
 * @return			�G���[�R�[�h
 */

newtErr NVMFuncCheck(newtRefArg fn, int16_t numArgs)
{
    // 1. �֐��I�u�W�F�N�g�łȂ���Η�O�𔭐�

    if (! NewtRefIsFunction(fn))
        return kNErrInvalidFunc;

    // 2. �����̐�����v���Ȃ���� WrongNumberOfArgs ��O�𔭐�

    if (! NVMFuncCheckNumArgs(fn, numArgs))
        return kNErrWrongNumberOfArgs;

    return kNErrNone;
}


/*------------------------------------------------------------------------*/
/** �l�C�e�B�u�֐��ircvr�Ȃ��j�̌ďo��
 *
 * @param fn		[in] �֐��I�u�W�F�N�g
 * @param numArgs	[in] �����̐�
 *
 * @return			�Ȃ�
 */

void NVMCallNativeFn(newtRefArg fn, int16_t numArgs)
{
    newtRefVar	r = kNewtRefUnbind;
    newtRefVar	indefinite;
    nvm_func_t	funcPtr;
	int32_t		minArgs;

    funcPtr = (nvm_func_t)NewtRefToAddress(NcGetSlot(fn, NSSYM0(funcPtr)));

	if (funcPtr == NULL)
		return;

	minArgs = NewtRefToInteger(NcGetSlot(fn, NSSYM0(numArgs)));
    indefinite = NcGetSlot(fn, NSSYM0(indefinite));

	if (NewtRefIsNIL(indefinite))
	{
		switch (minArgs)
		{
			case 0:
				r = (*funcPtr)();
				break;

			case 1:
				{
					newtRefVar	a;

					a = stk_pop();
					r = (*funcPtr)(a);
				}
				break;

			case 2:
				{
					newtRefVar	a[2];

					stk_pop_n(2, a);
					r = (*funcPtr)(a[0], a[1]);
				}
				break;

			case 3:
				{
					newtRefVar	a[3];

					stk_pop_n(3, a);
					r = (*funcPtr)(a[0], a[1], a[2]);
				}
				break;

			case 4:
				{
					newtRefVar	a[4];

					stk_pop_n(4, a);
					r = (*funcPtr)(a[0], a[1], a[2], a[3]);
				}
				break;

			case 5:
				{
					newtRefVar	a[5];

					stk_pop_n(5, a);
					r = (*funcPtr)(a[0], a[1], a[2], a[3], a[4]);
				}
				break;

			case 6:
				{
					newtRefVar	a[6];

					stk_pop_n(6, a);
					r = (*funcPtr)(a[0], a[1], a[2], a[3], a[4], a[5]);
				}
				break;

			case 7:
				{
					newtRefVar	a[7];

					stk_pop_n(7, a);
					r = (*funcPtr)(a[0], a[1], a[2], a[3], a[4], a[5], a[6]);
				}
				break;


			case 8:
				{
					newtRefVar	a[8];

					stk_pop_n(8, a);
					r = (*funcPtr)(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
				}
				break;

			case 9:
				{
					newtRefVar	a[9];

					stk_pop_n(9, a);
					r = (*funcPtr)(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8]);
				}
				break;

			default:
				stk_remove(minArgs);
				break;
		}
	}
	else
	{
		newtRefVar	args;

		args = NVMMakeArgsArray(numArgs - minArgs);

		switch (minArgs)
		{
			case 0:
				r = (*funcPtr)(args);
				break;

			case 1:
				{
					newtRefVar	a;

					a = stk_pop();
					r = (*funcPtr)(a, args);
				}
				break;

			case 2:
				{
					newtRefVar	a[2];

					stk_pop_n(2, a);
					r = (*funcPtr)(a[0], a[1], args);
				}
				break;

			case 3:
				{
					newtRefVar	a[3];

					stk_pop_n(3, a);
					r = (*funcPtr)(a[0], a[1], a[2], args);
				}
				break;

			case 4:
				{
					newtRefVar	a[4];

					stk_pop_n(4, a);
					r = (*funcPtr)(a[0], a[1], a[2], a[3], args);
				}
				break;

			case 5:
				{
					newtRefVar	a[5];

					stk_pop_n(5, a);
					r = (*funcPtr)(a[0], a[1], a[2], a[3], a[4], args);
				}
				break;

			case 6:
				{
					newtRefVar	a[6];

					stk_pop_n(6, a);
					r = (*funcPtr)(a[0], a[1], a[2], a[3], a[4], a[5], args);
				}
				break;

			case 7:
				{
					newtRefVar	a[7];

					stk_pop_n(7, a);
					r = (*funcPtr)(a[0], a[1], a[2], a[3], a[4], a[5], a[6], args);
				}
				break;


			case 8:
				{
					newtRefVar	a[8];

					stk_pop_n(8, a);
					r = (*funcPtr)(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], args);
				}
				break;

			case 9:
				{
					newtRefVar	a[9];

					stk_pop_n(9, a);
					r = (*funcPtr)(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8], args);
				}
				break;

			default:
				stk_remove(minArgs);
				break;
		}
	}

    stk_push(r);
}


/*------------------------------------------------------------------------*/
/** �l�C�e�B�u�֐��ircvr����j�̌ďo��
 *
 * @param fn		[in] �֐��I�u�W�F�N�g
 * @param rcvr		[in] ���V�[�o
 * @param numArgs	[in] �����̐�
 *
 * @return			�Ȃ�
 */

void NVMCallNativeFunc(newtRefArg fn, newtRefArg rcvr, int16_t numArgs)
{
    newtRefVar	r = kNewtRefUnbind;
    newtRefVar	indefinite;
    nvm_func_t	funcPtr;
	int32_t		minArgs;

    funcPtr = (nvm_func_t)NewtRefToAddress(NcGetSlot(fn, NSSYM0(funcPtr)));

	if (funcPtr == NULL)
		return;

	minArgs = NewtRefToInteger(NcGetSlot(fn, NSSYM0(numArgs)));
    indefinite = NcGetSlot(fn, NSSYM0(indefinite));

	if (NewtRefIsNIL(indefinite))
	{
		switch (minArgs)
		{
			case 0:
				r = (*funcPtr)(rcvr);
				break;

			case 1:
				{
					newtRefVar	a;

					a = stk_pop();
					r = (*funcPtr)(rcvr, a);
				}
				break;

			case 2:
				{
					newtRefVar	a[2];

					stk_pop_n(2, a);
					r = (*funcPtr)(rcvr, a[0], a[1]);
				}
				break;

			case 3:
				{
					newtRefVar	a[3];

					stk_pop_n(3, a);
					r = (*funcPtr)(rcvr, a[0], a[1], a[2]);
				}
				break;

			case 4:
				{
					newtRefVar	a[4];

					stk_pop_n(4, a);
					r = (*funcPtr)(rcvr, a[0], a[1], a[2], a[3]);
				}
				break;

			case 5:
				{
					newtRefVar	a[5];

					stk_pop_n(5, a);
					r = (*funcPtr)(rcvr, a[0], a[1], a[2], a[3], a[4]);
				}
				break;

			case 6:
				{
					newtRefVar	a[6];

					stk_pop_n(6, a);
					r = (*funcPtr)(rcvr, a[0], a[1], a[2], a[3], a[4], a[5]);
				}
				break;

			case 7:
				{
					newtRefVar	a[7];

					stk_pop_n(7, a);
					r = (*funcPtr)(rcvr, a[0], a[1], a[2], a[3], a[4], a[5], a[6]);
				}
				break;


			case 8:
				{
					newtRefVar	a[8];

					stk_pop_n(8, a);
					r = (*funcPtr)(rcvr, a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]);
				}
				break;

			case 9:
				{
					newtRefVar	a[9];

					stk_pop_n(9, a);
					r = (*funcPtr)(rcvr, a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8]);
				}
				break;

			default:
				stk_remove(minArgs);
				break;
		}
	}
	else
	{
		newtRefVar	args;

		args = NVMMakeArgsArray(numArgs - minArgs);

		switch (minArgs)
		{
			case 0:
				r = (*funcPtr)(rcvr, args);
				break;

			case 1:
				{
					newtRefVar	a;

					a = stk_pop();
					r = (*funcPtr)(rcvr, a, args);
				}
				break;

			case 2:
				{
					newtRefVar	a[2];

					stk_pop_n(2, a);
					r = (*funcPtr)(rcvr, a[0], a[1], args);
				}
				break;

			case 3:
				{
					newtRefVar	a[3];

					stk_pop_n(3, a);
					r = (*funcPtr)(rcvr, a[0], a[1], a[2], args);
				}
				break;

			case 4:
				{
					newtRefVar	a[4];

					stk_pop_n(4, a);
					r = (*funcPtr)(rcvr, a[0], a[1], a[2], a[3], args);
				}
				break;

			case 5:
				{
					newtRefVar	a[5];

					stk_pop_n(5, a);
					r = (*funcPtr)(rcvr, a[0], a[1], a[2], a[3], a[4], args);
				}
				break;

			case 6:
				{
					newtRefVar	a[6];

					stk_pop_n(6, a);
					r = (*funcPtr)(rcvr, a[0], a[1], a[2], a[3], a[4], a[5], args);
				}
				break;

			case 7:
				{
					newtRefVar	a[7];

					stk_pop_n(7, a);
					r = (*funcPtr)(rcvr, a[0], a[1], a[2], a[3], a[4], a[5], a[6], args);
				}
				break;


			case 8:
				{
					newtRefVar	a[8];

					stk_pop_n(8, a);
					r = (*funcPtr)(rcvr, a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], args);
				}
				break;

			case 9:
				{
					newtRefVar	a[9];

					stk_pop_n(9, a);
					r = (*funcPtr)(rcvr, a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7], a[8], args);
				}
				break;

			default:
				stk_remove(minArgs);
				break;
		}
	}

    stk_push(r);
}


/*------------------------------------------------------------------------*/
/** �֐��I�u�W�F�N�g�̌ďo��
 *
 * @param fn		[in] �֐��I�u�W�F�N�g
 * @param numArgs	[in] �����̐�
 *
 * @return			�Ȃ�
 */

void NVMFuncCall(newtRefArg fn, int16_t numArgs)
{
    newtErr	err;
	int		type;

    // 1. �֐��I�u�W�F�N�g�łȂ���Η�O�𔭐�
    // 2. �����̐�����v���Ȃ���� WrongNumberOfArgs ��O�𔭐�

    err = NVMFuncCheck(fn, numArgs);

    if (err != kNErrNone)
    {
        NVMThrowBC(err, fn, numArgs, true);
        return;
    }

	type = NewtRefFunctionType(fn);

    if (type == kNewtNativeFn || type == kNewtNativeFunc)
    {	// �l�C�e�B�u�֐��̌ďo��
    	// Save CALLSP to know if an exception occurred.
    	uint32_t saveCALLSP;
		reg_save(SP - numArgs + 1);
		FUNC = fn;
		saveCALLSP = CALLSP;

		switch (type)
		{
			case kNewtNativeFn:
				// rcvr�Ȃ�(old style)
				NVMCallNativeFn(fn, numArgs);
				break;

			case kNewtNativeFunc:
				// rcvr����(new style)
				NVMCallNativeFunc(fn, kNewtRefUnbind, numArgs);
				break;
		}

        if (saveCALLSP == CALLSP)
        {
			reg_pop();
		}
        return;
    }

    reg_save(SP - numArgs); // 3. VM ���W�X�^��ۑ��iPC �̍X�V��...�j
    NVMSetFn(fn);			// 4. FUNC �ɐV�����֐��I�u�W�F�N�g���Z�b�g
    PC = 0;					// 5. PC �� 0 ���Z�b�g

    // 6. ���[�J���t���[���iFUNC.argFrame�j���N���[������ LOCALS �ɃZ�b�g
    LOCALS = NcClone(NcGetSlot(FUNC, NSSYM0(argFrame)));

    // 7. LOCALS �̈����X���b�g�ɃX�^�b�N�ɂ���������Z�b�g����
    //    ������ LOCALS �̂S�ԃX���b�g����J�n��������E�֑}�������
    NVMBindArgs(numArgs);

    // 8. LOCALS �� _parent �X���b�g�� RCVR �ɃZ�b�g
    RCVR = NcGetSlot(LOCALS, NSSYM0(_parent));

    // 9. LOCALS �� _implementor �X���b�g�� IMPL �ɃZ�b�g
    IMPL = NcGetSlot(LOCALS, NSSYM0(_implementor));

    // 10. ���s�����W���[������
}


/*------------------------------------------------------------------------*/
/** ���\�b�h�̑��M
 *
 * @param impl		[in] �C���v�������^
 * @param receiver	[in] ���V�[�o
 * @param fn		[in] �֐��I�u�W�F�N�g
 * @param numArgs	[in] �����̐�
 *
 * @return			�Ȃ�
 */

void NVMMessageSend(newtRefArg impl, newtRefArg receiver, newtRefArg fn, int16_t numArgs)
{
    newtErr	err;
	int		type;

    // 1. ���\�b�h���֐��I�u�W�F�N�g�łȂ���Η�O�𔭐�
    // 2. �����̐�����v���Ȃ���� WrongNumberOfArgs ��O�𔭐�

    err = NVMFuncCheck(fn, numArgs);

    if (err != kNErrNone)
    {
        NVMThrowBC(err, fn, numArgs, true);
        return;
    }

	type = NewtRefFunctionType(fn);

    if (type == kNewtNativeFn || type == kNewtNativeFunc)
    {	// �l�C�e�B�u�֐��̌ďo��
    	// Save CALLSP to know if an exception occurred.
    	uint32_t saveCALLSP;
		reg_save(SP - numArgs + 1);
		FUNC = fn;
		RCVR = receiver;
		IMPL = impl;
		saveCALLSP = CALLSP;

		switch (type)
		{
			case kNewtNativeFn:
				// rcvr�Ȃ�(old style)
				NVMCallNativeFn(fn, numArgs);
				break;

			case kNewtNativeFunc:
				// rcvr����(new style)
				NVMCallNativeFunc(fn, receiver, numArgs);
				break;
		}

		if (saveCALLSP == CALLSP)
		{
			reg_pop();
		}
        return;
    }

    reg_save(SP - numArgs); // 3. VM ���W�X�^��ۑ��iPC �̍X�V��...�j
    NVMSetFn(fn);			// 4. FUNC �Ƀ��\�b�h���Z�b�g
    PC = 0;					// 5. PC �� 0 ���Z�b�g
    RCVR = receiver;		// 6. RCVR �� receiver ���Z�b�g
    IMPL = impl;			// 7. IMPL IMPL implementor ���Z�b�g

    // 8. ���[�J���t���[���iFUNC.argFrame�j���N���[������ LOCALS �ɃZ�b�g
    LOCALS = NcClone(NcGetSlot(FUNC, NSSYM0(argFrame)));

    // 9. RCVR �� LOCALS._parent �ɃZ�b�g
    NcSetSlot(LOCALS, NSSYM0(_parent), RCVR);

    // 10. IMPL �� LOCALS._implementor �ɃZ�b�g
    NcSetSlot(LOCALS, NSSYM0(_implementor), IMPL);

    // 11. LOCALS �̈����X���b�g�ɃX�^�b�N�ɂ���������Z�b�g����
    //     ������ LOCALS �̂S�ԃX���b�g����J�n��������E�֑}�������
    NVMBindArgs(numArgs);

    // 12. ���s�����W���[������
}


/*------------------------------------------------------------------------*/
/** ���\�b�h�̑��M
 *
 * @param b			[in] �I�y�R�[�h
 * @param errP		[out]�G���[�ԍ�
 *
 * @return			���\�b�h��
 */

newtRef	vm_send(int16_t b, newtErr * errP)
{
    // arg1 arg2 ... argN name receiver -- result

    newtRefVar	receiver;
    newtRefVar	impl;
    newtRefVar	name;
    newtRefVar	fn;
    newtErr	err = kNErrNone;

	if (errP != NULL)
		*errP = kNErrNone;

    receiver = stk_pop();
    name = stk_pop();

	if (! NewtRefIsFrame(receiver))
	{
		NVMThrowBC(kNErrNotAFrame, receiver, b, true);
		return name;
	}

	if (! NewtRefIsSymbol(name))
	{
		NVMThrowBC(kNErrNotASymbol, name, b, true);
		return name;
	}

    impl = NcFullLookupFrame(receiver, name);

    if (impl != kNewtRefUnbind)
	{
		fn = NcGetSlot(impl, name);
        NVMMessageSend(impl, receiver, fn, b);
    }
	else
	{
        err = kNErrUndefinedMethod;
	}

	if (errP != NULL)
		*errP = err;

    return name;
}


/*------------------------------------------------------------------------*/
/** ���\�b�h�̍đ��M
 *
 * @param b			[in] �I�y�R�[�h
 * @param errP		[out]�G���[�ԍ�
 *
 * @return			���\�b�h��
 */

newtRef vm_resend(int16_t b, newtErr * errP)
{
    newtRefVar	name;
    newtErr	err = kNErrNone;

	if (errP != NULL)
		*errP = kNErrNone;

    // arg1 arg2 ... argN name -- result

	name = stk_pop();

	if (! NewtRefIsSymbol(name))
	{
		NVMThrowBC(kNErrNotASymbol, name, b, true);
		return name;
	}

    if (NewtHasSlot(IMPL, NSSYM0(_proto)))
	{
		newtRefVar	impl;
		newtRefVar	fn;

		impl = NcGetSlot(IMPL, NSSYM0(_proto));
		impl = NcProtoLookupFrame(impl, name);

		if (impl != kNewtRefUnbind)
		{
			fn = NcGetSlot(impl, name);
			NVMMessageSend(impl, RCVR, fn, b);
		}
		else
		{
			err= kNErrUndefinedMethod;
		}
	}
	else
	{
		err = kNErrUndefinedMethod;
	}

	if (errP != NULL)
		*errP = err;

    return name;
}


#pragma mark -
#pragma mark *** Simple instructions
/*------------------------------------------------------------------------*/
/** �X�^�b�N�̃|�b�v
 *
 * @return			�Ȃ�
 */

void si_pop(void)
{
    stk_pop0();
}


/*------------------------------------------------------------------------*/
/** �X�^�b�N�̐擪�𕡐����ăv�b�V������
 *
 * @return			�Ȃ�
 */

void si_dup(void)
{
    stk_push(stk_top());
}


/*------------------------------------------------------------------------*/
/** �֐��̃��^�[��
 *
 * @return			�Ȃ�
 */

void si_return(void)
{
    newtRefVar	r;

    r = stk_top();
    reg_pop();
    stk_push(r);
}


/*------------------------------------------------------------------------*/
/** self ���X�^�b�N�Ƀv�b�V������
 *
 * @return			�Ȃ�
 */

void si_pushself(void)
{
    stk_push(RCVR);
}


/*------------------------------------------------------------------------*/
/** ���L�V�J���X�R�[�v���Z�b�g����
 *
 * @return			�Ȃ�
 */

void si_set_lex_scope(void)
{
    newtRefVar	fn;
    newtRefVar	af;

    fn = NcClone(stk_pop());
    af = NcClone(NcGetSlot(fn, NSSYM0(argFrame)));
    NcSetSlot(af, NSSYM0(_nextArgFrame), LOCALS);
    NcSetSlot(af, NSSYM0(_parent), RCVR);
    NcSetSlot(af, NSSYM0(_implementor), IMPL);
    NcSetSlot(fn, NSSYM0(argFrame), af);
    stk_push(fn);
}


/*------------------------------------------------------------------------*/
/** �C�e���[�^�����ɐi�߂�
 *
 * @return			�Ȃ�
 */

void si_iternext(void)
{
    iter_next(stk_pop());
}


/*------------------------------------------------------------------------*/
/** �C�e���[�^���I�����`�F�b�N
 *
 * @return			�Ȃ�
 */

void si_iterdone(void)
{
    newtRefVar	iter;

    iter = stk_pop();
    stk_push(NewtMakeBoolean(iter_done(iter)));
}


/*------------------------------------------------------------------------*/
/** ��O�n���h�����|�b�v����
 *
 * @return			�Ȃ�
 */

void si_pop_handlers(void)
{
    excp_pop_handlers();
    CURREXCP = kNewtRefUnbind;
}


#pragma mark -
#pragma mark *** Primitive functions
/*------------------------------------------------------------------------*/
/** ���Z
 *
 * @return			�Ȃ�
 */

void fn_add(void)
{
    newtRefVar	a1;
    newtRefVar	a2;

    a2 = stk_pop();
    a1 = stk_pop();

    stk_push(NcAdd(a1, a2));
}


/*------------------------------------------------------------------------*/
/** ���Z
 *
 * @return			�Ȃ�
 */

void fn_subtract(void)
{
    newtRefVar	a1;
    newtRefVar	a2;

    a2 = stk_pop();
    a1 = stk_pop();

    stk_push(NcSubtract(a1, a2));
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̎w�肳�ꂽ�ʒu����l���擾
 *
 * @return			�Ȃ�
 */

void fn_aref(void)
{
    newtRefVar	a1;
    newtRefVar	a2;

    a2 = stk_pop();
    a1 = stk_pop();

    stk_push(NcARef(a1, a2));
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̎w�肳�ꂽ�ʒu�ɒl���Z�b�g����
 *
 * @return			�Ȃ�
 */

void fn_set_aref(void)
{
    newtRefVar	a1;
    newtRefVar	a2;
    newtRefVar	a3;

    a3 = stk_pop0();
    a2 = stk_pop();
    a1 = stk_pop();

    stk_push(NcSetARef(a1, a2, a3));
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̓��l���`�F�b�N
 *
 * @return			�Ȃ�
 */

void fn_equals(void)
{
    newtRefVar	a1;
    newtRefVar	a2;

    a2 = stk_pop();
    a1 = stk_pop();

    stk_push(NcRefEqual(a1, a2));
}


/*------------------------------------------------------------------------*/
/** �u�[���l�̔ے�
 *
 * @return			�Ȃ�
 */

void fn_not(void)
{
    newtRefVar	x;
    newtRefVar	r;

    x = stk_pop();
    r = NewtMakeBoolean(NewtRefIsNIL(x));

    stk_push(r);
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̈Ⴂ���`�F�b�N
 *
 * @return			�Ȃ�
 */

void fn_not_equals(void)
{
    newtRefVar	a1;
    newtRefVar	a2;
    newtRefVar	r;

    a2 = stk_pop();
    a1 = stk_pop();

    r = NewtMakeBoolean(! NewtRefEqual(a1, a2));

    stk_push(r);
}


/*------------------------------------------------------------------------*/
/** ��Z
 *
 * @return			�Ȃ�
 */

void fn_multiply(void)
{
    newtRefVar	a1;
    newtRefVar	a2;

    a2 = stk_pop();
    a1 = stk_pop();

    stk_push(NcMultiply(a1, a2));
}


/*------------------------------------------------------------------------*/
/** ���Z
 *
 * @return			�Ȃ�
 */

void fn_divide(void)
{
    newtRefVar	a1;
    newtRefVar	a2;

    a2 = stk_pop();
    a1 = stk_pop();

    stk_push(NcDivide(a1, a2));
}


/*------------------------------------------------------------------------*/
/** �����̊��Z
 *
 * @return			�Ȃ�
 */

void fn_div(void)
{
    newtRefVar	a1;
    newtRefVar	a2;

    a2 = stk_pop();
    a1 = stk_pop();

    stk_push(NcDiv(a1, a2));
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̔�r(<)
 *
 * @return			�Ȃ�
 */

void fn_less_than(void)
{
    newtRefVar	a1;
    newtRefVar	a2;

    a2 = stk_pop();
    a1 = stk_pop();

    stk_push(NcLessThan(a1, a2));
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̔�r(>)
 *
 * @return			�Ȃ�
 */

void fn_greater_than(void)
{
    newtRefVar	a1;
    newtRefVar	a2;

    a2 = stk_pop();
    a1 = stk_pop();

    stk_push(NcGreaterThan(a1, a2));
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̔�r(>=)
 *
 * @return			�Ȃ�
 */

void fn_greater_or_equal(void)
{
    newtRefVar	a1;
    newtRefVar	a2;

    a2 = stk_pop();
    a1 = stk_pop();

    stk_push(NcGreaterOrEqual(a1, a2));
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̔�r(<=)
 *
 * @return			�Ȃ�
 */

void fn_less_or_equal(void)
{
    newtRefVar	a1;
    newtRefVar	a2;

    a2 = stk_pop();
    a1 = stk_pop();

    stk_push(NcLessOrEqual(a1, a2));
}


/*------------------------------------------------------------------------*/
/** �r�b�g���Z AND
 *
 * @return			�Ȃ�
 */

void fn_bit_and(void)
{
    newtRefVar	a1;
    newtRefVar	a2;

    a2 = stk_pop();
    a1 = stk_pop();

    stk_push(NcBAnd(a1, a2));
}


/*------------------------------------------------------------------------*/
/** �r�b�g���Z OR
 *
 * @return			�Ȃ�
 */

void fn_bit_or(void)
{
    newtRefVar	a1;
    newtRefVar	a2;

    a2 = stk_pop();
    a1 = stk_pop();

    stk_push(NcBOr(a1, a2));
}


/*------------------------------------------------------------------------*/
/** �r�b�g���Z NOT
 *
 * @return			�Ȃ�
 */

void fn_bit_not(void)
{
    newtRefVar	x;

    x = stk_pop();
    stk_push(NcBNot(x));
}


/*------------------------------------------------------------------------*/
/** �C�e���[�^�̍쐬
 *
 * @return			�Ȃ�
 */

void fn_new_iterator(void)
{
    newtRefVar	a1;
    newtRefVar	a2;

    a2 = stk_pop();
    a1 = stk_pop();

    if (! NewtRefIsFrameOrArray(a1) && ! NewtRefIsBinary(a1))
    {
        NewtThrow(kNErrNotAFrameOrArray, a1);
        return;
    }

    stk_push(iter_new(a1, a2));
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̒������擾
 *
 * @return			�Ȃ�
 */

void fn_length(void)
{
    newtRefVar	x;

    x = stk_pop();
    stk_push(NcLength(x));
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g���N���[����������
 *
 * @return			�Ȃ�
 */

void fn_clone(void)
{
    newtRefVar	x;

    x = stk_pop();
    stk_push(NcClone(x));
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�ɃN���X���Z�b�g����
 *
 * @return			�Ȃ�
 */

void fn_set_class(void)
{
    newtRefVar	a1;
    newtRefVar	a2;

    a2 = stk_pop();
    a1 = stk_pop();

    stk_push(NcSetClass(a1, a2));
}


/*------------------------------------------------------------------------*/
/** �z��I�u�W�F�N�g�ɃI�u�W�F�N�g��ǉ�����
 *
 * @return			�Ȃ�
 */

void fn_add_array_slot(void)
{
    newtRefVar	a1;
    newtRefVar	a2;

    a2 = stk_pop0();
    a1 = stk_pop();

    stk_push(NcAddArraySlot(a1, a2));
}


/*------------------------------------------------------------------------*/
/** �z��I�u�W�F�N�g�̗v�f�𕶎���ɍ�������
 *
 * @return			�Ȃ�
 */

void fn_stringer(void)
{
    newtRefVar	x;

    x = stk_pop();
    stk_push(NcStringer(x));
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g���̃A�N�Z�X�p�X�̗L���𒲂ׂ�
 *
 * @return			�Ȃ�
 */

void fn_has_path(void)
{
    newtRefVar	a1;
    newtRefVar	a2;

    a2 = stk_pop();
    a1 = stk_pop();

    stk_push(NcHasPath(a1, a2));
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̃N���X�V���{�����擾
 *
 * @return			�Ȃ�
 */

void fn_classof(void)
{
    newtRefVar	x;

    x = stk_pop();
    stk_push(NcClassOf(x));
}


#pragma mark -
#pragma mark *** Instructions
/*------------------------------------------------------------------------*/
/** ���߃Z�b�g�e�[�u���o�^�p�̃_�~�[
 *
 * @param b		[in] �I�y�f�[�^
 *
 * @return		�Ȃ�
 */

void is_dummy(int16_t b)
{
    NewtThrow0(kNErrInvalidInstruction);
}


/*------------------------------------------------------------------------*/
/** �V���v�����߂̎��s
 *
 * @param b		[in] �I�y�f�[�^
 *
 * @return		�Ȃ�
 */

void is_simple_instructions(int16_t b)
{
    if (b < kNBCSimpleInstructionsLen)
        (simple_instructions[b])();
    else
        NewtThrow0(kNErrInvalidInstruction);
}


/*------------------------------------------------------------------------*/
/** �X�^�b�N�Ƀv�b�V��
 *
 * @param b		[in] �I�y�f�[�^
 *
 * @return		�Ȃ�
 */

void is_push(int16_t b)
{
    stk_push(liter_get(b));
}


/*------------------------------------------------------------------------*/
/** �X�^�b�N�ɒ萔���v�b�V��
 *
 * @param b		[in] �I�y�f�[�^
 *
 * @return		�Ȃ�
 */

void is_push_constant(int16_t b)
{
    newtRefVar	r;

    r = (newtRef)b;

    if (NewtRefIsInteger(r))
	{
		int32_t	n;

		n = NewtRefToInteger(r);

		if (8191 < n)
		{	// ���̐�
			n |= 0xFFFFC000;
			r = NewtMakeInt30(r);
		}
	}
	else
	{
        r = (r & 0xffff);
	}

    stk_push(r);
}


/*------------------------------------------------------------------------*/
/** �O���[�o���֐��̌ďo��
 *
 * @param b		[in] �I�y�f�[�^
 *
 * @return		�Ȃ�
 */

void is_call(int16_t b)
{
    newtRefVar	name;
    newtRefVar	fn;

    name = stk_pop();
    fn = NcGetGlobalFn(name);

    if (NewtRefIsNIL(fn))
    {
        NVMThrowBC(kNErrUndefinedGlobalFunction, name, b, true);
        return;
    }

    NVMFuncCall(fn, b);
}


/*------------------------------------------------------------------------*/
/** �֐��I�u�W�F�N�g�̎��s
 *
 * @param b		[in] �I�y�f�[�^
 *
 * @return		�Ȃ�
 */

void is_invoke(int16_t b)
{
    NVMFuncCall(stk_pop(), b);
}


/*------------------------------------------------------------------------*/
/** ���\�b�h�̑��M
 *
 * @param b		[in] �I�y�f�[�^
 *
 * @return		�Ȃ�
 */

void is_send(int16_t b)
{
	newtRef name;
    newtErr	err;

    name = vm_send(b, &err);

    if (err != kNErrNone)
        NVMThrowBC(err, name, 0, true);
}


/*------------------------------------------------------------------------*/
/** ���\�b�h�̑��M
 *
 * @param b		[in] �I�y�f�[�^
 *
 * @return		�Ȃ�
 *
 * @note		���\�b�h����`����Ă��Ȃ��Ă���O�͔������Ȃ�
 */

void is_send_if_defined(int16_t b)
{
    newtErr	err;

    vm_send(b, &err);

    if (err != kNErrNone)
        stk_push(kNewtRefNIL);
}


/*------------------------------------------------------------------------*/
/** ���\�b�h�̍đ��M
 *
 * @param b		[in] �I�y�f�[�^
 *
 * @return		�Ȃ�
 */

void is_resend(int16_t b)
{
	newtRef name;
    newtErr	err;

    name = vm_resend(b, &err);

    if (err != kNErrNone)
        NVMThrowBC(err, name, 0, true);
}


/*------------------------------------------------------------------------*/
/** ���\�b�h�̍đ��M
 *
 * @param b		[in] �I�y�f�[�^
 *
 * @return		�Ȃ�
 *
 * @note		���\�b�h����`����Ă��Ȃ��Ă���O�͔������Ȃ�
 */

void is_resend_if_defined(int16_t b)
{
    newtErr	err;

    vm_resend(b, &err);

    if (err != kNErrNone)
        stk_push(kNewtRefNIL);
}


/*------------------------------------------------------------------------*/
/** ����
 *
 * @param b		[in] �I�y�f�[�^
 *
 * @return		�Ȃ�
 */

void is_branch(int16_t b)
{
    PC = b;
}


/*------------------------------------------------------------------------*/
/** �����t������i�X�^�b�N�̐擪���^�̏ꍇ�j
 *
 * @param b		[in] �I�y�f�[�^
 *
 * @return		�Ȃ�
 */

void is_branch_if_true(int16_t b)
{
    newtRefVar	x;

    x = stk_pop();

    if (NewtRefIsNotNIL(x))
        PC = b;
}


/*------------------------------------------------------------------------*/
/** �����t������i�X�^�b�N�̐擪���U�̏ꍇ�j
 *
 * @param b		[in] �I�y�f�[�^
 *
 * @return		�Ȃ�
 */

void is_branch_if_false(int16_t b)
{
    newtRefVar	x;

    x = stk_pop();

    if (NewtRefIsNIL(x))
        PC = b;
}


/*------------------------------------------------------------------------*/
/** �ϐ��̌���
 *
 * @param b		[in] �I�y�f�[�^
 *
 * @return		�Ȃ�
 */

void is_find_var(int16_t b)
{
    newtRefVar	name;
    newtRefVar	v;

    name = liter_get(b);

    v = NcLexicalLookup(LOCALS, name);

    if (v != kNewtRefUnbind)
    {
        stk_push(v);
        return;
    }

    v = NcFullLookup(RCVR, name);

    if (v != kNewtRefUnbind)
    {
        stk_push(v);
        return;
    }

    if (NewtHasGlobalVar(name))
    {
        stk_push(NcGetGlobalVar(name));
        return;
    }

    NewtThrowSymbol(kNErrUndefinedVariable, name);
    stk_push(kNewtRefUnbind);
}


/*------------------------------------------------------------------------*/
/** ���[�J���ϐ��̎�o��
 *
 * @param b		[in] �I�y�f�[�^
 *
 * @return		�Ȃ�
 */

void is_get_var(int16_t b)
{
    stk_push(NewtGetFrameSlot(LOCALS, b));
}


/*------------------------------------------------------------------------*/
/** �t���[���̍쐬
 *
 * @param b		[in] �I�y�f�[�^
 *
 * @return		�Ȃ�
 */

void is_make_frame(int16_t b)
{
    newtRefVar	map;
    newtRefVar	f;
    newtRefVar	v;
    int16_t	i;

    map = stk_pop();
    f = NewtMakeFrame(map, b);

    for (i = b - 1; 0 <= i; i--)
    {
        v = stk_pop0();
        NewtSetFrameSlot(f, i, v);
    }

    stk_push(f);
}


/*------------------------------------------------------------------------*/
/** �z��̍쐬
 *
 * @param b		[in] �I�y�f�[�^
 *
 * @return		�Ȃ�
 */

void is_make_array(int16_t b)
{
    newtRefVar	c;
    newtRefVar	x;
    newtRefVar	a = kNewtRefNIL;

    c = stk_pop();

    if ((uint16_t)b == 0xFFFF)
    {
        x = stk_pop();

        if (NewtRefIsInteger(x))
            a = NewtMakeArray(kNewtRefUnbind, NewtRefToInteger(x));
    }
    else
    {
        a = NewtMakeArray(kNewtRefUnbind, b);

        if (NewtRefIsNotNIL(a))
        {
            int16_t	i;

            for (i = b - 1; 0 <= i; i--)
            {
                x = stk_pop0();
                NewtSetArraySlot(a, i, x);
            }
        }
    }

    if (NewtRefIsNotNIL(a) && NewtRefIsNotNIL(c))
        NcSetClass(a, c);

    stk_push(a);
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̃A�N�Z�X�p�X�̒l����o��
 *
 * @param b		[in] �I�y�f�[�^
 *
 * @return		�Ȃ�
 */

void is_get_path(int16_t b)
{
    newtRefVar	a1;
    newtRefVar	a2;
    newtRefVar	r = kNewtRefNIL;

    a2 = stk_pop();
    a1 = stk_pop();

    if (NewtRefIsNotNIL(a1))
    {
        r = NcGetPath(a1, a2);
    }
    else
    {
        if (b != 0)
            NewtThrow0(kNErrPathFailed);
    }

    stk_push(r);
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̃A�N�Z�X�p�X�ɒl���Z�b�g����
 *
 * @param b		[in] �I�y�f�[�^
 *
 * @return		�Ȃ�
 */

void is_set_path(int16_t b)
{
    newtRefVar	a1;
    newtRefVar	a2;
    newtRefVar	a3;
    newtRefVar	r = kNewtRefNIL;

    a3 = stk_pop0();
    a2 = stk_pop();
    a1 = stk_pop();

    if (NewtRefIsNotNIL(a1))
        r = NcSetPath(a1, a2, a3);

    if (b == 1)
        stk_push(r);
}


/*------------------------------------------------------------------------*/
/** ���[�J���ϐ��ɒl���Z�b�g����
 *
 * @param b		[in] �I�y�f�[�^
 *
 * @return		�Ȃ�
 */

void is_set_var(int16_t b)
{
    newtRefVar	x;

    x = stk_pop0();
    NewtSetFrameSlot(LOCALS, b, x);
}


/*------------------------------------------------------------------------*/
/** �ϐ��ɒl���Z�b�g����
 *
 * @param b		[in] �I�y�f�[�^
 *
 * @return		�Ȃ�
 */

void is_find_and_set_var(int16_t b)
{
    newtRefVar	name;
    newtRefVar	v;

    name = liter_get(b);
    v = stk_pop0();

    if (NewtLexicalAssignment(LOCALS, name, v))
        return;

    if (NewtAssignment(RCVR, name, v))
        return;

    if (NewtHasGlobalVar(name))
    {
        NcDefGlobalVar(name, v);
        return;
    }

    NcSetSlot(LOCALS, name, v);
}


/*------------------------------------------------------------------------*/
/** ����
 *
 * @param b		[in] �I�y�f�[�^
 *
 * @return		�Ȃ�
 */

void is_incr_var(int16_t b)
{
    newtRefVar	addend;
    newtRefVar	n;
    int32_t	v;

    addend = stk_top();
    n = NewtGetFrameSlot(LOCALS, b);
    v = NewtRefToInteger(n) + NewtRefToInteger(addend);
    n = NewtMakeInteger(v);

    NewtSetFrameSlot(LOCALS, b, n);
    stk_push(n);
}


/*------------------------------------------------------------------------*/
/** ���[�v�I�������Ńu�����`
 *
 * @param b		[in] �I�y�f�[�^
 *
 * @return		�Ȃ�
 */

void is_branch_if_loop_not_done(int16_t b)
{
    newtRefVar	r_incr;
    newtRefVar	r_index;
    newtRefVar	r_limit;
    int32_t	incr;
    int32_t	index;
    int32_t	limit;

    r_limit = stk_pop();
    r_index = stk_pop();
    r_incr = stk_pop();

    if (! NewtRefIsInteger(r_incr))
    {
        NewtThrow(kNErrNotAnInteger, r_incr);
        return;
    }

    if (! NewtRefIsInteger(r_index))
    {
        NewtThrow(kNErrNotAnInteger, r_index);
        return;
    }

    if (! NewtRefIsInteger(r_limit))
    {
        NewtThrow(kNErrNotAnInteger, r_limit);
        return;
    }

    incr = NewtRefToInteger(r_incr);
    index = NewtRefToInteger(r_index);
    limit = NewtRefToInteger(r_limit);

    if (incr == 0)
    {
        NewtThrow(kNErrZeroForLoopIncr, incr);
        return;
    }

    if (incr > 0 && index <= limit)
        PC = b;
    else if (incr < 0 && index >= limit)
        PC = b;
}


/*------------------------------------------------------------------------*/
/** �֐����߂̎��s
 *
 * @param b		[in] �I�y�f�[�^
 *
 * @return		�Ȃ�
 */

void is_freq_func(int16_t b)
{
    if (b < kBCFuncsLen)
        (fn_instructions[b])();
    else
        NewtThrow0(kNErrInvalidInstruction);
}


/*------------------------------------------------------------------------*/
/** ��O�n���h���̍쐬
 *
 * @param b		[in] �I�y�f�[�^
 *
 * @return		�Ȃ�
 */

void is_new_handlers(int16_t b)
{
    newtRefVar	a1;
    newtRefVar	a2;
    int16_t	i;

    for (i = 0; i < b; i++)
    {
        a2 = stk_pop();
        a1 = stk_pop();

        if (! excp_push(a1, a2))
        {
            if (0 < i)
                excp_pop_handlers();
        }
    }
}


#pragma mark *** �_���v
/*------------------------------------------------------------------------*/
/** �o�̓t�@�C���ɖ��߃R�[�h�̖��O���_���v�o��
 *
 * @param f			[in] �o�̓t�@�C��
 * @param a			[in] �I�y�R�[�h
 * @param b			[in] �I�y�f�[�^
 *
 * @return			�Ȃ�
 */

void NVMDumpInstName(FILE * f, uint8_t a, int16_t b)
{
    bool	dump_b;
    char *	name;

    dump_b = false;
    name = NULL;

    if (a < kNBCInstructionsLen)
    {
        switch (a)
        {
            case 0:
                if (b < kNBCSimpleInstructionsLen)
                    name = simple_instruction_names[b];
                break;

            case 24:	// freq-func (A = 24)
                if (b < kBCFuncsLen)
                    name = fn_instruction_names[b];
                break;

            default:
                name = vm_instruction_names[a];
                dump_b = true;
                break;
        }
    }

    if (name == NULL)
        name = "bad instructions";

    NewtFprintf(f, " %24s", name);

    if (dump_b)
        NewtFprintf(f, " (b = % 3d)", b);
    else
        NewtFprintf(f, "%10c", ' ');
}


/*------------------------------------------------------------------------*/
/** �o�̓t�@�C���ɃX�^�b�N�̐擪�ƃ��[�J���t���[�����_���v�o��
 *
 * @param f			[in] �o�̓t�@�C��
 *
 * @return			�Ȃ�
 */

void NVMDumpInstResult(FILE * f)
{
    NewtFprintf(f, " ");
    NVMDumpStackTop(f, "\t");
    NewtPrintObj(f, LOCALS);
    NewtFprintf(f, "\n");
}


/*------------------------------------------------------------------------*/
/** �o�̓t�@�C���ɖ��߃R�[�h���_���v�o��
 *
 * @param f			[in] �o�̓t�@�C��
 * @param bc		[in] �o�C�g�R�[�h
 * @param pc		[in] �v���O�����J�E���^
 * @param len		[in] ���߃R�[�h�̃o�C�g��
 *
 * @return			�Ȃ�
 */

void NVMDumpInstCode(FILE * f, uint8_t * bc, uint32_t pc, uint16_t len)
{
    NewtFprintf(stderr, "%6d : %02x", pc, bc[pc]);

    if (len == 3)
    {
        NewtFprintf(f, " %02x", bc[pc + 1]);
        NewtFprintf(f, " %02x", bc[pc + 2]);
    }
    else
    {
        NewtFprintf(f, "      ");
    }
}


/*------------------------------------------------------------------------*/
/** �o�̓t�@�C���ɃX�^�b�N�̐擪���_���v�o��
 *
 * @param f			[in] �o�̓t�@�C��
 * @param s			[in] ��؂蕶����
 *
 * @return			�Ȃ�
 */

void NVMDumpStackTop(FILE * f, char * s)
{
    NewtPrintObj(f, stk_top());

    if (s != NULL)
        NewtFputs(s, f);
}


/*------------------------------------------------------------------------*/
/** �o�̓t�@�C���Ƀo�C�g�R�[�h���_���v�o��
 *
 * @param f			[in] �o�̓t�@�C��
 * @param bc		[in] �o�C�g�R�[�h
 * @param len		[in] �o�C�g�R�[�h�̒���
 *
 * @return			�Ȃ�
 */

void NVMDumpCode(FILE * f, uint8_t * bc, uint32_t len)
{
    uint8_t	op;
    uint8_t	a;
    int16_t	b;
    uint16_t	oplen;
    uint32_t	pc = 0;

    while (pc < len)
    {
        op = bc[pc];
        b = op & kNBCFieldMask;
        a = (op & 0xff)>>3;

        if (b == kNBCFieldMask)
        {
            oplen = 3;

            b = (int16_t)bc[pc + 1] << 8;
            b += bc[pc + 2];
    
            if (a == 0)
            {
                if (b == 0x01)
                    b = kNBCFieldMask;
            }
        }
        else
        {
            oplen = 1;
        }

        NVMDumpInstCode(f, bc, pc, oplen);
        NVMDumpInstName(f, a, b);
        NewtFprintf(f, "\n");

        pc += oplen;
    }
}


/*------------------------------------------------------------------------*/
/** �o�̓t�@�C���Ƀo�C�g�R�[�h���_���v�o��
 *
 * @param f				[in] �o�̓t�@�C��
 * @param instructions	[in] �o�C�g�R�[�h
 *
 * @return			�Ȃ�
 */

void NVMDumpBC(FILE * f, newtRefArg instructions)
{
    if (NewtRefIsBinary(instructions))
    {
        uint8_t *	bc;
        uint32_t	len;
    
        len = NewtLength(instructions);
        bc = NewtRefToBinary(instructions);

        NVMDumpCode(f, bc, len);
    }
}


/*------------------------------------------------------------------------*/
/** �o�̓t�@�C���Ɋ֐��I�u�W�F�N�g���_���v�o��
 *
 * @param f			[in] �o�̓t�@�C��
 * @param func		[in] �֐��I�u�W�F�N�g
 *
 * @return			�Ȃ�
 */

void NVMDumpFn(FILE * f, newtRefArg func)
{
    newtRefVar	fn;

    fn = (newtRef)func;

    if (NewtRefIsNIL(fn))
        fn = FUNC;
	else if (NewtRefIsSymbol(fn))
		fn = NcGetGlobalFn(fn);

    if (NewtRefIsFunction(fn))
    {
        newtRefVar	instructions;
    
        NewtPrintObject(f, fn);
        NewtFprintf(f, "\n");
    
        instructions = NcGetSlot(fn, NSSYM0(instructions));
        NVMDumpBC(f, instructions);
    }
}


/*------------------------------------------------------------------------*/
/** �o�̓t�@�C���ɃX�^�b�N���_���v�o��
 *
 * @param f			[in] �o�̓t�@�C��
 *
 * @return			�Ȃ�
 */

void NVMDumpStacks(FILE * f)
{
    int32_t	i;

    for (i = SP - 1; 0 <= i; i--)
    {
        NewtPrintObject(f, STACK[i]);
    }
}


#pragma mark -
#pragma mark *** �C���^�v���^
/*------------------------------------------------------------------------*/
/** ���W�X�^�̏����� 
 *
 * @return			�Ȃ�
 */

void NVMInitREG(void)
{
    FUNC = kNewtRefNIL;
    PC = 0;
    SP = 0;
    LOCALS = kNewtRefNIL;
    RCVR = kNewtRefNIL;
    IMPL = kNewtRefNIL;

    BC = NULL;
    BCLEN = 0;
}


/*------------------------------------------------------------------------*/
/** �X�^�b�N�̏����� 
 *
 * @return			�Ȃ�
 */

void NVMInitSTACK(void)
{
    NewtStackSetup(&vm_env.stack, NEWT_POOL, sizeof(newtRef), NEWT_NUM_STACK);
    NewtStackSetup(&vm_env.callstack, NEWT_POOL, sizeof(vm_reg_t), NEWT_NUM_CALLSTACK);
    NewtStackSetup(&vm_env.excpstack, NEWT_POOL, sizeof(vm_excp_t), NEWT_NUM_EXCPSTACK);

    CURREXCP = kNewtRefUnbind;
}


/*------------------------------------------------------------------------*/
/** �X�^�b�N�̌�n�� 
 *
 * @return			�Ȃ�
 */

void NVMCleanSTACK(void)
{
    NewtStackFree(&vm_env.stack);
    NewtStackFree(&vm_env.callstack);
    NewtStackFree(&vm_env.excpstack);
}


/*------------------------------------------------------------------------*/
/** �K�{�O���[�o���֐��̏����� 
 *
 * @return			�Ȃ�
 */

void NVMInitGlobalFns0(void)
{
    NewtDefGlobalFunc(NSSYM0(hasVariable),	NsHasVariable,		2, "HasVariable(frame, name)");
    NewtDefGlobalFunc(NSSYM0(hasVar),		NsHasVar,			1, "HasVar(name)");
    NewtDefGlobalFunc(NSSYM0(defGlobalFn),	NsDefGlobalFn,		2, "DefGlobalFn(name, fn)");
    NewtDefGlobalFunc(NSSYM0(defGlobalVar),	NsDefGlobalVar,		2, "DefGlobalVar(name, value)");
//    NewtDefGlobalFunc(NSSYM0(and),			NsAnd,				2, "And(n1, n2)");
//    NewtDefGlobalFunc(NSSYM0(or),			NsOr,				2, "Or(n1, n2)");
    NewtDefGlobalFunc(NSSYM0(mod),			NsMod,				2, "Mod(n1, n2)");
    NewtDefGlobalFunc(NSSYM0(shiftLeft),	NsShiftLeft,		2, "ShiftLeft(n1, n2)");
    NewtDefGlobalFunc(NSSYM0(shiftRight),	NsShiftRight,		2, "ShiftRight(n1, n2)");
    NewtDefGlobalFunc(NSSYM0(objectEqual),	NsObjectEqual,		2, "ObjectEqual(obj1, obj2)");		// �Ǝ��g��
    NewtDefGlobalFunc(NSSYM0(defMagicPointer),NsDefMagicPointer,2, "DefMagicPointer(mp, value)");	// �Ǝ��g��

#ifdef __NAMED_MAGIC_POINTER__
    NewtDefGlobalFunc(NSSYM0(makeRegex),	NsMakeRegex,		2, "MakeRegex(pattern, opt)");		// �Ǝ��g��
#endif /* __NAMED_MAGIC_POINTER__ */

	NewtDefGlobalFunc(NSSYM(RemoveSlot),	NsRemoveSlot,		2, "RemoveSlot(obj, slot)");

    NewtDefGlobalFunc(NSSYM(Throw),			NsThrow,			2, "Throw(name, data)");
    NewtDefGlobalFunc(NSSYM(Rethrow),		NsRethrow,			0, "Rethrow()");
    NewtDefGlobalFunc(NSSYM(CurrentException),NsCurrentException,	0, "CurrentException()");
}


/*------------------------------------------------------------------------*/
/** �g���݃O���[�o���֐��̏�����
 *
 * @return			�Ȃ�
 */

void NVMInitGlobalFns1(void)
{
    NewtDefGlobalFunc(NSSYM(PrimClassOf),	NsPrimClassOf,		1, "PrimClassOf(obj)");
    NewtDefGlobalFunc(NSSYM(TotalClone),	NsTotalClone,		1, "TotalClone(obj)");
    NewtDefGlobalFunc(NSSYM(HasSubclass),	NsHasSubclass,		2, "HasSubclass(sub, super)");
    NewtDefGlobalFunc(NSSYM(IsSubclass),	NsIsSubclass,		2, "IsSubclass(sub, super)");
    NewtDefGlobalFunc(NSSYM(IsInstance),	NsIsInstance,		2, "IsInstance(obj, class)");
    NewtDefGlobalFunc(NSSYM(IsArray),		NsIsArray,			1, "IsArray(obj)");
    NewtDefGlobalFunc(NSSYM(IsFrame),		NsIsFrame,			1, "IsFrame(obj)");
    NewtDefGlobalFunc(NSSYM(IsBinary),		NsIsBinary,			1, "IsBinary(obj)");
    NewtDefGlobalFunc(NSSYM(IsSymbol),		NsIsSymbol,			1, "IsSymbol(obj)");
    NewtDefGlobalFunc(NSSYM(IsString),		NsIsString,			1, "IsString(obj)");
    NewtDefGlobalFunc(NSSYM(IsCharacter),	NsIsCharacter,		1, "IsCharacter(obj)");
    NewtDefGlobalFunc(NSSYM(IsInteger),		NsIsInteger,		1, "IsInteger(obj)");
    NewtDefGlobalFunc(NSSYM(IsReal),		NsIsReal,			1, "IsReal(obj)");
    NewtDefGlobalFunc(NSSYM(IsNumber),		NsIsNumber,			1, "IsNumber(obj)");
    NewtDefGlobalFunc(NSSYM(IsImmediate),	NsIsImmediate,		1, "IsImmediate(obj)");
    NewtDefGlobalFunc(NSSYM(IsFunction),	NsIsFunction,		1, "IsFunction(obj)");
    NewtDefGlobalFunc(NSSYM(IsReadonly),	NsIsReadonly,		1, "IsReadonly(obj)");

    NewtDefGlobalFunc(NSSYM(Intern),		NsMakeSymbol,		1, "Intern(str)");
    NewtDefGlobalFunc(NSSYM(MakeBinary),	NsMakeBinary,		2, "MakeBinary(length, class)");
    NewtDefGlobalFunc(NSSYM(SetLength),		NsSetLength,		2, "SetLength(obj, len)");

    NewtDefGlobalFunc(NSSYM(HasSlot),		NsHasSlot,			2, "HasSlot(frame, slot)");
    NewtDefGlobalFunc(NSSYM(GetSlot),		NsGetSlot,			2, "GetSlot(frame, slot)");
    NewtDefGlobalFunc(NSSYM(SetSlot),		NsSetSlot,			3, "SetSlot(frame, slot, v)");
    NewtDefGlobalFunc(NSSYM(GetVariable),	NsGetVariable,		2, "GetVariable(frame, slot)");
    NewtDefGlobalFunc(NSSYM(SetVariable),	NsSetVariable,		3, "SetVariable(frame, slot, v)");

    NewtDefGlobalFunc(NSSYM(GetRoot),		NsGetRoot,			0, "GetRoot()");
    NewtDefGlobalFunc(NSSYM(GetGlobals),	NsGetGlobals,		0, "GetGlobals()");
    NewtDefGlobalFunc(NSSYM(GC),			NsGC,				0, "GC()");
    NewtDefGlobalFunc(NSSYM(Compile),		NsCompile,			1, "Compile(str)");

    NewtDefGlobalFunc(NSSYM(GetGlobalFn),	NsGetGlobalFn,		1, "GetGlobalFn(symbol)");
	NewtDefGlobalFunc(NSSYM(GetGlobalVar),	NsGetGlobalVar,		1, "GetGlobalVar(symbol)");
    NewtDefGlobalFunc(NSSYM(GlobalFnExists),NsGlobalFnExists,	1, "GlobalFnExists(symbol)");
    NewtDefGlobalFunc(NSSYM(GlobalVarExists),NsGlobalVarExists,	1, "GlobalVarExists(symbol)");
    NewtDefGlobalFunc(NSSYM(UndefGlobalFn),	NsUndefGlobalFn,	1, "UndefGlobalFn(symbol)");
    NewtDefGlobalFunc(NSSYM(UndefGlobalVar),NsUndefGlobalVar,	1, "UndefGlobalVar(symbol)");

    NewtDefGlobalFunc(NSSYM(Chr),			NsChr,				1, "Chr(integer)");
    NewtDefGlobalFunc(NSSYM(Ord),			NsOrd,				1, "Ord(char)");
    NewtDefGlobalFunc(NSSYM(StrLen),		NsStrLen,			1, "StrLen(str)");
    NewtDefGlobalFunc(NSSYM(SubStr),		NsSubStr,			3, "SubStr(str, start, count)");
    NewtDefGlobalFunc(NSSYM(StrEqual),		NsStrEqual,			2, "StrEqual(a, b)");
    NewtDefGlobalFunc(NSSYM(StrExactCompare),NsStrExactCompare,	2, "StrExactCompare(a, b)");
    NewtDefGlobalFunc(NSSYM(BeginsWith),	NsBeginsWith,		2, "BeginsWith(str, sub)");
    NewtDefGlobalFunc(NSSYM(EndsWith),		NsEndsWith,			2, "EndsWith(str, sub)");
    NewtDefGlobalFunc(NSSYM(SPrintObject),	NsSPrintObject,		1, "SPrintObject(obj)");
    NewtDefGlobalFunc(NSSYM(SymbolCompareLex),	NsSymbolCompareLex,	2, "SymbolCompareLex(symbol1, symbol2)");
}


/*------------------------------------------------------------------------*/
/** �ǉ��O���[�o���֐��̏�����
 *
 * @return			�Ȃ�
 */

void NVMInitExGlobalFns(void)
{
    NewtDefGlobalFunc(NSSYM(P),			NsPrintObject,		1, "P(obj)");
    NewtDefGlobalFunc(NSSYM(Print),		NsPrint,			1, "Print(obj)");

    NewtDefGlobalFunc(NSSYM(CompileFile),NsCompileFile,		1, "CompileFile(file)");

#ifdef HAVE_DLOPEN
    NewtDefGlobalFunc(NSSYM(LoadLib),	NsLoadLib,			1, "LoadLib(file)");
#endif /* HAVE_DLOPEN */

    NewtDefGlobalFunc(NSSYM(Load),		NsLoad,				1, "Load(file)");
	NewtDefGlobalFunc(NSSYM(Require),	NsRequire,			1, "Require(str)");

	NewtDefGlobalFunc(NSSYM(MakeNSOF),	NsMakeNSOF,			2, "MakeNSOF(obj, ver)");
	NewtDefGlobalFunc(NSSYM(ReadNSOF),	NsReadNSOF,			1, "ReadNSOF(nsof)");

    NewtDefGlobalFunc(NSSYM(GetEnv),	NsGetEnv,			1, "GetEnv(str)");

    NewtDefGlobalFunc(NSSYM(FileExists),NsFileExists,		1, "FileExists(path)");

    NewtDefGlobalFunc(NSSYM(DirName),	NsDirName,			1, "DirName(path)");
    NewtDefGlobalFunc(NSSYM(BaseName),	NsBaseName,			1, "BaseName(path)");
    NewtDefGlobalFunc(NSSYM(JoinPath),	NsJoinPath,			2, "JoinPath(dir, fname)");
    NewtDefGlobalFunc(NSSYM(ExpandPath),NsExpandPath,		1, "ExpandPath(path)");

    NewtDefGlobalFunc(NSSYM(Split),		NsSplit,			2, "Split(str, sep)");
    NewtDefGlobalFunc(NSSYM(ParamStr),	NsParamStr,			2, "ParamStr(baseString, paramStrArray)");
    NewtDefGlobalFunc(NSSYM(StrCat),	NsStrCat,			2, "StrCat(str1, str2)");

    NewtDefGlobalFunc(NSSYM(ExtractByte),NsExtractByte,		2, "ExtractByte(data, offset)");

    NewtDefGlobalFunc(NSSYM(Gets),		NsGets,				0, "Gets()");
    NewtDefGlobalFunc(NSSYM(Getc),		NsGetc,				0, "Getc()");
    NewtDefGlobalFunc(NSSYM(Getch),		NsGetch,			0, "Getch()");
}


/*------------------------------------------------------------------------*/
/**�@�f�o�b�O�p�O���[�o���֐��̏�����
 *
 * @return			�Ȃ�
 */

void NVMInitDebugGlobalFns(void)
{
    NewtDefGlobalFunc(NSSYM(DumpFn),		NsDumpFn,			1, "DumpFn(fn)");
    NewtDefGlobalFunc(NSSYM(DumpBC),		NsDumpBC,			1, "DumpBC(instructions)");
    NewtDefGlobalFunc(NSSYM(DumpStacks),	NsDumpStacks,		0, "DumpStacks()");
}


/*------------------------------------------------------------------------*/
/**�@�O���[�o���֐��̏�����
 *
 * @return			�Ȃ�
 */

void NVMInitGlobalFns(void)
{
    NVMInitGlobalFns0();		// �K�{
    NVMInitGlobalFns1();		// �g���݊֐�
    NVMInitExGlobalFns();		// �ǉ�
    NVMInitDebugGlobalFns();	// �f�o�b�O�p
}


/*------------------------------------------------------------------------*/
/**�@�O���[�o���ϐ��̏�����
 *
 * @return			�Ȃ�
 */

void NVMInitGlobalVars(void)
{
    NcDefGlobalVar(NSSYM0(printDepth), NSINT(3));
    NcDefGlobalVar(NSSYM0(printLength), NSINT(10));

    NcDefGlobalVar(NSSYM0(_STDOUT_), kNewtRefNIL);
    NcDefGlobalVar(NSSYM0(_STDERR_), kNewtRefNIL);
}


/*------------------------------------------------------------------------*/
/**�@VM �̏�����
 *
 * @return			�Ȃ�
 */

void NVMInit(void)
{
	newtRefVar  result;

	vm_env.level = 0;

    NVMInitREG();
    NVMInitSTACK();
    NVMInitGlobalFns();
    NVMInitGlobalVars();

	// �W�����C�u�����̃��[�h
	result = NcRequire0(NSSTR("egg"));
	NVMClearCurrException();

	if (result != kNewtRefUnbind)
	{
		stk_pop();
	}
}


/*------------------------------------------------------------------------*/
/**�@VM �̌�n��
 *
 * @return			�Ȃ�
 */

void NVMClean(void)
{
    NVMCleanSTACK();
}


/*------------------------------------------------------------------------*/
/**�@VM���[�v
 *
 * @param callsp	[in] �ďo���X�^�b�N�̃X�^�b�N�|�C���^
 *
 * @return			�Ȃ�
 */

void NVMLoop(uint32_t callsp)
{
    uint16_t	oplen;
    int16_t	b;
    uint8_t	op;
    uint8_t	a;

	vm_env.level++;

	if (NEWT_DEBUG)
		NewtDebugMsg("VM", "VM Level = %d\n", vm_env.level);

    while (callsp < CALLSP && PC < BCLEN)
    {
        op = BC[PC];

        b = op & kNBCFieldMask;
        a = (op & 0xff)>>3;

        if (b == kNBCFieldMask)
        {
            oplen = 3;

            b = (int16_t)BC[PC + 1] << 8;
            b += BC[PC + 2];
    
            if (a == 0)
            {
                if (b == 0x01)
                    b = kNBCFieldMask;
            }
        }
        else
        {
            oplen = 1;
        }
    
        if (NEWT_TRACE)
        {
            NVMDumpInstCode(stderr, BC, PC, oplen);
            NVMDumpInstName(stderr, a, b);
        }

        PC += oplen;
    
        if (a < kNBCInstructionsLen)
            (is_instructions[a])(b);

        if (NEWT_TRACE)
            NVMDumpInstResult(stderr);

        if (NEWT_NEEDGC)
            NewtGC();
    }

	vm_env.level--;
}


/*------------------------------------------------------------------------*/
/** �ďo���֐��I�u�W�F�N�g���Z�b�g
 *
 * @param fn		[in] �֐��I�u�W�F�N�g
 * @param numArgs	[in] �����̐�
 *
 * @return			�Ȃ�
 */

void NVMFnCall(newtRefArg fn, int16_t numArgs)
{
    stk_push(fn);  
    si_set_lex_scope();
    is_invoke(numArgs);
}


/*------------------------------------------------------------------------*/
/** �֐��I�u�W�F�N�g�����s
 *
 * @param fn		[in] �֐��I�u�W�F�N�g
 * @param numArgs	[in] �����̐�
 * @param errP		[out]�G���[�R�[�h
 *
 * @return			�X�^�b�N�̃g�b�v�I�u�W�F�N�g
 */

newtRef NVMCall(newtRefArg fn, int16_t numArgs, newtErr * errP)
{

	newtRefVar	result = kNewtRefUnbind;
	newtErr		err = kNErrNone;

    if (NewtRefIsNotNIL(fn))
    {
        NVMFnCall(fn, numArgs);
        NVMLoop(CALLSP - 1);
        result = stk_top();
    }

    if (errP != NULL)
	{
		if (err == kNErrNone)
			err = NVMGetExceptionErrCode(CURREXCP, true);

        *errP = err;
	}

    return result;
}


/*------------------------------------------------------------------------*/
/** �֐��I�u�W�F�N�g�����s
 *
 * @param fn		[in] �֐��I�u�W�F�N�g
 * @param errP		[out]�G���[�R�[�h
 *
 * @return			�X�^�b�N�̃g�b�v�I�u�W�F�N�g
 */

newtRef NVMInterpret(newtRefArg fn, newtErr * errP)
{
	newtRefVar	result = kNewtRefUnbind;
	newtErr		err = kNErrNone;

    if (NewtRefIsNotNIL(fn))
    {
        if (NEWT_TRACE)
            NewtFprintf(stderr, "*** trace ***\n");

		NVMInit();
		NewtGC();

        result = NVMCall(fn, 0, &err);

        NVMClean();
    }

    if (errP != NULL)
        *errP = err;

    return result;
}


/*------------------------------------------------------------------------*/
/** �\���؂��o�C�g�R�[�h�ɕϊ����Ď��s
 *
 * @param stree		[in] �\����
 * @param numStree  [in] �\���؂̒���
 * @param errP		[out]�G���[�R�[�h
 *
 * @return			�X�^�b�N�̃g�b�v�I�u�W�F�N�g
 */

newtRef NVMInterpret2(nps_syntax_node_t * stree, uint32_t numStree, newtErr * errP)
{
    newtRefVar	result = kNewtRefUnbind;

    if (errP != NULL)
        *errP = kNErrNone;

    if (stree != NULL)
    {
        newtRefVar	fn = kNewtRefNIL;

        fn = NBCGenBC(stree, numStree, true);
        NPSCleanup();

        result = NVMInterpret(fn, errP);
    }

    return result;
}


/*------------------------------------------------------------------------*/
/** �w�肳�ꂽ�֐��̏���\��
 *
 * @param name		[in] �֐���
 *
 * @return			�G���[�R�[�h
 */

newtErr NVMInfo(const char * name)
{
    newtErr	err = kNErrNone;

    NVMInit();

    if (name != NULL)
        NewtInfo(NewtMakeSymbol(name));
    else
        NewtInfoGlobalFns();

    NVMClean();

    return err;
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �t�@�C����Ǎ���ŃX�N���v�g�����s
 *
 * @param path		[in] �t�@�C���̃p�X
 * @param errP		[out]�G���[�R�[�h
 *
 * @return			�X�^�b�N�̃g�b�v�I�u�W�F�N�g
 */

newtRef NVMInterpretFile(const char * path, newtErr * errP)
{
    nps_syntax_node_t *	stree;
    uint32_t	numStree;
    newtRefVar	result = kNewtRefUnbind;
    newtErr	err;

    err = NPSParseFile(path, &stree, &numStree);

    if (err == kNErrNone)
        result = NVMInterpret2(stree, numStree, &err);

    if (errP != NULL)
        *errP = err;

    return result;
}


/*------------------------------------------------------------------------*/
/** ��������R���p�C�����ăX�N���v�g�����s
 *
 * @param s			[in] ������
 * @param errP		[out]�G���[�R�[�h
 *
 * @return			�X�^�b�N�̃g�b�v�I�u�W�F�N�g
 */

newtRef NVMInterpretStr(const char * s, newtErr * errP)
{
    nps_syntax_node_t *	stree;
    uint32_t	numStree;
    newtRefVar	result = kNewtRefUnbind;
    newtErr	err;

    err = NPSParseStr(s, &stree, &numStree);

    if (err == kNErrNone)
        result = NVMInterpret2(stree, numStree, &err);

    if (errP != NULL)
        *errP = err;

    return result;
}


/**
 * Execute a function as if it was a method of an object.
 *
 * @param inImpl		implementor.
 * @param inRcvr		object.
 * @param inFunction	function to execute.
 * @param inArgs		array of arguments.
 * @return the result of the call.
 */
newtRef
NVMMessageSendWithArgArray(
	newtRefArg inImpl,
	newtRefArg inRcvr,
	newtRefArg inFunction,
	newtRefArg inArgs)
{
	newtRefVar	result;
	vm_env_t	saveVM;
	int16_t		nbArgs;
	int			indexArgs;

	/* save the VM */
	saveVM = vm_env;
	
	nbArgs = NewtArrayLength(inArgs);
	/* Push the arguments on the stack */
	for (indexArgs = 0; indexArgs < nbArgs; indexArgs++)
	{
		stk_push(NewtGetArraySlot(inArgs, indexArgs));
	}
	
	/* Send the message */
	NVMMessageSend(inImpl, inRcvr, inFunction, nbArgs);
	NVMLoop(CALLSP - 1);
	result = stk_top();

	/* restore the VM */
	vm_env = saveVM;

	return result;
}
