/*------------------------------------------------------------------------*/
/**
 * @file	NewtConf.h
 * @brief   �R���t�B�O���
 *
 * @author  M.Nukui
 * @date	2003-11-07
 *
 * Copyright (C) 2003-2004 M.Nukui All rights reserved.
 */


#ifndef	NEWTCONF_H
#define	NEWTCONF_H


/* �}�N�� */

#define __NAMED_MAGIC_POINTER__					///< ���O�t�}�W�b�N�|�C���^���g�p

// VM
#define NEWT_NUM_STACK			512				///< ��x�Ɋm�ۂ���X�^�b�N��
#define NEWT_NUM_CALLSTACK		512				///< ��x�Ɋm�ۂ���ďo���X�^�b�N��
#define NEWT_NUM_EXCPSTACK		512				///< ��x�Ɋm�ۂ����O�X�^�b�N��

// Parser
#define NEWT_NUM_STREESTACK		1024			///< ��x�Ɋm�ۂ���\���؃X�^�b�N��

// Bytecode
#define NEWT_NUM_BYTECODE		512				///< ��x�Ɋm�ۂ��� Bytecode �̃�������
#define NEWT_NUM_BREAKSTACK		20				///< ��x�Ɋm�ۂ��� break ���̍�Ɨp�X�^�b�N��
#define NEWT_NUM_ONEXCPSTACK	20				///< ��x�Ɋm�ۂ��� OnException ���̍�Ɨp�X�^�b�N��

// Pool
#define NEWT_POOL_EXPANDSPACE	(1024 * 10)		///<�@�@�������v�[���̊g���T�C�Y

// IO
#define	NEWT_FGETS_BUFFSIZE		2048			///< fgets �̃o�b�t�@�T�C�Y
#define NEWT_SNPRINTF_BUFFSIZE	255				///< snprintf, vsnprintf �̃o�b�t�@�T�C�Y

// lex
#define	NEWT_LEX_LINEBUFFSIZE	500				///< �����͂̍s�o�b�t�@�T�C�Y


// for old style compatible
//#define __USE_OBSOLETE_STYLE__


#endif /* NEWTCONF_H */
