/**
 * @file	NewtStr.c
 * @brief   �����񏈗�
 *
 * @author  M.Nukui
 * @date	2004-01-25
 *
 * Copyright (C) 2003-2004 M.Nukui All rights reserved.
 */


/* �w�b�_�t�@�C�� */
#include <string.h>

#include "NewtCore.h"
#include "NewtStr.h"


/* �֐��v���g�^�C�v */
static newtRef  NewtParamStr(char * baseStr, size_t baseStrLen, newtRefArg paramStrArray, bool ifthen);



#pragma mark -
/*------------------------------------------------------------------------*/
/** �x�[�X������̃p�����[�^��u�������ĐV������������쐬����
 *
 * @param baseStr		[in] �x�[�X������iC������j
 * @param baseStrLen	[in] �x�[�X������̒���
 * @param paramStrArray [in] �p�����[�^�z��
 * @param ifthen		[in] ��������
 *
 * @return ������I�u�W�F�N�g
 */

newtRef NewtParamStr(char * baseStr, size_t baseStrLen, newtRefArg paramStrArray, bool ifthen)
{
	newtRefVar	dstStr;
	newtRefVar  r;
	size_t	fpos = 0;
	size_t	fst = 0;
	size_t	len;
	size_t	n;
	size_t	truePos;
	size_t	trueLen;
	size_t	falsePos;
	size_t	falseLen;
	char *	found;
	char	c;

	dstStr = NewtMakeString("", false);

	do
	{
		found = memchr(&baseStr[fst], '^', baseStrLen - fst);

		if (found == NULL)
			break;

		c = found[1];

		len = found - (baseStr + fpos);
		NewtStrCat2(dstStr, &baseStr[fpos], len);
		fpos += len;
		fst = fpos + 1;

		if ('0' <= c && c <= '9')
		{
			fpos += 2;
			fst = fpos;

			r = NewtGetArraySlot(paramStrArray, c - '0');

			if (NewtRefIsNotNIL(r))
			{
				NcStrCat(dstStr, r);
			}
		}
		else if (ifthen && c == '?')
		{
			c = found[2];

			if ('0' <= c && c <= '9')
			{
				n = c - '0';

				truePos = fpos + 3;
				found = memchr(&baseStr[truePos], '|', baseStrLen - truePos);

				if (found != NULL)
				{
					falsePos = found + 1 - baseStr;
					trueLen = falsePos - truePos - 1;

					found = memchr(&baseStr[falsePos], '|', baseStrLen - falsePos);

					if (found != NULL)
					{
						fpos = found + 1 - baseStr;
					}
					else
					{
						fpos = baseStrLen;
					}

					falseLen = fpos - falsePos - 1;
				}
				else
				{
					trueLen = baseStrLen - truePos;

					falsePos = baseStrLen;
					falseLen = 0;

					fpos = baseStrLen;
				}

				fst = fpos;

				r = NewtGetArraySlot(paramStrArray, n);

				if (NewtRefIsNotNIL(r))
				{
					r = NewtParamStr(&baseStr[truePos], trueLen, paramStrArray, false);
					NcStrCat(dstStr, r);
				}
				else
				{
					if (falsePos == baseStrLen)
						break;

					r = NewtParamStr(&baseStr[falsePos], falseLen, paramStrArray, false);
					NcStrCat(dstStr, r);
				}
			}
		}
	} while(true);

	len = baseStrLen - fpos;

	if (0 < len)
	{
		NewtStrCat2(dstStr, &baseStr[fpos], len);
	}

	return dstStr;
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �����𕶎��ɕϊ�����
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] ����
 *
 * @return			����
 *
 * @note			�O���[�o���֐��p
 */

newtRef NsChr(newtRefArg rcvr, newtRefArg r)
{
    if (! NewtRefIsInteger(r))
        return NewtThrow(kNErrNotAnInteger, r);

    return NewtMakeCharacter(NewtRefToInteger(r));
}


/*------------------------------------------------------------------------*/
/** �����𐮐��ɕϊ�����
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] ����
 *
 * @return			����
 *
 * @note			�O���[�o���֐��p
 */

newtRef NsOrd(newtRefArg rcvr, newtRefArg r)
{
    if (! NewtRefIsCharacter(r))
        return NewtThrow(kNErrNotAnInteger, r);

    return NewtMakeInteger(NewtRefToCharacter(r));
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** ������̒������擾
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] ������I�u�W�F�N�g
 *
 * @return			������̒���
 *
 * @note			�O���[�o���֐��p
 */

newtRef NsStrLen(newtRefArg rcvr, newtRefArg r)
{
    if (! NewtRefIsString(r))
        return NewtThrow(kNErrNotAString, r);

    return NewtMakeInteger(NewtStringLength(r));
}


/*------------------------------------------------------------------------*/
/** �I�u�W�F�N�g��\���\�ȕ�����ɕϊ�����
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �I�u�W�F�N�g
 *
 * @return			������I�u�W�F�N�g
 *
 * @note			�O���[�o���֐��p
 */

newtRef NsSPrintObject(newtRefArg rcvr, newtRefArg r)
{
    newtRefVar	str;

    str = NSSTR("");

	NcStrCat(str, r);

    return str;
}


/*------------------------------------------------------------------------*/
/** ��������w��̋�؂蕶���ŕ�������
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] ������I�u�W�F�N�g
 * @param sep		[in] ��؂蕶��
 *
 * @return			�z��I�u�W�F�N�g
 *
 * @note			�O���[�o���֐��p
 */

newtRef NsSplit(newtRefArg rcvr, newtRefArg r, newtRefArg sep)
{
	newtRefVar  result;

    if (! NewtRefIsString(r))
        return NewtThrow(kNErrNotAString, r);

	switch (NewtGetRefType(sep, true))
	{
		case kNewtCharacter:
			{
				newtRefVar  v;
				char *		next;
				char *		s;
				int			c;

				s = NewtRefToString(r);
				c = NewtRefToCharacter(sep);

				result = NewtMakeArray(kNewtRefNIL, 0);

				while (*s)
				{
					next = strchr(s, c);
					if (next == NULL) break;

					v = NewtMakeString2(s, next - s, false);
					NcAddArraySlot(result, v);
					s = next + 1;
				}

				if (s == NewtRefToString(r))
					v = r;
				else
					v = NSSTR(s);

				NcAddArraySlot(result, v);
			}
			break;

		default:
			{
				newtRefVar	initObj[] = {r};

				result = NewtMakeArray2(kNewtRefNIL, sizeof(initObj) / sizeof(newtRefVar), initObj);
			}
			break;
	}

    return result;
}


/*------------------------------------------------------------------------*/
/** �x�[�X������̃p�����[�^��u�������ĐV������������쐬����
 *
 * @param rcvr			[in] ���V�[�o
 * @param baseString	[in] �x�[�X������
 * @param paramStrArray [in] �p�����[�^�z��
 *
 * @return			������I�u�W�F�N�g
 *
 * @note			�O���[�o���֐��p
 */

newtRef NsParamStr(newtRefArg rcvr, newtRefArg baseString, newtRefArg paramStrArray)
{
    if (! NewtRefIsString(baseString))
        return NewtThrow(kNErrNotAString, baseString);

    if (! NewtRefIsArray(paramStrArray))
        return NewtThrow(kNErrNotAnArray, paramStrArray);

	return NewtParamStr(NewtRefToString(baseString), NewtStringLength(baseString), paramStrArray, true);
}

/*------------------------------------------------------------------------*/
/**
 * Extract the substring of a string.
 *
 * @param rcvr	self (ignored).
 * @param r		the string to create a substring of.
 * @param start	the offset of the first character of the substring.
 * @param count	the number of characters to extract or NIL to go til the end.
 * @return a new string
 *
 * @note highly unefficient.
 */

newtRef NsSubStr(newtRefArg rcvr, newtRefArg r, newtRefArg start, newtRefArg count)
{
	char* theString;
	char* theBuffer;
	int theStart, theEnd;
	size_t theLen;
	newtRefVar theResult;
	
	(void) rcvr;
	
	/* check parameters */
    if (! NewtRefIsString(r))
        return NewtThrow(kNErrNotAString, r);
    if (! NewtRefIsInteger(start))
        return NewtThrow(kNErrNotAnInteger, start);
    
    theString = NewtRefToString(r);
    theLen = strlen(theString);
    
    theStart = NewtRefToInteger(start);

    if (!NewtRefIsNIL(count))
    {
    	if (!NewtRefIsInteger(count))
    	{
        	return NewtThrow(kNErrNotAnInteger, count);
        }
        theEnd = theStart + NewtRefToInteger(count);
        if (theEnd > theLen)
        {
        	return NewtThrow(kNErrOutOfRange, count);
        }
	} else {
		theEnd = theLen;
	}

	/* new length */
	theLen = theEnd - theStart;
	
	/* create a buffer to copy the characters to */
	theBuffer = (char*) malloc(theLen + 1);
	(void) memcpy(theBuffer, (const char*) &theString[theStart], theLen);
	theBuffer[theLen] = 0;
	theResult = NewtMakeString(theBuffer, false);
	free(theBuffer);
	
	return theResult;
}

/*------------------------------------------------------------------------*/
/**
 * Determine if two strings are equal, ignoring case.
 *
 * @param rcvr	self (ignored).
 * @param a		the first string to consider.
 * @param b		the second string to consider.
 * @return true if the two strings are equal, nil otherwise.
 */

newtRef NsStrEqual(newtRefArg rcvr, newtRefArg a, newtRefArg b)
{
	char* aString;
	char* bString;
	newtRefVar theResult = kNewtRefNIL;
	
	(void) rcvr;

	/* check parameters */
    if (! NewtRefIsString(a))
    {
    	theResult = NewtThrow(kNErrNotAString, a);
    } else if (! NewtRefIsString(b)) {
        theResult = NewtThrow(kNErrNotAString, b);
	} else if (a == b) {
		theResult = kNewtRefTRUE;
	} else {    
		aString = NewtRefToString(a);
		bString = NewtRefToString(b);
	
		if (strcasecmp(aString, bString) == 0)
		{
			theResult = kNewtRefTRUE;
		}
	}
	
	return theResult;
}

/*------------------------------------------------------------------------*/
/**
 * Compare two strings, returning an integer representing the result of the
 * comparison. The comparison is case sensitive.
 *
 * @param rcvr	self (ignored).
 * @param a		the first string to consider.
 * @param b		the second string to consider.
 * @return an integer representing the result of the comparison (a < b -> < 0,
 *         a = b -> 0, a > b -> > 0)
 */

newtRef NsStrExactCompare(newtRefArg rcvr, newtRefArg a, newtRefArg b)
{
	char* aString;
	char* bString;
	newtRefVar theResult;
	
	(void) rcvr;

	/* check parameters */
    if (! NewtRefIsString(a))
    {
    	theResult = NewtThrow(kNErrNotAString, a);
    } else if (! NewtRefIsString(b)) {
        theResult = NewtThrow(kNErrNotAString, b);
	} else if (a == b) {
		theResult = NewtMakeInteger(0);
	} else {    
		aString = NewtRefToString(a);
		bString = NewtRefToString(b);
	
		theResult = NewtMakeInteger(strcmp(aString, bString));
	}
	
	return theResult;
}
