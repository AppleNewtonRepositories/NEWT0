/**
 * @file	NewtIconv.c
 * @brief   �����R�[�h�����ilibiconv�g�p�j
 *
 * @author  M.Nukui
 * @date	2005-07-17
 *
 * Copyright (C) 2005 M.Nukui All rights reserved.
 */


/* �w�b�_�t�@�C�� */
#include "NewtIconv.h"


#ifdef HAVE_LIBICONV
/*------------------------------------------------------------------------*/
/** NSOF�o�b�t�@��Ǎ���Ŕz��I�u�W�F�N�g�ɕϊ�����
 *
 * @param cd		[in] iconv�ϊ��f�B�X�N���v�^�[
 * @param src		[in] �ϊ����镶����
 * @param srclen	[in] �ϊ����镶����̒���
 * @param dstlenp	[out]�ϊ����ꂽ������̒���
 *
 * @return			�ϊ����ꂽ������
 *
 * @note			�ϊ����ꂽ������͌ďo������ free ����K�v����
 */

char * NewtIconv(iconv_t cd, char * src, size_t srclen, size_t* dstlenp)
{
	char *	dst = NULL;
	size_t	dstlen = 0;

	if (cd != (iconv_t)-1)
	{
		size_t	bufflen;

		bufflen = srclen * 3;
		dst = malloc(bufflen);

		if (dst)
		{
			char *	inbuf_p = src;
			char *	outbuf_p = dst;
			size_t	inbytesleft = srclen;
			size_t	outbytesleft = bufflen;
			size_t	status;

			iconv(cd, NULL, NULL, NULL, NULL);
			status = iconv(cd, &inbuf_p, &inbytesleft, &outbuf_p, &outbytesleft);

			if (status == (size_t)-1)
			{	// �ϊ��Ɏ��s�����̂Ńo�b�t�@���������
				free(dst);
				dst = NULL;
			}
			else
			{	// ����Ȃ������̃o�b�t�@��؂�l�߂�
				dstlen = bufflen - outbytesleft;
				dst = realloc(dst, dstlen);
			}
		}
	}

	if (dstlenp) *dstlenp = dstlen;

	return dst;
}

#endif /* HAVE_LIBICONV */
