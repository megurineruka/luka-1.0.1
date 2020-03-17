// +--------------------------------------------------
// | gcc.c 
// | hatsusakana@gmail.com 
// +--------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

#include "luka.h"

static const char *g_Name = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_";
static const char *g_Num  = "0123456789";
static const char *g_Sys[] = {
    "include", "package", "if", "else", "while", "function", "return", "for", ""
};

static int _isname (const char *s) {
    size_t i = 0;
    if (!strchr(g_Name, *s) || strchr(g_Num, *s))
        return 0;
    s++;

    while (*s != 0) {
        if (!strchr(g_Name, *s))
            return 0;
        s++;
    }

    //检查关键字
    while (strcmp(g_Sys[i], "") != 0) {
        if (strcmp(g_Sys[i++], s) == 0) return 0;
    }
    return 1;
}

/** 目录符 **/
static char _get_dir_c () {
#ifdef _WIN32
    return '\\';
#else
    return '/';
#endif
}

/** 是否为绝对路径 **/
static int _absolute_path (const char *path) {
#ifdef _WIN32
    return (toupper(*path) >= 'A' && toupper(*path) <= 'Z') && *(path + 1) == ':';
#else
    return *path == '/';
#endif
}

/** 剔除一部分字符串 **/
void str_rmv (char *s, size_t start, size_t len) {
	size_t s_len = 0;

	if (!s || *s == 0)
		return;

	s_len = strlen(s);
	if (start > s_len || len == 0)
		return;

	s_len = strlen(s + start + len);
	len = len > s_len ? s_len : len;

	memset(s + start, 0, len);
    memmove(s + start, s + start + len, s_len + 1);
}

/** 获得当前窗口目录 **/
static void _get_pwd (char *path, size_t len) {
#ifdef _WIN32
    _getcwd(path, len);
#else
    getcwd(path, len);
#endif
}

/** 移除./和../ **/
static void _clear_path (char *full_path) {
	char *p = NULL, *p2 = NULL;

	while ((p = strstr(full_path, "../")) != NULL || (p = strstr(full_path, "..\\")) != NULL) {
		p2 = p - 2;  //退回2个字符
		while (p2 != full_path && *p2 != '/' && *p2 != '\\')
			p2--;

		if (p2 == full_path)
			return;

		str_rmv(p2, 1, p + 2 - p2);
	}

	while ((p = strstr(full_path, "./")) != NULL || (p = strstr(full_path, ".\\")) != NULL) {
		str_rmv(p, 0, 2);
	}
}

static char *_trim (char *s) {
    char *p = s + strlen(s) - 1;
    while (*p <= 32)
        *p-- = '\0';
    p = s;

    while (*p <= 32 && *p != 0)
        p++;
    if (p != s)
        memmove(s, p, strlen(p) + 1);
    return s;
}

/** 读取整个文本文件 **/
static char *luka_readtxt (Luka *luka, const char *path) {
    FILE *fp = NULL;
    char *data = NULL;
    long  size = 0;

    if ((fp = fopen(path, "rb")) == NULL) {
        fprintf(stderr, "can't open '%s'\n", path);
        luka_exception(luka, 3);
    }

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fclose(fp);

    if (size < 0) {
        fprintf(stderr, "file size error %ld\n", size);
        luka_exception(luka, 3);
    }

    if ((fp = fopen(path, "rb")) == NULL) {
        fprintf(stderr, "can't open '%s'\n", path);
        luka_exception(luka, 3);
    }

    data = (char *)luka_alloc(luka, size + 10);
    fread(data, 1, size, fp);
    fclose(fp);
    return data;
}

/** 从路径中获取文件名 **/
static char *luka_basename (Luka *luka, const char *full_path) {
    char *p = NULL;

    if ((p = strrchr(full_path, '/')) == NULL && (p = strrchr(full_path, '\\')) == NULL) {
    	p = strrchr(full_path, ':');
    }

    return luka_strdup(luka, p + 1);
}

/** 从路径中剔除文件名 **/
static char *luka_basepath (Luka *luka, const char *path) {
    char *new_path = NULL, *p = NULL;

    new_path = luka_strdup(luka, path);
    if ((p = strrchr(new_path, '/')) == NULL && (p = strrchr(new_path, '\\')) == NULL) {
    	p = strrchr(new_path, ':');
    	p++;
    }

    *p = '\0';
    return new_path;
}

/** 获得之前连续几个\的数量 **/
static size_t _count_trans_num (const char *s) {
    size_t count = 0;
    while (*s == '\\') {
        count++; s--;
    }
    return count;
}

/** 扫描至c(需处理字符串转义/')'问题) **/
static const char *luka_strchr (const char *s, char c) {
    char instr = 0;
    size_t left = 0;

    while (*s != 0) {
        if (instr == 0) {
            if (*s == '\'' || *s == '\"') {
                instr = *s;
            } else if (*s == c && left == 0)
                return s;

            if (*s == '(') left++;
            else if (*s == ')') left--;
        } else {
            if (*s == instr && _count_trans_num(s - 1) % 2 == 0) {
                instr = 0;
            }
        }
        s++;
    }
    return NULL;
}

static char *luka_strsub (Luka *luka, const char *left, const char *right) {
    size_t len = 0;
    char  *ret = NULL;

    len = right - left + 1;
    ret = (char *)luka_alloc(luka, len + 10);
    memcpy(ret, left, len);
    return ret;
}

static void luka_linecheck (const char *s, size_t *line) {
    while (*s != '\0') {
        if (*s == '\n')
            *line = (*line) + 1;
        s++;
    }
}

static char **luka_splite (Luka *luka, const char *s, char c, size_t *n) {
    const char *left = s, *right = NULL;
    char **ret = NULL;
    size_t i = 0, number = 1;

    while ((right = luka_strchr(left, c)) != NULL) {
        number++;
        left = right + 1;
    }

    ret = (char **)luka_alloc(luka, sizeof(char *) * number);
    left = s;
    right = NULL;
    while ((right = luka_strchr(left, c)) != NULL) {
        if (left == right) {
            ret[i++] = luka_strdup(luka, "");
        } else {
            ret[i++] = _trim(luka_strsub(luka, left, right - 1));
        }
        left = right + 1;
    }

    ret[i++] = _trim(luka_strdup(luka, left));
    *n = number;
    return ret;
}

/************************************************************************************/

typedef struct LukaGCCData {
    char  *script_name;
    char  *script_path;
    char  *script_txt;

    size_t line;
    const char *p;

    const char *express_left, *express_right;
    char *express_str;
} LukaGCCData;

static void luka_gcc_create (Luka *luka, LukaGCCData *gcc_data, const char *full_path) {
    gcc_data->script_txt = luka_readtxt(luka, full_path);
    gcc_data->script_name = luka_basename(luka, full_path);
    gcc_data->script_path = luka_basepath(luka, full_path);

    gcc_data->line = 1;
    gcc_data->p = gcc_data->script_txt;
    gcc_data->express_left = gcc_data->express_right = NULL;
    gcc_data->express_str = NULL;
}

static void luka_gcc_destroy (Luka *luka, LukaGCCData *gcc) {
    luka_free(luka, gcc->script_path);
    luka_free(luka, gcc->script_txt);
}

typedef struct LukaGCC2 { 
    LukaGCC *head, *tail;
} LukaGCC2;

static void luka_gcc_decode_ex2 (Luka *luka, LukaGCC2 *gcc, LukaGCCData *gcc_data);

static void luka_gcc_decode_ex (Luka *luka,LukaGCC2 *gcc, const char *full_path) {
    LukaGCCData gcc_data = {0};
    luka_gcc_create(luka, &gcc_data, full_path);
    luka_gcc_decode_ex2(luka, gcc, &gcc_data);
    luka_gcc_destroy(luka, &gcc_data);
}

LukaGCC *luka_gcc_decode (Luka *luka, const char *luka_script_path) {
    LukaGCC2 gcc = {0};

    if (_absolute_path(luka_script_path)) {
        luka_gcc_decode_ex(luka, &gcc, luka_script_path);
    } else {
        char *full_path = NULL;
        char pwd_path[1024] = {0};
        size_t len1 = 0, len2 = 0;

        _get_pwd(pwd_path, sizeof(pwd_path) - 1);
        len1 = strlen(pwd_path);
        len2 = strlen(luka_script_path);
        full_path = (char *)luka_alloc(luka, len1 + len2 + 5);
        sprintf(full_path, "%s%c%s", pwd_path, _get_dir_c(), luka_script_path);
    	_clear_path(full_path);
        luka_gcc_decode_ex(luka, &gcc, full_path);
        luka_free(luka, full_path);
    }
    return gcc.head;
}

void luka_gcc_free (Luka *luka, LukaGCC *gcc) {
    size_t i = 0;
    LukaGCC *buf = NULL;

    while (gcc) {
        buf = gcc;
        gcc = gcc->next;

        if (buf->express) luka_free(luka, buf->express);

        for (i = 0; i < buf->a_len; i++)
            luka_free(luka, buf->a[i]);
        if (buf->a) luka_free(luka, buf->a);

        for (i = 0; i < buf->b_len; i++)
            luka_free(luka, buf->b[i]);
        if (buf->b) luka_free(luka, buf->b);

        for (i = 0; i < buf->c_len; i++)
            luka_free(luka, buf->c[i]);
        if (buf->c) luka_free(luka, buf->c);

        luka_free(luka, buf);
    }
}

static LukaGCC *luka_gcc_createnode (Luka *luka, const char *script_name, size_t line, LukaGCCType type, char *express, 
    char **a, char **b, char **c, size_t a_len, size_t b_len, size_t c_len, char *func_name, char **func_param, size_t func_len) {
    LukaGCC *node = (LukaGCC *)luka_alloc(luka, sizeof(LukaGCC));
    node->script_name = script_name;
    node->line = line;
    node->type = type;
    node->express = express;
    node->a = a;
    node->b = b;
    node->c = c;
    node->a_len = a_len;
    node->b_len = b_len;
    node->c_len = c_len;
    node->func_name = func_name;
    node->func_param = func_param;
    node->func_len = func_len;
    return node;
}

static void luka_gcc_push (Luka *luka, LukaGCC2 *gcc, LukaGCC *node) {
    if (!gcc->head) {
        gcc->head = gcc->tail = node;
    } else {
        gcc->tail->next = node;
        gcc->tail = node;
    }
}

typedef enum {
    LUKA_GCCS_NULL    = 0,   //空闲
    LUKA_GCCS_NOTE1   = 1,   //# //
    LUKA_GCCS_NOTE2   = 2,   // /* */
    LUKA_GCCS_INCLUDE = 3,   //include
    LUKA_GCCS_WHILE   = 4,   //while
    LUKA_GCCS_IF      = 5,   //if
    LUKA_GCCS_ELSE    = 6,   //else/else if
    LUKA_GCCS_EXPRESS = 7,   //express
    LUKA_GCCS_FOR     = 8,   //for
    LUKA_GCCS_FUNC    = 9,   //function
    LUKA_GCCS_RETURN  = 10,  //return
	LUKA_GCCS_PACKAGE = 11	 //package
} LukaGCCState;

static void luka_gcc_decode_ex2 (Luka *luka, LukaGCC2 *gcc, LukaGCCData *gcc_data) {
    LukaGCCState state = LUKA_GCCS_NULL;
    LukaGCC *node = NULL;
    size_t i = 0;

    while (*gcc_data->p != 0) {
        if (state == LUKA_GCCS_NULL) {
            while (*gcc_data->p <= 32 && *gcc_data->p != 0) {
                if (*gcc_data->p == '\n')
                    gcc_data->line++;
                gcc_data->p++;
            }
            if (*gcc_data->p == 0) break;

            //单行注释
            if (*gcc_data->p == '#') {
                state = LUKA_GCCS_NOTE1;
                gcc_data->p++;
            }

            //单行注释
            else if (strncmp(gcc_data->p, "//", 2) == 0) {
                state = LUKA_GCCS_NOTE1;
                gcc_data->p += 2;
            }

            //多行注释
            else if (strncmp(gcc_data->p, "/*", 2) == 0) {
                state = LUKA_GCCS_NOTE2;
                gcc_data->p += 2;
            }

            //include
            else if (strncmp(gcc_data->p, "include", 7) == 0 && (*(gcc_data->p + 7) == '(' || *(gcc_data->p + 7) <= 32)) {
                gcc_data->p += 7;
                state = LUKA_GCCS_INCLUDE;
            }

            //package
            else if (strncmp(gcc_data->p, "package", 7) == 0 && (*(gcc_data->p + 7) == '(' || *(gcc_data->p + 7) <= 32)) {
                gcc_data->p += 7;
                state = LUKA_GCCS_PACKAGE;
            }

            //if
            else if (strncmp(gcc_data->p, "if", 2) == 0 && (*(gcc_data->p + 2) == '(' || *(gcc_data->p + 2) <= 32)) {
                gcc_data->p += 2;
                state = LUKA_GCCS_IF;
            }

            //else
            else if (strncmp(gcc_data->p, "else", 4) == 0 && (*(gcc_data->p + 4) == '(' || *(gcc_data->p + 4) <= 32)) {
                gcc_data->p += 4;
                state = LUKA_GCCS_ELSE;
            }

            //while
            else if (strncmp(gcc_data->p, "while", 5) == 0 && (*(gcc_data->p + 5) == '(' || *(gcc_data->p + 5) <= 32)) {
                gcc_data->p += 5;
                state = LUKA_GCCS_WHILE;
            }

            //for
            else if (strncmp(gcc_data->p, "for", 3) == 0 && (*(gcc_data->p + 3) == '(' || *(gcc_data->p + 3) <= 32)) {
                gcc_data->p += 3;
                state = LUKA_GCCS_FOR;
            }

            //function
            else if (strncmp(gcc_data->p, "function", 8) == 0 && (*(gcc_data->p + 8) == '(' || *(gcc_data->p + 8) <= 32)) {
                gcc_data->p += 8;
                state = LUKA_GCCS_FUNC;
            }

            //return
            else if (strncmp(gcc_data->p, "return", 6) == 0 && (*(gcc_data->p + 6) == ';' || *(gcc_data->p + 6) <= 32)) {
                gcc_data->p += 6;
                state = LUKA_GCCS_RETURN;
            }

            //break
            else if (strncmp(gcc_data->p, "break", 5) == 0 && (*(gcc_data->p + 5) == ';' || *(gcc_data->p + 5) <= 32)) {
                gcc_data->p += 5;
                
                while (*gcc_data->p <= 32 && *gcc_data->p != 0) {
                    if (*gcc_data->p == '\n')
                        gcc_data->line++;
                    gcc_data->p++;
                }

                if (*gcc_data->p != ';') {
                    fprintf(stderr, "can't gcc %s:%d, break error\n", gcc_data->script_name, gcc_data->line);
                    luka_exception(luka, 3);
                } else {
                    gcc_data->p++;

                    //break
                    node = luka_gcc_createnode(luka, gcc_data->script_name, gcc_data->line, LUKA_GCC_BREAK, NULL, 
                        NULL, NULL, NULL, 0, 0, 0, NULL, NULL, 0);
                    luka_gcc_push(luka, gcc, node);
                }
            }

            //continue
            else if (strncmp(gcc_data->p, "continue", 8) == 0 && (*(gcc_data->p + 8) == ';' || *(gcc_data->p + 8) <= 32)) {
                gcc_data->p += 8;
                
                while (*gcc_data->p <= 32 && *gcc_data->p != 0) {
                    if (*gcc_data->p == '\n')
                        gcc_data->line++;
                    gcc_data->p++;
                }

                if (*gcc_data->p != ';') {
                    fprintf(stderr, "can't gcc %s:%d, break error\n", gcc_data->script_name, gcc_data->line);
                    luka_exception(luka, 3);
                } else {
                    gcc_data->p++;

                    //continue
                    node = luka_gcc_createnode(luka, gcc_data->script_name, gcc_data->line, LUKA_GCC_CONTINUE, NULL, 
                        NULL, NULL, NULL, 0, 0, 0, NULL, NULL, 0);
                    luka_gcc_push(luka, gcc, node);
                }
            }

            //null express
            else if (*gcc_data->p == ';') {
                gcc_data->p++;
            }

            //}
            else if (*gcc_data->p == '}') {
                gcc_data->p++;

                //end
                node = luka_gcc_createnode(luka, gcc_data->script_name, gcc_data->line, LUKA_GCC_END, NULL, 
                    NULL, NULL, NULL, 0, 0, 0, NULL, NULL, 0);
                luka_gcc_push(luka, gcc, node);
            }

            //express
            else {
                gcc_data->express_left = gcc_data->p;
                gcc_data->express_right = luka_strchr(gcc_data->p, ';');
                if (!gcc_data->express_right) {
                    fprintf(stderr, "can't gcc %s:%d '%.12s...'\n", gcc_data->script_name, gcc_data->line, gcc_data->p);
                    luka_exception(luka, 3);
                }
                
                gcc_data->p = gcc_data->express_right + 1;
                gcc_data->express_str = luka_strsub(luka, gcc_data->express_left, gcc_data->express_right - 1);
                luka_linecheck(gcc_data->express_str, &gcc_data->line);
                _trim(gcc_data->express_str);
                gcc_data->express_left = gcc_data->express_right = NULL;
                state = LUKA_GCCS_EXPRESS;
            }
        }

        //单行注释
        else if (state == LUKA_GCCS_NOTE1) {
            if (*gcc_data->p == '\0') 
                state = LUKA_GCCS_NULL;
            if (*gcc_data->p == '\n') {
                gcc_data->line++;
                state = LUKA_GCCS_NULL;
            }
            gcc_data->p++;
        }

        //多行注释
        else if (state == LUKA_GCCS_NOTE2) {
            if (*gcc_data->p == '\0')
                state = LUKA_GCCS_NULL;
            if (*gcc_data->p == '\n')
                gcc_data->line++;
            if (strncmp(gcc_data->p, "*/", 2) == 0) {
                gcc_data->p += 2;
                state = LUKA_GCCS_NULL;
            } else {
                gcc_data->p++;
            }
        }

        //include
        else if (state == LUKA_GCCS_INCLUDE) {
        	char include_path[2048] = {0};

            while (*gcc_data->p <= 32 && *gcc_data->p != 0) {
                if (*gcc_data->p == '\n')
                    gcc_data->line++;
                gcc_data->p++;
            }

            if (*gcc_data->p++ != '(') {
                fprintf(stderr, "include error '%s':%d, errcode 1\n", gcc_data->script_name, gcc_data->line);
                luka_exception(luka, 3);
            }

            gcc_data->express_left = gcc_data->p;
            gcc_data->express_right = luka_strchr(gcc_data->p, ')');
            if (!gcc_data->express_right || gcc_data->express_left == gcc_data->express_right) {
                fprintf(stderr, "include error '%s':%d, errcode 2\n", gcc_data->script_name, gcc_data->line);
                luka_exception(luka, 3);
            }
            gcc_data->p = gcc_data->express_right + 1;

            gcc_data->express_str = luka_strsub(luka, gcc_data->express_left, gcc_data->express_right - 1);
            luka_linecheck(gcc_data->express_str, &gcc_data->line);
            _trim(gcc_data->express_str);
            gcc_data->express_left = gcc_data->express_right = NULL;
            state = LUKA_GCCS_NULL;

            if (strlen(gcc_data->express_str) == 0) {
                fprintf(stderr, "include error '%s':%d, errcode 3\n", gcc_data->script_name, gcc_data->line);
                luka_exception(luka, 3);
            }

            while (*gcc_data->p <= 32 && *gcc_data->p != 0) {
                if (*gcc_data->p == '\n')
                    gcc_data->line++;
                gcc_data->p++;
            }

            if (*gcc_data->p++ != ';') {
                fprintf(stderr, "include error '%s':%d, errcode 4\n", gcc_data->script_name, gcc_data->line);
                luka_exception(luka, 3);
            }

            //include
            memset(include_path, 0, sizeof(include_path));
            sprintf(include_path, "%s%c%s", gcc_data->script_path, _get_dir_c(), gcc_data->express_str);
        	_clear_path(include_path);
            luka_free(luka, gcc_data->express_str);
            gcc_data->express_str = NULL;

            luka_gcc_decode_ex(luka, gcc, include_path);
        }

        //package
        else if (state == LUKA_GCCS_PACKAGE) {
			while (*gcc_data->p <= 32 && *gcc_data->p != 0) {
                if (*gcc_data->p == '\n')
                    gcc_data->line++;
                gcc_data->p++;
            }

            if (*gcc_data->p++ != '(') {
                fprintf(stderr, "package error '%s':%d, errcode 1\n", gcc_data->script_name, gcc_data->line);
                luka_exception(luka, 3);
            }
			gcc_data->express_left = gcc_data->p;
            gcc_data->express_right = luka_strchr(gcc_data->p, ')');
            if (!gcc_data->express_right || gcc_data->express_left == gcc_data->express_right) {
                fprintf(stderr, "package error '%s':%d, errcode 2\n", gcc_data->script_name, gcc_data->line);
                luka_exception(luka, 3);
            }
            gcc_data->p = gcc_data->express_right + 1;

            gcc_data->express_str = luka_strsub(luka, gcc_data->express_left, gcc_data->express_right - 1);
            luka_linecheck(gcc_data->express_str, &gcc_data->line);
            _trim(gcc_data->express_str);
            gcc_data->express_left = gcc_data->express_right = NULL;
            state = LUKA_GCCS_NULL;

            if (strlen(gcc_data->express_str) == 0) {
                fprintf(stderr, "package error '%s':%d, errcode 3\n", gcc_data->script_name, gcc_data->line);
                luka_exception(luka, 3);
            }

            while (*gcc_data->p <= 32 && *gcc_data->p != 0) {
                if (*gcc_data->p == '\n')
                    gcc_data->line++;
                gcc_data->p++;
            }

            if (*gcc_data->p++ != ';') {
                fprintf(stderr, "package error '%s':%d, errcode 4\n", gcc_data->script_name, gcc_data->line);
                luka_exception(luka, 3);
            }

            //package
			luka_package(luka, gcc_data->express_str);
            luka_free(luka, gcc_data->express_str);
            gcc_data->express_str = NULL;
		}

        //if
        else if (state == LUKA_GCCS_IF) {
            while (*gcc_data->p <= 32 && *gcc_data->p != 0) {
                if (*gcc_data->p == '\n')
                    gcc_data->line++;
                gcc_data->p++;
            }

            if (*gcc_data->p++ != '(') {
                fprintf(stderr, "if error '%s':%d, errcode 1\n", gcc_data->script_name, gcc_data->line);
                luka_exception(luka, 3);
            }

            gcc_data->express_left = gcc_data->p;
            gcc_data->express_right = luka_strchr(gcc_data->p, ')');
            if (!gcc_data->express_right || gcc_data->express_left == gcc_data->express_right) {
                fprintf(stderr, "if error '%s':%d, errcode 2\n", gcc_data->script_name, gcc_data->line);
                luka_exception(luka, 3);
            }
            gcc_data->p = gcc_data->express_right + 1;

            gcc_data->express_str = luka_strsub(luka, gcc_data->express_left, gcc_data->express_right - 1);
            luka_linecheck(gcc_data->express_str, &gcc_data->line);
            _trim(gcc_data->express_str);
            gcc_data->express_left = gcc_data->express_right = NULL;
            state = LUKA_GCCS_NULL;

            if (strlen(gcc_data->express_str) == 0) {
                fprintf(stderr, "if error '%s':%d, errcode 3\n", gcc_data->script_name, gcc_data->line);
                luka_exception(luka, 3);
            }

            while (*gcc_data->p <= 32 && *gcc_data->p != 0) {
                if (*gcc_data->p == '\n')
                    gcc_data->line++;
                gcc_data->p++;
            }

            if (*gcc_data->p++ != '{') {
                fprintf(stderr, "if error '%s':%d, errcode 4\n", gcc_data->script_name, gcc_data->line);
                luka_exception(luka, 3);
            }

            //if
            node = luka_gcc_createnode(luka, gcc_data->script_name, gcc_data->line, LUKA_GCC_IF, gcc_data->express_str, 
                NULL, NULL, NULL, 0, 0, 0, NULL, NULL, 0);
            luka_gcc_push(luka, gcc, node);
            gcc_data->express_str = NULL;
            state = LUKA_GCCS_NULL;
        }

        else if (state == LUKA_GCCS_ELSE) {
            while (*gcc_data->p <= 32 && *gcc_data->p != 0) {
                if (*gcc_data->p == '\n')
                    gcc_data->line++;
                gcc_data->p++;
            }

            //else 
            if (*gcc_data->p == '{') {
                gcc_data->p++;

                node = luka_gcc_createnode(luka, gcc_data->script_name, gcc_data->line, LUKA_GCC_ELSE, NULL, 
                    NULL, NULL, NULL, 0, 0, 0, NULL, NULL, 0);
                luka_gcc_push(luka, gcc, node);
                state = LUKA_GCCS_NULL;
            }

            //else if
            else if (strncmp(gcc_data->p, "if", 2) == 0) {
                gcc_data->p += 2;

                while (*gcc_data->p <= 32 && *gcc_data->p != 0) {
                    if (*gcc_data->p == '\n')
                        gcc_data->line++;
                    gcc_data->p++;
                }

                if (*gcc_data->p++ != '(') {
                    fprintf(stderr, "else if error '%s':%d, errcode 1\n", gcc_data->script_name, gcc_data->line);
                    luka_exception(luka, 3);
                }

                gcc_data->express_left = gcc_data->p;
                gcc_data->express_right = luka_strchr(gcc_data->p, ')');
                if (!gcc_data->express_right || gcc_data->express_left == gcc_data->express_right) {
                    fprintf(stderr, "else if error '%s':%d, errcode 2\n", gcc_data->script_name, gcc_data->line);
                    luka_exception(luka, 3);
                }
                gcc_data->p = gcc_data->express_right + 1;

                gcc_data->express_str = luka_strsub(luka, gcc_data->express_left, gcc_data->express_right - 1);
                luka_linecheck(gcc_data->express_str, &gcc_data->line);
                _trim(gcc_data->express_str);
                gcc_data->express_left = gcc_data->express_right = NULL;
                state = LUKA_GCCS_NULL;

                if (strlen(gcc_data->express_str) == 0) {
                    fprintf(stderr, "else if error '%s':%d, errcode 3\n", gcc_data->script_name, gcc_data->line);
                    luka_exception(luka, 3);
                }

                while (*gcc_data->p <= 32 && *gcc_data->p != 0) {
                    if (*gcc_data->p == '\n')
                        gcc_data->line++;
                    gcc_data->p++;
                }

                if (*gcc_data->p++ != '{') {
                    fprintf(stderr, "else if error '%s':%d, errcode 4\n", gcc_data->script_name, gcc_data->line);
                    luka_exception(luka, 3);
                }

                //else if
                node = luka_gcc_createnode(luka, gcc_data->script_name, gcc_data->line, LUKA_GCC_ELSEIF, gcc_data->express_str, 
                    NULL, NULL, NULL, 0, 0, 0, NULL, NULL, 0);
                luka_gcc_push(luka, gcc, node);
                gcc_data->express_str = NULL;
                state = LUKA_GCCS_NULL;
            }

            else {
                fprintf(stderr, "else error '%s':%d, errcode 3\n", gcc_data->script_name, gcc_data->line);
                luka_exception(luka, 3);
            }
        }

        //while
        else if (state == LUKA_GCCS_WHILE) {
            while (*gcc_data->p <= 32 && *gcc_data->p != 0) {
                if (*gcc_data->p == '\n')
                    gcc_data->line++;
                gcc_data->p++;
            }

            if (*gcc_data->p++ != '(') {
                fprintf(stderr, "while error '%s':%d, errcode 1\n", gcc_data->script_name, gcc_data->line);
                luka_exception(luka, 3);
            }

            gcc_data->express_left = gcc_data->p;
            gcc_data->express_right = luka_strchr(gcc_data->p, ')');
            if (!gcc_data->express_right || gcc_data->express_left == gcc_data->express_right) {
                fprintf(stderr, "while error '%s':%d, errcode 2\n", gcc_data->script_name, gcc_data->line);
                luka_exception(luka, 3);
            }
            gcc_data->p = gcc_data->express_right + 1;

            gcc_data->express_str = luka_strsub(luka, gcc_data->express_left, gcc_data->express_right - 1);
            luka_linecheck(gcc_data->express_str, &gcc_data->line);
            _trim(gcc_data->express_str);
            gcc_data->express_left = gcc_data->express_right = NULL;
            state = LUKA_GCCS_NULL;

            if (strlen(gcc_data->express_str) == 0) {
                fprintf(stderr, "while error '%s':%d, errcode 3\n", gcc_data->script_name, gcc_data->line);
                luka_exception(luka, 3);
            }

            while (*gcc_data->p <= 32 && *gcc_data->p != 0) {
                if (*gcc_data->p == '\n')
                    gcc_data->line++;
                gcc_data->p++;
            }

            if (*gcc_data->p++ != '{') {
                fprintf(stderr, "while error '%s':%d, errcode 4\n", gcc_data->script_name, gcc_data->line);
                luka_exception(luka, 3);
            }

            //while
            node = luka_gcc_createnode(luka, gcc_data->script_name, gcc_data->line, LUKA_GCC_WHILE, gcc_data->express_str, 
                NULL, NULL, NULL, 0, 0, 0, NULL, NULL, 0);
            luka_gcc_push(luka, gcc, node);
            gcc_data->express_str = NULL;
            state = LUKA_GCCS_NULL;
        }

        //for
        else if (state == LUKA_GCCS_FOR) {
            char **for_ret = NULL;
            size_t for_len = 0;
            char **a = NULL, **b = NULL, **c = NULL;
            size_t a_len = 0, b_len = 0, c_len = 0;

            while (*gcc_data->p <= 32 && *gcc_data->p != 0) {
                if (*gcc_data->p == '\n')
                    gcc_data->line++;
                gcc_data->p++;
            }

            if (*gcc_data->p++ != '(') {
                fprintf(stderr, "for error '%s':%d, errcode 1\n", gcc_data->script_name, gcc_data->line);
                luka_exception(luka, 3);
            }

            gcc_data->express_left = gcc_data->p;
            gcc_data->express_right = luka_strchr(gcc_data->p, ')');
            if (!gcc_data->express_right || gcc_data->express_left == gcc_data->express_right) {
                fprintf(stderr, "for error '%s':%d, errcode 2\n", gcc_data->script_name, gcc_data->line);
                luka_exception(luka, 3);
            }
            gcc_data->p = gcc_data->express_right + 1;

            gcc_data->express_str = luka_strsub(luka, gcc_data->express_left, gcc_data->express_right - 1);
            luka_linecheck(gcc_data->express_str, &gcc_data->line);
            _trim(gcc_data->express_str);
            gcc_data->express_left = gcc_data->express_right = NULL;
            state = LUKA_GCCS_NULL;

            if (strlen(gcc_data->express_str) == 0) {
                fprintf(stderr, "for error '%s':%d, errcode 3\n", gcc_data->script_name, gcc_data->line);
                luka_exception(luka, 3);
            }

            while (*gcc_data->p <= 32 && *gcc_data->p != 0) {
                if (*gcc_data->p == '\n')
                    gcc_data->line++;
                gcc_data->p++;
            }

            if (*gcc_data->p++ != '{') {
                fprintf(stderr, "for error '%s':%d, errcode 4\n", gcc_data->script_name, gcc_data->line);
                luka_exception(luka, 3);
            }

            for_ret = luka_splite(luka, gcc_data->express_str, ';', &for_len);
            if (for_len != 3) {
                fprintf(stderr, "for error '%s':%d, errcode 5\n", gcc_data->script_name, gcc_data->line);
                luka_exception(luka, 3);
            }
            luka_free(luka, gcc_data->express_str);
            gcc_data->express_str = NULL;
            state = LUKA_GCCS_NULL;

            //for参数
            if (strlen(for_ret[0]) == 0) {
                a = NULL;
                a_len = 0;
            } else {
                a = luka_splite(luka, for_ret[0], ',', &a_len);
            }

            if (strlen(for_ret[1]) == 0) {
                b = NULL;
                b_len = 0;
            } else {
                b = luka_splite(luka, for_ret[1], ',', &b_len);
            }

            if (strlen(for_ret[2]) == 0) {
                c = NULL;
                c_len = 0;
            } else {
                c = luka_splite(luka, for_ret[2], ',', &c_len);
            }

            if (a)
            {
                for (i = 0; i < a_len; i++) {
                    if (strlen(a[i]) == 0) {
                        fprintf(stderr, "for error '%s':%d, errcode 7\n", gcc_data->script_name, gcc_data->line);
                        luka_exception(luka, 3);
                    }
                }
            }

            if (b)
            {
                for (i = 0; i < b_len; i++) {
                    if (strlen(b[i]) == 0) {
                        fprintf(stderr, "for error '%s':%d, errcode 8\n", gcc_data->script_name, gcc_data->line);
                        luka_exception(luka, 3);
                    }
                }
            }

            if (c)
            {
                for (i = 0; i < c_len; i++) {
                    if (strlen(c[i]) == 0) {
                        fprintf(stderr, "for error '%s':%d, errcode 9\n", gcc_data->script_name, gcc_data->line);
                        luka_exception(luka, 3);
                    }
                }
            }

            node = luka_gcc_createnode(luka, gcc_data->script_name, gcc_data->line, LUKA_GCC_FOR, NULL, 
                a, b, c, a_len, b_len, c_len, NULL, NULL, 0);
            luka_gcc_push(luka, gcc, node);
        }

        //function
        else if (state == LUKA_GCCS_FUNC) {
            char *func_name = NULL;
            char **func_param = NULL;
            size_t i = 0, func_len = 0;

            while (*gcc_data->p <= 32 && *gcc_data->p != 0) {
                if (*gcc_data->p == '\n')
                    gcc_data->line++;
                gcc_data->p++;
            }

            gcc_data->express_left = gcc_data->p;
            gcc_data->express_right = luka_strchr(gcc_data->p, '(');
            if (!gcc_data->express_right || gcc_data->express_left == gcc_data->express_right) {
                fprintf(stderr, "function error '%s':%d, errcode 1\n", gcc_data->script_name, gcc_data->line);
                luka_exception(luka, 3);
            }
            gcc_data->p = gcc_data->express_right + 1;

            gcc_data->express_str = luka_strsub(luka, gcc_data->express_left, gcc_data->express_right - 1);
            luka_linecheck(gcc_data->express_str, &gcc_data->line);
            _trim(gcc_data->express_str);
            gcc_data->express_left = gcc_data->express_right = NULL;

            //func_name
            func_name = gcc_data->express_str;
            gcc_data->express_str = NULL;

            gcc_data->express_left = gcc_data->p;
            gcc_data->express_right = luka_strchr(gcc_data->p, ')');
            if (!gcc_data->express_right) {
                fprintf(stderr, "function error '%s':%d, errcode 2\n", gcc_data->script_name, gcc_data->line);
                luka_exception(luka, 3);
            }
            gcc_data->p = gcc_data->express_right + 1;

            //无参数
            if (gcc_data->express_left != gcc_data->express_right) {
                gcc_data->express_str = luka_strsub(luka, gcc_data->express_left, gcc_data->express_right - 1);
                luka_linecheck(gcc_data->express_str, &gcc_data->line);
                _trim(gcc_data->express_str);
                gcc_data->express_left = gcc_data->express_right = NULL;

                if (strlen(gcc_data->express_str) != 0) {
                    func_param = luka_splite(luka, gcc_data->express_str, ',', &func_len);
                    for (i = 0; i < func_len; i++) {
                        if (!_isname(func_param[i])) {
                            fprintf(stderr, "function error '%s':%d, errcode 3\n", gcc_data->script_name, gcc_data->line);
                            luka_exception(luka, 3);
                        }
                    }
                }

                luka_free(luka, gcc_data->express_str);
                gcc_data->express_str = NULL;
            }

            while (*gcc_data->p <= 32 && *gcc_data->p != 0) {
                if (*gcc_data->p == '\n')
                    gcc_data->line++;
                gcc_data->p++;
            }

            if (*gcc_data->p++ != '{') {
                fprintf(stderr, "function error '%s':%d, errcode 4\n", gcc_data->script_name, gcc_data->line);
                luka_exception(luka, 3);
            }

            node = luka_gcc_createnode(luka, gcc_data->script_name, gcc_data->line, LUKA_GCC_FUNC, NULL, 
                NULL, NULL, NULL, 0, 0, 0, func_name, func_param, func_len);
            luka_gcc_push(luka, gcc, node);
            gcc_data->express_str = NULL;
            state = LUKA_GCCS_NULL;
        }

        //return
        else if (state == LUKA_GCCS_RETURN) {
            while (*gcc_data->p <= 32 && *gcc_data->p != 0) {
                if (*gcc_data->p == '\n')
                    gcc_data->line++;
                gcc_data->p++;
            }

            if (*gcc_data->p == ';') {
                node = luka_gcc_createnode(luka, gcc_data->script_name, gcc_data->line, LUKA_GCC_RETURN, luka_strdup(luka, "null"), 
                    NULL, NULL, NULL, 0, 0, 0, NULL, NULL, 0);
                luka_gcc_push(luka, gcc, node);

                gcc_data->p++;
                gcc_data->express_str = NULL;
                state = LUKA_GCCS_NULL;
            } else {
                gcc_data->express_left = gcc_data->p;
                gcc_data->express_right = luka_strchr(gcc_data->p, ';');
                if (!gcc_data->express_right) {
                    fprintf(stderr, "return error '%s':%d, errcode 1\n", gcc_data->script_name, gcc_data->line);
                    luka_exception(luka, 3);
                }
                gcc_data->p = gcc_data->express_right + 1;

                gcc_data->express_str = luka_strsub(luka, gcc_data->express_left, gcc_data->express_right - 1);
                luka_linecheck(gcc_data->express_str, &gcc_data->line);
                _trim(gcc_data->express_str);
                gcc_data->express_left = gcc_data->express_right = NULL;

                node = luka_gcc_createnode(luka, gcc_data->script_name, gcc_data->line, LUKA_GCC_RETURN, gcc_data->express_str, 
                    NULL, NULL, NULL, 0, 0, 0, NULL, NULL, 0);
                luka_gcc_push(luka, gcc, node);
                gcc_data->express_str = NULL;
                state = LUKA_GCCS_NULL;
            }
        }

        //表达式
        else if (state == LUKA_GCCS_EXPRESS) {
            node = luka_gcc_createnode(luka, gcc_data->script_name, gcc_data->line, LUKA_GCC_EXPRESS, gcc_data->express_str, 
                NULL, NULL, NULL, 0, 0, 0, NULL, NULL, 0);
            luka_gcc_push(luka, gcc, node);
            gcc_data->express_str = NULL;
            state = LUKA_GCCS_NULL;
        }

        else {
            fprintf(stderr, "can't gcc %s:%d '%.12s...', errcode %d\n", gcc_data->script_name, gcc_data->line, gcc_data->p, state);
            luka_exception(luka, 3);
        }
    }

    if (state != LUKA_GCCS_NULL) {
        fprintf(stderr, "'%s' not end\n", gcc_data->script_name);
        luka_exception(luka, 3);
    }
}


/************************************************************************************/

static void luka_express_destroy (Luka *luka, LukaExpress *express);
static void luka_express_node_destroy (Luka *luka, LukaExpressNode *node);
static void luka_express_node_destroy2 (Luka *luka, LukaExpressNode *node);

/** 出列 **/
static LukaExpressNode *luka_express_queue_pop (LukaExpress *express) {
    LukaExpressNode *node = NULL;

    if (!express->queue)
        return NULL;

    node = express->queue;
    express->queue = node->next;
    if (!express->queue)
        express->queue_tail = NULL;
    return node;
}

/** 入表达式 **/
static void luka_express_RPN_push (LukaExpress *express, LukaExpressNode *buf) {
    if (express->RPN == NULL) {
        express->RPN = express->RPN_tail = buf;
    } else {
        express->RPN_tail->next = buf;
        buf->last = express->RPN_tail;
        express->RPN_tail = buf;
    }
}

/** 从表达式查找一个操作符 **/
static LukaExpressNode *luka_express_RPN_find (LukaExpress *express) {
    LukaExpressNode *mov = express->RPN;
    while (mov) {
        if (mov->type == LUKA_EXP_OPER)
            return mov;
        mov = mov->next;
    }
    return NULL;
}

/** 从表达式删除一个节点 **/
static void luka_express_RPN_rmv (Luka *luka, LukaExpress *express, LukaExpressNode *node) {
    LukaExpressNode *left = node->last, *right = node->next;

    if (!left && right) {
        express->RPN = express->RPN->next;
        if (!express->RPN)
            express->RPN_tail = NULL;
    } else if (left && right) {
        left->next = right;
        right->last = left;
    } else if (left && !right) {
        left->next = NULL;
        express->RPN_tail = left;
    }
    luka_express_node_destroy(luka, node);
}

/** 节点替换 **/
static void luka_express_RPN_update (Luka *luka, LukaExpress *express, LukaExpressNode *node, voidp data) {
    luka_express_node_destroy2(luka, node);
    node->type = LUKA_EXP_DATA;
    node->data = data;
}

/** 入栈 **/
static void luka_express_stack_push (LukaExpress *express, LukaExpressNode *buf) {
    buf->next = express->stack;
    express->stack = buf;
}

/** 栈顶 **/
static LukaExpressNode *luka_express_stack_top (LukaExpress *express) {
    return express->stack;
}

/** 出栈 **/
static LukaExpressNode *luka_express_stack_pop (LukaExpress *express) {
    LukaExpressNode *node = express->stack;
    express->stack = node->next;
    return node;
}

/** 转逆波兰表达式 **/
static int luka_express_reset (Luka *luka, LukaExpress *express) {
    LukaExpressNode *buf = NULL, *buf2 = NULL;

    if (!express->queue) {
        luka_express_destroy(luka, express);
        return 0;
    }

    while ((buf = luka_express_queue_pop(express)) != NULL) {
        if (buf->type == LUKA_EXP_OPER) {
            // '('
            if (buf->oper == LUKA_OPER_SMA_BRACL) {
                luka_express_stack_push(express, buf);
            }

            // ')'
            else if (buf->oper == LUKA_OPER_SMA_BRACR) {
                while ((buf2 = luka_express_stack_top(express)) && buf2->oper != LUKA_OPER_SMA_BRACL) {
                    buf2 = luka_express_stack_pop(express);
                    luka_express_RPN_push(express, buf2);
                }

                if (!buf2) {
                    luka_express_node_destroy(luka, buf);
                    luka_express_destroy(luka, express);
                    return 0;
                }
                buf2 = luka_express_stack_pop(express);
                luka_express_node_destroy(luka, buf2);
            }

            //其他运算符
            else {
                while ((buf2 = luka_express_stack_top(express)) && buf2->oper <= buf->oper && buf2->oper != LUKA_OPER_SMA_BRACL) {
                    buf2 = luka_express_stack_pop(express);
                    luka_express_RPN_push(express, buf2);
                }
                luka_express_stack_push(express, buf);
            }
        }
        
        //操作数
        else {
            luka_express_RPN_push(express, buf);
        }
    }

    //清空栈
    while ((buf2 = luka_express_stack_top(express)) != NULL) {
        if (buf2->oper == LUKA_OPER_SMA_BRACL) {
            luka_express_destroy(luka, express);
            return 0;
        }

        buf2 = luka_express_stack_pop(express);
        luka_express_RPN_push(express, buf2);
    }
    return 1;
}

/** 复制一份逆波兰表达式 **/
static LukaExpress *luka_express_copy (Luka *luka, LukaExpress *express) {
    LukaExpress *express_cp = NULL;
    LukaExpressNode *mov = NULL, *buf = NULL;

    express_cp = luka_alloc(luka, sizeof(LukaExpress));
    mov = express->RPN;
    while (mov) {
        buf = luka_alloc(luka, sizeof(LukaExpressNode));
        buf->type = mov->type;
        buf->last = buf->next = NULL;

        if (mov->type == LUKA_EXP_DATA) {
            buf->data = mov->data;
        } else if (mov->type == LUKA_EXP_VAR) {
            buf->var_name = luka_strdup(luka, mov->var_name);
        } else if (mov->type == LUKA_EXP_OPER) {
            buf->oper = mov->oper;
        } else if (mov->type == LUKA_EXP_FUNC) {
            buf->func_name = luka_strdup(luka, mov->func_name);
            if (mov->func_param) {
                size_t i = 0;
                buf->func_len = mov->func_len;
                buf->func_param = (LukaExpress **)luka_alloc(luka, sizeof(LukaExpress*) * mov->func_len);
                for (i = 0; i < mov->func_len; i++) {
                    buf->func_param[i] = luka_express_copy(luka, mov->func_param[i]);
                }
            }
        } else if (mov->type == LUKA_EXP_OBJECT) {
            buf->data = mov->data;
            buf->obj_name = luka_strdup(luka, mov->obj_name);
        } else if (mov->type == LUKA_EXP_ARRAY) {
            buf->data = mov->data;
            buf->arr_index = mov->arr_index;
        }

        luka_express_RPN_push(express_cp, buf);
        mov = mov->next;
    }
    return express_cp;
}

/** 测试运行表达式 **/
static int luka_express_test (Luka *luka, LukaExpress *express) {
    size_t i = 0;
    LukaExpress *express_cp = NULL;
    LukaExpressNode *oper = NULL, *left1 = NULL, *left2 = NULL;
    
    express_cp = luka_express_copy(luka, express);

    while ((oper = luka_express_RPN_find(express_cp)) != NULL) {
        left1 = oper->last;
        if (left1) left2 = left1->last;

        if (!left1)
            return 0;

        if (!left2) {
            //只有LUKA_OPER_ARRAY_NOP可以是单目
            if (oper->oper != LUKA_OPER_ARRAY_NOP) {
                return 0;
            }
        }

        //a[0]
        if (oper->oper == LUKA_OPER_ARRAY) {
            left2->type = LUKA_EXP_VAR;
            luka_express_RPN_rmv(luka, express_cp, left1);
            luka_express_RPN_rmv(luka, express_cp, oper);
        }

        //a[] = 1
        else if (oper->oper == LUKA_OPER_ARRAY_NOP) {
            left1->type = LUKA_EXP_VAR;
            luka_express_RPN_rmv(luka, express_cp, oper);
        }

        //a->b = 2
        else if (oper->oper == LUKA_OPER_OBJ) {
            left2->type = LUKA_EXP_VAR;
            luka_express_RPN_rmv(luka, express_cp, left1);
            luka_express_RPN_rmv(luka, express_cp, oper);
        }

        else if (oper->oper == LUKA_OPER_EQU || oper->oper == LUKA_OPER_DIVIDEQU || oper->oper == LUKA_OPER_TIMESEQU || 
            oper->oper == LUKA_OPER_REMAINEQU || oper->oper == LUKA_OPER_PLUSEQU || oper->oper == LUKA_OPER_MINUSEQU) {
            if (left2->type != LUKA_EXP_VAR) return 0;
            luka_express_RPN_rmv(luka, express_cp, left1);
            luka_express_RPN_rmv(luka, express_cp, oper);
        }

        else {
            if (left1->type == LUKA_EXP_FUNC && left1->func_param) {
                for (i = 0; i < left1->func_len; i++) {
                    if (!luka_express_test(luka, left1->func_param[i])) return 0;
                }
            }

            if (left2->type == LUKA_EXP_FUNC && left2->func_param) {
                for (i = 0; i < left2->func_len; i++) {
                    if (!luka_express_test(luka, left2->func_param[i])) return 0;
                }
            }

            luka_express_RPN_rmv(luka, express_cp, left1);
            luka_express_RPN_rmv(luka, express_cp, oper);
        }
    }

    if (!express_cp->RPN || express_cp->RPN->next) {
        return 0;
    }

    luka_express_destroy(luka, express_cp);
    return 1;
}

static void luka_express_node_destroy (Luka *luka, LukaExpressNode *node) {
    luka_express_node_destroy2(luka, node);
    luka_free(luka, node);
}

static void luka_express_node_destroy2 (Luka *luka, LukaExpressNode *node) {
    if (node->type == LUKA_EXP_VAR) {
        if (node->var_name) luka_free(luka, node->var_name);
    } else if (node->type == LUKA_EXP_FUNC) {
        size_t i = 0;
        for (i = 0; i < node->func_len; i++) {
            luka_express_destroy(luka, node->func_param[i]);
        }
        if (node->func_param)
            luka_free(luka, node->func_param);
        luka_free(luka, node->func_name);
    } else if (node->type == LUKA_EXP_OBJECT) {
        luka_free(luka, node->obj_name);
    }
}

static void luka_express_destroy (Luka *luka, LukaExpress *express) {
    LukaExpressNode *mov = NULL, *buf = NULL;

    mov = express->queue;
    while (mov) {
        buf = mov;
        mov = mov->next;
        luka_express_node_destroy(luka, buf);
    }

    mov = express->RPN;
    while (mov) {
        buf = mov;
        mov = mov->next;
        luka_express_node_destroy(luka, buf);
    }

    mov = express->stack;
    while (mov) {
        buf = mov;
        mov = mov->next;
        luka_express_node_destroy(luka, buf);
    }

    luka_free(luka, express);
}

/************************************************************************************/

static char *g_Oper[] = {
    "->", "]", "[]", "(", ")", "/", "*", "%", "+", "-", ">=", ">", "<=", "<", "==", "!=", "&&", "||", "=", "/=", "*=", "%=" "+=", "-=", ""
};

static int luka_express_end (const char *s) {
    size_t i = 1;

    if (*s <= 32)
        return 1;

    for (i = LUKA_OPER_OBJ; i < LUKA_OPER_NULL; i++) {
        if (i == LUKA_OPER_ARRAY) continue;
        if (strlen(g_Oper[i]) != 0 && strncmp(g_Oper[i], s, strlen(g_Oper[i])) == 0)
            return 1;
    }
    return 0;
}

static void luka_express_add (Luka *luka, LukaExpress *express, LukaExpressNode *node) {
    if (!express->queue) {
        express->queue = express->queue_tail = node;
    } else {
        express->queue_tail->next = node;
        node->last = express->queue_tail;
        express->queue_tail = node;
    }
}

static void luka_express_add_null (Luka *luka, LukaExpress *express) {
    LukaExpressNode *node = (LukaExpressNode *)luka_alloc(luka, sizeof(LukaExpressNode));
    node->type = LUKA_EXP_DATA;
    node->data = luka_null(luka);
    luka_express_add(luka, express, node);
}

static void luka_express_add_true (Luka *luka, LukaExpress *express) {
    LukaExpressNode *node = (LukaExpressNode *)luka_alloc(luka, sizeof(LukaExpressNode));
    node->type = LUKA_EXP_DATA;
    node->data = luka_true(luka);
    luka_express_add(luka, express, node);
}

static void luka_express_add_false (Luka *luka, LukaExpress *express) {
    LukaExpressNode *node = (LukaExpressNode *)luka_alloc(luka, sizeof(LukaExpressNode));
    node->type = LUKA_EXP_DATA;
    node->data = luka_false(luka);
    luka_express_add(luka, express, node);
}

static void luka_express_add_int (Luka *luka, LukaExpress *express, int i) {
    LukaExpressNode *node = (LukaExpressNode *)luka_alloc(luka, sizeof(LukaExpressNode));
    node->type = LUKA_EXP_DATA;
    node->data = luka_put_int(luka, i);
    luka_data_up(luka, node->data);
    luka_express_add(luka, express, node);
}

static void luka_express_add_double (Luka *luka, LukaExpress *express, double d) {
    LukaExpressNode *node = (LukaExpressNode *)luka_alloc(luka, sizeof(LukaExpressNode));
    node->type = LUKA_EXP_DATA;
    node->data = luka_put_double(luka, d);
    luka_data_up(luka, node->data);
    luka_express_add(luka, express, node);
}

static void luka_express_add_string (Luka *luka, LukaExpress *express, char *str) {
    LukaExpressNode *node = (LukaExpressNode *)luka_alloc(luka, sizeof(LukaExpressNode));
    node->type = LUKA_EXP_DATA;
    node->data = luka_put_string(luka, str);
    luka_data_up(luka, node->data);
    luka_express_add(luka, express, node);
}

static void luka_express_add_var (Luka *luka, LukaExpress *express, char *var) {
    LukaExpressNode *node = (LukaExpressNode *)luka_alloc(luka, sizeof(LukaExpressNode));
    node->type = LUKA_EXP_VAR;
    node->var_name = var;
    luka_express_add(luka, express, node);
}

static void luka_express_add_oper (Luka *luka, LukaExpress *express, LukaOper oper) {
    LukaExpressNode *node = (LukaExpressNode *)luka_alloc(luka, sizeof(LukaExpressNode));
    node->type = LUKA_EXP_OPER;
    node->oper = oper;
    luka_express_add(luka, express, node);
}

static void luka_express_add_func (Luka *luka, LukaExpress *express, char *func_name, LukaExpress **func_param, size_t func_len) {
    LukaExpressNode *node = (LukaExpressNode *)luka_alloc(luka, sizeof(LukaExpressNode));
    node->type = LUKA_EXP_FUNC;
    node->func_name = func_name;
    node->func_param = func_param;
    node->func_len = func_len;
    luka_express_add(luka, express, node);
}

static int luka_express_int    (const char **s, int *i);
static int luka_express_double (const char **s, double *d);
static int luka_express_string (Luka *luka, const char **s, char **str);
static int luka_express_var    (Luka *luka, const char **s, char **var_name);
static int luka_express_oper   (Luka *luka, const char **s, LukaOper *oper);
static int luka_express_func   (Luka *luka, LukaGCC *gcc, const char **s, char **func_name, 
                                LukaExpress ***func_param, size_t *func_len);

/** 解析表达式 **/
LukaExpress *luka_express_decode (Luka *luka, LukaGCC *gcc) {
    LukaExpress *express = NULL;
    LukaExpressNode *mov = NULL, *buf = NULL;
    const char *s = gcc->express;

    //value
    int       i   = 0;
    double    d   = 0;
    char     *str = NULL;
    char     *var = NULL;
    LukaOper oper = LUKA_OPER_NULL;

    //func
    char *func_name = NULL;
    LukaExpress **func_param = NULL;
    size_t func_len = 0;

    //是否在数组选择器中
    size_t isarray = 0;

    express = (LukaExpress *)luka_alloc(luka, sizeof(LukaExpress));

    while (*s != 0) {
        if (*s <= 32) {
            s++;
        }

        else if (strncmp(s, "null", 4) == 0 && luka_express_end(s + 4)) {
            luka_express_add_null(luka, express);
            s += 4;
        }

        else if (strncmp(s, "true", 4) == 0 && luka_express_end(s + 4)) {
            luka_express_add_true(luka, express);
            s += 4;
        }

        else if (strncmp(s, "false", 5) == 0 && luka_express_end(s + 5)) {
            luka_express_add_false(luka, express);
            s += 5;
        }

        else if (luka_express_oper(luka, &s, &oper)) {
            luka_express_add_oper(luka, express, oper);
        }

        else if (luka_express_int(&s, &i)) {
            luka_express_add_int(luka, express, i);
        }

        else if (luka_express_double(&s, &d)) {
            luka_express_add_double(luka, express, d);
        }

        else if (luka_express_string(luka, &s, &str)) {
            luka_express_add_string(luka, express, str);
        }

        else if (luka_express_var(luka, &s, &var)) {
            if (!_isname(var)) {
                fprintf(stderr, "express error '%s':%d ==> can't use '%s'\n", gcc->script_name, gcc->line, var);
                luka_exception(luka, 3);
            }
            luka_express_add_var(luka, express, var);
        }

        else if (isarray == 0 && *s == '[') {
            isarray = 1;
            luka_express_add_oper(luka, express, LUKA_OPER_SMA_BRACL);
            s++;
        }

        else if (isarray == 1 && *s == ']') {
            isarray = 0;
            luka_express_add_oper(luka, express, LUKA_OPER_SMA_BRACR);
            luka_express_add_oper(luka, express, LUKA_OPER_ARRAY);
            s++;
        }

        else if (luka_express_func(luka, gcc, &s, &func_name, &func_param, &func_len)) {
            luka_express_add_func(luka, express, func_name, func_param, func_len);
        }

        else {
            fprintf(stderr, "express error '%s':%d ==> '%s', errcode 1\n", gcc->script_name, gcc->line, gcc->express);
            luka_exception(luka, 3);
        }
    }

    //负号处理()

    //转逆波兰表达式
    if (!luka_express_reset(luka, express)) {
        fprintf(stderr, "express error '%s':%d ==> '%s', errcode 2\n", gcc->script_name, gcc->line, gcc->express);
        luka_exception(luka, 3);
    }

    //测试表达式
    if (!luka_express_test(luka, express)) {
        fprintf(stderr, "express error '%s':%d ==> '%s', errcode 3\n", gcc->script_name, gcc->line, gcc->express);
        luka_exception(luka, 3);
    }
    return express;
}

//int
static int luka_express_int (const char **s, int *i) {
    const char *p = *s;

    if (*p == '-')
        p++;

    if (*p < '0' || *p > '9')
        return 0;

    while (*p >= '0' && *p <= '9')
        p++;

    if (*p == '.')
        return 0;

    if (luka_express_end(p) == 0 && *p != ']')
        return 0;
    
    *i = atoi(*s);
    *s = p;
    return 1;
}

//double
static int luka_express_double (const char **s, double *d) {
    const char *p = *s;

    if (*p == '-')
        p++;

    if ((*p < '0' || *p > '9') && *p != '.')
        return 0;

    while ((*p >= '0' && *p <= '9') || *p == '.') {
        p++;
    }

    if (luka_express_end(p) == 0)
        return 0;

    *d = (double)atof(*s);
    *s = p;
    return 1;
}

/** 从s中剔除c **/
static void _str_rmv_c (char *s, char c) {
    char *p = s;
    while (*p != '\0') {
        if (*p != c) *s++ = *p;
        p++;
    }
    *s = '\0';
}

/** 处理转义 **/
static void _trans_str (char *s) {
    char *p = s;
    while (*p != '\0') {
        if (*p == '\\') {
            *p = 1;
            if (*(p + 1) == 'n') *(p + 1) = '\n';
            else if (*(p + 1) == 'r') *(p + 1) = '\r';
            else if (*(p + 1) == 't') *(p + 1) = '\t';
            p++;
        }
        p++;
    }
    _str_rmv_c(s, 1);
}

//string
static int luka_express_string (Luka *luka, const char **s, char **str) {
    const char *p = *s, *p_end = NULL;
    char c = *p;

    if (*p != '\'' && *p != '\"')
        return 0;

    p_end = p + 1;
    while (*p_end != '\0' && ((p_end = strchr(p_end, c)) != NULL) && _count_trans_num(p_end - 1) % 2 != 0) {
        p_end++;
    }

    if (!p_end || *p_end == '\0')
        return 0;

    //空字符串
    if (p + 1 == p_end) {
        *str = luka_strdup(luka, "");
    } else {
        *str = luka_strsub(luka, p + 1, p_end - 1);
        _trans_str(*str);
    }

    *s = p_end + 1;
    return 1;
}

//变量名(字母数字下划线,首字母非数字,非关键字,非已使用的函数名)
static int luka_express_var (Luka *luka, const char **s, char **var_name) {
    const char *p = *s;
    const char *right = NULL;

    if (!strchr(g_Name, *p) || strchr(g_Num, *p))
        return 0;
    right = p++;

    while (*p != 0 && strchr(g_Name, *p)) {
        right = p++;
    }

    //检查有无(
    while (*p <= 32 && *p != 0) {
        p++;
    }

    //函数调用
    if (*p == '(')
        return 0;

    *var_name = luka_strsub(luka, *s, right);
    *s = right + 1;
    return 1;
}

static char *g_Oper2[] = {
    "->", "[]", ">=", ">", "<=", "<", "==", "!=", "&&", "||", "/=", "*=", "%=", "+=", "-=", "(", ")", "/", "*", "%", "+", "-", "="
};

static int g_Oper2_Id[] = {
    LUKA_OPER_OBJ, LUKA_OPER_ARRAY_NOP, LUKA_OPER_MOREEQU, LUKA_OPER_MORE,
    LUKA_OPER_LESSEQU, LUKA_OPER_LESS, LUKA_OPER_EQU2, LUKA_OPER_NOTQEU,
    LUKA_OPER_AND, LUKA_OPER_OR, LUKA_OPER_DIVIDEQU, LUKA_OPER_TIMESEQU,
    LUKA_OPER_REMAINEQU, LUKA_OPER_PLUSEQU, LUKA_OPER_MINUSEQU, 
    LUKA_OPER_SMA_BRACL, LUKA_OPER_SMA_BRACR, LUKA_OPER_DIVIDE, LUKA_OPER_TIMES,
    LUKA_OPER_REMAINDER, LUKA_OPER_PLUS, LUKA_OPER_MINUS, LUKA_OPER_EQU, LUKA_OPER_NULL
};

//oper
static int luka_express_oper (Luka *luka, const char **s, LukaOper *oper) {
    int i = 0;
    const char *p = *s;
    while (g_Oper2_Id[i] != LUKA_OPER_NULL) {
        if (strncmp(p, g_Oper2[i], strlen(g_Oper2[i])) == 0) {
            *s = p + strlen(g_Oper2[i]);
            *oper = g_Oper2_Id[i];
            return 1;
        }
        i++;
    }
    return 0;
}

//函数调用(命名规则与变量名一致)
static int luka_express_func (Luka *luka, LukaGCC *gcc, const char **s, char **func_name, LukaExpress ***func_param, size_t *func_len) {
    const char *p = *s;
    const char *left = NULL, *right = NULL;
    char *str = NULL, **strs = NULL;
    size_t i = 0, strs_len = 0;
    LukaGCC *gcc2 = NULL;

    if (!strchr(g_Name, *p) || strchr(g_Num, *p))
        return 0;
    right = p++;

    while (*p != 0 && strchr(g_Name, *p)) {
        right = p++;
    }

    //检查有无(
    while (*p <= 32 && *p != 0) {
        p++;
    }

    //函数调用
    if (*p++ != '(')
        return 0;

    *func_name = luka_strsub(luka, *s, right);

    left = p;
    right = luka_strchr(p, ')');
    if (!right) {
        luka_free(luka, *func_name);
        *func_name = NULL;
        return 0;
    }

    if (left == right) {
        *s = right + 1;
        return 1;
    }

    p = right + 1;
    str = _trim(luka_strsub(luka, left, right - 1));
    if (strlen(str) != 0) {
        strs = luka_splite(luka, str, ',', &strs_len);
    }
    luka_free(luka, str);
    *s = p;

    *func_param = (LukaExpress **)luka_alloc(luka, sizeof(LukaExpress *) * strs_len);
    *func_len = strs_len;

    for (i = 0; i < strs_len; i++) {
        gcc2 = luka_alloc(luka, sizeof(LukaGCC));
        gcc2->type = LUKA_GCC_EXPRESS;
        gcc2->script_name = gcc->script_name;
        gcc2->line = gcc->line;
        gcc2->express = luka_strdup(luka, strs[i]);
        gcc2->next = NULL;
        if (strlen(strs[i]) == 0)
            return 0;
        (*func_param)[i] = luka_express_decode(luka, gcc2);
        luka_gcc_free(luka, gcc2);
    }
    luka_free(luka, strs);
    return 1;
}

/************************************************************************************/

static voidp luka_expressnode_exec (Luka *luka, RBTreeC *vars, LukaExpress *express, LukaExpressNode *node) {
    size_t i = 0;
    voidp dataID = luka_null(luka);

    if (node->type == LUKA_EXP_DATA) {
        dataID = node->data;
    } else if (node->type == LUKA_EXP_VAR) {
        dataID = rbtreec_get(luka, vars, node->var_name);
        dataID = dataID != NULL ? dataID : luka_null(luka);
    } else if (node->type == LUKA_EXP_FUNC) {
        voidp *func_param = NULL;
        size_t  func_len = 0;

        if (node->func_param) {
            func_len = node->func_len;
            func_param = (voidp *)luka_alloc(luka, sizeof(voidp) * func_len);
            for (i = 0; i < node->func_len; i++) {
                func_param[i] = luka_express_exec(luka, vars, node->func_param[i]);
            }
        }

        dataID = luka_call_func(luka, node->func_name, func_param, func_len);

        if (func_param) {
            for (i = 0; i < node->func_len; i++) {
                luka_data_check(luka, func_param[i]);
            }
        	luka_free(luka, func_param);
        }
    } else if (node->type == LUKA_EXP_OBJECT) {
        dataID = luka_object_get(luka, luka_get_object(luka, node->data), node->obj_name);
        dataID = dataID != NULL ? dataID : luka_null(luka);
    } else if (node->type == LUKA_EXP_ARRAY) {
        dataID = luka_array_get(luka, luka_get_array(luka, node->data), node->arr_index);
        dataID = dataID != NULL ? dataID : luka_null(luka);
    }
    return dataID;
}

/** 执行表达式(核心) **/
voidp luka_express_exec (Luka *luka, RBTreeC *vars, LukaExpress *express) {
    size_t i = 0;
    voidp dataID = luka_null(luka);

    LukaExpress *express_cp = NULL;
    LukaExpressNode *oper = NULL;
    LukaExpressNode *left1 = NULL, *left2 = NULL;
    voidp left1_p = NULL;
    voidp left2_p = NULL;
    voidp buf_p = NULL;
    voidp new_p = NULL;

    express_cp = luka_express_copy(luka, express);

    while ((oper = luka_express_RPN_find(express_cp)) != NULL) {
        left1 = oper->last;
        if (left1) left2 = left1->last;
        
        if (oper->oper != LUKA_OPER_OR)
            left1_p = luka_expressnode_exec(luka, vars, express_cp, left1);

        //->
        if (oper->oper == LUKA_OPER_OBJ) {
            left2_p = luka_expressnode_exec(luka, vars, express_cp, left2);
            if (luka_is_object(luka, left2_p) && left1->type == LUKA_EXP_VAR) {
                luka_express_node_destroy2(luka, left2);
                left2->type = LUKA_EXP_OBJECT;
                left2->data = left2_p;
                left2->obj_name = luka_strdup(luka, left1->var_name);
            } else {
                luka_express_RPN_update(luka, express_cp, left2, luka_null(luka));
            }

            luka_express_RPN_rmv(luka, express_cp, left1);
            luka_express_RPN_rmv(luka, express_cp, oper);
        }

        //[]有参
        else if (oper->oper == LUKA_OPER_ARRAY) {
            left2_p = luka_expressnode_exec(luka, vars, express_cp, left2);
            if (luka_is_array(luka, left2_p) && luka_is_int(luka, left1_p) && luka_get_int(luka, left1_p) >= 0) {
                luka_express_node_destroy2(luka, left2);
                left2->type = LUKA_EXP_ARRAY;
                left2->data = left2_p;
                left2->arr_index = luka_get_int(luka, left1_p);
            } else {
                luka_express_RPN_update(luka, express_cp, left2, luka_null(luka));
            }

            luka_express_RPN_rmv(luka, express_cp, left1);
            luka_express_RPN_rmv(luka, express_cp, oper);
        }
        
        //[]无参
        else if (oper->oper == LUKA_OPER_ARRAY_NOP) {
            left1_p = luka_expressnode_exec(luka, vars, express_cp, left1);
            if (luka_is_array(luka, left1_p)) {
                luka_express_node_destroy2(luka, left1);
                left1->type = LUKA_EXP_ARRAY;
                left1->data = left1_p;
                left1->arr_index = luka_array_length(luka, luka_get_array(luka, left1_p));
            } else {
                luka_express_RPN_update(luka, express_cp, left1, luka_null(luka));
            }

            luka_express_RPN_rmv(luka, express_cp, oper);
        }

        //'/'
        else if (oper->oper == LUKA_OPER_DIVIDE) {
            left2_p = luka_expressnode_exec(luka, vars, express_cp, left2);

            if ((luka_is_int(luka, left1_p) || luka_is_double(luka, left1_p)) && ((luka_is_int(luka, left2_p) || luka_is_double(luka, left2_p)))) {
                if (luka_is_int(luka, left1_p) && luka_is_int(luka, left2_p)) {
                	new_p = luka_put_int(luka, luka_get_int(luka, left2_p) / luka_get_int(luka, left1_p));
                } else {
                	new_p = luka_put_double(luka, luka_get_double(luka, left2_p) / luka_get_double(luka, left1_p));
                }
                luka_express_RPN_update(luka, express_cp, left2, new_p);
            } else {
                luka_express_RPN_update(luka, express_cp, left2, luka_null(luka));
            }

            luka_data_check(luka, left1_p);
            luka_data_check(luka, left2_p);
            luka_express_RPN_rmv(luka, express_cp, left1);
            luka_express_RPN_rmv(luka, express_cp, oper);
        }

        //*
        else if (oper->oper == LUKA_OPER_TIMES) {
            left2_p = luka_expressnode_exec(luka, vars, express_cp, left2);

            if ((luka_is_int(luka, left1_p) || luka_is_double(luka, left1_p)) && ((luka_is_int(luka, left2_p) || luka_is_double(luka, left2_p)))) {
                if (luka_is_int(luka, left1_p) && luka_is_int(luka, left2_p)) {
                	new_p = luka_put_int(luka, luka_get_int(luka, left2_p) * luka_get_int(luka, left1_p));
                } else {
                	new_p = luka_put_double(luka, luka_get_double(luka, left2_p) * luka_get_double(luka, left1_p));
                }
                luka_express_RPN_update(luka, express_cp, left2, new_p);
            } else {
                luka_express_RPN_update(luka, express_cp, left2, luka_null(luka));
            }
            
            luka_data_check(luka, left1_p);
            luka_data_check(luka, left2_p);
            luka_express_RPN_rmv(luka, express_cp, left1);
            luka_express_RPN_rmv(luka, express_cp, oper);
        }

        //%
        else if (oper->oper == LUKA_OPER_REMAINDER) {
            left2_p = luka_expressnode_exec(luka, vars, express_cp, left2);

            if (luka_is_int(luka, left1_p) && luka_is_int(luka, left2_p)) {
            	new_p = luka_put_int(luka, luka_get_int(luka, left2_p) % luka_get_int(luka, left1_p));
                luka_express_RPN_update(luka, express_cp, left2, new_p);
            } else {
                luka_express_RPN_update(luka, express_cp, left2, luka_null(luka));
            }
            
            luka_data_check(luka, left1_p);
            luka_data_check(luka, left2_p);
            luka_express_RPN_rmv(luka, express_cp, left1);
            luka_express_RPN_rmv(luka, express_cp, oper);
        }

        //+
        else if (oper->oper == LUKA_OPER_PLUS) {
            left2_p = luka_expressnode_exec(luka, vars, express_cp, left2);
            
            //数字相加
            if ((luka_is_int(luka, left1_p) || luka_is_double(luka, left1_p)) && ((luka_is_int(luka, left2_p) || luka_is_double(luka, left2_p)))) {
                if (luka_is_int(luka, left1_p) && luka_is_int(luka, left2_p)) {
					new_p = luka_put_int(luka, luka_get_int(luka, left2_p) + luka_get_int(luka, left1_p));
				} else {
					new_p = luka_put_double(luka, luka_get_double(luka, left2_p) + luka_get_double(luka, left1_p));
				}
				luka_express_RPN_update(luka, express_cp, left2, new_p);
            }

            //字符串相加
            else if (luka_is_string(luka, left1_p) && luka_is_string(luka, left2_p)) {
                const char *left1_s = luka_get_string(luka, left1_p);
                const char *left2_s = luka_get_string(luka, left2_p);

                char *new_str = (char *)luka_alloc(luka, strlen(left1_s) + strlen(left2_s) + 10);
                sprintf(new_str, "%s%s", left2_s, left1_s);
                new_p = luka_put_string(luka, new_str);
				luka_express_RPN_update(luka, express_cp, left2, new_p);
            }

            else {
                luka_express_RPN_update(luka, express_cp, left2, luka_null(luka));
            }
            
            luka_data_check(luka, left1_p);
            luka_data_check(luka, left2_p);
            luka_express_RPN_rmv(luka, express_cp, left1);
            luka_express_RPN_rmv(luka, express_cp, oper);
        }

        //-
        else if (oper->oper == LUKA_OPER_MINUS) {
            left2_p = luka_expressnode_exec(luka, vars, express_cp, left2);
            if ((luka_is_int(luka, left1_p) || luka_is_double(luka, left1_p)) && ((luka_is_int(luka, left2_p) || luka_is_double(luka, left2_p)))) {
                if (luka_is_int(luka, left1_p) && luka_is_int(luka, left2_p)) {
					new_p = luka_put_int(luka, luka_get_int(luka, left2_p) - luka_get_int(luka, left1_p));
				} else {
					new_p = luka_put_double(luka, luka_get_double(luka, left2_p) - luka_get_double(luka, left1_p));
				}
				luka_express_RPN_update(luka, express_cp, left2, new_p);
            } else {
                luka_express_RPN_update(luka, express_cp, left2, luka_null(luka));
            }
            
            luka_data_check(luka, left1_p);
            luka_data_check(luka, left2_p);
            luka_express_RPN_rmv(luka, express_cp, left1);
            luka_express_RPN_rmv(luka, express_cp, oper);
        }

        //>=
        else if (oper->oper == LUKA_OPER_MOREEQU) {
            left2_p = luka_expressnode_exec(luka, vars, express_cp, left2);
            if ((luka_is_int(luka, left1_p) || luka_is_double(luka, left1_p)) && ((luka_is_int(luka, left2_p) || luka_is_double(luka, left2_p)))) {
                if (luka_is_int(luka, left1_p) && luka_is_int(luka, left2_p)) {
                    if (luka_get_int(luka, left2_p) >= luka_get_int(luka, left1_p)) {
                        luka_express_RPN_update(luka, express_cp, left2, luka_true(luka));
                    } else {
                        luka_express_RPN_update(luka, express_cp, left2, luka_false(luka));
                    }
                } else {
                    if (luka_get_double(luka, left2_p) >= luka_get_double(luka, left1_p)) {
                        luka_express_RPN_update(luka, express_cp, left2, luka_true(luka));
                    } else {
                        luka_express_RPN_update(luka, express_cp, left2, luka_false(luka));
                    }
                }
            } else {
                luka_express_RPN_update(luka, express_cp, left2, luka_null(luka));
            }

            luka_data_check(luka, left1_p);
            luka_data_check(luka, left2_p);
            luka_express_RPN_rmv(luka, express_cp, left1);
            luka_express_RPN_rmv(luka, express_cp, oper);
        }

        //>
        else if (oper->oper == LUKA_OPER_MORE) {
            left2_p = luka_expressnode_exec(luka, vars, express_cp, left2);
            if ((luka_is_int(luka, left1_p) || luka_is_double(luka, left1_p)) && ((luka_is_int(luka, left2_p) || luka_is_double(luka, left2_p)))) {
                if (luka_is_int(luka, left1_p) && luka_is_int(luka, left2_p)) {
                    if (luka_get_int(luka, left2_p) > luka_get_int(luka, left1_p)) {
                        luka_express_RPN_update(luka, express_cp, left2, luka_true(luka));
                    } else {
                        luka_express_RPN_update(luka, express_cp, left2, luka_false(luka));
                    }
                } else {
                    if (luka_get_double(luka, left2_p) > luka_get_double(luka, left1_p)) {
                        luka_express_RPN_update(luka, express_cp, left2, luka_true(luka));
                    } else {
                        luka_express_RPN_update(luka, express_cp, left2, luka_false(luka));
                    }
                }
            } else {
                luka_express_RPN_update(luka, express_cp, left2, luka_null(luka));
            }

            luka_data_check(luka, left1_p);
            luka_data_check(luka, left2_p);
            luka_express_RPN_rmv(luka, express_cp, left1);
            luka_express_RPN_rmv(luka, express_cp, oper);
        }

        //<=
        else if (oper->oper == LUKA_OPER_LESSEQU) {
            left2_p = luka_expressnode_exec(luka, vars, express_cp, left2);
            if ((luka_is_int(luka, left1_p) || luka_is_double(luka, left1_p)) && ((luka_is_int(luka, left2_p) || luka_is_double(luka, left2_p)))) {
                if (luka_is_int(luka, left1_p) && luka_is_int(luka, left2_p)) {
                    if (luka_get_int(luka, left2_p) <= luka_get_int(luka, left1_p)) {
                        luka_express_RPN_update(luka, express_cp, left2, luka_true(luka));
                    } else {
                        luka_express_RPN_update(luka, express_cp, left2, luka_false(luka));
                    }
                } else {
                    if (luka_get_double(luka, left2_p) <= luka_get_double(luka, left1_p)) {
                        luka_express_RPN_update(luka, express_cp, left2, luka_true(luka));
                    } else {
                        luka_express_RPN_update(luka, express_cp, left2, luka_false(luka));
                    }
                }
            } else {
                luka_express_RPN_update(luka, express_cp, left2, luka_null(luka));
            }

            luka_data_check(luka, left1_p);
            luka_data_check(luka, left2_p);
            luka_express_RPN_rmv(luka, express_cp, left1);
            luka_express_RPN_rmv(luka, express_cp, oper);
        }

        //<
        else if (oper->oper == LUKA_OPER_LESS) {
            left2_p = luka_expressnode_exec(luka, vars, express_cp, left2);
            if ((luka_is_int(luka, left1_p) || luka_is_double(luka, left1_p)) && ((luka_is_int(luka, left2_p) || luka_is_double(luka, left2_p)))) {
                if (luka_is_int(luka, left1_p) && luka_is_int(luka, left2_p)) {
                    if (luka_get_int(luka, left2_p) < luka_get_int(luka, left1_p)) {
                        luka_express_RPN_update(luka, express_cp, left2, luka_true(luka));
                    } else {
                        luka_express_RPN_update(luka, express_cp, left2, luka_false(luka));
                    }
                } else {
                    if (luka_get_double(luka, left2_p) < luka_get_double(luka, left1_p)) {
                        luka_express_RPN_update(luka, express_cp, left2, luka_true(luka));
                    } else {
                        luka_express_RPN_update(luka, express_cp, left2, luka_false(luka));
                    }
                }
            } else {
                luka_express_RPN_update(luka, express_cp, left2, luka_null(luka));
            }

            luka_data_check(luka, left1_p);
            luka_data_check(luka, left2_p);
            luka_express_RPN_rmv(luka, express_cp, left1);
            luka_express_RPN_rmv(luka, express_cp, oper);
        }

        //==
        else if (oper->oper == LUKA_OPER_EQU2) {
            left2_p = luka_expressnode_exec(luka, vars, express_cp, left2);
            if ((luka_is_int(luka, left1_p) || luka_is_double(luka, left1_p)) && ((luka_is_int(luka, left2_p) || luka_is_double(luka, left2_p)))) {
                if (luka_is_int(luka, left1_p) && luka_is_int(luka, left2_p)) {
                    if (luka_get_int(luka, left2_p) == luka_get_int(luka, left1_p)) {
                        luka_express_RPN_update(luka, express_cp, left2, luka_true(luka));
                    } else {
                        luka_express_RPN_update(luka, express_cp, left2, luka_false(luka));
                    }
                } else {
                    if (luka_get_double(luka, left2_p) == luka_get_double(luka, left1_p)) {
                        luka_express_RPN_update(luka, express_cp, left2, luka_true(luka));
                    } else {
                        luka_express_RPN_update(luka, express_cp, left2, luka_false(luka));
                    }
                }
            } else if (luka_is_same(luka, left2_p, left1_p)) {
                if (luka_is_null(luka, left2_p) || luka_is_true(luka, left2_p) || luka_is_false(luka, left2_p)) {
                    luka_express_RPN_update(luka, express_cp, left2, luka_true(luka));
                } else if (luka_is_string(luka, left2_p) && strcmp(luka_get_string(luka, left2_p), luka_get_string(luka, left1_p)) == 0) {
                    luka_express_RPN_update(luka, express_cp, left2, luka_true(luka));
                } else {
                    luka_express_RPN_update(luka, express_cp, left2, luka_false(luka));
                }
            } else {
                luka_express_RPN_update(luka, express_cp, left2, luka_false(luka));
            }

            luka_data_check(luka, left1_p);
            luka_data_check(luka, left2_p);
            luka_express_RPN_rmv(luka, express_cp, left1);
            luka_express_RPN_rmv(luka, express_cp, oper);
        }

        //!=
        else if (oper->oper == LUKA_OPER_NOTQEU) {
            left2_p = luka_expressnode_exec(luka, vars, express_cp, left2);

            if ((luka_is_int(luka, left1_p) || luka_is_double(luka, left1_p)) && ((luka_is_int(luka, left2_p) || luka_is_double(luka, left2_p)))) {
                if (luka_is_int(luka, left1_p) && luka_is_int(luka, left2_p)) {
                    if (luka_get_int(luka, left2_p) == luka_get_int(luka, left1_p)) {
                        luka_express_RPN_update(luka, express_cp, left2, luka_false(luka));
                    } else {
                        luka_express_RPN_update(luka, express_cp, left2, luka_true(luka));
                    }
                } else {
                    if (luka_get_double(luka, left2_p) == luka_get_double(luka, left1_p)) {
                        luka_express_RPN_update(luka, express_cp, left2, luka_false(luka));
                    } else {
                        luka_express_RPN_update(luka, express_cp, left2, luka_true(luka));
                    }
                }
            } else if (luka_is_same(luka, left2_p, left1_p)) {
                if (luka_is_null(luka, left2_p) || luka_is_true(luka, left2_p) || luka_is_false(luka, left2_p)) {
                    luka_express_RPN_update(luka, express_cp, left2, luka_false(luka));
                } else if (luka_is_string(luka, left2_p) && strcmp(luka_get_string(luka, left2_p), luka_get_string(luka, left1_p)) == 0) {
                    luka_express_RPN_update(luka, express_cp, left2, luka_false(luka));
                } else {
                    luka_express_RPN_update(luka, express_cp, left2, luka_true(luka));
                }
            } else {
                luka_express_RPN_update(luka, express_cp, left2, luka_true(luka));
            }

            luka_data_check(luka, left1_p);
            luka_data_check(luka, left2_p);
            luka_express_RPN_rmv(luka, express_cp, left1);
            luka_express_RPN_rmv(luka, express_cp, oper);
        }

        //&&
        else if (oper->oper == LUKA_OPER_AND) {
            left2_p = luka_expressnode_exec(luka, vars, express_cp, left2);

            if (luka_is_true(luka, left2_p) && luka_is_true(luka, left1_p)) {
                luka_express_RPN_update(luka, express_cp, left2, luka_true(luka));
            } else {
                luka_express_RPN_update(luka, express_cp, left2, luka_false(luka));
            }

            luka_data_check(luka, left1_p);
            luka_data_check(luka, left2_p);
            luka_express_RPN_rmv(luka, express_cp, left1);
            luka_express_RPN_rmv(luka, express_cp, oper);
        }

        //||
        else if (oper->oper == LUKA_OPER_OR) {
            left2_p = luka_expressnode_exec(luka, vars, express_cp, left2);

            if (luka_is_true(luka, left2_p)) {
                luka_express_RPN_update(luka, express_cp, left2, luka_true(luka));
            } else {
                left1_p = luka_expressnode_exec(luka, vars, express_cp, left1);
                if (luka_is_true(luka, left1_p)) {
                    luka_express_RPN_update(luka, express_cp, left2, luka_true(luka));
                } else {
                    luka_express_RPN_update(luka, express_cp, left2, luka_false(luka));
                }
            }

            luka_express_RPN_rmv(luka, express_cp, left1);
            luka_express_RPN_rmv(luka, express_cp, oper);
        }

        //=
        else if (oper->oper == LUKA_OPER_EQU) {
			void *p_buf = NULL;

            if (left2->type == LUKA_EXP_VAR) {
				p_buf = rbtreec_get(luka, vars, left2->var_name);
                rbtreec_put(luka, vars, left2->var_name, left1_p);
                luka_express_RPN_update(luka, express_cp, left2, left1_p);
            } else if (left2->type == LUKA_EXP_OBJECT) {
				p_buf = luka_object_get(luka, luka_get_object(luka, left2->data), left2->obj_name);
                luka_object_put(luka, luka_get_object(luka, left2->data), left2->obj_name, left1_p);
                luka_express_RPN_update(luka, express_cp, left2, left1_p);
            } else if (left2->type == LUKA_EXP_ARRAY) {
				p_buf = luka_array_get(luka, luka_get_array(luka, left2->data), left2->arr_index);
            	luka_array_put(luka, luka_get_array(luka, left2->data), left2->arr_index, left1_p);
                luka_express_RPN_update(luka, express_cp, left2, left1_p);
            }

			if (p_buf != left1_p) {
				if (p_buf)
                    luka_data_down(luka, p_buf);
				luka_data_up(luka, left1_p);
			}
			
            luka_express_RPN_rmv(luka, express_cp, left1);
            luka_express_RPN_rmv(luka, express_cp, oper);
        }

        //'/='
        else if (oper->oper == LUKA_OPER_DIVIDEQU) {
			void *p_buf = NULL;

            left2_p = luka_expressnode_exec(luka, vars, express_cp, left2);

            if ((luka_is_int(luka, left1_p) || luka_is_double(luka, left1_p)) && ((luka_is_int(luka, left2_p) || luka_is_double(luka, left2_p)))) {
                if (luka_is_int(luka, left1_p) && luka_is_int(luka, left2_p)) {
                    buf_p = luka_put_int(luka, luka_get_int(luka, left2_p) / luka_get_int(luka, left1_p));
                } else {
                    buf_p = luka_put_double(luka, luka_get_double(luka, left2_p) / luka_get_double(luka, left1_p));
                }

                if (left2->type == LUKA_EXP_VAR) {
					p_buf = rbtreec_get(luka, vars, left2->var_name);
                    rbtreec_put(luka, vars, left2->var_name, buf_p);
                    luka_express_RPN_update(luka, express_cp, left2, buf_p);
                } else if (left2->type == LUKA_EXP_OBJECT) {
					p_buf = luka_object_get(luka, luka_get_object(luka, left2->data), left2->obj_name);
                	luka_object_put(luka, luka_get_object(luka, left2->data), left2->obj_name, buf_p);
                    luka_express_RPN_update(luka, express_cp, left2, buf_p);
                } else if (left2->type == LUKA_EXP_ARRAY) {
					p_buf = luka_array_get(luka, luka_get_array(luka, left2->data), left2->arr_index);
                	luka_array_put(luka, luka_get_array(luka, left2->data), left2->arr_index, buf_p);
                    luka_express_RPN_update(luka, express_cp, left2, buf_p);
                }
            } else {
                luka_express_RPN_update(luka, express_cp, left2, luka_null(luka));
            }

			if (p_buf != left1_p) {
				if (p_buf)
                    luka_data_down(luka, p_buf);
				luka_data_up(luka, buf_p);
			}
            
            luka_express_RPN_rmv(luka, express_cp, left1);
            luka_express_RPN_rmv(luka, express_cp, oper);
        }

        //'*='
        else if (oper->oper == LUKA_OPER_TIMESEQU) {
			void *p_buf = NULL;

            left2_p = luka_expressnode_exec(luka, vars, express_cp, left2);

            if ((luka_is_int(luka, left1_p) || luka_is_double(luka, left1_p)) && ((luka_is_int(luka, left2_p) || luka_is_double(luka, left2_p)))) {
                if (luka_is_int(luka, left1_p) && luka_is_int(luka, left2_p)) {
                    buf_p = luka_put_int(luka, luka_get_int(luka, left2_p) * luka_get_int(luka, left1_p));
                } else {
                    buf_p = luka_put_double(luka, luka_get_double(luka, left2_p) * luka_get_double(luka, left1_p));
                }

                if (left2->type == LUKA_EXP_VAR) {
					p_buf = rbtreec_get(luka, vars, left2->var_name);
                    rbtreec_put(luka, vars, left2->var_name, buf_p);
                    luka_express_RPN_update(luka, express_cp, left2, buf_p);
                } else if (left2->type == LUKA_EXP_OBJECT) {
					p_buf = luka_object_get(luka, luka_get_object(luka, left2->data), left2->obj_name);
                	luka_object_put(luka, luka_get_object(luka, left2->data), left2->obj_name, buf_p);
                    luka_express_RPN_update(luka, express_cp, left2, buf_p);
                } else if (left2->type == LUKA_EXP_ARRAY) {
					p_buf = luka_array_get(luka, luka_get_array(luka, left2->data), left2->arr_index);
                	luka_array_put(luka, luka_get_array(luka, left2->data), left2->arr_index, buf_p);
                    luka_express_RPN_update(luka, express_cp, left2, buf_p);
                }
            } else {
                luka_express_RPN_update(luka, express_cp, left2, luka_null(luka));
            }

			if (p_buf != left1_p) {
				if (p_buf)
                    luka_data_down(luka, p_buf);
				luka_data_up(luka, buf_p);
			}
            
            luka_express_RPN_rmv(luka, express_cp, left1);
            luka_express_RPN_rmv(luka, express_cp, oper);
        }

        //'%='
        else if (oper->oper == LUKA_OPER_REMAINEQU) {
			void *p_buf = NULL;

            left2_p = luka_expressnode_exec(luka, vars, express_cp, left2);

            if (luka_is_int(luka, left1_p) && luka_is_int(luka, left2_p)) {
                buf_p = luka_put_int(luka, luka_get_int(luka, left2_p) % luka_get_int(luka, left1_p));

                if (left2->type == LUKA_EXP_VAR) {
					p_buf = rbtreec_get(luka, vars, left2->var_name);
                    rbtreec_put(luka, vars, left2->var_name, buf_p);
                    luka_express_RPN_update(luka, express_cp, left2, buf_p);
                } else if (left2->type == LUKA_EXP_OBJECT) {
					p_buf = luka_object_get(luka, luka_get_object(luka, left2->data), left2->obj_name);
                	luka_object_put(luka, luka_get_object(luka, left2->data), left2->obj_name, buf_p);
                    luka_express_RPN_update(luka, express_cp, left2, buf_p);
                } else if (left2->type == LUKA_EXP_ARRAY) {
					p_buf = luka_array_get(luka, luka_get_array(luka, left2->data), left2->arr_index);
                	luka_array_put(luka, luka_get_array(luka, left2->data), left2->arr_index, buf_p);
                    luka_express_RPN_update(luka, express_cp, left2, buf_p);
                }
            } else {
                luka_express_RPN_update(luka, express_cp, left2, luka_null(luka));
            }

			if (p_buf != left1_p) {
				if (p_buf)
                    luka_data_down(luka, p_buf);
				luka_data_up(luka, buf_p);
			}
            
            luka_express_RPN_rmv(luka, express_cp, left1);
            luka_express_RPN_rmv(luka, express_cp, oper);
        }

        //'+='
        else if (oper->oper == LUKA_OPER_PLUSEQU) {
			void *p_buf = NULL;

            left2_p = luka_expressnode_exec(luka, vars, express_cp, left2);

            if ((luka_is_int(luka, left1_p) || luka_is_double(luka, left1_p)) && ((luka_is_int(luka, left2_p) || luka_is_double(luka, left2_p)))) {
                if (luka_is_int(luka, left1_p) && luka_is_int(luka, left2_p)) {
                    buf_p = luka_put_int(luka, luka_get_int(luka, left2_p) + luka_get_int(luka, left1_p));
                } else {
                    buf_p = luka_put_double(luka, luka_get_double(luka, left2_p) + luka_get_double(luka, left1_p));
                }

                if (left2->type == LUKA_EXP_VAR) {
					p_buf = rbtreec_get(luka, vars, left2->var_name);
                    rbtreec_put(luka, vars, left2->var_name, buf_p);
                    luka_express_RPN_update(luka, express_cp, left2, buf_p);
                } else if (left2->type == LUKA_EXP_OBJECT) {
					p_buf = luka_object_get(luka, luka_get_object(luka, left2->data), left2->obj_name);
                	luka_object_put(luka, luka_get_object(luka, left2->data), left2->obj_name, buf_p);
                    luka_express_RPN_update(luka, express_cp, left2, buf_p);
                } else if (left2->type == LUKA_EXP_ARRAY) {
					p_buf = luka_array_get(luka, luka_get_array(luka, left2->data), left2->arr_index);
                	luka_array_put(luka, luka_get_array(luka, left2->data), left2->arr_index, buf_p);
                    luka_express_RPN_update(luka, express_cp, left2, buf_p);
                }
            } else if (luka_is_string(luka, left1_p) && luka_is_string(luka, left2_p)) {
                const char *left1_s = luka_get_string(luka, left1_p);
                const char *left2_s = luka_get_string(luka, left2_p);

                char *new_str = (char *)luka_alloc(luka, strlen(left1_s) + strlen(left2_s) + 10);
                sprintf(new_str, "%s%s", left2_s, left1_s);
                buf_p = luka_put_string(luka, new_str);
                rbtreec_put(luka, vars, left2->var_name, buf_p);
				luka_express_RPN_update(luka, express_cp, left2, buf_p);

                p_buf = left2_p;
                luka_data_check(luka, left1_p);
            } else {
                luka_express_RPN_update(luka, express_cp, left2, luka_null(luka));
            }

			if (p_buf != left1_p) {
				if (p_buf)
                    luka_data_down(luka, p_buf);
				luka_data_up(luka, buf_p);
			}

            luka_express_RPN_rmv(luka, express_cp, left1);
            luka_express_RPN_rmv(luka, express_cp, oper);
        }

        //'-='
        else if (oper->oper == LUKA_OPER_MINUSEQU) {
			void *p_buf = NULL;

            left2_p = luka_expressnode_exec(luka, vars, express_cp, left2);

            if ((luka_is_int(luka, left1_p) || luka_is_double(luka, left1_p)) && ((luka_is_int(luka, left2_p) || luka_is_double(luka, left2_p)))) {
                if (luka_is_int(luka, left1_p) && luka_is_int(luka, left2_p)) {
                    buf_p = luka_put_int(luka, luka_get_int(luka, left2_p) - luka_get_int(luka, left1_p));
                } else {
                    buf_p = luka_put_double(luka, luka_get_double(luka, left2_p) - luka_get_double(luka, left1_p));
                }

                if (left2->type == LUKA_EXP_VAR) {
					p_buf = rbtreec_get(luka, vars, left2->var_name);
                    rbtreec_put(luka, vars, left2->var_name, buf_p);
                    luka_express_RPN_update(luka, express_cp, left2, buf_p);
                } else if (left2->type == LUKA_EXP_OBJECT) {
					p_buf = luka_object_get(luka, luka_get_object(luka, left2->data), left2->obj_name);
                	luka_object_put(luka, luka_get_object(luka, left2->data), left2->obj_name, buf_p);
                    luka_express_RPN_update(luka, express_cp, left2, buf_p);
                } else if (left2->type == LUKA_EXP_ARRAY) {
					p_buf = luka_array_get(luka, luka_get_array(luka, left2->data), left2->arr_index);
                	luka_array_put(luka, luka_get_array(luka, left2->data), left2->arr_index, buf_p);
                    luka_express_RPN_update(luka, express_cp, left2, buf_p);
                }
            } else {
                luka_express_RPN_update(luka, express_cp, left2, luka_null(luka));
            }

			if (p_buf != left1_p) {
				if (p_buf)
                    luka_data_down(luka, p_buf);
				luka_data_up(luka, buf_p);
			}
            
            luka_express_RPN_rmv(luka, express_cp, left1);
            luka_express_RPN_rmv(luka, express_cp, oper);
        }
    }

    dataID = luka_expressnode_exec(luka, vars, express_cp, express_cp->RPN);
    luka_express_destroy(luka, express_cp);
    return dataID;
}


