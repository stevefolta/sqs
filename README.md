sqs
====

"sqs" is a tiny scripting language inspired by Python and Lua.


### Tiny?

It's meant to be useful and capable, yet it compiles in under half a second on my unexceptional hardware.  Lua compiles in about 3s, Python takes about 1m45s.  The executable is about half the size of Lua's, and only slightly larger than that of dash (a Posix shell that's meant to be as small as possible).


### What's it look like?

```
name = "world"
print("Hello, {name}!")

fn greet(whom)
	print("Hello, {whom}!")
greet("world")

# To do shell-scripting things, use the "$" statement:
$ grep TODO README
$ grep TODO {glob('*.c')}
object-files = [ "a.o" "b.o" "c.o" ]
if Path(object-files[0]).exists
  $ cc {object-files} -o "a.out"

# Or capture the results in an expression:
print("Today: {$(date '+%Y-%m-%d')}")

# Object-oriented programming is supported:
class Person
  (name)
  init(initial-name)
    name = initial-name
  id
    return name
person = Person("world")
print("Hello, {person.id}")
```

See the "nutshell" file for more.


### Who would use this?

It's meant for the places where you'd want to use Python to do shell-type scripting, but don't want something as heavy as Python.  Or you want shelling-out to be easier than using subprocess.run().  Or you'd want to use Lua, but want easier shelling-out and string interpolation.


### Advantages over Python

sqs fixes some of the annoying things about Python:

- You don't need to remember to type the "f" before every string.  String interpolation is the default.
- You don't need to type "self." all the damn time.  sqs is object-oriented from day one, so instance variables and self-calls are in scope.
- "kebab-case" is allowed (and encouraged).
- You don't need to remember the ":" after "else" and other control-flow statements.
- `init` instead of `__init__`, `+` instead of `__add__`, etc.


### Advantages over Lua

- String interpolation.
- kebab-case.
- Zero-based arrays.
- sqs's `run()` function has better control over program arguments than Lua's `system()` function.


### Dependencies

There are two: a POSIX-compliant libc (glibc and musl are known to work); and the Boehm garbage collector.


### Documentation

[Statements](docs/statements.html)  
[Expressions](docs/expressions.html)  
[Built-in Classes](docs/builtin-classes.html)  
[Built-in Functions](docs/builtin-functions.html)
[Modules](docs/modules.html)




