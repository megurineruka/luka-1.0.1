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

/**
 * tostring
 */
static voidp luka_pkg_c_tostring (Luka *luka, voidp *p, size_t n) {
	voidp pr = luka_null(luka);

	if (luka_is_string(luka, p[0])) {
		pr = p[0];
	} else if (luka_is_int(luka, p[0])) {
		char temp[25] = {0};
		memset(temp, 0, sizeof(temp));
		sprintf(temp, "%d", luka_get_int(luka, p[0]));
		pr = luka_put_string(luka, luka_strdup(luka, temp));
	} else if (luka_is_double(luka, p[0])) {
		char temp[25] = {0};
		memset(temp, 0, sizeof(temp));
		sprintf(temp, "%g", luka_get_double(luka, p[0]));
		pr = luka_put_string(luka, luka_strdup(luka, temp));
	} else if (luka_is_byte(luka, p[0])) {
		char *temp2 = NULL;
		bytep datap = NULL;
		size_t size = 0;
		datap = luka_get_byte(luka, p[0], &size);

		temp2 = (char *)luka_alloc(luka, size + 5);
		memcpy(temp2, datap, size);
		pr = luka_put_string(luka, temp2);
	}
	return pr;
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

void luka_pkg_c_regs (Luka *luka) {
	luka_reg(luka, "clock",     luka_pkg_c_clock);
	luka_reg(luka, "time",      luka_pkg_c_time);
	luka_reg(luka, "localtime", luka_pkg_c_localtime);
	luka_reg(luka, "difftime",  luka_pkg_c_difftime);
	luka_reg(luka, "mktime",    luka_pkg_c_mktime);
	luka_reg(luka, "asctime",   luka_pkg_c_asctime);
	luka_reg(luka, "ctime",     luka_pkg_c_ctime);
	luka_reg(luka, "gmtime",    luka_pkg_c_gmtime);
#ifndef _WIN32
	luka_reg(luka, "strftime",  luka_pkg_c_strftime);
	luka_reg(luka, "strtotime", luka_pkg_c_strtotime);
#endif

	luka_reg(luka, "fopen",     luka_pkg_c_fopen);
	luka_reg(luka, "fclose",    luka_pkg_c_fclose);
	luka_reg(luka, "fgetc",     luka_pkg_c_fgetc);
	luka_reg(luka, "fputc",     luka_pkg_c_fputc);
	luka_reg(luka, "fgets",     luka_pkg_c_fgets);
	luka_reg(luka, "fputs",     luka_pkg_c_fputs);
	luka_reg(luka, "fread",     luka_pkg_c_fread);
	luka_reg(luka, "fwrite",    luka_pkg_c_fwrite);
	luka_reg(luka, "feof",      luka_pkg_c_feof);
	luka_reg(luka, "ferror",    luka_pkg_c_ferror);
	luka_reg(luka, "clearerr",  luka_pkg_c_clearerr);
	luka_reg(luka, "ftell",     luka_pkg_c_ftell);
	luka_reg(luka, "rewind",    luka_pkg_c_rewind);
	luka_reg(luka, "fseek",     luka_pkg_c_fseek);

	luka_reg(luka, "strlen",    luka_pkg_c_strlen);
	luka_reg(luka, "strupr",    luka_pkg_c_strupr);
	luka_reg(luka, "strlowr",   luka_pkg_c_strlowr);
	luka_reg(luka, "strstr",    luka_pkg_c_strstr);
	luka_reg(luka, "strncmp",   luka_pkg_c_strncmp);
	luka_reg(luka, "substr",    luka_pkg_c_substr);
	luka_reg(luka, "tostring",  luka_pkg_c_tostring);

	luka_reg(luka, "sleep",     luka_pkg_c_sleep);
#ifdef _WIN32
	luka_reg(luka, "Sleep",     luka_pkg_c_Sleep);
#endif
	luka_reg(luka, "system",    luka_pkg_c_system);
	luka_reg(luka, "atoi",      luka_pkg_c_atoi);
	luka_reg(luka, "atof",      luka_pkg_c_atof);
	luka_reg(luka, "abs",       luka_pkg_c_abs);
	luka_reg(luka, "srand",     luka_pkg_c_srand);
	luka_reg(luka, "rand",      luka_pkg_c_rand);
}
