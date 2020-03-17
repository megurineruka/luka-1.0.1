// +--------------------------------------------------
// | c.c
// | 标准C
// | hatsusakana@gmail.com 
// +--------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <ctype.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "md5.h"
#include "cJson.h"
#include "../../kernel/luka.h"

// +--------------------------------------------------
// | 时间
// +--------------------------------------------------

/** 
 * clock
**/
static voidp luka_pkg_c_clock (Luka *luka, voidp *p, size_t n) {
	return luka_put_int(luka, (int)clock());
}

/** 
 * time
 * 获得时间戳
**/
static voidp luka_pkg_c_time (Luka *luka, voidp *p, size_t n) {
	return luka_put_int(luka, (int)time(NULL));
}

/** 
 * localtime
 * 获得时间结构体
**/
static voidp luka_pkg_c_localtime (Luka *luka, voidp *p, size_t n) {
	time_t t = time(NULL);
	struct tm *tmd = NULL;
	voidp obj = NULL;

	if (n >= 1 && luka_is_int(luka, p[0])) {
		t = (time_t)luka_get_int(luka, p[0]);
	}

	tmd = localtime(&t);
	obj = luka_put_object(luka);
	luka_object_put(luka, luka_get_object(luka, obj), "tm_year", luka_put_int(luka, tmd->tm_year));
	luka_object_put(luka, luka_get_object(luka, obj), "tm_mon", luka_put_int(luka, tmd->tm_mon));
	luka_object_put(luka, luka_get_object(luka, obj), "tm_mday", luka_put_int(luka, tmd->tm_mday));
	luka_object_put(luka, luka_get_object(luka, obj), "tm_hour", luka_put_int(luka, tmd->tm_hour));
	luka_object_put(luka, luka_get_object(luka, obj), "tm_min", luka_put_int(luka, tmd->tm_min));
	luka_object_put(luka, luka_get_object(luka, obj), "tm_sec", luka_put_int(luka, tmd->tm_sec));
	luka_object_put(luka, luka_get_object(luka, obj), "tm_isdst", luka_put_int(luka, tmd->tm_isdst));
	return obj;
}


/** 
 * difftime
 * 计算时间差
**/
static voidp luka_pkg_c_difftime (Luka *luka, voidp *p, size_t n) {
	if (n < 2 || !luka_is_int(luka, p[0]) || !luka_is_int(luka, p[1]))
		return luka_null(luka);
	return luka_put_double(luka, difftime(luka_get_int(luka, p[0]), luka_get_int(luka, p[1])));
}

/** 
 * mktime
 * 输入日期结构体,获得时间戳
**/
static voidp luka_pkg_c_mktime (Luka *luka, voidp *p, size_t n) {
	LukaObject *obj = NULL;
	struct tm tmd = {0};

	if (n < 1 || !luka_is_object(luka, p[0]))
		return luka_null(luka);

	obj = luka_get_object(luka, p[0]);
	if (!obj)
		return luka_null(luka);

	tmd.tm_year = luka_get_int(luka, luka_object_get(luka, obj, "tm_year"));
	tmd.tm_mon = luka_get_int(luka, luka_object_get(luka, obj, "tm_mon"));
	tmd.tm_mday = luka_get_int(luka, luka_object_get(luka, obj, "tm_mday"));
	tmd.tm_hour = luka_get_int(luka, luka_object_get(luka, obj, "tm_hour"));
	tmd.tm_min = luka_get_int(luka, luka_object_get(luka, obj, "tm_min"));
	tmd.tm_sec = luka_get_int(luka, luka_object_get(luka, obj, "tm_sec"));
	tmd.tm_isdst = luka_get_int(luka, luka_object_get(luka, obj, "tm_isdst"));
	return luka_put_int(luka, (int)mktime(&tmd));
}

/** 
 * asctime
 * 输入日期结构体,获得日期字符串
**/
static voidp luka_pkg_c_asctime (Luka *luka, voidp *p, size_t n) {
	LukaObject *obj = NULL;
	struct tm tmd = {0};

	if (n < 1 || !luka_is_object(luka, p[0]))
		return luka_null(luka);

	obj = luka_get_object(luka, p[0]);
	if (!obj)
		return luka_null(luka);

	tmd.tm_year = luka_get_int(luka, luka_object_get(luka, obj, "tm_year"));
	tmd.tm_mon = luka_get_int(luka, luka_object_get(luka, obj, "tm_mon"));
	tmd.tm_mday = luka_get_int(luka, luka_object_get(luka, obj, "tm_mday"));
	tmd.tm_hour = luka_get_int(luka, luka_object_get(luka, obj, "tm_hour"));
	tmd.tm_min = luka_get_int(luka, luka_object_get(luka, obj, "tm_min"));
	tmd.tm_sec = luka_get_int(luka, luka_object_get(luka, obj, "tm_sec"));
	tmd.tm_isdst = luka_get_int(luka, luka_object_get(luka, obj, "tm_isdst"));
	return luka_put_string(luka, luka_strdup(luka, asctime(&tmd)));
}

/** 
 * ctime
 * 输入时间戳,获得日期字符串
**/
static voidp luka_pkg_c_ctime (Luka *luka, voidp *p, size_t n) {
	time_t t = 0;

	if (n < 1 || !luka_is_int(luka, p[0]))
		return luka_null(luka);

	t = (time_t)luka_get_int(luka, p[0]);
	return luka_put_string(luka, luka_strdup(luka, ctime(&t)));
}

/** 
 * gmtime
 * 输入时间戳,获得日期结构体
**/
static voidp luka_pkg_c_gmtime (Luka *luka, voidp *p, size_t n) {
	time_t t = 0;
	struct tm *tmd = NULL;
	voidp obj = NULL;

	if (n < 1 || !luka_is_int(luka, p[0]))
		return luka_null(luka);

	t = (time_t)luka_get_int(luka, p[0]);
	tmd = gmtime(&t);
	obj = luka_put_object(luka);
	luka_object_put(luka, luka_get_object(luka, obj), "tm_year", luka_put_int(luka, tmd->tm_year));
	luka_object_put(luka, luka_get_object(luka, obj), "tm_mon", luka_put_int(luka, tmd->tm_mon));
	luka_object_put(luka, luka_get_object(luka, obj), "tm_mday", luka_put_int(luka, tmd->tm_mday));
	luka_object_put(luka, luka_get_object(luka, obj), "tm_hour", luka_put_int(luka, tmd->tm_hour));
	luka_object_put(luka, luka_get_object(luka, obj), "tm_min", luka_put_int(luka, tmd->tm_min));
	luka_object_put(luka, luka_get_object(luka, obj), "tm_sec", luka_put_int(luka, tmd->tm_sec));
	luka_object_put(luka, luka_get_object(luka, obj), "tm_isdst", luka_put_int(luka, tmd->tm_isdst));
	return obj;
}

#ifndef _WIN32

/**
 * strftime
 * 时间戳格式化输出时间
**/
static voidp luka_pkg_c_strftime (Luka *luka, voidp *p, size_t n) {
	char buf[128] = {0};
	time_t t = time(NULL);
	const char *fmt = "%Y-%m-%d %H:%M:%S";
    struct tm *tmd = NULL;

	if (n >= 1 && luka_is_string(luka, p[0])) {
		fmt = luka_get_string(luka, p[0]);
	}

	if (n >= 2 && luka_is_int(luka, p[1])) {
		t = (time_t)luka_get_int(luka, p[1]);
	}

    tmd = localtime(&t);
    strftime(buf, sizeof(buf) - 1, fmt, tmd);
	return luka_put_string(luka, luka_strdup(luka, buf));
}

/**
 * strtotime
 * 时间戳格式化输出时间
**/
static voidp luka_pkg_c_strtotime (Luka *luka, voidp *p, size_t n) {
	const char *buf = NULL;
	time_t t = 0;
	const char *fmt = "%Y-%m-%d %H:%M:%S";
    struct tm tmd;

	if (n == 0 || !luka_is_string(luka, p[0]))
		return luka_null(luka);

	buf = luka_get_string(luka, p[0]);

	if (n > 1 && luka_is_string(luka, p[1])) {
		fmt = luka_get_string(luka, p[1]);
	}

	buf = luka_get_string(luka, p[0]);
    strptime(buf, fmt, &tmd);
	return luka_put_int(luka, (int)mktime(&tmd));
}

#endif

// +--------------------------------------------------
// | 文件
// +--------------------------------------------------

/**
 * fopen
 */
static voidp luka_pkg_c_fopen (Luka *luka, voidp *p, size_t n) {
	FILE *fp = NULL;

	if (n < 2 || !luka_is_string(luka, p[0]) || !luka_is_string(luka, p[1]))
		return luka_null(luka);

	fp = fopen(luka_get_string(luka, p[0]), luka_get_string(luka, p[1]));
	if (!fp)
		return luka_null(luka);

	return luka_put_voidp(luka, fp);
}

/**
 * fclose
 */
static voidp luka_pkg_c_fclose (Luka *luka, voidp *p, size_t n) {
	void *fp = NULL;

	if (n < 1 || !luka_is_voidp(luka, p[0]))
		return luka_null(luka);

	fp = luka_get_voidp(luka, p[0]);
	if (fp) fclose((FILE *)fp);
	return luka_true(luka);
}

/**
 * fgetc
 */
static voidp luka_pkg_c_fgetc (Luka *luka, voidp *p, size_t n) {
	void *fp = NULL;

	if (n < 1 || !luka_is_voidp(luka, p[0]))
		return luka_null(luka);

	fp = luka_get_voidp(luka, p[0]);
	if (!fp)
		return luka_null(luka);

	return luka_put_int(luka, fgetc((FILE *)fp));
}

/**
 * fputc
 */
static voidp luka_pkg_c_fputc (Luka *luka, voidp *p, size_t n) {
	void *fp = NULL;

	if (n < 2 || !luka_is_int(luka, p[0]) || !luka_is_voidp(luka, p[1]))
		return luka_null(luka);

	fp = luka_get_voidp(luka, p[1]);
	if (!fp)
		return luka_null(luka);

	return luka_put_int(luka, fputc(luka_get_int(luka, p[0]), (FILE *)fp));
}

/**
 * fgets
 */
static voidp luka_pkg_c_fgets (Luka *luka, voidp *p, size_t n) {
	void *fp = NULL;
	int size = 0;
	char *str = NULL;

	if (n < 2 || !luka_is_int(luka, p[0]) || !luka_is_voidp(luka, p[1]))
		return luka_null(luka);

	fp = luka_get_voidp(luka, p[1]);
	if (!fp)
		return luka_null(luka);

	size = luka_get_int(luka, p[0]);
	if (size <= 0)
		return luka_null(luka);

	str = (char *)luka_alloc(luka, size + 2);
	fgets(str, size, (FILE *)fp);
	return luka_put_string(luka, str);
}

/**
 * fputs
 */
static voidp luka_pkg_c_fputs (Luka *luka, voidp *p, size_t n) {
	void *fp = NULL;

	if (n < 2 || !luka_is_string(luka, p[0]) || !luka_is_voidp(luka, p[1]))
		return luka_null(luka);

	fp = luka_get_voidp(luka, p[1]);
	if (!fp)
		return luka_null(luka);

	return luka_put_int(luka, fputs(luka_get_string(luka, p[0]), (FILE *)fp));
}

/**
 * fread
 */
static voidp luka_pkg_c_fread (Luka *luka, voidp *p, size_t n) {
	void *fp = NULL;
	int size = 0;
	bytep data = NULL;
	int ret = 0;

	if (n < 2 || !luka_is_int(luka, p[0]) || !luka_is_voidp(luka, p[1]))
		return luka_null(luka);

	fp = luka_get_voidp(luka, p[1]);
	if (!fp)
		return luka_null(luka);

	size = luka_get_int(luka, p[0]);
	if (size <= 0)
		return luka_null(luka);

	data = (bytep)luka_alloc(luka, size);
	ret = fread(data, 1, size, (FILE *)fp);
	return luka_put_byte(luka, data, ret);
}

/**
 * fwrite
 */
static voidp luka_pkg_c_fwrite (Luka *luka, voidp *p, size_t n) {
	void *fp = NULL;
	const char *buf = NULL;
	bytep data = NULL;
	size_t size = 0;

	if (n < 2 || (!luka_is_string(luka, p[0]) && !luka_is_byte(luka, p[0])) || !luka_is_voidp(luka, p[1]))
		return luka_null(luka);

	fp = luka_get_voidp(luka, p[1]);
	if (!fp)
		return luka_null(luka);

	if (luka_is_string(luka, p[0])) {
		buf = luka_get_string(luka, p[0]);
		data = (bytep)buf;
		size = strlen(buf);
	} else {
		data = luka_get_byte(luka, p[0], &size);
	}

	return luka_put_int(luka, fwrite(data, 1, size, (FILE *)fp));
}

/**
 * feof
 */
static voidp luka_pkg_c_feof (Luka *luka, voidp *p, size_t n) {
	void *fp = NULL;

	if (n < 1 || !luka_is_voidp(luka, p[0]))
		return luka_null(luka);

	fp = luka_get_voidp(luka, p[0]);
	if (!fp)
		return luka_null(luka);

	return luka_put_int(luka, feof((FILE *)fp));
}

/**
 * ferror
 */
static voidp luka_pkg_c_ferror (Luka *luka, voidp *p, size_t n) {
	void *fp = NULL;

	if (n < 1 || !luka_is_voidp(luka, p[0]))
		return luka_null(luka);

	fp = luka_get_voidp(luka, p[0]);
	if (!fp)
		return luka_null(luka);

	return luka_put_int(luka, ferror((FILE *)fp));
}

/**
 * clearerr
 */
static voidp luka_pkg_c_clearerr (Luka *luka, voidp *p, size_t n) {
	void *fp = NULL;

	if (n < 1 || !luka_is_voidp(luka, p[0]))
		return luka_null(luka);

	fp = luka_get_voidp(luka, p[0]);
	if (!fp)
		return luka_null(luka);

	clearerr((FILE *)fp);
	return luka_true(luka);
}

/**
 * ftell
 */
static voidp luka_pkg_c_ftell (Luka *luka, voidp *p, size_t n) {
	void *fp = NULL;

	if (n < 1 || !luka_is_voidp(luka, p[0]))
		return luka_null(luka);

	fp = luka_get_voidp(luka, p[0]);
	if (!fp)
		return luka_null(luka);

	return luka_put_int(luka, ftell((FILE *)fp));
}

/**
 * rewind
 */
static voidp luka_pkg_c_rewind (Luka *luka, voidp *p, size_t n) {
	void *fp = NULL;

	if (n < 1 || !luka_is_voidp(luka, p[0]))
		return luka_null(luka);

	fp = luka_get_voidp(luka, p[0]);
	if (!fp)
		return luka_null(luka);

	rewind((FILE *)fp);
	return luka_true(luka);
}

/**
 * fseek
 */
static voidp luka_pkg_c_fseek (Luka *luka, voidp *p, size_t n) {
	void *fp = NULL;
	int offset = 0;
	int base = 0;

	if (n < 3 || !luka_is_voidp(luka, p[0]) || !luka_is_int(luka, p[1]) || !luka_is_int(luka, p[2]))
		return luka_null(luka);

	fp = luka_get_voidp(luka, p[0]);
	if (!fp)
		return luka_null(luka);

	offset = luka_get_int(luka, p[1]);
	base = luka_get_int(luka, p[2]);
	return luka_put_int(luka, fseek((FILE *)fp, offset, base));
}

/**
 * popen
**/
static voidp luka_pkg_c_popen (Luka *luka, voidp *p, size_t n) {
	FILE *fp = NULL;

	if (n < 2 || !luka_is_string(luka, p[0]) || !luka_is_string(luka, p[1]))
		return luka_null(luka);

	fp = popen(luka_get_string(luka, p[0]), luka_get_string(luka, p[1]));
	if (!fp)
		return luka_null(luka);

	return luka_put_voidp(luka, fp);
}

/**
 * pclose
 */
static voidp luka_pkg_c_pclose (Luka *luka, voidp *p, size_t n) {
	void *fp = NULL;

	if (n < 1 || !luka_is_voidp(luka, p[0]))
		return luka_null(luka);

	fp = luka_get_voidp(luka, p[0]);
	if (fp) pclose((FILE *)fp);
	return luka_true(luka);
}

// +--------------------------------------------------
// | 字符串
// +--------------------------------------------------

/**
 * strlen
 */
static voidp luka_pkg_c_strlen (Luka *luka, voidp *p, size_t n) {
	const char *s = NULL;

	if (n < 1 || !luka_is_string(luka, p[0]))
		return luka_null(luka);

	s = luka_get_string(luka, p[0]);
	return luka_put_int(luka, strlen(s));
}

/**
 * strupr
 */
static voidp luka_pkg_c_strupr (Luka *luka, voidp *p, size_t n) {
	char *s = NULL, *orign = NULL;

	if (n < 1 || !luka_is_string(luka, p[0]))
		return luka_null(luka);

	s = luka_strdup(luka, luka_get_string(luka, p[0]));
	for (orign = s; *orign != 0; orign++)
		*orign = toupper(*orign);
	return luka_put_string(luka, s);
}

/**
 * strlowr
 */
static voidp luka_pkg_c_strlowr (Luka *luka, voidp *p, size_t n) {
	char *s = NULL, *orign = NULL;

	if (n < 1 || !luka_is_string(luka, p[0]))
		return luka_null(luka);

	s = luka_strdup(luka, luka_get_string(luka, p[0]));
	for (orign = s; *orign != 0; orign++)
		*orign = tolower(*orign);
	return luka_put_string(luka, s);
}

/**
 * strstr
 */
static voidp luka_pkg_c_strstr (Luka *luka, voidp *p, size_t n) {
	const char *s1 = NULL, *s2 = NULL;
	const char *s3 = NULL;

	if (n < 2 || !luka_is_string(luka, p[0]) || !luka_is_string(luka, p[1]))
		return luka_null(luka);

	s1 = luka_get_string(luka, p[0]);
	s2 = luka_get_string(luka, p[1]);
	if (*s1 == 0 || *s2 == 0)
		return luka_null(luka);

	s3 = strstr(s1, s2);
	if (!s3)
		return luka_null(luka);

	return luka_put_int(luka, s3 - s1);
}

/**
 * strncmp
 */
static voidp luka_pkg_c_strncmp (Luka *luka, voidp *p, size_t n) {
	const char *s1 = NULL, *s2 = NULL;
	int s3 = 0, ret = 0;

	if (n < 3 || !luka_is_string(luka, p[0]) || !luka_is_string(luka, p[1]) || !luka_is_int(luka, p[2]))
		return luka_null(luka);

	s1 = luka_get_string(luka, p[0]);
	s2 = luka_get_string(luka, p[1]);
	if (*s1 == 0 || *s2 == 0)
		return luka_null(luka);

	s3 = luka_get_int(luka, p[2]);
	if (s3 <= 0)
		return luka_null(luka);

	ret = strncmp(s1, s2, s3);
	return ret == 0 ? luka_true(luka) : luka_false(luka);
}

/**
 * substr
 */
static voidp luka_pkg_c_substr (Luka *luka, voidp *p, size_t n) {
	const char *s = NULL;
	size_t s_len = 0;
	int start = 0, length = 0;
	char *ret = NULL;

	if (n < 3 || !luka_is_string(luka, p[0]) || !luka_is_int(luka, p[1]) || !luka_is_int(luka, p[2]))
		return luka_null(luka);

	s = luka_get_string(luka, p[0]);
	s_len = strlen(s);
	start = luka_get_int(luka, p[1]);
	length = luka_get_int(luka, p[2]);

	if (*s == 0 || start < 0 || start >= s_len || length <= 0)
		return luka_null(luka);

	length = length > (int)s_len ? (int)s_len : length;
	ret = (char *)luka_alloc(luka, length + 2);
	memcpy(ret, s + start, length);
	return luka_put_string(luka, ret);
}

// +--------------------------------------------------
// | 其他
// +--------------------------------------------------

/** 
 * sleep
**/
static voidp luka_pkg_c_sleep (Luka *luka, voidp *p, size_t n) {
	int sec = 0;

	if (n < 1 || !luka_is_int(luka, p[0]))
			return luka_null(luka);

	sec = luka_get_int(luka, p[0]);
	if (sec <= 0)
		return luka_null(luka);

#ifdef _WIN32
	if (sec * 1000 < 0)
		return luka_false(luka);

	Sleep(sec * 1000);
#else
	sleep(sec);
#endif

	return luka_true(luka);
}

#ifdef _WIN32

/**
 * Sleep
**/
static voidp luka_pkg_c_Sleep (Luka *luka, voidp *p, size_t n) {
	int sec = 0;

	if (n < 1 || !luka_is_int(luka, p[0]))
			return luka_null(luka);

	sec = luka_get_int(luka, p[0]);
	if (sec <= 0)
		return luka_null(luka);

	Sleep(sec);
	return luka_true(luka);
}

#endif

/**
 * system
**/
static voidp luka_pkg_c_system (Luka *luka, voidp *p, size_t n) {
	if (n < 1 || !luka_is_string(luka, p[0]))
		return luka_null(luka);

	system(luka_get_string(luka, p[0]));
	return luka_true(luka);
}

/** 
 * atoi
**/
static voidp luka_pkg_c_atoi (Luka *luka, voidp *p, size_t n) {
	if (n < 1 || !luka_is_string(luka, p[0]))
		return luka_null(luka);

	return luka_put_int(luka, atoi(luka_get_string(luka, p[0])));
}

/** 
 * atof
**/
static voidp luka_pkg_c_atof (Luka *luka, voidp *p, size_t n) {
	if (n < 1 || !luka_is_string(luka, p[0]))
		return luka_null(luka);

	return luka_put_double(luka, atof(luka_get_string(luka, p[0])));
}

/** 
 * abs
**/
static voidp luka_pkg_c_abs (Luka *luka, voidp *p, size_t n) {
	if (n < 1 || !luka_is_int(luka, p[0]))
		return luka_null(luka);

	return luka_put_int(luka, abs(luka_get_int(luka, p[0])));
}

/** 
 * srand
**/
static voidp luka_pkg_c_srand (Luka *luka, voidp *p, size_t n) {
	if (n < 1 || !luka_is_int(luka, p[0]))
		return luka_null(luka);

	srand(luka_get_int(luka, p[0]));
	return luka_true(luka);
}

/** 
 * rand
**/
static voidp luka_pkg_c_rand (Luka *luka, voidp *p, size_t n) {
	if (n == 0)
		return luka_put_int(luka, rand());
	else if (n == 1 && luka_is_int(luka, p[0]))
		return luka_put_int(luka, rand() % luka_get_int(luka, p[0]));
	else if (n >= 2 && luka_is_int(luka, p[0]) && luka_is_int(luka, p[1])) {
		int left = luka_get_int(luka, p[0]), right = luka_get_int(luka, p[1]);
		return luka_put_int(luka, rand() % (right - left) + left);
	}
	return luka_put_int(luka, rand());
}

/** 
 * is_win32
**/
static voidp luka_pkg_c_is_win32 (Luka *luka, voidp *p, size_t n) {
#ifdef _WIN32
	return luka_true(luka);
#else
	return luka_false(luka);
#endif
}

static const char *ALPHA_BASE64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static char *base64_encode (Luka *luka, const char *src) {
    size_t i = 0, j = 0, len = 0;
    char buf[3] = {0};

    char  *dst = NULL;
    size_t size = 0;

    if (!src || (len = strlen(src)) == 0)
        return NULL;

    size = (len%3 == 0 ? len/3 : len/3 + 1) * 4 + 1;
    dst = (char *)luka_alloc(luka, size);
    if (!dst)
        return NULL;

    for (i = 0, j = 0; i < len; ) {
        buf[0] = src[i++];
        buf[1] = i < len ? src[i++] : '\0';
        buf[2] = i < len ? src[i++] : '\0';
        
        dst[j++] = ALPHA_BASE64[(buf[0] >> 2) & 0x3F];
        dst[j++] = ALPHA_BASE64[((buf[0] << 4) | ((buf[1] & 0xFF) >> 4)) & 0x3F];
        dst[j++] = ALPHA_BASE64[((buf[1] << 2) | ((buf[2] & 0xFF) >> 6)) & 0x3F];
        dst[j++] = ALPHA_BASE64[buf[2] & 0x3F];
    }
    
    switch (len % 3) {
        case 1: dst[--j] = '=';
        case 2: dst[--j] = '=';
    }
    return dst;
}

static char *base64_decode (Luka *luka, const char *src) {
    size_t i = 0, j = 0, len = 0;
    char buf[4] = {0};
    int toInt[128] = {-1};

    char  *dst = NULL;
    size_t size = 0;

    if (!src || (len = strlen(src)) == 0 || len % 4 != 0)
        return NULL;

    size = len/4*3 + 1;
    dst = (char *)luka_alloc(luka, size);
    if (!dst)
        return NULL;

    for (i = 0; i < 64; i++)
        toInt[ALPHA_BASE64[i]] = i;

    for (i = 0, j = 0; i < len; i += 4) {
        buf[0] = toInt[src[i]];
        buf[1] = toInt[src[i + 1]];
        dst[j++] = (((buf[0] << 2) | (buf[1] >> 4)) & 0xff);
        buf[2] = toInt[src[i + 2]];
        dst[j++] = (((buf[1] << 4) | (buf[2] >> 2)) & 0xff);
        buf[3] = toInt[src[i + 3]];
        dst[j++] = (((buf[2] << 6) | buf[3]) & 0xff);
    }
    return dst;
}

/** 
 * base64_encode
**/
static voidp luka_pkg_c_base64_encode (Luka *luka, voidp *p, size_t n) {
	const char *str = NULL;

	if (n < 1 || !luka_is_string(luka, p[0]))
		return luka_null(luka);

	str = luka_get_string(luka, p[0]);
	if (*str == 0)
		return luka_null(luka);

	return luka_put_string(luka, base64_encode(luka, str));
}

/** 
 * base64_decode
**/
static voidp luka_pkg_c_base64_decode (Luka *luka, voidp *p, size_t n) {
	const char *str = NULL;

	if (n < 1 || !luka_is_string(luka, p[0]))
		return luka_null(luka);

	str = luka_get_string(luka, p[0]);
	if (*str == 0)
		return luka_null(luka);

	return luka_put_string(luka, base64_decode(luka, str));
}

/** 
 * md5
**/
static voidp luka_pkg_c_md5 (Luka *luka, voidp *p, size_t n) {
	const char *str = NULL;
	char buf[256] = {0};

	if (n < 1 || !luka_is_string(luka, p[0]))
		return luka_null(luka);

	str = luka_get_string(luka, p[0]);
	if (*str == 0)
		return luka_null(luka);

	MD5Encode32(str, buf, sizeof(buf) - 1);
	return luka_put_string(luka, luka_strdup(luka, buf));
}

static int hex2dec (char c) {
    if ('0' <= c && c <= '9') {
        return c - '0';
    } else if ('a' <= c && c <= 'f') {
        return c - 'a' + 10;
    } else if ('A' <= c && c <= 'F') {
        return c - 'A' + 10;
    }
    return -1;
}

static char dec2hex (short int c) {
    if (0 <= c && c <= 9) {
        return c + '0';
    } else if (10 <= c && c <= 15) {
        return c + 'A' - 10;
    }
    return -1;
}

/** 
 * url_encode
**/
static voidp luka_pkg_c_url_encode (Luka *luka, voidp *p, size_t n) {
	const char *str = NULL;
	size_t i = 0, len = 0;

	char *new_str = NULL;
	size_t new_len = 0, new_i = 0;

	if (n < 1 || !luka_is_string(luka, p[0]))
		return luka_null(luka);

	str = luka_get_string(luka, p[0]);
	if (*str == 0)
		return p[0];

	len = strlen(str);
	new_len = (len * 4) + 5;
	new_str = (char *)luka_alloc(luka, new_len);

	for (i = 0; i < len; ++i) {
		char c = str[i];

		if (    ('0' <= c && c <= '9') ||
                ('a' <= c && c <= 'z') ||
                ('A' <= c && c <= 'Z') ||
                c == '/' || c == '.')
        {
            new_str[new_i++] = c;
        } else {
            int j = (short int)c;
            int i1, i0;

            if (j < 0)
                j += 256;
            i1 = j / 16;
            i0 = j - i1 * 16;
            new_str[new_i++] = '%';
            new_str[new_i++] = dec2hex(i1);
            new_str[new_i++] = dec2hex(i0);
        }
	}

	return luka_put_string(luka, new_str);
}

/** 
 * url_decode
**/
static voidp luka_pkg_c_url_decode (Luka *luka, voidp *p, size_t n) {
	const char *str = NULL;
	size_t i = 0, len = 0;

	char *new_str = NULL;
	size_t new_len = 0, new_i = 0;

	if (n < 1 || !luka_is_string(luka, p[0]))
		return luka_null(luka);

	str = luka_get_string(luka, p[0]);
	if (*str == 0)
		return p[0];

	len = strlen(str);
	new_len = len + 5;
	new_str = (char *)luka_alloc(luka, new_len);

	for (i = 0; i < len; ++i) {
        char c = str[i];

        if (c != '%') {
            if (c == '+')
                new_str[new_i++] = ' ';
            else
                new_str[new_i++] = c;
        } else {
            char c1 = str[++i];
            char c0 = str[++i];
            int num = 0;
            num = hex2dec(c1) * 16 + hex2dec(c0);
            new_str[new_i++] = num;
        }
    }

	return luka_put_string(luka, new_str);
}

/********************************************************************/

typedef struct StringBuffer {
    unsigned char *data;
    size_t size;
    struct StringBuffer *next;
} StringBuffer;

static void sb_add (Luka *luka, StringBuffer **sb, unsigned char *data, size_t size) {
    StringBuffer *b = NULL, *bf = NULL;
    
    b = (StringBuffer *)luka_alloc(luka, sizeof(StringBuffer));
    if (b == NULL) return;
    b->data = (unsigned char *)luka_alloc(luka, size);
    if (b->data == NULL) {
        free(b);
        return;
    }
    memcpy(b->data, data, size);
    b->size = size;
    b->next = NULL;
    
    if (*sb == NULL) {
        *sb = b;
    } else {
        bf = *sb;
        while (bf->next) bf = bf->next;
        bf->next = b;
    }
}

static void sb_addstr (Luka *luka, StringBuffer **sb, const char *str) {
	if (str && *str != 0)
		sb_add(luka, sb, (unsigned char *)str, strlen(str));
}

static char *sb_tostring (Luka *luka, StringBuffer *sb) {
    StringBuffer *b = sb;
    int size = 1, i = 0;
    char *data = NULL;

    while (b) {
        size += b->size;
        b = b->next;
    }

    data = (char *)luka_alloc(luka, size);
    if (!data) return NULL;
    memset(data, 0, size);

    i = 0;
    b = sb;
    while (b) {
        memcpy(data + i, b->data, b->size);
        i += b->size;
        b = b->next;
    }
    return data;
}

static void sb_free (Luka *luka, StringBuffer **sb) {
    StringBuffer *b = NULL, *bf = NULL;

    if (!sb || *sb == NULL) return;

    b = (*sb);
    while (b) {
        bf = b;
        b = b->next;
        luka_free(luka, bf->data);
        luka_free(luka, bf);
    }
    *sb = NULL;
}

static cJSON *luka_pkg_c_json_encode_obj_tostring   (Luka *luka, LukaObject *object);
static cJSON *luka_pkg_c_json_encode_array_tostring (Luka *luka, LukaArray *array);

static cJSON *luka_pkg_c_json_encode_obj_tostring (Luka *luka, LukaObject *object) {
	StrList *list = NULL, *buf = NULL;
	cJSON *json = cJSON_CreateObject();

	list = luka_object_each(luka, object);
	buf = list;
	while (buf) {
		voidp data = luka_object_get(luka, object, buf->s);
		cJSON *json_son = NULL;

		if (luka_is_null(luka, data)) {
			json_son = cJSON_CreateNull();
		} else if (luka_is_true(luka, data)) {
			json_son = cJSON_CreateTrue();
		} else if (luka_is_false(luka, data)) {
			json_son = cJSON_CreateFalse();
		} else if (luka_is_int(luka, data)) {
			json_son = cJSON_CreateNumber((double)luka_get_int(luka, data));
		} else if (luka_is_double(luka, data)) {
			json_son = cJSON_CreateNumber((double)luka_get_double(luka, data));
		} else if (luka_is_string(luka, data)) {
			json_son = cJSON_CreateString(luka_get_string(luka, data));
		} else if (luka_is_object(luka, data)) {
			json_son = luka_pkg_c_json_encode_obj_tostring(luka, luka_get_object(luka, data));
		} else if (luka_is_array(luka, data)) {
			json_son = luka_pkg_c_json_encode_array_tostring(luka, luka_get_array(luka, data));
		}

		if (json_son) {
			cJSON_AddItemToObject(json, buf->s, json_son);
		}

		buf = buf->next;
	}
	sl_free(luka, list);
	return json;
}

static cJSON *luka_pkg_c_json_encode_array_tostring (Luka *luka, LukaArray *array) {
	size_t i = 0;
	cJSON *json = cJSON_CreateArray();

	for (i = 0; i < luka_array_length(luka, array); i++) {
		voidp data = luka_array_get(luka, array, i);
		cJSON *json_son = NULL;

		if (luka_is_null(luka, data)) {
			json_son = cJSON_CreateNull();
		} else if (luka_is_true(luka, data)) {
			json_son = cJSON_CreateTrue();
		} else if (luka_is_false(luka, data)) {
			json_son = cJSON_CreateFalse();
		} else if (luka_is_int(luka, data)) {
			json_son = cJSON_CreateNumber((double)luka_get_int(luka, data));
		} else if (luka_is_double(luka, data)) {
			json_son = cJSON_CreateNumber((double)luka_get_double(luka, data));
		} else if (luka_is_string(luka, data)) {
			json_son = cJSON_CreateString(luka_get_string(luka, data));
		} else if (luka_is_object(luka, data)) {
			json_son = luka_pkg_c_json_encode_obj_tostring(luka, luka_get_object(luka, data));
		} else if (luka_is_array(luka, data)) {
			json_son = luka_pkg_c_json_encode_array_tostring(luka, luka_get_array(luka, data));
		}

		if (json_son) {
			cJSON_AddItemToArray(json, json_son);
		}
	}

	return json;
}

/**
 * json_encode
 */
static voidp luka_pkg_c_json_encode (Luka *luka, voidp *p, size_t n) {
	LukaObject *object = NULL;
	LukaArray  *array  = NULL;
	cJSON *json = NULL;
	char *json_string = NULL, *new_string = NULL;

	if (n < 1 || (!luka_is_object(luka, p[0]) && !luka_is_array(luka, p[0])))
		return luka_null(luka);

	if (luka_is_object(luka, p[0])) {
		object = luka_get_object(luka, p[0]);
		json = luka_pkg_c_json_encode_obj_tostring(luka, object);
	}

	else {
		array = luka_get_array(luka, p[0]);
		json = luka_pkg_c_json_encode_array_tostring(luka, array);
	}

	json_string = cJSON_PrintUnformatted(json);
	if (!json_string) {
		cJSON_Delete(json);
		return luka_null(luka);
	}

	new_string = luka_strdup(luka, json_string);
	free(json_string);
	cJSON_Delete(json);
	return luka_put_string(luka, new_string);
}

voidp luka_pkg_c_json_decode_ex (Luka *luka, cJSON *json);
voidp luka_pkg_c_json_decode_to_obj (Luka *luka, cJSON *obj);
voidp luka_pkg_c_json_decode_to_array (Luka *luka, cJSON *array);

voidp luka_pkg_c_json_decode_ex (Luka *luka, cJSON *json) {
	voidp data = luka_null(luka);

	if (json->type == cJSON_False) {
		return luka_false(luka);
	} else if (json->type == cJSON_True) {
		return luka_true(luka);
	} else if (json->type == cJSON_NULL) {
		return luka_null(luka);
	} else if (json->type == cJSON_Number) {
		return luka_put_double(luka, json->valuedouble);
	} else if (json->type == cJSON_String) {
		return luka_put_string(luka, luka_strdup(luka, json->valuestring));
	} else if (json->type == cJSON_Array) {
		data = luka_pkg_c_json_decode_to_array(luka, json);
	} else if (json->type == cJSON_Object) {
		data = luka_pkg_c_json_decode_to_obj(luka, json);
	}

	return data;
}

voidp luka_pkg_c_json_decode_to_obj (Luka *luka, cJSON *obj) {
	LukaObject *object = NULL;
	voidp data = luka_put_object(luka);
	cJSON *buf = obj->child;
	object = luka_get_object(luka, data);

	while (buf) {
		cJSON *son = cJSON_GetObjectItem(obj, buf->string);
		luka_object_put(luka, object, buf->string, luka_pkg_c_json_decode_ex(luka, son));
		buf = buf->next;
	}

	return data;
}

voidp luka_pkg_c_json_decode_to_array (Luka *luka, cJSON *array) {
	LukaArray *array2 = NULL;
	voidp data = luka_put_array(luka);
	size_t i = 0;
	array2 = luka_get_array(luka, data);

	for (i = 0; i < cJSON_GetArraySize(array); i++) {
		cJSON *son = cJSON_GetArrayItem(array, i);
		luka_array_push(luka, array2, luka_pkg_c_json_decode_ex(luka, son));
	}

	return data;
}

/**
 * json_decode
 */
static voidp luka_pkg_c_json_decode (Luka *luka, voidp *p, size_t n) {
	const char *str = NULL;
	cJSON *json = NULL;
	voidp data = luka_null(luka);

	if (n < 1 || !luka_is_string(luka, p[0]))
		return luka_null(luka);

	str = luka_get_string(luka, p[0]);
	if (*str == 0)
		return luka_null(luka);

	json = cJSON_Parse(str);
	if (!json)
		return luka_null(luka);

	data = luka_pkg_c_json_decode_ex(luka, json);
	cJSON_Delete(json);
	return data;
}

void luka_pkg_c_regs (Luka *luka) {
	luka_reg(luka, "clock",         luka_pkg_c_clock);
	luka_reg(luka, "time",          luka_pkg_c_time);
	luka_reg(luka, "localtime",     luka_pkg_c_localtime);
	luka_reg(luka, "difftime",      luka_pkg_c_difftime);
	luka_reg(luka, "mktime",        luka_pkg_c_mktime);
	luka_reg(luka, "asctime",       luka_pkg_c_asctime);
	luka_reg(luka, "ctime",         luka_pkg_c_ctime);
	luka_reg(luka, "gmtime",        luka_pkg_c_gmtime);
#ifndef _WIN32
	luka_reg(luka, "strftime",      luka_pkg_c_strftime);
	luka_reg(luka, "strtotime",     luka_pkg_c_strtotime);
#endif

	luka_reg(luka, "fopen",         luka_pkg_c_fopen);
	luka_reg(luka, "fclose",        luka_pkg_c_fclose);
	luka_reg(luka, "fgetc",         luka_pkg_c_fgetc);
	luka_reg(luka, "fputc",         luka_pkg_c_fputc);
	luka_reg(luka, "fgets",         luka_pkg_c_fgets);
	luka_reg(luka, "fputs",         luka_pkg_c_fputs);
	luka_reg(luka, "fread",         luka_pkg_c_fread);
	luka_reg(luka, "fwrite",        luka_pkg_c_fwrite);
	luka_reg(luka, "feof",          luka_pkg_c_feof);
	luka_reg(luka, "ferror",        luka_pkg_c_ferror);
	luka_reg(luka, "clearerr",      luka_pkg_c_clearerr);
	luka_reg(luka, "ftell",         luka_pkg_c_ftell);
	luka_reg(luka, "rewind",        luka_pkg_c_rewind);
	luka_reg(luka, "fseek",         luka_pkg_c_fseek);
	luka_reg(luka, "popen",         luka_pkg_c_popen);
	luka_reg(luka, "pclose",        luka_pkg_c_pclose);

	luka_reg(luka, "strlen",        luka_pkg_c_strlen);
	luka_reg(luka, "strupr",        luka_pkg_c_strupr);
	luka_reg(luka, "strlowr",       luka_pkg_c_strlowr);
	luka_reg(luka, "strstr",        luka_pkg_c_strstr);
	luka_reg(luka, "strncmp",       luka_pkg_c_strncmp);
	luka_reg(luka, "substr",        luka_pkg_c_substr);

	luka_reg(luka, "sleep",         luka_pkg_c_sleep);
#ifdef _WIN32
	luka_reg(luka, "Sleep",         luka_pkg_c_Sleep);
#endif
	luka_reg(luka, "system",        luka_pkg_c_system);
	luka_reg(luka, "atoi",          luka_pkg_c_atoi);
	luka_reg(luka, "atof",          luka_pkg_c_atof);
	luka_reg(luka, "abs",           luka_pkg_c_abs);
	luka_reg(luka, "srand",         luka_pkg_c_srand);
	luka_reg(luka, "rand",          luka_pkg_c_rand);

	luka_reg(luka, "is_win32",      luka_pkg_c_is_win32);
	luka_reg(luka, "base64_encode", luka_pkg_c_base64_encode);
	luka_reg(luka, "base64_decode", luka_pkg_c_base64_decode);
	luka_reg(luka, "md5",           luka_pkg_c_md5);
	luka_reg(luka, "url_encode",    luka_pkg_c_url_encode);
	luka_reg(luka, "url_decode",    luka_pkg_c_url_decode);
	luka_reg(luka, "json_encode",   luka_pkg_c_json_encode);
	luka_reg(luka, "json_decode",   luka_pkg_c_json_decode);
}
