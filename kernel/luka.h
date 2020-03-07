// +--------------------------------------------------
// | luka.h 
// | hatsusakana@gmail.com 
// +--------------------------------------------------

#ifndef LUKA_H
#define LUKA_H

#define LUKA_MAIN   	"luka_main"  //虚拟主函数
#define LUKA_VERSION	"1.0.1"			//版本号
#define LUKA_DODATE		"2020/2/19"		//开发时间
#define LUKA_MAX    	100000000       //数组最大长度

typedef unsigned char *  bytep;
typedef void *           voidp;

typedef struct Luka Luka;

#ifdef __cplusplus
extern "C" {
#endif

// +--------------------------------------------------
// | data.c 
// +--------------------------------------------------

//RBTreeP
typedef struct RBTreeP RBTreeP;
RBTreeP    *rbtreep_create      ();
void       *rbtreep_alloc       (RBTreeP *tree, size_t n);
void        rbtreep_free        (RBTreeP *tree, void *p);
void        rbtreep_destroy     (RBTreeP *tree);

//RBTreeV
typedef struct RBTreeV RBTreeV;
RBTreeV    *rbtreev_create      (Luka *luka);
void        rbtreev_put         (Luka *luka, RBTreeV *tree, void *p);
int         rbtreev_exist       (Luka *luka, RBTreeV *tree, void *p);
void        rbtreev_rmv         (Luka *luka, RBTreeV *tree, void *p);
void        rbtreev_destroy     (Luka *luka, RBTreeV *tree);
voidp       rbtreev_get_fdata   (Luka *luka, RBTreeV *tree);
void        rbtreev_get_fdata2  (Luka *luka, RBTreeV *tree, void (*func_p)(Luka *, voidp p));

//RBTreeC
typedef struct RBTreeC RBTreeC;
RBTreeC    *rbtreec_create      (Luka *luka, void (*proc)(Luka *, void *));
void        rbtreec_put         (Luka *luka, RBTreeC *tree, const char *s, void *value);
void       *rbtreec_get         (Luka *luka, RBTreeC *tree, const char *s);
void        rbtreec_rmv         (Luka *luka, RBTreeC *tree, const char *s);
void        rbtreec_each        (Luka *luka, RBTreeC *tree, void *k, void (*cb)(Luka *, void *k, const char *p, void *value));
void        rbtreec_destroy     (Luka *luka, RBTreeC *tree);

//StrList
typedef struct StrList {
	char *s;
	struct StrList *next;
} StrList;
int         sl_push             (Luka *luka, StrList **sl, const char *s);
int         sl_push2            (Luka *luka, StrList **sl, char *s);
int         sb_exist            (Luka *luka, StrList *sl, const char *s);
void        sl_free             (Luka *luka, StrList *sl);

//LukaObject
typedef struct LukaObject LukaObject;
LukaObject *luka_object_create  (Luka *luka);
void        luka_object_setcb   (Luka *luka, LukaObject *object, void (*in_proc)(Luka *, voidp), void (*out_proc)(Luka *, voidp));
void        luka_object_put     (Luka *luka, LukaObject *object, const char *key, voidp p);
voidp       luka_object_get     (Luka *luka, LukaObject *object, const char *key);
void        luka_object_rmv     (Luka *luka, LukaObject *object, const char *key);
StrList    *luka_object_each    (Luka *luka, LukaObject *object);
void        luka_object_destroy (Luka *luka, LukaObject *object);

//LukaArray
typedef struct LukaArray LukaArray;
LukaArray  *luka_array_create   (Luka *luka);
void        luka_array_setcb    (Luka *luka, LukaArray *array, void (*in_proc)(Luka *luka, voidp p), void (*out_proc)(Luka *luka, voidp p));
void        luka_array_push     (Luka *luka, LukaArray *array, voidp p);
void        luka_array_put      (Luka *luka, LukaArray *array, size_t i, voidp p);
size_t      luka_array_length   (Luka *luka, LukaArray *array);
voidp       luka_array_get      (Luka *luka, LukaArray *array, size_t i);
void        luka_array_destroy  (Luka *luka, LukaArray *array);

//LukaStack
typedef struct LukaStack LukaStack;
LukaStack  *luka_stack_create   (Luka *luka);
void        luka_stack_push     (Luka *luka, LukaStack *stack, void *p);
void       *luka_stack_top      (Luka *luka, LukaStack *stack);
void        luka_stack_pop      (Luka *luka, LukaStack *stack);
void        luka_stack_destroy  (Luka *luka, LukaStack *stack);

// +--------------------------------------------------
// | gcc.c 
// +--------------------------------------------------

typedef enum {
    LUKA_GCC_IF       =  0,
    LUKA_GCC_ELSEIF   =  1,
    LUKA_GCC_ELSE     =  2,
    LUKA_GCC_WHILE    =  3,
    LUKA_GCC_FOR      =  4,
    LUKA_GCC_END      =  5,
    LUKA_GCC_EXPRESS  =  6,
    LUKA_GCC_RETURN   =  7,
    LUKA_GCC_FUNC     =  8,
    LUKA_GCC_BREAK    =  9,
    LUKA_GCC_CONTINUE = 10
} LukaGCCType;

typedef struct LukaGCC {
    const char *script_name;
    size_t line;

    LukaGCCType type;
    char  *express;
    char **a, **b, **c;
    size_t a_len, b_len, c_len;
    char  *func_name;
    char **func_param;
    size_t func_len;

    struct LukaGCC *next;
} LukaGCC;

LukaGCC *luka_gcc_decode (Luka *luka, const char *luka_script_path);
void     luka_gcc_free   (Luka *luka, LukaGCC *gcc);

// +--------------------------------------------------
// | LukaExpress 
// +--------------------------------------------------

typedef struct LukaExpress LukaExpress;

typedef enum {
    LUKA_EXP_DATA = 0, LUKA_EXP_VAR = 1, LUKA_EXP_FUNC = 2, LUKA_EXP_OPER = 3, LUKA_EXP_OBJECT = 4, LUKA_EXP_ARRAY = 5
} LukaExpressType;

typedef enum {
    LUKA_OPER_OBJ         =  0,     //->
    LUKA_OPER_ARRAY       =  1,     //[]有参
    LUKA_OPER_ARRAY_NOP   =  2,     //[]无参
    LUKA_OPER_SMA_BRACL   =  3,     //(
    LUKA_OPER_SMA_BRACR   =  4,     //)
    LUKA_OPER_DIVIDE      =  5,     //'/'
    LUKA_OPER_TIMES       =  6,     //*
    LUKA_OPER_REMAINDER   =  7,     //%
    LUKA_OPER_PLUS        =  8,     //+
    LUKA_OPER_MINUS       =  9,     //-
    LUKA_OPER_MOREEQU     = 10,     //>=
    LUKA_OPER_MORE        = 11,     //>
    LUKA_OPER_LESSEQU     = 12,     //<=
    LUKA_OPER_LESS        = 13,     //<
    LUKA_OPER_EQU2        = 14,     //==
    LUKA_OPER_NOTQEU      = 15,     //!=
    LUKA_OPER_AND         = 16,     //&&
    LUKA_OPER_OR          = 17,     //||
    LUKA_OPER_EQU         = 18,     //=
    LUKA_OPER_DIVIDEQU    = 19,     //'/='
    LUKA_OPER_TIMESEQU    = 20,     //*=
    LUKA_OPER_REMAINEQU   = 21,     //%=
    LUKA_OPER_PLUSEQU     = 22,     //+=
    LUKA_OPER_MINUSEQU    = 23,     //-=
    LUKA_OPER_NULL        = 24,     //
} LukaOper;

typedef struct LukaExpressNode {
    LukaExpressType type;

    voidp data;
    char *var_name;
    char *func_name;
    LukaExpress **func_param;
    size_t func_len;
    LukaOper oper;
    char *obj_name;
    size_t arr_index;

    struct LukaExpressNode *last;
    struct LukaExpressNode *next;
} LukaExpressNode;

struct LukaExpress {
    LukaExpressNode *queue, *queue_tail;
    LukaExpressNode *RPN, *RPN_tail;
    LukaExpressNode *stack;
};

LukaExpress *luka_express_decode (Luka *luka, LukaGCC *gcc);
voidp        luka_express_exec   (Luka *luka, RBTreeC *vars, LukaExpress *express);

// +--------------------------------------------------
// | func.c 
// +--------------------------------------------------

void luka_regs    (Luka *luka);
void luka_package (Luka *luka, const char *pkg_name);

// +--------------------------------------------------
// | luka.c 
// +--------------------------------------------------

int         luka_main         (int argc, char *argv[]);

void       *luka_alloc        (Luka *luka, size_t n);
void       *luka_alloc2       (Luka *luka, size_t n);
char       *luka_strdup       (Luka *luka, const char *s);
void        luka_free         (Luka *luka, void *p);
void        luka_exception    (Luka *luka, int code);
void        luka_reg          (Luka *luka, const char *func_name, voidp (*func_p)(Luka *, voidp *, size_t));
voidp       luka_call_func    (Luka *luka, const char *func_name, voidp *p, size_t n);

voidp       luka_null         (Luka *luka);
voidp       luka_true         (Luka *luka);
voidp       luka_false        (Luka *luka);
voidp       luka_put_int      (Luka *luka, int i);
voidp       luka_put_double   (Luka *luka, double d);
voidp       luka_put_string   (Luka *luka, char *s);
voidp       luka_put_byte     (Luka *luka, bytep datap, size_t size);
voidp       luka_put_object   (Luka *luka);
voidp       luka_put_array    (Luka *luka);
voidp       luka_put_voidp    (Luka *luka, void *p);

int         luka_is_null      (Luka *luka, voidp p);
int         luka_is_true      (Luka *luka, voidp p);
int         luka_is_false     (Luka *luka, voidp p);
int         luka_is_int       (Luka *luka, voidp p);
int         luka_is_double    (Luka *luka, voidp p);
int         luka_is_string    (Luka *luka, voidp p);
int         luka_is_byte      (Luka *luka, voidp p);
int         luka_is_object    (Luka *luka, voidp p);
int         luka_is_array     (Luka *luka, voidp p);
int         luka_is_voidp     (Luka *luka, voidp p);
int         luka_is_same      (Luka *luka, voidp p1, voidp p2);

int         luka_get_int      (Luka *luka, voidp p);
double      luka_get_double   (Luka *luka, voidp p);
const char *luka_get_string   (Luka *luka, voidp p);
bytep       luka_get_byte     (Luka *luka, voidp p, size_t *size);
LukaObject *luka_get_object   (Luka *luka, voidp p);
LukaArray  *luka_get_array    (Luka *luka, voidp p);
void       *luka_get_voidp    (Luka *luka, voidp p);

void        luka_data_up      (Luka *luka, voidp p);
void        luka_data_down    (Luka *luka, voidp p);
int         luka_data_index   (Luka *luka, voidp p);
void        luka_data_trash   (Luka *luka, voidp p);

#ifdef __cplusplus
}
#endif

#endif

