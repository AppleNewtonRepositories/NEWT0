/*------------------------------------------------------------------------*/
/**
 * @file	NewtPrint.c
 * @brief   �v�����g�֌W
 *
 * @author  M.Nukui
 * @date	2005-04-11
 *
 * Copyright (C) 2003-2005 M.Nukui All rights reserved.
 */


/* �w�b�_�t�@�C�� */
#include <ctype.h>

#include "NewtPrint.h"
#include "NewtCore.h"
#include "NewtObj.h"
#include "NewtEnv.h"
#include "NewtIO.h"


/* �֐��v���g�^�C�v */

static int32_t		NewtGetPrintLength(void);
static int32_t		NewtGetPrintDepth(void);

static bool			NewtSymbolIsPrint(char * str, int len);
static bool			NewtStrIsPrint(char * str, int len);
static char *		NewtCharToEscape(int c);

static void			NIOPrintEscapeStr(newtStream_t * f, char * str, int len);
static void			NIOPrintRef(newtStream_t * f, newtRefArg r);
static void			NIOPrintSpecial(newtStream_t * f, newtRefArg r);
static void			NIOPrintInteger(newtStream_t * f, newtRefArg r);
static void			NIOPrintReal(newtStream_t * f, newtRefArg r);
static void			NIOPrintObjCharacter(newtStream_t * f, newtRefArg r);
static void			NIOPrintObjMagicPointer(newtStream_t * f, newtRefArg r);
static void			NIOPrintObjBinary(newtStream_t * f, newtRefArg r);
static void			NIOPrintObjSymbol(newtStream_t * f, newtRefArg r);
static void			NIOPrintObjString(newtStream_t * f, newtRefArg r);
static void			NIOPrintObjArray(newtStream_t * f, newtRefArg r, int32_t depth, bool literal);
static void			NIOPrintFnFrame(newtStream_t * f, newtRefArg r);
static void			NIOPrintRegexFrame(newtStream_t * f, newtRefArg r);
static void			NIOPrintObjFrame(newtStream_t * f, newtRefArg r, int32_t depth, bool literal);
static void			NIOPrintLiteral(newtStream_t * f, newtRefArg r, bool * literalP);
static void			NIOPrintObj2(newtStream_t * f, newtRefArg r, int32_t depth, bool literal);

static void			NIOPrintCharacter(newtStream_t * f, newtRefArg r);
static void			NIOPrintSymbol(newtStream_t * f, newtRefArg r);
static void			NIOPrintString(newtStream_t * f, newtRefArg r);
static void			NIOPrintArray(newtStream_t * f, newtRefArg r);
static void			NIOPrintObj(newtStream_t * f, newtRefArg r);
static void			NIOPrint(newtStream_t * f, newtRefArg r);

static void			NIOInfo(newtStream_t * f, newtRefArg r);


#pragma mark -
/*------------------------------------------------------------------------*/
/** �z��܂��̓t���[���̃v�����g�\�Ȓ�����Ԃ�
 *
 * @return			�v�����g�\�Ȓ���
 */

int32_t NewtGetPrintLength(void)
{
	newtRefVar	n;
	int32_t		printLength = -1;

	n = NcGetGlobalVar(NSSYM0(printLength));

	if (NewtRefIsInteger(n))
		printLength = NewtRefToInteger(n);

	if (printLength < 0)
		printLength = 0x7fffffff;

	return printLength;
}


/*------------------------------------------------------------------------*/
/** �z��܂��̓t���[���̃v�����g�\�Ȑ[����Ԃ�
 *
 * @return			�v�����g�\�Ȑ[��
 */

int32_t NewtGetPrintDepth(void)
{
    newtRefVar	n;
    int32_t		depth = 3;

    n = NcGetGlobalVar(NSSYM0(printDepth));

    if (NewtRefIsInteger(n))
        depth = NewtRefToInteger(n);

	return depth;
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �V���{�������񂪕\���\�����ׂ�
 *
 * @param str		[in] �V���{��������
 * @param len		[in] ������̒���
 *
 * @retval			true	�\���\
 * @retval			false	�\���s��
 */

bool NewtSymbolIsPrint(char * str, int len)
{
	int c;
	int i;

	if (len == 0)
		return false;

	c = str[0];

	if ('0' <= c && c <= '9')
		return false;

	for (i = 0; i < len && str[i]; i++)
	{
		c = str[i];

		if (c == '_') continue;
		if ('a' <= c && c <= 'z') continue;
		if ('A' <= c && c <= 'Z') continue;
		if ('0' <= c && c <= '9') continue;

		return false;
	}

	return true;
}


/*------------------------------------------------------------------------*/
/** �����񂪕\���\�����ׂ�
 *
 * @param str		[in] ������
 * @param len		[in] ������̒���
 *
 * @retval			true	�\���\
 * @retval			false	�\���s��
 */

bool NewtStrIsPrint(char * str, int len)
{
	int		i;

	for (i = 0; i < len && str[i]; i++)
	{
		if (str[i] == '"')
			return false;

		if (! isprint(str[i]))
			return false;
	}

	return true;
}


/*------------------------------------------------------------------------*/
/** �������G�X�P�[�v������ɕϊ�����
 *
 * @param c			[in] ����
 *
 * @return			�G�X�P�[�v������
 */

char * NewtCharToEscape(int c)
{
	char *	s = NULL;

	switch (c)
	{
		case '\n':
			s = "\\n";
			break;

		case '\r':
			s = "\\r";
			break;

		case '\t':
			s = "\\t";
			break;

		case '"':
			s = "\\\"";
			break;
	}

	return s;
}


/*------------------------------------------------------------------------*/
/** ��������G�X�P�[�v�Ńv�����g����
 *
 * @param f			[in] �o�̓t�@�C��
 * @param str		[in] ������
 * @param len		[in] ������̒���
 *
 * @return			�Ȃ�
 *
 * @note			newtStream_t ���g�p
 */

void NIOPrintEscapeStr(newtStream_t * f, char * str, int len)
{
	bool	unicode = false;
	char *	s;
	int		c;
	int		i;

	for (i = 0; i < len; i++)
	{
		c = str[i];

		s = NewtCharToEscape(c);

		if (s != NULL)
		{
			if (unicode)
			{
				NIOFputs("\\u", f);
				unicode = false;
			}

			NIOFputs(s, f);
		}
		else if (isprint(c))
		{
			if (unicode)
			{
				NIOFputs("\\u", f);
				unicode = false;
			}

			NIOFputc(c, f);
		}
		else
		{
			if (! unicode)
			{
				NIOFputs("\\u", f);
				unicode = true;
			}

			NIOFprintf(f, "%02x%02x", c, str[++i]);
		}
	}
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �o�̓t�@�C���ɃI�u�W�F�N�g�Q�Ƃ��P�U�i���Ńv�����g����
 *
 * @param f			[in] �o�̓t�@�C��
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�Ȃ�
 *
 * @note			newtStream_t ���g�p
 */

void NIOPrintRef(newtStream_t * f, newtRefArg r)
{
    NIOFprintf(f, "#%08x", r);
}


/*------------------------------------------------------------------------*/
/** �o�̓t�@�C���ɓ���I�u�W�F�N�g���v�����g����
 *
 * @param f			[in] �o�̓t�@�C��
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�Ȃ�
 *
 * @note			newtStream_t ���g�p
 */

void NIOPrintSpecial(newtStream_t * f, newtRefArg r)
{
    int	n;

    n = NewtRefToSpecial(r);
    NIOFprintf(f, "<Special, %04x>", n);
}


/*------------------------------------------------------------------------*/
/** �o�̓t�@�C���ɐ����I�u�W�F�N�g���v�����g����
 *
 * @param f			[in] �o�̓t�@�C��
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�Ȃ�
 *
 * @note			newtStream_t ���g�p
 */

void NIOPrintInteger(newtStream_t * f, newtRefArg r)
{
    int	n;

    n = NewtRefToInteger(r);
    NIOFprintf(f, "%d", n);
}


/*------------------------------------------------------------------------*/
/** �o�̓t�@�C���ɕ��������_�I�u�W�F�N�g���v�����g����
 *
 * @param f			[in] �o�̓t�@�C��
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�Ȃ�
 *
 * @note			newtStream_t ���g�p
 */

void NIOPrintReal(newtStream_t * f, newtRefArg r)
{
    double	n;

    n = NewtRefToReal(r);
    NIOFprintf(f, "%f", n);
}


/*------------------------------------------------------------------------*/
/** �o�̓t�@�C���ɕ����I�u�W�F�N�g���v�����g����
 *
 * @param f			[in] �o�̓t�@�C��
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�Ȃ�
 *
 * @note			newtStream_t ���g�p
 */

void NIOPrintObjCharacter(newtStream_t * f, newtRefArg r)
{
	char *	s;
    int		c;

    c = NewtRefToCharacter(r);

	NIOFputc('$', f);

	s = NewtCharToEscape(c);

	if (s != NULL)
		NIOFputs(s, f);
	else if (0xff00 & c)
		NIOFprintf(f, "\\u%04x", c);
	else if (isprint(c))
		NIOFputc(c, f);
	else
		NIOFprintf(f, "\\%02x", c);
}


#ifdef __NAMED_MAGIC_POINTER__

/*------------------------------------------------------------------------*/
/** �o�̓t�@�C���Ƀ}�W�b�N�|�C���^���v�����g����
 *
 * @param f			[in] �o�̓t�@�C��
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�Ȃ�
 *
 * @note			newtStream_t ���g�p
 */

void NIOPrintObjMagicPointer(newtStream_t * f, newtRefArg r)
{
	newtRefVar	sym;

	sym = NewtMPToSymbol(r);

	NIOFputc('@', f);
	NIOPrintObjSymbol(f, sym);
}

#else

/*------------------------------------------------------------------------*/
/** �o�̓t�@�C���Ƀ}�W�b�N�|�C���^���v�����g����
 *
 * @param f			[in] �o�̓t�@�C��
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�Ȃ�
 *
 * @note			newtStream_t ���g�p
 */

void NIOPrintObjMagicPointer(newtStream_t * f, newtRefArg r)
{
	int32_t	table;
	int32_t	index;

	table = NewtMPToTable(r);
	index = NewtMPToIndex(r);

    NIOFprintf(f, "@%d", index);
}

#endif

/*------------------------------------------------------------------------*/
/** �o�̓t�@�C���Ƀo�C�i���I�u�W�F�N�g���v�����g����
 *
 * @param f			[in] �o�̓t�@�C��
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�Ȃ�
 *
 * @note			newtStream_t ���g�p
 */

void NIOPrintObjBinary(newtStream_t * f, newtRefArg r)
{
    newtRefVar	klass;
    int	ptr;
    int	len;

    ptr = r;
    len = NewtBinaryLength(r);
    NIOFputs("<Binary, ", f);

    //
    klass = NcClassOf(r);

    if (NewtRefIsSymbol(klass))
    {
        NIOFputs("class \"", f);
        NIOPrintObj2(f, klass, 0, true);
        NIOFputs("\", ", f);
    }

    NIOFprintf(f, "length %d>", len);
}


/*------------------------------------------------------------------------*/
/** �o�̓t�@�C���ɃV���{���I�u�W�F�N�g���v�����g����
 *
 * @param f			[in] �o�̓t�@�C��
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�Ȃ�
 *
 * @note			newtStream_t ���g�p
 */

void NIOPrintObjSymbol(newtStream_t * f, newtRefArg r)
{
    newtSymDataRef	sym;
	int		len;

    sym = NewtRefToSymbol(r);
	len = NewtSymbolLength(r);

	if (NewtSymbolIsPrint(sym->name, len))
	{
		NIOFputs(sym->name, f);
	}
	else
	{
		NIOFputc('|', f);
		NIOPrintEscapeStr(f, sym->name, len);
		NIOFputc('|', f);
	}
}


/*------------------------------------------------------------------------*/
/** �o�̓t�@�C���ɕ�����I�u�W�F�N�g���v�����g����
 *
 * @param f			[in] �o�̓t�@�C��
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�Ȃ�
 *
 * @note			newtStream_t ���g�p
 */

void NIOPrintObjString(newtStream_t * f, newtRefArg r)
{
    char *	s;
	int		len;

    s = NewtRefToString(r);
	len = NewtStringLength(r);

	if (NewtStrIsPrint(s, len))
	{
//		NIOFprintf(f, "\"%s\"", s);
		NIOFputs("\"", f);
		NIOFputs(s, f);
		NIOFputs("\"", f);
	}
	else
	{
		NIOFputc('"', f);
		NIOPrintEscapeStr(f, s, len);
		NIOFputc('"', f);
	}
}


/*------------------------------------------------------------------------*/
/** �o�̓t�@�C���ɔz��I�u�W�F�N�g���v�����g����
 *
 * @param f			[in] �o�̓t�@�C��
 * @param r			[in] �I�u�W�F�N�g
 * @param depth		[in] �[��
 * @param literal	[in] ���e�����t���O
 *
 * @return			�Ȃ�
 *
 * @note			newtStream_t ���g�p
 */

void NIOPrintObjArray(newtStream_t * f, newtRefArg r, int32_t depth, bool literal)
{
    newtObjRef	obj;
    newtRef *	slots;
    newtRefVar	klass;
    uint32_t	len;
    uint32_t	i;

    obj = NewtRefToPointer(r);
    len = NewtObjSlotsLength(obj);
    slots = NewtObjToSlots(obj);

    if (literal && NcClassOf(r) == NSSYM0(pathExpr))
    {
        for (i = 0; i < len; i++)
        {
            if (0 < i)
                NIOFputs(".", f);
    
            NIOPrintObj2(f, slots[i], 0, literal);
        }
    }
    else
    {
        NIOFputs("[", f);
    
        klass = NcClassOf(r);
    
        if (NewtRefIsNotNIL(klass) && ! NewtRefEqual(klass, NSSYM0(array)))
        {
            NIOPrintObj2(f, klass, 0, true);
            NIOFputs(": ", f);
        }
    
        if (depth < 0)
        {
            NIOPrintRef(f, r);
        }
        else
        {
			int32_t	printLength;

            depth--;
			printLength = NewtGetPrintLength();

            for (i = 0; i < len; i++)
            {
                if (0 < i)
                    NIOFputs(", ", f);
        
				if (printLength <= i)
				{
					NIOFputs("...", f);
					break;
				}

                NIOPrintObj2(f, slots[i], depth, literal);
            }
        }

        NIOFputs("]", f);
    }
}


/*------------------------------------------------------------------------*/
/** �o�̓t�@�C���Ɋ֐��I�u�W�F�N�g���v�����g����
 *
 * @param f			[in] �o�̓t�@�C��
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�Ȃ�
 *
 * @note			newtStream_t ���g�p
 */

void NIOPrintFnFrame(newtStream_t * f, newtRefArg r)
{
	newtRefVar	indefinite;
    int32_t		numArgs;
	char *		indefiniteStr = "";

    numArgs = NewtRefToInteger(NcGetSlot(r, NSSYM0(numArgs)));
	indefinite = NcGetSlot(r, NSSYM0(indefinite));

	if (NewtRefIsNotNIL(indefinite))
		indefiniteStr = "...";

	NIOFprintf(f, "<function, %d arg(s)%s #%08x>", numArgs, indefiniteStr, r);
}


/*------------------------------------------------------------------------*/
/** �o�̓t�@�C���ɐ��K�\���I�u�W�F�N�g���v�����g����
 *
 * @param f			[in] �o�̓t�@�C��
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�Ȃ�
 *
 * @note			newtStream_t ���g�p
 */

void NIOPrintRegexFrame(newtStream_t * f, newtRefArg r)
{
	newtRefVar	pattern;
	newtRefVar	option;

	pattern = NcGetSlot(r, NSSYM0(pattern));
	option = NcGetSlot(r, NSSYM0(option));

//    NIOFprintf(f, "/%s/", NewtRefToString(pattern));
    NIOFputs("/", f);
    NIOFputs(NewtRefToString(pattern), f);
    NIOFputs("/", f);

	if (NewtRefIsString(option))
		NIOFputs(NewtRefToString(option), f);
}


/*------------------------------------------------------------------------*/
/** �o�̓t�@�C���Ƀt���[���I�u�W�F�N�g���v�����g����
 *
 * @param f			[in] �o�̓t�@�C��
 * @param r			[in] �I�u�W�F�N�g
 * @param depth		[in] �[��
 * @param literal	[in] ���e�����t���O
 *
 * @return			�Ȃ�
 *
 * @note			newtStream_t ���g�p
 */

void NIOPrintObjFrame(newtStream_t * f, newtRefArg r, int32_t depth, bool literal)
{
    newtObjRef	obj;
    newtRef *	slots;
    newtRefVar	slot;
    uint32_t	index;
    uint32_t	len;
    uint32_t	i;

    if (NewtRefIsFunction(r) && ! NEWT_DUMPBC)
    {
        NIOPrintFnFrame(f, r);
        return;
    }

    if (NewtRefIsRegex(r) && ! NEWT_DUMPBC)
    {
        NIOPrintRegexFrame(f, r);
        return;
    }

    obj = NewtRefToPointer(r);
    len = NewtObjSlotsLength(obj);
    slots = NewtObjToSlots(obj);

    NIOFputs("{", f);

    if (depth < 0)
    {
        NIOPrintRef(f, r);
    }
    else
    {
		int32_t	printLength;

        depth--;
		printLength = NewtGetPrintLength();

        for (i = 0; i < len; i++)
        {
            if (0 < i)
                NIOFputs(", ", f);

			if (printLength <= i)
			{
				NIOFputs("...", f);
				break;
			}

            slot = NewtGetMapIndex(obj->as.map, i, &index);
			if (slot == kNewtRefUnbind) break;

            NIOPrintObjSymbol(f, slot);
            NIOFputs(": ", f);
            NIOPrintObj2(f, slots[i], depth, literal);
        }
    }

    NIOFputs("}", f);
}


/*------------------------------------------------------------------------*/
/** �o�̓t�@�C���Ƀ��e�����̈���v�����g����
 *
 * @param f			[in] �o�̓t�@�C��
 * @param r			[in] �I�u�W�F�N�g
 * @param literalP	[i/o]���e�����t���O
 *
 * @return			�Ȃ�
 *
 * @note			newtStream_t ���g�p
 */

void NIOPrintLiteral(newtStream_t * f, newtRefArg r, bool * literalP)
{
    if (! *literalP && NewtRefIsLiteral(r))
    {
		NIOFputc('\'', f);
        *literalP = true;
    }
}


/*------------------------------------------------------------------------*/
/** �o�̓t�@�C���ɃI�u�W�F�N�g���v�����g�i�ċA�ďo���p�j
 *
 * @param f			[in] �o�̓t�@�C��
 * @param r			[in] �I�u�W�F�N�g
 * @param depth		[in] �[��
 * @param literal	[in] ���e�����t���O
 *
 * @return			�Ȃ�
 *
 * @note			newtStream_t ���g�p
 */

void NIOPrintObj2(newtStream_t * f, newtRefArg r, int32_t depth, bool literal)
{
    switch (NewtGetRefType(r, true))
    {
        case kNewtNil:
            NIOFputs("NIL", f);
            break;

        case kNewtTrue:
            NIOFputs("TRUE", f);
            break;

        case kNewtUnbind:
            NIOFputs("#UNBIND", f);
            break;

        case kNewtSpecial:
            NIOPrintSpecial(f, r);
            break;

        case kNewtInt30:
        case kNewtInt32:
            NIOPrintInteger(f, r);
            break;

        case kNewtReal:
            NIOPrintReal(f, r);
            break;

        case kNewtCharacter:
            NIOPrintObjCharacter(f, r);
            break;

        case kNewtMagicPointer:
			if (0 <= depth)
			{
				newtRefVar	r2;

				r2 = NcResolveMagicPointer(r);

				if (! NewtRefIsMagicPointer(r2))
				{
					NIOPrintObj2(f, r2, depth, literal);
					break;
				}
			}

			NIOPrintObjMagicPointer(f, r);
            break;

        case kNewtBinary:
            NIOPrintObjBinary(f, r);
            break;

        case kNewtArray:
            NIOPrintLiteral(f, r, &literal);
            NIOPrintObjArray(f, r, depth, literal);
            break;

        case kNewtFrame:
            NIOPrintLiteral(f, r, &literal);
            NIOPrintObjFrame(f, r, depth, literal);
            break;

        case kNewtSymbol:
            NIOPrintLiteral(f, r, &literal);
            NIOPrintObjSymbol(f, r);
            break;

        case kNewtString:
            NIOPrintObjString(f, r);
            break;

        default:
            NIOFputs("###UNKNOWN###", f);
            break;
    }
}


/*------------------------------------------------------------------------*/
/** �o�̓t�@�C���ɃI�u�W�F�N�g���v�����g
 *
 * @param f			[in] �o�̓t�@�C��
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�Ȃ�
 *
 * @note			newtStream_t ���g�p
 */

void NIOPrintObj(newtStream_t * f, newtRefArg r)
{
    NIOPrintObj2(f, r, NewtGetPrintDepth(), false);
}


/*------------------------------------------------------------------------*/
/** �o�̓t�@�C���ɃI�u�W�F�N�g���v�����g
 *
 * @param f			[in] �o�̓t�@�C��
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�Ȃ�
 */

void NewtPrintObj(FILE * f, newtRefArg r)
{
	newtStream_t	stream;

	NIOSetFile(&stream, f);
    NIOPrintObj(&stream, r);
}


/*------------------------------------------------------------------------*/
/** �o�̓t�@�C���ɃI�u�W�F�N�g���v�����g�i���s����j
 *
 * @param f			[in] �o�̓t�@�C��
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�Ȃ�
 */

void NewtPrintObject(FILE * f, newtRefArg r)
{
	newtStream_t	stream;

	NIOSetFile(&stream, f);

    NIOPrintObj(&stream, r);
    NIOFputs("\n", &stream);
}


/*------------------------------------------------------------------------*/
/** �o�̓t�@�C���ɕ����I�u�W�F�N�g���v�����g����
 *
 * @param f			[in] �o�̓t�@�C��
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�Ȃ�
 *
 * @note			newtStream_t ���g�p
 */

void NIOPrintCharacter(newtStream_t * f, newtRefArg r)
{
	NIOFputc(NewtRefToCharacter(r), f);
}


/*------------------------------------------------------------------------*/
/** �o�̓t�@�C���ɃV���{���I�u�W�F�N�g���v�����g����
 *
 * @param f			[in] �o�̓t�@�C��
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�Ȃ�
 *
 * @note			newtStream_t ���g�p
 */

void NIOPrintSymbol(newtStream_t * f, newtRefArg r)
{
    newtSymDataRef	sym;

	sym = NewtRefToSymbol(r);
	NIOFputs(sym->name, f);
}


/*------------------------------------------------------------------------*/
/** �o�̓t�@�C���ɕ�����I�u�W�F�N�g���v�����g����
 *
 * @param f			[in] �o�̓t�@�C��
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�Ȃ�
 *
 * @note			newtStream_t ���g�p
 */

void NIOPrintString(newtStream_t * f, newtRefArg r)
{
	NIOFputs(NewtRefToString(r), f);
}


/*------------------------------------------------------------------------*/
/** �o�̓t�@�C���ɔz��I�u�W�F�N�g���v�����g����
 *
 * @param f			[in] �o�̓t�@�C��
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�Ȃ�
 *
 * @note			newtStream_t ���g�p
 */

void NIOPrintArray(newtStream_t * f, newtRefArg r)
{
	newtRef *	slots;
	uint32_t	len;
	uint32_t	i;

	slots = NewtRefToSlots(r);
	len = NewtLength(r);

	for (i = 0; i < len; i++)
	{
		NIOPrint(f, slots[i]);
	}
}


/*------------------------------------------------------------------------*/
/** �o�̓t�@�C���ɃI�u�W�F�N�g���v�����g
 *
 * @param f			[in] �o�̓t�@�C��
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�Ȃ�
 *
 * @note			newtStream_t ���g�p
 */

void NIOPrint(newtStream_t * f, newtRefArg r)
{
    switch (NewtGetRefType(r, true))
    {
        case kNewtNil:
        case kNewtTrue:
        case kNewtUnbind:
            break;

        case kNewtSpecial:
            NIOPrintSpecial(f, r);
            break;

        case kNewtInt30:
        case kNewtInt32:
            NIOPrintInteger(f, r);
            break;

        case kNewtReal:
            NIOPrintReal(f, r);
            break;

        case kNewtCharacter:
			NIOPrintCharacter(f, r);
            break;

        case kNewtMagicPointer:
			{
				newtRefVar	r2;

				r2 = NcResolveMagicPointer(r);

				if (! NewtRefIsMagicPointer(r2))
				{
					NIOPrint(f, r2);
				}
			}
            break;

        case kNewtArray:
			NIOPrintArray(f, r);
			break;

        case kNewtBinary:
        case kNewtFrame:
            break;

        case kNewtSymbol:
			NIOPrintSymbol(f, r);
            break;

        case kNewtString:
			NIOPrintString(f, r);
            break;

        default:
            break;
    }
}


/*------------------------------------------------------------------------*/
/** �o�̓t�@�C���ɃI�u�W�F�N�g���v�����g
 *
 * @param f			[in] �o�̓t�@�C��
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�Ȃ�
 */

void NewtPrint(FILE * f, newtRefArg r)
{
	newtStream_t	stream;

	NIOSetFile(&stream, f);
	NIOPrint(&stream, r);
}


/*------------------------------------------------------------------------*/
/** �W���o�͂Ɋ֐�����\��
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�Ȃ�
 *
 * @note			newtStream_t ���g�p
 */

void NIOInfo(newtStream_t * f, newtRefArg r)
{
    newtRefVar	fn = kNewtRefUnbind;

    if (NewtRefIsSymbol(r))
        fn = NcGetGlobalFn(r);
    else
        fn = (newtRef)r;

    if (NewtRefIsFunction(fn))
    {
        newtRefVar	docString;

        docString = NcGetSlot(fn, NSSYM0(docString));

        if (NewtRefIsNotNIL(docString))
        {
            if (NewtRefIsString(docString))
			{
//                NIOFprintf(f, "%s\n", NewtRefToString(docString));
                NIOFputs(NewtRefToString(docString), f);
                NIOFputs("\n", f);
            }
			else
            {
			    NIOPrintObj(f, docString);
			}
        }
    }
}


/*------------------------------------------------------------------------*/
/** �W���o�͂Ɋ֐�����\��
 *
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			�Ȃ�
 */

void NewtInfo(newtRefArg r)
{
	newtStream_t	stream;

	NIOSetFile(&stream, stdout);
	NIOInfo(&stream, r);
}


/*------------------------------------------------------------------------*/
/** �W���o�͂ɑS�O���[�o���֐��̊֐�����\��
 *
 * @return			�Ȃ�
 */

void NewtInfoGlobalFns(void)
{
	newtStream_t	stream;
    newtRef *	slots;
    newtRefVar	fns;
    uint32_t	len;
    uint32_t	i;

	NIOSetFile(&stream, stdout);

    fns = NcGetGlobalFns();
    len = NewtLength(fns);

    slots = NewtRefToSlots(fns);

    for (i = 0; i < len; i++)
    {
        NIOInfo(&stream, slots[i]);
    }
}

