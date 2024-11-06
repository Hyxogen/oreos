#ifndef __KERNEL_LIST
#define __KERNEL_LIST

#include <stdbool.h>

/*
 * So at first, this was a singly linked list. But the min alloc size of kmalloc
 * is 24 bytes, so there is no space saved when using a singly linked list
 */

struct list_node {
	void *_private;
	struct list_node *next;
	struct list_node *prev;
};

struct list_node *lst_node_new(void *data);
/* only destroys the specified node and calls the del function on _private if
 * not NULL */
struct list_node *lst_node_unlink(struct list_node *node);
void lst_node_free(struct list_node *node);
void lst_node_del(struct list_node *node, void (*del)(void *));

void lst_node_add_after(struct list_node *list, struct list_node *node);
void lst_node_add_before(struct list_node *list, struct list_node *node);

struct list {
	struct list_node *head;
	struct list_node *tail;
};

void lst_init(struct list *list);
void lst_free(struct list *list, void (*del)(void*));

struct list_node *lst_append(struct list *list, void *data);
struct list_node *lst_prepend(struct list *list, void *data);

/* this moves the contents of the other list */
void lst_append_list(struct list *list, struct list *other);

struct list_node *lst_find(const struct list *list, bool (*p)(const void *, void *),
			   void *opaque);

struct list_node *lst_unlink(struct list *list, struct list_node *node);
void lst_del(struct list *list, struct list_node *node, void (*del)(void*));
void lst_foreach(struct list *list, void (*f)(void *, void*), void *opaque);

bool lst_isempty(const struct list *list);

#endif
