/*------------------------------------------------------------------------*/
/**
 * @file	NewtFns.c
 * @brief   �g���݊֐�
 *
 * @author  M.Nukui
 * @date	2003-11-07
 *
 * Copyright (C) 2003-2004 M.Nukui All rights reserved.
 */


/* �w�b�_�t�@�C�� */
#include <string.h>

#include "NewtErrs.h"
#include "NewtFns.h"
#include "NewtEnv.h"
#include "NewtObj.h"
#include "NewtVM.h"
#include "NewtBC.h"
#include "NewtPrint.h"


/* �֐��v���g�^�C�v */
static newtRef		NewtRefTypeToClass(uint16_t type);
static newtRef		NewtObjClassOf(newtRefArg r);
static newtRef		NewtObjSetClass(newtRefArg r, newtRefArg c);
static bool			NewtArgsIsNumber(newtRefArg r1, newtRefArg r2, bool * real);


#ifdef __USE_OBSOLETE_STYLE__
#pragma mark -
/*------------------------------------------------------------------------*/
/** �v���g�p���ŃV���{�������� (OBSOLETE)
 *
 * @param start		[in] �J�n�I�u�W�F�N�g
 * @param name		[in] �V���{���I�u�W�F�N�g
 *
 * @return			�������ꂽ�I�u�W�F�N�g
 */

newtRef NSProtoLookup(newtRefArg start, newtRefArg name)
{
	return NcProtoLookup(start, name);
}

/*------------------------------------------------------------------------*/
/** ���L�V�J���X�R�[�v�ŃV���{�������� (OBSOLETE)
 *
 * @param start		[in] �J�n�I�u�W�F�N�g
 * @param name		[in] �V���{���I�u�W�F�N�g
 *
 * @return			�������ꂽ�I�u�W�F�N�g
 */

newtRef NSLexicalLookup(newtRefArg start, newtRef name)
{
	return NcLexicalLookup(start, name);
}

/*------------------------------------------------------------------------*/
/** �v���g�A�y�A�����g�p���ŃV���{�������� (OBSOLETE)
 *
 * @param start		[in] �J�n�I�u�W�F�N�g
 * @param name		[in] �V���{���I�u�W�F�N�g
 *
 * @return			�������ꂽ�I�u�W�F�N�g
 */

newtRef NSFullLookup(newtRefArg start, newtRefArg name)
{
	return NcFullLookup(start, name);
}

/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g���N���[���������� (OBSOLETE)
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�N���[���������ꂽ�I�u�W�F�N�g
 */

newtRef NSClone(newtRefArg r)
{
	return NcClone(r);
}

/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̐[���N���[������������i�}�W�b�N�|�C���^�͒ǐՂ��Ȃ��j (OBSOLETE)
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�N���[���������ꂽ�I�u�W�F�N�g
 */

newtRef NSTotalClone(newtRefArg r)
{
	return NcTotalClone(r);
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �V���{���e�[�u������V���{�������� (OBSOLETE)
 *
 * @param r			[in] �V���{���e�[�u��
 * @param name		[in] �V���{����
 *
 * @return			�������ꂽ�V���{���I�u�W�F�N�g
 */

newtRef NSLookupSymbol(newtRefArg r, newtRefArg name)
{
    return NcLookupSymbol(r, name);
}

/*------------------------------------------------------------------------*/
/** ��O�𔭐�(OBSOLETE)
 *
 * @param name		[in] ��O�V���{��
 * @param data		[in] ��O�f�[�^
 *
 * @retval			NIL
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NSThrow(newtRefArg name, newtRefArg data)
{
    NVMThrow(name, data);
    return kNewtRefNIL;
}

/*------------------------------------------------------------------------*/
/** rethrow ���� (OBSOLETE)
 *
 * @retval			NIL
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NSRethrow(void)
{
    NVMRethrow();
    return kNewtRefNIL;
}

/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̒������擾 (OBSOLETE)
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�I�u�W�F�N�g�̒���
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NSLength(newtRefArg r)
{
    return NcLength(r);
}

/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�́i�[���j�������擾 (OBSOLETE)
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�I�u�W�F�N�g�̒���
 *
 * @note			�t���[���̏ꍇ�̓v���g�p���Œ������v�Z����
 *					�X�N���v�g����̌ďo���p
 */

newtRef NSDeeplyLength(newtRefArg r)
{
    return NcDeeplyLength(r);
}


/*------------------------------------------------------------------------*/
/** �t���[�����̃X���b�g�̗L���𒲂ׂ� (OBSOLETE)
 *
 * @param frame		[in] �t���[��
 * @param slot		[in] �X���b�g�V���{��
 *
 * @retval			TRUE	�X���b�g�����݂���
 * @retval			NIL		�X���b�g�����݂��Ȃ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NSHasSlot(newtRefArg frame, newtRefArg slot)
{
    return NcHasSlot(frame, slot);
}

/*------------------------------------------------------------------------*/
/** �t���[������X���b�g�̒l���擾 (OBSOLETE)
 *
 * @param frame		[in] �t���[��
 * @param slot		[in] �X���b�g�V���{��
 *
 * @return			�X���b�g�̒l
 */

newtRef NSGetSlot(newtRefArg frame, newtRefArg slot)
{
	return NcGetSlot(frame, slot);
}


/*------------------------------------------------------------------------*/
/** �t���[���ɃX���b�g�̒l���Z�b�g���� (OBSOLETE)
 *
 * @param frame		[in] �t���[��
 * @param slot		[in] �X���b�g�V���{��
 * @param v			[in] �l�I�u�W�F�N�g
 *
 * @return			�l�I�u�W�F�N�g
 */

newtRef NSSetSlot(newtRefArg frame, newtRefArg slot, newtRefArg v)
{
	return NcSetSlot(frame, slot, v);
}

/*------------------------------------------------------------------------*/
/** �t���[������X���b�g���폜���� (OBSOLETE)
 *
 * @param frame		[in] �t���[��
 * @param slot		[in] �X���b�g�V���{��
 *
 * @return			�t���[��
 */

newtRef NSRemoveSlot(newtRefArg frame, newtRefArg slot)
{
	return NcRemoveSlot(frame, slot);
}

/*------------------------------------------------------------------------*/
/** �z��ɒl���Z�b�g���� (OBSOLETE)
 *
 * @param r			[in] �z��I�u�W�F�N�g
 * @param p			[in] �ʒu
 * @param v			[in] �l�I�u�W�F�N�g
 *
 * @return			�l�I�u�W�F�N�g
 */

newtRef NSSetArraySlot(newtRefArg r, newtRefArg p, newtRefArg v)
{
	return NcSetArraySlot(r, p, v);
}

/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g���̃A�N�Z�X�p�X�̗L���𒲂ׂ� (OBSOLETE)
 *
 * @param r			[in] �I�u�W�F�N�g
 * @param p			[in] �A�N�Z�X�p�X
 *
 * @retval			TRUE	�A�N�Z�X�p�X�����݂���
 * @retval			NIL		�A�N�Z�X�p�X�����݂��Ȃ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NSHasPath(newtRefArg r, newtRefArg p)
{
    return NewtMakeBoolean(NewtHasPath(r, p));
}

/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̃A�N�Z�X�p�X�̒l���擾 (OBSOLETE)
 *
 * @param r			[in] �I�u�W�F�N�g
 * @param p			[in] �A�N�Z�X�p�X
 *
 * @return			�l�I�u�W�F�N�g
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NSGetPath(newtRefArg r, newtRefArg p)
{
    return NewtGetPath(r, p, NULL);
}

/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̃A�N�Z�X�p�X�ɒl���Z�b�g���� (OBSOLETE)
 *
 * @param r			[in] �I�u�W�F�N�g
 * @param p			[in] �A�N�Z�X�p�X
 * @param v			[in] �l�I�u�W�F�N�g
 *
 * @return			�l�I�u�W�F�N�g
 */

newtRef NSSetPath(newtRefArg r, newtRefArg p, newtRefArg v)
{
	return NcSetPath(r, p, v);
}

/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̎w�肳�ꂽ�ʒu����l���擾 (OBSOLETE)
 *
 * @param r			[in] �I�u�W�F�N�g
 * @param p			[in] �ʒu
 *
 * @return			�l�I�u�W�F�N�g
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NSARef(newtRefArg r, newtRefArg p)
{
	return NcARef(r, p);
}

/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̎w�肳�ꂽ�ʒu�ɒl���Z�b�g���� (OBSOLETE)
 *
 * @param r			[in] �I�u�W�F�N�g
 * @param p			[in] �ʒu
 * @param v			[in] �l�I�u�W�F�N�g
 *
 * @return			�l�I�u�W�F�N�g
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NSSetARef(newtRefArg r, newtRefArg p, newtRefArg v)
{
	return NcSetARef(r, p, v);
}

/*------------------------------------------------------------------------*/
/** �v���g�E�y�A�����g�p���Ńt���[�����̃X���b�g�̗L���𒲂ׂ� (OBSOLETE)
 *
 * @param r			[in] �t���[��
 * @param name		[in] �X���b�g�V���{��
 *
 * @retval			TRUE	�X���b�g�����݂���
 * @retval			NIL		�X���b�g�����݂��Ȃ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NSHasVariable(newtRefArg r, newtRefArg name)
{
    return NewtMakeBoolean(NewtHasVariable(r, name));
}


/*------------------------------------------------------------------------*/
/** �ϐ��̗L���𒲂ׂ� (OBSOLETE)
 *
 * @param name		[in] �ϐ����V���{��
 *
 * @retval			TRUE	�X���b�g�����݂���
 * @retval			NIL		�X���b�g�����݂��Ȃ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NSHasVar(newtRefArg name)
{
    return NewtMakeBoolean(NewtHasVar(name));
}

/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̃N���X�V���{�����擾 (OBSOLETE)
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�N���X�V���{��
 */

newtRef NSClassOf(newtRefArg r)
{
	return NcClassOf(r);
}

/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̃N���X�V���{�����Z�b�g���� (OBSOLETE)
 *
 * @param r			[in] �I�u�W�F�N�g
 * @param c			[in] �N���X�V���{��
 *
 * @retval			�I�u�W�F�N�g	�N���X�V���{�����Z�b�g�ł����ꍇ
 * @retval			NIL			�N���X�V���{�����Z�b�g�ł��Ȃ������ꍇ
 */

newtRef NSSetClass(newtRefArg r, newtRefArg c)
{
	return NcSetClass(r, c);
}

/*------------------------------------------------------------------------*/
/** �Q�Ƃ̔�r (OBSOLETE)
 *
 * @param r1		[in] �Q�ƂP
 * @param r2		[in] �Q�ƂQ
 *
 * @retval			TRUE	���l
 * @retval			NIL		���l�łȂ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NSRefEqual(newtRefArg r1, newtRefArg r2)
{
    return NewtMakeBoolean(NewtRefEqual(r1, r2));
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̔�r (OBSOLETE)
 *
 * @param r1		[in] �I�u�W�F�N�g�P
 * @param r2		[in] �I�u�W�F�N�g�Q
 *
 * @retval			TRUE	���l
 * @retval			NIL		���l�łȂ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NSObjectEqual(newtRefArg r1, newtRefArg r2)
{
    return NewtMakeBoolean(NewtObjectEqual(r1, r2));
}


/*------------------------------------------------------------------------*/
/** sub �� supr �̃T�u�N���X���܂ނ��`�F�b�N���� (OBSOLETE)
 *
 * @param sub		[in] �V���{���I�u�W�F�N�g�P
 * @param supr		[in] �V���{���I�u�W�F�N�g�Q
 *
 * @retval			TRUE	�T�u�N���X���܂�
 * @retval			NIL		�T�u�N���X���܂܂Ȃ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NSHasSubclass(newtRefArg sub, newtRefArg supr)
{
    return NewtMakeBoolean(NewtHasSubclass(sub, supr));
}

/*------------------------------------------------------------------------*/
/** sub �� supr �̃T�u�N���X���`�F�b�N���� (OBSOLETE)
 *
 * @param sub		[in] �V���{���I�u�W�F�N�g�P
 * @param supr		[in] �V���{���I�u�W�F�N�g�Q
 *
 * @retval			TRUE	�T�u�N���X�ł���
 * @retval			NIL		�T�u�N���X�łȂ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NSIsSubclass(newtRefArg sub, newtRefArg supr)
{
    return NewtMakeBoolean(NewtIsSubclass(sub, supr));
}

/*------------------------------------------------------------------------*/
/** obj �� r �̃C���X�^���X���`�F�b�N���� (OBSOLETE)
 *
 * @param obj		[in] �I�u�W�F�N�g
 * @param r			[in] �N���X�V���{��
 *
 * @retval			TRUE	�C���X�^���X�ł���
 * @retval			NIL		�C���X�^���X�łȂ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NSIsInstance(newtRefArg obj, newtRefArg r)
{
    return NewtMakeBoolean(NewtIsInstance(obj, r));
}

/*------------------------------------------------------------------------*/
/** r ���z�񂩃`�F�b�N���� (OBSOLETE)
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			TRUE	�z��ł���
 * @retval			NIL		�z��łȂ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NSIsArray(newtRefArg r)
{
    return NewtMakeBoolean(NewtRefIsArray(r));
}

/*------------------------------------------------------------------------*/
/** r ���t���[�����`�F�b�N���� (OBSOLETE)
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			TRUE	�t���[���ł���
 * @retval			NIL		�t���[���łȂ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NSIsFrame(newtRefArg r)
{
    return NewtMakeBoolean(NewtRefIsFrame(r));
}

/*------------------------------------------------------------------------*/
/** r ���V���{�����`�F�b�N���� (OBSOLETE)
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			TRUE	�V���{���ł���
 * @retval			NIL		�V���{���łȂ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NSIsSymbol(newtRefArg r)
{
    return NewtMakeBoolean(NewtRefIsSymbol(r));
}

/*------------------------------------------------------------------------*/
/** r �������񂩃`�F�b�N���� (OBSOLETE)
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			TRUE	������ł���
 * @retval			NIL		������łȂ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NSIsString(newtRefArg r)
{
    return NewtMakeBoolean(NewtRefIsString(r));
}

/*------------------------------------------------------------------------*/
/** r ���������`�F�b�N���� (OBSOLETE)
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			TRUE	�����ł���
 * @retval			NIL		�����łȂ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NSIsCharacter(newtRefArg r)
{
    return NewtMakeBoolean(NewtRefIsCharacter(r));
}

/*------------------------------------------------------------------------*/
/** r ���������`�F�b�N���� (OBSOLETE)
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			TRUE	�����ł���
 * @retval			NIL		�����łȂ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NSIsInteger(newtRefArg r)
{
    return NewtMakeBoolean(NewtRefIsInteger(r));
}

/*------------------------------------------------------------------------*/
/** r �����������_�����`�F�b�N���� (OBSOLETE)
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			TRUE	���������_���ł���
 * @retval			NIL		���������_���łȂ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NSIsReal(newtRefArg r)
{
    return NewtMakeBoolean(NewtRefIsReal(r));
}

/*------------------------------------------------------------------------*/
/** �z��I�u�W�F�N�g�ɒl��ǉ����� (OBSOLETE)
 *
 * @param r			[in] �z��I�u�W�F�N�g
 * @param v			[in] �l�I�u�W�F�N�g
 *
 * @return			�l�I�u�W�F�N�g
 */

newtRef NSAddArraySlot(newtRefArg r, newtRefArg v)
{
	return NcAddArraySlot(r, v);
}

/*------------------------------------------------------------------------*/
/** �z��I�u�W�F�N�g�̗v�f�𕶎���ɍ������� (OBSOLETE)
 *
 * @param r			[in] �z��I�u�W�F�N�g
 *
 * @return			������I�u�W�F�N�g
 */

newtRef NSStringer(newtRefArg r)
{
	return NcStringer(r);
}

/*------------------------------------------------------------------------*/
/** ������I�u�W�F�N�g�̍Ō�ɃI�u�W�F�N�g�𕶎��񉻂��Ēǉ����� (OBSOLETE)
 *
 * @param str		[in] ������I�u�W�F�N�g
 * @param v			[in] �I�u�W�F�N�g
 *
 * @return			������I�u�W�F�N�g
 */

newtRef NSStrCat(newtRefArg str, newtRefArg v)
{
	return NcStrCat(str, v);
}

/*------------------------------------------------------------------------*/
/** ������I�u�W�F�N�g����V���{�����쐬���� (OBSOLETE)
 *
 * @param r			[in] ������I�u�W�F�N�g
 *
 * @return			�V���{���I�u�W�F�N�g
 */

newtRef NSMakeSymbol(newtRefArg r)
{
	return NcMakeSymbol(r);
}

/*------------------------------------------------------------------------*/
/** �t���[���I�u�W�F�N�g���쐬���� (OBSOLETE)
 *
 * @return			�t���[���I�u�W�F�N�g
 */

newtRef NSMakeFrame(void)
{
	return NcMakeFrame();
}

/*------------------------------------------------------------------------*/
/** �o�C�i���I�u�W�F�N�g���쐬���� (OBSOLETE)
 *
 * @param length	[in] ����
 * @param klass		[in] �N���X
 *
 * @return			�o�C�i���I�u�W�F�N�g
 */

newtRef NSMakeBinary(newtRefArg length, newtRefArg klass)
{
	return NcMakeBinary(length, klass);
}

#endif /* __USE_OBSOLETE_STYLE__ */


#pragma mark -
/*------------------------------------------------------------------------*/
/** �v���g�p���ŃV���{���������i�t���[����������j
 *
 * @param start		[in] �J�n�I�u�W�F�N�g
 * @param name		[in] �V���{���I�u�W�F�N�g
 *
 * @return			�������ꂽ�I�u�W�F�N�g�����t���[��
 */

newtRef NcProtoLookupFrame(newtRefArg start, newtRefArg name)
{
    newtRefVar	current = start;

    while (NewtRefIsNotNIL(current))
    {
		current = NcResolveMagicPointer(current);

		if (NewtRefIsMagicPointer(current))
			return kNewtRefUnbind;

        if (NewtHasSlot(current, name))
            return current;

        current = NcGetSlot(current, NSSYM0(_proto));
    }

    return kNewtRefUnbind;
}


/*------------------------------------------------------------------------*/
/** �v���g�p���ŃV���{��������
 *
 * @param start		[in] �J�n�I�u�W�F�N�g
 * @param name		[in] �V���{���I�u�W�F�N�g
 *
 * @return			�������ꂽ�I�u�W�F�N�g
 */

newtRef NcProtoLookup(newtRefArg start, newtRefArg name)
{
    newtRefVar	current;

	current = NcProtoLookupFrame(start, name);

	if (current != kNewtRefUnbind)
		return NcGetSlot(current, name);
	else
		return kNewtRefUnbind;
}


/*------------------------------------------------------------------------*/
/** ���L�V�J���X�R�[�v�ŃV���{��������
 *
 * @param start		[in] �J�n�I�u�W�F�N�g
 * @param name		[in] �V���{���I�u�W�F�N�g
 *
 * @return			�������ꂽ�I�u�W�F�N�g
 */

newtRef NcLexicalLookup(newtRefArg start, newtRef name)
{
    newtRefVar	current = start;

    while (NewtRefIsNotNIL(current))
    {
		current = NcResolveMagicPointer(current);

		if (NewtRefIsMagicPointer(current))
			return kNewtRefUnbind;

        if (NewtHasSlot(current, name))
            return NcGetSlot(current, name);

        current = NcGetSlot(current, NSSYM0(_nextArgFrame));
    }

    return kNewtRefUnbind;
}


/*------------------------------------------------------------------------*/
/** �v���g�A�y�A�����g�p���ŃV���{���������i�t���[����������j
 *
 * @param start		[in] �J�n�I�u�W�F�N�g
 * @param name		[in] �V���{���I�u�W�F�N�g
 *
 * @return			�������ꂽ�I�u�W�F�N�g�����t���[��
 */

newtRef NcFullLookupFrame(newtRefArg start, newtRefArg name)
{
    newtRefVar	current;
    newtRefVar	left = start;

    while (NewtRefIsNotNIL(left))
    {
        current = left;

        while (NewtRefIsNotNIL(current))
        {
			current = NcResolveMagicPointer(current);

			if (NewtRefIsMagicPointer(current))
				return kNewtRefUnbind;

            if (NewtHasSlot(current, name))
                return current;
    
            current = NcGetSlot(current, NSSYM0(_proto));
        }

        left = NcGetSlot(left, NSSYM0(_parent));
    }

    return kNewtRefUnbind;
}


/*------------------------------------------------------------------------*/
/** �v���g�A�y�A�����g�p���ŃV���{��������
 *
 * @param start		[in] �J�n�I�u�W�F�N�g
 * @param name		[in] �V���{���I�u�W�F�N�g
 *
 * @return			�������ꂽ�I�u�W�F�N�g
 */

newtRef NcFullLookup(newtRefArg start, newtRefArg name)
{
    newtRefVar	current;

	current = NcFullLookupFrame(start, name);

	if (current != kNewtRefUnbind)
		return NcGetSlot(current, name);
	else
		return kNewtRefUnbind;
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �V���{���e�[�u������V���{��������
 *
 * @param r			[in] �V���{���e�[�u��
 * @param name		[in] �V���{����
 *
 * @return			�������ꂽ�V���{���I�u�W�F�N�g
 */

newtRef NcLookupSymbol(newtRefArg r, newtRefArg name)
{
    return NewtLookupSymbolArray(r, name, 0);
}


/*------------------------------------------------------------------------*/
/** ��O�𔭐�
 *
 * @param rcvr		[in] ���V�[�o
 * @param name		[in] ��O�V���{��
 * @param data		[in] ��O�f�[�^
 *
 * @retval			NIL
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NsThrow(newtRefArg rcvr, newtRefArg name, newtRefArg data)
{
    NVMThrow(name, data);
    return kNewtRefNIL;
}


/*------------------------------------------------------------------------*/
/** rethrow ����
 *
 * @param rcvr		[in] ���V�[�o
 *
 * @retval			NIL
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NsRethrow(newtRefArg rcvr)
{
    NVMRethrow();
    return kNewtRefNIL;
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g���N���[����������
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�N���[���������ꂽ�I�u�W�F�N�g
 */

newtRef NcClone(newtRefArg r)
{
    if (NewtRefIsPointer(r))
        return NewtObjClone(r);
    else
        return (newtRef)r;
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̐[���N���[������������i�}�W�b�N�|�C���^�͒ǐՂ��Ȃ��j
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�N���[���������ꂽ�I�u�W�F�N�g
 */

newtRef NsTotalClone(newtRefArg rcvr, newtRefArg r)
{
	newtRefVar	result;

	result = NcClone(r);

	if (NewtRefIsFrameOrArray(result))
	{
        newtRef *	slots;
        uint32_t	n;
        uint32_t	i;
    
        slots = NewtRefToSlots(result);
		n = NewtLength(result);
    
        for (i = 0; i < n; i++)
        {
            slots[i] = NsTotalClone(rcvr, slots[i]);
        }
	}

	return result;
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̒������擾
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�I�u�W�F�N�g�̒���
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NcLength(newtRefArg r)
{
    return NewtMakeInteger(NewtLength(r));
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�́i�[���j�������擾
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�I�u�W�F�N�g�̒���
 *
 * @note			�t���[���̏ꍇ�̓v���g�p���Œ������v�Z����
 *					�X�N���v�g����̌ďo���p
 */

newtRef NsDeeplyLength(newtRefArg rcvr, newtRefArg r)
{
    return NewtMakeInteger(NewtDeeplyLength(r));
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�́i�[���j�������擾
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �I�u�W�F�N�g
 * @param len		[in] ����
 *
 * @return			�I�u�W�F�N�g�̒���
 *
 * @note			�t���[���̏ꍇ�̓v���g�p���Œ������v�Z����
 *					�X�N���v�g����̌ďo���p
 */

newtRef NsSetLength(newtRefArg rcvr, newtRefArg r, newtRefArg len)
{
	int32_t	n;

	if (NewtRefIsReadonly(r))
		return NewtThrow(kNErrObjectReadOnly, r);

    if (! NewtRefIsInteger(len))
        return NewtThrow(kNErrNotAnInteger, len);

	n = NewtRefToInteger(len);

    switch (NewtGetRefType(r, true))
    {
        case kNewtBinary:
        case kNewtString:
            NewtBinarySetLength(r, n);
            break;

        case kNewtArray:
            NewtSlotsSetLength(r, n, kNewtRefUnbind);
			break;

        case kNewtFrame:
			return NewtThrow(kNErrUnexpectedFrame, r);

		default:
			return NewtThrow(kNErrNotAnArrayOrString, r);
    }

    return r;
}


/*------------------------------------------------------------------------*/
/** �t���[�����̃X���b�g�̗L���𒲂ׂ�
 *
 * @param rcvr		[in] ���V�[�o
 * @param frame		[in] �t���[��
 * @param slot		[in] �X���b�g�V���{��
 *
 * @retval			TRUE	�X���b�g�����݂���
 * @retval			NIL		�X���b�g�����݂��Ȃ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NsHasSlot(newtRefArg rcvr, newtRefArg frame, newtRefArg slot)
{
    return NewtMakeBoolean(NewtHasSlot(frame, slot));
}


/*------------------------------------------------------------------------*/
/** �t���[������X���b�g�̒l���擾
 *
 * @param rcvr		[in] ���V�[�o
 * @param frame		[in] �t���[��
 * @param slot		[in] �X���b�g�V���{��
 *
 * @return			�X���b�g�̒l
 */

newtRef NsGetSlot(newtRefArg rcvr, newtRefArg frame, newtRefArg slot)
{
    newtObjRef	obj;

    obj = NewtRefToPointer(frame);

    if (obj != NULL)
		return NcResolveMagicPointer(NewtObjGetSlot(obj, slot));
    else
        return kNewtRefUnbind;
}


/*------------------------------------------------------------------------*/
/** �t���[���ɃX���b�g�̒l���Z�b�g����
 *
 * @param rcvr		[in] ���V�[�o
 * @param frame		[in] �t���[��
 * @param slot		[in] �X���b�g�V���{��
 * @param v			[in] �l�I�u�W�F�N�g
 *
 * @return			�l�I�u�W�F�N�g
 */

newtRef NsSetSlot(newtRefArg rcvr, newtRefArg frame, newtRefArg slot, newtRefArg v)
{
    newtObjRef	obj;

    obj = NewtRefToPointer(frame);

    if (obj != NULL)
	{
		if (NewtRefIsReadonly(frame))
			return NewtThrow(kNErrObjectReadOnly, frame);

		return NewtObjSetSlot(obj, slot, v);
    }

   return kNewtRefNIL;
}


/*------------------------------------------------------------------------*/
/** �t���[������X���b�g���폜����
 *
 * @param rcvr		[in] ���V�[�o
 * @param frame		[in] �t���[��
 * @param slot		[in] �X���b�g�V���{��
 *
 * @return			�t���[��
 */

newtRef NsRemoveSlot(newtRefArg rcvr, newtRefArg frame, newtRefArg slot)
{
    newtObjRef	obj;

    obj = NewtRefToPointer(frame);
    NewtObjRemoveSlot(obj, slot);

    return frame;
}


/*------------------------------------------------------------------------*/
/** �z��ɒl���Z�b�g����
 *
 * @param r			[in] �z��I�u�W�F�N�g
 * @param p			[in] �ʒu
 * @param v			[in] �l�I�u�W�F�N�g
 *
 * @return			�l�I�u�W�F�N�g
 */

newtRef NcSetArraySlot(newtRefArg r, newtRefArg p, newtRefArg v)
{
    if (! NewtRefIsInteger(p))
        return NewtThrow(kNErrNotAnInteger, p);

    return NewtSetArraySlot(r, NewtRefToInteger(p), v);
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g���̃A�N�Z�X�p�X�̗L���𒲂ׂ�
 *
 * @param r			[in] �I�u�W�F�N�g
 * @param p			[in] �A�N�Z�X�p�X
 *
 * @retval			TRUE	�A�N�Z�X�p�X�����݂���
 * @retval			NIL		�A�N�Z�X�p�X�����݂��Ȃ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NcHasPath(newtRefArg r, newtRefArg p)
{
    return NewtMakeBoolean(NewtHasPath(r, p));
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̃A�N�Z�X�p�X�̒l���擾
 *
 * @param r			[in] �I�u�W�F�N�g
 * @param p			[in] �A�N�Z�X�p�X
 *
 * @return			�l�I�u�W�F�N�g
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NcGetPath(newtRefArg r, newtRefArg p)
{
    return NewtGetPath(r, p, NULL);
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̃A�N�Z�X�p�X�ɒl���Z�b�g����
 *
 * @param r			[in] �I�u�W�F�N�g
 * @param p			[in] �A�N�Z�X�p�X
 * @param v			[in] �l�I�u�W�F�N�g
 *
 * @return			�l�I�u�W�F�N�g
 */

newtRef NcSetPath(newtRefArg r, newtRefArg p, newtRefArg v)
{
    newtRefVar	slot;
    newtRefVar	target;

    target = NewtGetPath(r, p, &slot);

    if (target == kNewtRefUnbind)
        NewtThrow(kNErrOutOfBounds, r);

    if (NewtRefIsArray(target))
        return NcSetArraySlot(target, slot, v);
    else
        return NcSetSlot(target, slot, v);
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̎w�肳�ꂽ�ʒu����l���擾
 *
 * @param r			[in] �I�u�W�F�N�g
 * @param p			[in] �ʒu
 *
 * @return			�l�I�u�W�F�N�g
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NcARef(newtRefArg r, newtRefArg p)
{
    if (! NewtRefIsInteger(p))
        return NewtThrow(kNErrNotAnInteger, p);

    return NewtARef(r, NewtRefToInteger(p));
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̎w�肳�ꂽ�ʒu�ɒl���Z�b�g����
 *
 * @param r			[in] �I�u�W�F�N�g
 * @param p			[in] �ʒu
 * @param v			[in] �l�I�u�W�F�N�g
 *
 * @return			�l�I�u�W�F�N�g
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NcSetARef(newtRefArg r, newtRefArg p, newtRefArg v)
{
    if (! NewtRefIsInteger(p))
        return NewtThrow(kNErrNotAnInteger, p);

    return NewtSetARef(r, NewtRefToInteger(p), v);
}


/*------------------------------------------------------------------------*/
/** �v���g�E�y�A�����g�p���Ńt���[�����̃X���b�g�̗L���𒲂ׂ�
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �t���[��
 * @param name		[in] �X���b�g�V���{��
 *
 * @retval			TRUE	�X���b�g�����݂���
 * @retval			NIL		�X���b�g�����݂��Ȃ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NsHasVariable(newtRefArg rcvr, newtRefArg r, newtRefArg name)
{
    return NewtMakeBoolean(NewtHasVariable(r, name));
}


/*------------------------------------------------------------------------*/
/** �v���g�E�y�A�����g�p���Ńt���[������X���b�g�̒l���擾
 *
 * @param rcvr		[in] ���V�[�o
 * @param frame		[in] �t���[��
 * @param slot		[in] �X���b�g�V���{��
 *
 * @return			�X���b�g�̒l
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NsGetVariable(newtRefArg rcvr, newtRefArg frame, newtRefArg slot)
{
    return NcFullLookup(frame, slot);
}


/*------------------------------------------------------------------------*/
/** �v���g�E�y�A�����g�p���Ńt���[������X���b�g�̒l���擾
 *
 * @param rcvr		[in] ���V�[�o
 * @param frame		[in] �t���[��
 * @param slot		[in] �X���b�g�V���{��
 * @param v			[in] �l�I�u�W�F�N�g
 *
 * @return			�l�I�u�W�F�N�g
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NsSetVariable(newtRefArg rcvr, newtRefArg frame, newtRefArg slot, newtRefArg v)
{
	if (NewtAssignment(frame, slot, v))
		return v;
	else
		return kNewtRefUnbind;
}


/*------------------------------------------------------------------------*/
/** �ϐ��̗L���𒲂ׂ�
 *
 * @param rcvr		[in] ���V�[�o
 * @param name		[in] �ϐ����V���{��
 *
 * @retval			TRUE	�X���b�g�����݂���
 * @retval			NIL		�X���b�g�����݂��Ȃ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NsHasVar(newtRefArg rcvr, newtRefArg name)
{
    return NewtMakeBoolean(NewtHasVar(name));
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�^�C�v��Ή�����N���X�V���{���ɕϊ�����
 *
 * @param type		[in] �I�u�W�F�N�g�^�C�v
 *
 * @return			�N���X�V���{��
 */

newtRef NewtRefTypeToClass(uint16_t type)
{
    newtRefVar	klass = kNewtRefUnbind;

	switch (type)
	{
		case kNewtInt30:
		case kNewtInt32:
			klass = NS_INT;
			break;

		case kNewtCharacter:
			klass = NS_CHAR;
			break;

		case kNewtTrue:
			klass = NSSYM0(boolean);
			break;

		case kNewtSpecial:
		case kNewtNil:
		case kNewtUnbind:
			klass = NSSYM0(weird_immediate);
			break;

        case kNewtFrame:
            klass = NSSYM0(frame);
            break;

        case kNewtArray:
            klass = NSSYM0(array);
            break;

        case kNewtReal:
			klass = NSSYM0(real);
            break;

        case kNewtSymbol:
			klass = NSSYM0(symbol);
            break;

        case kNewtString:
            klass = NSSYM0(string);
            break;

        case kNewtBinary:
			klass = NSSYM0(binary);
            break;
	}

    return klass;
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̃N���X�V���{�����擾
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�N���X�V���{��
 */

newtRef NewtObjClassOf(newtRefArg r)
{
    newtObjRef	obj;
    newtRefVar	klass = kNewtRefNIL;

    obj = NewtRefToPointer(r);

    if (obj != NULL)
    {
        if (NewtObjIsFrame(obj))
        {
//            klass = NewtObjGetSlot(obj, NS_CLASS);
			klass = NcProtoLookup(r, NS_CLASS);

            if (NewtRefIsNIL(klass))
				klass = NSSYM0(frame);
        }
        else
        {
            klass = obj->as.klass;

			if (klass == kNewtSymbolClass)
				klass = NSSYM0(symbol);
            else if (NewtRefIsNIL(klass))
				klass = NewtRefTypeToClass(NewtGetObjectType(obj, true));
        }
    }

    return klass;
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̃N���X�V���{�����Z�b�g����
 *
 * @param r			[in] �I�u�W�F�N�g
 * @param c			[in] �N���X�V���{��
 *
 * @return			�I�u�W�F�N�g
 */

newtRef NewtObjSetClass(newtRefArg r, newtRefArg c)
{
    newtObjRef	obj;

    obj = NewtRefToPointer(r);

    if (obj != NULL)
    {
        if (NewtObjIsFrame(obj))
		{
			if (NewtRefIsReadonly(r))
			{
				NewtThrow(kNErrObjectReadOnly, r);
				return r;
			}

			NewtObjSetSlot(obj, NS_CLASS, c);
        }
		else
		{
            obj->as.klass = c;
		}
    }

    return r;
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̃v���~�e�B�u�N���X���擾
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�v���~�e�B�u�N���X
 */

newtRef NsPrimClassOf(newtRefArg rcvr, newtRefArg r)
{
    newtRefVar	klass;

    if (NewtRefIsPointer(r))
	{
		switch (NewtGetRefType(r, true))
		{
			case kNewtFrame:
				klass = NSSYM0(frame);
				break;

			case kNewtArray:
				klass = NSSYM0(array);
				break;

			default:
				klass = NSSYM0(binary);
				break;
		}
	}
	else
	{
		klass = NSSYM(immediate);
	}

	return klass;
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̃N���X�V���{�����擾
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�N���X�V���{��
 */

newtRef NcClassOf(newtRefArg r)
{
    if (NewtRefIsPointer(r))
        return NewtObjClassOf(r);
    else
		return NewtRefTypeToClass(NewtGetRefType(r, false));
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̃N���X�V���{�����Z�b�g����
 *
 * @param r			[in] �I�u�W�F�N�g
 * @param c			[in] �N���X�V���{��
 *
 * @retval			�I�u�W�F�N�g	�N���X�V���{�����Z�b�g�ł����ꍇ
 * @retval			NIL			�N���X�V���{�����Z�b�g�ł��Ȃ������ꍇ
 */

newtRef NcSetClass(newtRefArg r, newtRefArg c)
{
    if (NewtRefIsPointer(r))
		return NewtObjSetClass(r, c);
    else
        return kNewtRefNIL;;
}


/*------------------------------------------------------------------------*/
/** �Q�Ƃ̔�r
 *
 * @param r1		[in] �Q�ƂP
 * @param r2		[in] �Q�ƂQ
 *
 * @retval			TRUE	���l
 * @retval			NIL		���l�łȂ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NcRefEqual(newtRefArg r1, newtRefArg r2)
{
    return NewtMakeBoolean(NewtRefEqual(r1, r2));
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̔�r
 *
 * @param rcvr		[in] ���V�[�o
 * @param r1		[in] �I�u�W�F�N�g�P
 * @param r2		[in] �I�u�W�F�N�g�Q
 *
 * @retval			TRUE	���l
 * @retval			NIL		���l�łȂ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NsObjectEqual(newtRefArg rcvr, newtRefArg r1, newtRefArg r2)
{
    return NewtMakeBoolean(NewtObjectEqual(r1, r2));
}


/*------------------------------------------------------------------------*/
/** �V���{��������I�ɔ�r�i�啶���������͋�ʂ���Ȃ��j
 *
 * @param rcvr		[in] ���V�[�o
 * @param r1		[in] �V���{���P
 * @param r2		[in] �V���{���Q
 *
 * @retval			���̐���	r1 < r2
 * @retval			0		r1 = r2
 * @retval			���̐���	r1 > r2
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NsSymbolCompareLex(newtRefArg rcvr, newtRefArg r1, newtRefArg r2)
{
	if (! NewtRefIsSymbol(r1))
        return NewtThrow(kNErrNotASymbol, r1);

	if (! NewtRefIsSymbol(r2))
        return NewtThrow(kNErrNotASymbol, r2);

    return NewtMakeInteger(NewtSymbolCompareLex(r1, r2));
}


/*------------------------------------------------------------------------*/
/** sub �� supr �̃T�u�N���X���܂ނ��`�F�b�N����
 *
 * @param rcvr		[in] ���V�[�o
 * @param sub		[in] �V���{���I�u�W�F�N�g�P
 * @param supr		[in] �V���{���I�u�W�F�N�g�Q
 *
 * @retval			TRUE	�T�u�N���X���܂�
 * @retval			NIL		�T�u�N���X���܂܂Ȃ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NsHasSubclass(newtRefArg rcvr, newtRefArg sub, newtRefArg supr)
{
    return NewtMakeBoolean(NewtHasSubclass(sub, supr));
}


/*------------------------------------------------------------------------*/
/** sub �� supr �̃T�u�N���X���`�F�b�N����
 *
 * @param rcvr		[in] ���V�[�o
 * @param sub		[in] �V���{���I�u�W�F�N�g�P
 * @param supr		[in] �V���{���I�u�W�F�N�g�Q
 *
 * @retval			TRUE	�T�u�N���X�ł���
 * @retval			NIL		�T�u�N���X�łȂ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NsIsSubclass(newtRefArg rcvr, newtRefArg sub, newtRefArg supr)
{
    return NewtMakeBoolean(NewtIsSubclass(sub, supr));
}


/*------------------------------------------------------------------------*/
/** obj �� r �̃C���X�^���X���`�F�b�N����
 *
 * @param rcvr		[in] ���V�[�o
 * @param obj		[in] �I�u�W�F�N�g
 * @param r			[in] �N���X�V���{��
 *
 * @retval			TRUE	�C���X�^���X�ł���
 * @retval			NIL		�C���X�^���X�łȂ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NsIsInstance(newtRefArg rcvr, newtRefArg obj, newtRefArg r)
{
    return NewtMakeBoolean(NewtIsInstance(obj, r));
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** r ���z�񂩃`�F�b�N����
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			TRUE	�z��ł���
 * @retval			NIL		�z��łȂ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NsIsArray(newtRefArg rcvr, newtRefArg r)
{
    return NewtMakeBoolean(NewtRefIsArray(r));
}


/*------------------------------------------------------------------------*/
/** r ���t���[�����`�F�b�N����
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			TRUE	�t���[���ł���
 * @retval			NIL		�t���[���łȂ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NsIsFrame(newtRefArg rcvr, newtRefArg r)
{
    return NewtMakeBoolean(NewtRefIsFrame(r));
}


/*------------------------------------------------------------------------*/
/** r ���o�C�i�����`�F�b�N����
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			TRUE	�o�C�i���ł���
 * @retval			NIL		�o�C�i���łȂ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NsIsBinary(newtRefArg rcvr, newtRefArg r)
{
    return NewtMakeBoolean(NewtRefIsBinary(r));
}


/*------------------------------------------------------------------------*/
/** r ���V���{�����`�F�b�N����
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			TRUE	�V���{���ł���
 * @retval			NIL		�V���{���łȂ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NsIsSymbol(newtRefArg rcvr, newtRefArg r)
{
    return NewtMakeBoolean(NewtRefIsSymbol(r));
}


/*------------------------------------------------------------------------*/
/** r �������񂩃`�F�b�N����
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			TRUE	������ł���
 * @retval			NIL		������łȂ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NsIsString(newtRefArg rcvr, newtRefArg r)
{
    return NewtMakeBoolean(NewtRefIsString(r));
}


/*------------------------------------------------------------------------*/
/** r ���������`�F�b�N����
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			TRUE	�����ł���
 * @retval			NIL		�����łȂ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NsIsCharacter(newtRefArg rcvr, newtRefArg r)
{
    return NewtMakeBoolean(NewtRefIsCharacter(r));
}


/*------------------------------------------------------------------------*/
/** r ���������`�F�b�N����
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			TRUE	�����ł���
 * @retval			NIL		�����łȂ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NsIsInteger(newtRefArg rcvr, newtRefArg r)
{
    return NewtMakeBoolean(NewtRefIsInteger(r));
}


/*------------------------------------------------------------------------*/
/** r �����������_�����`�F�b�N����
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			TRUE	���������_���ł���
 * @retval			NIL		���������_���łȂ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NsIsReal(newtRefArg rcvr, newtRefArg r)
{
    return NewtMakeBoolean(NewtRefIsReal(r));
}


/*------------------------------------------------------------------------*/
/** r �����l�f�[�^���`�F�b�N����
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			TRUE	���l�f�[�^�ł���
 * @retval			NIL		���l�f�[�^�łȂ�
 */

newtRef NsIsNumber(newtRefArg rcvr, newtRefArg r)
{
    return NewtMakeBoolean(NewtRefIsInteger(r) || NewtRefIsReal(r));
}


/*------------------------------------------------------------------------*/
/** r ���C�~�f�B�G�C�g�i���l�j���`�F�b�N����
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			TRUE	�C�~�f�B�G�C�g�ł���
 * @retval			NIL		�C�~�f�B�G�C�g�łȂ�
 */

newtRef NsIsImmediate(newtRefArg rcvr, newtRefArg r)
{
    return NewtMakeBoolean(NewtRefIsImmediate(r));
}


/*------------------------------------------------------------------------*/
/** r ���֐��I�u�W�F�N�g���`�F�b�N����
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			TRUE	�֐��I�u�W�F�N�g�ł���
 * @retval			NIL		�֐��I�u�W�F�N�g�łȂ�
 */

newtRef NsIsFunction(newtRefArg rcvr, newtRefArg r)
{
    return NewtMakeBoolean(NewtRefIsFunction(r));
}


/*------------------------------------------------------------------------*/
/** r �����[�h�I�����[���`�F�b�N����
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			TRUE	���[�h�I�����[�ł���
 * @retval			NIL		���[�h�I�����[�łȂ�
 */

newtRef NsIsReadonly(newtRefArg rcvr, newtRefArg r)
{
    return NewtMakeBoolean(NewtRefIsReadonly(r));
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �z��I�u�W�F�N�g�ɒl��ǉ�����
 *
 * @param r			[in] �z��I�u�W�F�N�g
 * @param v			[in] �l�I�u�W�F�N�g
 *
 * @return			�l�I�u�W�F�N�g
 */

newtRef NcAddArraySlot(newtRefArg r, newtRefArg v)
{
    newtObjRef	obj;

    obj = NewtRefToPointer(r);

    if (obj != NULL)
		NewtObjAddArraySlot(obj, v);

    return v;
}


/*------------------------------------------------------------------------*/
/** �z��I�u�W�F�N�g�̗v�f�𕶎���ɍ�������
 *
 * @param r			[in] �z��I�u�W�F�N�g
 *
 * @return			������I�u�W�F�N�g
 */

newtRef NcStringer(newtRefArg r)
{
    newtRef *	slots;
    newtRefVar	str;
    uint32_t	len;
    uint32_t	i;

    if (! NewtRefIsArray(r))
        return NewtThrow(kNErrNotAnArray, r);

    str = NSSTR("");

    len = NewtArrayLength(r);
    slots = NewtRefToSlots(r);

    for (i = 0; i < len; i++)
    {
        NcStrCat(str, slots[i]);
    }

    return str;
}


/*------------------------------------------------------------------------*/
/** ������I�u�W�F�N�g�̍Ō�ɃI�u�W�F�N�g�𕶎��񉻂��Ēǉ�����
 *
 * @param rcvr		[in] ���V�[�o
 * @param str		[in] ������I�u�W�F�N�g
 * @param v			[in] �I�u�W�F�N�g
 *
 * @return			������I�u�W�F�N�g
 */

newtRef NsStrCat(newtRefArg rcvr, newtRefArg str, newtRefArg v)
{
	char	wk[32];
    char *	s = NULL;

    switch (NewtGetRefType(v, true))
    {
        case kNewtInt30:
        case kNewtInt32:
            {
                int	n;

                n = (int)NewtRefToInteger(v);
                sprintf(wk, "%d", n);
                s = wk;
            }
            break;

        case kNewtReal:
            {
                double	n;

                n = NewtRefToReal(v);
                sprintf(wk, "%f", n);
                s = wk;
            }
            break;

        case kNewtCharacter:
			{
				int		c;

				c = NewtRefToCharacter(v);
                sprintf(wk, "%c", c);
                s = wk;
			}
            break;

        case kNewtSymbol:
            {
                newtSymDataRef	sym;

                sym = NewtRefToSymbol(v);
                s = sym->name;
            }
            break;

        case kNewtString:
            s = NewtRefToString(v);
            break;
    }

    if (s != NULL)
        NewtStrCat(str, s);

    return str;
}


/*------------------------------------------------------------------------*/
/** ������I�u�W�F�N�g����V���{�����쐬����
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] ������I�u�W�F�N�g
 *
 * @return			�V���{���I�u�W�F�N�g
 */

newtRef NsMakeSymbol(newtRefArg rcvr, newtRefArg r)
{
    char *	s;

    if (! NewtRefIsString(r))
        return NewtThrow(kNErrNotAString, r);

    s = NewtRefToString(r);

    return NewtMakeSymbol(s);
}


/*------------------------------------------------------------------------*/
/** �t���[���I�u�W�F�N�g���쐬����
 *
 * @param rcvr		[in] ���V�[�o
 *
 * @return			�t���[���I�u�W�F�N�g
 */

newtRef NsMakeFrame(newtRefArg rcvr)
{
	return NewtMakeFrame(kNewtRefUnbind, 0);
}


/*------------------------------------------------------------------------*/
/** �o�C�i���I�u�W�F�N�g���쐬����
 *
 * @param rcvr		[in] ���V�[�o
 * @param length	[in] ����
 * @param klass		[in] �N���X
 *
 * @return			�o�C�i���I�u�W�F�N�g
 */

newtRef NsMakeBinary(newtRefArg rcvr, newtRefArg length, newtRefArg klass)
{
    if (! NewtRefIsInteger(length))
        return NewtThrow(kNErrNotAnInteger, length);

	return NewtMakeBinary(klass, NULL, NewtRefToInteger(length), false);
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �����̃r�b�gAND
 *
 * @param r1		[in] �����I�u�W�F�N�g�P
 * @param r2		[in] �����I�u�W�F�N�g�Q
 *
 * @return			���l�I�u�W�F�N�g
 */

newtRef NcBAnd(newtRefArg r1, newtRefArg r2)
{
    if (! NewtRefIsInteger(r1))
        return NewtThrow(kNErrNotAnInteger, r1);

    if (! NewtRefIsInteger(r2))
        return NewtThrow(kNErrNotAnInteger, r2);

    return (r1 & r2);
}


/*------------------------------------------------------------------------*/
/** �����̃r�b�gOR
 *
 * @param r1		[in] �����I�u�W�F�N�g�P
 * @param r2		[in] �����I�u�W�F�N�g�Q
 *
 * @return			���l�I�u�W�F�N�g
 */

newtRef NcBOr(newtRefArg r1, newtRefArg r2)
{
    if (! NewtRefIsInteger(r1))
        return NewtThrow(kNErrNotAnInteger, r1);

    if (! NewtRefIsInteger(r2))
        return NewtThrow(kNErrNotAnInteger, r2);

    return (r1 | r2);
}


/*------------------------------------------------------------------------*/
/** �����̃r�b�gNOT
 *
 * @param r		[in] �����I�u�W�F�N�g
 *
 * @return			���l�I�u�W�F�N�g
 */

newtRef NcBNot(newtRefArg r)
{
    if (! NewtRefIsInteger(r))
        return NewtThrow(kNErrNotAnInteger, r);

    return NewtMakeInteger(~ NewtRefToInteger(r));
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �u�[�����Z AND
 *
 * @param rcvr		[in] ���V�[�o
 * @param r1		[in] �I�u�W�F�N�g�P
 * @param r2		[in] �I�u�W�F�N�g�Q
 *
 * @retval			TRUE
 * @retval			NIL
 */

newtRef NsAnd(newtRefArg rcvr, newtRefArg r1, newtRefArg r2)
{
	bool	result;

	result = (NewtRefIsNotNIL(r1) && NewtRefIsNotNIL(r2));

    return NewtMakeBoolean(result);
}


/*------------------------------------------------------------------------*/
/** �u�[�����Z OR
 *
 * @param rcvr		[in] ���V�[�o
 * @param r1		[in] �I�u�W�F�N�g�P
 * @param r2		[in] �I�u�W�F�N�g�Q
 *
 * @retval			TRUE
 * @retval			NIL
 */

newtRef NsOr(newtRefArg rcvr, newtRefArg r1, newtRefArg r2)
{
	bool	result;

	result = (NewtRefIsNotNIL(r1) || NewtRefIsNotNIL(r2));

    return NewtMakeBoolean(result);
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** ���l�����̃`�F�b�N
 *
 * @param r1		[in] �I�u�W�F�N�g�P
 * @param r2		[in] �I�u�W�F�N�g�Q
 * @param real		[out]���������_���܂�
 *
 * @retval			true	���������l
 * @retval			false   ���������l�łȂ�
 */


bool NewtArgsIsNumber(newtRefArg r1, newtRefArg r2, bool * real)
{
    *real = false;

    if (NewtRefIsReal(r1))
    {
        *real = true;
    }
    else
    {
        if (! NewtRefIsInteger(r1))
            return false;
    }

    if (NewtRefIsReal(r2))
    {
        *real = true;
    }
    else
    {
        if (! NewtRefIsInteger(r2))
            return false;
    }

    return true;
}


/*------------------------------------------------------------------------*/
/** ���Z(r1 + r2)
 *
 * @param r1		[in] ���l�I�u�W�F�N�g�P
 * @param r2		[in] ���l�I�u�W�F�N�g�Q
 *
 * @return			���l�I�u�W�F�N�g
 */

newtRef NcAdd(newtRefArg r1, newtRefArg r2)
{
    bool	real;

    if (! NewtArgsIsNumber(r1, r2, &real))
		return NewtThrow0(kNErrNotANumber);

    if (real)
    {
        double real1;
        double real2;

        real1 = NewtRefToReal(r1);
        real2 = NewtRefToReal(r2);

        return NewtMakeReal(real1 + real2);
    }
    else
    {
        int32_t	int1;
        int32_t	int2;

        int1 = NewtRefToInteger(r1);
        int2 = NewtRefToInteger(r2);

        return NewtMakeInteger(int1 + int2);
    }
}


/*------------------------------------------------------------------------*/
/** ���Z(r1 - r2)
 *
 * @param r1		[in] ���l�I�u�W�F�N�g�P
 * @param r2		[in] ���l�I�u�W�F�N�g�Q
 *
 * @return			���l�I�u�W�F�N�g
 */

newtRef NcSubtract(newtRefArg r1, newtRefArg r2)
{
    bool	real;

    if (! NewtArgsIsNumber(r1, r2, &real))
		return NewtThrow0(kNErrNotANumber);

    if (real)
    {
        double real1;
        double real2;

        real1 = NewtRefToReal(r1);
        real2 = NewtRefToReal(r2);

        return NewtMakeReal(real1 - real2);
    }
    else
    {
        int32_t	int1;
        int32_t	int2;

        int1 = NewtRefToInteger(r1);
        int2 = NewtRefToInteger(r2);

        return NewtMakeInteger(int1 - int2);
    }
}


/*------------------------------------------------------------------------*/
/** ��Z(r1 x r2)
 *
 * @param r1		[in] ���l�I�u�W�F�N�g�P
 * @param r2		[in] ���l�I�u�W�F�N�g�Q
 *
 * @return			���l�I�u�W�F�N�g
 */

newtRef NcMultiply(newtRefArg r1, newtRefArg r2)
{
    bool	real;

    if (! NewtArgsIsNumber(r1, r2, &real))
		return NewtThrow0(kNErrNotANumber);

    if (real)
    {
        double real1;
        double real2;

        real1 = NewtRefToReal(r1);
        real2 = NewtRefToReal(r2);

        return NewtMakeReal(real1 * real2);
    }
    else
    {
        int32_t	int1;
        int32_t	int2;

        int1 = NewtRefToInteger(r1);
        int2 = NewtRefToInteger(r2);

        return NewtMakeInteger(int1 * int2);
    }
}


/*------------------------------------------------------------------------*/
/** ���Z(r1 / r2)
 *
 * @param r1		[in] ���l�I�u�W�F�N�g�P
 * @param r2		[in] ���l�I�u�W�F�N�g�Q
 *
 * @return			���l�I�u�W�F�N�g
 */

newtRef NcDivide(newtRefArg r1, newtRefArg r2)
{
    bool	real;

    if (! NewtArgsIsNumber(r1, r2, &real))
		return NewtThrow0(kNErrNotANumber);

    if (real)
    {
        double real1;
        double real2;

        real1 = NewtRefToReal(r1);
        real2 = NewtRefToReal(r2);

        if (real2 == 0.0)
            return NewtThrow(kNErrDiv0, r2);

        return NewtMakeReal(real1 / real2);
    }
    else
    {
        int32_t	int1;
        int32_t	int2;

        int1 = NewtRefToInteger(r1);
        int2 = NewtRefToInteger(r2);

        if (int2 == 0)
            return NewtThrow(kNErrDiv0, r2);

        return NewtMakeInteger(int1 / int2);
    }
}


/*------------------------------------------------------------------------*/
/** �����̊��Z(r1 / r2)
 *
 * @param r1		[in] �����I�u�W�F�N�g�P
 * @param r2		[in] �����I�u�W�F�N�g�Q
 *
 * @return			�����I�u�W�F�N�g
 */

newtRef NcDiv(newtRefArg r1, newtRefArg r2)
{
    if (! NewtRefIsInteger(r1))
        return NewtThrow(kNErrNotAnInteger, r1);

    if (! NewtRefIsInteger(r2))
        return NewtThrow(kNErrNotAnInteger, r2);

    return NewtMakeInteger(NewtRefToInteger(r1) / NewtRefToInteger(r2));
}


/*------------------------------------------------------------------------*/
/** r1 �� r2 �Ŋ������̗]����v�Z
 *
 * @param rcvr		[in] ���V�[�o
 * @param r1		[in] �����I�u�W�F�N�g�P
 * @param r2		[in] �����I�u�W�F�N�g�Q
 *
 * @return			�����I�u�W�F�N�g
 */

newtRef NsMod(newtRefArg rcvr, newtRefArg r1, newtRefArg r2)
{
    if (! NewtRefIsInteger(r1))
        return NewtThrow(kNErrNotAnInteger, r1);

    if (! NewtRefIsInteger(r2))
        return NewtThrow(kNErrNotAnInteger, r2);

    return NewtMakeInteger(NewtRefToInteger(r1) % NewtRefToInteger(r2));
}


/*------------------------------------------------------------------------*/
/** �r�b�g�V�t�g(r1 << r2)
 *
 * @param rcvr		[in] ���V�[�o
 * @param r1		[in] �����I�u�W�F�N�g�P
 * @param r2		[in] �����I�u�W�F�N�g�Q
 *
 * @return			�����I�u�W�F�N�g
 */

newtRef NsShiftLeft(newtRefArg rcvr, newtRefArg r1, newtRefArg r2)
{
    if (! NewtRefIsInteger(r1))
        return NewtThrow(kNErrNotAnInteger, r1);

    if (! NewtRefIsInteger(r2))
        return NewtThrow(kNErrNotAnInteger, r2);

    return NewtMakeInteger(NewtRefToInteger(r1) << NewtRefToInteger(r2));
}


/*------------------------------------------------------------------------*/
/** �r�b�g�V�t�g(r1 >> r2)
 *
 * @param rcvr		[in] ���V�[�o
 * @param r1		[in] �����I�u�W�F�N�g�P
 * @param r2		[in] �����I�u�W�F�N�g�Q
 *
 * @return			�����I�u�W�F�N�g
 */

newtRef NsShiftRight(newtRefArg rcvr, newtRefArg r1, newtRefArg r2)
{
    if (! NewtRefIsInteger(r1))
        return NewtThrow(kNErrNotAnInteger, r1);

    if (! NewtRefIsInteger(r2))
        return NewtThrow(kNErrNotAnInteger, r2);

    return NewtMakeInteger(NewtRefToInteger(r1) >> NewtRefToInteger(r2));
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̑召��r(r1 < r2)
 *
 * @param r1		[in] �I�u�W�F�N�g�P
 * @param r2		[in] �I�u�W�F�N�g�Q
 *
 * @retval			TRUE
 * @retval			NIL
 *
 * @note			�X�N���v�g�̌ďo���p
 */

newtRef NcLessThan(newtRefArg r1, newtRefArg r2)
{
    return NewtMakeBoolean(NewtObjectCompare(r1, r2) < 0);
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̑召��r(r1 > r2)
 *
 * @param r1		[in] �I�u�W�F�N�g�P
 * @param r2		[in] �I�u�W�F�N�g�Q
 *
 * @retval			TRUE
 * @retval			NIL
 *
 * @note			�X�N���v�g�̌ďo���p
 */

newtRef NcGreaterThan(newtRefArg r1, newtRefArg r2)
{
    return NewtMakeBoolean(NewtObjectCompare(r1, r2) > 0);
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̑召��r(r1 >= r2)
 *
 * @param r1		[in] �I�u�W�F�N�g�P
 * @param r2		[in] �I�u�W�F�N�g�Q
 *
 * @retval			TRUE
 * @retval			NIL
 *
 * @note			�X�N���v�g�̌ďo���p
 */

newtRef NcGreaterOrEqual(newtRefArg r1, newtRefArg r2)
{
    return NewtMakeBoolean(NewtObjectCompare(r1, r2) >= 0);
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̑召��r(r1 <= r2)
 *
 * @param r1		[in] �I�u�W�F�N�g�P
 * @param r2		[in] �I�u�W�F�N�g�Q
 *
 * @retval			TRUE
 * @retval			NIL
 *
 * @note			�X�N���v�g�̌ďo���p
 */

newtRef NcLessOrEqual(newtRefArg r1, newtRefArg r2)
{
    return NewtMakeBoolean(NewtObjectCompare(r1, r2) <= 0);
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** ���K�\���I�u�W�F�N�g�i�t���[���j�̐���
 *
 * @param rcvr		[in] ���V�[�o
 *
 * @return			��O�t���[��
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NsCurrentException(newtRefArg rcvr)
{
	return NVMCurrentException();
}


#ifdef __NAMED_MAGIC_POINTER__
/*------------------------------------------------------------------------*/
/** ���K�\���I�u�W�F�N�g�i�t���[���j�̐���
 *
 * @param rcvr		[in] ���V�[�o
 * @param pattern	[in] �p�^�[��������
 * @param opt		[in] �I�v�V����������
 *
 * @return			NIL
 */

newtRef NsMakeRegex(newtRefArg rcvr, newtRefArg pattern, newtRefArg opt)
{
    newtRefVar	v[] = {
                            NSSYM0(_proto),		NSNAMEDMP0(protoREGEX),
                            NSSYM0(pattern),	pattern,
                            NSSYM0(option),		opt,
                        };

	return NewtMakeFrame2(sizeof(v) / (sizeof(newtRefVar) * 2), v);
}

#endif /* __NAMED_MAGIC_POINTER__ */


#pragma mark -
/*------------------------------------------------------------------------*/
/** �W���o�͂ɃI�u�W�F�N�g���v�����g
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			NIL
 *
 * @note			�O���[�o���֐��p
 */

newtRef NsPrintObject(newtRefArg rcvr, newtRefArg r)
{
    NewtPrintObject(stdout, r);
    return kNewtRefNIL;
}


/*------------------------------------------------------------------------*/
/** �W���o�͂ɃI�u�W�F�N�g���v�����g
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			NIL
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NsPrint(newtRefArg rcvr, newtRefArg r)
{
    NewtPrint(stdout, r);
    return kNewtRefNIL;
}


/*------------------------------------------------------------------------*/
/** �W���o�͂Ɋ֐��I�u�W�F�N�g���_���v�o��
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �֐��I�u�W�F�N�g
 *
 * @return			NIL
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NsDumpFn(newtRefArg rcvr, newtRefArg r)
{
    NVMDumpFn(stdout, r);
    return kNewtRefNIL;
}


/*------------------------------------------------------------------------*/
/** �W���o�͂Ƀo�C�g�R�[�h���_���v�o��
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �o�C�g�R�[�h
 *
 * @return			NIL
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NsDumpBC(newtRefArg rcvr, newtRefArg r)
{
    NVMDumpBC(stdout, r);
    return kNewtRefNIL;
}


/*------------------------------------------------------------------------*/
/** �W���o�͂ɃX�^�b�N���_���v�o��
 *
 * @param rcvr		[in] ���V�[�o
 *
 * @return			NIL
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NsDumpStacks(newtRefArg rcvr)
{
    NVMDumpStacks(stdout);
    return kNewtRefNIL;
}


/*------------------------------------------------------------------------*/
/** �W���o�͂Ɋ֐�����\��
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			NIL
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NsInfo(newtRefArg rcvr, newtRefArg r)
{
    NewtInfo(r);
    return kNewtRefNIL;
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** ������I�u�W�F�N�g���R���p�C��
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] ������I�u�W�F�N�g
 *
 * @return			�֐��I�u�W�F�N�g
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NsCompile(newtRefArg rcvr, newtRefArg r)
{
    if (! NewtRefIsString(r))
        return NewtThrow(kNErrNotAString, r);

    return NBCCompileStr(NewtRefToString(r), true);
}


/*------------------------------------------------------------------------*/
/** ���ϐ��̎擾
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] ������I�u�W�F�N�g
 *
 * @return			������I�u�W�F�N�g
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NsGetEnv(newtRefArg rcvr, newtRefArg r)
{
    if (! NewtRefIsString(r))
        return NewtThrow(kNErrNotAString, r);

    return NewtGetEnv(NewtRefToString(r));
}
