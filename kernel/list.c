#include <kernel/list.h>
#include <kernel/libc/assert.h>
#include <kernel/malloc/malloc.h>

static void lst_node_init(struct list_node *node, void *data)
{
	node->_private = data;
	node->next = NULL;
	node->prev = NULL;
}

struct list_node *lst_node_new(void *data)
{
	struct list_node *node = kmalloc(sizeof(*node));

	if (node)
		lst_node_init(node, data);
	return node;
}

struct list_node *lst_node_unlink(struct list_node *node)
{
	if (node->prev)
		node->prev->next = node->next;
	if (node->next)
		node->next->prev = node->prev;

	node->next = NULL;
	node->prev = NULL;

	return node;
}

static void _lst_node_free(struct list_node *node)
{
	node->prev = NULL;
	node->next = NULL;
	kfree(node);
}

void lst_node_free(struct list_node *node)
{
	/* TODO some debug check to make sure the list is not dangling */
	_lst_node_free(node);
}

void lst_node_del(struct list_node *node, void (*del)(void *))
{
	if (node)  {
		lst_node_unlink(node);

		if (del)
			del(node->_private);

		lst_node_free(node);
	}
}

void lst_node_add_after(struct list_node *list, struct list_node *node)
{
	if (list->next) {
		list->next->prev = node;
		node->next = list->next;
	}

	list->next = node;
	node->prev = list;
}

void lst_node_add_before(struct list_node *list, struct list_node *node)
{
	if (list->prev) {
		list->prev->next = node;
		node->prev = list->prev;
	}

	list->prev = node;
	node->next = list;
}

static void _lst_node_destroy(struct list_node *head, void (*del)(void*))
{
	while (head) {
		struct list_node *next = head->next;

		del(head->_private);
		lst_node_free(head);

		head = next;
	}
}

static struct list_node *_lst_node_find(const struct list_node *head,
					bool (*p)(const void *, void *), void *opaque)
{
	while (head) {
		if (p(head->_private, opaque))
			break;
		head = head->next;
	}
	return (struct list_node *)head;
}

void lst_init(struct list *list)
{
	list->head = NULL;
	list->tail = NULL;
}

void lst_free(struct list *list, void (*del)(void*))
{
	_lst_node_destroy(list->head, del);

	list->head = NULL;
	list->tail = NULL;
}

static void lst_append_node(struct list *list, struct list_node *node)
{
	if (list->tail) {
		assert(list->head);

		lst_node_add_after(list->tail, node);
	} else {
		assert(!list->head);

		list->head = list->tail = node;
	}
}

static void lst_prepend_node(struct list *list, struct list_node *node)
{
	if (list->head) {
		assert(list->tail);

		lst_node_add_before(list->head, node);
	} else {
		assert(!list->tail);

		list->head = list->tail = node;
	}
}

struct list_node *lst_append(struct list *list, void *data)
{
	struct list_node *node = lst_node_new(data);

	if (node)
		lst_append_node(list, node);

	return node;
}

struct list_node *lst_prepend(struct list *list, void *data)
{
	struct list_node *node = lst_node_new(data);

	if (node)
		lst_prepend_node(list, node);

	return node;
}

void lst_append_list(struct list *list, struct list *other)
{
	if (other->head) {
		assert(other->tail);
		lst_append_node(list, other->head);
	}

	other->head = NULL;
	other->tail = NULL;
}

struct list_node *lst_find(const struct list *list, bool (*p)(const void *, void *),
			   void *opaque)
{
	return _lst_node_find(list->head, p, opaque);
}

struct list_node *lst_unlink(struct list *list, struct list_node *node)
{
	if (list->head == node) {
		assert(!node->prev);
		list->head = node->next;
	}

	if (list->tail == node) {
		assert(!node->next);
		list->tail = node->prev;
	}

	node->prev = NULL;
	node->next = NULL;

	return node;
}

void lst_del(struct list *list, struct list_node *node, void (*del)(void*))
{
	lst_unlink(list, node);
	lst_node_del(node, del);
}

static void _lst_node_foreach(struct list_node *head, void (*f)(void *, void*), void *opaque)
{
	while (head) {
		f(head->_private, opaque);
		head = head->next;
	}
}

void lst_foreach(struct list *list, void (*f)(void *, void*), void *opaque)
{
	_lst_node_foreach(list->head, f, opaque);
}

bool lst_isempty(const struct list *list)
{
	return list->head == NULL;
}
