/* tree.h -- AVL trees (in the spirit of BSD's 'queue.h')	-*- C -*-	*/

/* Copyright (c) 2005 Ian Piumarta
 * Copyright (c) 2009 Stephan Beal
 * 
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the 'Software'), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, and/or sell copies of the
 * Software, and to permit persons to whom the Software is furnished to do so,
 * provided that the above copyright notice(s) and this permission notice appear
 * in all copies of the Software and that both the above copyright notice(s) and
 * this permission notice appear in supporting documentation.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS'.  USE ENTIRELY AT YOUR OWN RISK.
 */

/* This file defines an AVL balanced binary tree [Georgii M. Adelson-Velskii and
 * Evgenii M. Landis, 'An algorithm for the organization of information',
 * Doklady Akademii Nauk SSSR, 146:263-266, 1962 (Russian).  Also in Myron
 * J. Ricci (trans.), Soviet Math, 3:1259-1263, 1962 (English)].
 * 
 * An AVL tree is headed by pointers to the root node and to a function defining
 * the ordering relation between nodes.  Each node contains an arbitrary payload
 * plus three fields per tree entry: the depth of the subtree for which it forms
 * the root and two pointers to child nodes (singly-linked for minimum space, at
 * the expense of direct access to the parent node given a pointer to one of the
 * children).  The tree is rebalanced after every insertion or removal.  The
 * tree may be traversed in two directions: forward (in-order left-to-right) and
 * reverse (in-order, right-to-left).
 * 
 * Because of the recursive nature of many of the operations on trees it is
 * necessary to define a number of helper functions for each type of tree node.
 * The macro AVL_DEFINE(node_tag, entry_name) defines these functions with
 * unique names according to the node_tag.  This macro should be invoked,
 * thereby defining the necessary functions, once per node tag in the program.
 * 
 * For details on the use of these macros, see the tree(3) manual page.
 *
 * The orignal source was gleened from http://piumarta.com/software/tree/
 * and hacked/cleaned up slightly by Stephan Beal in June 2009.
 */

#ifndef _piumarta_com_software_tree__tree_h_included
#define _piumarta_com_software_tree__tree_h_included

enum { AVL_DELTA_MAX = 1 };

#define AVL_ENTRY_STRUCT(type)			\
  struct {					\
    struct type	*avl_left;			\
    struct type	*avl_right;			\
    unsigned short	 avl_height;            \
  }
#define AVL_ENTRY_init_m \
    { NULL, NULL, 0 }
#define AVL_HEAD(name, type)				\
  struct name {						\
    struct type *th_root;				\
    int  (*th_cmp)(struct type const *lhs, struct type const *rhs);	\
  }
#define AVL_HEAD_INITIALIZER(cmp) { 0, cmp }

#define AVL_DELTA(self, field)								\
  (( (((self)->field.avl_left) ? (self)->field.avl_left->field.avl_height  : 0))	\
   - (((self)->field.avl_right) ? (self)->field.avl_right->field.avl_height : 0))


#define AVL_BALANCE(node,field) AVL_BALANCE_##node##_##field
#define AVL_ROTL(node,field) AVL_ROTL_##node##_##field
#define AVL_ROTR(node,field) AVL_ROTR_##node##_##field
#define AVL_INSERT_M(node,field) AVL_INSERT_##node##_##field
#define AVL_REMOVE_M(node,field) AVL_REMOVE_##node##_##field
#define AVL_FIND_M(node,field) AVL_FIND_##node##_##field
#define AVL_APPLY_FORWARD_M(node,field) AVL_APPLY_FORWARD_ALL_##node##_##field
#define AVL_APPLY_REVERSE_M(node,field) AVL_APPLY_REVERSE_ALL_##node##_##field
#define AVL_COMPARE_SIGNATURE(node) int (*compare)(struct node const *lhs, struct node const *rhs)
#define AVL_MOVE_RIGHT(node) AVL_MOVE_RIGHT_##node
#define AVL_FREE_NODES_M(node,field) AVL_FREE_NODES_##node##_##field

/* Recursion prevents the following from being defined as macros. */
/** Declares public API functions for an AVL with the given node/field parameters. */
#define AVL_DECLARE(NoDe,FiElD) \
    struct NoDe *AVL_INSERT_M(NoDe,FiElD)                               \
        (struct NoDe *self, struct NoDe *elm, AVL_COMPARE_SIGNATURE(NoDe)); \
    struct NoDe *AVL_FIND_M(NoDe,FiElD)                                 \
        (struct NoDe *self, struct NoDe *elm, AVL_COMPARE_SIGNATURE(NoDe)); \
    struct NoDe * AVL_MOVE_RIGHT(NoDe)( struct NoDe *self, struct NoDe *rhs); \
    struct NoDe *AVL_REMOVE_M(NoDe,FiElD)                               \
        (struct NoDe *self, struct NoDe *elm, AVL_COMPARE_SIGNATURE(NoDe)); \
    void AVL_APPLY_FORWARD_M(NoDe,FiElD)                                \
        (struct NoDe *self, void (*function)(struct NoDe *self, void *data), void *data); \
    void AVL_APPLY_REVERSE_M(NoDe,FiElD)                                \
        (struct NoDe *self, void (*function)(struct NoDe *self, void *data), void *data); \
    void AVL_FREE_NODES_M(NoDe,FiElD) \
        (struct NoDe *self, void (*dtor)(struct NoDe *))

/** Implement AVL tree API for the given node/field combination. */
#define AVL_DEFINE(node, field)                                        \
    AVL_DECLARE(node,field);                                           \
    static struct node *AVL_ROTL(node,field)(struct node *self);        \
    static struct node *AVL_ROTR(node,field)(struct node *self);        \
    static struct node *AVL_BALANCE(node,field)(struct node *self)      \
    {                                                                   \
        int delta= AVL_DELTA(self, field);                             \
                                                                        \
        if (delta < -AVL_DELTA_MAX)                                    \
        {                                                               \
            if (AVL_DELTA(self->field.avl_right, field) > 0)           \
                self->field.avl_right= AVL_ROTR(node,field)(self->field.avl_right); \
            return AVL_ROTL(node,field)(self);                         \
        }                                                               \
        else if (delta > AVL_DELTA_MAX)                                \
        {                                                               \
            if (AVL_DELTA(self->field.avl_left, field) < 0)            \
                self->field.avl_left= AVL_ROTL(node,field)(self->field.avl_left); \
            return AVL_ROTR(node,field)(self);                         \
        }                                                               \
        self->field.avl_height= 0;                                      \
        if (self->field.avl_left && (self->field.avl_left->field.avl_height > self->field.avl_height)) \
            self->field.avl_height= self->field.avl_left->field.avl_height; \
        if (self->field.avl_right && (self->field.avl_right->field.avl_height > self->field.avl_height)) \
            self->field.avl_height= self->field.avl_right->field.avl_height; \
        self->field.avl_height += 1;                                    \
        return self;                                                    \
    }                                                                   \
                                                                        \
    static struct node *AVL_ROTL(node,field)(struct node *self)         \
    {                                                                   \
        struct node *r= self->field.avl_right;                          \
        self->field.avl_right= r->field.avl_left;                       \
        r->field.avl_left= AVL_BALANCE(node,field)(self);              \
        return AVL_BALANCE(node,field)(r);                             \
    }                                                                   \
                                                                        \
    static struct node *AVL_ROTR(node,field)(struct node *self)         \
    {                                                                   \
        struct node *l= self->field.avl_left;                           \
        self->field.avl_left= l->field.avl_right;                       \
        l->field.avl_right= AVL_BALANCE(node,field)(self);             \
        return AVL_BALANCE(node,field)(l);                             \
    }                                                                   \
                                                                        \
    struct node *AVL_INSERT_M(node,field)                              \
        (struct node *self, struct node *elm, AVL_COMPARE_SIGNATURE(node)) \
    {                                                                   \
        if (!self)                                                      \
            return elm;                                                 \
        if (compare(elm, self) < 0)                                     \
            self->field.avl_left= AVL_INSERT_M(node,field)(self->field.avl_left, elm, compare); \
        else                                                            \
            self->field.avl_right= AVL_INSERT_M(node,field)(self->field.avl_right, elm, compare); \
        return AVL_BALANCE(node,field)(self);                          \
    }                                                                   \
                                                                        \
    struct node *AVL_FIND_M(node,field)                             \
    (struct node *self, struct node *elm, AVL_COMPARE_SIGNATURE(node)) \
    {                                                                   \
        if (!self)                                                      \
            return 0;                                                   \
        const int cmp = (self==elm) ? 0 : compare(elm, self);           \
        if (0 == cmp)                                                   \
            return self;                                                \
        if (0 > cmp)                                     \
            return AVL_FIND_M(node,field)(self->field.avl_left, elm, compare); \
        else                                                            \
            return AVL_FIND_M(node,field)(self->field.avl_right, elm, compare); \
    }                                                                   \
													\
    struct node * AVL_MOVE_RIGHT(node)(struct node *self, struct node *rhs) \
    {                                                                   \
        if (!self)                                                      \
            return rhs;                                                 \
        self->field.avl_right= AVL_MOVE_RIGHT(node)(self->field.avl_right, rhs); \
        return AVL_BALANCE(node,field)(self);                          \
    }                                                                   \
													\
    struct node *AVL_REMOVE_M(node,field)                           \
    (struct node *self, struct node *elm, AVL_COMPARE_SIGNATURE(node)) \
    {                                                                   \
        if (!self) return 0;                                            \
        const int cmp = (self==elm) ? 0 : compare(elm, self);           \
        if (0 == cmp)                                \
        {                                                               \
            struct node *tmp= AVL_MOVE_RIGHT(node)(self->field.avl_left, self->field.avl_right); \
            self->field.avl_left= 0;                                    \
            self->field.avl_right= 0;                                   \
            return tmp;                                                 \
        }                                                               \
        else if(0 > cmp) {                                            \
            self->field.avl_left = AVL_REMOVE_M(node,field)(self->field.avl_left, elm, compare); \
        } \
        else {                                                          \
            self->field.avl_right= AVL_REMOVE_M(node,field)(self->field.avl_right, elm, compare); \
        } \
        return self;/*AVL_BALANCE(node,field)(self);*/                  \
    }                                                                   \
                                                                        \
    void AVL_APPLY_FORWARD_M(node,field)                                \
        (struct node *self, void (*function)(struct node *n, void *data), void *data) \
    {                                                                   \
        if (self)                                                       \
        {                                                               \
            AVL_APPLY_FORWARD_M(node,field)(self->field.avl_left, function, data); \
            function(self, data);                                       \
            AVL_APPLY_FORWARD_M(node,field)(self->field.avl_right, function, data); \
        }                                                               \
    }                                                                   \
                                                                        \
  void AVL_APPLY_REVERSE_M(node,field)								\
    (struct node *self, void (*function)(struct node * n, void *data), void *data)			\
  {													\
    if (self)												\
      {													\
	AVL_APPLY_REVERSE_M(node,field)(self->field.avl_right, function, data);			\
	function(self, data);										\
	AVL_APPLY_REVERSE_M(node,field)(self->field.avl_left, function, data);			\
      }													\
  } \
    /** removes and deallocates all entries starting at node, calling dtor(item) for each one.*/ \
    void AVL_FREE_NODES_M(node,field)                           \
        (struct node *node, void (*dtor)(struct node *node))            \
    {                                                                   \
        if (! node) return;                                             \
        AVL_FREE_NODES_M(node,field)(node->field.avl_left, dtor);          \
        node->field.avl_left = 0;                                       \
        AVL_FREE_NODES_M(node,field)(node->field.avl_right, dtor);         \
        node->field.avl_right = 0;                                      \
	if( NULL != dtor ) dtor(node);                                   \
    }


#define AVL_INSERT(head, node, field, elm)						\
  ((head)->th_root= AVL_INSERT_M(node,field)((head)->th_root, (elm), (head)->th_cmp))

#define AVL_FIND(head, node, field, elm)				\
    (AVL_FIND_M(node,field)((head)->th_root, (elm), (head)->th_cmp))

#define AVL_REMOVE(head, node, field, elm)						\
  ((head)->th_root= AVL_REMOVE_M(node,field)((head)->th_root, (elm), (head)->th_cmp))

#define AVL_DEPTH(head, field)			\
  ((head)->th_root->field.avl_height)

#define AVL_APPLY_FORWARD(head, node, field, function, data)                   \
  AVL_APPLY_FORWARD_M(node,field)((head)->th_root, function, data)

#define AVL_APPLY_REVERSE(head, node, field, function, data)	\
  AVL_APPLY_REVERSE_M(node,field)((head)->th_root, function, data)

#define AVL_FREE_NODES(head, node, field, dtor)             \
    AVL_FREE_NODES_M(node,field)((head)->th_root, dtor); (head)->th_root=0

#define AVL_HEAD_INIT(head, cmp) while(head) {       \
        (head)->th_root= 0;                     \
        (head)->th_cmp= (cmp);			\
        break; \
    } 


#endif /* _piumarta_com_software_tree__tree_h_included */
