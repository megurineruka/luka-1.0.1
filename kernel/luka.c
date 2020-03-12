// +--------------------------------------------------
// | luka.c 
// | hatsusakana@gmail.com 
// +--------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include "luka.h"

int main (int argc, char *argv[]) {
    return argc > 1 ? luka_main(argc, argv) : 0;
}

// +--------------------------------------------------
// | 数据结构
// +--------------------------------------------------

typedef struct LukaData    LukaData;
typedef struct LukaCode    LukaCode;

typedef enum {
    LUKA_OOM = 1, LUKA_SYS = 2, LUKA_GCC = 3, LUKA_RUN = 4
} LukaError;

struct Luka {
    RBTreeP  *memery;
    jmp_buf   jumpp;

    RBTreeC  *c;
    RBTreeV  *data;
    RBTreeC  *func;

    LukaData *ntf[3];
};

typedef enum {
    LUKA_NULL   = 0, 
    LUKA_TRUE   = 1, 
    LUKA_FALSE  = 2, 
    LUKA_INT    = 3, 
    LUKA_DOUBLE = 4, 
    LUKA_STRING = 5, 
    LUKA_BYTE   = 6, 
    LUKA_OBJECT = 7, 
    LUKA_ARRAY  = 8,
    LUKA_VOIDP  = 9
} LukaType;

typedef struct LukaVal {
    int      i;
    double   d;
    char    *s;
    
    //byte
    bytep  data;
    size_t size;

    LukaObject *object;
    LukaArray  *array;

    void *p;
} LukaVal;

struct LukaData {
    LukaType type;
    LukaVal val;
    int index;
};

typedef enum {
    LUKA_IF       = 0,
    LUKA_ELSEIF   = 1,
    LUKA_ELSE     = 2,
    LUKA_WHILE    = 3,
    LUKA_FOR      = 4,
    LUKA_END      = 5,
    LUKA_EXPRESS  = 6,
    LUKA_RETURN   = 7,
    LUKA_BREAK    = 8,
    LUKA_CONTINUE = 9
} LukaCodeType;

struct LukaCode {
    LukaCodeType type;
    LukaExpress *express;

    //for循环使用
    LukaExpress **a, **b, **c;
    size_t a_len, b_len, c_len;
    
    struct LukaCode *last;
    struct LukaCode *next;
    struct LukaCode *jump;
};

typedef struct LukaFunc {
    char **func_param;
    size_t func_len;
    LukaCode *func_codes, *func_codes_tail;
} LukaFunc;

typedef struct LukaIWF {
    LukaCode *p;
    int flag;
} LukaIWF;

// +--------------------------------------------------
// | Luka 
// +--------------------------------------------------

static Luka *luka_create () {
    Luka *luka = (Luka *)calloc(1, sizeof(Luka));
    if (luka) {
        luka->memery = rbtreep_create();
        if (!luka->memery) {
            free(luka);
            return NULL;
        }
    }
    return luka;
}

/** 异常处理 **/
void luka_exception (Luka *luka, int code) {
    fprintf(stderr, "error code=%d\n", code);
    longjmp(luka->jumpp, code);
}

void *luka_alloc (Luka *luka, size_t n) {
    void *p = rbtreep_alloc(luka->memery, n);
    if (!p) luka_exception(luka, LUKA_OOM);
    return p;
}

void *luka_alloc2 (Luka *luka, size_t n) {
    void *p = rbtreep_alloc(luka->memery, n);
    return p;
}

char *luka_strdup (Luka *luka, const char *s) {
    size_t n = strlen(s);
    char *r = luka_alloc(luka, n + 1);
    memcpy(r, s, n);
    return r;
}

void luka_free (Luka *luka, void *p) {
    rbtreep_free(luka->memery, p);
}

static void luka_destroy (Luka *luka) {
    rbtreep_destroy(luka->memery);
    free(luka);
}

// +--------------------------------------------------
// | LukaC 
// +--------------------------------------------------

static voidp luka_func_call (Luka *luka, const char *func_name, voidp *p, size_t n);

/** 注册C函数 **/
void luka_reg (Luka *luka, const char *func_name, voidp (*func_p)(Luka *, voidp *, size_t)) {
    if (!func_name || strlen(func_name) == 0 || !func_p)
        luka_exception(luka, LUKA_SYS);

    if (rbtreec_get(luka, luka->c, func_name))
        luka_exception(luka, LUKA_SYS);

    rbtreec_put(luka, luka->c, func_name, func_p);
}

/** 调用C/luka函数 **/
voidp luka_call_func (Luka *luka, const char *func_name, voidp *p, size_t n) {
	voidp ret = NULL;
    voidp (*func_p)(Luka *, voidp *p, size_t n) = NULL;

    func_p = rbtreec_get(luka, luka->c, func_name);
    if (func_p) {
    	ret = func_p(luka, p, n);
    } else {
    	ret = luka_func_call(luka, func_name, p, n);
    }
    return ret;
}

// +--------------------------------------------------
// | LukaData 
// +--------------------------------------------------

/*************************添加数据*************************/

static void luka_data_init (Luka *luka) {
    luka->ntf[0] = (LukaData *)luka_alloc(luka, sizeof(LukaData)); //null
    luka->ntf[0]->type = LUKA_NULL;
    luka->ntf[1] = (LukaData *)luka_alloc(luka, sizeof(LukaData)); //true
    luka->ntf[1]->type = LUKA_TRUE;
    luka->ntf[2] = (LukaData *)luka_alloc(luka, sizeof(LukaData)); //false
    luka->ntf[2]->type = LUKA_FALSE;
}

voidp luka_null  (Luka*luka) {
    return (voidp)luka->ntf[0];
}

voidp luka_true  (Luka*luka) {
    return (voidp)luka->ntf[1];
}

voidp luka_false (Luka*luka) {
    return (voidp)luka->ntf[2];
}

voidp luka_put_int (Luka *luka, int i) {
    LukaData *data = (LukaData *)luka_alloc(luka, sizeof(LukaData));
    data->type = LUKA_INT;
    data->val.i = i;
    rbtreev_put(luka, luka->data, data);
    return data;
}

voidp luka_put_double (Luka *luka, double d) {
    LukaData *data = (LukaData *)luka_alloc(luka, sizeof(LukaData));
    data->type = LUKA_DOUBLE;
    data->val.d = d;
    rbtreev_put(luka, luka->data, data);
    return data;
}

voidp luka_put_string (Luka *luka, char *s) {
    LukaData *data = (LukaData *)luka_alloc(luka, sizeof(LukaData));
    data->type = LUKA_STRING;
    data->val.s = s;
    rbtreev_put(luka, luka->data, data);
    return data;
}

voidp luka_put_byte (Luka *luka, bytep datap, size_t size) {
    LukaData *data = (LukaData *)luka_alloc(luka, sizeof(LukaData));
    data->type = LUKA_BYTE;
    data->val.data = datap;
    data->val.size = size;
    rbtreev_put(luka, luka->data, data);
    return data;
}

voidp luka_put_object (Luka *luka) {
    LukaData *data = (LukaData *)luka_alloc(luka, sizeof(LukaData));
    data->type = LUKA_OBJECT;
    data->val.object = luka_object_create(luka);
    luka_object_setcb(luka, data->val.object, luka_data_up, luka_data_down);
    rbtreev_put(luka, luka->data, data);
    return data;
}

voidp luka_put_array (Luka *luka) {
    LukaData *data = (LukaData *)luka_alloc(luka, sizeof(LukaData));
    data->type = LUKA_ARRAY;
    data->val.array = luka_array_create(luka);
    luka_array_setcb(luka, data->val.array, luka_data_up, luka_data_down);
    rbtreev_put(luka, luka->data, data);
    return data;
}

voidp luka_put_voidp (Luka *luka, void *p) {
    LukaData *data = (LukaData *)luka_alloc(luka, sizeof(LukaData));
    data->type = LUKA_VOIDP;
    data->val.p = p;
    rbtreev_put(luka, luka->data, data);
    return data;
}

/*************************数据类型*************************/

int luka_is_null (Luka *luka, voidp p) {
    LukaData *pi = (LukaData *)p;
    return pi->type == LUKA_NULL;
}

int luka_is_true (Luka *luka, voidp p) {
    LukaData *pi = (LukaData *)p;
    return pi->type == LUKA_TRUE;
}

int luka_is_false (Luka *luka, voidp p) {
    LukaData *pi = (LukaData *)p;
    return pi->type == LUKA_FALSE;
}

int luka_is_int (Luka *luka, voidp p) {
    LukaData *pi = (LukaData *)p;
    return pi->type == LUKA_INT;
}

int luka_is_double (Luka *luka, voidp p) {
    LukaData *pi = (LukaData *)p;
    return pi->type == LUKA_DOUBLE;
}

int luka_is_string (Luka *luka, voidp p) {
    LukaData *pi = (LukaData *)p;
    return pi->type == LUKA_STRING;
}

int luka_is_byte (Luka *luka, voidp p) {
    LukaData *pi = (LukaData *)p;
    return pi->type == LUKA_BYTE;
}

int luka_is_object (Luka *luka, voidp p) {
    LukaData *pi = (LukaData *)p;
    return pi->type == LUKA_OBJECT;
}

int luka_is_array (Luka *luka, voidp p) {
    LukaData *pi = (LukaData *)p;
    return pi->type == LUKA_ARRAY;
}

int luka_is_voidp (Luka *luka, voidp p) {
    LukaData *pi = (LukaData *)p;
    return pi->type == LUKA_VOIDP;
}

int luka_is_same (Luka *luka, voidp p1, voidp p2) {
    LukaData *pi1 = (LukaData *)p1, *pi2 = (LukaData *)p2;
    return pi1->type == pi2->type;
}

/*************************获得数据*************************/

int luka_get_int (Luka *luka, voidp p) {
    LukaData *pi = (LukaData *)p;
    return pi->type == LUKA_INT ? pi->val.i : 0;
}

double luka_get_double (Luka *luka, voidp p) {
    LukaData *pi = (LukaData *)p;
    return pi->type == LUKA_DOUBLE ? pi->val.d : 0;
}

const char *luka_get_string (Luka *luka, voidp p) {
    LukaData *pi = (LukaData *)p;
    return pi->type == LUKA_STRING ? pi->val.s : "";
}

bytep luka_get_byte(Luka *luka, voidp p, size_t *size) {
	LukaData *pi = (LukaData *)p;
	if (pi->type != LUKA_BYTE) return NULL;

	*size = pi->val.size;
	return pi->val.data;
}

LukaObject *luka_get_object (Luka *luka, voidp p) {
    LukaData *pi = (LukaData *)p;
    return pi->type == LUKA_OBJECT ? pi->val.object : NULL;
}

LukaArray *luka_get_array (Luka *luka, voidp p) {
    LukaData *pi = (LukaData *)p;
    return pi->type == LUKA_ARRAY ? pi->val.array : NULL;
}

void *luka_get_voidp(Luka *luka, voidp p) {
	LukaData *pi = (LukaData *)p;
	return pi->type == LUKA_VOIDP ? pi->val.p : NULL;
}

/*************************数据清理*************************/

void luka_data_destroy (Luka *luka, LukaData *data) {
	if (data->type == LUKA_STRING) {
		luka_free(luka, data->val.s);
	} else if (data->type == LUKA_BYTE) {
		luka_free(luka, data->val.data);
	} else if (data->type == LUKA_OBJECT) {
		luka_object_destroy(luka, data->val.object);
	} else if (data->type == LUKA_ARRAY) {
		luka_array_destroy(luka, data->val.array);
	}

	luka_free(luka, data);
}

void luka_data_up (Luka *luka, voidp p) {
	LukaData *data = (LukaData *)p;

	if (data->type == LUKA_NULL || data->type == LUKA_TRUE || data->type == LUKA_FALSE)
		return;

	data->index++;
}

void luka_data_down (Luka *luka, voidp p) {
	LukaData *data = (LukaData *)p;

	if (data->type == LUKA_NULL || data->type == LUKA_TRUE || data->type == LUKA_FALSE)
		return;

	if (--data->index == 0) {
		rbtreev_rmv(luka, luka->data, p);
		luka_data_destroy(luka, data);
	}
}

void luka_data_check (Luka *luka, voidp p) {
	LukaData *data = (LukaData *)p;

	if (data->type == LUKA_NULL || data->type == LUKA_TRUE || data->type == LUKA_FALSE)
		return;

    if (data->index == 0) {
		rbtreev_rmv(luka, luka->data, p);
		luka_data_destroy(luka, data);
    }
}

// +--------------------------------------------------
// | LukaFunc 
// +--------------------------------------------------

/** 添加luka函数 **/
static void luka_func_add (Luka *luka, const char *func_name, char **func_param, size_t func_len) {
    LukaFunc *func = NULL;

    if (!func_name || strlen(func_name) == 0)
        luka_exception(luka, LUKA_SYS);

    if (rbtreec_get(luka, luka->func, func_name))
        luka_exception(luka, LUKA_SYS);

    func = (LukaFunc *)luka_alloc(luka, sizeof(LukaFunc));
    func->func_param = func_param;
    func->func_len = func_len;
    rbtreec_put(luka, luka->func, func_name, func);
}

/** 添加代码 **/
static void luka_func_push (Luka *luka, LukaFunc *func, LukaCodeType type, LukaExpress *express, 
    LukaExpress **a, LukaExpress **b, LukaExpress **c, size_t a_len, size_t b_len, size_t c_len) {
    LukaCode *code = (LukaCode *)luka_alloc(luka, sizeof(LukaCode));
    code->type = type;
    code->express = express;
    code->a = a;
    code->b = b;
    code->c = c;
    code->a_len = a_len;
    code->b_len = b_len;
    code->c_len = c_len;

    if (!func->func_codes) {
        func->func_codes = func->func_codes_tail = code;
    } else {
        func->func_codes_tail->next = code;
        code->last = func->func_codes_tail;
        func->func_codes_tail = code;
    }
}

/** 检查表达式中的函数调用 **/
static void luka_express_func_check (Luka *luka, LukaExpress *express) {
    size_t i = 0;
    LukaExpressNode *mov = express->RPN;

    while (mov) {
        if (mov->type == LUKA_EXP_FUNC) {
            if (!rbtreec_get(luka, luka->c, mov->func_name) && !rbtreec_get(luka, luka->func, mov->func_name)) {
                fprintf(stderr, "'%s' not define\n", mov->func_name);
                luka_exception(luka, LUKA_GCC);
            }

            if (mov->func_param) {
                for (i = 0; i < mov->func_len; i++) {
                    luka_express_func_check(luka, mov->func_param[i]);
                }
            }
        }

        mov = mov->next;
    }
}

/** 检查表达式中的变量名称 **/
static void luka_express_var_check (Luka *luka, LukaExpress *express) {
    size_t i = 0;
    LukaExpressNode *mov = express->RPN;

    while (mov) {
        if (mov->type == LUKA_EXP_VAR) {
            if (rbtreec_get(luka, luka->c, mov->var_name) || rbtreec_get(luka, luka->func, mov->var_name)) {
                fprintf(stderr, "can't use '%s'\n", mov->var_name);
                luka_exception(luka, LUKA_GCC);
            }

            if (mov->func_param) {
                for (i = 0; i < mov->func_len; i++) {
                    luka_express_var_check(luka, mov->func_param[i]);
                }
            }
        }

        mov = mov->next;
    }
}

/** 梳理结构 **/
static void luka_func_test (Luka *luka, void *k, const char *p, void *value) {
    size_t i = 0;
    LukaFunc *func = (LukaFunc *)value;
    LukaCode *code_mov = NULL, *code_buf = NULL;
    LukaStack *stack = NULL;

    if (!func->func_codes)
        return;

    //if/while/end对应
    stack = luka_stack_create(luka);
    code_mov = func->func_codes;
    while (code_mov) {
        if (code_mov->type == LUKA_FOR || code_mov->type == LUKA_IF || code_mov->type == LUKA_ELSEIF || code_mov->type == LUKA_ELSE || code_mov->type == LUKA_WHILE) {
            luka_stack_push(luka, stack, code_mov);
        } else if (code_mov->type == LUKA_END) {
            code_buf = luka_stack_top(luka, stack);
            if (!code_buf) {
                fprintf(stderr, "'%s' can't match '{}'\n", p);
                luka_exception(luka, LUKA_GCC);
            }
            code_buf->jump = code_mov;
            code_mov->jump = code_buf;
            luka_stack_pop(luka, stack);
        }
        code_mov = code_mov->next;
    }

    if (luka_stack_top(luka, stack)) {
        fprintf(stderr, "'%s' can't match '{}'\n", p);
        luka_exception(luka, LUKA_GCC);
    }
    luka_stack_destroy(luka, stack);

    //continue/break检查
    stack = luka_stack_create(luka);
    code_mov = func->func_codes;
    while (code_mov) {
        if (code_mov->type == LUKA_FOR || code_mov->type == LUKA_WHILE) {
            luka_stack_push(luka, stack, code_mov);
        } else if (code_mov->type == LUKA_END && (code_mov->jump->type == LUKA_FOR || code_mov->jump->type == LUKA_WHILE)) {
            luka_stack_pop(luka, stack);
        }

        if (code_mov->type == LUKA_BREAK || code_mov->type == LUKA_CONTINUE) {
            code_buf = luka_stack_top(luka, stack);
            if (!code_buf) {
                if (code_mov->type == LUKA_BREAK) fprintf(stderr, "can't use break in here\n");
                else fprintf(stderr, "can't use continue in here\n");
                luka_exception(luka, LUKA_GCC);
            }
        }

        code_mov = code_mov->next;
    }
    luka_stack_destroy(luka, stack);

    //if/else if/else对应检查
    code_mov = func->func_codes;
    while (code_mov) {
        if (code_mov->type == LUKA_ELSEIF || code_mov->type == LUKA_ELSE) {
            if (!code_buf || code_buf->type != LUKA_END) {
                fprintf(stderr, "'%s' else/else if error1\n", p);
                luka_exception(luka, LUKA_GCC);
            }
            if (code_buf->jump->type != LUKA_IF && code_buf->jump->type != LUKA_ELSEIF) {
                fprintf(stderr, "'%s' else/else if error2\n", p);
                luka_exception(luka, LUKA_GCC);
            }
        }
        code_buf = code_mov;
        code_mov = code_mov->next;
    }

    //调用函数检查
    code_mov = func->func_codes;
    while (code_mov) {
        if (code_mov->express) {
            luka_express_func_check(luka, code_mov->express);
            luka_express_var_check(luka, code_mov->express);
        }
        code_mov = code_mov->next;
    }
}

/** 检查a/b是否为同一if/else **/
static int luka_func_check_if (LukaCode *a, LukaCode *b) {
    if (a == b)
        return 1;

    while (b->type != LUKA_IF) {
        b = b->last->jump;
        if (a == b) return 1;
    }
    return 0;
}

/** 调用luka函数 **/
static voidp luka_func_call (Luka *luka, const char *func_name, voidp *p, size_t n) {
    size_t i = 0;

    LukaFunc  *func  = NULL;
    LukaCode  *mov   = NULL;
    RBTreeC   *vars  = NULL;
    LukaStack *stack = NULL;
    LukaIWF   *iwf   = NULL;
    voidp dataID    = luka_null(luka);
    int ifwhile_flag = 0;

    LukaIWF *buf = NULL;

    vars  = rbtreec_create(luka, NULL);
    func  = rbtreec_get(luka, luka->func, func_name);
    stack = luka_stack_create(luka);

    //形参
    if (func->func_param && p) {
        size_t i = 0;
        for (i = 0; i < func->func_len && i < n; i++) {
             rbtreec_put(luka, vars, func->func_param[i], p[i]);
        }
    }

    mov = func->func_codes;
    while (mov) {
        //表达式
        if (mov->type == LUKA_EXPRESS) {
            dataID = luka_express_exec(luka, vars, mov->express);
            luka_data_check(luka, dataID);
            mov = mov->next;
        }
        
        //return
        else if (mov->type == LUKA_RETURN) {
            dataID = luka_express_exec(luka, vars, mov->express);
            rbtreec_destroy(luka, vars);
            luka_stack_destroy(luka, stack);
            return dataID;
        }
        
        //if
        else if (mov->type == LUKA_IF) {
            dataID = luka_express_exec(luka, vars, mov->express);
            if (dataID == luka_true(luka)) {
                iwf = (LukaIWF *)luka_alloc(luka, sizeof(LukaIWF));
                iwf->p = mov;
                luka_stack_push(luka, stack, iwf);
                mov = mov->next;
            } else {
                mov = mov->jump;
            }
        }

        //else if
        else if (mov->type == LUKA_ELSEIF) {
            if ((buf = luka_stack_top(luka, stack)) != NULL && luka_func_check_if(buf->p, mov)) {
                mov = mov->jump;
            } else {
                dataID = luka_express_exec(luka, vars, mov->express);
                if (dataID == luka_true(luka)) {
                    iwf = (LukaIWF *)luka_alloc(luka, sizeof(LukaIWF));
                    iwf->p = mov;
                    luka_stack_push(luka, stack, iwf);
                    mov = mov->next;
                } else {
                    mov = mov->jump;
                }
            }
        }
        
        //else
        else if (mov->type == LUKA_ELSE) {
            if ((buf = luka_stack_top(luka, stack)) != NULL && luka_func_check_if(buf->p, mov)) {
                mov = mov->jump;
            } else {
                iwf = (LukaIWF *)luka_alloc(luka, sizeof(LukaIWF));
                iwf->p = mov;
                luka_stack_push(luka, stack, iwf);
                mov = mov->next;
            }
        } 
        
        //while
        else if (mov->type == LUKA_WHILE) {
            dataID = luka_express_exec(luka, vars, mov->express);
            if (dataID == luka_true(luka)) {
                //第一次进入循环
                if ((buf = luka_stack_top(luka, stack)) == NULL || buf->p != mov) {
                    iwf = (LukaIWF *)luka_alloc(luka, sizeof(LukaIWF));
                    iwf->p = mov;
                    luka_stack_push(luka, stack, iwf);
                }
                mov = mov->next;
            } else {
                if ((buf = luka_stack_top(luka, stack)) != NULL && buf->p == mov) {
                    luka_stack_pop(luka, stack);
                }
                mov = mov->jump;
            }
        }
        
        //for
        else if (mov->type == LUKA_FOR) {
            //第一次进入循环
            if ((buf = luka_stack_top(luka, stack)) == NULL || buf->p != mov) {
                if (mov->a) {
                    for (i = 0; i < mov->a_len; i++) {
                        dataID = luka_express_exec(luka, vars, mov->a[i]);
                        luka_data_check(luka, dataID);
                    }
                }
            }

            if (mov->b) {
                int b_flag = 0;
                for (i = 0; i < mov->b_len; i++) {
                    dataID = luka_express_exec(luka, vars, mov->b[i]);
                    if (dataID != luka_true(luka)) {
                        b_flag = 1;
                        break;
                    }
                }

                //for->b存在false
                if (b_flag == 1) {
                    if ((buf = luka_stack_top(luka, stack)) != NULL && buf->p == mov) {
                        luka_stack_pop(luka, stack);
                    }
                    mov = mov->jump;
                } else {
                    if ((buf = luka_stack_top(luka, stack)) == NULL || buf->p != mov) {
                        iwf = (LukaIWF *)luka_alloc(luka, sizeof(LukaIWF));
                        iwf->p = mov;
                        luka_stack_push(luka, stack, iwf);
                    }
                    mov = mov->next;
                }
            } else {
                if ((buf = luka_stack_top(luka, stack)) == NULL || buf->p != mov) {
                    iwf = (LukaIWF *)luka_alloc(luka, sizeof(LukaIWF));
                    iwf->p = mov;
                    luka_stack_push(luka, stack, iwf);
                }
                mov = mov->next;
            }
        }
        
        //end
        else if (mov->type == LUKA_END) {
            //while end
            if (mov->jump->type == LUKA_WHILE) {
                if ((buf = luka_stack_top(luka, stack)) != NULL && buf->p == mov->jump) {
                    mov = mov->jump;
                } else {
                    mov = mov->next;
                }
            }
            
            //for end
            else if (mov->jump->type == LUKA_FOR) {
                if ((buf = luka_stack_top(luka, stack)) != NULL && buf->p == mov->jump) {
                    mov = mov->jump;

                    //执行for->c
                    if (mov->c) {
                        for (i = 0; i < mov->c_len; i++) {
                            dataID = luka_express_exec(luka, vars, mov->c[i]);
                        }
                    }
                } else {
                    mov = mov->next;
                }
            }
            
            //if/else end
            else {
                mov = mov->next;
                if (mov && mov->type != LUKA_ELSEIF && mov->type != LUKA_ELSE) {
                    if ((buf = luka_stack_top(luka, stack)) != NULL && luka_func_check_if(buf->p, mov->last->jump)) {
                        luka_stack_pop(luka, stack);
                    }
                }
            }
        }

        //break
        else if (mov->type == LUKA_BREAK) {
            LukaIWF *wi_mov = luka_stack_top(luka, stack);
            while (wi_mov->p->type != LUKA_FOR && wi_mov->p->type != LUKA_WHILE) {
                luka_stack_pop(luka, stack);
                wi_mov = luka_stack_top(luka, stack);
            }

            mov = wi_mov->p->jump;
            luka_stack_pop(luka, stack);
        }

        //continue
        else if (mov->type == LUKA_CONTINUE) {
            LukaIWF *wi_mov = luka_stack_top(luka, stack);
            while (wi_mov->p->type != LUKA_FOR && wi_mov->p->type != LUKA_WHILE) {
                luka_stack_pop(luka, stack);
                wi_mov = luka_stack_top(luka, stack);
            }

            mov = wi_mov->p->jump;
        }
    }

    rbtreec_destroy(luka, vars);
    luka_stack_destroy(luka, stack);
    return luka_null(luka);
}

// +--------------------------------------------------
// | LukaGCC 
// +--------------------------------------------------

static void luka_gcc (Luka *luka, const char *luka_script_path) {
    LukaGCC *gcc = NULL, *gcc2 = NULL;
    LukaFunc *func_p = NULL, *func_main = NULL;
    LukaExpress *express = NULL;

    size_t i = 0;
    int nesting = 0;

    func_p = func_main = rbtreec_get(luka, luka->func, LUKA_MAIN);
    gcc = luka_gcc_decode(luka, luka_script_path);
    while (gcc) {
        if (gcc->type == LUKA_GCC_EXPRESS) {
            express = luka_express_decode(luka, gcc);
            luka_func_push(luka, func_p, LUKA_EXPRESS, express, NULL, NULL, NULL, 0, 0, 0);
        } else if (gcc->type == LUKA_GCC_IF) {
            express = luka_express_decode(luka, gcc);
            luka_func_push(luka, func_p, LUKA_IF, express, NULL, NULL, NULL, 0, 0, 0);
            nesting++;
        } else if (gcc->type == LUKA_GCC_END) {
            if (nesting == 0) {
                if (func_p != func_main) {
                    func_p = func_main;
                } else {
                    fprintf(stderr, "can't gcc '%s':%d error '}'\n", gcc->script_name, gcc->line);
                    luka_exception(luka, LUKA_GCC);
                }
            } else {
                luka_func_push(luka, func_p, LUKA_END, NULL, NULL, NULL, NULL, 0, 0, 0);
                nesting--;

                if (nesting < 0) {
                    fprintf(stderr, "can't gcc '%s':%d error '}'\n", gcc->script_name, gcc->line);
                    luka_exception(luka, LUKA_GCC);
                }
            }
        } else if (gcc->type == LUKA_GCC_ELSEIF) {
            express = luka_express_decode(luka, gcc);
            luka_func_push(luka, func_p, LUKA_ELSEIF, express, NULL, NULL, NULL, 0, 0, 0);
            nesting++;
        } else if (gcc->type == LUKA_GCC_ELSE) {
            luka_func_push(luka, func_p, LUKA_ELSE, NULL, NULL, NULL, NULL, 0, 0, 0);
            nesting++;
        } else if (gcc->type == LUKA_GCC_WHILE) {
            express = luka_express_decode(luka, gcc);
            luka_func_push(luka, func_p, LUKA_WHILE, express, NULL, NULL, NULL, 0, 0, 0);
            nesting++;
        } else if (gcc->type == LUKA_GCC_FOR) {
            LukaExpress **a = NULL, **b = NULL, **c = NULL;

            if (gcc->a) {
                a = (LukaExpress **)luka_alloc(luka, sizeof(LukaExpress *) * gcc->a_len);
                for (i = 0; i < gcc->a_len; i++) {
                    gcc2 = luka_alloc(luka, sizeof(LukaGCC));
                    gcc2->type = LUKA_GCC_EXPRESS;
                    gcc2->express = luka_strdup(luka, gcc->a[i]);
                    gcc2->next = NULL;
                    a[i] = luka_express_decode(luka, gcc2);
                    luka_gcc_free(luka, gcc2);
                }
            }

            if (gcc->b) {
                b = (LukaExpress **)luka_alloc(luka, sizeof(LukaExpress *) * gcc->b_len);
                for (i = 0; i < gcc->b_len; i++) {
                    gcc2 = luka_alloc(luka, sizeof(LukaGCC));
                    gcc2->type = LUKA_GCC_EXPRESS;
                    gcc2->express = luka_strdup(luka, gcc->b[i]);
                    gcc2->next = NULL;
                    b[i] = luka_express_decode(luka, gcc2);
                    luka_gcc_free(luka, gcc2);
                }
            }

            if (gcc->c) {
                c = (LukaExpress **)luka_alloc(luka, sizeof(LukaExpress *) * gcc->c_len);
                for (i = 0; i < gcc->c_len; i++) {
                    gcc2 = luka_alloc(luka, sizeof(LukaGCC));
                    gcc2->type = LUKA_GCC_EXPRESS;
                    gcc2->express = luka_strdup(luka, gcc->c[i]);
                    gcc2->next = NULL;
                    c[i] = luka_express_decode(luka, gcc2);
                    luka_gcc_free(luka, gcc2);
                }
            }

            luka_func_push(luka, func_p, LUKA_FOR, NULL, a, b, c, gcc->a_len, gcc->b_len, gcc->c_len);
            nesting++;
        }

        else if (gcc->type == LUKA_GCC_FUNC) {
            if (nesting != 0 && func_p != func_main) {
                fprintf(stderr, "can't gcc '%s':%d error define '%s'\n", gcc->script_name, gcc->line, gcc->func_name);
                luka_exception(luka, LUKA_GCC);
            }

            luka_func_add(luka, gcc->func_name, gcc->func_param, gcc->func_len);
            func_p = rbtreec_get(luka, luka->func, gcc->func_name);
        }

        else if (gcc->type == LUKA_GCC_RETURN) {
            express = luka_express_decode(luka, gcc);
            luka_func_push(luka, func_p, LUKA_RETURN, express, NULL, NULL, NULL, 0, 0, 0);
        }

        else if (gcc->type == LUKA_GCC_BREAK) {
            luka_func_push(luka, func_p, LUKA_BREAK, NULL, NULL, NULL, NULL, 0, 0, 0);
        }

        else if (gcc->type == LUKA_GCC_CONTINUE) {
            luka_func_push(luka, func_p, LUKA_CONTINUE, NULL, NULL, NULL, NULL, 0, 0, 0);
        }

        gcc = gcc->next;
    }
    luka_gcc_free(luka, gcc);

    if (nesting != 0)  {
        fprintf(stderr, "gcc failed, '}' is not over\n");
        luka_exception(luka, LUKA_GCC);
    }

    //梳理结构
    rbtreec_each(luka, luka->func, NULL, luka_func_test);
}

// +--------------------------------------------------
// | 执行脚本
// +--------------------------------------------------

int luka_main (int argc, char *argv[]) {
    Luka *luka = NULL;
    const char *luka_script_path = NULL;
    voidp *func_param = NULL;
    size_t i = 0, func_len = 0;

    luka_script_path = argv[1];
    if (!luka_script_path) {
        return -1;
    }

    luka = luka_create();
    if (!luka) {
        return -1;
    }

    if (setjmp(luka->jumpp) != 0) {
        luka_destroy(luka);
        return -2;
    }

    luka->c = rbtreec_create(luka, NULL);
    luka->data = rbtreev_create(luka);
    luka->func = rbtreec_create(luka, NULL);

    luka_regs(luka);
    luka_data_init(luka);
    luka_gcc(luka, luka_script_path);

    if (!rbtreec_get(luka, luka->func, LUKA_MAIN)) {
        luka_destroy(luka);
        return -3;
    }

    if (argc >= 3) {
        func_len = argc - 2;
        func_param = (voidp *)luka_alloc(luka, sizeof(voidp) * func_len);
        for (i = 0; i < func_len; i++) {
            func_param[i] =  luka_put_string(luka, luka_strdup(luka, argv[i + 2]));
        }

        luka_call_func(luka, LUKA_MAIN, func_param, func_len);
        luka_free(luka, func_param);
    } else {
        luka_func_call(luka, LUKA_MAIN, NULL, 0);
    }

    luka_destroy(luka);
    return 1;
}
