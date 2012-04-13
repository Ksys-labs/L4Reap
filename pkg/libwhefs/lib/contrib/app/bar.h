# 1 "foo.h"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "foo.h"
# 1 "avl_tree.h" 1
# 49 "avl_tree.h"
enum
{ AVL_DELTA_MAX = 1 };
# 2 "foo.h" 2

struct whio_avl_node *AVL_INSERT_whio_avl_node_link(struct whio_avl_node
						    *self,
						    struct whio_avl_node *elm,
						    int (*compare) (struct
								    whio_avl_node
								    const
								    *lhs,
								    struct
								    whio_avl_node
								    const
								    *rhs));
struct whio_avl_node *AVL_FIND_whio_avl_node_link(struct whio_avl_node *self,
						  struct whio_avl_node *elm,
						  int (*compare) (struct
								  whio_avl_node
								  const *lhs,
								  struct
								  whio_avl_node
								  const
								  *rhs));
struct whio_avl_node *AVL_MOVE_RIGHT_whio_avl_node(struct whio_avl_node *self,
						   struct whio_avl_node *rhs);
struct whio_avl_node *AVL_REMOVE_whio_avl_node_link(struct whio_avl_node
						    *self,
						    struct whio_avl_node *elm,
						    int (*compare) (struct
								    whio_avl_node
								    const
								    *lhs,
								    struct
								    whio_avl_node
								    const
								    *rhs));
void AVL_APPLY_FORWARD_ALL_whio_avl_node_link(struct whio_avl_node *self,
					      void (*function) (struct
								whio_avl_node
								* self,
								void *data),
					      void *data);
void AVL_APPLY_REVERSE_ALL_whio_avl_node_link(struct whio_avl_node *self,
					      void (*function) (struct
								whio_avl_node
								* self,
								void *data),
					      void *data);
void AVL_FREE_NODES_whio_avl_node_link(struct whio_avl_node *self,
				       void (*dtor) (struct whio_avl_node *));
static struct whio_avl_node *AVL_ROTL_whio_avl_node_link(struct whio_avl_node
							 *self);
static struct whio_avl_node *AVL_ROTR_whio_avl_node_link(struct whio_avl_node
							 *self);
static struct whio_avl_node *
AVL_BALANCE_whio_avl_node_link(struct whio_avl_node *self)
{
  int delta =
    (((((self)->link.avl_left) ? (self)->link.avl_left->link.
       avl_height : 0)) -
     (((self)->link.avl_right) ? (self)->link.avl_right->link.
      avl_height : 0));
  if (delta < -AVL_DELTA_MAX)
  {
    if ((((((self->link.avl_right)->link.avl_left) ? (self->link.avl_right)->
	   link.avl_left->link.avl_height : 0)) -
	 (((self->link.avl_right)->link.avl_right) ? (self->link.avl_right)->
	  link.avl_right->link.avl_height : 0)) > 0)
      self->link.avl_right =
	AVL_ROTR_whio_avl_node_link(self->link.avl_right);
    return AVL_ROTL_whio_avl_node_link(self);
  }
  else if (delta > AVL_DELTA_MAX)
  {
    if ((((((self->link.avl_left)->link.avl_left) ? (self->link.avl_left)->
	   link.avl_left->link.avl_height : 0)) -
	 (((self->link.avl_left)->link.avl_right) ? (self->link.avl_left)->
	  link.avl_right->link.avl_height : 0)) < 0)
      self->link.avl_left = AVL_ROTL_whio_avl_node_link(self->link.avl_left);
    return AVL_ROTR_whio_avl_node_link(self);
  }
  self->link.avl_height = 0;
  if (self->link.avl_left
      && (self->link.avl_left->link.avl_height > self->link.avl_height))
    self->link.avl_height = self->link.avl_left->link.avl_height;
  if (self->link.avl_right
      && (self->link.avl_right->link.avl_height > self->link.avl_height))
    self->link.avl_height = self->link.avl_right->link.avl_height;
  self->link.avl_height += 1;
  return self;
}

static struct whio_avl_node *
AVL_ROTL_whio_avl_node_link(struct whio_avl_node *self)
{
  struct whio_avl_node *r = self->link.avl_right;
  self->link.avl_right = r->link.avl_left;
  r->link.avl_left = AVL_BALANCE_whio_avl_node_link(self);
  return AVL_BALANCE_whio_avl_node_link(r);
}

static struct whio_avl_node *
AVL_ROTR_whio_avl_node_link(struct whio_avl_node *self)
{
  struct whio_avl_node *l = self->link.avl_left;
  self->link.avl_left = l->link.avl_right;
  l->link.avl_right = AVL_BALANCE_whio_avl_node_link(self);
  return AVL_BALANCE_whio_avl_node_link(l);
}

struct whio_avl_node *
AVL_INSERT_whio_avl_node_link(struct whio_avl_node *self,
			      struct whio_avl_node *elm,
			      int (*compare) (struct whio_avl_node const *lhs,
					      struct whio_avl_node const
					      *rhs))
{
  if (!self)
    return elm;
  if (compare(elm, self) < 0)
    self->link.avl_left =
      AVL_INSERT_whio_avl_node_link(self->link.avl_left, elm, compare);
  else
    self->link.avl_right =
      AVL_INSERT_whio_avl_node_link(self->link.avl_right, elm, compare);
  return AVL_BALANCE_whio_avl_node_link(self);
}

struct whio_avl_node *
AVL_FIND_whio_avl_node_link(struct whio_avl_node *self,
			    struct whio_avl_node *elm,
			    int (*compare) (struct whio_avl_node const *lhs,
					    struct whio_avl_node const *rhs))
{
  if (!self)
    return 0;
  const int cmp = (self == elm) ? 0 : compare(elm, self);
  if (0 == cmp)
    return self;
  if (0 > cmp)
    return AVL_FIND_whio_avl_node_link(self->link.avl_left, elm, compare);
  else
    return AVL_FIND_whio_avl_node_link(self->link.avl_right, elm, compare);
}

struct whio_avl_node *
AVL_MOVE_RIGHT_whio_avl_node(struct whio_avl_node *self,
			     struct whio_avl_node *rhs)
{
  if (!self)
    return rhs;
  self->link.avl_right =
    AVL_MOVE_RIGHT_whio_avl_node(self->link.avl_right, rhs);
  return AVL_BALANCE_whio_avl_node_link(self);
}

struct whio_avl_node *
AVL_REMOVE_whio_avl_node_link(struct whio_avl_node *self,
			      struct whio_avl_node *elm,
			      int (*compare) (struct whio_avl_node const *lhs,
					      struct whio_avl_node const
					      *rhs))
{
  if (!self)
    return 0;
  const int cmp = (self == elm) ? 0 : compare(elm, self);
  if (0 == cmp)
  {
    struct whio_avl_node *tmp =
      AVL_MOVE_RIGHT_whio_avl_node(self->link.avl_left, self->link.avl_right);
    self->link.avl_left = 0;
    self->link.avl_right = 0;
    return tmp;
  }
  else if (0 > cmp)
  {
    self->link.avl_left =
      AVL_REMOVE_whio_avl_node_link(self->link.avl_left, elm, compare);
  }
  else
  {
    self->link.avl_right =
      AVL_REMOVE_whio_avl_node_link(self->link.avl_right, elm, compare);
  }
  return self;
}

void
AVL_APPLY_FORWARD_ALL_whio_avl_node_link(struct whio_avl_node *self,
					 void (*function) (struct
							   whio_avl_node * n,
							   void *data),
					 void *data)
{
  if (self)
  {
    AVL_APPLY_FORWARD_ALL_whio_avl_node_link(self->link.avl_left, function,
					     data);
    function(self, data);
    AVL_APPLY_FORWARD_ALL_whio_avl_node_link(self->link.avl_right, function,
					     data);
  }
}

void
AVL_APPLY_REVERSE_ALL_whio_avl_node_link(struct whio_avl_node *self,
					 void (*function) (struct
							   whio_avl_node * n,
							   void *data),
					 void *data)
{
  if (self)
  {
    AVL_APPLY_REVERSE_ALL_whio_avl_node_link(self->link.avl_right, function,
					     data);
    function(self, data);
    AVL_APPLY_REVERSE_ALL_whio_avl_node_link(self->link.avl_left, function,
					     data);
  }
}

void
AVL_FREE_NODES_whio_avl_node_link(struct whio_avl_node *whio_avl_node,
				  void (*dtor) (struct whio_avl_node *
						whio_avl_node))
{
  if (!whio_avl_node)
    return;
  AVL_FREE_NODES_whio_avl_node_link(whio_avl_node->link.avl_left, dtor);
  whio_avl_node->link.avl_left = 0;
  AVL_FREE_NODES_whio_avl_node_link(whio_avl_node->link.avl_right, dtor);
  whio_avl_node->link.avl_right = 0;
  if (NULL != dtor)
    dtor(whio_avl_node);
}
