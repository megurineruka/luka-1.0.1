// +--------------------------------------------------
// | func.c
// | hatsusakana@gmail.com 
// +--------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "luka.h"

/** luka **/
static voidp luka_c_luka (Luka *luka, voidp *p, size_t n) {
	printf("*******************************************************\n");
	printf("*                    luka-1.0.1                       *\n");
	printf("*               hatsusakana@gmail.com                 *\n");
	printf("*******************************************************\n");
	return luka_null(luka);
}

/** print **/
static voidp luka_c_print (Luka *luka, voidp *p, size_t n) {
	size_t i = 0;

	for (i = 0; i < n; i++) {
		if (luka_is_null(luka, p[i])) {
			printf("null");
		} else if (luka_is_true(luka, p[i])) {
			printf("true");
		} else if (luka_is_false(luka, p[i])) {
			printf("false");
		} else if (luka_is_int(luka, p[i])) {
			printf("%d", luka_get_int(luka, p[i]));
		} else if (luka_is_double(luka, p[i])) {
			printf("%g", luka_get_double(luka, p[i]));
		} else if (luka_is_string(luka, p[i])) {
			printf("%s", luka_get_string(luka, p[i]));
		}
	}
	return luka_null(luka);
}

/** object **/
static voidp luka_c_object (Luka *luka, voidp *p, size_t n) {
	return luka_put_object(luka);
}

/** array **/
static voidp luka_c_array (Luka *luka, voidp *p, size_t n) {
	return luka_put_array(luka);
}

/** count **/
static voidp luka_c_count (Luka *luka, voidp *p, size_t n) {
	LukaArray *array = NULL;

	if (n < 1)
		return luka_null(luka);

	array = luka_get_array(luka, p[0]);
	return luka_put_int(luka, luka_array_length(luka, array));
}

static void luka_c_dump_ex (Luka *luka, voidp p) {
	size_t i = 0, size = 0;
	StrList *sl = NULL, *sl_mov = NULL;

	if (luka_is_null(luka, p)) {
		printf("[null]");
	} else if (luka_is_true(luka, p)) {
		printf("[true]");
	} else if (luka_is_false(luka, p)) {
		printf("[false]");
	} else if (luka_is_int(luka, p)) {
		printf("[int]%d", luka_get_int(luka, p));
	} else if (luka_is_double(luka, p)) {
		printf("[double]%g", luka_get_double(luka, p));
	} else if (luka_is_string(luka, p)) {
		printf("[string]%s", luka_get_string(luka, p));
	} else if (luka_is_byte(luka, p)) {
		luka_get_byte(luka, p, &size);
		printf("[byte]%d", size);
	} else if (luka_is_object(luka, p)) {
		printf("[obj]{");
		sl = luka_object_each(luka, luka_get_object(luka, p));
		sl_mov = sl;
		while (sl_mov) {
			printf("\"%s\":", sl_mov->s);
			luka_c_dump_ex(luka, luka_object_get(luka, luka_get_object(luka, p), sl_mov->s));
			if (sl_mov->next) {
				printf(",");
			}
			sl_mov = sl_mov->next;
		}
		sl_free(luka, sl);
		printf("}");
	} else if (luka_is_array(luka, p)) {
		printf("[array][");
		for (i = 0; i < luka_array_length(luka, luka_get_array(luka, p)); i++) {
			luka_c_dump_ex(luka, luka_array_get(luka, luka_get_array(luka, p), i));
			if (i + 1 < luka_array_length(luka, luka_get_array(luka, p)))
				printf(",");
		}
		printf("]");
	} else if (luka_is_voidp(luka, p)) {
		printf("[voidp]%ld", (long)luka_get_voidp(luka, p));
	}
}

/** dump **/
static voidp luka_c_dump (Luka *luka, voidp *p, size_t n) {
	size_t i = 0;

	for (i = 0; i < n; i++) {
		luka_c_dump_ex(luka, p[i]);
		printf("\n");
	}
	return luka_null(luka);
}

/** exit **/
static voidp luka_c_exit (Luka *luka, voidp *p, size_t n) {
    int ret = 0;

    if (n >=1 && luka_is_int(luka, p[0])) {
        ret = luka_get_int(luka, p[0]);
    }

    exit(ret);
    return luka_null(luka);
}

// +--------------------------------------------------
// | Package 
// +--------------------------------------------------

void luka_pkg_c_regs      (Luka *luka);

typedef struct Package {
	const char *pkg_name;
	void (*pkg_p)(Luka *);
	int pkg_load;
} Package;

static Package g_Package[] = {
	{"c",       luka_pkg_c_regs,      0},
	{NULL,      NULL,                 0}
};

void luka_package (Luka *luka, const char *pkg_name) {
	size_t i = 0;

	while (g_Package[i].pkg_name != NULL) {
		if (strcmp(g_Package[i].pkg_name, pkg_name) == 0) {
			if (g_Package[i].pkg_load == 0) {
				g_Package[i].pkg_p(luka);
				g_Package[i].pkg_load = 1;
			}
			return;
		}
		i++;
	}

	fprintf(stderr, "can't find package '%s'\n", pkg_name);
}

// +--------------------------------------------------
// | 注册Luka语言系统函数
// +--------------------------------------------------

void luka_regs (Luka *luka) {
	luka_reg(luka, "luka",    luka_c_luka);
	luka_reg(luka, "print",   luka_c_print);
	luka_reg(luka, "object",  luka_c_object);
	luka_reg(luka, "array",   luka_c_array);
	luka_reg(luka, "count",   luka_c_count);
	luka_reg(luka, "dump",    luka_c_dump);
	luka_reg(luka, "exit",    luka_c_exit);
}

