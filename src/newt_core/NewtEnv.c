/*------------------------------------------------------------------------*/
/**
 * @file	NewtEnv.c
 * @brief   ���s��
 *
 * @author  M.Nukui
 * @date	2003-11-07
 *
 * Copyright (C) 2003-2004 M.Nukui All rights reserved.
 */


/* �w�b�_�t�@�C�� */
#include "version.h"
#include "NewtEnv.h"
#include "NewtObj.h"
#include "NewtFns.h"
#include "NewtGC.h"
#include "NewtStr.h"
#include "NewtFile.h"


/* �}�N�� */
#define SYM_TABLE			(newt_env.sym_table)				///< �V���{���e�[�u��
#define ROOT				(newt_env.root)						///< ���[�g�I�u�W�F�N�g
#define GLOBALS				(newt_env.globals)					///< �O���[�o���ϐ��e�[�u��
#define GLOBAL_FNS			(newt_env.global_fns)				///< �O���[�o���֐��e�[�u��
#define MAGIC_POINTERS		(newt_env.magic_pointers)			///< �}�W�b�N�|�C���^�e�[�u��

#define INITSYM2(sym, str)	sym = NewtMakeSymbol(str)			///< �悭�g���V���{���̏�����
#define INITSYM(name)		INITSYM2(newt_sym.name, #name)		///< �悭�g���V���{���̏������i���ꕶ���Ȃ��j


/* �O���[�o���ϐ� */
newt_env_t	newt_env;		///< ���s��
newt_sym_t	newt_sym;		///< �悭�g���V���{���̕ۊǏꏊ


/* �֐��v���g�^�C�v */
static void		NewtInitSYM(void);
static void		NewtInitSysEnv(void);
static void		NewtInitARGV(int argc, const char * argv[]);
static void		NewtInitVersInfo(void);
static void		NewtInitEnv(int argc, const char * argv[]);


#pragma mark -
/*------------------------------------------------------------------------*/
/** �悭�g���V���{���̏�����
 *
 * @return			�Ȃ�
 */

void NewtInitSYM(void)
{
    // frame
    INITSYM(_proto);
    INITSYM(_parent);

    // function
    INITSYM(_implementor);
    INITSYM(_nextArgFrame);
    INITSYM(CodeBlock);
    INITSYM2(NS_CLASS, "class");
    INITSYM(instructions);
    INITSYM(literals);
    INITSYM(argFrame);
    INITSYM(numArgs);
    INITSYM(indefinite);

    // native function
    INITSYM(_function.native0);
    INITSYM(_function.native);
    INITSYM(funcPtr);
    INITSYM(docString);

    // classes or types
    INITSYM(binary);
    INITSYM(string);
    INITSYM(real);
    INITSYM(array);
    INITSYM(frame);
    INITSYM2(NS_INT, "int");
    INITSYM(int32);
    INITSYM(pathExpr);

    // for loop
    INITSYM(collect);
    INITSYM(deeply);

    // class
    INITSYM2(NS_CHAR, "char");
    INITSYM(boolean);
    INITSYM(weird_immediate);
    INITSYM(forEachState);

    // functions�i�K�{�j
    INITSYM(hasVariable);
    INITSYM(hasVar);
    INITSYM(defGlobalFn);
    INITSYM(defGlobalVar);
    INITSYM(and);
    INITSYM(or);
    INITSYM(mod);
    INITSYM(shiftLeft);
    INITSYM(shiftRight);
    INITSYM(objectEqual);
    INITSYM(defMagicPointer);
	INITSYM(makeRegex);

    // exception type
    INITSYM(type.ref);
    INITSYM(ext.ex.msg);

    // exception frame
    INITSYM(name);
    INITSYM(data);
    INITSYM(message);
    INITSYM(error);

    INITSYM(errorCode);
    INITSYM(symbol);
    INITSYM(value);
    INITSYM(index);

    // root
	INITSYM(sym_table);
    INITSYM(globals);
    INITSYM(global_fns);
    INITSYM(magic_pointers);

    // for print
    INITSYM(printDepth);
    INITSYM(printLength);

	// for regex
    INITSYM(protoREGEX);
    INITSYM(regex);
    INITSYM(pattern);
    INITSYM(option);

	// for require
    INITSYM(requires);

    // ENV
    INITSYM(_ENV_);
    INITSYM(NEWTLIB);

	// ARGV
    INITSYM(_ARGV_);
    INITSYM(_EXEDIR_);

    // stdout, stderr
    INITSYM(_STDOUT_);
    INITSYM(_STDERR_);
}


/*------------------------------------------------------------------------*/
/** �V�X�e�����ϐ��̏�����
 *
 * @return			�Ȃ�
 */

void NewtInitSysEnv(void)
{
	struct {
		char *		name;
		newtRefVar  slot;
		char *		defaultValue;
		bool		ispath;
	} envs[] = {
		{"NEWTLIB",		NSSYM0(NEWTLIB),	NULL,				true},
		{"PLATFORM",	NSSYM(PLATFORM),	__PLATFORM__,		false},
		{"DYLIBSUFFIX",	NSSYM(DYLIBSUFFIX),	__DYLIBSUFFIX__,	false},
		{NULL,			kNewtRefUnbind,		NULL,				false}
	};

	newtRefVar  env;
	newtRefVar  proto;
	newtRefVar  v;
	uint16_t	i;

	env = NcMakeFrame();
	proto = NcMakeFrame();

	for (i = 0; envs[i].name != NULL; i++)
	{
		v = NewtGetEnv(envs[i].name);

		if (NewtRefIsString(v))
		{
			if (envs[i].ispath)
				v = NcSplit(v, NewtMakeCharacter(':'));

			NcSetSlot(proto, envs[i].slot, v);
		}
		else if (envs[i].defaultValue)
		{
			NcSetSlot(proto, envs[i].slot, NewtMakeString(envs[i].defaultValue, true));
		}
	}

	NcSetSlot(env, NSSYM0(_proto), NewtPackLiteral(proto));
    NcSetSlot(GLOBALS, NSSYM0(_ENV_), env);
}


/*------------------------------------------------------------------------*/
/** �R�}���h���C�������̏�����
 *
 * @param argc		[in] �R�}���h���C�������̐�
 * @param argv		[in] �R�}���h���C�������̔z��
 *
 * @return			�Ȃ�
 */
 
void NewtInitARGV(int argc, const char * argv[])
{
	newtRefVar  exepath;
	newtRefVar  r;
	uint16_t	i;

	r = NewtMakeArray(kNewtRefUnbind, argc - 1);
    NcSetSlot(GLOBALS, NSSYM0(_ARGV_), r);

	for (i = 1; i < argc; i++)
	{
		NewtSetArraySlot(r, i - 1, NewtMakeString(argv[i], true));
	}

	exepath = NewtExpandPath(argv[0]);
    NcSetSlot(GLOBALS, NSSYM0(_EXEDIR_), NcDirName(exepath));
}


/*------------------------------------------------------------------------*/
/** �o�[�W�������̏�����
 *
 * @return			�Ȃ�
 */

void NewtInitVersInfo(void)
{
	newtRefVar  versInfo;

	versInfo = NcMakeFrame();

	// �v���_�N�g��
    NcSetSlot(versInfo, NSSYM(name), NewtMakeString(NEWT_NAME, true));
	// �v���g�ԍ�
    NcSetSlot(versInfo, NSSYM(proto), NewtMakeString(NEWT_PROTO, true));
	// �o�[�W�����ԍ�
    NcSetSlot(versInfo, NSSYM(version), NewtMakeString(NEWT_VERSION, true));
	// �r���h�ԍ�
    NcSetSlot(versInfo, NSSYM(build), NewtMakeString(NEWT_BUILD, true));
	// �R�s�[���C�g
    NcSetSlot(versInfo, NSSYM(copyright), NewtMakeString(NEWT_COPYRIGHT, true));
	// �X�^�b�t���[��
    NcSetSlot(versInfo, NSSYM(staff), NewtMakeString(NEWT_STAFF, true));

	// ���[�h�I�����[�ɂ��ăO���[�o���ϐ��ɓ����
    NcSetSlot(GLOBALS, NSSYM(_VERSION_), NewtPackLiteral(versInfo));
}


/*------------------------------------------------------------------------*/
/** ���s���̏�����
 *
 * @param argc		[in] �R�}���h���C�������̐�
 * @param argv		[in] �R�}���h���C�������̔z��
 *
 * @return			�Ȃ�
 */

void NewtInitEnv(int argc, const char * argv[])
{
	// �V���{���e�[�u���̍쐬
    SYM_TABLE = NewtMakeArray(kNewtRefUnbind, 0);
    NewtInitSYM();

	// ���[�g�t���[���̍쐬
    ROOT = NcMakeFrame();
	// �O���[�o���ϐ��e�[�u���̍쐬
    GLOBALS = NcMakeFrame();
	// �O���[�o���֐��e�[�u���̍쐬
    GLOBAL_FNS = NcMakeFrame();
	// �}�W�b�N�|�C���^�e�[�u���̍쐬
#ifdef __NAMED_MAGIC_POINTER__
    MAGIC_POINTERS = NcMakeFrame();
#else
    MAGIC_POINTERS = NewtMakeArray(kNewtRefUnbind, 0);
#endif

	// ���[�g�t���[���Ɋe�e�[�u�����i�[
    NcSetSlot(ROOT, NSSYM0(globals), GLOBALS);
    NcSetSlot(ROOT, NSSYM0(global_fns), GLOBAL_FNS);
    NcSetSlot(ROOT, NSSYM0(magic_pointers), MAGIC_POINTERS);
    NcSetSlot(ROOT, NSSYM0(sym_table), SYM_TABLE);

	// ���ϐ��̏�����
	NewtInitSysEnv();
	// ARGV �̏�����
	NewtInitARGV(argc, argv);
	// �o�[�W�������̏�����
	NewtInitVersInfo();
}


/*------------------------------------------------------------------------*/
/** �C���^�v���^�̏�����
 *
 * @param argc		[in] �R�}���h���C�������̐�
 * @param argv		[in] �R�}���h���C�������̔z��
 *
 * @return			�Ȃ�
 */

void NewtInit(int argc, const char * argv[])
{
	// �������v�[���̊m��
    NEWT_POOL = NewtPoolAlloc(NEWT_POOL_EXPANDSPACE);
	// ���s���̏�����
    NewtInitEnv(argc, argv);
}


/*------------------------------------------------------------------------*/
/** �C���^�v���^�̌�n��
 *
 * @return			�Ȃ�
 */

void NewtCleanup(void)
{
    // ��n�������邱��

	// �������v�[���̉��
    if (NEWT_POOL != NULL)
    {
        NewtPoolRelease(NEWT_POOL);
        NewtMemFree(NEWT_POOL);
        NEWT_POOL = NULL;
    }
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �V���{���e�[�u������V���{������������
 *
 * @param name		[in] �V���{���̖��O
 *
 * @return			�V���{���I�u�W�F�N�g
 */

newtRef NewtLookupSymbolTable(const char * name)
{
    return NewtLookupSymbol(SYM_TABLE, name, 0, 0);
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �O���[�o���֐��̗L���𒲂ׂ�
 *
 * @param r			[in] �V���{���I�u�W�F�N�g
 *
 * @retval			true	�O���[�o���֐������݂���
 * @retval			false   �O���[�o���֐������݂��Ȃ�
 */

bool NewtHasGlobalFn(newtRefArg r)
{
    return NewtHasSlot(GLOBAL_FNS, r);
}


/*------------------------------------------------------------------------*/
/** �O���[�o���ϐ��̗L���𒲂ׂ�
 *
 * @param r			[in] �V���{���I�u�W�F�N�g
 *
 * @retval			true	�O���[�o���ϐ������݂���
 * @retval			false   �O���[�o���ϐ������݂��Ȃ�
 */

bool NewtHasGlobalVar(newtRefArg r)
{
    return NewtHasSlot(GLOBALS, r);
}


#ifdef __USE_OBSOLETE_STYLE__
#pragma mark -
/*------------------------------------------------------------------------*/
/** �O���[�o���֐��̗L���𒲂ׂ�
 *
 * @param r			[in] �V���{���I�u�W�F�N�g
 *
 * @retval			TRUE	�O���[�o���֐������݂���
 * @retval			NIL		�O���[�o���֐������݂��Ȃ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NSHasGlobalFn(newtRefArg r)
{
    return NewtMakeBoolean(NewtHasGlobalFn(r));
}


/*------------------------------------------------------------------------*/
/** �O���[�o���֐��̎擾
 *
 * @param r			[in] �V���{���I�u�W�F�N�g
 *
 * @return			�֐��I�u�W�F�N�g
 */

newtRef NSGetGlobalFn(newtRefArg r)
{
    return NSGetSlot(GLOBAL_FNS, r);
}


/*------------------------------------------------------------------------*/
/** �O���[�o���֐��̒�`
 *
 * @param r			[in] �V���{���I�u�W�F�N�g
 * @param fn		[in] �֐��I�u�W�F�N�g
 *
 * @return			�֐��I�u�W�F�N�g
 */

newtRef NSDefGlobalFn(newtRefArg r, newtRefArg fn)
{
    return NSSetSlot(GLOBAL_FNS, r, fn);
}


/*------------------------------------------------------------------------*/
/** Undefine a global function.
 *
 * @param r			[in] �V���{���I�u�W�F�N�g
 *
 * @return			NIL
 */

newtRef NSUndefGlobalFn(newtRefArg r)
{
    (void) NSRemoveSlot(GLOBAL_FNS, r);
    return kNewtRefNIL;
}


/*------------------------------------------------------------------------*/
/** �O���[�o���ϐ��̗L���𒲂ׂ�
 *
 * @param r			[in] �V���{���I�u�W�F�N�g
 *
 * @retval			TRUE	�O���[�o���ϐ������݂���
 * @retval			NIL		�O���[�o���ϐ������݂��Ȃ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NSHasGlobalVar(newtRefArg r)
{
    return NsGlobalVarExists(kNewtRefNIL, r);
}


/*------------------------------------------------------------------------*/
/** �O���[�o���ϐ��̎擾
 *
 * @param r			[in] �V���{���I�u�W�F�N�g
 *
 * @return			�I�u�W�F�N�g
 */

newtRef NSGetGlobalVar(newtRefArg r)
{
    return NSGetSlot(GLOBALS, r);
}


/*------------------------------------------------------------------------*/
/** �O���[�o���ϐ��ɒl���Z�b�g����
 *
 * @param r			[in] �V���{���I�u�W�F�N�g
 * @param v			[in] �l�I�u�W�F�N�g
 *
 * @return			�I�u�W�F�N�g
 */

newtRef NSSetGlobalVar(newtRefArg r, newtRefArg v)
{
    return NSSetSlot(GLOBALS, r, v);
}


/*------------------------------------------------------------------------*/
/** Undefine a global variable.
 *
 * @param r			[in] �V���{���I�u�W�F�N�g
 *
 * @return			NIL
 */

newtRef NSUndefGlobalVar(newtRefArg r)
{
    (void) NcRemoveSlot(GLOBALS, r);
    return kNewtRefNIL;
}


/*------------------------------------------------------------------------*/
/** �}�W�b�N�|�C���^�̎Q�Ƃ���������
 *
 * @param r			[in] �}�W�b�N�|�C���^
 *
 * @return			�I�u�W�F�N�g
 */

newtRef NSResolveMagicPointer(newtRefArg r)
{
	return NcResolveMagicPointer(r);
}


/*------------------------------------------------------------------------*/
/** �}�W�b�N�|�C���^�̒�`
 *
 * @param r			[in] �}�W�b�N�|�C���^
 * @param fn		[in] �I�u�W�F�N�g
 *
 * @return			�I�u�W�F�N�g
 */

newtRef NSDefMagicPointer(newtRefArg r, newtRefArg v)
{
	return NcDefMagicPointer(r, v);
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** ���[�g�I�u�W�F�N�g�̎擾
 *
 * @return			���[�g�I�u�W�F�N�g
 */

newtRef NSGetRoot(void)
{
    return ROOT;
}


/*------------------------------------------------------------------------*/
/** �O���[�o���ϐ��e�[�u���̎擾
 *
 * @return			�O���[�o���ϐ��e�[�u��
 */

newtRef NSGetGlobals(void)
{
    return GLOBALS;
}


/*------------------------------------------------------------------------*/
/** �O���[�o���֐��e�[�u���̎擾
 *
 * @return			�O���[�o���֐��e�[�u��
 */

newtRef NSGetGlobalFns(void)
{
    return GLOBAL_FNS;
}


/*------------------------------------------------------------------------*/
/** �}�W�b�N�|�C���^�֐��e�[�u���̎擾
 *
 * @return			�}�W�b�N�|�C���^�֐��e�[�u��
 */

newtRef NSGetMagicPointers(void)
{
    return MAGIC_POINTERS;
}


/*------------------------------------------------------------------------*/
/** �V���{���e�[�u���̎擾
 *
 * @return			�V���{���e�[�u��
 */

newtRef NSGetSymTable(void)
{
    return SYM_TABLE;
}

#endif /* __USE_OBSOLETE_STYLE__ */


#pragma mark -
/*------------------------------------------------------------------------*/
/** �O���[�o���֐��̗L���𒲂ׂ�
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �V���{���I�u�W�F�N�g
 *
 * @retval			TRUE	�O���[�o���֐������݂���
 * @retval			NIL		�O���[�o���֐������݂��Ȃ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NsGlobalFnExists(newtRefArg rcvr, newtRefArg r)
{
    return NewtMakeBoolean(NewtHasGlobalFn(r));
}


/*------------------------------------------------------------------------*/
/** �O���[�o���֐��̗L���𒲂ׂ� (OBSOLETE)
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �V���{���I�u�W�F�N�g
 *
 * @retval			TRUE	�O���[�o���֐������݂���
 * @retval			NIL		�O���[�o���֐������݂��Ȃ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NsHasGlobalFn(newtRefArg rcvr, newtRefArg r)
{
    return NewtMakeBoolean(NewtHasGlobalFn(r));
}


/*------------------------------------------------------------------------*/
/** �O���[�o���֐��̎擾
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �V���{���I�u�W�F�N�g
 *
 * @return			�֐��I�u�W�F�N�g
 */

newtRef NsGetGlobalFn(newtRefArg rcvr, newtRefArg r)
{
    return NcGetSlot(GLOBAL_FNS, r);
}


/*------------------------------------------------------------------------*/
/** �O���[�o���֐��̒�`
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �V���{���I�u�W�F�N�g
 * @param fn		[in] �֐��I�u�W�F�N�g
 *
 * @return			�֐��I�u�W�F�N�g
 */

newtRef NsDefGlobalFn(newtRefArg rcvr, newtRefArg r, newtRefArg fn)
{
    return NcSetSlot(GLOBAL_FNS, r, fn);
}


/*------------------------------------------------------------------------*/
/** Undefine a global function.
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �V���{���I�u�W�F�N�g
 *
 * @return			NIL
 */

newtRef NsUndefGlobalFn(newtRefArg rcvr, newtRefArg r)
{
    (void) NcRemoveSlot(GLOBAL_FNS, r);
    return kNewtRefNIL;
}


/*------------------------------------------------------------------------*/
/** �O���[�o���ϐ��̗L���𒲂ׂ�
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �V���{���I�u�W�F�N�g
 *
 * @retval			TRUE	�O���[�o���ϐ������݂���
 * @retval			NIL		�O���[�o���ϐ������݂��Ȃ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NsGlobalVarExists(newtRefArg rcvr, newtRefArg r)
{
    return NewtMakeBoolean(NewtHasGlobalVar(r));
}


/*------------------------------------------------------------------------*/
/** �O���[�o���ϐ��̗L���𒲂ׂ� (OBSOLETE)
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �V���{���I�u�W�F�N�g
 *
 * @retval			TRUE	�O���[�o���ϐ������݂���
 * @retval			NIL		�O���[�o���ϐ������݂��Ȃ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NsHasGlobalVar(newtRefArg rcvr, newtRefArg r)
{
    return NsGlobalVarExists(rcvr, r);
}


/*------------------------------------------------------------------------*/
/** �O���[�o���ϐ��̎擾
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �V���{���I�u�W�F�N�g
 *
 * @return			�I�u�W�F�N�g
 */

newtRef NsGetGlobalVar(newtRefArg rcvr, newtRefArg r)
{
    return NcGetSlot(GLOBALS, r);
}


/*------------------------------------------------------------------------*/
/** �O���[�o���ϐ��ɒl���Z�b�g����
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �V���{���I�u�W�F�N�g
 * @param v			[in] �l�I�u�W�F�N�g
 *
 * @return			�I�u�W�F�N�g
 */

newtRef NsDefGlobalVar(newtRefArg rcvr, newtRefArg r, newtRefArg v)
{
    return NcSetSlot(GLOBALS, r, v);
}


/*------------------------------------------------------------------------*/
/** �O���[�o���ϐ��ɒl���Z�b�g���� (OBSOLETE)
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �V���{���I�u�W�F�N�g
 * @param v			[in] �l�I�u�W�F�N�g
 *
 * @return			�I�u�W�F�N�g
 */

newtRef NsSetGlobalVar(newtRefArg rcvr, newtRefArg r, newtRefArg v)
{
    return NcSetSlot(GLOBALS, r, v);
}


/*------------------------------------------------------------------------*/
/** Undefine a global variable.
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �V���{���I�u�W�F�N�g
 *
 * @return			NIL
 */

newtRef NsUndefGlobalVar(newtRefArg rcvr, newtRefArg r)
{
    (void) NcRemoveSlot(GLOBALS, r);
    return kNewtRefNIL;
}


#ifdef __NAMED_MAGIC_POINTER__

/*------------------------------------------------------------------------*/
/** �}�W�b�N�|�C���^�̎Q�Ƃ���������
 *
 * @param r			[in] �}�W�b�N�|�C���^
 *
 * @return			�I�u�W�F�N�g
 */

newtRef NcResolveMagicPointer(newtRefArg r)
{
	newtRefVar	sym;

	if (! NewtRefIsMagicPointer(r))
		return r;

	sym = NewtMPToSymbol(r);

	if (NewtHasSlot(MAGIC_POINTERS, sym))
		return NcGetSlot(MAGIC_POINTERS, sym);
	else
		return r;
}


/*------------------------------------------------------------------------*/
/** �}�W�b�N�|�C���^�̒�`
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �}�W�b�N�|�C���^
 * @param fn		[in] �I�u�W�F�N�g
 *
 * @return			�I�u�W�F�N�g
 */

newtRef NsDefMagicPointer(newtRefArg rcvr, newtRefArg r, newtRefArg v)
{
	newtRefVar	sym;

	if (NewtRefIsMagicPointer(r))
	{
		sym = NewtMPToSymbol(r);
	}
	else if (NewtRefIsSymbol(r))
	{
		sym = r;
	}
	else
	{
		return r;
	}

    return NcSetSlot(MAGIC_POINTERS, sym, v);
}


#else

/*------------------------------------------------------------------------*/
/** �}�W�b�N�|�C���^�̎Q�Ƃ���������
 *
 * @param r			[in] �}�W�b�N�|�C���^
 *
 * @return			�I�u�W�F�N�g
 */

newtRef NcResolveMagicPointer(newtRefArg r)
{
	int32_t	table = 0;
	int32_t	index;

	if (! NewtRefIsMagicPointer(r))
		return r;

	table = NewtMPToTable(r);
	index = NewtMPToIndex(r);

	if (table != 0)
	{	// �e�[�u���ԍ� 0 �ȊO�͖��T�|�[�g
		return r;
	}

	if (index < NewtLength(MAGIC_POINTERS))
	{
		newtRefVar	result;

		result = NewtGetArraySlot(MAGIC_POINTERS, index);

		if (result != kNewtRefUnbind)
			return result;
	}

	return r;
}


/*------------------------------------------------------------------------*/
/** �}�W�b�N�|�C���^�̒�`
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �}�W�b�N�|�C���^
 * @param fn		[in] �I�u�W�F�N�g
 *
 * @return			�I�u�W�F�N�g
 */

newtRef NsDefMagicPointer(newtRefArg rcvr, newtRefArg r, newtRefArg v)
{
	int32_t	table = 0;
	int32_t	index;

	if (NewtRefIsMagicPointer(r))
	{
		table = NewtMPToTable(r);
		index = NewtMPToIndex(r);

		if (table != 0)
		{	// �e�[�u���ԍ� 0 �ȊO�͖��T�|�[�g
			return kNewtRefUnbind;
		}
	}
	else if (NewtRefIsInteger(r))
	{
		index = NewtRefToInteger(r);
	}
	else
	{
		return kNewtRefUnbind;
	}

	if (NewtLength(MAGIC_POINTERS) <= index)
	{	// �e�[�u���̒������g��
		NewtSetLength(MAGIC_POINTERS, index + 1);
	}

	return NewtSetArraySlot(MAGIC_POINTERS, index, v);
}

#endif


#pragma mark -
/*------------------------------------------------------------------------*/
/** ���[�g�I�u�W�F�N�g�̎擾
 *
 * @param rcvr		[in] ���V�[�o
 *
 * @return			���[�g�I�u�W�F�N�g
 */

newtRef NsGetRoot(newtRefArg rcvr)
{
    return ROOT;
}


/*------------------------------------------------------------------------*/
/** �O���[�o���ϐ��e�[�u���̎擾
 *
 * @param rcvr		[in] ���V�[�o
 *
 * @return			�O���[�o���ϐ��e�[�u��
 */

newtRef NsGetGlobals(newtRefArg rcvr)
{
    return GLOBALS;
}


/*------------------------------------------------------------------------*/
/** �O���[�o���֐��e�[�u���̎擾
 *
 * @param rcvr		[in] ���V�[�o
 *
 * @return			�O���[�o���֐��e�[�u��
 */

newtRef NsGetGlobalFns(newtRefArg rcvr)
{
    return GLOBAL_FNS;
}


/*------------------------------------------------------------------------*/
/** �}�W�b�N�|�C���^�֐��e�[�u���̎擾
 *
 * @param rcvr		[in] ���V�[�o
 *
 * @return			�}�W�b�N�|�C���^�֐��e�[�u��
 */

newtRef NsGetMagicPointers(newtRefArg rcvr)
{
    return MAGIC_POINTERS;
}


/*------------------------------------------------------------------------*/
/** �V���{���e�[�u���̎擾
 *
 * @param rcvr		[in] ���V�[�o
 *
 * @return			�V���{���e�[�u��
 */

newtRef NsGetSymTable(newtRefArg rcvr)
{
    return SYM_TABLE;
}
