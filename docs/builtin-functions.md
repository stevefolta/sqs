sqs Built-in Functions
=====

<dl>

<dt> print(<i>[object]</i>, <i>[options]</i>) </dt>
<dd> Prints the object.  If it's not a String, the "string" method will be called on it.  <i>options</i> is a dictionary.  If a "file" option is given, the object is written to the file (by calling its "write()" method) instead of to stdout.  If an "end" option is given, that is printed at the end instead of a newline (it can be an empty string).  If no arguments are given, a newline is printed to stdout. </dd>

</dl>


