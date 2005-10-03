/*------------------------------------------------------------------------*/
/**
 * @file	NewtObj.c
 * @brief   �I�u�W�F�N�g�V�X�e��
 *
 * @author  M.Nukui
 * @date	2003-11-07
 *
 * Copyright (C) 2003-2004 M.Nukui All rights reserved.
 */


/* �w�b�_�t�@�C�� */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "NewtCore.h"
#include "NewtGC.h"
#include "NewtIO.h"


/* �֐��v���g�^�C�v */
static newtRef		NewtMakeSymbol0(const char *s);
static bool			NewtBSearchSymTable(newtRefArg r, const char * name, uint32_t hash, int32_t st, int32_t * indexP);
static newtObjRef   NewtObjMemAlloc(newtPool pool, uint32_t n, bool literal);
static newtObjRef   NewtObjRealloc(newtPool pool, newtObjRef obj, uint32_t n);
static void			NewtGetObjData(newtRefArg r, uint8_t * data, uint32_t len);
static newtObjRef   NewtObjBinarySetLength(newtObjRef obj, uint32_t n);
static uint32_t		NewtObjSymbolLength(newtObjRef obj);
static uint32_t		NewtObjStringLength(newtObjRef obj);
static newtObjRef   NewtObjStringSetLength(newtObjRef obj, uint32_t n);
static void			NewtMakeInitSlots(newtRefArg r, uint32_t st, uint32_t n, uint32_t step, newtRefVar v[]);
static newtObjRef   NewtObjSlotsSetLength(newtObjRef obj, uint32_t n, newtRefArg v);
static int			NewtInt32Compare(newtRefArg r1, newtRefArg r2);
static int			NewtRealCompare(newtRefArg r1, newtRefArg r2);
static int			NewtStringCompare(newtRefArg r1, newtRefArg r2);
static int			NewtBinaryCompare(newtRefArg r1, newtRefArg r2);
static uint16_t		NewtArgsType(newtRefArg r1, newtRefArg r2);

static newtRef		NewtMakeThrowSymbol(int32_t err);

static bool			NewtObjHasProto(newtObjRef obj);
static bool			NewtMapIsSorted(newtRefArg r);
static void			NewtObjRemoveArraySlot(newtObjRef obj, int32_t n);
static void			NewtDeeplyCopyMap(newtRef * dst, int32_t * pos, newtRefArg src);
static newtRef		NewtDeeplyCloneMap(newtRefArg map, int32_t len);
static void			NewtObjRemoveFrameSlot(newtObjRef obj, newtRefArg slot);
static bool			NewtStrNBeginsWith(char * str, uint32_t len, char * sub, uint32_t sublen);
static bool			NewtStrIsSubclass(char * sub, uint32_t sublen, char * supr, uint32_t suprlen);
static bool			NewtStrHasSubclass(char * sub, uint32_t sublen, char * supr, uint32_t suprlen);


#pragma mark -
/*------------------------------------------------------------------------*/
/** �V���{���̃n�b�V���l���v�Z
 *
 * @param name		[in] �V���{����
 *
 * @return			�n�b�V���l
 */

uint32_t NewtSymbolHashFunction(const char * name)
{
    uint32_t result = 0;
    char c;

    while (*name)
    {
        c = *name;

        if (c >= 'a' && c <= 'z')
            result = result + c - ('a' - 'A');
        else
            result = result + c;

        name++;
    }

    return result * 2654435769U;
}


/*------------------------------------------------------------------------*/
/** �V���{���I�u�W�F�N�g�̍쐬
 *
 * @param s			[in] ������
 *
 * @return			�V���{���I�u�W�F�N�g
 */

newtRef NewtMakeSymbol0(const char *s)
{
    newtObjRef	obj;
    uint32_t	size;

    size = sizeof(uint32_t) + strlen(s) + 1;
    obj = NewtObjAlloc(kNewtSymbolClass, size, 0, true);

    if (obj != NULL)
    {
        newtSymDataRef	objData;
        uint32_t	setlen;

        objData = NewtObjToSymbol(obj);

        setlen = NewtObjSize(obj) - size;

        if (0 < setlen)
            memset(objData + size, 0, setlen);

        objData->hash = NewtSymbolHashFunction(s);
        strcpy(objData->name, s);

        return NewtMakePointer(obj);
    }

    return kNewtRefUnbind;
}


/*------------------------------------------------------------------------*/
/** �V���{���e�[�u���̈ʒu����
 *
 * @param r			[in] �V���{���e�[�u��
 * @param name		[in] �V���{��������
 * @param hash		[in] �n�b�V���l
 * @param st		[in] �J�n�ʒu
 * @param indexP	[out]�ʒu
 *
 * @retval			true	����
 * @retval			false   ���s
 */

bool NewtBSearchSymTable(newtRefArg r, const char * name, uint32_t hash,
    int32_t st, int32_t * indexP)
{
    newtSymDataRef	sym;
    newtRef *	slots;
    int32_t	len;
    int32_t	ed;
    int32_t	md = st;
    int16_t	comp;

    slots = NewtRefToSlots(r);

    if (hash == 0)
        hash = NewtSymbolHashFunction(name);

    len = NewtArrayLength(r);
    ed = len - 1;

    while (st <= ed)
    {
        md = (st + ed) / 2;

        sym = NewtRefToSymbol(slots[md]);

		if (hash < sym->hash)
			comp = -1;
		else if (hash > sym->hash)
			comp = 1;
		else
			comp = 0;

        if (comp == 0)
            comp = strcasecmp(name, sym->name);

        if (comp == 0)
        {
            *indexP = md;
            return true;
        }

        if (comp < 0)
            ed = md - 1;
        else
            st = md + 1;
    }

    if (len < st)
        *indexP = len;
    else
        *indexP = st;

    return false;
}


/*------------------------------------------------------------------------*/
/** �V���{���̃��b�N�A�b�v
 *
 * @param r			[in] �V���{���e�[�u��
 * @param name		[in] �V���{��������
 * @param hash		[in] �n�b�V���l
 * @param st		[in] �J�n�ʒu
 *
 * @return			�V���{���I�u�W�F�N�g
 *
 * @note			���o�^�̏ꍇ�̓V���{���I�u�W�F�N�g���쐬���V���{���e�[�u���ɓo�^����
 */

newtRef NewtLookupSymbol(newtRefArg r, const char * name, uint32_t hash, int32_t st)
{
    newtRefVar	sym;
    int32_t	index;

    if (NewtBSearchSymTable(r, name, 0, st, &index))
        return NewtGetArraySlot(r, index);

    sym = NewtMakeSymbol0(name);
    NewtInsertArraySlot(r, index, sym);

    return sym;
}


/*------------------------------------------------------------------------*/
/** �V���{���̃��b�N�A�b�v
 *
 * @param r			[in] �V���{���e�[�u��
 * @param name		[in] �V���{��������
 * @param st		[in] �J�n�ʒu
 *
 * @return			�V���{���I�u�W�F�N�g
 */

newtRef NewtLookupSymbolArray(newtRefArg r, newtRefArg name, int32_t st)
{
    newtSymDataRef	sym;

    sym = NewtRefToSymbol(name);

    if (sym != NULL)
        return NewtLookupSymbol(r, sym->name, sym->hash, st);
    else
        return kNewtRefUnbind;
}


/**
 * Return the ASCII string associated with a symbol ref.
 * Remark: doesn't check that the passed object is indeed a symbol.
 *
 * @param inSymbol	symbol object
 * @return a pointer to the name of the symbol
 */
 
const char*	NewtSymbolGetName(newtRefArg inSymbol)
{
	return NewtRefToSymbol(inSymbol)->name;
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̃I�u�W�F�N�g�^�C�v�̎擾
 *
 * @param r			[in] �I�u�W�F�N�g
 * @param detail	[in] ���e�����t���O
 *
 * @return			�I�u�W�F�N�g�^�C�v
 */

uint16_t NewtGetRefType(newtRefArg r, bool detail)
{
    uint16_t	type = kNewtUnknownType;

    switch (r & 3)
    {
        case 0:	// Integer
            type = kNewtInt30;
            break;

        case 1:	// Pointer
            if (detail)
                type = NewtGetObjectType(NewtRefToPointer(r), true);
            else
                type = kNewtPointer;
            break;

        case 2:	// Character or Special
            switch (r)
            {
                case kNewtRefNIL:
                    type = kNewtNil;
                    break;

                case kNewtRefTRUE:
                    type = kNewtTrue;
                    break;

                case kNewtRefUnbind:
                    type = kNewtUnbind;
                    break;

                case kNewtSymbolClass:
                    type = kNewtSymbol;
                    break;

                default:
                    if ((r & 0xF) == 6)
                        type = kNewtCharacter;
                    else
                        type = kNewtSpecial;
                    break;
            }
            break;

        case 3:	// Magic Pointer
            type = kNewtMagicPointer;
            break;
    }

    return type;
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�f�[�^�̃I�u�W�F�N�g�^�C�v�̎擾
 *
 * @param obj		[in] �I�u�W�F�N�g�f�[�^
 * @param detail	[in] �f�B�e�C���t���O
 *
 * @return			�I�u�W�F�N�g�^�C�v
 */

uint16_t NewtGetObjectType(newtObjRef obj, bool detail)
{
    uint16_t	type = kNewtUnknownType;

    switch (NewtObjType(obj))
    {
        case 0:	// binary
            type = kNewtBinary;

            if (detail)
            {
                if (obj->as.klass == kNewtSymbolClass)
                    type = kNewtSymbol;
                else if (NewtRefEqual(obj->as.klass, NSSYM0(string)))
                    type = kNewtString;
                else if (NewtRefEqual(obj->as.klass, NSSYM0(int32)))
                    type = kNewtInt32;
                else if (NewtRefEqual(obj->as.klass, NSSYM0(real)))
                    type = kNewtReal;
				else if (NewtIsSubclass(obj->as.klass, NSSYM0(string)))
                    type = kNewtString;
            }
            break;

        case 1:	// array
            type = kNewtArray;
            break;

        case 3:	// frame
            type = kNewtFrame;
            break;
    }

    return type;
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�f�[�^�̎��f�[�^�T�C�Y���v�Z
 *
 * @param n			[in] �f�[�^�T�C�Y
 *
 * @return			���f�[�^�T�C�Y
 */

uint32_t NewtObjCalcDataSize(uint32_t n)
{
    if (n < 4)
        return 4;
    else
        return n;
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�f�[�^�̃������m��
 *
 * @param pool		[in] �������v�[��
 * @param n			[in] �f�[�^�T�C�Y
 * @param literal	[in] ���e�����t���O
 *
 * @return			�I�u�W�F�N�g�f�[�^
 */

newtObjRef NewtObjMemAlloc(newtPool pool, uint32_t n, bool literal)
{
    newtObjRef	obj;
    uint32_t	newSize;

    if (literal)
    {
        newSize = NewtAlign(sizeof(newtObj) + n, 4);
        obj = NewtObjChainAlloc(pool, newSize, 0);
    }
    else
    {
        newSize = NewtObjCalcDataSize(n);
        obj = NewtObjChainAlloc(pool, sizeof(newtObj) + sizeof(uint8_t *), newSize);
    }

    return obj;
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̃������m��
 *
 * @param r			[in] �N���X�^�}�b�v
 * @param n			[in] �T�C�Y
 * @param type		[in] �I�u�W�F�N�g�^�C�v
 * @param literal	[in] ���e�����t���O
 *
 * @return			�I�u�W�F�N�g
 */

newtObjRef NewtObjAlloc(newtRefArg r, uint32_t n, uint16_t type, bool literal)
{
    newtObjRef	obj;

    obj = NewtObjMemAlloc(NEWT_POOL, n, literal);
    if (obj == NULL) return NULL;

    obj->header.h |= (n << 8) | type;

    if (NEWT_SWEEP)
        obj->header.h |= kNewtObjSweep;

    if ((type & kNewtObjFrame) != 0)
        obj->as.map = r;
    else
        obj->as.klass = r;

    return obj;
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�f�[�^�̃������Ċm��
 *
 * @param pool		[in] �������v�[��
 * @param obj		[in] �I�u�W�F�N�g�f�[�^
 * @param n			[in] �T�C�Y
 *
 * @return			�I�u�W�F�N�g�f�[�^
 */

newtObjRef NewtObjRealloc(newtPool pool, newtObjRef obj, uint32_t n)
{
    uint8_t **	datap;
    uint8_t *	data;
    int32_t	oldSize;
    int32_t	newSize;
    int32_t	addSize;

    oldSize = NewtObjCalcDataSize(NewtObjSize(obj));
    newSize = NewtObjCalcDataSize(n);
    addSize = newSize - oldSize;

    if (0 < addSize)
        NewtCheckGC(pool, addSize);

    datap = (uint8_t **)(obj + 1);
    data = NewtMemRealloc(pool, *datap, newSize);
    if (data == NULL) return NULL;

    pool->usesize += addSize;

    if (data != *datap)
        *datap = data;

    obj->header.h = ((n << 8) | (obj->header.h & 0xff));

    return obj;
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�f�[�^�̃T�C�Y�ύX
 *
 * @param obj		[in] �I�u�W�F�N�g�f�[�^
 * @param n			[in] �T�C�Y
 *
 * @return			�I�u�W�F�N�g�f�[�^
 */

newtObjRef NewtObjResize(newtObjRef obj, uint32_t n)
{
    if (NewtObjIsReadonly(obj))
    {
        NewtThrow0(kNErrObjectReadOnly);
        return NULL;
    }

    return NewtObjRealloc(NEWT_POOL, obj, n);
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�f�[�^�̃f�[�^�����擾
 *
 * @param obj		[in] �I�u�W�F�N�g�f�[�^
 *
 * @return			�f�[�^��
 */

void * NewtObjData(newtObjRef obj)
{
    void *	data;

    data = (void *)(obj + 1);

    if (NewtObjIsLiteral(obj))
        return data;
    else
        return *((void **)data);
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�̃N���[������
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�N���[���������ꂽ�I�u�W�F�N�g
 */

newtRef NewtObjClone(newtRefArg r)
{
    newtObjRef	obj;

    obj = NewtRefToPointer(r);

    if (obj != NULL)
    {
        newtObjRef	newObj = NULL;
        uint32_t	size;
        uint16_t	type;

        size = NewtObjSize(obj);
        type = NewtObjType(obj);

        switch (NewtGetObjectType(obj, true))
        {
            case kNewtSymbol:
            case kNewtReal:
                return r;

            case kNewtFrame:
                {
                    newtRefVar	map;

                    if (NewtRefIsLiteral(obj->as.map))
                        map = obj->as.map;
                    else
                        map = NcClone(obj->as.map);

                    newObj = NewtObjAlloc(map, size, type, false);
                }
                break;

            default:
                newObj = NewtObjAlloc(obj->as.klass, size, type, false);
                break;
        }

        if (newObj != NULL)
        {
            uint8_t *	src;
            uint8_t *	dst;

            src = NewtObjToBinary(obj);
            dst = NewtObjToBinary(newObj);
            memcpy(dst, src, size);

            return NewtMakePointer(newObj);
        }
    }

    return (newtRef)r;
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�̃��e������
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			���e���������ꂽ�I�u�W�F�N�g
 */

newtRef NewtPackLiteral(newtRefArg r)
{
    newtObjRef	obj;

    if (NewtRefIsLiteral(r))
        return r;

    obj = NewtRefToPointer(r);

    if (obj != NULL)
    {
        newtObjRef	newObj = NULL;
        uint32_t	size;
        uint16_t	type;

        size = NewtObjSize(obj);
        type = NewtObjType(obj);

        if (NewtObjIsFrame(obj))
        {
            obj->as.map = NewtPackLiteral(obj->as.map);
            newObj = NewtObjAlloc(obj->as.map, size, type, true);
		}
        else
        {
            newObj = NewtObjAlloc(obj->as.klass, size, type, true);
        }

        if (newObj != NULL)
        {
            uint8_t *	src;
            uint8_t *	dst;

            src = NewtObjToBinary(obj);
            dst = NewtObjToBinary(newObj);
            memcpy(dst, src, size);

            if (NewtObjIsSlotted(newObj))
            {
                newtRef *	slots;
                uint32_t	len;
                uint32_t	i;

                len = NewtObjSlotsLength(newObj);
                slots = NewtObjToSlots(newObj);

                for (i = 0; i < len; i++)
                {
                    slots[i] = NewtPackLiteral(slots[i]);
                }
            }

            newObj->header.h |= kNewtObjLiteral;

            // obj �� free ���Ă͂����Ȃ�
            // GC �ɂ܂�����

            return NewtMakePointer(newObj);
        }
    }

    return (newtRef)r;
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�̃f�[�^�����o�b�t�@�Ɏ�o��
 *
 * @param r			[in] �I�u�W�F�N�g
 * @param data		[out]�o�b�t�@
 * @param len		[in] �o�b�t�@��
 *
 * @return			�Ȃ�
 */

void NewtGetObjData(newtRefArg r, uint8_t * data, uint32_t len)
{
    newtObjRef	obj;
    uint8_t *	objData;

    obj = NewtRefToPointer(r);
    objData = NewtObjToBinary(obj);

    memcpy(data, objData, len);
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�����e�������`�F�b�N����
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			true	���e����
 * @retval			false   ���e�����łȂ�
 */

bool NewtRefIsLiteral(newtRefArg r)
{
    if (NewtRefIsPointer(r))
    {
        newtObjRef	obj;

        obj = NewtRefToPointer(r);

        return NewtObjIsLiteral(obj);
    }

    return true;
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�ɃX�E�B�[�v�t���O�������Ă��邩�`�F�b�N����
 *
 * @param r			[in] �I�u�W�F�N�g
 * @param mark		[in] �}�[�N
 *
 * @retval			true	�X�E�B�[�v�t���O�������Ă���
 * @retval			false   �X�E�B�[�v�t���O�������Ă��Ȃ�
 */

bool NewtRefIsSweep(newtRefArg r, bool mark)
{
    if (NewtRefIsPointer(r))
    {
        newtObjRef	obj;

        obj = NewtRefToPointer(r);

        return NewtObjIsSweep(obj, mark);
    }

    return true;
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�� NIL ���`�F�b�N����
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			true	NIL �܂��� #UNBIND
 * @retval			false   NIL �łȂ�
 */

bool NewtRefIsNIL(newtRefArg r)
{
    return (kNewtRefNIL == r || kNewtRefUnbind == r);
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g���V���{�����`�F�b�N����
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			true	�V���{��
 * @retval			false   �V���{���łȂ�
 */

bool NewtRefIsSymbol(newtRefArg r)
{
    return (kNewtSymbol == NewtGetRefType(r, true));
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̃n�b�V���l���擾����
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�n�b�V���l
 */

uint32_t NewtRefToHash(newtRefArg r)
{
    newtSymDataRef	sym;

    sym = NewtRefToSymbol(r);

    if (sym != NULL)
        return sym->hash;
    else
        return 0;
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�������񂩃`�F�b�N����
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			true	������
 * @retval			false   ������łȂ�
 */

bool NewtRefIsString(newtRefArg r)
{
    return (kNewtString == NewtGetRefType(r, true));
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g���������`�F�b�N����
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			true	����
 * @retval			false   �����łȂ�
 */

bool NewtRefIsInteger(newtRefArg r)
{
	return (NewtRefIsInt30(r) || NewtRefIsInt32(r));
}


/*------------------------------------------------------------------------*/
/** �����I�u�W�F�N�𐮐��ɂ���
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			����
 */

int32_t NewtRefToInteger(newtRefArg r)
{
    int32_t	v = 0;

    if (NewtRefIsInt30(r))
        v = NewtRefToInt30(r);
    else
        NewtGetObjData(r, (uint8_t *)&v, sizeof(v));

    return v;
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g��32bit�������`�F�b�N����
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			true	32bit����
 * @retval			false   32bit�����łȂ�
 */

bool NewtRefIsInt32(newtRefArg r)
{
    return (kNewtInt32 == NewtGetRefType(r, true));
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�����������_���`�F�b�N����
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			true	���������_
 * @retval			false   ���������_�łȂ�
 */

bool NewtRefIsReal(newtRefArg r)
{
    return (kNewtReal == NewtGetRefType(r, true));
}


/*------------------------------------------------------------------------*/
/** ���l�I�u�W�F�N�𕂓������_�ɂ���
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			���������_
 */

double NewtRefToReal(newtRefArg r)
{
    double	v = 0.0;

    if (NewtRefIsInteger(r))
        v = NewtRefToInteger(r);
    else
        NewtGetObjData(r, (uint8_t *)&v, sizeof(v));

    return v;
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g���o�C�i���I�u�W�F�N�g���`�F�b�N����
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			true	�o�C�i���I�u�W�F�N�g
 * @retval			false   �o�C�i���I�u�W�F�N�g�łȂ�
 */

bool NewtRefIsBinary(newtRefArg r)
{
    if (NewtRefIsPointer(r))
    {
        uint16_t	type;

        type = NewtGetObjectType(NewtRefToPointer(r), false);

        return (type == kNewtBinary);
    }

    return false;
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̃I�u�W�F�N�g�f�[�^���擾����
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�I�u�W�F�N�g�f�[�^
 */

void * NewtRefToData(newtRefArg r)
{
    newtObjRef	obj;

    obj = NewtRefToPointer(r);

    return NewtObjData(obj);
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g���z��I�u�W�F�N�g���`�F�b�N����
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			true	�z��I�u�W�F�N�g
 * @retval			false   �z��I�u�W�F�N�g�łȂ�
 */

bool NewtRefIsArray(newtRefArg r)
{
    return (NewtGetRefType(r, true) == kNewtArray);
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g���t���[���I�u�W�F�N�g���`�F�b�N����
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			true	�t���[���I�u�W�F�N�g
 * @retval			false   �t���[���I�u�W�F�N�g�łȂ�
 */

bool NewtRefIsFrame(newtRefArg r)
{
    return (NewtGetRefType(r, true) == kNewtFrame);
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g���t���[���܂��͔z��I�u�W�F�N�g���`�F�b�N����
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			true	�t���[���܂��͔z��I�u�W�F�N�g
 * @retval			false   �t���[���܂��͔z��I�u�W�F�N�g�łȂ�
 */

bool NewtRefIsFrameOrArray(newtRefArg r)
{
    uint16_t	type;

    type = NewtGetRefType(r, true);
    return (type == kNewtFrame || type == kNewtArray);
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g���C�~�f�B�G�C�g�i���l�j���`�F�b�N����
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			true	�C�~�f�B�G�C�g�ł���
 * @retval			false   �C�~�f�B�G�C�g�łȂ�
 */

bool NewtRefIsImmediate(newtRefArg r)
{
#ifdef __NAMED_MAGIC_POINTER__
    if (NewtRefIsMagicPointer(r))
		return false;
#endif /* __NAMED_MAGIC_POINTER__ */

    return ! NewtRefIsPointer(r);
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g���R�[�h�u���b�N�i�֐��I�u�W�F�N�g�j���`�F�b�N����
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			true	�R�[�h�u���b�N
 * @retval			false   �R�[�h�u���b�N�łȂ�
 */

bool NewtRefIsCodeBlock(newtRefArg r)
{
    if (NewtRefIsFrame(r))
    {
        newtRefVar	klass;

        klass = NcClassOf(r);

        if (NewtRefEqual(klass, NSSYM0(CodeBlock)))
            return true;
    }

    return false;
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g���l�C�e�B�u�֐��ircvr�Ȃ��֐��I�u�W�F�N�g�j���`�F�b�N����
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			true	�l�C�e�B�u�֐�
 * @retval			false   �l�C�e�B�u�֐��łȂ�
 */

bool NewtRefIsNativeFn(newtRefArg r)
{
    if (NewtRefIsFrame(r))
        return NewtRefEqual(NcClassOf(r), NSSYM0(_function.native0));
	else
		return false;
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g���l�C�e�B�u�֐��ircvr����֐��I�u�W�F�N�g�j���`�F�b�N����
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			true	�l�C�e�B�u�֐�
 * @retval			false   �l�C�e�B�u�֐��łȂ�
 */

bool NewtRefIsNativeFunc(newtRefArg r)
{
    if (NewtRefIsFrame(r))
        return NewtRefEqual(NcClassOf(r), NSSYM0(_function.native));
	else
		return false;
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g���֐��I�u�W�F�N�g���`�F�b�N����
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			true	�֐��I�u�W�F�N�g
 * @retval			false   �֐��I�u�W�F�N�g�łȂ�
 */

bool NewtRefIsFunction(newtRefArg r)
{
	return (NewtRefFunctionType(r) != kNewtNotFunction);
}


/*------------------------------------------------------------------------*/
/** �֐��I�u�W�F�N�g�̃^�C�v���擾����
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			kNewtNotFunction	�֐��I�u�W�F�N�g�łȂ�
 * @retval			kNewtCodeBlock		�o�C�g�R�[�h�֐�
 * @retval			kNewtNativeFn		�l�C�e�B�u�֐��ircvr�Ȃ��AOld Style�j
 * @retval			kNewtNativeFunc		�l�C�e�B�u�֐��ircvr����ANew Style�j
 */

int NewtRefFunctionType(newtRefArg r)
{
    if (NewtRefIsFrame(r))
    {
        newtRefVar	klass;

        klass = NcClassOf(r);

        if (NewtRefEqual(klass, NSSYM0(CodeBlock)))
			return kNewtCodeBlock;

		if (NewtRefEqual(klass, NSSYM0(_function.native0)))
			return kNewtNativeFn;

		if (NewtRefEqual(klass, NSSYM0(_function.native)))
			return kNewtNativeFunc;
    }

    return kNewtNotFunction;
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�����K�\���I�u�W�F�N�g���`�F�b�N����
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @retval			true	���K�\���I�u�W�F�N�g
 * @retval			false   ���K�\���I�u�W�F�N�g�łȂ�
 */

bool NewtRefIsRegex(newtRefArg r)
{
    if (NewtRefIsFrame(r))
    {
        newtRefVar	klass;

        klass = NcClassOf(r);

        if (NewtRefEqual(klass, NSSYM0(regex)))
            return true;
    }

    return false;
}


/*------------------------------------------------------------------------*/
/** �����I�u�W�F�N�g���A�h���X�ɕϊ�����
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�A�h���X
 */

void * NewtRefToAddress(newtRefArg r)
{
	if (NewtRefIsInteger(r))
		return (void *)(((uint32_t)NewtRefToInteger(r)) << NOBJ_ADDR_SHIFT);
	else
		return NULL;
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �o�C�i���I�u�W�F�N�g���쐬����
 *
 * @param klass		[in] �N���X
 * @param data		[in] �������f�[�^
 * @param size		[in] �T�C�Y
 * @param literal	[in] ���e�����t���O
 *
 * @return			�o�C�i���I�u�W�F�N�g
 */

newtRef NewtMakeBinary(newtRefArg klass, uint8_t * data, uint32_t size, bool literal)
{
    newtObjRef	obj;

    obj = NewtObjAlloc(klass, size, 0, literal);

    if (obj != NULL)
    {
        uint8_t *	objData;

        objData = NewtObjToBinary(obj);

        if (data != NULL && 0 < size)
        {
            uint32_t	setlen;

            setlen = NewtObjSize(obj) - size;

            if (0 < setlen)
                memset(objData + size, 0, setlen);

            memcpy(objData, data, size);
        }
        else
        {
            memset(objData, 0, NewtObjSize(obj));
        }

        return NewtMakePointer(obj);
    }

    return kNewtRefUnbind;
}


/*------------------------------------------------------------------------*/
/** �o�C�i���I�u�W�F�N�g�̃I�u�W�F�N�g�f�[�^�̃T�C�Y��ύX����
 *
 * @param obj		[in] �I�u�W�F�N�g�f�[�^
 * @param n			[in] �T�C�Y
 *
 * @return			�T�C�Y�̕ύX���ꂽ�I�u�W�F�N�g�f�[�^
 */

newtObjRef NewtObjBinarySetLength(newtObjRef obj, uint32_t n)
{
    return NewtObjResize(obj, n);
}


/*------------------------------------------------------------------------*/
/** �o�C�i���I�u�W�F�N�g�̃T�C�Y��ύX����
 *
 * @param r			[in] �I�u�W�F�N�g
 * @param n			[in] �T�C�Y
 *
 * @return			�I�u�W�F�N�g
 */

newtRef NewtBinarySetLength(newtRefArg r, uint32_t n)
{
    newtObjRef	obj;

    obj = NewtRefToPointer(r);
    NewtObjBinarySetLength(obj, n);

    return r;
}


/*------------------------------------------------------------------------*/
/** �V���{���I�u�W�F�N�g���쐬����
 *
 * @param s			[in] ������
 *
 * @return			�V���{���I�u�W�F�N�g
 *
 * @note			�V���{�������ɑ��݂���ꍇ�͍쐬�����Ɋ��ɂ���V���{���I�u�W�F�N�g��Ԃ�
 */

newtRef NewtMakeSymbol(const char *s)
{
    return NewtLookupSymbolTable(s);
}


/*------------------------------------------------------------------------*/
/** �V���{���̃I�u�W�F�N�g�f�[�^�̒������擾����
 *
 * @param obj		[in] �I�u�W�F�N�g�f�[�^
 *
 * @return			����
 */

uint32_t NewtObjSymbolLength(newtObjRef obj)
{
    newtSymDataRef	sym;

    sym = NewtObjToSymbol(obj);
    return strlen(sym->name);
}


/*------------------------------------------------------------------------*/
/** ������I�u�W�F�N�g���쐬����
 *
 * @param s			[in] ������
 * @param literal	[in] ���e�����t���O
 *
 * @return			������I�u�W�F�N�g
 */

newtRef NewtMakeString(const char *s, bool literal)
{
    return NewtMakeBinary(NSSYM0(string), (uint8_t *)s, strlen(s) + 1, literal); 
}


/*------------------------------------------------------------------------*/
/** �������w�肵�ĕ�����I�u�W�F�N�g���쐬����
 *
 * @param s			[in] ������
 * @param len		[in] ������̒���
 * @param literal	[in] ���e�����t���O
 *
 * @return			������I�u�W�F�N�g
 */

newtRef NewtMakeString2(const char *s, uint32_t len, bool literal)
{
	newtRefVar  r;

    r = NewtMakeBinary(NSSYM0(string), (uint8_t *)s, len + 1, literal); 

	if (NewtRefIsNotNIL(r))
	{
        char *	objData;

        objData = NewtRefToString(r);

		if (s != NULL && 0 < len)
		{
//			strncpy(objData, s, len);
			objData[len] = '\0';
		}
		else
		{
			objData[0] = '\0';
		}
	}

	return r;
}


/*------------------------------------------------------------------------*/
/** ������I�u�W�F�N�g�̃I�u�W�F�N�g�f�[�^�̒������擾����
 *
 * @param obj		[in] �I�u�W�F�N�g�f�[�^
 *
 * @return			����
 */

uint32_t NewtObjStringLength(newtObjRef obj)
{
    char *	s;

    s = NewtObjToString(obj);
    return strlen(s);
}


/*------------------------------------------------------------------------*/
/** ������I�u�W�F�N�g�̃I�u�W�F�N�g�f�[�^�̒�����ύX����
 *
 * @param obj		[in] �I�u�W�F�N�g�f�[�^
 * @param len		[in] ����
 *
 * @return			�������ύX���ꂽ�I�u�W�F�N�g�f�[�^
 */

newtObjRef NewtObjStringSetLength(newtObjRef obj, uint32_t n)
{
    return NewtObjBinarySetLength(obj, n + 1);
}


/*------------------------------------------------------------------------*/
/** ������I�u�W�F�N�g�̒�����ύX����
 *
 * @param r			[in] �I�u�W�F�N�g
 * @param n			[in] ����
 *
 * @return			�I�u�W�F�N�g
 */

newtRef NewtStringSetLength(newtRefArg r, uint32_t n)
{
    newtObjRef	obj;

    obj = NewtRefToPointer(r);
    NewtObjStringSetLength(obj, n);

    return r;
}


/*------------------------------------------------------------------------*/
/** �����I�u�W�F�N�g���쐬����
 *
 * @param v			[in] ����
 *
 * @return			�����I�u�W�F�N�g
 */

newtRef NewtMakeInteger(int32_t v)
{
	if (-536870912 <= v && v <= 536870911)
	{   // 30bit �ȓ��̏ꍇ
		return NewtMakeInt30(v);
	}
	else
	{
		return NewtMakeInt32(v);
	}
}


/*------------------------------------------------------------------------*/
/** 32bit�����I�u�W�F�N�g���쐬����
 *
 * @param v			[in] ����
 *
 * @return			32bit�����I�u�W�F�N�g
 */

newtRef NewtMakeInt32(int32_t v)
{
    return NewtMakeBinary(NSSYM0(int32), (uint8_t *)&v, sizeof(v), true); 
}


/*------------------------------------------------------------------------*/
/** ���������_�I�u�W�F�N�g���쐬����
 *
 * @param v			[in] ���������_
 *
 * @return			���������_�I�u�W�F�N�g
 */

newtRef NewtMakeReal(double v)
{
    return NewtMakeBinary(NSSYM0(real), (uint8_t *)&v, sizeof(v), true); 
}


/*------------------------------------------------------------------------*/
/** �z��I�u�W�F�N�g���쐬����
 *
 * @param klass		[in] �N���X
 * @param n			[in] ����
 *
 * @return			�z��I�u�W�F�N�g
 */

newtRef NewtMakeArray(newtRefArg klass, uint32_t n)
{
    return NewtMakeSlotsObj(klass, n, 0);
}

void NewtMakeInitSlots(newtRefArg r, uint32_t st, uint32_t n, uint32_t step, newtRefVar v[])
{
    if (v != NULL)
    {
        newtRef *	slots;
        uint32_t	i;
    
        slots = NewtRefToSlots(r);
    
        for (i = 0; i < n; i++)
        {
            slots[st + i] = *v;
            v += step;
        }
    }
}


/*------------------------------------------------------------------------*/
/** �z��I�u�W�F�N�g���쐬���ď���������
 *
 * @param klass		[in] �N���X
 * @param n			[in] ����
 * @param v			[in] �������f�[�^
 *
 * @return			�z��I�u�W�F�N�g
 */

newtRef NewtMakeArray2(newtRefArg klass, uint32_t n, newtRefVar v[])
{
    newtRefVar	r;

    r = NewtMakeSlotsObj(klass, n, 0);

    if (NewtRefIsNotNIL(r))
        NewtMakeInitSlots(r, 0, n, 1, v);

    return r;
}


/*------------------------------------------------------------------------*/
/** �}�b�v���쐬���ď���������
 *
 * @param superMap	[in] �X�[�p�}�b�v
 * @param n			[in] ����
 * @param v			[in] �������f�[�^
 *
 * @return			�}�b�v�I�u�W�F�N�g
 */

newtRef NewtMakeMap(newtRefArg superMap, uint32_t n, newtRefVar v[])
{
    newtRefVar	r;
    int32_t	flags = 0;

    r = NewtMakeSlotsObj(NewtMakeInteger(flags), n + 1, 0);
    NewtSetArraySlot(r, 0, superMap);

    if (NewtRefIsNotNIL(superMap))
    {
        flags = NewtRefToInteger(NcClassOf(superMap));
        flags &= ~ kNewtMapSorted;
    }

    if (NewtRefIsNotNIL(r) && v != NULL)
    {
//        NewtMakeInitSlots(r, 1, n, 2, v);

        newtRef *	slots;
        uint32_t	i;
    
        slots = NewtRefToSlots(r);
    
        for (i = 1; i <= n; i++)
        {
            slots[i] = *v;

            if (slots[i] == NSSYM0(_proto))
                flags |= kNewtMapProto;

            v += 2;
        }
    }

    NcSetClass(r, NewtMakeInteger(flags));

    return r;
}


/*------------------------------------------------------------------------*/
/** �}�b�v�Ƀt���O���Z�b�g����
 *
 * @param map		[in] �}�b�v�I�u�W�F�N�g
 * @param bit		[in] �t���O
 *
 * @return			�Ȃ�
 */

void NewtSetMapFlags(newtRefArg map, int32_t bit)
{
    int32_t	flags;

    flags = NewtRefToInteger(NcClassOf(map));
    flags |= bit;
    NcSetClass(map, NewtMakeInteger(flags));
}


/*------------------------------------------------------------------------*/
/** �}�b�v�̃t���O���N���A����
 *
 * @param map		[in] �}�b�v�I�u�W�F�N�g
 * @param bit		[in] �t���O
 *
 * @return			�Ȃ�
 */

void NewtClearMapFlags(newtRefArg map, int32_t bit)
{
    int32_t	flags;

    flags = NewtRefToInteger(NcClassOf(map));
    flags &= ~ bit;
    NcSetClass(map, NewtMakeInteger(flags));
}


/*------------------------------------------------------------------------*/
/** �}�b�v�̒������擾����
 *
 * @param map		[in] �}�b�v�I�u�W�F�N�g
 *
 * @return			����
 */

uint32_t NewtMapLength(newtRefArg map)
{
    uint32_t	len = 0;
    newtRefVar	r;

    r = (newtRef)map;

    while (NewtRefIsNotNIL(r))
    {
        len += NewtLength(r) - 1;
        r = NewtGetArraySlot(r, 0);
    }

    return len;
}


/*------------------------------------------------------------------------*/
/** �t���[���I�u�W�F�N�g���쐬����
 *
 * @param map		[in] �}�b�v
 * @param n			[in] ����
 *
 * @return			�t���[���I�u�W�F�N�g
 */

newtRef NewtMakeFrame(newtRefArg map, uint32_t n)
{
    newtRefVar	m;

    m = (newtRef)map;

    if (NewtRefIsNIL(m))
        m = NewtMakeMap(kNewtRefNIL, n, NULL);

    return NewtMakeSlotsObj(m, n, kNewtObjFrame);
}


/*------------------------------------------------------------------------*/
/** �t���[���I�u�W�F�N�g���쐬���ď���������
 *
 * @param n			[in] ����
 * @param v			[in] �������f�[�^
 *
 * @return			�t���[���I�u�W�F�N�g
 */

newtRef NewtMakeFrame2(uint32_t n, newtRefVar v[])
{
    newtRefVar	m;
    newtRefVar	r;

    m = NewtMakeMap(kNewtRefNIL, n, v);
    r = NewtMakeFrame(m, n);

    if (NewtRefIsNotNIL(r))
        NewtMakeInitSlots(r, 0, n, 2, v + 1);

    return r;
}


/*------------------------------------------------------------------------*/
/** �X���b�g�I�u�W�F�N�g���쐬����
 *
 * @param r			[in] �N���X�^�}�b�v
 * @param n			[in] ����
 * @param type		[in] �^�C�v
 *
 * @return			�I�u�W�F�N�g
 */

newtRef NewtMakeSlotsObj(newtRefArg r, uint32_t n, uint16_t type)
{
    newtObjRef	obj;
    uint32_t	size;

    size = sizeof(newtRef) * n;
    obj = NewtObjAlloc(r, size, kNewtObjSlotted | type, false);

    if (obj != NULL)
    {
        newtRef *	slots;
        uint32_t	i;

        slots = NewtObjToSlots(obj);

        for (i = 0; i < n; i++)
        {
            slots[i] = kNewtRefUnbind;
        }

        return NewtMakePointer(obj);
    }

    return kNewtRefNIL;
}


/*------------------------------------------------------------------------*/
/** �X���b�g�I�u�W�F�N�g�̃I�u�W�F�N�g�f�[�^�̒������擾����
 *
 * @param obj		[in] �I�u�W�F�N�g�f�[�^
 *
 * @return			����
 */

uint32_t NewtObjSlotsLength(newtObjRef obj)
{
    return NewtObjSize(obj) / sizeof(newtRef);
}


/*------------------------------------------------------------------------*/
/** �X���b�g�I�u�W�F�N�g�̃I�u�W�F�N�g�f�[�^�̒�����ύX����
 *
 * @param obj		[in] �I�u�W�F�N�g�f�[�^
 * @param n			[in] ����
 * @param v			[in] �������f�[�^
 *
 * @return			�����̕ύX���ꂽ�I�u�W�F�N�g�f�[�^
 */

newtObjRef NewtObjSlotsSetLength(newtObjRef obj, uint32_t n, newtRefArg v)
{
    uint32_t	size;
    uint32_t	len;

    len = NewtObjSlotsLength(obj);
    size = sizeof(newtRef) * n;
    obj = NewtObjResize(obj, size);

    if (obj != NULL)
    {
        newtRef *	slots;
        uint32_t	i;

        slots = NewtObjToSlots(obj);

        for (i = len; i < n; i++)
        {
            slots[i] = v;
        }
    }

    return obj;
}


/*------------------------------------------------------------------------*/
/** �X���b�g�I�u�W�F�N�g�̃I�u�W�F�N�g�f�[�^�ɒl��ǉ�����
 *
 * @param obj		[in] �I�u�W�F�N�g�f�[�^
 * @param v			[in] �l�I�u�W�F�N�g
 *
 * @return			�l�I�u�W�F�N�g
 */

newtRef NewtObjAddArraySlot(newtObjRef obj, newtRefArg v)
{
    uint32_t	len;

    len = NewtObjSlotsLength(obj);
    NewtObjSlotsSetLength(obj, len + 1, v);

    return v;
}


/*------------------------------------------------------------------------*/
/** �X���b�g�I�u�W�F�N�g�̒�����ύX����
 *
 * @param r			[in] �I�u�W�F�N�g
 * @param n			[in] ����
 * @param v			[in] �������f�[�^
 *
 * @return			�����̕ύX���ꂽ�I�u�W�F�N�g
 */

newtRef NewtSlotsSetLength(newtRefArg r, uint32_t n, newtRefArg v)
{
    newtObjRef	obj;

    obj = NewtRefToPointer(r);
    NewtObjSlotsSetLength(obj, n, v);

    return r;
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̒�����ύX����
 *
 * @param r			[in] �I�u�W�F�N�g
 * @param n			[in] ����
 *
 * @return			�����̕ύX���ꂽ�I�u�W�F�N�g
 */

newtRef NewtSetLength(newtRefArg r, uint32_t n)
{
    uint16_t	type;

    type = NewtGetRefType(r, true);

    switch (type)
    {
        case kNewtBinary:
            NewtBinarySetLength(r, n);
            break;

        case kNewtString:
            NewtStringSetLength(r, n);
            break;

        case kNewtArray:
        case kNewtFrame:
            NewtSlotsSetLength(r, n, kNewtRefUnbind);
            break;
    }

    return r;
}


/*------------------------------------------------------------------------*/
/** �A�h���X���琮���I�u�W�F�N�g���쐬����
 *
 * @param addr		[in] �A�h���X
 *
 * @return			�����I�u�W�F�N�g
 */

newtRef NewtMakeAddress(void * addr)
{
	return NewtMakeInteger(((uint32_t)addr) >> NOBJ_ADDR_SHIFT);
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �G���[�ԍ��̗�O�𔭐�����
 *
 * @param err		[in] �G���[�ԍ�
 *
 * @return			kNewtRefUnbind
 */

newtRef NewtThrow0(int32_t err)
{
	return NewtThrow(err, kNewtRefUnbind);
}


/*------------------------------------------------------------------------*/
/** �G���[�ԍ��ɑΉ������O�V���{�����쐬����
 *
 * @param err		[in] �G���[�ԍ�
 *
 * @return			��O�V���{��
 */

newtRef NewtMakeThrowSymbol(int32_t err)
{
	newtRefVar  symstr;
	int32_t		errbase;

	symstr = NSSTR("evt.ex.fr");
	errbase = (err % 100) * 100;

	switch (errbase)
	{
		case kNErrObjectBase:
			NewtStrCat(symstr, ".obj");
			break;

		case kNErrBadTypeBase:
			NewtStrCat(symstr, ".type");
			break;

		case kNErrCompilerBase:
			NewtStrCat(symstr, ".compr");
			break;

		case kNErrInterpreterBase:
			NewtStrCat(symstr, ".intrp");
			break;

		case kNErrFileBase:
			NewtStrCat(symstr, ".file");
			break;

		case kNErrMiscBase:
			break;
	}

	NewtStrCat(symstr, ";type.ref.frame");

	return NcMakeSymbol(symstr);
}


/*------------------------------------------------------------------------*/
/** �G���[�ԍ��ƒl�I�u�W�F�N�g���f�[�^�ɗ�O�𔭐�����
 *
 * @param err		[in] �G���[�ԍ�
 * @param value		[in] �l�I�u�W�F�N�g
 *
 * @return			kNewtRefUnbind
 */

newtRef NewtThrow(int32_t err, newtRefArg value)
{
    newtRefVar	sym;
    newtRefVar	data;

	sym = NewtMakeThrowSymbol(err);

    data = NcMakeFrame();
    NcSetSlot(data, NSSYM0(errorCode), NewtMakeInteger(err));

	if (value != kNewtRefUnbind)
		NcSetSlot(data, NSSYM0(value), value);

    return NcThrow(sym, data);
}


/*------------------------------------------------------------------------*/
/** �G���[�ԍ��ƃV���{�����f�[�^�ɗ�O�𔭐�����
 *
 * @param err		[in] �G���[�ԍ�
 * @param symbol	[in] �V���{��
 *
 * @return			kNewtRefUnbind
 */

newtRef NewtThrowSymbol(int32_t err, newtRefArg symbol)
{
    newtRefVar	sym;
    newtRefVar	data;

	sym = NewtMakeThrowSymbol(err);

    data = NcMakeFrame();
    NcSetSlot(data, NSSYM0(errorCode), NewtMakeInteger(err));

	if (symbol != kNewtRefUnbind)
		NcSetSlot(data, NSSYM0(symbol), symbol);

    return NcThrow(sym, data);
}


/*------------------------------------------------------------------------*/
/** Out Of Bounds �G���[�𔭐�����
 *
 * @param value		[in] �l�I�u�W�F�N�g
 * @param index		[in] �ʒu
 *
 * @return			kNewtRefUnbind
 */

newtRef NewtErrOutOfBounds(newtRefArg value, int32_t index)
{
	newtRefVar  symstr;
    newtRefVar	data;

	symstr = NSSTR("evt.ex.fr");
	NewtStrCat(symstr, ";type.ref.frame");

    data = NcMakeFrame();
    NcSetSlot(data, NSSYM0(errorCode), NewtMakeInteger(kNErrOutOfBounds));
	NcSetSlot(data, NSSYM0(value), value);
	NcSetSlot(data, NSSYM0(index), NewtMakeInteger(index));

    return NcThrow(NcMakeSymbol(symstr), data);
}


/*------------------------------------------------------------------------*/
/** �G���[���b�Z�[�W��\������
 *
 * @param err		[in] �G���[�ԍ�
 *
 * @return			�Ȃ�
 */

void NewtErrMessage(int32_t err)
{
    switch (err)
    {
        case kNErrNone:
            break;

        case kNErrObjectReadOnly:
            NewtFprintf(stderr, "*** Object Read Only\n");
            break;
    }
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** 32bit�����̔�r
 *
 * @param r1		[in] 32bit�����P
 * @param r2		[in] 32bit�����Q
 *
 * @retval			-1		r1 < r2
 * @retval			0		r1 = r2
 * @retval			1		r1 > r2
 */

int NewtInt32Compare(newtRefArg r1, newtRefArg r2)
{
    int32_t	i1;
    int32_t	i2;

    i1 = NewtRefToInteger(r1);
    i2 = NewtRefToInteger(r2);

    if (i1 < i2)
        return -1;
    else if (i1 > i2)
        return 1;
    else
        return 0;
}


/*------------------------------------------------------------------------*/
/** ���������_�̔�r
 *
 * @param r1		[in] ���������_�P
 * @param r2		[in] ���������_�Q
 *
 * @retval			-1		r1 < r2
 * @retval			0		r1 = r2
 * @retval			1		r1 > r2
 */

int NewtRealCompare(newtRefArg r1, newtRefArg r2)
{
    double real1;
    double real2;

    real1 = NewtRefToReal(r1);
    real2 = NewtRefToReal(r2);

    if (real1 < real2)
        return -1;
    else if (real1 > real2)
        return 1;
    else
        return 0;
}


/*------------------------------------------------------------------------*/
/** �V���{��������I�ɔ�r�i�啶���������͋�ʂ���Ȃ��j
 *
 * @param r1		[in] �V���{���P
 * @param r2		[in] �V���{���Q
 *
 * @retval			���̐���	r1 < r2
 * @retval			0		r1 = r2
 * @retval			���̐���	r1 > r2
 */

int NewtSymbolCompareLex(newtRefArg r1, newtRefArg r2)
{
    newtSymDataRef	sym1;
    newtSymDataRef	sym2;

	if (r1 == r2)
		return 0;

    sym1 = NewtRefToSymbol(r1);
    sym2 = NewtRefToSymbol(r2);

    return strcasecmp(sym1->name, sym2->name);
}


/*------------------------------------------------------------------------*/
/** ������I�u�W�F�N�g�̔�r
 *
 * @param r1		[in] ������I�u�W�F�N�g�P
 * @param r2		[in] ������I�u�W�F�N�g�Q
 *
 * @retval			���̐���	r1 < r2
 * @retval			0		r1 = r2
 * @retval			���̐���	r1 > r2
 */

int NewtStringCompare(newtRefArg r1, newtRefArg r2)
{
    char *	s1;
    char *	s2;

    s1 = NewtRefToString(r1);
    s2 = NewtRefToString(r2);

    return strcmp(s1, s2);
}


/*------------------------------------------------------------------------*/
/** �o�C�i���I�u�W�F�N�g�̔�r
 *
 * @param r1		[in] �o�C�i���I�u�W�F�N�g�P
 * @param r2		[in] �o�C�i���I�u�W�F�N�g�Q
 *
 * @retval			-1		r1 < r2
 * @retval			0		r1 = r2
 * @retval			1		r1 > r2
 */

int NewtBinaryCompare(newtRefArg r1, newtRefArg r2)
{
    int32_t	len1;
    int32_t	len2;
    int32_t	len;
    uint8_t *	d1;
    uint8_t *	d2;
    int		r;

    len1 = NewtBinaryLength(r1);
    len2 = NewtBinaryLength(r2);

    if (len1 == 0 || len2 == 0)
        return (len1 - len2);

    d1 = NewtRefToBinary(r1);
    d2 = NewtRefToBinary(r2);

    if (len1 < len2)
        len = len1;
    else
        len = len2;

    r = memcmp(d1, d2, len);

    if (r == 0)
    {
        if (len1 < len2)
            r = -1;
        else if (len1 > len2)
            r = 1;
    }

    return r;
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �v�Z�\�Ȉ����Ȃ�Όv�Z���ʂ̃I�u�W�F�N�g�^�C�v��Ԃ�
 *
 * @param r1		[in] �I�u�W�F�N�g�P
 * @param r2		[in] �I�u�W�F�N�g�Q
 *
 * @retval			�I�u�W�F�N�g�^�C�v		�v�Z�\
 * @retval			kNewtUnknownType	�v�Z�s��
 */

uint16_t NewtArgsType(newtRefArg r1, newtRefArg r2)
{
    uint16_t	type1;
    uint16_t	type2;

    type1 = NewtGetRefType(r1, true);
    type2 = NewtGetRefType(r2, true);

	if (type1 == type2)
		return type1;

	if (type1 == kNewtInt30)
		type1 = kNewtInt32;

	if (type2 == kNewtInt30)
		type2 = kNewtInt32;

    if (type1 == kNewtInt32 && type2 == kNewtReal)
        type1 = kNewtReal;
    else if (type1 == kNewtReal && type2 == kNewtInt32)
        type2 = kNewtReal;

    if (type1 == type2)
        return type1;
    else
        return kNewtUnknownType;
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̑召��r
 *
 * @param r1		[in] �I�u�W�F�N�g�P
 * @param r2		[in] �I�u�W�F�N�g�Q
 *
 * @retval			1		r1 > r2
 * @retval			0		r1 = r2
 * @retval			-1		r1 < r2
 */

int16_t NewtObjectCompare(newtRefArg r1, newtRefArg r2)
{
    int	r = -1;

    switch (NewtArgsType(r1, r2))
    {
        case kNewtInt30:
            if ((int32_t)r1 < (int32_t)r2)
                r = -1;
            else if ((int32_t)r1 > (int32_t)r2)
                r = 1;
            else
                r = 0;
            break;

        case kNewtCharacter:
            if (r1 < r2)
                r = -1;
            else if (r1 > r2)
                r = 1;
            else
                r = 0;
            break;

        case kNewtInt32:
            r = NewtInt32Compare(r1, r2);
            break;

        case kNewtReal:
            r = NewtRealCompare(r1, r2);
            break;

        case kNewtSymbol:
            r = NewtSymbolCompareLex(r1, r2);
            break;

        case kNewtString:
            r = NewtStringCompare(r1, r2);
            break;

        case kNewtBinary:
            r = NewtBinaryCompare(r1, r2);
            break;
    }

    return r;
}


/*------------------------------------------------------------------------*/
/** �Q�Ƃ̔�r
 *
 * @param r1		[in] �Q�ƂP
 * @param r2		[in] �Q�ƂQ
 *
 * @retval			true	���l
 * @retval			false   ���l�łȂ�
 */

bool NewtRefEqual(newtRefArg r1, newtRefArg r2)
{
    int	r = -1;

    if (r1 == r2)
        return true;
    else if (NewtRefIsSymbol(r1))
        return false;

    switch (NewtArgsType(r1, r2))
    {
        case kNewtInt32:
            r = NewtInt32Compare(r1, r2);
            break;

        case kNewtReal:
            r = NewtRealCompare(r1, r2);
            break;

		default:
			return false;
    }

	return (r == 0);
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̔�r
 *
 * @param r1		[in] �I�u�W�F�N�g�P
 * @param r2		[in] �I�u�W�F�N�g�Q
 *
 * @retval			true	���l
 * @retval			false   ���l�łȂ�
 */

bool NewtObjectEqual(newtRefArg r1, newtRefArg r2)
{
    if (r1 == r2)
        return true;
    else if (NewtRefIsSymbol(r1))
        return false;
//        return NewtSymbolEqual(r1, r2);
    else
        return (NewtObjectCompare(r1, r2) == 0);
}


/*------------------------------------------------------------------------*/
/** �V���{���I�u�W�F�N�g�̔�r
 *
 * @param r1		[in] �V���{���I�u�W�F�N�g�P
 * @param r2		[in] �V���{���I�u�W�F�N�g�Q
 *
 * @retval			true	���l
 * @retval			false   ���l�łȂ�
 */

bool NewtSymbolEqual(newtRefArg r1, newtRefArg r2)
{
    newtSymDataRef	sym1;
    newtSymDataRef	sym2;

    if (r1 == r2)
        return true;

    if (! NewtRefIsSymbol(r1))
        return false;

    if (! NewtRefIsSymbol(r2))
        return false;

    sym1 = NewtRefToSymbol(r1);
    sym2 = NewtRefToSymbol(r2);

    if (sym1->hash == sym2->hash)
        return (strcasecmp(sym1->name, sym2->name) == 0);
    else
        return false;
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̒������擾����
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			����
 */

uint32_t NewtLength(newtRefArg r)
{
    uint32_t	len = 0;

    switch (NewtGetRefType(r, true))
    {
        case kNewtSymbol:
        case kNewtString:
        case kNewtBinary:
            len = NewtBinaryLength(r);
            break;

        case kNewtArray:
            len = NewtArrayLength(r);
            break;

        case kNewtFrame:
            len = NewtFrameLength(r);
            break;
    }

    return len;
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�́i�[���j�������擾
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�I�u�W�F�N�g�̒���
 *
 * @note			�t���[���̏ꍇ�̓v���g�p���Œ������v�Z����
 */

uint32_t NewtDeeplyLength(newtRefArg r)
{
    uint32_t	len = 0;

    switch (NewtGetRefType(r, true))
    {
        case kNewtFrame:
            len = NewtDeeplyFrameLength(r);
            break;

        default:
            len = NewtLength(r);
            break;
    }

    return len;
}


/*------------------------------------------------------------------------*/
/** �o�C�i���I�u�W�F�N�g�̒������擾
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�I�u�W�F�N�g�̒���
 */

uint32_t NewtBinaryLength(newtRefArg r)
{
    uint32_t	len = 0;

//    if (NewtIsBinary(r))
    {
        newtObjRef	obj;

        obj = NewtRefToPointer(r);
        len = NewtObjSize(obj);
    }

    return len;
}


/*------------------------------------------------------------------------*/
/** �V���{���I�u�W�F�N�g�̒������擾
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�I�u�W�F�N�g�̒���
 */

uint32_t NewtSymbolLength(newtRefArg r)
{
    newtObjRef	obj;

    obj = NewtRefToPointer(r);
    return NewtObjSymbolLength(obj);
}


/*------------------------------------------------------------------------*/
/** ������I�u�W�F�N�g�̒������擾
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�I�u�W�F�N�g�̒���
 */

uint32_t NewtStringLength(newtRefArg r)
{
    newtObjRef	obj;

    obj = NewtRefToPointer(r);
    return NewtObjStringLength(obj);
}


/*------------------------------------------------------------------------*/
/** �X���b�g�I�u�W�F�N�g�̒����i�X���b�g�̐��j���擾
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�I�u�W�F�N�g�̒���
 */

uint32_t NewtSlotsLength(newtRefArg r)
{
    newtObjRef	obj;

    obj = NewtRefToPointer(r);
    return NewtObjSlotsLength(obj);
}


/*------------------------------------------------------------------------*/
/** �v���g�p���Ńt���[���I�u�W�F�N�g�̒����i�X���b�g�̐��j���擾
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�I�u�W�F�N�g�̒���
 */

uint32_t NewtDeeplyFrameLength(newtRefArg r)
{
    newtRefVar	f;
    uint32_t	total = 0;
    uint32_t	len;

    f = r;

    while (true)
    {
        len = NewtFrameLength(f);
        total += len;

        if (len == 0) break;
        f = NcGetSlot(f, NSSYM0(_proto));
        if (NewtRefIsNIL(f)) break;

        total--;
    }

    return total;
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �t���[���̃I�u�W�F�N�g�f�[�^�� _proto �X���b�g�������`�F�b�N����
 *
 * @param obj		[in] �t���[���̃I�u�W�F�N�g�f�[�^
 *
 * @retval			true	_proto �X���b�g������
 * @retval			false	_proto �X���b�g�������Ȃ�
 */

bool NewtObjHasProto(newtObjRef obj)
{
    int32_t	flags;

    if (NewtRefIsNIL(obj->as.map))
        return false;

    flags = NewtRefToInteger(NcClassOf(obj->as.map));

    return ((flags & kNewtMapProto) != 0);
}


/*------------------------------------------------------------------------*/
/** �t���[���̃I�u�W�F�N�g�f�[�^����X���b�g�̒l����o��
 *
 * @param obj		[in] �t���[���̃I�u�W�F�N�g�f�[�^
 * @param slot		[in] �X���b�g�V���{��
 *
 * @return			�l�I�u�W�F�N�g
 */

newtRef NewtObjGetSlot(newtObjRef obj, newtRefArg slot)
{
    uint32_t	i;

	if (! NewtObjIsFrame(obj))
		return kNewtRefUnbind;

    if (slot == NSSYM0(_proto) && ! NewtObjHasProto(obj))
        return kNewtRefUnbind;

    if (NewtFindMapIndex(obj->as.map, slot, &i))
    {
        newtRef *	slots;

        slots = NewtObjToSlots(obj);
        return slots[i];
    }

    return kNewtRefUnbind;
}


/*------------------------------------------------------------------------*/
/** �}�b�v�̃\�[�g�t���O���`�F�b�N����
 *
 * @param r			[in] �}�b�v�I�u�W�F�N�g
 *
 * @retval			true	�\�[�g�t���O�� ON
 * @retval			false   �\�[�g�t���O�� OFF
 */

bool NewtMapIsSorted(newtRefArg r)
{
    newtRefVar	klass;
    uint32_t	flags;

    klass = NcClassOf(r);
    if (! NewtRefIsInteger(klass)) return false;

    flags = NewtRefToInteger(klass);

    return ((flags & kNewtMapSorted) != 0);
}


/*------------------------------------------------------------------------*/
/** �t���[���̃I�u�W�F�N�g�f�[�^�ɃX���b�g�̒l���Z�b�g����
 *
 * @param obj		[in] �t���[���̃I�u�W�F�N�g�f�[�^
 * @param slot		[in] �X���b�g�V���{��
 * @param v			[in] �l�I�u�W�F�N�g
 *
 * @return			�l�I�u�W�F�N�g
 */

newtRef NewtObjSetSlot(newtObjRef obj, newtRefArg slot, newtRefArg v)
{
    uint32_t	i;

/*
    if (NewtObjIsReadonly(obj))
        return NewtThrow0(kNErrObjectReadOnly);
*/

    if (NewtFindMapIndex(obj->as.map, slot, &i))
    {
        newtRef *	slots;

        slots = NewtObjToSlots(obj);
        slots[i] = v;
    }
    else
    {
        uint32_t	len;

        if (NewtRefIsLiteral(obj->as.map))
        {
            newtRefVar	map;

            map = NewtMakeMap(kNewtRefNIL, 1, NULL);

            NewtSetArraySlot(map, 0, obj->as.map);
            NewtSetArraySlot(map, 1, slot);

			if (NewtObjHasProto(obj))
				NewtSetMapFlags(map, kNewtMapProto);

            obj->as.map = map;
        }
        else
        {
            if (NewtMapIsSorted(obj->as.map))
            {
                // �}�b�v���\�[�g����Ă���ꍇ...

                newtSymDataRef	sym;
                int32_t	index;
            
                sym = NewtRefToSymbol(slot);
    
                NewtBSearchSymTable(obj->as.map, sym->name, sym->hash, 1, &index);
                NewtInsertArraySlot(obj->as.map, index, slot);
            }
            else
            {
                NcAddArraySlot(obj->as.map, slot);
            }
        }

        len = NewtObjSlotsLength(obj);
        NewtObjSlotsSetLength(obj, len + 1, v);

        if (slot == NSSYM0(_proto))
            NewtSetMapFlags(obj->as.map, kNewtMapProto);
    }

    return v;
}


/*------------------------------------------------------------------------*/
/** �z��̃I�u�W�F�N�g�f�[�^����w��ʒu�̗v�f���폜����
 *
 * @param obj		[in] �z��̃I�u�W�F�N�g�f�[�^
 * @param n			[in] �ʒu
 *
 * @return			�Ȃ�
 */

void NewtObjRemoveArraySlot(newtObjRef obj, int32_t n)
{
    newtRef *	slots;
    uint32_t	len;
    uint32_t	i;

    if (NewtObjIsReadonly(obj))
    {
        NewtThrow0(kNErrObjectReadOnly);
        return;
    }

    slots = NewtObjToSlots(obj);
    len = NewtObjSlotsLength(obj);

    for (i = n + 1; i < len; i++)
    {
        slots[i - 1] = slots[i] ;
    }

    NewtObjSlotsSetLength(obj, len - 1, kNewtRefUnbind);
}


/*------------------------------------------------------------------------*/
/** �}�b�v��[���R�s�[����
 *
 * @param dst		[out]�R�s�[��
 * @param pos		[i/o]�R�s�[�ʒu
 * @param src		[in] �R�s�[��
 *
 * @return			�Ȃ�
 */

void NewtDeeplyCopyMap(newtRef * dst, int32_t * pos, newtRefArg src)
{
    newtRefVar	superMap;
    newtRef *	slots;
    int32_t	len;
    int32_t	p;
    int32_t	i;

    superMap = NewtGetArraySlot(src, 0);
    len = NewtLength(src);

    if (NewtRefIsNotNIL(superMap))
        NewtDeeplyCopyMap(dst, pos, superMap);

    slots = NewtRefToSlots(src);
    p = *pos;

    for (i = 1; i < len; i++, p++)
    {
        dst[p] = slots[i];
    }

    *pos = p;
}


/*------------------------------------------------------------------------*/
/** �}�b�v��[���N���[����������
 *
 * @param map		[in] �}�b�v�I�u�W�F�N�g
 * @param len		[in] ����
 *
 * @return			�N���[���������ꂽ�I�u�W�F�N�g
 */

newtRef NewtDeeplyCloneMap(newtRefArg map, int32_t len)
{
    newtRefVar	newMap;
    int32_t	flags;
    int32_t	i = 1;

    flags = NewtRefToInteger(NcClassOf(map));
    newMap = NewtMakeMap(kNewtRefNIL, len, NULL);
    NcSetClass(newMap, NewtMakeInteger(flags));

    NewtDeeplyCopyMap(NewtRefToSlots(newMap), &i, map);

    return newMap;
}


/*------------------------------------------------------------------------*/
/** �t���[���̃I�u�W�F�N�g�f�[�^����X���b�g���폜����
 *
 * @param obj		[in] �t���[���̃I�u�W�F�N�g�f�[�^
 * @param slot		[in] �X���b�g�V���{��
 *
 * @return			�Ȃ�
 */

void NewtObjRemoveFrameSlot(newtObjRef obj, newtRefArg slot)
{
    uint32_t	i;

    if (NewtObjIsReadonly(obj))
    {
        NewtThrow0(kNErrObjectReadOnly);
        return;
    }

    if (NewtFindMapIndex(obj->as.map, slot, &i))
    {
        int32_t	mapIndex;

        mapIndex = NewtFindArrayIndex(obj->as.map, slot, 1);

        if (mapIndex == -1)
        {
            obj->as.map = NewtDeeplyCloneMap(obj->as.map, NewtObjSlotsLength(obj));
            mapIndex = NewtFindArrayIndex(obj->as.map, slot, 1);
        }
        else if (NewtRefIsLiteral(obj->as.map))
        {
            obj->as.map = NcClone(obj->as.map);
        }

        NewtObjRemoveArraySlot(obj, i);
        NewtObjRemoveArraySlot(NewtRefToPointer(obj->as.map), mapIndex);

        if (slot == NSSYM0(_proto))
            NewtClearMapFlags(obj->as.map, kNewtMapProto);
    }
}


/*------------------------------------------------------------------------*/
/** �t���[���܂��͔z��̃I�u�W�F�N�g�f�[�^����X���b�g�܂��͎w��ʒu�̗v�f���폜����
 *
 * @param obj		[in] �t���[���̃I�u�W�F�N�g�f�[�^
 * @param slot		[in] �X���b�g�V���{���^�ʒu
 *
 * @return			�Ȃ�
 */

void NewtObjRemoveSlot(newtObjRef obj, newtRefArg slot)
{
    if (NewtObjIsFrame(obj))
    {
        NewtObjRemoveFrameSlot(obj, slot);
    }
    else
    {
        int32_t	i;

        i = NewtRefToInteger(slot);
        NewtObjRemoveArraySlot(obj, i);
    }
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �}�b�v����w��ʒu�̃X���b�g�V���{������o��
 *
 * @param r			[in] �}�b�v�I�u�W�F�N�g
 * @param index		[in] �ʒu
 * @param indexP	[i/o]�}�b�v�S�̂���݂����݂̊J�n�ʒu
 *
 * @return			�X���b�g�V���{��
 */

newtRef NewtGetMapIndex(newtRefArg r, uint32_t index, uint32_t * indexP)
{
    newtRefVar	superMap;
    newtRefVar	v;
    int32_t	len;
    int32_t	n;

    superMap = NewtGetArraySlot(r, 0);

    if (NewtRefIsNIL(superMap))
    {
        *indexP = 0;
    }
    else
    {
        v = NewtGetMapIndex(superMap, index, indexP);

        if (v != kNewtRefUnbind)
            return v;
    }

    len = NewtArrayLength(r);
    n = index - *indexP;

    if (n < 0)
        return kNewtRefUnbind;

    if (n + 1 < len)
        return NewtGetArraySlot(r, n + 1);

    *indexP += len - 1;

    return kNewtRefUnbind;
}


/*------------------------------------------------------------------------*/
/** �z�񂩂�l����������
 *
 * @param r			[in] �z��
 * @param v			[in] �l�I�u�W�F�N�g
 * @param st		[in] �J�n�ʒu
 *
 * @retval			�ʒu		����
 * @retval			-1		���s
 */

int32_t NewtFindArrayIndex(newtRefArg r, newtRefArg v, uint16_t st)
{
    uint32_t	len;

    len = NewtArrayLength(r);

    if (st < len)
    {
        newtRef *	slots;

        slots = NewtRefToSlots(r);

        if (NewtMapIsSorted(r))
        {
            // �}�b�v���\�[�g����Ă���ꍇ...

            newtSymDataRef	sym;
            int32_t	index;
        
            sym = NewtRefToSymbol(v);

            if (NewtBSearchSymTable(r, sym->name, sym->hash, st, &index))
                return index;
        }
        else
        {
            uint32_t	i;

            for (i = st; i < len; i++)
            {
//                if (NewtRefEqual(slots[i], v))
                if (slots[i] == v)
                    return i;
            }
        }
    }

    return -1;
}


/*------------------------------------------------------------------------*/
/** �}�b�v����l����������
 *
 * @param r			[in] �}�b�v�I�u�W�F�N�g
 * @param v			[in] �X���b�g�V���{��
 * @param indexP	[out]�ʒu
 *
 * @retval			true	����
 * @retval			false	���s
 */

bool NewtFindMapIndex(newtRefArg r, newtRefArg v, uint32_t * indexP)
{
    newtRefVar	superMap;
    int32_t	i;

    superMap = NewtGetArraySlot(r, 0);

    if (NewtRefIsNIL(superMap))
    {
        *indexP = 0;
    }
    else
    {
        if (NewtFindMapIndex(superMap, v, indexP))
            return true;
    }

    i = NewtFindArrayIndex(r, v, 1);

    if (0 <= i)
    {
        *indexP += i - 1;
        return true;
    }

    *indexP += NewtArrayLength(r) - 1;

    return false;
}


/*------------------------------------------------------------------------*/
/** �t���[���I�u�W�F�N�g�̃}�b�v���擾
 *
 * @param r			[in] �t���[��
 *
 * @return			�}�b�v
 */

newtRef NewtFrameMap(newtRefArg r)
{
    newtObjRef	obj;

    obj = NewtRefToPointer(r);

    if (obj != NULL)
		return obj->as.map;
    else
        return kNewtRefNIL;
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �t���[���I�u�W�F�N�g����X���b�g�̈ʒu��T��
 *
 * @param frame		[in] �t���[��
 * @param slot		[in] �X���b�g�V���{��
 *
 * @retval			�X���b�g�̈ʒu		�݂������ꍇ
 * @retval			-1				�݂���Ȃ������ꍇ
 */

int32_t NewtFindSlotIndex(newtRefArg frame, newtRefArg slot)
{
    newtRefVar	map;
    uint32_t	i;

    map = NewtFrameMap(frame);

    if (NewtRefIsNIL(map))
        return -1;
    else if (NewtFindMapIndex(map, slot, &i))
        return i;
    else
        return -1;
}


/*------------------------------------------------------------------------*/
/** �t���[���I�u�W�F�N�g�� _proto �X���b�g�������`�F�b�N����
 *
 * @param frame		[in] �t���[��
 *
 * @retval			true	_proto �X���b�g������
 * @retval			false	_proto �X���b�g�������Ȃ�
 */

bool NewtHasProto(newtRefArg frame)
{
    newtObjRef obj;

    obj = NewtRefToPointer(frame);

    return NewtObjHasProto(obj);
}


/*------------------------------------------------------------------------*/
/** �t���[�����̃X���b�g�̗L���𒲂ׂ�
 *
 * @param frame		[in] �t���[��
 * @param slot		[in] �X���b�g�V���{��
 *
 * @retval			true	�X���b�g�����݂���
 * @retval			false	�X���b�g�����݂��Ȃ�
 */

bool NewtHasSlot(newtRefArg frame, newtRefArg slot)
{
    newtRefVar	map;
    uint32_t	i;

    map = NewtFrameMap(frame);

    if (NewtRefIsNIL(map))
        return false;
    else if (slot == NSSYM0(_proto))
        return NewtHasProto(frame);
    else
        return NewtFindMapIndex(map, slot, &i);
}


/*------------------------------------------------------------------------*/
/** �X���b�g�I�u�W�F�N�g�̃A�N�Z�X�p�X����l���擾����
 *
 * @param r			[in] �I�u�W�F�N�g
 * @param p			[in] �A�N�Z�X�p�X
 *
 * @return			�l�I�u�W�F�N�g
 */

newtRef NewtSlotsGetPath(newtRefArg r, newtRefArg p)
{
    if (NewtRefIsArray(r))
        return NewtGetArraySlot(r, p);
    else
        return NcFullLookup(r, p);
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g���̃A�N�Z�X�p�X�̗L���𒲂ׂ�
 *
 * @param r			[in] �I�u�W�F�N�g
 * @param p			[in] �A�N�Z�X�p�X
 *
 * @retval			true	�A�N�Z�X�p�X�����݂���
 * @retval			false	�A�N�Z�X�p�X�����݂��Ȃ�
 */

bool NewtHasPath(newtRefArg r, newtRefArg p)
{
    return (NcGetPath(r, p) != kNewtRefUnbind);
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̃A�N�Z�X�p�X�̒l���擾
 *
 * @param r			[in] �I�u�W�F�N�g
 * @param p			[in] �A�N�Z�X�p�X
 * @param slotP		[out]�X���b�g
 *
 * @return			�l�I�u�W�F�N�g
 */

newtRef NewtGetPath(newtRefArg r, newtRefArg p, newtRefVar * slotP)
{
    newtRefVar	v;

    v = r;

    if (NcClassOf(p) == NSSYM0(pathExpr))
//    if (NewtRefEqual(NcClassOf(p), NSSYM0(pathExpr)))
    {
        newtRefVar	path;
        int32_t	len;
        int32_t	i;

        len = NewtArrayLength(p);

        if (slotP != NULL)
            len--;

        for (i = 0; i < len; i++)
        {
            path = NewtGetArraySlot(p, i);
            v = NewtSlotsGetPath(v, path);

            if (v == kNewtRefUnbind)
                break;
        }

        if (slotP != NULL)
            *slotP = NewtGetArraySlot(p, len);
    }
    else
    {
        if (slotP != NULL)
            *slotP = p;
        else
            v = NewtSlotsGetPath(r, p);
    }

    return v;
}


/*------------------------------------------------------------------------*/
/** �o�C�i���I�u�W�F�N�g�̎w��ʒu����l���擾����
 *
 * @param r			[in] �o�C�i���I�u�W�F�N�g
 * @param p			[in] �ʒu
 *
 * @return			�l�I�u�W�F�N�g
 */

newtRef NewtGetBinarySlot(newtRefArg r, uint32_t p)
{
    uint32_t	len;

    len = NewtBinaryLength(r);

    if (p < len)
    {
        uint8_t *	data;
    
        data = NewtRefToBinary(r);
        return NewtMakeInteger(data[p]);
    }

    return kNewtRefUnbind;
}


/*------------------------------------------------------------------------*/
/** �o�C�i���I�u�W�F�N�g�̎w��ʒu�ɒl���Z�b�g����
 *
 * @param r			[in] �o�C�i���I�u�W�F�N�g
 * @param p			[in] �ʒu
 * @param v			[in] �l�I�u�W�F�N�g
 *
 * @return			�l�I�u�W�F�N�g
 */

newtRef NewtSetBinarySlot(newtRefArg r, uint32_t p, newtRefArg v)
{
    uint32_t	len;

    if (NewtRefIsReadonly(r))
        return NewtThrow(kNErrObjectReadOnly, r);

    len = NewtBinaryLength(r);

    if (p < len)
    {
        uint8_t *	data;
        int32_t	n;

        if (! NewtRefIsInteger(v))
            return NewtThrow(kNErrNotAnInteger, v);

        n = NewtRefToInteger(v);
        data = NewtRefToBinary(r);
        data[p] = n;
    }
    else
    {
        NewtErrOutOfBounds(r, p);
    }

    return v;
}


/*------------------------------------------------------------------------*/
/** ������̎w��ʒu���當�����擾
 *
 * @param r			[in] �����I�u�W�F�N�g
 * @param p			[in] �ʒu
 *
 * @return			�����I�u�W�F�N�g
 */

newtRef NewtGetStringSlot(newtRefArg r, uint32_t p)
{
    uint32_t	len;

    len = NewtStringLength(r);

    if (p < len)
    {
        char *	str;
    
        str = NewtRefToString(r);
        return NewtMakeCharacter(str[p]);
    }

    return kNewtRefUnbind;
}


/*------------------------------------------------------------------------*/
/** ������̎w��ʒu�ɕ������Z�b�g����
 *
 * @param r			[in] �����I�u�W�F�N�g
 * @param p			[in] �ʒu
 * @param v			[in] �����I�u�W�F�N�g
 *
 * @return			�����I�u�W�F�N�g
 */

newtRef NewtSetStringSlot(newtRefArg r, uint32_t p, newtRefArg v)
{
    uint32_t	slen;
    uint32_t	len;

    if (NewtRefIsReadonly(r))
        return NewtThrow(kNErrObjectReadOnly, r);

    slen = NewtStringLength(r);
	len = NewtBinaryLength(r);

    if (p + 1 < len)
    {
        char *	str;
        int		c;

        if (! NewtRefIsCharacter(v))
            return NewtThrow(kNErrNotACharacter, v);

        c = NewtRefToCharacter(v);
        str = NewtRefToString(r);
        str[p] = c;

		if (slen <= p)
		{	// �����񂪉��т��̂ŏI�[�������Z�b�g
			str[p + 1] = '\0';
		}
    }
    else
    {
        NewtErrOutOfBounds(r, p);
    }

    return v;
}


/*------------------------------------------------------------------------*/
/** �X���b�g�I�u�W�F�N�g�̎w��ʒu�g����l���擾
 *
 * @param r			[in] �I�u�W�F�N�g
 * @param p			[in] �ʒu
 *
 * @return			�l�I�u�W�F�N�g
 */

newtRef NewtSlotsGetSlot(newtRefArg r, uint32_t p)
{
    uint32_t	len;

    len = NewtSlotsLength(r);

    if (p < len)
    {
        newtRef *	slots;
    
        slots = NewtRefToSlots(r);
        return slots[p];
    }

    return kNewtRefUnbind;
}


/*------------------------------------------------------------------------*/
/** �X���b�g�I�u�W�F�N�g�ɂ̎w��ʒu�ɒl���Z�b�g����
 *
 * @param r			[in] �I�u�W�F�N�g
 * @param p			[in] �ʒu
 * @param v			[in] �l�I�u�W�F�N�g
 *
 * @return			�l�I�u�W�F�N�g
 */

newtRef NewtSlotsSetSlot(newtRefArg r, uint32_t p, newtRefArg v)
{
    uint32_t	len;

    len = NewtSlotsLength(r);

    if (p < len)
    {
        newtRef *	slots;
    
        slots = NewtRefToSlots(r);
        NewtGCHint(r[p], -1);
        slots[p] = v;
    }
    else
    {
        NewtErrOutOfBounds(r, p);
    }

    return v;
}


/*------------------------------------------------------------------------*/
/** �X���b�g�I�u�W�F�N�g�ɒl��}������
 *
 * @param r			[in] �I�u�W�F�N�g
 * @param p			[in] �ʒu
 * @param v			[in] �l�I�u�W�F�N�g
 *
 * @return			�l�I�u�W�F�N�g
 */

newtRef NewtSlotsInsertSlot(newtRefArg r, uint32_t p, newtRefArg v)
{
    newtRef *	slots;
    newtObjRef	obj;
    uint32_t	len;

    obj = NewtRefToPointer(r);
    len = NewtObjSlotsLength(obj);
    NewtObjSlotsSetLength(obj, len + 1, kNewtRefUnbind);

    slots = NewtRefToSlots(r);

    if (len < p)
        p = len;

    if (0 < len - p)
        memmove(slots + p + 1, slots + p, (len - p) * sizeof(newtRef));

    slots[p] = v;

    return v;
}


/*------------------------------------------------------------------------*/
/** �z��̎w��ʒu����l���擾����
 *
 * @param r			[in] �z��I�u�W�F�N�g
 * @param p			[in] �ʒu
 *
 * @return			�l�I�u�W�F�N�g
 */

newtRef NewtGetArraySlot(newtRefArg r, uint32_t p)
{
    return NewtSlotsGetSlot(r, p);
}


/*------------------------------------------------------------------------*/
/** �z��̎w��ʒu�ɒl���Z�b�g����
 *
 * @param r			[in] �z��I�u�W�F�N�g
 * @param p			[in] �ʒu
 * @param v			[in] �l�I�u�W�F�N�g
 *
 * @return			�l�I�u�W�F�N�g
 */

newtRef NewtSetArraySlot(newtRefArg r, uint32_t p, newtRefArg v)
{
    return NewtSlotsSetSlot(r, p, v);
}


/*------------------------------------------------------------------------*/
/** �z��̎w��ʒu�ɒl��}������
 *
 * @param r			[in] �z��I�u�W�F�N�g
 * @param p			[in] �ʒu
 * @param v			[in] �l�I�u�W�F�N�g
 *
 * @return			�l�I�u�W�F�N�g
 */

newtRef NewtInsertArraySlot(newtRefArg r, uint32_t p, newtRefArg v)
{
    return NewtSlotsInsertSlot(r, p, v);
}


/*------------------------------------------------------------------------*/
/** �t���[���̃X���b�g����l���擾����
 *
 * @param r			[in] �t���[��
 * @param p			[in] �X���b�g�V���{��
 *
 * @return			�l�I�u�W�F�N�g
 */

newtRef NewtGetFrameSlot(newtRefArg r, uint32_t p)
{
    return NewtSlotsGetSlot(r, p);
}


/*------------------------------------------------------------------------*/
/** �t���[���̃X���b�g�ɒl���Z�b�g����
 *
 * @param r			[in] �t���[��
 * @param p			[in] �X���b�g�V���{��
 * @param v			[in] �l�I�u�W�F�N�g
 *
 * @return			�l�I�u�W�F�N�g
 */

newtRef NewtSetFrameSlot(newtRefArg r, uint32_t p, newtRefArg v)
{
    return NewtSlotsSetSlot(r, p, v);
}


/**
 * Return the slot key for a given index.
 * This method can be used with/like NewtGetFrameSlot to iterate on the slots
 * (until we get optimized FOREACH/FOREACH_WITH_TAG/END_FOREACH macros).
 *
 * @param inFrame		frame to access the slot from
 * @param inIndex		index of the slot to return the key of
 * @return the key of the slot or unbind if there isn't that many slots.
 */

newtRef NewtGetFrameKey(newtRefArg inFrame, uint32_t inIndex)
{
	uint32_t start = 0;
	return NewtGetMapIndex(NewtFrameMap(inFrame), inIndex, &start);
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̎w�肳�ꂽ�ʒu����l���擾
 *
 * @param r			[in] �I�u�W�F�N�g
 * @param p			[in] �ʒu
 *
 * @return			�l�I�u�W�F�N�g
 */

newtRef NewtARef(newtRefArg r, uint32_t p)
{
    newtRefVar	v = kNewtRefNIL;

    switch (NewtGetRefType(r, true))
    {
        case kNewtArray:
            v = NewtGetArraySlot(r, p);
            break;

        case kNewtString:
            v = NewtGetStringSlot(r, p);
            break;

        case kNewtBinary:
            v = NewtGetBinarySlot(r, p);
            break;
    }

    return v;
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g�̎w�肳�ꂽ�ʒu�ɒl���Z�b�g����
 *
 * @param r			[in] �I�u�W�F�N�g
 * @param p			[in] �ʒu
 * @param v			[in] �l�I�u�W�F�N�g
 *
 * @return			�l�I�u�W�F�N�g
 */

newtRef NewtSetARef(newtRefArg r, uint32_t p, newtRefArg v)
{
    newtRefVar	result = kNewtRefUnbind;

    switch (NewtGetRefType(r, true))
    {
        case kNewtArray:
            result = NewtSetArraySlot(r, p, v);
            break;

        case kNewtString:
            result = NewtSetStringSlot(r, p, v);
            break;

        case kNewtBinary:
            result = NewtSetBinarySlot(r, p, v);
            break;
    }

    return result;
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �������ꂽ�ϐ��̕ۑ��ꏊ�ɒl���Z�b�g����
 *
 * @param start		[in] �J�n�I�u�W�F�N�g
 * @param name		[in] �ϐ����V���{��
 * @param value		[in] �l�I�u�W�F�N�g
 *
 * @retval			true	�l���Z�b�g�ł���
 * @retval			false	�l���Z�b�g�ł��Ȃ�����
 */

bool NewtAssignment(newtRefArg start, newtRefArg name, newtRefArg value)
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
            {
                NcSetSlot(current, name, value);
                return true;
            }
     
            current = NcGetSlot(current, NSSYM0(_proto));
        }

        left = NcGetSlot(left, NSSYM0(_parent));
    }

    return false;
}


/*------------------------------------------------------------------------*/
/** ���L�V�J���X�R�[�v�Ō������ꂽ�ϐ��̕ۑ��ꏊ�ɒl���Z�b�g����
 *
 * @param start		[in] �J�n�I�u�W�F�N�g
 * @param name		[in] �ϐ����V���{��
 * @param value		[in] �l�I�u�W�F�N�g
 *
 * @retval			true	�l���Z�b�g�ł���
 * @retval			false	�l���Z�b�g�ł��Ȃ�����
 */

bool NewtLexicalAssignment(newtRefArg start, newtRefArg name, newtRefArg value)
{
    newtRefVar	current = start;

    while (NewtRefIsNotNIL(current))
    {
		current = NcResolveMagicPointer(current);

		if (NewtRefIsMagicPointer(current))
			return kNewtRefUnbind;

        if (NewtHasSlot(current, name))
        {
            NcSetSlot(current, name, value);
            return true;
        }

        current = NcGetSlot(current, NSSYM0(_nextArgFrame));
    }

    return false;
}


/*------------------------------------------------------------------------*/
/** ���L�V�J���X�R�[�v�ŕϐ��̑��݂𒲂ׂ�
 *
 * @param start		[in] �J�n�I�u�W�F�N�g
 * @param name		[in] �ϐ����V���{��
 *
 * @retval			true	�ϐ�������
 * @retval			false	�ϐ����Ȃ�
 */

bool NewtHasLexical(newtRefArg start, newtRefArg name)
{
    newtRefVar	current = start;

    while (NewtRefIsNotNIL(current))
    {
		current = NcResolveMagicPointer(current);

		if (NewtRefIsMagicPointer(current))
			return false;

        if (NewtHasSlot(current, name))
            return true;

        current = NcGetSlot(current, NSSYM0(_nextArgFrame));
    }

    return false;
}


/*------------------------------------------------------------------------*/
/** �v���g�E�y�A�����g�p���Ńt���[�����̃X���b�g�̗L���𒲂ׂ�
 *
 * @param r			[in] �t���[��
 * @param name		[in] �X���b�g�V���{��
 *
 * @retval			true	�X���b�g�����݂���
 * @retval			false	�X���b�g�����݂��Ȃ�
 */

bool NewtHasVariable(newtRefArg r, newtRefArg name)
{
    newtRefVar	current;
    newtRefVar	left = r;

    while (NewtRefIsNotNIL(left))
    {
        current = left;

        while (NewtRefIsNotNIL(current))
        {
			if (NewtRefIsMagicPointer(current))
				return false;

            if (NewtHasSlot(current, name))
                return true;
    
            current = NcGetSlot(current, NSSYM0(_proto));
        }

        left = NcGetSlot(left, NSSYM0(_parent));
    }

    return false;
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �l�C�e�B�u�֐��̊֐��I�u�W�F�N�g����֐��̃|�C���^���擾����
 *
 * @param r			[in] �֐��I�u�W�F�N�g
 *
 * @return			�֐��̃|�C���^
 */

void * NewtRefToNativeFn(newtRefArg r)
{
    newtRefVar	fn;

    fn = NcGetSlot(r, NSSYM0(funcPtr));

    if (NewtRefIsInteger(fn))
        return (void *)NewtRefToInteger(fn);
    else
        return NULL;
}


/*------------------------------------------------------------------------*/
/** �l�C�e�B�u�֐��ircvr�Ȃ��j�̊֐��I�u�W�F�N�g���쐬����
 *
 * @param funcPtr		[in] �֐��̃|�C���^
 * @param numArgs		[in] �����̐�
 * @param indefinite	[in] �s�蒷�t���O
 * @param doc			[in] ������
 *
 * @return				�֐��I�u�W�F�N�g
 */

newtRef NewtMakeNativeFn0(void * funcPtr, uint32_t numArgs, bool indefinite, char * doc)
{
    newtRefVar	fnv[] = {
                            NS_CLASS,			NSSYM0(_function.native0),
                            NSSYM0(funcPtr),	kNewtRefNIL,
                            NSSYM0(numArgs),	kNewtRefNIL,
                            NSSYM0(indefinite),	kNewtRefNIL,
                            NSSYM0(docString),	kNewtRefNIL,
                        };

    newtRefVar	fn;

    // function
    fn = NewtMakeFrame2(sizeof(fnv) / (sizeof(newtRefVar) * 2), fnv);

    NcSetSlot(fn, NSSYM0(funcPtr), NewtMakeAddress(funcPtr));
    NcSetSlot(fn, NSSYM0(numArgs), NewtMakeInteger(numArgs));
    NcSetSlot(fn, NSSYM0(indefinite), NewtMakeBoolean(indefinite));
    NcSetSlot(fn, NSSYM0(docString), NSSTRCONST(doc));

    return fn;
}


/*------------------------------------------------------------------------*/
/** �l�C�e�B�u�֐��ircvr�Ȃ��j�̃O���[�o���֐���o�^����
 *
 * @param sym			[in] �O���[�o���֐���
 * @param funcPtr		[in] �֐��̃|�C���^
 * @param numArgs		[in] �����̐�
 * @param indefinite	[in] �s�蒷�t���O
 * @param doc			[in] ������
 *
 * @return				�֐��I�u�W�F�N�g
 */

newtRef NewtDefGlobalFn0(newtRefArg sym, void * funcPtr, uint32_t numArgs, bool indefinite, char * doc)
{
    newtRefVar	fn;

    fn = NewtMakeNativeFn0(funcPtr, numArgs, indefinite, doc);
    return NcDefGlobalFn(sym, fn);
}


/*------------------------------------------------------------------------*/
/** �l�C�e�B�u�֐��ircvr����j�̊֐��I�u�W�F�N�g���쐬����
 *
 * @param funcPtr		[in] �֐��̃|�C���^
 * @param numArgs		[in] �����̐�
 * @param indefinite	[in] �s�蒷�t���O
 * @param doc			[in] ������
 *
 * @return				�֐��I�u�W�F�N�g
 */

newtRef NewtMakeNativeFunc0(void * funcPtr, uint32_t numArgs, bool indefinite, char * doc)
{
    newtRefVar	fnv[] = {
                            NS_CLASS,			NSSYM0(_function.native),
                            NSSYM0(funcPtr),	kNewtRefNIL,
                            NSSYM0(numArgs),	kNewtRefNIL,
                            NSSYM0(indefinite),	kNewtRefNIL,
                            NSSYM0(docString),	kNewtRefNIL,
                        };

    newtRefVar	fn;

    // function
    fn = NewtMakeFrame2(sizeof(fnv) / (sizeof(newtRefVar) * 2), fnv);

    NcSetSlot(fn, NSSYM0(funcPtr), NewtMakeAddress(funcPtr));
    NcSetSlot(fn, NSSYM0(numArgs), NewtMakeInteger(numArgs));
    NcSetSlot(fn, NSSYM0(indefinite), NewtMakeBoolean(indefinite));
    NcSetSlot(fn, NSSYM0(docString), NSSTRCONST(doc));

    return fn;
}


/*------------------------------------------------------------------------*/
/** �l�C�e�B�u�֐��ircvr����j�̃O���[�o���֐���o�^����
 *
 * @param sym			[in] �O���[�o���֐���
 * @param funcPtr		[in] �֐��̃|�C���^
 * @param numArgs		[in] �����̐�
 * @param indefinite	[in] �s�蒷�t���O
 * @param doc			[in] ������
 *
 * @return				�֐��I�u�W�F�N�g
 */

newtRef NewtDefGlobalFunc0(newtRefArg sym, void * funcPtr, uint32_t numArgs, bool indefinite, char * doc)
{
    newtRefVar	fn;

    fn = NewtMakeNativeFunc0(funcPtr, numArgs, indefinite, doc);
    return NcDefGlobalFn(sym, fn);
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** ������̑O����������������ƈ�v���邩�`�F�b�N����
 *
 * @param str		[in] ������
 * @param len		[in] ������̒���
 * @param sub		[in] ����������
 * @param sublen	[in] ����������̒���
 *
 * @retval			true	�O����������������ƈ�v����
 * @retval			false	�O����������������ƈ�v���Ȃ�
 */

bool NewtStrNBeginsWith(char * str, uint32_t len, char * sub, uint32_t sublen)
{
    while (0 < len && 0 < sublen && *str != '\0' && *sub != '\0')
    {
        if (*str != *sub)
            return false;

        str++; len--;
        sub++; sublen--;
    }

    return (sublen == 0 || *sub == '\0');
}


/*------------------------------------------------------------------------*/
/** sub �� supr �̃T�u�N���X�����`�F�b�N����
 *
 * @param sub		[in] �T�u�N���X������
 * @param sublen	[in] �T�u�X�[�p�N���X������̒���
 * @param supr		[in] �X�[�p�N���X������
 * @param suprlen	[in] �X�[�p�N���X������̒���
 *
 * @retval			true	�T�u�N���X
 * @retval			false	�T�u�N���X�łȂ�
 */

bool NewtStrIsSubclass(char * sub, uint32_t sublen, char * supr, uint32_t suprlen)
{
    if (sublen == suprlen)
        return (strncasecmp(sub, supr, suprlen) == 0);

    if (sublen < suprlen)
        return false;

    if (sub[suprlen] != '.')
        return false;

    return NewtStrNBeginsWith(sub, sublen, supr, suprlen);
}


/*------------------------------------------------------------------------*/
/** sub �� supr �̃T�u�N���X���܂ނ��`�F�b�N����
 *
 * @param sub		[in] �T�u�N���X������
 * @param sublen	[in] �T�u�X�[�p�N���X������̒���
 * @param supr		[in] �X�[�p�N���X������
 * @param suprlen	[in] �X�[�p�N���X������̒���
 *
 * @retval			true	�T�u�N���X���܂�
 * @retval			false	�T�u�N���X���܂܂Ȃ�
 */

bool NewtStrHasSubclass(char * sub, uint32_t sublen, char * supr, uint32_t suprlen)
{
    char *	last;
    char *	w;

    last = sub + sublen;

    do {
        w = strchr(sub, ';');
        if (w == NULL) break;

        if (NewtStrIsSubclass(sub, w - sub, supr, suprlen))
            return true;

        sub = w + 1;
        sublen = last - sub;
    } while (true);

    return NewtStrIsSubclass(sub, sublen, supr, suprlen);
}


/*------------------------------------------------------------------------*/
/** sub �� supr �̃T�u�N���X���܂ނ��`�F�b�N����
 *
 * @param sub		[in] �T�u�N���X
 * @param supr		[in] �X�[�p�N���X
 *
 * @retval			true	�T�u�N���X���܂�
 * @retval			false	�T�u�N���X���܂܂Ȃ�
 */

bool NewtHasSubclass(newtRefArg sub, newtRefArg supr)
{
    newtSymDataRef	subSym;
    newtSymDataRef	suprSym;

    if (! NewtRefIsSymbol(sub)) return false;
    if (! NewtRefIsSymbol(supr)) return false;
    if (sub == supr) return true;

    subSym = NewtRefToSymbol(sub);
    suprSym = NewtRefToSymbol(supr);

    return NewtStrHasSubclass(subSym->name, NewtSymbolLength(sub),
                suprSym->name, NewtSymbolLength(supr));
}


/*------------------------------------------------------------------------*/
/** sub �� supr �̃T�u�N���X���`�F�b�N����
 *
 * @param sub		[in] �V���{���I�u�W�F�N�g�P
 * @param supr		[in] �V���{���I�u�W�F�N�g�Q
 *
 * @retval			true	�T�u�N���X
 * @retval			false	�T�u�N���X�łȂ�
 */

bool NewtIsSubclass(newtRefArg sub, newtRefArg supr)
{
    newtSymDataRef	subSym;
    newtSymDataRef	suprSym;

    if (! NewtRefIsSymbol(sub)) return false;
    if (! NewtRefIsSymbol(supr)) return false;
    if (sub == supr) return true;

    subSym = NewtRefToSymbol(sub);
    suprSym = NewtRefToSymbol(supr);

    return NewtStrIsSubclass(subSym->name, NewtSymbolLength(sub),
                suprSym->name, NewtSymbolLength(supr));
}


/*------------------------------------------------------------------------*/
/** obj �� r �̃C���X�^���X���`�F�b�N����
 *
 * @param obj		[in] �I�u�W�F�N�g
 * @param r			[in] �N���X�V���{��
 *
 * @retval			true	�C���X�^���X
 * @retval			false	�C���X�^���X�łȂ�
 */

bool NewtIsInstance(newtRefArg obj, newtRefArg r)
{
    return NewtIsSubclass(NcClassOf(obj), r);
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** ������I�u�W�F�N�g�̍Ō�ɕ������ǉ�����
 *
 * @param r			[in] ������I�u�W�F�N�g
 * @param s			[in] �ǉ����镶����
 *
 * @return			������I�u�W�F�N�g
 */

newtRef NewtStrCat(newtRefArg r, char * s)
{
    if (NewtRefIsPointer(r))
		return NewtStrCat2(r, s, strlen(s));
	else
		return r;
}


/*------------------------------------------------------------------------*/
/** ������I�u�W�F�N�g�̍Ō�Ɏw�肳�ꂽ�����̕������ǉ�����
 *
 * @param r			[in] ������I�u�W�F�N�g
 * @param s			[in] �ǉ����镶����
 * @param slen		[in] �ǉ����镶����̒���
 *
 * @return			������I�u�W�F�N�g
 */

newtRef NewtStrCat2(newtRefArg r, char * s, uint32_t slen)
{
    newtObjRef	obj;

    obj = NewtRefToPointer(r);

    if (obj != NULL)
    {
        uint32_t	tgtlen;
        uint32_t	dstlen;

		tgtlen = NewtObjStringLength(obj);
		dstlen = tgtlen + slen;

		if (NewtObjSize(obj) <= dstlen)
			obj = NewtObjStringSetLength(obj, dstlen);

        if (obj != NULL)
        {
            char *	data;

            data = NewtObjToString(obj);
            memcpy(data + tgtlen, s, slen);
			data[dstlen] = '\0';
        }
    }

    return r;
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** ���ϐ��̎擾
 *
 * @param s			[in] ������
 *
 * @return			������I�u�W�F�N�g
 */

newtRef NewtGetEnv(const char * s)
{
	char *  v;

	v = getenv(s);

	if (v != NULL)
		return NSSTRCONST(v);
	else
		return kNewtRefUnbind;
}
