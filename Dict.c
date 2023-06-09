#include "Dict.h"
#include "String.h"
#include "Class.h"
#include "Array.h"
#include "Object.h"
#include "Boolean.h"
#include "Int.h"
#include "Memory.h"
#include "Error.h"
#include <string.h>
#include <stdio.h>

typedef uint16_t Dict_index_t;
#define MAX_NODES UINT16_MAX

typedef struct DictNode {
	Dict_index_t left, right;
	Dict_index_t level;
	String* key;
	struct Object* value;
	} DictNode;

#define capacity_increment 32

// Index zero represents a null "pointer".  The node at index zero has the tree
// root as its "left".

#define Node(index) (self->tree[index])


Class Dict_class;
static Class DictIterator_class;
static Class DictIteratorKeyValue_class;



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
		if (self->capacity > MAX_NODES)
			Error("Dictionary overflow!  Dicts only support up to %d entries.", MAX_NODES);
		self->tree = realloc_mem(self->tree, self->capacity * sizeof(DictNode));
		memset(self->tree + old_capacity, 0, capacity_increment * sizeof(DictNode));
		}

	self->size += 1;
	Dict_index_t node = self->size;
	DictNode* t = &Node(node);
	t->key = key;
	t->value = value;
	return node;
}


static Dict_index_t Dict_insert(Dict* self, struct String* key, struct Object* value, Dict_index_t node)
{
	if (node == 0)
		return Dict_create_node(self, key, value);
	DictNode* t = &Node(node);
	// Note: "t" can be invalidated by Dict_insert().
	int cmp = String_cmp(key, t->key);
	if (cmp < 0) {
		// Stupid GCC caches the address of self->tree[node], even at -O0!
		Dict_index_t new_left = Dict_insert(self, key, value, t->left);
		self->tree[node].left = new_left;
		}
	else if (cmp > 0) {
		Dict_index_t new_right = Dict_insert(self, key, value, t->right);
		self->tree[node].right = new_right;
		}
	else
		self->tree[node].value = value;

	node = Dict_skew(self, node);
	node = Dict_split(self, node);
	return node;
}

static Dict_index_t IdentityDict_insert(Dict* self, Object* key, Object* value, Dict_index_t node)
{
	if (node == 0)
		return Dict_create_node(self, (String*) key, value);
	DictNode* t = &Node(node);
	// Note: "t" can be invalidated by IdentityDict_insert().
	ptrdiff_t cmp = key - (Object*) t->key;
	if (cmp < 0) {
		// Stupid GCC caches the address of self->tree[node], even at -O0!
		Dict_index_t new_left = IdentityDict_insert(self, key, value, t->left);
		self->tree[node].left = new_left;
		}
	else if (cmp > 0) {
		Dict_index_t new_right = IdentityDict_insert(self, key, value, t->right);
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
	self->class_ = &Dict_class;
	self->capacity = capacity_increment;
	self->size = 0;
	self->tree = (DictNode*) alloc_mem(self->capacity * sizeof(DictNode));
	Node(0).left = 0;
}


void Dict_set_at(Dict* self, String* key, Object* value)
{
	Dict_index_t new_left = Dict_insert(self, key, value, Node(0).left);
	Node(0).left = new_left;
}

void IdentityDict_set_at(Dict* self, Object* key, Object* value)
{
	Dict_index_t new_left = IdentityDict_insert(self, key, value, Node(0).left);
	Node(0).left = new_left;
}



struct Object* Dict_at(Dict* self, String* key)
{
	if (self->size == 0)
		return NULL;

	int node = Node(0).left;
	while (node != 0) {
		DictNode* t = &Node(node);
		int cmp = String_cmp(key, t->key);
		if (cmp < 0)
			node = t->left;
		else if (cmp > 0)
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
		int cmp = String_cmp(key, t->key);
		if (cmp < 0)
			node = t->left;
		else if (cmp > 0)
			node = t->right;
		else
			return t->key;
		}

	return NULL;
}

Object* IdentityDict_at(Dict* self, Object* key)
{
	if (self->size == 0)
		return NULL;

	int node = Node(0).left;
	while (node != 0) {
		DictNode* t = &Node(node);
		ptrdiff_t cmp = key - (Object*) t->key;
		if (cmp < 0)
			node = t->left;
		else if (cmp > 0)
			node = t->right;
		else
			return t->value;
		}

	return NULL;
}


static void Dict_dump_node(Dict* self, Dict_index_t node, int level)
{
	DictNode* t = &Node(node);
	for (int i = level; i > 0; --i)
		printf("  ");
	printf("\"%s\" (%d) [%d]\n", String_c_str(t->key), t->level, node);
	if (t->left)
		Dict_dump_node(self, t->left, level + 1);
	else if (t->right) {
		for (int i = level + 1; i > 0; --i)
			printf("  ");
		printf("-\n");
		}
	if (t->right)
		Dict_dump_node(self, t->right, level + 1);
	else if (t->left) {
		for (int i = level + 1; i > 0; --i)
			printf("  ");
		printf("-\n");
		}
}


void Dict_dump(Dict* self)
{
	if (Node(0).left == 0)
		printf("Empty Dict.\n");
	else
		Dict_dump_node(self, Node(0).left, 0);
}




static void DictIterator_push_tree(DictIterator* self, size_t node)
{
	while (node) {
		Array_append(self->stack, (Object*) node);
		node = self->dict->tree[node].left;
		}
}


DictIterator* new_DictIterator(Dict* dict)
{
	DictIterator* self = alloc_obj(DictIterator);
	self->class_ = &DictIterator_class;
	self->dict = dict;
	self->stack = new_Array();

	DictIterator_push_tree(self, dict->tree[0].left);

	return self;
}


DictIteratorResult DictIterator_next(DictIterator* self)
{
	DictIteratorResult result = { NULL, NULL };

	if (self->stack->size == 0)
		return result;

	// Get the result.
	DictNode* node = &self->dict->tree[(size_t) Array_back(self->stack)];
	result.key = node->key;
	result.value = node->value;

	// Go forward.
	int right = node->right;
	Array_pop_back(self->stack);
	DictIterator_push_tree(self, right);

	return result;
}


typedef struct DictIteratorKeyValue {
	Class* class_;
	DictIteratorResult result;
	} DictIteratorKeyValue;


static Object* Dict_init_builtin(Object* super, Object** args)
{
	Dict_init((Dict*) super);
	return super;
}

static Object* Dict_at_builtin(Object* super, Object** args)
{
	String* key = String_enforce(args[0], "Dict.[]");
	return (Object*) Dict_at((Dict*) super, key);
}

static Object* Dict_set_at_builtin(Object* super, Object** args)
{
	String* key = String_enforce(args[0], "Dict.[]=");
	Dict_set_at((Dict*) super, key, args[1]);
	return args[1];
}

static Object* Dict_iterator_builtin(Object* super, Object** args)
{
	return (Object*) new_DictIterator((Dict*) super);
}

static Object* Dict_size_builtin(Object* super, Object** args)
{
	return (Object*) new_Int(((Dict*) super)->size);
}

static Object* Dict_contains_builtin(Object* super, Object** args)
{
	Dict* self = (Dict*) super;
	String* key = String_enforce(args[0], "Dict.contains");
	return make_bool(Dict_at(self, key) != NULL);
}


static Object* DictIterator_next_builtin(Object* super, Object** args)
{
	DictIteratorResult result = DictIterator_next((DictIterator*) super);
	if (result.key == NULL)
		return NULL;

	DictIteratorKeyValue* kv = alloc_obj(DictIteratorKeyValue);
	kv->class_ = &DictIteratorKeyValue_class;
	kv->result = result;
	return (Object*) kv;
}

static Object* DictIteratorKeyValue_key(Object* super, Object** args)
{
	return (Object*) ((DictIteratorKeyValue*) super)->result.key;
}

static Object* DictIteratorKeyValue_value(Object* super, Object** args)
{
	return ((DictIteratorKeyValue*) super)->result.value;
}


void Dict_init_class()
{
	init_static_class(Dict);
	static const BuiltinMethodSpec builtin_methods[] = {
		{ "init", 0, Dict_init_builtin },
		{ "[]", 1, Dict_at_builtin },
		{ "[]=", 1, Dict_set_at_builtin },
		{ "iterator", 0, Dict_iterator_builtin },
		{ "size", 0, Dict_size_builtin },
		{ "contains", 0, Dict_contains_builtin },
		{ NULL },
		};
	Class_add_builtin_methods(&Dict_class, builtin_methods);

	init_static_class(DictIterator);
	static const BuiltinMethodSpec builtin_iterator_methods[] = {
		{ "next", 0, DictIterator_next_builtin },
		{ NULL },
		};
	Class_add_builtin_methods(&DictIterator_class, builtin_iterator_methods);

	init_static_class(DictIteratorKeyValue);
	static const BuiltinMethodSpec builtin_kv_methods[] = {
		{ "key", 0, DictIteratorKeyValue_key },
		{ "value", 0, DictIteratorKeyValue_value },
		{ NULL },
		};
	Class_add_builtin_methods(&DictIteratorKeyValue_class, builtin_kv_methods);
}


bool Dict_option_turned_on(Dict* dict, String* option_name)
{
	Object* option_value = Dict_at(dict, option_name);
	return IS_TRUTHY(option_value);
}


bool Dict_option_turned_off(Dict* dict, String* option_name)
{
	Object* option_value = Dict_at(dict, option_name);
	if (option_value == NULL)
		return false;
	return !IS_TRUTHY(option_value);
}



