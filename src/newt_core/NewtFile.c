/*------------------------------------------------------------------------*/
/**
 * @file	NewtFile.c
 * @brief   �t�@�C������
 *
 * @author  M.Nukui
 * @date	2004-01-25
 *
 * Copyright (C) 2003-2004 M.Nukui All rights reserved.
 */


/* �w�b�_�t�@�C�� */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef __WIN32__
	#include "win/dlfcn.h"
#else
	#include <dlfcn.h>
	#include <pwd.h>
#endif

#include "NewtCore.h"
#include "NewtVM.h"
#include "NewtBC.h"
#include "NewtIO.h"
#include "NewtFile.h"


/*------------------------------------------------------------------------*/
/** ���I���C�u�������C���X�g�[������
 *
 * @param fname		[in] �t�@�C���̃p�X
 *
 * @return			���I���C�u�����̃f�X�N�v���^
 */

void * NewtDylibInstall(const char * fname)
{
    newt_install_t	install_call;
    void *	lib;

    lib = dlopen(fname, RTLD_LAZY);

    if (lib != NULL)
    {
        install_call = (newt_install_t)dlsym(lib, "newt_install");

        if (install_call == NULL)
        {
            dlclose(lib);
            return NULL;
        }

        (*install_call)();
    }

    return lib;
}


/*------------------------------------------------------------------------*/
/** �t�@�C���̑��݊m�F
 *
 * @param path		[in] �t�@�C���̃p�X
 *
 * @retval			true	�t�@�C�������݂���
 * @retval			false   �t�@�C�������݂��Ȃ�
 */

bool NewtFileExists(char * path)
{
	FILE *  f;

	f = fopen(path, "r");

	if (f != NULL)
	{
		fclose(f);
		return true;
	}

	return false;
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �t�@�C���Z�p���[�^��Ԃ�
 *
 * @return			�t�@�C���Z�p���[�^
 */

char NewtGetFileSeparator(void)
{
	return '/';
}


/*------------------------------------------------------------------------*/
/** �z�[���f�B���N�g���̃p�X���擾
 *
 * @param s			[in] �t�@�C���̃p�X
 * @param subdir	[out]�T�u�f�B���N�g��
 *
 * @return			�z�[���f�B���N�g���̃p�X
 *
 * @note			�擾���ꂽ�z�[���f�B���N�g���̕������ free ����K�v������
 */

#ifdef __WIN32__

char * NewtGetHomeDir(const char * s, char ** subdir)
{	// Windows �̏ꍇ
	return NULL;
}

#else

char * NewtGetHomeDir(const char * s, char ** subdir)
{	// UNIX �̏ꍇ
	struct passwd * pswd = NULL;
	uint32_t	len;
	char *  login = NULL;
	char *  dir = NULL;
	char *  sepp;
	char	sep;

	sep = NewtGetFileSeparator();
 	sepp = strchr(s + 1, sep);

	if (sepp != NULL)
	{
		len = sepp - (s + 1);
		login = malloc(len + 1);
		strncpy(login, s + 1, len);
		pswd = getpwnam(s + 1);
	}
	else
	{
		login = (char *)s + 1;
	}

	if (*login != '\0')
		pswd = getpwnam(login);
	else
		pswd = getpwuid(getuid());

	if (pswd != NULL)
		dir = pswd->pw_dir;

	if (subdir != NULL)
		*subdir = sepp;

	if (s + 1 != login)
		free(login);

	return dir;
}

#endif


/*------------------------------------------------------------------------*/
/** �f�B���N�g�����ƃt�@�C��������p�X���쐬
 *
 * @param s1		[in] �f�B���N�g����
 * @param s2		[in] �t�@�C����
 * @param sep		[in] �t�@�C���Z�p���[�^
 *
 * @return			�쐬���ꂽ�p�X
 *
 * @note			�擾���ꂽ�z�[���f�B���N�g���̕������ free ����K�v������
 */

char * NewtJoinPath(char * s1, char * s2, char sep)
{
	char *		path;
	uint32_t	len;
	uint32_t	len1;
	uint32_t	len2;

	len1 = strlen(s1);
	len2 = strlen(s2);

	len = len1 + len2 + 2;

	path = malloc(len);
	if (path == NULL) return NULL;

	strcpy(path, s1);

	path[len1] = sep;
	strncpy(path + len1 + 1, s2, len2 + 1);

	return path;
}


/*------------------------------------------------------------------------*/
/** ���΃p�X���΃p�X�ɓW�J����
 *
 * @param s			[i/o]���΃p�X����΃p�X
 *
 * @return			��΃p�X
 */

char * NewtRelToAbsPath(char * s)
{
	char *  src;
	char *  dst;
	char	sep;

	sep = NewtGetFileSeparator();

	for (src = dst = s; *src != '\0';)
	{
		if (src[0] == sep && src[1] == '.')
		{
			if (src[2] == sep || src[2] == '\0')
			{
				src += 2;
				continue;
			}
			else if (src[2] == '.' && src[3] == sep)
			{
				src += 3;

				while (s < dst)
				{
					dst--;
					if (*dst == sep) break;
				}

				continue;
			}
		}

		if (src != dst)
			*dst = *src;

		src++;
		dst++;
	}

	if (s < dst && *(dst - 1) == sep)
		*(dst - 1) = '\0';
	else if (src != dst)
		*dst = '\0';

	return s;
}


/*------------------------------------------------------------------------*/
/** ���΃p�X���΃p�X�ɓW�J����
 *
 * @param s			[in] ���΃p�X�iC������j
 *
 * @return			��΃p�X�i������I�u�W�F�N�g�j
 *
 * @note			~, ~user �̓z�[���f�B���N�g���ɓW�J�����
 */

newtRef NewtExpandPath(const char *	s)
{
    newtRefVar	r = kNewtRefUnbind;
	char *  subdir = NULL;
	char *  dir = NULL;
	char *  wd = NULL;
	char	sep;

	sep = NewtGetFileSeparator();

	if (*s == sep)
	{
		dir = (char *)s;
	}
#ifdef __WIN32__
	else if (isalpha(*s) && s[1] == ':')
	{
		dir = (char *)s;
	}
#endif
	else if (*s == '~')
	{
		dir = NewtGetHomeDir(s, &subdir);

		if (subdir != NULL && subdir[1] != '\0')
			subdir++;
		else
			subdir = NULL;
	}
	else
	{
		subdir = (char *)s;
	}

	if (dir == NULL)
		dir = wd = getcwd(NULL, 0); // for Mac OS X 10.2
//		dir = wd = getwd(NULL);

	if (subdir != NULL)
	{
		dir = NewtJoinPath(dir, subdir, sep);
		NewtRelToAbsPath(dir);
	}

	r = NSSTR(dir);

	if (subdir != NULL)
		free(dir);

	if (wd != NULL)
		free(wd);

	return r;
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �p�X����t�@�C��������o��
 *
 * @param s			[in] �p�X�ւ̃|�C���^
 * @param len		[in] �p�X�̕�����
 *
 * @return			�t�@�C����
 */

char * NewtBaseName(char * s, uint32_t len)
{
	uint32_t	base = 0;
	uint32_t	i;
	char		sep;

	sep = NewtGetFileSeparator();

	for (i = 0; i < len; i++)
	{
		if (s[i] == sep)
			base = i + 1;
	}

	if (base < len)
		return (s + base);
	else
		return NULL;
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** ���C�u�����̃��[�h
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] ���[�h���郉�C�u�����̃p�X
 *
 * @return			���I���C�u�����̃f�X�N�v���^
 */

newtRef NsLoadLib(newtRefArg rcvr, newtRefArg r)
{
    char *	fname;
    void *	lib;

    if (! NewtRefIsString(r))
        return NewtThrow(kNErrNotAString, r);

    fname = NewtRefToString(r);
    lib = NewtDylibInstall(fname);

    if (lib != NULL)
	{
        return NewtMakeInteger((int32_t)lib);
	}
	else
	{
		const char *  errmsg;

		errmsg = dlerror();

		if (errmsg != NULL)
		{
			NewtFprintf(stderr,"%s\n", errmsg);
		}

        return NewtThrow(kNErrDylibNotOpen, r);
	}
}


/*------------------------------------------------------------------------*/
/** �\�[�X�t�@�C���̃��[�h
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] ���[�h����\�[�X�t�@�C���̃p�X
 *
 * @return			���s���ʂ̃I�u�W�F�N�g
 */

newtRef NsLoad(newtRefArg rcvr, newtRefArg r)
{
	newtRefVar  result = kNewtRefUnbind;
    newtRefVar	fn;
    char *	fname;

    if (! NewtRefIsString(r))
        return NewtThrow(kNErrNotAString, r);

    fname = NewtRefToString(r);
    fn = NBCCompileFile(fname, true);

    if (NewtRefIsNotNIL(fn))
        result = NVMCall(fn, 0, NULL);

    return result;
}


/*------------------------------------------------------------------------*/
/** ���C�u�����̗v��
 *
 * @param r			[in] ���[�h���郉�C�u�����̃V���{��������
 *
 * @return			���[�h���ꂽ���C�u�����̃V���{��
 *
 * @note			�V���{���ɂ�胉�C�u������v������B
 *					�g���q�͕K�v�Ȃ��B�K�X���C�u�����p�X�ɂ�茟�������B
 *					��x���[�h���ꂽ���C�u�����͓Ǎ��܂�Ȃ��B
 *					���C�u������������Ȃ��Ă���O�͔������Ȃ��B
 */

newtRef NcRequire0(newtRefArg r)
{
	newtRefVar  newtlib;
	newtRefVar  requires;
	newtRefVar  sym;
	newtRefVar  env;

	if (NewtRefIsSymbol(r))
	{
		sym = r;
	}
	else
	{
		if (! NewtRefIsString(r))
			return NewtThrow(kNErrNotASymbol, r);

		sym = NcMakeSymbol(r);
	}

    requires = NcGetGlobalVar(NSSYM0(requires));

	if (! NewtRefIsFrame(requires))
	{
		requires = NcMakeFrame();
		NcDefGlobalVar(NSSYM0(requires), requires);
	}

	if (NewtHasSlot(requires, sym))
		return kNewtRefNIL;

	{
		newtRefVar	initObj[] = {kNewtRefUnbind, NSSTR("."), kNewtRefUnbind};
		struct {
			newtRefVar  exts;
			bool		dylib;
		} lib_exts[] = {
			{NSSTR("dylib"),	true},
			{NSSTR("so"),		true},
			{NSSTR("dll"),		true},
			{NSSTR("newt"),		false},
		};

		newtRefVar  lib;
		newtRefVar  dir;
		newtRefVar  patharray;
		newtRefVar  path;
		uint32_t	len;
		uint32_t	i;
		uint32_t	j;

		patharray = NewtMakeArray2(kNewtRefNIL, sizeof(initObj) / sizeof(newtRefVar), initObj);

		env = NcGetGlobalVar(NSSYM0(_ENV_));
		newtlib = NcGetVariable(env, NSSYM0(NEWTLIB));

		if (NewtRefIsNIL(newtlib))
		{
			newtRefVar	initPath[] = {NSSTR("."), NcGetGlobalVar(NSSYM0(_EXEDIR_))};

			newtlib = NewtMakeArray2(kNewtRefNIL, sizeof(initPath) / sizeof(newtRefVar), initPath);
		}

		len = NewtLength(newtlib);

		for (i = 0; i < len; i++)
		{
			dir = NewtGetArraySlot(newtlib, i);
			NewtSetArraySlot(patharray, 0, NcJoinPath(dir, r));

			for (j = 0; j < sizeof(lib_exts) / sizeof(lib_exts[0]); j++)
			{
				NewtSetArraySlot(patharray, 2, lib_exts[j].exts);
				path = NcStringer(patharray);

				if (NewtFileExists(NewtRefToString(path)))
				{
					if (lib_exts[j].dylib)
					{
						lib = NcLoadLib(path);
						NcSetSlot(requires, sym, lib);
					}
					else
					{
						NcSetSlot(requires, sym, path);
						NcLoad(path);
					}

					return sym;
				}
			}
		}
	}

    return kNewtRefUnbind;
}


/*------------------------------------------------------------------------*/
/** ���C�u�����̗v��
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] ���[�h���郉�C�u�����̃V���{��������
 *
 * @return			���[�h���ꂽ���C�u�����̃V���{��
 *
 * @note			�V���{���ɂ�胉�C�u������v������B
 *					�g���q�͕K�v�Ȃ��B�K�X���C�u�����p�X�ɂ�茟�������B
 *					��x���[�h���ꂽ���C�u�����͓Ǎ��܂�Ȃ��B
 */

newtRef NsRequire(newtRefArg rcvr, newtRefArg r)
{
	newtRefVar  result;

	result = NcRequire0(r);

	if (result == kNewtRefUnbind)
		NewtThrow(kNErrFileNotFound, r);

	return result;
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �t�@�C���̑��݊m�F
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �t�@�C���̃p�X
 *
 * @retval			true	�t�@�C�������݂���
 * @retval			false   �t�@�C�������݂��Ȃ�
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NsFileExists(newtRefArg rcvr, newtRefArg r)
{
    if (! NewtRefIsString(r))
        return NewtThrow(kNErrNotAString, r);

	return NewtMakeBoolean(NewtFileExists(NewtRefToString(r)));
}


#pragma mark -
/*------------------------------------------------------------------------*/
/** �p�X����f�B���N�g��������o��
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �t�@�C���̃p�X
 *
 * @return			�f�B���N�g����
 */

newtRef NsDirName(newtRefArg rcvr, newtRefArg r)
{
	char *  base;
	char *  s;
	char	sep;

    if (! NewtRefIsString(r))
        return NewtThrow(kNErrNotAString, r);

	s = NewtRefToString(r);
    base = NewtBaseName(s, NewtStringLength(r));
	sep = NewtGetFileSeparator();

	if (base != NULL && s < base)
	{
		if (base - 1 != s && *(base - 1) == sep)
			base--;

		if (s < base)
			return NewtMakeString2(s, base - s, false);
	}

	return NSSTR(".");
}


/*------------------------------------------------------------------------*/
/** �p�X����t�@�C��������o��
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] �t�@�C���̃p�X
 *
 * @return			�t�@�C����
 *
 * @note			�X�N���v�g����̌ďo���p
 */

newtRef NsBaseName(newtRefArg rcvr, newtRefArg r)
{
	char *  base;

    if (! NewtRefIsString(r))
        return NewtThrow(kNErrNotAString, r);

    base = NewtBaseName(NewtRefToString(r), NewtStringLength(r));

	if (base != NULL)
		return NSSTR(base);
	else
		return r;
}


/*------------------------------------------------------------------------*/
/** �f�B���N�g�����ƃt�@�C��������p�X���쐬
 *
 * @param rcvr		[in] ���V�[�o
 * @param r1		[in] �f�B���N�g����
 * @param r2		[in] �t�@�C����
 *
 * @return			�쐬���ꂽ�p�X
 */

newtRef NsJoinPath(newtRefArg rcvr, newtRefArg r1, newtRefArg r2)
{
	char		sep = NewtGetFileSeparator();
	newtRefVar	initObj[] = {r1, NewtMakeCharacter(sep), r2};
	newtRefVar  r;

	r = NewtMakeArray2(kNewtRefNIL, sizeof(initObj) / sizeof(newtRefVar), initObj);

	return NcStringer(r);
}


/*------------------------------------------------------------------------*/
/** ���΃p�X���΃p�X�ɓW�J����
 *
 * @param rcvr		[in] ���V�[�o
 * @param r			[in] ���΃p�X
 *
 * @return			��΃p�X
 *
 * @note			~, ~user �̓z�[���f�B���N�g���ɓW�J�����
 *					�X�N���v�g����̌ďo���p
 */

newtRef NsExpandPath(newtRefArg rcvr, newtRefArg r)
{
    if (! NewtRefIsString(r))
        return NewtThrow(kNErrNotAString, r);

    return NewtExpandPath(NewtRefToString(r));
}
