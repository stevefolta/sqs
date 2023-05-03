#include "Object.h"
#include "Class.h"
#include "Dict.h"
#include "Nil.h"
#include "Error.h"
#include <stddef.h>


Object* Object_find_method(Object* self, struct String* name)
{
	Class* class_ = (self ? self->class_ : &Nil_class);
	while (class_) {
		if (class_->methods) {
			Object* method = Dict_at(class_->methods, name);
			if (method)
				return method;
			}
		class_ = class_->superclass;
		}

	return NULL;
}



Class Object_class;
void Object_init_class()
{
	Class_init_static(&Object_class, "Object", 0);
	Object_class.superclass = NULL;
}


