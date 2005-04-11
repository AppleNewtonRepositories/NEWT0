/*------------------------------------------------------------------------*/
/**
 * @file	main.c
 * @brief   ���C���֐��iCUI �R�}���h�j
 *
 * @author  M.Nukui
 * @date	2003-11-07
 *
 * Copyright (C) 2003-2004 M.Nukui All rights reserved.
 */


/* �w�b�_�t�@�C�� */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "NewtCore.h"
#include "NewtBC.h"
#include "NewtVM.h"
#include "NewtParser.h"
#include "lookup_words.h"
#include "yacc.h"
#include "version.h"


/* �萔 */

/// �I�v�V����
enum {
    optNone			= 0,
    optNos2,
    optCopyright,
    optVersion,
    optStaff,
};


/* ���[�J���ϐ� */

/// �I�v�V�����L�[���[�h�̃��b�N�A�b�v�e�[�u��
static keyword_t	reserved_words[] = {
        // �A���t�@�x�b�g���Ƀ\�[�g���Ă�������
        {"copyright",	optCopyright},
        {"newton",		optNos2},
        {"nos2",		optNos2},
        {"staff",		optStaff},
        {"version",		optVersion},
    };


/// ��ƃf�B���N�g��
static const char *		newt_currdir;


/* �֐��v���g�^�C�v */
#ifdef __cplusplus
extern "C" {
#endif


int main (int argc, const char * argv[]);


#ifdef __cplusplus
}
#endif


static void		newt_result_message(newtRefArg r, newtErr err);

static newtErr  newt_info(int argc, const char * argv[], int n);
static newtErr  newt_interpret_str(int argc, const char * argv[], int n);
static newtErr  newt_interpret_file(int argc, const char * argv[], int n);

static void		newt_chdir(void);

static void		newt_show_copyright(void);
static void		newt_show_version(void);
static void		newt_show_staff(void);
static void		newt_show_usage(void);

static void		newt_invalid_option(char c);
static void		newt_option_switchs(const char * s);
static void		newt_option(const char * s);
static newtErr  newt_option_with_arg(char c, int argc, const char * argv[], int n);


#pragma mark -
/*------------------------------------------------------------------------*/
/** ���ʂ�\��
 *
 * @param r			[in] �I�u�W�F�N�g
 * @param err		[in] �G���[�R�[�h
 *
 * @return			�Ȃ�
 */

void newt_result_message(newtRefArg r, newtErr err)
{
    if (err != kNErrNone)
        NewtErrMessage(err);
    else if (NEWT_DEBUG)
        NsPrint(kNewtRefNIL, r);
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �R�}���h���C�������Ŏw�肳�ꂽ�֐��̏���\��
 *
 * @param argc		[in] �R�}���h���C�������̐�
 * @param argv		[in] �R�}���h���C�������̔z��
 * @param n			[in] �R�}���h���C�������̈ʒu
 *
 * @return			�G���[�R�[�h
 */

newtErr newt_info(int argc, const char * argv[], int n)
{
    newtErr	err = kNErrNone;
    int		i;

    NewtInit(argc, argv, argc);
	newt_chdir();

    if (n < argc)
    {
        for (i = n; i < argc; i++)
        {
            err = NVMInfo(argv[i]);
            NewtErrMessage(err);
        }
    }
    else
    {   //�������Ȃ��ꍇ�͊֐��ꗗ��\��
        err = NVMInfo(NULL);
        NewtErrMessage(err);
    }

    NewtCleanup();

    return err;
}


/*------------------------------------------------------------------------*/
/** �R�}���h���C�������Ŏw�肳�ꂽ��������C���^�v���^���s����
 *
 * @param argc		[in] �R�}���h���C�������̐�
 * @param argv		[in] �R�}���h���C�������̔z��
 * @param n			[in] �R�}���h���C�������̈ʒu
 *
 * @return			�G���[�R�[�h
 */

newtErr newt_interpret_str(int argc, const char * argv[], int n)
{
    newtRefVar	result;
    newtErr	err;

    NewtInit(argc, argv, n + 1);
	newt_chdir();
    result = NVMInterpretStr(argv[n], &err);
    newt_result_message(result, err);
    NewtCleanup();

    return err;
}


/*------------------------------------------------------------------------*/
/** �t�@�C�����C���^�v���^���s����
 *
 * @param argc		[in] �R�}���h���C�������̐�
 * @param argv		[in] �R�}���h���C�������̔z��
 * @param n			[in] �R�}���h���C�������̈ʒu
 *
 * @return			�G���[�R�[�h
 */

newtErr newt_interpret_file(int argc, const char * argv[], int n)
{
    const char * path = NULL;
    newtRefVar	result;
    newtErr	err;

    if (n < argc)
    {
        path = argv[n];
    }

    NewtInit(argc, argv, n + 1);
	newt_chdir();
    result = NVMInterpretFile(path, &err);
    newt_result_message(result, err);
    NewtCleanup();

    return err;
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** ��ƃf�B���N�g����ύX */
void newt_chdir(void)
{
	if (newt_currdir != NULL)
		chdir(newt_currdir);
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �R�s�[���C�g��\�� */
void newt_show_copyright(void)
{
    fprintf(stderr, "%s - %s\n", NEWT_NAME, NEWT_COPYRIGHT);
}


/** �o�[�W��������\�� */
void newt_show_version(void)
{
    fprintf(stderr, "%s%s %s(%s)\n", NEWT_NAME, NEWT_PROTO,
                NEWT_VERSION, NEWT_BUILD);
}


/** �X�^�b�t����\�� */
void newt_show_staff(void)
{
    fprintf(stderr, "%s%s %s\n\n%s\n", NEWT_NAME, NEWT_PROTO, NEWT_VERSION,
                NEWT_STAFF);
}


/** �g�p�@��\�� */
void newt_show_usage(void)
{
    fprintf(stderr, "Usage: %s %s\n%s", NEWT_NAME, NEWT_PARAMS, NEWT_USAGE);
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �I�v�V�����G���[��\��
 *
 * @param s			[in] �I�v�V��������
 *
 * @return			�Ȃ�
 */

void newt_invalid_option(char c)
{
    fprintf(stderr, "invalid option -%c (-h will show valid options)\n", c);
}


/*------------------------------------------------------------------------*/
/** �I�v�V�����X�C�b�`�̉��
 *
 * @param s			[in] �I�v�V�����X�C�b�`
 *
 * @return			�Ȃ�
 */

void newt_option_switchs(const char * s)
{
    while (*s)
    {
        switch (*s)
        {
            case 'd':
                NEWT_DEBUG = true;
                break;

            case 't':
                NEWT_TRACE = true;
                break;

            case 'l':
                NEWT_DUMPLEX = true;
                break;

            case 's':
                NEWT_DUMPSYNTAX = true;
                break;

            case 'b':
                NEWT_DUMPBC = true;
                break;

            case 'v':
                newt_show_version();
                exit(0);
                break;

            case 'h':
                newt_show_usage();
                exit(0);
                break;

            default:
                newt_invalid_option(*s);
                exit(0);
                break;
        }

        s++;
    }
}


/*------------------------------------------------------------------------*/
/** �I�v�V����������̉��
 *
 * @param s			[in] �I�v�V����������
 *
 * @return			�Ȃ�
 */

void newt_option(const char * s)
{
    int	wlen;

    wlen = sizeof(reserved_words) / sizeof(keyword_t);

    switch (lookup_words(reserved_words, wlen, s))
    {
		// NOS2 �R���p�`�u��
		case optNos2:
			NEWT_MODE_NOS2 = true;
			break;

        // �R�s�[���C�g
        case optCopyright:
            newt_show_copyright();
            exit(0);
            break;

        // �o�[�W����
        case optVersion:
            newt_show_version();
            exit(0);
            break;

        // �X�^�b�t���[��
        case optStaff:
            newt_show_staff();
            exit(0);
            break;
    }
}


/*------------------------------------------------------------------------*/
/** �I�v�V�����̈�������͂���
 *
 * @param c			[in] �I�v�V��������
 * @param argc		[in] �R�}���h���C�������̐�
 * @param argv		[in] �R�}���h���C�������̔z��
 * @param n			[in] �R�}���h���C�������̈ʒu
 *
 * @return			�G���[�R�[�h
 */

newtErr newt_option_with_arg(char c, int argc, const char * argv[], int n)
{
    newtErr	err = kNErrNone;

    switch (c)
    {
        case 'i':
            err = newt_info(argc, argv, n);
            break;

        case 'e':
            if (n < argc)
                err = newt_interpret_str(argc, argv, n);
            break;
    }

    return err;
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** main �֐��iCUI �R�}���h)
 *
 * @brief NewtonScript �C���^�v���^
 *
 * @param argc		[in] �R�}���h���C�������̐�
 * @param argv		[in] �R�}���h���C�������̔z��
 *
 * @return			�G���[�R�[�h
 */

int main (int argc, const char * argv[])
{
    const char *	s;
    newtErr	err = kNErrNone;
    int		i;

    for (i = 1; i < argc; i++)
    {
        s = argv[i];

        if (strlen(s) == 0) break;
        if (*s != '-') break;

        s++;

        if (*s == '-')
        {
            newt_option(s + 1);
            i++;
            break;
        }

        switch (*s)
        {
            case 'C':
                i++;
				newt_currdir = argv[i];
                break;

            case 'i':
            case 'e':
                i++;
                err = newt_option_with_arg(*s, argc, argv, i);
                exit(err);
                break;

            default:
                newt_option_switchs(s);
                break;
        }
    }

    err = newt_interpret_file(argc, argv, i);

    return err;
}


/*------------------------------------------------------------------------*/
/** �p�[�T�G���[
 *
 * @param s			[in] �G���[���b�Z�[�W������
 *
 * @return			�Ȃ�
 */

void yyerror(char * s)
{
	if (s[0] && s[1] == ':')
		NPSErrorStr(*s, s + 2);
	else
		NPSErrorStr('E', s);
}
