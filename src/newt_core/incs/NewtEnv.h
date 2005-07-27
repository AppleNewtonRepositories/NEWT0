/*------------------------------------------------------------------------*/
/**
 * @file	NewtEnv.h
 * @brief   ���s��
 *
 * @author  M.Nukui
 * @date	2003-11-07
 *
 * Copyright (C) 2003-2004 M.Nukui All rights reserved.
 */


#ifndef	NEWTENV_H
#define	NEWTENV_H


/* �w�b�_�t�@�C�� */
#include "NewtType.h"
#include "NewtMem.h"


/* �}�N�� */
#define NEWT_DEBUG			(newt_env._debug)				///< �f�o�b�O�t���O
#define NEWT_TRACE			(newt_env._trace)				///< �g���[�X�t���O
#define NEWT_DUMPLEX		(newt_env._dumpLex)				///< �_���v�����̓t���O
#define NEWT_DUMPSYNTAX		(newt_env._dumpSyntax)			///< �_���v�\���؃t���O
#define NEWT_DUMPBC			(newt_env._dumpBC)				///< �_���v�o�C�g�R�[�h�t���O
#define NEWT_POOL			(newt_env.pool)					///< �������v�[��
#define NEWT_SWEEP			(newt_env.sweep)				///< SWEEP�t���O
#define NEWT_NEEDGC			(newt_env.needgc)				///< GC�t���O
#define NEWT_MODE_NOS2		(newt_env.mode.nos2)			///< NOS2 �R���p�`�u��

#define NSSTR(s)			(NewtMakeString(s, false))		///< ������I�u�W�F�N�g�̍쐬
#define NSSTRCONST(s)		(NewtMakeString(s, true))		///< ������萔�I�u�W�F�N�g�̍쐬
#define NSINT(n)			(NewtMakeInteger(n))			///< �����I�u�W�F�N�g�̍쐬
#define NSREAL(n)			(NewtMakeReal(n))				///< ���������_�I�u�W�F�N�g�̍쐬

#define NSSYM0(name)		newt_sym.name					///< �ۊǏꏊ����V���{���I�u�W�F�N�g���擾
#define NSSYM(name)			(NewtMakeSymbol(#name))			///< �V���{���I�u�W�F�N�g�̍쐬

#define NSNAMEDMP(name)		(NewtMakeNamedMP(#name))		///< ���O�t�}�W�b�N�|�C���^�̍쐬
#define NSNAMEDMP0(name)	(NewtSymbolToMP(NSSYM0(name)))	///< �ۊǏꏊ���疼�O�t�}�W�b�N�|�C���^�̍쐬
#define NSMP(n)				(NewtMakeMagicPointer(0, n))	///< �}�W�b�N�|�C���^�̍쐬

#define NS_CLASS			NSSYM0(__class)					///< class �V���{��
#define NS_INT				NSSYM0(__int)					///< int �V���{��
#define NS_CHAR				NSSYM0(__char)					///< char �V���{��

#define NcGlobalFnExists(r)				NsGlobalFnExists(kNewtRefNIL, r)
#define NcGetGlobalFn(r)				NsGetGlobalFn(kNewtRefNIL, r)
#define NcDefGlobalFn(r, fn)			NsDefGlobalFn(kNewtRefNIL, r, fn)
#define NcUndefGlobalFn(r)				NsUndefGlobalFn(kNewtRefNIL, r)
#define NcGlobalVarExists(r)			NsGlobalVarExists(kNewtRefNIL, r)
#define NcGetGlobalVar(r)				NsGetGlobalVar(kNewtRefNIL, r)
#define NcDefGlobalVar(r, v)			NsDefGlobalVar(kNewtRefNIL, r, v)
#define NcUndefGlobalVar(r)				NsUndefGlobalVar(kNewtRefNIL, r)
#define NcDefMagicPointer(r, v)			NsDefMagicPointer(kNewtRefNIL, r, v)
#define NcGetRoot()						NsGetRoot(kNewtRefNIL)
#define NcGetGlobals()					NsGetGlobals(kNewtRefNIL)
#define NcGetGlobalFns()				NsGetGlobalFns(kNewtRefNIL)
#define NcGetMagicPointers()			NsGetMagicPointers(kNewtRefNIL)
#define NcGetSymTable()					NsGetSymTable(kNewtRefNIL)

// OBSOLETE
#define NcHasGlobalFn(r)				NsGlobalFnExists(kNewtRefNIL, r)
#define NcHasGlobalVar(r)				NsGlobalVarExists(kNewtRefNIL, r)
#define NcSetGlobalVar(r, v)			NsDefGlobalVar(kNewtRefNIL, r, v)


/// ���s��
typedef struct {
    newtRefVar	sym_table;		///< �V���{���e�[�u��
    newtRefVar	root;			///< ���[�g
    newtRefVar	globals;		///< �O���[�o���ϐ��e�[�u��
    newtRefVar	global_fns;		///< �O���[�o���֐��e�[�u��
    newtRefVar	magic_pointers;	///< �}�W�b�N�|�C���^�e�[�u��

	// �������֌W
    newtPool	pool;			///< �������v�[��
    bool		sweep;			///< ���݂� sweep ��ԁi�g�O������j
    bool		needgc;			///< GC ���K�v

	/// ���[�h
	struct {
		bool	nos2;			///< NOS2 �R���p�`�u��
	} mode;

    // �f�o�b�O
    bool		_debug;			///< �f�o�b�O�t���O
    bool		_trace;			///< �g���[�X�t���O
    bool		_dumpLex;		///< �����̓_���v�t���O
    bool		_dumpSyntax;	///< �\���؃_���v�t���O
    bool		_dumpBC;		///< �o�C�g�R�[�h�_���v�t���O
} newt_env_t;


/// �悭�g���V���{��
typedef struct {
    // frame
    newtRefVar	_proto;				///< _proto
    newtRefVar	_parent;			///< _parent

    // function
    newtRefVar	_implementor;		///< _implementor
    newtRefVar	_nextArgFrame;		///< _nextArgFrame
    newtRefVar	CodeBlock;			///< CodeBlock
    newtRefVar	__class;			///< class
    newtRefVar	instructions;		///< instructions
    newtRefVar	literals;			///< literals
    newtRefVar	argFrame;			///< argFrame
    newtRefVar	numArgs;			///< numArgs
    newtRefVar	indefinite;			///< indefinite

    // native function

	/// function
    struct {
        newtRefVar	native0;		///< function.native0 �ircvr�Ȃ��j
        newtRefVar	native;			///< function.native �ircvr����j
    } _function;

    newtRefVar	funcPtr;			///< funcPtr
    newtRefVar	docString;			///< docString

    // classes or types
    newtRefVar	binary;				///< binary
    newtRefVar	string;				///< string
    newtRefVar	real;				///< real
    newtRefVar	array;				///< array
    newtRefVar	frame;				///< frame
    newtRefVar	__int;				///< int
    newtRefVar	int32;				///< int32
    newtRefVar	pathExpr;			///< pathExpr

    // for loop
    newtRefVar	collect;			///< collect
    newtRefVar	deeply;				///< deeply

    // class	
    newtRefVar	__char;				///< char
    newtRefVar	boolean;			///< boolean
    newtRefVar	weird_immediate;	///< weird_immediate
    newtRefVar	forEachState;		///< forEachState

    // functions�i�K�{�j
    newtRefVar	hasVariable;		///< hasVariable
    newtRefVar	hasVar;				///< hasVar
    newtRefVar	defGlobalFn;		///< defGlobalFn
    newtRefVar	defGlobalVar;		///< defGlobalVar
//    newtRefVar	and;				///< and
//    newtRefVar	or;					///< or
    newtRefVar	mod;				///< mod
    newtRefVar	shiftLeft;			///< <<
    newtRefVar	shiftRight;			///< >>
    newtRefVar	objectEqual;		///< ==
    newtRefVar	defMagicPointer;	///< @0 := value
    newtRefVar	makeRegex;			///< makeRegex

    // exception frame

	/// type
    struct {
        newtRefVar	ref;			///< type.ref
    } type;

	/// ext
    struct {
		/// ext.ex
        struct {
            newtRefVar	msg;		///< ext.ex.msg
        } ex;
    } ext;

    newtRefVar	name;				///< name
    newtRefVar	data;				///< data
    newtRefVar	message;			///< message
    newtRefVar	error;				///< error

    newtRefVar	errorCode;			///< errorCode
    newtRefVar	symbol;				///< symbol
    newtRefVar	value;				///< value
    newtRefVar	index;				///< index

    // root
    newtRefVar	sym_table;			///< sym_table
    newtRefVar	globals;			///< globals
    newtRefVar	global_fns;			///< global_fns
    newtRefVar	magic_pointers;		///< magic_pointers

    // for print
    newtRefVar	printDepth;			///< printDepth
    newtRefVar	printLength;		///< printLength

    // for regex
    newtRefVar	protoREGEX;			///< @protoREGEX
    newtRefVar	regex;				///< regex
    newtRefVar	pattern;			///< pattern
    newtRefVar	option;				///< option

	// for require
    newtRefVar	requires;			///< requires

	// ENV
    newtRefVar	_ENV_;				///< _ENV_
    newtRefVar	NEWTLIB;			///< NEWTLIB

	// ARGV
    newtRefVar	_ARGV_;				///< _ARGV_
	newtRefVar  _EXEDIR_;			///< _EXEDIR_

	// stdout, stderr
	newtRefVar  _STDOUT_;			///< _STDOUT_
	newtRefVar  _STDERR_;			///< _STDERR_
} newt_sym_t;


/* �O���[�o���ϐ� */
extern newt_env_t	newt_env;		///< ���s��
extern newt_sym_t	newt_sym;		///< �悭�g���V���{���̕ۊǏꏊ


/* �֐��v���g�^�C�v */

#ifdef __cplusplus
extern "C" {
#endif


char *		NewtDefaultEncoding(void);
void		NewtInit(int argc, const char * argv[], int n);
void		NewtCleanup(void);

newtRef		NewtLookupSymbolTable(const char * name);

bool		NewtHasGlobalFn(newtRefArg r);
bool		NewtHasGlobalVar(newtRefArg r);

// NewtonScript native functions(new style)
newtRef		NsGlobalFnExists(newtRefArg rcvr, newtRefArg r);
newtRef		NsGetGlobalFn(newtRefArg rcvr, newtRefArg r);
newtRef		NsDefGlobalFn(newtRefArg rcvr, newtRefArg r, newtRefArg fn);
newtRef		NsUndefGlobalFn(newtRefArg rcvr, newtRefArg r);
newtRef		NsGlobalVarExists(newtRefArg rcvr, newtRefArg r);
newtRef		NsGetGlobalVar(newtRefArg rcvr, newtRefArg r);
newtRef		NsDefGlobalVar(newtRefArg rcvr, newtRefArg r, newtRefArg v);
newtRef		NsUndefGlobalVar(newtRefArg rcvr, newtRefArg r);
newtRef		NcResolveMagicPointer(newtRefArg r);
newtRef		NsDefMagicPointer(newtRefArg rcvr, newtRefArg r, newtRefArg v);

newtRef		NsGetRoot(newtRefArg rcvr);
newtRef		NsGetGlobals(newtRefArg rcvr);
newtRef		NsGetGlobalFns(newtRefArg rcvr);
newtRef		NsGetMagicPointers(newtRefArg rcvr);
newtRef		NsGetSymTable(newtRefArg rcvr);

#ifdef __USE_OBSOLETE_STYLE__
newtRef		NsHasGlobalFn(newtRefArg rcvr, newtRefArg r);					// OBSOLETE
newtRef		NsHasGlobalVar(newtRefArg rcvr, newtRefArg r);					// OBSOLETE
newtRef		NsSetGlobalVar(newtRefArg rcvr, newtRefArg r, newtRefArg v);	// OBSOLETE
#endif /* __USE_OBSOLETE_STYLE__ */


#ifdef __cplusplus
}
#endif


#endif /* NEWTENV_H */

