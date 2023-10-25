#include "Regex.h"
#include "Class.h"
#include "Object.h"
#include "String.h"
#include "Dict.h"
#include "Int.h"
#include "Boolean.h"
#include "Memory.h"
#include "Error.h"
#include <string.h>

Class Regex_class;
Class RegexMatch_class;

static String extended_syntax, case_insensitive, newline;
static String not_bol, not_eol;


void free_regex(void* ptr, void* data)
{
	regfree((regex_t*) ptr);
}

extern char* Regex_adjust_regex(Regex* self, String* regex_string);

Object* Regex_init(Object* super, Object** args)
{
	Regex* self = (Regex*) super;

	// Options.
	int flags = REG_EXTENDED;
	if (args[1] && args[1]->class_ == &Dict_class) {
		Dict* options = (Dict*) args[1];
		if (Dict_option_turned_off(options, &extended_syntax))
			flags &= ~REG_EXTENDED;
		if (Dict_option_turned_on(options, &case_insensitive))
			flags |= REG_ICASE;
		if (Dict_option_turned_on(options, &newline))
			flags |= REG_NEWLINE;
		}

	String* regex_string = String_enforce(args[0], "Regex.init");
	char* adjusted_regex = Regex_adjust_regex(self, regex_string);

	// Compile the regex.
	self->regex = (regex_t*) alloc_mem(sizeof(regex_t));
	int result = regcomp(self->regex, adjusted_regex, flags);
	if (result) {
		size_t error_length = regerror(result, self->regex, NULL, 0);
		char* message = alloc_mem_no_pointers(error_length);
		regerror(result, self->regex, message, error_length);
		Error("Couldn't compile regex (%s).", message);
		}
	mem_add_finalizer(self->regex, free_regex, NULL);

	return super;
}

char* Regex_adjust_regex(Regex* self, String* regex_string)
{
	// Count the groups, and get the named groups.
	self->num_groups = 0;
	char* adjusted_regex_string = alloc_mem(regex_string->size + 1);
	const char* p = regex_string->str;
	const char* end = p + regex_string->size;
	char* out = adjusted_regex_string;
	while (p < end) {
		char c = *p++;
		*out++ = c;
		if (p < end && *p == '\'')
			*out++ = *p++;

		else if (c == '(') {
			self->num_groups += 1;
			String* group_name = NULL;
			if (p + 3 < end && memcmp(p, "?P<", 3) == 0) {
				// Named capture group.
				p += 3;
				const char* name_start = p;

				// Find the end of the name.
				while (true) {
					if (p >= end)
						Error("Unterminated capture group name in regex.");
					if (*p == '>') {
						group_name = new_String(name_start, p - name_start);
						p += 1;
						break;
						}
					p += 1;
					}

				// Add the name.
				if (self->capture_groups == NULL)
					self->capture_groups = new_Dict();
				// We need a non-zero index (zero == NULL == no entry in the Dict), so
				// leave it +1, which will be the index we eventually want anyway.
				Dict_set_at(self->capture_groups, group_name, (Object*) (size_t) self->num_groups);
				}
			}
		}

	// Make it a C string.
	*out++ = 0;
	return adjusted_regex_string;
}


typedef struct RegexMatch {
	Class* class_;
	String* str;
	Regex* regex;
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
	match->regex = self;
	match->matches = matches;
	return (Object*) match;
}


Object* RegexMatch_at(Object* super, Object** args)
{
	RegexMatch* self = (RegexMatch*) super;

	// Get the index, either given directly or as the name of a group.
	size_t index = 0;
	if (args[0] && args[0]->class_ == &String_class) {
		if (self->regex->capture_groups)
			index = (size_t) Dict_at(self->regex->capture_groups, (String*) args[0]);
		if (index == 0)
			Error("No capture group named \"%s\" in regex.", String_c_str((String*) args[0]));
		}
	else
		index = Int_enforce(args[0], "RegexMatch.[]");

	if (index >= self->regex->num_groups + 1)
		return NULL;
	regmatch_t* match = &self->matches[index];
	if (match->rm_so == -1)
		return (Object*) &empty_string;
	return (Object*) new_static_String(self->str->str + match->rm_so, match->rm_eo - match->rm_so);
}

Object* RegexMatch_remainder(Object* super, Object** args)
{
	RegexMatch* self = (RegexMatch*) super;

	regoff_t remainder_start = self->matches[0].rm_eo;
	return (Object*) new_static_String(self->str->str + remainder_start, self->str->size - remainder_start);
}


void Regex_init_class()
{
	init_static_class(Regex);
	static const BuiltinMethodSpec builtin_methods[] = {
		{ "init", 2, Regex_init },
		{ "match", 2, Regex_match },
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
		{ "remainder", 0, RegexMatch_remainder },
		{ NULL },
		};
	Class_add_builtin_methods(&RegexMatch_class, match_methods);
}



