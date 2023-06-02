sqs
====

"sqs" is a tiny scripting language inspired by Python and Lua.


### Tiny?

It's meant to be useful and capable, yet it compiles in under half a second on my unexceptional hardware.  Lua compiles in about 3s, Python takes about 1m45s.  The executable is about half the size of Lua's.


### What's it look like?

```
name = "world"
print("Hello, {name}!")

run([ "echo" "Hello," "world" ])
fn capture(command)
  return run(command, { capture = true }).output.trim()
today = capture([ "date", "+%Y-%m-%d" ])
print("Today: {today}")

class Person
  (name)
  init(initial-name)
    name = initial-name
  id
    return name
person = Person("world")
print("Hello, {person.id}")

object-files = [ "a.o" "b.o" "c.o" ]
if Path(object-files[0]).exists
  $ cc {object-files} -o a.out
```

See the "nutshell" file for more.


### Who would use this?

Nobody, probably.  It's meant for the places where you'd want to use Python to do shell-type scripting, but don't want something as heavy as Python.  But Python is everywhere.  I was thinking sqs might be useful on small systems like the Ox64, but even that has Python in its standard buildroot.  And then there's Micropython...


### Advantages over Python

Nonetheless, sqs fixes some of the annoying things about Python:

- You don't need to type "self." all the damn time.  sqs is object-oriented from day one, so instance variables and self-calls are in scope.
- You don't need to remember to type the "f" before every string.  String interpolation is the default.
- "kebab-case" is allowed (and encouraged).
- You don't need to remember the ":" after "else" and other control-flow statements.
- `init` instead of `__init__`, `+` instead of `__add__`, etc.


### Advantages over Lua

- String interpolation.
- kebab-case.
- Zero-based arrays.
- sqs's `run()` function has better control over program arguments than Lua's `system()` function.


### Documentation

[Statements](docs/statements.html)

[Expressions](docs/expressions.html)

[Built-in Classes](docs/builtin-classes.md)

[Built-in Functions](docs/builtin-functions.md)




