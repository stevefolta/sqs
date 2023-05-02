#include "Dict.h"
#include "String.h"
#include <string.h>
#include <stdio.h>

typedef uint16_t Dict_index_t;

typedef struct DictNode {
	Dict_index_t left, right;
	Dict_index_t level;
	String* key;
	Object* value;
	} DictNode;

#define capacity_increment 32

// Index zero represents a null "pointer".  The node at index zero has the tree
// root as its "left".

#define Node(index) (self->tree[index])


static Dict_index_t Dict_skew(Dict* self, Dict_index_t node)
{
	if (node == 0)
		return 0;
	DictNode* t = &Node(node);
	if (t->left == 0)
		return node;
	Dict_index_t left = t->left;
	DictNode* l = &Node(left);
	if (l->level == t->level) {
		// Swap the pointers of the horizontal left links.
		t->left = l->right;
		l->right = node;
		return left;
		}
	return node;
}


static Dict_index_t Dict_split(Dict* self, Dict_index_t node)
{
	if (node == 0)
		return 0;
	DictNode* t = &Node(node);
	Dict_index_t right = t->right;
	if (right == 0)
		return node;
	DictNode* r = &Node(right);
	Dict_index_t right_right = r->right;
	if (right_right == 0)
		return node;
	if (Node(node).level == Node(right_right).level) {
		// We have two horizontal right links.  Elevate the middle node as the root
		// of this subtree.
		t->right = r->left;
		r->left = node;
		r->level += 1;
		return right;
		}
	return node;
}

static Dict_index_t Dict_create_node(Dict* self, struct String* key, struct Object* value)
{
	if (self->size + 1 >= self->capacity) {
		int old_capacity = self->capacity;
		self->capacity += capacity_increment;
		self->tree = realloc_mem(self->tree, self->capacity * sizeof(DictNode));
		memset(self->tree + old_capacity, 0, capacity_increment * sizeof(DictNode));
		}

	self->size += 1;
	Dict_index_t node = self->size;
	DictNode* t = &Node(node);
	t->key = key;
	t->value = value;
	return node;;
}


static Dict_index_t Dict_insert(Dict* self, struct String* key, struct Object* value, Dict_index_t node)
{
	if (node == 0)
		return Dict_create_node(self, key, value);
	DictNode* t = &Node(node);
	// Note: "t" can be invalidated by Dict_insert().
	if (String_less_than(key, t->key)) {
		// Stupid GCC caches the address of self->tree[node], even at -O0!
		Dict_index_t new_left = Dict_insert(self, key, value, t->left);
		self->tree[node].left = new_left;
		}
	else if (String_less_than(t->key, key)) {
		Dict_index_t new_right = Dict_insert(self, key, value, t->right);
		self->tree[node].right = new_right;
		}
	else
		self->tree[node].value = value;

	node = Dict_skew(self, node);
	node = Dict_split(self, node);
	return node;
}


Dict* new_Dict()
{
	Dict* dict = alloc_obj(Dict);
	Dict_init(dict);
	return dict;
}


void Dict_init(Dict* self)
{
	self->capacity = capacity_increment;
	self->size = 0;
	self->tree = (DictNode*) alloc_mem(self->capacity * sizeof(DictNode));
	Node(0).left = 0;
}


void Dict_set_at(Dict* self, struct String* key, struct Object* value)
{
	Node(0).left = Dict_insert(self, key, value, Node(0).left);
}


struct Object* Dict_at(Dict* self, String* key)
{
	if (self->size == 0)
		return NULL;

	int node = Node(0).left;
	while (node != 0) {
		DictNode* t = &Node(node);
		if (String_less_than(key, t->key))
			node = t->left;
		else if (String_less_than(t->key, key))
			node = t->right;
		else
			return t->value;
		}

	return NULL;
}


struct String* Dict_key_at(Dict* self, struct String* key)
{
	if (self->size == 0)
		return NULL;

	int node = Node(0).left;
	while (node != 0) {
		DictNode* t = &Node(node);
		if (String_less_than(key, t->key))
			node = t->left;
		else if (String_less_than(t->key, key))
			node = t->right;
		else
			return t->key;
		}

	return NULL;
}


static void Dict_dump_node(Dict* self, Dict_index_t node, int level)
{
	DictNode* t = &Node(node);
	for (int i = level; i > 0; --i)
		printf("  ");
	printf("\"%s\" (%d)\n", String_c_str(t->key), t->level);
	if (t->left)
		Dict_dump_node(self, t->left, level + 1);
	if (t->right)
		Dict_dump_node(self, t->right, level + 1);
}


void Dict_dump(Dict* self)
{
	if (Node(0).left == 0)
		printf("Empty Dict.\n");
	else
		Dict_dump_node(self, Node(0).left, 0);
}



