#include "Regex.h"
#include "Class.h"
#include "Object.h"
#include "String.h"
#include "Dict.h"
#include "Int.h"
#include "Boolean.h"
#include "Memory.h"
#include "Error.h"

Class Regex_class;
Class RegexMatch_class;

static String extended_syntax, case_insensitive, newline;
static String not_bol, not_eol;


void free_regex(void* ptr, void* data)
{
	regfree((regex_t*) ptr);
}

Object* Regex_init(Object* super, Object** args)
{
	Regex* self = (Regex*) super;

	// Options.
	int flags = REG_EXTENDED;
	if (args[1] && args[1]->class_ == &Dict_class) {
		Dict* options = (Dict*) args[1];
		Object* option = Dict_at(options, &extended_syntax);
		if (option && !IS_TRUTHY(option))
			flags &= ~REG_EXTENDED;
		if (IS_TRUTHY(Dict_at(options, &case_insensitive)))
			flags |= REG_ICASE;
		if (IS_TRUTHY(Dict_at(options, &newline)))
			flags |= REG_NEWLINE;
		}

	// Count the groups.
	String* regex_string = String_enforce(args[0], "Regex.init");
	const char* p = regex_string->str;
	const char* end = p + regex_string->size;
	self->num_groups = 0;
	for (; p < end; ++p) {
		if (*p == '\'')
			p += 1;
		else if (*p == '(')
			self->num_groups += 1;
		}

	// Compile the regex.
	self->regex = (regex_t*) alloc_mem(sizeof(regex_t));
	int result = regcomp(self->regex, String_c_str(regex_string), flags);
	if (result) {
		size_t error_length = regerror(result, self->regex, NULL, 0);
		char* message = alloc_mem_no_pointers(error_length);
		regerror(result, self->regex, message, error_length);
		Error("Couldn't compile regex (%s).", message);
		}
	mem_add_finalizer(self->regex, free_regex, NULL);

	return super;
}

typedef struct RegexMatch {
	Class* class_;
	String* str;
	int num_groups;
	regmatch_t* matches;
	} RegexMatch;


Object* Regex_match(Object* super, Object** args)
{
	Regex* self = (Regex*) super;

	String* str = String_enforce(args[0], "Regex.match");

	// Options.
	int flags = 0;
	if (args[1] && args[1]->class_ == &Dict_class) {
		Dict* options = (Dict*) args[1];
		if (IS_TRUTHY(Dict_at(options, &not_bol)))
			flags |= REG_NOTBOL;
		if (IS_TRUTHY(Dict_at(options, &not_eol)))
			flags |= REG_NOTEOL;
		}

	// Match.
	regmatch_t* matches =
		(regmatch_t*) alloc_mem_no_pointers((self->num_groups + 1) * sizeof(regmatch_t));
	int result = regexec(self->regex, String_c_str(str), self->num_groups + 1, matches, flags);
	if (result != 0)
		return NULL;

	// Create the RegexMatch.
	RegexMatch* match = alloc_obj(RegexMatch);
	match->class_ = &RegexMatch_class;
	match->str = str;
	match->num_groups = self->num_groups;
	match->matches = matches;
	return (Object*) match;
}


Object* RegexMatch_at(Object* super, Object** args)
{
	RegexMatch* self = (RegexMatch*) super;
	size_t index = Int_enforce(args[0], "RegexMatch.[]");
	if (index >= self->num_groups + 1)
		return NULL;
	regmatch_t* match = &self->matches[index];
	if (match->rm_so == -1)
		return (Object*) new_static_String(NULL, 0);
	return (Object*) new_static_String(self->str->str + match->rm_so, match->rm_eo - match->rm_so);
}


void Regex_init_class()
{
	init_static_class(Regex);
	static const BuiltinMethodSpec builtin_methods[] = {
		{ "init", 2, Regex_init },
		{ "match", 1, Regex_match },
		{ NULL },
		};
	Class_add_builtin_methods(&Regex_class, builtin_methods);

	String_init_static_c(&extended_syntax, "extended-syntax");
	String_init_static_c(&case_insensitive, "case-insensitive");
	String_init_static_c(&newline, "newline");
	String_init_static_c(&not_bol, "not-bol");
	String_init_static_c(&not_eol, "not-eol");

	init_static_class(RegexMatch);
	static const BuiltinMethodSpec match_methods[] = {
		{ "[]", 1, RegexMatch_at },
		{ NULL },
		};
	Class_add_builtin_methods(&RegexMatch_class, match_methods);
}



