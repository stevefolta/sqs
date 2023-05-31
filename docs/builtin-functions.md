sqs Built-in Functions
=====

<dl>

<dt> print(<i>[object]</i>, <i>[options]</i>) </dt>
<dd>
Prints the object.  If it's not a String, the <code>string</code> method will be called on it.  <i>options</i>, if given, must be a Dict.  If a <code>file</code> or <code>out</code> option is given, the object is written to it (by calling its <code>write()</code> method) instead of to stdout.  If an <code>end</code> option is given, that is printed at the end instead of a newline (it can be an empty string).  If no arguments are given, a newline is printed to stdout.
</dd>

<dt> run(<i>arguments</i>, <i>[options]</i>) </dt>
<dd>
Runs a program.  Normally, <i>arguments</i> should be an Array, with the program name or path as the first (zeroth) argument, and the command-line arguments to the program as the rest of the arguments.  <i>arguments</i> can also be a String; in that case it will be run through a shell.  <i>options</i>, if given, must be a Dict.  The only supported option is <code>capture</code>; if true, the stdout of the program will be captured as a String.  Returns an object that supports these methods: <code>return-code</code> (the integer exit code); <code>ok</code> (equivalent to <code>return-code == 0</code>); and <code>output</code> (the captured output if the <code>capture</code> option was turned on).
</dd>

<dt> sleep(<i>seconds</i>) </dt>
<dd>
Sleeps for the given number of seconds.  <i>seconds</i> must be an integer, since sqs only supports integers currently.
</dd>

<dt> glob(<i>pattern</i>, <i>[options]</i>) </dt>
<dd>
Returns an Array of paths of files in the filesystem matching the glob-style pattern.  <i>options</i>, if given, must be a Dict.  Supported options are <code>mark-directories</code> (appends "/" to directories), <code>sort</code> (defaults to <code>true</code>), <code>escape</code> (allows escaping characters with a backslash; defaults to <code>true</code>), and <code>tilde</code> (allows tilde-expansion).
</dd>

</dl>


