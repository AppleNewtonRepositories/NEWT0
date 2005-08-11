/*------------------------------------------------------------------------*/
/**
 * @file	NewtIO.c
 * @brief   ���o�͏���
 *
 * @author M.Nukui
 * @date 2004-06-05
 *
 * Copyright (C) 2003-2004 M.Nukui All rights reserved.
 */


/* �w�b�_�t�@�C�� */
#include <string.h>
#include <stdio.h>

#include "NewtCore.h"
#include "NewtIO.h"


/*------------------------------------------------------------------------*/
/** ���o�̓X�g���[���\���̂Ƀt�@�C���̏����Z�b�g����
 *
 * @param stream	[out]���o�̓X�g���[��
 * @param f			[in] �t�@�C��
 *
 * @return			�Ȃ�
 */

void NIOSetFile(newtStream_t * stream, FILE * f)
{
	stream->file = f;

	if (f == stdout)
		stream->obj = NcGetGlobalVar(NSSYM0(_STDOUT_));
	else if (f == stderr)
		stream->obj = NcGetGlobalVar(NSSYM0(_STDERR_));
	else
		stream->obj = kNewtRefUnbind;
}


/*------------------------------------------------------------------------*/
/** printf �t�H�[�}�b�g�ŏo�͂���i�s�蒷�j
 *
 * @param stream	[in] �o�̓X�g���[��
 * @param format	[in] �t�H�[�}�b�g
 * @param ...		[in] printf ����
 *
 * @return			printf �̖߂�l
 *
 * @note			newtStream_t ���g�p
 */

int NIOFprintf(newtStream_t * stream, const char * format, ...)
{
	va_list	args;
	int		result;

	va_start(args, format);
	result = NIOVfprintf(stream, format, args);
	va_end(args);

	return result;
}


/*------------------------------------------------------------------------*/
/** vprintf �t�H�[�}�b�g�ŏo�͂���
 *
 * @param stream	[in] �o�̓X�g���[��
 * @param format	[in] �t�H�[�}�b�g
 * @param ap		[in] vprintf ����
 *
 * @return			vprintf �̖߂�l
 *
 * @note			newtStream_t ���g�p
 *					������ɒǉ�����ꍇ�̐��� NEWT_SNPRINTF_BUFFSIZE (NewtConf.h)
 */

int NIOVfprintf(newtStream_t * stream, const char * format, va_list ap)
{
	int		result = 0;

	if (NewtRefIsString(stream->obj))
	{
		char	wk[NEWT_SNPRINTF_BUFFSIZE];

		result = vsnprintf(wk, sizeof(wk), format, ap);

		if (0 < result)
		{
			if (sizeof(wk) < result)
				wk[sizeof(wk) - 1] = '\0';

			NewtStrCat(stream->obj, wk);
		}
	}
	else
	{
		result = vfprintf(stream->file, format, ap);
	}

	return result;
}


/*------------------------------------------------------------------------*/
/** �������o�͂���
 *
 * @param c			[in] ����
 * @param stream	[in] �o�̓X�g���[��
 *
 * @return			fputc �̖߂�l
 *
 * @note			newtStream_t ���g�p
 */

int NIOFputc(int c, newtStream_t * stream)
{
	int		result = 0;

	if (NewtRefIsString(stream->obj))
	{
		char	wk[4];

		sprintf(wk, "%c", c);
		NewtStrCat(stream->obj, wk);
	}
	else
	{
		result = fputc(c, stream->file);
	}

	return result;
}


/*------------------------------------------------------------------------*/
/** ��������o�͂���
 *
 * @param str		[in] ������
 * @param stream	[in] �o�̓X�g���[��
 *
 * @return			fputs �̖߂�l
 *
 * @note			newtStream_t ���g�p
 */

int NIOFputs(const char *str, newtStream_t * stream)
{
	int		result = 0;

	if (NewtRefIsString(stream->obj))
	{
		NewtStrCat(stream->obj, (char *)str);
	}
	else
	{
		result = fputs(str, stream->file);
	}

	return result;
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** printf �t�H�[�}�b�g�ŏo�͂���i�s�蒷�j
 *
 * @param f			[in] �o�̓X�g���[��
 * @param format	[in] �t�H�[�}�b�g
 * @param ...		[in] printf ����
 *
 * @return			printf �̖߂�l
 */

int NewtFprintf(FILE * f, const char * format, ...)
{
	newtStream_t	stream;
	va_list	args;
	int		result;

	NIOSetFile(&stream, f);

	va_start(args, format);
	result = NIOVfprintf(&stream, format, args);
	va_end(args);

	return result;
}


/*------------------------------------------------------------------------*/
/** �����̏o��
 *
 * @param c			[in] ����
 * @param f			[in] �o�̓X�g���[��
 *
 * @return			fputc �̖߂�l
 */

int NewtFputc(int c, FILE * f)
{
	newtStream_t	stream;

	NIOSetFile(&stream, f);
	return NIOFputc(c, &stream);
}


/*------------------------------------------------------------------------*/
/** �����̏o��
 *
 * @param str		[in] ������
 * @param f			[in] �o�̓X�g���[��
 *
 * @return			fputs �̖߂�l
 */

int NewtFputs(const char *str, FILE * f)
{
	newtStream_t	stream;

	NIOSetFile(&stream, f);
	return NIOFputs(str, &stream);
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** printf �t�H�[�}�b�g�ŏo�͂���i�s�蒷�j
 *
 * @param title		[in] �^�C�g��
 * @param format	[in] �t�H�[�}�b�g
 * @param ...		[in] printf ����
 *
 * @return			printf �̖߂�l
 */

int NewtDebugMsg(const char * title, const char * format, ...)
{
	newtStream_t	stream;
	va_list	args;
	int		result;

	NIOSetFile(&stream, stderr);

	if (title != NULL)
	{
		NIOFputs("[", &stream);
		NIOFputs(title, &stream);
		NIOFputs("] ", &stream);
	}

	va_start(args, format);
	result = NIOVfprintf(&stream, format, args);
	va_end(args);

	return result;
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** ���̓X�g���[�����當�������o��
 *
 * @param stream	[in] ���̓X�g���[��
 *
 * @retval			������I�u�W�F�N�g	���̓f�[�^�����݂���ꍇ
 * @retval			NIL				���̓f�[�^�����݂��Ȃ��ꍇ
 *
 * @note			���� NEWT_FGETS_BUFFSIZE (NewtConf.h)
 */

newtRef NewtFgets(FILE * stream)
{
	newtRefVar  result = kNewtRefNIL;
	char	buff[NEWT_FGETS_BUFFSIZE];
	char *  str;
	char	c;
	int		maxsize;
	int		oldlen;
	int		len;

	maxsize = sizeof(buff) - 1;

	while (str = fgets(buff, sizeof(buff), stream))
	{
		len = strlen(str);

		if (result == kNewtRefNIL)
		{	// ������I�u�W�F�N�g�쐬
			result = NewtMakeString2(str, len, false);

			if (NewtRefIsNIL(result))
			{	// ���������m�ۂł��Ȃ�����
				return NewtThrow0(kNErrOutOfObjectMemory);
			}
		}
		else
		{	// �ǉ�
			oldlen = NewtStringLength(result);
			result = NewtStrCat2(result, str, len);

			if (NewtStringLength(result) < oldlen + len)
			{	// ���������m�ۂł��Ȃ�����
				return NewtThrow0(kNErrOutOfObjectMemory);
			}
		}

		if (len < maxsize)
			break;

		// �Ō�̕������`�F�b�N
		c = buff[maxsize - 1];

		if (c == '\n')
			break;

		if (c == '\r')
		{
			// �P������ǂ�
			c = fgetc(stream);

			if (c != '\n')
			{	// CRLF �łȂ��iCR �̂݁j�ꍇ
				// ��ǂ݂����������X�g���[���ɖ߂�
				ungetc(c, stream);
				break;
			}
		}
	}

    return result;
}


/*------------------------------------------------------------------------*/
/** �W�����͂��當�������o��
 *
 * @param rcvr		[in] ���V�[�o
 *
 * @retval			������I�u�W�F�N�g	���̓f�[�^�����݂���ꍇ
 * @retval			NIL				���̓f�[�^�����݂��Ȃ��ꍇ
 */

newtRef NsGets(newtRefArg rcvr)
{
	return NewtFgets(stdin);
}


/*------------------------------------------------------------------------*/
/** ���̓X�g���[�����當������o��
 *
 * @param stream	[in] ���̓X�g���[��
 *
 * @retval			�����I�u�W�F�N�g	���̓f�[�^�����݂���ꍇ
 * @retval			NIL				EOF
 */

newtRef NewtFgetc(FILE * stream)
{
	int		c;

	c = fgetc(stream);

	if (c == EOF)
		return kNewtRefNIL;
	else
		return NewtMakeCharacter(c);
}


/*------------------------------------------------------------------------*/
/** �W�����͂��當������o��
 *
 * @param rcvr		[in] ���V�[�o
 *
 * @retval			�����I�u�W�F�N�g	���̓f�[�^�����݂���ꍇ
 * @retval			NIL				���̓f�[�^�����݂��Ȃ��ꍇ
 */

newtRef NsGetc(newtRefArg rcvr)
{
	return NewtFgetc(stdin);
}
