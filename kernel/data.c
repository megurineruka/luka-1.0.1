// +--------------------------------------------------
// | data.c 
// | hatsusakana@gmail.com 
// +--------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "luka.h"

enum RB_COLOR {BLACK, RED};

// +--------------------------------------------------
// | 红黑树(内存回收器)
// +--------------------------------------------------

typedef struct RBTreePNode {
    enum RB_COLOR color;
    void *p;
    struct RBTreePNode *left, *right, *parent;
} RBTreePNode;

struct RBTreeP {
    RBTreePNode *root, *nil;
};

RBTreeP *rbtreep_create () {
    RBTreeP *tree = NULL;

    tree = (RBTreeP *)calloc(1, sizeof(RBTreeP));
    if (!tree)
        return NULL;
    
    tree->nil = (RBTreePNode *)calloc(1, sizeof(RBTreePNode));
    if (!tree->nil) {
        free(tree);
        return NULL;
    }

    tree->nil->color = BLACK;
    tree->nil->left = NULL;
    tree->nil->right = NULL;
    tree->nil->parent = NULL;
    tree->root = tree->nil;
    return tree;
}

static RBTreePNode *rbtreep_search (RBTreeP *tree, void *p) {
    RBTreePNode *x = tree->root;
    while (x != tree->nil) {
        if (p > x->p) x = x->right;
        else if(p < x->p) x = x->left;
        else if(p == x->p) return x;
    }
    return NULL;
}

static void rbtreep_left_rotate (RBTreeP *tree, RBTreePNode *x) {
    RBTreePNode *y = x->right;
    x->right = y->left;

    if (y->left != tree->nil)
        y->left->parent = x;
    if (x != tree->nil)
        y->parent = x->parent;
    if(x->parent == tree->nil) 
        tree->root = y; 
    else {
        if (x->parent->left == x)
            x->parent->left = y;
        else
            x->parent->right = y;
    }
    y->left = x;
    if (x != tree->nil)
        x->parent = y;
}

static void rbtreep_right_rotate (RBTreeP *tree, RBTreePNode *x) {
    RBTreePNode *y = x->left;
    x->left = y->right;

    if (y->right != tree->nil)
        y->right->parent = x;
    if (x != tree->nil)
        y->parent = x->parent;
    if (x->parent == tree->nil)
        tree->root = y;
    else {
        if(x->parent->left == x)
            x->parent->left = y;
        else
            x->parent->right = y;
    }
    y->right = x;
    if(x != tree->nil)
        x->parent = y;
}

static void rbtreep_insert_fixup (RBTreeP *tree, RBTreePNode *z) {
    RBTreePNode *y = NULL;
    while ((z->parent != NULL) && (z->parent->color == RED)) {
        if (z->parent == z->parent->parent->left) { 
            y = z->parent->parent->right; 
            if ((y != NULL) && (y->color == RED)) {
                z->parent->color = BLACK; 
                y->color = BLACK;
                z->parent->parent->color = RED; 
                z = z->parent->parent; 
            } else {
                if (z == z->parent->right) { 
                    z = z->parent;
                    rbtreep_left_rotate(tree, z);
                }
                z->parent->color = BLACK; 
                z->parent->parent->color = RED;
                rbtreep_right_rotate(tree, z->parent->parent);
            }        
        } else { 
            y = z->parent->parent->left;
            if ((y != NULL) && (y->color == RED)) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rbtreep_right_rotate(tree, z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                rbtreep_left_rotate(tree, z->parent->parent);
            }
        }
    }
    tree->root->color = BLACK;
    tree->nil->color = BLACK;
}

static int rbtreep_insert (RBTreeP *tree, void *p) {
    RBTreePNode *x = tree->root, *y = tree->nil, *z;

    z = (RBTreePNode *)calloc(1, sizeof(RBTreePNode));
    if (!z)
        return 0;
    z->color = RED;
    z->p = p;
    z->parent = NULL;
    z->left = NULL;
    z->right = NULL;
    while (x != tree->nil) {
        y = x;
        if (z->p < x->p)
            x = x->left;
        else if (z->p > x->p)
            x = x->right;
        else if (z->p == x->p) {
            free(z);
            return 0;
        }
    }
    z->parent = y;
    if (y != tree->nil) {
        if(z->p > y->p)
            y->right = z;
        else if (z->p<y->p)
            y->left = z;
    } else {
        tree->root = z; 
    }
    z->left = tree->nil;
    z->right = tree->nil;
    rbtreep_insert_fixup(tree, z);
    return 1;
}

static void rbtreep_delete_fixup (RBTreeP *tree, RBTreePNode *x) {
    RBTreePNode *w = NULL;

    while ((x != tree->root) && (x->color == BLACK)) {
        if (x == x->parent->left) {
            w = x->parent->right;
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                rbtreep_left_rotate(tree, x->parent);
                w = x->parent->right;
            }
            if (w->left->color == BLACK && w->right->color == BLACK) {
                w->color = RED;
                x = x->parent;
            } else {
                if(w->right->color == BLACK) {
                    w->left->color = BLACK;
                    w->color = RED;
                    rbtreep_right_rotate(tree, w);
                    w = x->parent->right;
                } 
                w->color = w->parent->color;
                x->parent->color = BLACK;
                w->right->color = BLACK;
                rbtreep_left_rotate(tree, x->parent);
                x = tree->root;
            }
        } else {
            w = x->parent->left;
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                rbtreep_right_rotate(tree, x->parent);
                w = x->parent->left;
            }
            if (w->right->color == BLACK && w->left->color == BLACK) {
                w->color = RED;
                x = x->parent;
            } else {
                if (w->left->color == BLACK) {
                    w->right->color = BLACK;
                    w->color = RED;
                    rbtreep_left_rotate(tree, w);
                    w = x->parent->left;
                }
                w->color = w->parent->color;
                x->parent->color = BLACK;
                w->left->color = BLACK;
                rbtreep_right_rotate(tree, x->parent);
                x = tree->root;
            }
        }
    }
    x->color = BLACK;
}

static void rbtreep_delete (RBTreeP *tree, RBTreePNode *z) {
    RBTreePNode *y = NULL, *x = NULL;

    if (z->left == tree->nil || z->right == tree->nil) y=z;
    else {
        y = z->right;
        while (y->left != tree->nil) y = y->left;
    }
    if (y->left != tree->nil) x = y->left;
    else x = y->right;
    x->parent = y->parent;
    if (y->parent == tree->nil)
        tree->root = x;
    else {
        if (y->parent->left == y) y->parent->left = x;
        else y->parent->right = x;
    }
    if (y != z) z->p = y->p;
    if (y->color == BLACK) rbtreep_delete_fixup(tree, x);
    free(y);
}

void *rbtreep_alloc (RBTreeP *tree, size_t n) {
    void *p = calloc(1, n);
    if (!p)
        return NULL;

    if (!rbtreep_insert(tree, p)) {
        free(p);
        return NULL;
    }
    return p;
}

void rbtreep_free (RBTreeP *tree, void *p) {
    rbtreep_delete(tree, rbtreep_search(tree, p));
    free(p);
}

static void rbtreep_destroy_ex (RBTreeP *tree, RBTreePNode *node) {
    if (node == tree->nil)
        return;
    
    free(node->p);
    rbtreep_destroy_ex(tree, node->left);
    rbtreep_destroy_ex(tree, node->right);
    free(node);
}

void rbtreep_destroy (RBTreeP *tree) {
    rbtreep_destroy_ex(tree, tree->root);
    free(tree->nil);
    free(tree);
}

// +--------------------------------------------------
// | 红黑树(void *)
// +--------------------------------------------------

typedef struct RBTreeVNode {
    enum RB_COLOR color;
    void *p;
    struct RBTreeVNode *left, *right, *parent;
} RBTreeVNode;

struct RBTreeV {
    RBTreeVNode *root, *nil;
};

RBTreeV *rbtreev_create (Luka *luka) {
    RBTreeV *tree = (RBTreeV *)luka_alloc(luka, sizeof(RBTreeV));
    tree->nil = (RBTreeVNode *)luka_alloc(luka, sizeof(RBTreeVNode));
    tree->nil->color = BLACK;
    tree->nil->left = NULL;
    tree->nil->right = NULL;
    tree->nil->parent = NULL;
    tree->root = tree->nil;
    return tree;
}

static RBTreeVNode *rbtreev_search (RBTreeV *tree, void *p) {
    RBTreeVNode *x = tree->root;
    while (x != tree->nil) {
        if (p > x->p) x = x->right;
        else if(p < x->p) x = x->left;
        else if(p == x->p) return x;
    }
    return NULL;
}

static void rbtreev_left_rotate (RBTreeV *tree, RBTreeVNode *x) {
    RBTreeVNode *y = x->right;
    x->right = y->left;

    if (y->left != tree->nil)
        y->left->parent = x;
    if (x != tree->nil)
        y->parent = x->parent;
    if(x->parent == tree->nil) 
        tree->root = y; 
    else {
        if (x->parent->left == x)
            x->parent->left = y;
        else
            x->parent->right = y;
    }
    y->left = x;
    if (x != tree->nil)
        x->parent = y;
}

static void rbtreev_right_rotate (RBTreeV *tree, RBTreeVNode *x) {
    RBTreeVNode *y = x->left;
    x->left = y->right;

    if (y->right != tree->nil)
        y->right->parent = x;
    if (x != tree->nil)
        y->parent = x->parent;
    if (x->parent == tree->nil)
        tree->root = y;
    else {
        if(x->parent->left == x)
            x->parent->left = y;
        else
            x->parent->right = y;
    }
    y->right = x;
    if(x != tree->nil)
        x->parent = y;
}

static void rbtreev_insert_fixup (RBTreeV *tree, RBTreeVNode *z) {
    RBTreeVNode *y = NULL;
    while ((z->parent != NULL) && (z->parent->color == RED)) {
        if (z->parent == z->parent->parent->left) { 
            y = z->parent->parent->right; 
            if ((y != NULL) && (y->color == RED)) {
                z->parent->color = BLACK; 
                y->color = BLACK;
                z->parent->parent->color = RED; 
                z = z->parent->parent; 
            } else {
                if (z == z->parent->right) { 
                    z = z->parent;
                    rbtreev_left_rotate(tree, z);
                }
                z->parent->color = BLACK; 
                z->parent->parent->color = RED;
                rbtreev_right_rotate(tree, z->parent->parent);
            }        
        } else { 
            y = z->parent->parent->left;
            if ((y != NULL) && (y->color == RED)) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rbtreev_right_rotate(tree, z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                rbtreev_left_rotate(tree, z->parent->parent);
            }
        }
    }
    tree->root->color = BLACK;
    tree->nil->color = BLACK;
}

static void rbtreev_insert (Luka *luka, RBTreeV *tree, void *p) {
    RBTreeVNode *x = tree->root, *y = tree->nil, *z;

    z = (RBTreeVNode *)luka_alloc(luka, sizeof(RBTreeVNode));
    z->color = RED;
    z->p = p;
    z->parent = NULL;
    z->left = NULL;
    z->right = NULL;
    while (x != tree->nil) {
        y = x;
        if (z->p < x->p)
            x = x->left;
        else if (z->p > x->p)
            x = x->right;
        else if (z->p == x->p) {
            //不应该到这里
            luka_free(luka, z);
            return;
        }
    }
    z->parent = y;
    if (y != tree->nil) {
        if(z->p > y->p)
            y->right = z;
        else if (z->p<y->p)
            y->left = z;
    } else {
        tree->root = z; 
    }
    z->left = tree->nil;
    z->right = tree->nil;
    rbtreev_insert_fixup(tree, z);
}

static void rbtreev_delete_fixup (RBTreeV *tree, RBTreeVNode *x) {
    RBTreeVNode *w = NULL;

    while ((x != tree->root) && (x->color == BLACK)) {
        if (x == x->parent->left) {
            w = x->parent->right;
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                rbtreev_left_rotate(tree, x->parent);
                w = x->parent->right;
            }
            if (w->left->color == BLACK && w->right->color == BLACK) {
                w->color = RED;
                x = x->parent;
            } else {
                if(w->right->color == BLACK) {
                    w->left->color = BLACK;
                    w->color = RED;
                    rbtreev_right_rotate(tree, w);
                    w = x->parent->right;
                } 
                w->color = w->parent->color;
                x->parent->color = BLACK;
                w->right->color = BLACK;
                rbtreev_left_rotate(tree, x->parent);
                x = tree->root;
            }
        } else {
            w = x->parent->left;
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                rbtreev_right_rotate(tree, x->parent);
                w = x->parent->left;
            }
            if (w->right->color == BLACK && w->left->color == BLACK) {
                w->color = RED;
                x = x->parent;
            } else {
                if (w->left->color == BLACK) {
                    w->right->color = BLACK;
                    w->color = RED;
                    rbtreev_left_rotate(tree, w);
                    w = x->parent->left;
                }
                w->color = w->parent->color;
                x->parent->color = BLACK;
                w->left->color = BLACK;
                rbtreev_right_rotate(tree, x->parent);
                x = tree->root;
            }
        }
    }
    x->color = BLACK;
}

static void rbtreev_delete (Luka *luka, RBTreeV *tree, RBTreeVNode *z) {
    RBTreeVNode *y = NULL, *x = NULL;

    if (z->left == tree->nil || z->right == tree->nil) y=z;
    else {
        y = z->right;
        while (y->left != tree->nil) y = y->left;
    }
    if (y->left != tree->nil) x = y->left;
    else x = y->right;
    x->parent = y->parent;
    if (y->parent == tree->nil)
        tree->root = x;
    else {
        if (y->parent->left == y) y->parent->left = x;
        else y->parent->right = x;
    }
    if (y != z) z->p = y->p;
    if (y->color == BLACK) rbtreev_delete_fixup(tree, x);
    luka_free(luka, y);
}

void rbtreev_put (Luka *luka, RBTreeV *tree, void *p) {
    rbtreev_insert(luka, tree, p);
}

int rbtreev_exist (Luka *luka, RBTreeV *tree, void *p) {
    RBTreeVNode *node = rbtreev_search(tree, p);
    return node ? 1 : 0;
}

void rbtreev_rmv (Luka *luka, RBTreeV *tree, void *p) {
    RBTreeVNode *node = rbtreev_search(tree, p);
    if (node) rbtreev_delete(luka, tree, node);
}

static void rbtreev_destroy_ex (Luka *luka, RBTreeV *tree, RBTreeVNode *node) {
    if (node == tree->nil)
        return;
    
    rbtreev_destroy_ex(luka, tree, node->left);
    rbtreev_destroy_ex(luka, tree, node->right);
    luka_free(luka, node);
}

void rbtreev_destroy (Luka *luka, RBTreeV *tree) {
    rbtreev_destroy_ex(luka, tree, tree->root);
    luka_free(luka, tree->nil);
    luka_free(luka, tree);
}


static void rbtreev_get_fdata_ex (Luka *luka, RBTreeV *tree, RBTreeVNode *node, voidp *p) {
    if (node == tree->nil || *p != NULL)
        return;

	if (luka_data_index(luka, node->p) == 0) {
		*p = node->p;
		return;
	}

    rbtreev_get_fdata_ex(luka, tree, node->left, p);
    rbtreev_get_fdata_ex(luka, tree, node->right, p);
}

/** 获得一个无效的数据 **/
voidp rbtreev_get_fdata (Luka *luka, RBTreeV *tree) {
	voidp p = NULL;
	rbtreev_get_fdata_ex(luka, tree, tree->root, &p);
	return p;
}

static void rbtreev_get_fdata2_ex (Luka *luka, RBTreeV *tree, RBTreeVNode *node, void (*func_p)(Luka *, voidp p)) {
    if (node == tree->nil)
        return;

    func_p(luka, node->p);
	rbtreev_get_fdata2_ex(luka, tree, node->left, func_p);
	rbtreev_get_fdata2_ex(luka, tree, node->right, func_p);
    luka_free(luka, node);
}

/** 获得所有数据 **/
void rbtreev_get_fdata2 (Luka *luka, RBTreeV *tree, void (*func_p)(Luka *, voidp p)) {
	rbtreev_get_fdata2_ex(luka, tree, tree->root, func_p);
	tree->root = tree->nil;
}

// +--------------------------------------------------
// | 红黑树(char *)
// +--------------------------------------------------

typedef struct RBTreeCNode {
    enum RB_COLOR color;
    char *p;
    void *value;
    struct RBTreeCNode *left, *right, *parent;
} RBTreeCNode;

struct RBTreeC {
    RBTreeCNode *root, *nil;
    void (*proc)(Luka *luka, void *value);
};

RBTreeC *rbtreec_create (Luka *luka, void (*proc)(Luka *, void *)) {
    RBTreeC *tree = (RBTreeC *)luka_alloc(luka, sizeof(RBTreeC));
    tree->proc = proc;

    tree->nil = (RBTreeCNode *)luka_alloc(luka, sizeof(RBTreeCNode));
    tree->nil->color = BLACK;
    tree->nil->left = NULL;
    tree->nil->right = NULL;
    tree->nil->parent = NULL;
    tree->root = tree->nil;
    return tree;
}

static RBTreeCNode *rbtreec_search (RBTreeC *tree, const char *s) {
    int ret = 0;
    RBTreeCNode *x = tree->root;
    while (x != tree->nil) {
        ret = strcmp(s, x->p);
        if (ret > 0) x = x->right;
        else if (ret < 0) x = x->left;
        else return x;
    }
    return NULL;
}

static void rbtreec_left_rotate (RBTreeC *tree, RBTreeCNode *x) {
    RBTreeCNode *y = x->right;
    x->right = y->left;

    if (y->left != tree->nil)
        y->left->parent = x;
    if (x != tree->nil)
        y->parent = x->parent;
    if(x->parent == tree->nil) 
        tree->root = y; 
    else {
        if (x->parent->left == x)
            x->parent->left = y;
        else
            x->parent->right = y;
    }
    y->left = x;
    if (x != tree->nil)
        x->parent = y;
}

static void rbtreec_right_rotate (RBTreeC *tree, RBTreeCNode *x) {
    RBTreeCNode *y = x->left;
    x->left = y->right;

    if (y->right != tree->nil)
        y->right->parent = x;
    if (x != tree->nil)
        y->parent = x->parent;
    if (x->parent == tree->nil)
        tree->root = y;
    else {
        if(x->parent->left == x)
            x->parent->left = y;
        else
            x->parent->right = y;
    }
    y->right = x;
    if(x != tree->nil)
        x->parent = y;
}

static void rbtreec_insert_fixup (RBTreeC *tree, RBTreeCNode *z) {
    RBTreeCNode *y = NULL;
    while ((z->parent != NULL) && (z->parent->color == RED)) {
        if (z->parent == z->parent->parent->left) { 
            y = z->parent->parent->right; 
            if ((y != NULL) && (y->color == RED)) {
                z->parent->color = BLACK; 
                y->color = BLACK;
                z->parent->parent->color = RED; 
                z = z->parent->parent; 
            } else {
                if (z == z->parent->right) { 
                    z = z->parent;
                    rbtreec_left_rotate(tree, z);
                }
                z->parent->color = BLACK; 
                z->parent->parent->color = RED;
                rbtreec_right_rotate(tree, z->parent->parent);
            }        
        } else { 
            y = z->parent->parent->left;
            if ((y != NULL) && (y->color == RED)) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rbtreec_right_rotate(tree, z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                rbtreec_left_rotate(tree, z->parent->parent);
            }
        }
    }
    tree->root->color = BLACK;
    tree->nil->color = BLACK;
}

static void rbtreec_insert (Luka *luka, RBTreeC *tree, const char *s, void *value) {
    int ret = 0;
    RBTreeCNode *x = tree->root, *y = tree->nil, *z;

    z = (RBTreeCNode *)luka_alloc(luka, sizeof(RBTreeCNode));
    z->color = RED;
    z->p = luka_strdup(luka, s);
    z->value = value;
    z->parent = NULL;
    z->left = NULL;
    z->right = NULL;
    while (x != tree->nil) {
        y = x;
        ret = strcmp(z->p, x->p);
        if (ret < 0)
            x = x->left;
        else if (ret > 0)
            x = x->right;
        else {
            if (tree->proc)
                tree->proc(luka, x->value);
            luka_free(luka, z->p);
            luka_free(luka, z);
            x->value = value;
            return;
        }
    }
    z->parent = y;
    if (y != tree->nil) {
        ret = strcmp(z->p, y->p);
        if(ret > 0)
            y->right = z;
        else
            y->left = z;
    } else {
        tree->root = z; 
    }
    z->left = tree->nil;
    z->right = tree->nil;
    rbtreec_insert_fixup(tree, z);
}

static void rbtreec_delete_fixup (RBTreeC *tree, RBTreeCNode *x) {
    RBTreeCNode *w = NULL;

    while ((x != tree->root) && (x->color == BLACK)) {
        if (x == x->parent->left) {
            w = x->parent->right;
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                rbtreec_left_rotate(tree, x->parent);
                w = x->parent->right;
            }
            if (w->left->color == BLACK && w->right->color == BLACK) {
                w->color = RED;
                x = x->parent;
            } else {
                if(w->right->color == BLACK) {
                    w->left->color = BLACK;
                    w->color = RED;
                    rbtreec_right_rotate(tree, w);
                    w = x->parent->right;
                } 
                w->color = w->parent->color;
                x->parent->color = BLACK;
                w->right->color = BLACK;
                rbtreec_left_rotate(tree, x->parent);
                x = tree->root;
            }
        } else {
            w = x->parent->left;
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                rbtreec_right_rotate(tree, x->parent);
                w = x->parent->left;
            }
            if (w->right->color == BLACK && w->left->color == BLACK) {
                w->color = RED;
                x = x->parent;
            } else {
                if (w->left->color == BLACK) {
                    w->right->color = BLACK;
                    w->color = RED;
                    rbtreec_left_rotate(tree, w);
                    w = x->parent->left;
                }
                w->color = w->parent->color;
                x->parent->color = BLACK;
                w->left->color = BLACK;
                rbtreec_right_rotate(tree, x->parent);
                x = tree->root;
            }
        }
    }
    x->color = BLACK;
}

static void rbtreec_delete (Luka *luka, RBTreeC *tree, RBTreeCNode *z) {
    RBTreeCNode *y = NULL, *x = NULL;

    luka_free(luka, z->p);
    if (z->left == tree->nil || z->right == tree->nil) y=z;
    else {
        y = z->right;
        while (y->left != tree->nil) y = y->left;
    }
    if (y->left != tree->nil) x = y->left;
    else x = y->right;
    x->parent = y->parent;
    if (y->parent == tree->nil)
        tree->root = x;
    else {
        if (y->parent->left == y) y->parent->left = x;
        else y->parent->right = x;
    }
    if (y != z) z->p = y->p;
    if (y->color == BLACK) rbtreec_delete_fixup(tree, x);
    luka_free(luka, y);
}

void rbtreec_put (Luka *luka, RBTreeC *tree, const char *s, void *value) {
    rbtreec_insert(luka, tree, s, value);
}

void *rbtreec_get (Luka *luka, RBTreeC *tree, const char *s) {
    RBTreeCNode *node = rbtreec_search(tree, s);
    return node ? node->value : NULL;
}

void rbtreec_rmv (Luka *luka, RBTreeC *tree, const char *s) {
    RBTreeCNode *node = rbtreec_search(tree, s);
    if (node) rbtreec_delete(luka, tree, node);
}

static void rbtreec_each_ex (Luka *luka, RBTreeC *tree, RBTreeCNode *node, void *k, void (*cb)(Luka *, void *k, const char *p, void *value)) {
    if (node == tree->nil)
        return;

    rbtreec_each_ex(luka, tree, node->left, k, cb);
    cb(luka, k, node->p, node->value);
    rbtreec_each_ex(luka, tree, node->right, k, cb);
}

void rbtreec_each (Luka *luka, RBTreeC *tree, void *k, void (*cb)(Luka *, void *k, const char *p, void *value)) {
    rbtreec_each_ex(luka, tree, tree->root, k, cb);
}

static void rbtreec_destroy_ex (Luka *luka, RBTreeC *tree, RBTreeCNode *node) {
    if (node == tree->nil)
        return;

    if (tree->proc)
        tree->proc(luka, node->value);
    luka_free(luka, node->p);
    
    rbtreec_destroy_ex(luka, tree, node->left);
    rbtreec_destroy_ex(luka, tree, node->right);
    luka_free(luka, node);
}

void rbtreec_destroy (Luka *luka, RBTreeC *tree) {
    rbtreec_destroy_ex(luka, tree, tree->root);
    luka_free(luka, tree->nil);
    luka_free(luka, tree);
}

// +--------------------------------------------------
// | StrList
// +--------------------------------------------------

int sl_push (Luka *luka, StrList **sl, const char *s) {
	StrList *sl_node = NULL, *mov = NULL;
	size_t len = 0;

	if (!sl || !s || *s == 0)
		return 0;

	len = strlen(s);
	sl_node = (StrList *)luka_alloc(luka, sizeof(StrList));
	if (!sl_node)
		return 0;
	memset(sl_node, 0, sizeof(StrList));

	sl_node->s = (char *)luka_alloc(luka, len + 2);
	if (!sl_node->s) {
		free(sl_node);
		return 0;
	}
	strncpy(sl_node->s, s, len + 1);

	if (!(*sl)) {
		*sl = sl_node;
	} else {
		mov = *sl;
		while (mov->next) {
			mov = mov->next;
		}
		mov->next = sl_node;
	}
	return 1;
}

int sl_push2 (Luka *luka, StrList **sl, char *s) {
	StrList *sl_node = NULL, *mov = NULL;

	if (!sl || !s || *s == 0)
		return 0;

	sl_node = (StrList *)luka_alloc(luka, sizeof(StrList));
	if (!sl_node)
		return 0;
	sl_node->next = NULL;
	sl_node->s = s;

	if (!(*sl)) {
		*sl = sl_node;
	} else {
		mov = *sl;
		while (mov->next) {
			mov = mov->next;
		}
		mov->next = sl_node;
	}
	return 1;
}

int sb_exist (Luka *luka, StrList *sl, const char *s) {
	if (!sl || !s || *s == 0)
		return 0;

	while (sl) {
		if (strcmp(sl->s, s) == 0)
			return 1;
		sl = sl->next;
	}
	return 0;
}

void sl_free (Luka *luka, StrList *sl) {
	StrList *mov = NULL, *buf = NULL;

	if (!sl)
		return;

	mov = sl;
	while (mov) {
		buf = mov;
		mov = mov->next;
		luka_free(luka, buf->s);
		luka_free(luka, buf);
	}
}

// +--------------------------------------------------
// | LukaObject
// +--------------------------------------------------

struct LukaObject {
	RBTreeC *object;

	void (*in_proc)(Luka *luka, voidp p);
	void (*out_proc)(Luka *luka, voidp p);
};

LukaObject *luka_object_create (Luka *luka) {
	LukaObject *obj = NULL;

	obj = (LukaObject *)luka_alloc(luka, sizeof(LukaObject));
	obj->object = rbtreec_create(luka, NULL);
	return obj;
}

void luka_object_setcb (Luka *luka, LukaObject *object, void (*in_proc)(Luka *luka, voidp p), void (*out_proc)(Luka *luka, voidp p)) {
	object->in_proc = in_proc;
	object->out_proc = out_proc;
}

void luka_object_put (Luka *luka, LukaObject *object, const char *key, voidp p) {
	voidp data = NULL;

	if (!key || *key == 0)
		return;

	if (object->out_proc) {
		data = luka_object_get(luka, object, key);
		if (data)
			object->out_proc(luka, data);
	}

	rbtreec_put(luka, object->object, key, p);

	if (object->in_proc) {
		object->in_proc(luka, p);
	}
}

voidp luka_object_get (Luka *luka, LukaObject *object, const char *key) {
	if (!key || *key == 0)
		return NULL;

	return rbtreec_get(luka, object->object, key);
}

void luka_object_rmv (Luka *luka, LukaObject *object, const char *key) {
	voidp data = NULL;

	if (!key || *key == 0)
		return;

	data = luka_object_get(luka, object, key);
	if (!data)
		return;

	rbtreec_rmv(luka, object->object, key);

	if (object->out_proc) {
		object->out_proc(luka, data);
	}
}

void luka_object_each_proc (Luka *luka, void *k, const char *p, void *value) {
	StrList **sl = (StrList **)k;
	sl_push(luka, sl, p);
}

StrList *luka_object_each (Luka *luka, LukaObject *object) {
	StrList *sl = NULL;
	rbtreec_each(luka, object->object, (void **)&sl, luka_object_each_proc);
	return sl;
}

void luka_object_destroy_proc (Luka *luka, void *k, const char *p, void *value) {
	LukaObject *object = (LukaObject *)k;

	if (object->out_proc) {
		object->out_proc(luka, value);
	}
}

void luka_object_destroy (Luka *luka, LukaObject *object) {
	rbtreec_each(luka, object->object, object, luka_object_destroy_proc);
	rbtreec_destroy(luka, object->object);
	luka_free(luka, object);
}

// +--------------------------------------------------
// | LukaArray
// +--------------------------------------------------

struct LukaArray {
	RBTreeC *array;
	size_t length;

	void (*in_proc)(Luka *luka, voidp p);
	void (*out_proc)(Luka *luka, voidp p);
};

LukaArray *luka_array_create (Luka *luka) {
	LukaArray *array = NULL;

	array = (LukaArray *)luka_alloc(luka, sizeof(LukaArray));
	array->array = rbtreec_create(luka, NULL);
	return array;
}

void luka_array_setcb (Luka *luka, LukaArray *array, void (*in_proc)(Luka *luka, voidp p), void (*out_proc)(Luka *luka, voidp p)) {
	array->in_proc = in_proc;
	array->out_proc = out_proc;
}

void luka_array_push (Luka *luka, LukaArray *array, voidp p) {
	char temp[12] = {0};

	if (array->length == LUKA_MAX)
		return;

	sprintf(temp, "%d", array->length);
	rbtreec_put(luka, array->array, temp, p);

	if (array->in_proc) {
		array->in_proc(luka, p);
	}

	array->length++;
}

void luka_array_put (Luka *luka, LukaArray *array, size_t i, voidp p) {
	size_t j = 0;
	voidp pold = NULL;

	if (i >= LUKA_MAX)
		return;

	if ((pold = luka_array_get(luka, array, i)) != NULL) {
        char temp[12] = {0};
        sprintf(temp, "%d", i);
        rbtreec_put(luka, array->array, temp, p);
        if (array->out_proc)
		  array->out_proc(luka, pold);
        return;
	}

	for (j = array->length; j < i; j++) {
		luka_array_push(luka, array, luka_null(luka));
	}

	luka_array_push(luka, array, p);
}

size_t luka_array_length (Luka *luka, LukaArray *array) {
	return array->length;
}

voidp luka_array_get (Luka *luka, LukaArray *array, size_t i) {
	char temp[12] = {0};
	void *p = NULL;

	if (i >= array->length)
		return NULL;

	sprintf(temp, "%d", i);
	p = rbtreec_get(luka, array->array, temp);
	return p;
}

void luka_array_destroy_proc (Luka *luka, void *k, const char *p, void *value) {
	LukaArray *array = (LukaArray *)k;

	if (array->out_proc) {
		array->out_proc(luka, value);
	}
}

void luka_array_destroy (Luka *luka, LukaArray *array) {
	rbtreec_each(luka, array->array, array, luka_array_destroy_proc);
	rbtreec_destroy(luka, array->array);
	luka_free(luka, array);
}

// +--------------------------------------------------
// | LukaStack
// +--------------------------------------------------

typedef struct LukaStackNode {
    void *p;
    struct LukaStackNode *next;
} LukaStackNode;

struct LukaStack {
    LukaStackNode *head;
};

LukaStack *luka_stack_create (Luka *luka) {
    return (LukaStack *)luka_alloc(luka, sizeof(LukaStack));
}

void luka_stack_push (Luka *luka, LukaStack *stack, void *p) {
    LukaStackNode *node = (LukaStackNode *)luka_alloc(luka, sizeof(LukaStackNode));
    node->p = p;
    node->next = stack->head;
    stack->head = node;
}

void *luka_stack_top (Luka *luka, LukaStack *stack) {
    return stack->head ? stack->head->p : NULL;
}

void luka_stack_pop (Luka *luka, LukaStack *stack) {
    LukaStackNode *node = stack->head;
    stack->head = node->next;
    luka_free(luka, node);
}

void luka_stack_destroy (Luka *luka, LukaStack *stack) {
    LukaStackNode *mov = NULL, *buf = NULL;

    mov = stack->head;
    while (mov) {
        buf = mov;
        mov = mov->next;
        luka_free(luka, buf);
    }
    luka_free(luka, stack);
}

