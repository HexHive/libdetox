/**
 * This file contains an implementation of a red-black tree for non-overlapping
 * memory regions.
 * A query in the tree will return the memory region that contains the query
 * address.
 * The rb_clear() function assumes that the memory for the nodes has been
 * allocated using malloc() and calls free() to release it.
 */
#include "fbt_rbtree.h"
#include "fbt_debug.h"
#include "fbt_libc.h"

#ifndef NULL
    #define NULL 0
#endif


/* private function prototypes */
static struct rb_node *rb_insert_case1(struct rb_node *root,
                                        struct rb_node *node)
                                        __attribute__ ((warn_unused_result));
static struct rb_node *rb_insert_case3(struct rb_node *root,
                                        struct rb_node *node)
                                        __attribute__ ((warn_unused_result));
static struct rb_node *rb_insert_case4(struct rb_node *root,
                                        struct rb_node *node);
static struct rb_node *rb_insert_case5(struct rb_node *root,
                                        struct rb_node *node)
                                        __attribute__ ((warn_unused_result));

static struct rb_node *rb_rotate_left(struct rb_node *root,
                                       struct rb_node *node)
                                       __attribute__ ((warn_unused_result));
static struct rb_node *rb_rotate_right(struct rb_node *root,
                                        struct rb_node *node)
                                        __attribute__ ((warn_unused_result));

static struct rb_node *rb_insert_recursive(struct rb_node *root,
                                           struct rb_node *tree,
                                           struct rb_node *node)
                                           __attribute__ ((warn_unused_result));

static struct rb_node *rb_grandparent(struct rb_node *node);
static struct rb_node *rb_uncle(struct rb_node* node);


/* public functions */

/**
 * find the node that contains the query address addr.
 * This function searches a red-black tree defined by its root element for
 * a node that contains the query address, which means that the query address is
 * between addr_begin and addr_end of the node.
 * If such a node is found in the tree, a pointer to it is returned. Otherwise,
 * the function returns NULL.
 * @param root the root of the tree to search
 * @param addr the query address
 * @return the node that contains addr, or NULL if no such node is found
 */
struct rb_node *rb_query(struct rb_node *root, rb_key addr)
{
    struct rb_node *node = root;
    while (NULL != node) {
        if (addr < node->addr_begin) {
            node = node->left;
        } else if (addr >= node->addr_end) {
            node = node->right;
        } else {
            /* addr is between addr_begin and addr_end of this node */
            break;
        }
    }
    return node;
}

/**
 * insert a node into a red-black-tree.
 * Inserts the supplied node into the tree defined by the given root element.
 * To insert into an empty tree (that is, create a new one), supply NULL as
 * the root argument.
 * Because the insertion could rotate the current root element away from the
 * root position, this function returns a pointer to the new root element after
 * the insertion.
 * Not using the return value of this function produces a compile warning,
 * because the calling function should save the new root.
 * @param root the current root of the tree
 * @param node the new node to insert into the tree
 * @return the new root element
 */
struct rb_node *rb_insert(struct rb_node *root, struct rb_node* node)
{
    // printf("inserting node %p with addr_begin=%p, name=%s\n", (void*) node, (void*) node->addr_begin, node->section_name);
    node->right = NULL;
    node->left = NULL;
    if (NULL == root) {
        node->color = COLOR_BLACK;
        node->parent = NULL;
        return node;    // insertion into an empty tree -> node is now root
    } else {
        node->color = COLOR_RED;
        return rb_insert_recursive(root, root, node);
    }
}

/**
 * delete the whole tree and release all its memory.
 * This function deletes a complete tree and releases the memory of all its
 * elements using free().
 * Do not use this function to delete only a part of a tree, as it does not
 * perform any balancing.
 * @param root the root of the tree to delete
 */
void rb_clear(struct rb_node *root)
{
    if (NULL != root) {
        rb_clear(root->left);
        rb_clear(root->right);
        free(root);
    }
}

/* private functions */

/**
 * recursive insertion function.
 * Inserts a node into a red-black tree given by the root of the tree and the
 * current subtree that this instance of the function is working on.
 * @param root the root of the tree
 * @param branch the current subtree or branch
 * @param node the node to insert
 * @return the new root of the tree
 */
static struct rb_node *rb_insert_recursive(struct rb_node *root,
                                            struct rb_node *branch,
                                            struct rb_node *node)
{
    if (node->addr_begin < branch->addr_begin) {
        if (NULL == branch->left) {
            node->parent = branch;
            branch->left = node;
            return rb_insert_case1(root, node);
        } else {
            return rb_insert_recursive(root, branch->left, node);
        }
    } else if (node->addr_begin > branch->addr_begin) {
        if (NULL == branch->right) {
            node->parent = branch;
            branch->right = node;
            return rb_insert_case1(root, node);
        } else {
            return rb_insert_recursive(root, branch->right, node);
        }
    } else {
        /* we should not be here! do not insert nodes with the same key_begin */
        fbt_suicide_str("Warning: tried to insert two nodes with the same"
                    " addr_begin key into a red-black tree. Ignored. (rb_insert_recursive: fbt_rbtree.c)\n");
    }
    return root;
}

static struct rb_node *rb_insert_case1(struct rb_node *root,
                                        struct rb_node *node)
{
    if (NULL == node->parent) {
        // node is the new root
        node->color = COLOR_BLACK;
        return node;
    } else if (COLOR_RED == node->parent->color) {
        return rb_insert_case3(root, node);
    }
    /* case 2 */
    return root;
}


static struct rb_node *rb_insert_case3(struct rb_node *root,
                                        struct rb_node *node)
{
    struct rb_node *u = rb_uncle(node);
    if ((NULL != u) && (COLOR_RED == u->color)) {
        u->color = COLOR_BLACK;
        node->parent->color = COLOR_BLACK;
        struct rb_node *gp = rb_grandparent(node);
        gp->color = COLOR_RED;
        return rb_insert_case1(root, gp);
    } else {
        return rb_insert_case4(root, node);
    }
}

static struct rb_node *rb_insert_case4(struct rb_node *root,
                                        struct rb_node *node)
{
    struct rb_node *gp = rb_grandparent(node);
    if ((node == node->parent->left) && (node->parent == gp->right)) {
        root = rb_rotate_right(root, node->parent);
        node = node->right;
    } else if ((node == node->parent->right) && (node->parent == gp->left)) {
        root = rb_rotate_left(root, node->parent);
        node = node->left;
    }
    return rb_insert_case5(root, node);
}

static struct rb_node *rb_insert_case5(struct rb_node *root,
                                        struct rb_node *node)
{
    struct rb_node *gp = rb_grandparent(node);
    node->parent->color = COLOR_BLACK;
    gp->color = COLOR_RED;

    if (node == node->parent->left) {
        /* here, also gp->left == node->parent holds */
        return rb_rotate_right(root, gp);
    } else {
        /* node->parent == gp->right and node->parent->right == node */
        return rb_rotate_left(root, gp);
    }
}



static struct rb_node *rb_rotate_left(struct rb_node *root,
                                       struct rb_node *node)
{
    assert(NULL != node->right);
    struct rb_node *parent, *rc;
    rc = node->right;
    parent = node->parent;
    rc->parent = parent;

    if (root == node) {
        root = rc;
    } else if (parent->right == node) {
        parent->right = rc;
    } else {
        parent->left = rc;
    }
    node->parent = rc;
    node->right = rc->left;
    if (NULL != rc->left) {
        rc->left->parent = node;
    }
    rc->left = node;
    return root;
}

static struct rb_node *rb_rotate_right(struct rb_node *root,
                                        struct rb_node *node)
{
    assert(NULL != node->left);
    struct rb_node *lc, *parent;
    lc = node->left;
    parent = node->parent;
    lc->parent = parent;
    if (root == node) {
        root = lc;
    } else if (parent->right == node) {
        parent->right = lc;
    } else {
        parent->left = lc;
    }
    node->parent = lc;
    node->left = lc->right;
    if (NULL != lc->right) {
        lc->right->parent = node;
    }
    lc->right = node;
    return root;
}

static struct rb_node *rb_grandparent(struct rb_node *node)
{
    if ((NULL != node) && (NULL != node->parent)) {
        return node->parent->parent;
    } else {
        return NULL;
    }
}

static struct rb_node *rb_uncle(struct rb_node* node)
{
    struct rb_node *gp = rb_grandparent(node);
    if (NULL == gp) {
        return NULL;
    }
    if (gp->right == node->parent) {
        return gp->left;
    } else {
        return gp->right;
    }
}
