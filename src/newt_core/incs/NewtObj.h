/*------------------------------------------------------------------------*/
/**
 * @file	NewtObj.h
 * @brief   �I�u�W�F�N�g�V�X�e��
 *
 * @author  M.Nukui
 * @date	2003-11-07
 *
 * Copyright (C) 2003-2004 M.Nukui All rights reserved.
 */


#ifndef	NEWTOBJ_H
#define	NEWTOBJ_H


/* �w�b�_�t�@�C�� */
#include "NewtType.h"


/* �}�N�� */
/// addr <--> integer�@���̃V�t�g
//#define NOBJ_ADDR_SHIFT		2
#define NOBJ_ADDR_SHIFT		0


#define NewtRefIsInt30(r)			((r & 3) == 0)						///< 30bit�����I�u�W�F�N�g���H
#define	NewtRefToInt30(r)			(int32_t)((int32_t)r >> 2)			///< �I�u�W�F�N�g�� 30bit�����ɕϊ�
#define	NewtMakeInt30(v)			(newtRef)((int32_t)(v) << 2)		///< 30bit�����I�u�W�F�N�g���쐬

#define	NewtRefIsPointer(r)			((r & 3) == 1)						///< �|�C���^�I�u�W�F�N�g���H
#define	NewtRefToPointer(r)			(newtObjRef)((uint32_t)r - 1)		///< �I�u�W�F�N�g�Q�Ƃ��|�C���^�ɕϊ�
#define	NewtMakePointer(v)			(newtRef)((uint32_t)(v) + 1)		///< �|�C���^�I�u�W�F�N�g���쐬

#define	NewtRefIsCharacter(r)		((r & 0xF) == 6)					///< �����I�u�W�F�N�g���H
#define	NewtRefToCharacter(r)		(int)(((uint32_t)r >> 4) & 0xFFFF)	///< �I�u�W�F�N�g�Q�Ƃ𕶎��ɕϊ�
#define	NewtMakeCharacter(v)		(newtRef)(((uint32_t)(v) << 4) | 6)	///< �����I�u�W�F�N�g���쐬

#define	NewtRefIsSpecial(r)			((r & 0xF) == 2)					///< ����I�u�W�F�N�g���H
#define	NewtRefToSpecial(r)			(int32_t)((uint32_t)r >> 2)			///< �I�u�W�F�N�g�Q�Ƃ����l�ɕϊ�

#define NewtRefIsMagicPointer(r)	((r & 3) == 3)						///< �}�W�b�N�|�C���^���H

#ifdef __NAMED_MAGIC_POINTER__
	#define NewtMakeNamedMP(r)			((newtRef)((uint32_t)NewtMakeSymbol(r) | 3))		///< ���O�t�}�W�b�N�|�C���^���쐬
	#define NewtMPToSymbol(r)			((newtRef)((uint32_t)r & 0xFFFFFFFD))				///< ���O�t�}�W�b�N�|�C���^���V���{���ɕϊ�
	#define NewtSymbolToMP(r)			((newtRef)((uint32_t)r | 3))						///< �V���{���𖼑O�t�}�W�b�N�|�C���^�ɕϊ�
#else
	#define NewtMakeMagicPointer(t, i)	((newtRef)((t << 14) | ((i & 0x03ff) << 2) | 3))	///< �}�W�b�N�|�C���^���쐬
	#define	NewtMPToTable(r)			((int32_t)((uint32_t)r >> 14))						///< �}�W�b�N�|�C���^�̃e�[�u���ԍ����擾
	#define	NewtMPToIndex(r)			((int32_t)(((uint32_t)r >> 2) & 0x03ff))			///< �}�W�b�N�|�C���^�̃C���f�b�N�X���擾
#endif

#define	NewtRefIsNotNIL(v)			(! NewtRefIsNIL(v))								///< NIL �ȊO���H
#define	NewtMakeBoolean(v)			((newtRef)((v)?(kNewtRefTRUE):(kNewtRefNIL)))	///< �u�[���l�I�u�W�F�N�g���쐬

#define	NewtRefToBinary(r)			((uint8_t *)NewtRefToData(r))		///< �o�C�i���f�[�^�ւ̃|�C���^���擾
#define	NewtRefToSymbol(r)			((newtSymDataRef)NewtRefToData(r))	///< �V���{���f�[�^�ւ̃|�C���^���擾
#define	NewtRefToString(r)			((char *)NewtRefToData(r))			///< ������f�[�^�ւ̃|�C���^���擾
#define	NewtRefToSlots(r)			((newtRef *)NewtRefToData(r))		///< �X���b�g�f�[�^�ւ̃|�C���^���擾

//
#define	NewtArrayLength(r)			NewtSlotsLength(r)					///< �z��̒������擾
#define	NewtFrameLength(r)			NewtSlotsLength(r)					///< �t���[���̒������擾

//
#define	NewtObjType(v)				(v->header.h & 3)						///< �I�u�W�F�N�g�^�C�v���擾
#define	NewtObjIsSlotted(v)			((v->header.h & kNewtObjSlotted) != 0)  ///< �I�u�W�F�N�g�f�[�^���X���b�g���H
#define	NewtObjIsArray(v)			(NewtObjType(v) == 1)					///< �I�u�W�F�N�g�f�[�^���z�񂩁H
#define	NewtObjIsFrame(v)			(NewtObjType(v) == 3)					///< �I�u�W�F�N�g�f�[�^���t���[�����H
#define NewtObjIsLiteral(v)			((v->header.h & kNewtObjLiteral) == kNewtObjLiteral)		///< ���e�������H
#define NewtObjIsSweep(v, mark)		(((v->header.h & kNewtObjSweep) == kNewtObjSweep) == mark)  ///< �X�E�B�[�v�Ώۂ��H
#define	NewtObjSize(v)				(v->header.h >> 8)					///< �I�u�W�F�N�g�f�[�^�̃T�C�Y���擾
#define NewtObjBinaryClass(v)		(v->as.klass)						///< Low-level API. Use NewtObjClassOf when needed.
#define NewtObjArrayClass(v)		(v->as.klass)						///< Low-level API. Use NewtObjClassOf when needed.
#define	NewtObjToBinary(v)			((uint8_t *)NewtObjData(v))			///< �o�C�i���f�[�^���ւ̃|�C���^
#define	NewtObjToSymbol(v)			((newtSymDataRef)NewtObjData(v))	///< �V���{���f�[�^���ւ̃|�C���^
#define	NewtObjToString(v)			((char *)NewtObjData(v))			///< ������f�[�^���ւ̃|�C���^
#define	NewtObjToSlots(v)			((newtRef *)NewtObjData(v))			///< �X���b�g�f�[�^���ւ̃|�C���^

//
#define NewtHasVar(name)			NVMHasVar(name)						///< �ϐ��̑��݃`�F�b�N
#define NewtObjIsReadonly(obj)		NewtObjIsLiteral(obj)				///< �I�u�W�F�N�g�f�[�^�����[�h�I�����[���H
#define NewtRefIsReadonly(r)		NewtRefIsLiteral(r)					///< �I�u�W�F�N�g�����[�h�I�����[���H

#ifdef __USE_OBSOLETE_STYLE__
// old style
#define NewtMakeNativeFn(funcPtr, numArgs, doc)			NewtMakeNativeFn0(funcPtr, numArgs, false, doc)	
#define NewtDefGlobalFn(sym, funcPtr, numArgs, doc)		NewtDefGlobalFn0(sym, funcPtr, numArgs, false, doc)	

#endif /* __USE_OBSOLETE_STYLE__ */

// new style
#define NewtMakeNativeFunc(funcPtr, numArgs, doc)		NewtMakeNativeFunc0(funcPtr, numArgs, false, doc)	
#define NewtDefGlobalFunc(sym, funcPtr, numArgs, doc)	NewtDefGlobalFunc0(sym, funcPtr, numArgs, false, doc)	


/* �萔 */

enum {
	kNewtNotFunction			= 0,
	kNewtCodeBlock,						// �o�C�g�R�[�h�֐�
	kNewtNativeFn,						// �l�C�e�B�u�֐��ircvr�Ȃ��Aold style�j
	kNewtNativeFunc						// �l�C�e�B�u�֐��ircvr����Anew style�j
};


/* �^�錾 */

/// �g�����C�u�����̃C���X�g�[���p�G���g���֐�
typedef void(*newt_install_t)(void);


/* �֐��v���g�^�C�v */

#ifdef __cplusplus
extern "C" {
#endif


uint32_t	NewtSymbolHashFunction(const char * name);
newtRef		NewtLookupSymbol(newtRefArg r, const char * name, uint32_t hash, int32_t st);
newtRef		NewtLookupSymbolArray(newtRefArg r, newtRefArg name, int32_t st);
const char*	NewtSymbolGetName(newtRefArg inSymbol);

uint16_t	NewtGetRefType(newtRefArg r, bool detail);
uint16_t	NewtGetObjectType(newtObjRef obj, bool detail);

uint32_t	NewtObjCalcDataSize(uint32_t n);
newtObjRef	NewtObjAlloc(newtRefArg r, uint32_t n, uint16_t type, bool literal);
newtObjRef	NewtObjResize(newtObjRef r, uint32_t n);
void *		NewtObjData(newtObjRef obj);
newtRef		NewtObjClone(newtRefArg r);
newtRef		NewtPackLiteral(newtRefArg r);

bool		NewtRefIsLiteral(newtRefArg r);
bool		NewtRefIsSweep(newtRefArg r, bool mark);
bool		NewtRefIsNIL(newtRefArg r);
bool		NewtRefIsSymbol(newtRefArg r);
uint32_t	NewtRefToHash(newtRefArg r);
bool		NewtRefIsString(newtRefArg r);
bool		NewtRefIsInteger(newtRefArg r);
int32_t		NewtRefToInteger(newtRefArg r);
bool		NewtRefIsInt32(newtRefArg r);
bool		NewtRefIsReal(newtRefArg r);
double		NewtRefToReal(newtRefArg r);
bool		NewtRefIsBinary(newtRefArg r);
void *		NewtRefToData(newtRefArg r);
bool		NewtRefIsArray(newtRefArg r);
bool		NewtRefIsFrame(newtRefArg r);
bool		NewtRefIsFrameOrArray(newtRefArg r);
bool		NewtRefIsImmediate(newtRefArg r);
bool		NewtRefIsCodeBlock(newtRefArg r);
bool		NewtRefIsNativeFn(newtRefArg r);
bool		NewtRefIsNativeFunc(newtRefArg r);
bool		NewtRefIsFunction(newtRefArg r);
int			NewtRefFunctionType(newtRefArg r);
bool		NewtRefIsRegex(newtRefArg r);
void *		NewtRefToAddress(newtRefArg r);

newtRef		NewtMakeBinary(newtRefArg klass, uint8_t * data, uint32_t size, bool literal);
newtRef		NewtMakeSymbol(const char *s);
newtRef		NewtMakeString(const char *s, bool literal);
newtRef		NewtMakeString2(const char *s, uint32_t len, bool literal);
newtRef		NewtBinarySetLength(newtRefArg r, uint32_t n);
newtRef		NewtStringSetLength(newtRefArg r, uint32_t n);
newtRef		NewtMakeInteger(int32_t v);
newtRef		NewtMakeInt32(int32_t v);
newtRef		NewtMakeReal(double v);
newtRef		NewtMakeArray(newtRefArg klass, uint32_t n);
newtRef		NewtMakeArray2(newtRefArg klass, uint32_t n, newtRefVar v[]);
newtRef		NewtMakeMap(newtRefArg superMap, uint32_t n, newtRefVar v[]);
void		NewtSetMapFlags(newtRefArg map, int32_t bit);
void		NewtClearMapFlags(newtRefArg map, int32_t bit);
uint32_t	NewtMapLength(newtRefArg map);
newtRef		NewtMakeFrame(newtRefArg map, uint32_t n);
newtRef		NewtMakeFrame2(uint32_t n, newtRefVar v[]);
newtRef		NewtMakeSlotsObj(newtRefArg r, uint32_t n, uint16_t type);
uint32_t	NewtObjSlotsLength(newtObjRef obj);
newtRef		NewtObjAddArraySlot(newtObjRef obj, newtRefArg v);
newtRef		NewtSlotsSetLength(newtRefArg r, uint32_t n, newtRefArg v);
newtRef		NewtSetLength(newtRefArg r, uint32_t n);
newtRef		NewtMakeAddress(void * addr);

newtRef		NewtThrow0(int32_t err); 
newtRef		NewtThrow(int32_t err, newtRefArg value); 
newtRef		NewtThrowSymbol(int32_t err, newtRefArg symbol); 
newtRef		NewtErrOutOfBounds(newtRefArg value, int32_t index);
void		NewtErrMessage(int32_t err);

int			NewtSymbolCompareLex(newtRefArg r1, newtRefArg r2);
int16_t		NewtObjectCompare(newtRefArg r1, newtRefArg r2);
bool		NewtRefEqual(newtRefArg r1, newtRefArg r2);
bool		NewtObjectEqual(newtRefArg r1, newtRefArg r2);
bool		NewtSymbolEqual(newtRefArg r1, newtRefArg r2);

uint32_t	NewtLength(newtRefArg r);
uint32_t	NewtDeeplyLength(newtRefArg r);
uint32_t	NewtBinaryLength(newtRefArg r);
uint32_t	NewtSymbolLength(newtRefArg r);
uint32_t	NewtStringLength(newtRefArg r);
uint32_t	NewtSlotsLength(newtRefArg r);
uint32_t	NewtDeeplyFrameLength(newtRefArg r);

newtRef		NewtObjGetSlot(newtObjRef obj, newtRefArg slot);
newtRef		NewtObjSetSlot(newtObjRef obj, newtRefArg slot, newtRefArg v);
void		NewtObjRemoveSlot(newtObjRef obj, newtRefArg slot);

newtRef		NewtGetMapIndex(newtRefArg r, uint32_t index, uint32_t * indexP);
int32_t		NewtFindArrayIndex(newtRefArg r, newtRefArg v, uint16_t st);
bool		NewtFindMapIndex(newtRefArg r, newtRefArg v, uint32_t * indexP);
newtRef		NewtFrameMap(newtRefArg r);

int32_t		NewtFindSlotIndex(newtRefArg frame, newtRefArg slot);
bool		NewtHasProto(newtRefArg frame);
bool		NewtHasSlot(newtRefArg frame, newtRefArg slot);
newtRef		NewtSlotsGetPath(newtRefArg r, newtRefArg p);
bool		NewtHasPath(newtRefArg r, newtRefArg p);
newtRef		NewtGetPath(newtRefArg r, newtRefArg p, newtRefVar * slotP);
newtRef		NewtGetBinarySlot(newtRefArg r, uint32_t p);
newtRef		NewtSetBinarySlot(newtRefArg r, uint32_t p, newtRefArg v);
newtRef		NewtGetStringSlot(newtRefArg r, uint32_t p);
newtRef		NewtSetStringSlot(newtRefArg r, uint32_t p, newtRefArg v);
newtRef		NewtSlotsGetSlot(newtRefArg r, uint32_t p);
newtRef		NewtSlotsSetSlot(newtRefArg r, uint32_t p, newtRefArg v);
newtRef		NewtSlotsInsertSlot(newtRefArg r, uint32_t p, newtRefArg v);
newtRef		NewtGetArraySlot(newtRefArg r, uint32_t p);
newtRef		NewtSetArraySlot(newtRefArg r, uint32_t p, newtRefArg v);
newtRef		NewtInsertArraySlot(newtRefArg r, uint32_t p, newtRefArg v);
newtRef		NewtGetFrameSlot(newtRefArg r, uint32_t p);
newtRef		NewtSetFrameSlot(newtRefArg r, uint32_t p, newtRefArg v);
newtRef		NewtGetFrameKey(newtRefArg inFrame, uint32_t inIndex);

newtRef		NewtARef(newtRefArg r, uint32_t p);
newtRef		NewtSetARef(newtRefArg r, uint32_t p, newtRefArg v);

bool		NewtAssignment(newtRefArg start, newtRefArg name, newtRefArg value);
bool		NewtLexicalAssignment(newtRefArg start, newtRefArg name, newtRefArg value);
bool		NewtHasLexical(newtRefArg start, newtRefArg name);
bool		NewtHasVariable(newtRefArg r, newtRefArg name);

void *		NewtRefToNativeFn(newtRefArg r);
// old style
newtRef		NewtMakeNativeFn0(void * funcPtr, uint32_t numArgs, bool indefinite, char * doc);
newtRef		NewtDefGlobalFn0(newtRefArg sym, void * funcPtr, uint32_t numArgs, bool indefinite, char * doc);
// new style
newtRef		NewtMakeNativeFunc0(void * funcPtr, uint32_t numArgs, bool indefinite, char * doc);
newtRef		NewtDefGlobalFunc0(newtRefArg sym, void * funcPtr, uint32_t numArgs, bool indefinite, char * doc);

bool		NewtHasSubclass(newtRefArg sub, newtRefArg supr);
bool		NewtIsSubclass(newtRefArg sub, newtRefArg supr);
bool		NewtIsInstance(newtRefArg obj, newtRefArg r);

newtRef		NewtStrCat(newtRefArg r, char * s);
newtRef		NewtStrCat2(newtRefArg r, char * s, uint32_t slen);

newtRef		NewtGetEnv(const char * s);


#ifdef __cplusplus
}
#endif


#endif /* NEWTOBJ_H */

