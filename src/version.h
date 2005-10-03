/*------------------------------------------------------------------------*/
/**
 * @file	version.h
 * @brief   �o�[�W�������
 *
 * @author  M.Nukui
 * @date	2003-11-07
 *
 * Copyright (C) 2003-2005 M.Nukui All rights reserved.
 */


#ifndef	NEWTVERSION_H
#define	NEWTVERSION_H

/* �}�N����` */
#define NEWT_NAME		"newt"									///< �R�}���h��
#define NEWT_PROTO		"/0"									///< �v���g
#define NEWT_VERSION	"0.1.1"									///< �o�[�W����
#define NEWT_COPYRIGHT	"Copyright (C) 2003-2005 Makoto Nukui"  ///< �R�s�[���C�g
#define NEWT_BUILD		"2005-10-03-2"							///< �r���h�ԍ�
#define NEWT_PARAMS		"[switches] [--] [programfile]"			///< ����

/// �g�p���@
#define NEWT_USAGE		"  -t              enable trace mode\n"			\
						"  -l              dump lex info\n"				\
						"  -s              dump syntax tree\n"			\
						"  -b              dump byte code\n"			\
						"  -C directory    change working directory\n"	\
						"  -e 'command'    one line of script\n"		\
						"  -i [symbols]    print function info\n"		\
						"  -v              print version number\n"		\
						"  -h              print this help message\n"	\
						"  --newton,--nos2 Newton OS 2.0 compatible	\n"	\
						"  --copyright     print copyright\n"			\
						"  --version       print version number\n"

/// �X�^�b�t���[��
#define NEWT_STAFF		"Program\n"										\
                        "  Makoto Nukui\n"								\
                        "\n"											\
                        "Patch contribute\n"							\
                        "  Paul Guyot\n"								\
                        "\n"											\
                        "Special Thanks\n"								\
                        "  sumim\n"										\
                        "  Rihatsu\n"									\
                        "  Paul Guyot\n"								\
						""

#endif /* NEWTVERSION_H */
