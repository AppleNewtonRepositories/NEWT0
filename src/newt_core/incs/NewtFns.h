/*------------------------------------------------------------------------*/
/**
 * @file	NewtFns.h
 * @brief   �g���݊֐�
 *
 * @author  M.Nukui
 * @date	2003-11-07
 *
 * @note	NS...�@�Ŏn�܂�֐��̓C���^�v���^�̊֐��Ƃ��Ē�`�\
 *
 * Copyright (C) 2003-2004 M.Nukui All rights reserved.
 */


#ifndef	NEWTFNS_H
#define	NEWTFNS_H

/*

�֐��l�[�~���O���[��

  Ns******	NewtonScript �l�C�e�B�u�R�[�h�i�������� rcvr ����A�X�N���v�g����g�p�j
  Nc******	NewtonScript �l�C�e�B�u�R�[�h�i�������� rcvr �Ȃ��AC���ꂩ��g�p�j
  NVM*****	VM�֘A
  NPS*****	�p�[�T�[�֘A
  Newt*****	�I�u�W�F�N�g�֘A�A���̑�

�g��Ȃ��悤�ɁF(OBSOLETE)
  NS******	Cocoa APIs

*/


/* �w�b�_�t�@�C�� */
#include "NewtType.h"


/* �}�N�� */
#define NcSelf()					NVMSelf()						///< self ���擾
#define NcGetVariable(frame, slot)	NcFullLookup(frame, slot)

#define NcThrow(name, data)			NsThrow(kNewtRefNIL, name, data)
#define NcTotalClone(r)				NsTotalClone(kNewtRefNIL, r)
#define NcDeeplyLength(r)			NsDeeplyLength(kNewtRefNIL, r)
#define NcHasSlot(frame, slot)		NsHasSlot(kNewtRefNIL, frame, slot)
#define NcGetSlot(frame, slot)		NsGetSlot(kNewtRefNIL, frame, slot)
#define NcSetSlot(frame, slot, v)	NsSetSlot(kNewtRefNIL, frame, slot, v)
#define NcRemoveSlot(frame, slot)	NsRemoveSlot(kNewtRefNIL, frame, slot)
#define NcStrCat(str, v)			NsStrCat(kNewtRefNIL, str, v)
#define NcMakeSymbol(r)				NsMakeSymbol(kNewtRefNIL, r)
#define NcMakeFrame()				NsMakeFrame(kNewtRefNIL)
#define NcMakeBinary(len, klass)	NsMakeBinary(kNewtRefNIL, len, klass)
#define NcPrintObject(r)			NsPrintObject(kNewtRefNIL, r)
#define NcPrint(r)					NsPrint(kNewtRefNIL, r)


#ifdef __USE_OBSOLETE_STYLE__

#define NSSelf()					NVMSelf()						///< self ���擾
#define NSCurrentException			NVMCurrentException				///< ���݂̗�O�G���[

#endif /* __USE_OBSOLETE_STYLE__ */



/* �֐��v���g�^�C�v */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __USE_OBSOLETE_STYLE__
// NewtonScript native functions(old style)
newtRef		NSProtoLookup(newtRefArg start, newtRefArg name);
newtRef		NSLexicalLookup(newtRefArg start, newtRef name);
newtRef		NSFullLookup(newtRefArg start, newtRefArg name);

newtRef		NSLookupSymbol(newtRefArg r, newtRefArg name);

newtRef		NSThrow(newtRefArg name, newtRefArg data);
newtRef		NSRethrow(void);

newtRef		NSClone(newtRefArg r);
newtRef		NSTotalClone(newtRefArg r);
newtRef		NSLength(newtRefArg r);
newtRef		NSDeeplyLength(newtRefArg r);
newtRef		NSHasSlot(newtRefArg frame, newtRefArg slot);
newtRef		NSGetSlot(newtRefArg frame, newtRefArg slot);
newtRef		NSSetSlot(newtRefArg frame, newtRefArg slot, newtRefArg v);
newtRef		NSRemoveSlot(newtRefArg frame, newtRefArg slot);
newtRef		NSSetArraySlot(newtRefArg r, newtRefArg p, newtRefArg v);
newtRef		NSHasPath(newtRefArg r, newtRefArg p);
newtRef		NSGetPath(newtRefArg r, newtRefArg p);
newtRef		NSSetPath(newtRefArg r, newtRefArg p, newtRefArg v);
newtRef		NSARef(newtRefArg r, newtRefArg p);
newtRef		NSSetARef(newtRefArg r, newtRefArg p, newtRefArg v);
newtRef		NSHasVariable(newtRefArg r, newtRefArg name);
newtRef		NSHasVar(newtRefArg name);

newtRef		NSClassOf(newtRefArg r);
newtRef		NSSetClass(newtRefArg r, newtRefArg c);
newtRef		NSRefEqual(newtRefArg r1, newtRefArg r2);
newtRef		NSObjectEqual(newtRefArg r1, newtRefArg r2);
newtRef		NSHasSubclass(newtRefArg sub, newtRefArg supr);
newtRef		NSIsSubclass(newtRefArg sub, newtRefArg supr);
newtRef		NSIsInstance(newtRefArg obj, newtRefArg rr);
newtRef		NSIsArray(newtRefArg r);
newtRef		NSIsFrame(newtRefArg r);
newtRef		NSIsSymbol(newtRefArg r);
newtRef		NSIsString(newtRefArg r);
newtRef		NSIsCharacter(newtRefArg r);
newtRef		NSIsInteger(newtRefArg r);
newtRef		NSIsReal(newtRefArg r);

newtRef		NSAddArraySlot(newtRefArg r, newtRefArg v);
newtRef		NSStringer(newtRefArg r);
newtRef		NSStrCat(newtRefArg str, newtRefArg v);
newtRef		NSMakeSymbol(newtRefArg r);
newtRef		NSMakeFrame(void);
newtRef		NSMakeBinary(newtRefArg length, newtRefArg klass);
#endif /* __USE_OBSOLETE_STYLE__ */

// NewtonScript native functions(new style)
newtRef		NcProtoLookupFrame(newtRefArg start, newtRefArg name);
newtRef		NcProtoLookup(newtRefArg start, newtRefArg name);
newtRef		NcLexicalLookup(newtRefArg start, newtRef name);
newtRef		NcFullLookupFrame(newtRefArg start, newtRefArg name);
newtRef		NcFullLookup(newtRefArg start, newtRefArg name);
newtRef		NcLookupSymbol(newtRefArg r, newtRefArg name);

newtRef		NsThrow(newtRefArg rcvr, newtRefArg name, newtRefArg data);
newtRef		NsRethrow(newtRefArg rcvr);
newtRef		NcClone(newtRefArg r);									// bytecode
newtRef		NsTotalClone(newtRefArg rcvr, newtRefArg r);
newtRef		NcLength(newtRefArg r);									// bytecode
newtRef		NsDeeplyLength(newtRefArg rcvr, newtRefArg r);
newtRef		NsSetLength(newtRefArg rcvr, newtRefArg r, newtRefArg len);
newtRef		NsHasSlot(newtRefArg rcvr, newtRefArg frame, newtRefArg slot);
newtRef		NsGetSlot(newtRefArg rcvr, newtRefArg frame, newtRefArg slot);
newtRef		NsSetSlot(newtRefArg rcvr, newtRefArg frame, newtRefArg slot, newtRefArg v);
newtRef		NsRemoveSlot(newtRefArg rcvr, newtRefArg frame, newtRefArg slot);
newtRef		NcSetArraySlot(newtRefArg r, newtRefArg p, newtRefArg v);
newtRef		NcHasPath(newtRefArg r, newtRefArg p);					// bytecode
newtRef		NcGetPath(newtRefArg r, newtRefArg p);					// bytecode
newtRef		NcSetPath(newtRefArg r, newtRefArg p, newtRefArg v);	// bytecode
newtRef		NcARef(newtRefArg r, newtRefArg p);						// bytecode
newtRef		NcSetARef(newtRefArg r, newtRefArg p, newtRefArg v);	// bytecode
newtRef		NsHasVariable(newtRefArg rcvr, newtRefArg r, newtRefArg name);
newtRef		NsGetVariable(newtRefArg rcvr, newtRefArg frame, newtRefArg slot);
newtRef		NsSetVariable(newtRefArg rcvr, newtRefArg frame, newtRefArg slot, newtRefArg v);
newtRef		NsHasVar(newtRefArg rcvr, newtRefArg name);
newtRef		NsPrimClassOf(newtRefArg rcvr, newtRefArg r);
newtRef		NcClassOf(newtRefArg r);								// bytecode
newtRef		NcSetClass(newtRefArg r, newtRefArg c);					// bytecode
newtRef		NcRefEqual(newtRefArg r1, newtRefArg r2);				// bytecode
newtRef		NsObjectEqual(newtRefArg rcvr, newtRefArg r1, newtRefArg r2);
newtRef		NsSymbolCompareLex(newtRefArg rcvr, newtRefArg r1, newtRefArg r2);
newtRef		NsHasSubclass(newtRefArg rcvr, newtRefArg sub, newtRefArg supr);
newtRef		NsIsSubclass(newtRefArg rcvr, newtRefArg sub, newtRefArg supr);
newtRef		NsIsInstance(newtRefArg rcvr, newtRefArg obj, newtRefArg rr);
newtRef		NsIsArray(newtRefArg rcvr, newtRefArg r);
newtRef		NsIsFrame(newtRefArg rcvr, newtRefArg r);
newtRef		NsIsBinary(newtRefArg rcvr, newtRefArg r);
newtRef		NsIsSymbol(newtRefArg rcvr, newtRefArg r);
newtRef		NsIsString(newtRefArg rcvr, newtRefArg r);
newtRef		NsIsCharacter(newtRefArg rcvr, newtRefArg r);
newtRef		NsIsInteger(newtRefArg rcvr, newtRefArg r);
newtRef		NsIsReal(newtRefArg rcvr, newtRefArg r);
newtRef		NsIsNumber(newtRefArg rcvr, newtRefArg r);
newtRef		NsIsImmediate(newtRefArg rcvr, newtRefArg r);
newtRef		NsIsFunction(newtRefArg rcvr, newtRefArg r);
newtRef		NsIsReadonly(newtRefArg rcvr, newtRefArg r);

newtRef		NcAddArraySlot(newtRefArg r, newtRefArg v);				// bytecode
newtRef		NcStringer(newtRefArg r);								// bytecode
newtRef		NsStrCat(newtRefArg rcvr, newtRefArg str, newtRefArg v);
newtRef		NsMakeSymbol(newtRefArg rcvr, newtRefArg r);
newtRef		NsMakeFrame(newtRefArg rcvr);
newtRef		NsMakeBinary(newtRefArg rcvr, newtRefArg length, newtRefArg klass);

newtRef		NcBAnd(newtRefArg r1, newtRefArg r2);					// bytecode
newtRef		NcBOr(newtRefArg r1, newtRefArg r2);					// bytecode
newtRef		NcBNot(newtRefArg r);									// bytecode
newtRef		NsAnd(newtRefArg rcvr, newtRefArg r1, newtRefArg r2); 
newtRef		NsOr(newtRefArg rcvr, newtRefArg r1, newtRefArg r2); 
newtRef		NcAdd(newtRefArg r1, newtRefArg r2);					// bytecode
newtRef		NcSubtract(newtRefArg r1, newtRefArg r2);				// bytecode
newtRef		NcMultiply(newtRefArg r1, newtRefArg r2);				// bytecode
newtRef		NcDivide(newtRefArg r1, newtRefArg r2); 				// bytecode
newtRef		NcDiv(newtRefArg r1, newtRefArg r2); 					// bytecode
newtRef		NsMod(newtRefArg rcvr, newtRefArg r1, newtRefArg r2); 
newtRef		NsShiftLeft(newtRefArg rcvr, newtRefArg r1, newtRefArg r2); 
newtRef		NsShiftRight(newtRefArg rcvr, newtRefArg r1, newtRefArg r2); 
newtRef		NcLessThan(newtRefArg r1, newtRefArg r2);				// bytecode
newtRef		NcGreaterThan(newtRefArg r1, newtRefArg r2);			// bytecode
newtRef		NcGreaterOrEqual(newtRefArg r1, newtRefArg r2);			// bytecode 
newtRef		NcLessOrEqual(newtRefArg r1, newtRefArg r2);			// bytecode

newtRef		NsCurrentException(newtRefArg rcvr);
newtRef		NsMakeRegex(newtRefArg rcvr, newtRefArg pattern, newtRefArg opt);

newtRef		NsPrintObject(newtRefArg rcvr, newtRefArg r);
newtRef		NsPrint(newtRefArg rcvr, newtRefArg r);
newtRef		NsInfo(newtRefArg rcvr, newtRefArg r);
newtRef		NsDumpFn(newtRefArg rcvr, newtRefArg r);
newtRef		NsDumpBC(newtRefArg rcvr, newtRefArg r);
newtRef		NsDumpStacks(newtRefArg rcvr);

newtRef		NsCompile(newtRefArg rcvr, newtRefArg r);
newtRef		NsGetEnv(newtRefArg rcvr, newtRefArg r);


#ifdef __cplusplus
}
#endif


#endif /* NEWTFNS_H */

