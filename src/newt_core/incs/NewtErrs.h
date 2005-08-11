/*------------------------------------------------------------------------*/
/**
 * @file	NewtErrs.h
 * @brief   �G���[�R�[�h
 *
 * @author  M.Nukui
 * @date	2003-11-07
 *
 * @note	�Ǝ���`�ȊO�� Apple Newton OS �ɏ���
 *
 * Copyright (C) 2003-2004 M.Nukui All rights reserved.
 */


#ifndef	NEWTERRS_H
#define	NEWTERRS_H

/* �}�N����` */

#define kNErrNone						0						///< �G���[�Ȃ�
#define kNErrBase						(-48000)				///< �V�X�e����`�G���[

// �G���[�x�[�X
#define kNErrObjectBase					(kNErrBase - 200)		///< �I�u�W�F�N�g�G���[
#define kNErrBadTypeBase				(kNErrBase - 400)		///< �s���^�C�v�G���[
#define kNErrCompilerBase				(kNErrBase - 600)		///< �R���p�C���G���[
#define kNErrInterpreterBase			(kNErrBase - 800)		///< �C���^�v���^�G���[
#define kNErrFileBase					(kNErrBase - 1000)		///< �t�@�C���G���[�i�Ǝ���`�j
#define kNErrSystemBase					(kNErrBase - 1100)		///< �V�X�e���G���[�i�Ǝ���`�j
#define kNErrMiscBase					(kNErrBase - 2000)		///< ���̑��̃G���[�i�Ǝ���`�j

// �I�u�W�F�N�g�G���[
#define kNErrObjectPointerOfNonPtr		(kNErrObjectBase - 0)
#define kNErrBadMagicPointer			(kNErrObjectBase - 1)
#define kNErrEmptyPath					(kNErrObjectBase - 2)
#define kNErrBadSegmentInPath			(kNErrObjectBase - 3)
#define kNErrPathFailed					(kNErrObjectBase - 4)
#define kNErrOutOfBounds				(kNErrObjectBase - 5)
#define kNErrObjectsNotDistinct			(kNErrObjectBase - 6)
#define kNErrLongOutOfRange				(kNErrObjectBase - 7)
#define kNErrSettingHeapSizeTwice		(kNErrObjectBase - 8)
#define kNErrGcDuringGc					(kNErrObjectBase - 9)
#define kNErrBadArgs					(kNErrObjectBase - 10)
#define kNErrStringTooBig				(kNErrObjectBase - 11)
#define kNErrTFramesObjectPtrOfNil		(kNErrObjectBase - 12)
#define kNErrUnassignedTFramesObjectPtr	(kNErrObjectBase - 13)
#define kNErrObjectReadOnly				(kNErrObjectBase - 14)
#define kNErrOutOfObjectMemory			(kNErrObjectBase - 16)
#define kNErrDerefMagicPointer			(kNErrObjectBase - 17)
#define kNErrNegativeLength				(kNErrObjectBase - 18)
#define kNErrOutOfRange					(kNErrObjectBase - 19)
#define kNErrCouldntResizeLockedObject	(kNErrObjectBase - 20)
#define kNErrBadPackageRef				(kNErrObjectBase - 21)
#define kNErrBadExceptionName			(kNErrObjectBase - 22)

// �s���^�C�v�G���[
#define kNErrNotAFrame					(kNErrBadTypeBase - 0)
#define kNErrNotAnArray					(kNErrBadTypeBase - 1)
#define kNErrNotAString					(kNErrBadTypeBase - 2)
#define kNErrNotAPointer				(kNErrBadTypeBase - 3)
#define kNErrNotANumber					(kNErrBadTypeBase - 4)
#define kNErrNotAReal					(kNErrBadTypeBase - 5)
#define kNErrNotAnInteger				(kNErrBadTypeBase - 6)
#define kNErrNotACharacter				(kNErrBadTypeBase - 7)
#define kNErrNotABinaryObject			(kNErrBadTypeBase - 8)
#define kNErrNotAPathExpr				(kNErrBadTypeBase - 9)
#define kNErrNotASymbol					(kNErrBadTypeBase - 10)
#define kNErrNotAFunction				(kNErrBadTypeBase - 11)
#define kNErrNotAFrameOrArray			(kNErrBadTypeBase - 12)
#define kNErrNotAnArrayOrNil			(kNErrBadTypeBase - 13)
#define kNErrNotAStringOrNil			(kNErrBadTypeBase - 14)
#define kNErrNotABinaryObjectOrNil		(kNErrBadTypeBase - 15)
#define kNErrUnexpectedFrame			(kNErrBadTypeBase - 16)
#define kNErrUnexpectedBinaryObject		(kNErrBadTypeBase - 17)
#define kNErrUnexpectedImmediate		(kNErrBadTypeBase - 18)
#define kNErrNotAnArrayOrString			(kNErrBadTypeBase - 19)
#define kNErrNotAVBO					(kNErrBadTypeBase - 20)
#define kNErrNotAPackage				(kNErrBadTypeBase - 21)
#define kNErrNotNil						(kNErrBadTypeBase - 22)
#define kNErrNotASymbolOrNil			(kNErrBadTypeBase - 23)
#define kNErrNotTrueOrNil				(kNErrBadTypeBase - 24)
#define kNErrNotAnIntegerOrArray		(kNErrBadTypeBase - 25)

// �R���p�C���G���[
#define kNErrSyntaxError				(kNErrCompilerBase - 1)
#define kNErrAssignToConstant			(kNErrCompilerBase - 3)
#define kNErrCantTest					(kNErrCompilerBase - 4)
#define kNErrGlobalVarNotAllowed		(kNErrCompilerBase - 5)
#define kNErrCantHaveSameName			(kNErrCompilerBase - 6)
#define kNErrCantRedefineConstant		(kNErrCompilerBase - 7)
#define kNErrCantHaveSameNameInScope	(kNErrCompilerBase - 8)
#define kNErrNonLiteralExpression		(kNErrCompilerBase - 9)		///< �萔��`
#define kNErrEndOfInputString			(kNErrCompilerBase - 10)
#define kNErrOddNumberOfDigits			(kNErrCompilerBase - 11)	///< \\u
#define kNErrNoEscapes					(kNErrCompilerBase - 12)
#define kNErrInvalidHexCharacter		(kNErrCompilerBase - 13)	///< \\u ������
#define kNErrNotTowDigitHex				(kNErrCompilerBase - 17)	///< $\\ ����
#define kNErrNotFourDigitHex			(kNErrCompilerBase - 18)	///< $\u ����
#define kNErrIllegalCharacter			(kNErrCompilerBase - 19)
#define kNErrInvalidHexadecimal			(kNErrCompilerBase - 20)
#define kNErrInvalidReal				(kNErrCompilerBase - 21)
#define kNErrInvalidDecimal				(kNErrCompilerBase - 22)
#define kNErrNotConstant				(kNErrCompilerBase - 27)
#define kNErrNotDecimalDigit			(kNErrCompilerBase - 28)	///<  @

// �C���^�v���^�G���[
#define kNErrNotInBreakLoop				(kNErrInterpreterBase - 0)
#define kNErrTooManyArgs				(kNErrInterpreterBase - 2)
#define kNErrWrongNumberOfArgs			(kNErrInterpreterBase - 3)
#define kNErrZeroForLoopIncr			(kNErrInterpreterBase - 4)
#define kNErrNoCurrentException			(kNErrInterpreterBase - 6)
#define kNErrUndefinedVariable			(kNErrInterpreterBase - 7)
#define kNErrUndefinedGlobalFunction	(kNErrInterpreterBase - 8)
#define kNErrUndefinedMethod			(kNErrInterpreterBase - 9)
#define kNErrMissingProtoForResend		(kNErrInterpreterBase - 10)
#define kNErrNilContext					(kNErrInterpreterBase - 11)
#define kNErrBadCharForString			(kNErrInterpreterBase - 15)

// �C���^�v���^�G���[�i�Ǝ���`�j
#define kNErrInvalidFunc				(kNErrInterpreterBase - 100)	///< �֐����s��
#define kNErrInvalidInstruction			(kNErrInterpreterBase - 101)	///< �o�C�g�R�[�h�̃C���X�g���N�V�������s��

// �t�@�C���G���[�i�Ǝ���`�j
#define kNErrFileNotFound				(kNErrFileBase - 0)				///< �t�@�C�������݂��Ȃ�
#define kNErrFileNotOpen				(kNErrFileBase - 1)				///< �t�@�C�����I�[�v���ł��Ȃ�
#define kNErrDylibNotOpen				(kNErrFileBase - 2)				///< ���I���C�u�������I�[�v���ł��Ȃ�

// �V�X�e���G���[�i�Ǝ���`�j
#define kNErrSystemError				(kNErrSystemBase - 0)			///< �V�X�e���G���[

// ���̑��̃G���[�i�Ǝ���`�j
#define kNErrDiv0						(kNErrMiscBase - 0)				///< 0�Ŋ���Z����
																		// Newton OS �ł͗�O�͔������Ȃ��H
#define kNErrRegcomp					(kNErrMiscBase - 1)				///< ���K�\���̃R���p�C���G���[
#define kNErrNSOFWrite					(kNErrMiscBase - 2)				///< NSOF�̏����݃G���[
#define kNErrNSOFRead					(kNErrMiscBase - 3)				///< NSOF�̓Ǎ��݃G���[

#endif /* NEWTERRS_H */
