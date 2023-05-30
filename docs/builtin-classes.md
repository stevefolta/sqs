sqs Built-in Classes
=====

## String

<dl>

<dt> +(<i>other</i>) </dt>
<dd> Adds the other string and returns the result. <dd>

<dt>==(<i>other</i>), !=(<i>other</i>), <(<i>other</i>), <=(<i>other</i>), >(<i>other</i>), >=(<i>other</i>) </dt>
<dd> Comparison operators. </dd>

<dt> strip(<i>other</i>), trim(<i>other</i>) </dt>
<dd> Returns the string with whitespace removed from the start and end.  "strip" and "trim" are synonyms. </dd>
<dt> lstrip(<i>other</i>), ltrim(<i>other</i>) </dt>
<dd> Returns the string with whitespace removed from the start.  "lstrip" and "ltrim" are synonyms. </dd>
<dt> rstrip(<i>other</i>), rtrim(<i>other</i>) </dt>
<dd> Returns the string with whitespace removed from the end.  "rstrip" and "rtrim" are synonyms. </dd>

<dt> split(<i>[delimiter]</i>) </dt>
<dd>
If the <i>delimiter</i> is not given, returns an Array of the whitespace-separated words in the string.
If the <i>delimiter</i> (a String) is given, returns an Array of substrings split at the delimiter.  Unlike the whitespace-based splitting, the result may contain empty strings.
</dd>

<dt> starts-with(<i>other</i>) </dt>
<dt> ends-with(<i>other</i>) </dt>
<dd> Returns whether the string starts or ends with the other string. </dd>

<dt> contains(<i>other</i>) </dt>
<dd> Returns whether the string contains the other string. </dd>

<dt> is-valid </dt>
<dd> Returns whether the string is valid UTF-8. </dd>

<dt> decode-8851-1 </dt>
<dd> Returns another String (in normal UTF-8 encoding) with this String's bytes interpreted as being encoded in ISO-8851-1. </dd>

</dl>


## Array

<dl>

<dt> size </dt>
<dd> Returns the size of the array. </dd>

<dt> [](<i>index</i>) </dt>
<dd> Return the item at the index.  Negative indexes select items from the back of the array.  Results in an error if the index is out-of-range. </dd>

<dt> []=(<i>index</i>, <i>value</i>) </dt>
<dd> Sets the item at the index, growing the array if needed. </dd>

<dt> +(<i>other</i>) </dt>
<dd> Returns a new array with the two arrays concatenated. </dd>

<dt> append(<i>value</i>) </dt>
<dd> Adds an item to the end of the array.  Returns the new item. </dd>

<dt> join(<i>string</i>) </dt>
<dd> Returns a string of all the array's items with the string (if given) between them. The items will have the "string" method called on them.  </dd>

<dt> pop(), pop-back() </dt>
<dd> Removes the last item of the array and returns it.  "pop" and "pop-back" are synonyms. </dd>

<dt> back </dt>
<dd> Returns the last item in the array. </dd>

<dt> slice(<i>start</i>, <i>[end]</i>) </dt>
<dd> Return a slice of the array from the start index (inclusive) to the end index (non-inclusive).  If <i>end</i> isn't given, the end of the array is used. </dd>

<dt> copy </dt>
<dd> Returns a shallow copy of the array. </dd>

<dt> iterator </dt>
<dd> Returns a new iterator on the array.  Mostly used by the "for" statement. </dd>

<dt> contains(<i>item</i>) </dt>
<dd> Returns whether the array contains the item. </dd>

</dl>

# Dict

<dl>

<dt> [](<i>key</i>) </dt>
<dd> Returns the item at <i>key</i>, which must be a String. </dd>

<dt> []=(<i>key</i>, <i>value</i>) </dt>
<dd> Sets or adds an entry in the Dict.  <i>key</i> must be a String. </dd>

<dt> size </dt>
<dd> Returns the number of keys/values in the Dict. </dd>

<dt> contains(<i>key</i>) </dt>
<dd> Returns whether the Dict contains the key. </dd>

<dt> iterator </dt>
<dd> Returns an iterator on the Dict.  Mostly used by the "for" statement.  The iterator's "next" method returns an object with "key" and "value" methods (or "nil" when it reaches the end). </dd>

</dl>


## Regex

The Regex class supports Posix Extended Regular Expression Syntax.  It also supports named groups with the Python-style "(?P<<i>name</i>>...)" syntax.

<dl>

<dt> init(<i>regular-expression</i>, <i>[options]</i>) <dt>
<dd> Raw string literals are helpful for <i>regular-expression</i>.  <i>options</i>, if given, is a Dict.  All options are Booleans; the available options are "extended-syntax" (defaults to "true"), "case-insensitive", and "newline". </dd>

<dt> match(<i>string</i>, <i>[options]</i>) </dt>
<dd> Matches the regular expression against <i>string</i>.  Returns "nil" if it doesn't match, or a match object if it does.  The match object supports the "[]" method; match[0] is the whole match, and subsequent integers give the matched groups.  Groups can also be accessed by name if they were given names, eg. "match['url']".  <i>options</i> are Booleans, and may specify "not-bol" and/or "not-eol". </dd>

</dl>


## File

<dl>

<dt> init(<i>path</i>, <i>[mode]</i>) </dt>
<dd> Opens the file at <i>path</i>.  <i>mode</i> defaults to "r", and must be one of "r" (read), "r+" (open for reading & writing), "w" (create or truncate to zero length and write), "w+" (create or truncate and open for read and write), "a" (append, file is created if it doesn't exist), or "a+" (open for reading and appending, file is created if it doesn't exist). </dd>

<dt> write(<i>data</i>) </dt>
<dd> <i>data</i> must be either a String or a ByteArray. </dd>

<dt> read(<i>data</i>) </dt>
<dd> <i>data</i> must be a ByteArray.  This will attempt to read enough to fill the entire ByteArray, and return the number of bytes that it did read.  If it returns zero, the end-of-file has been reached. </dd>

<dt> close() </dt>
<dd> Closes the file.  Usually called by a "with" statement.  It is an error to call read() or write() on a File after it has been closed. </dd>

<dt> lines </dt>
<dd> Returns an iterator that returns the next line in the file when its next() method is called (or "nil" when there are no more lines left). </dd>

</dl>


## Path

Represents the path of a file or directory in the filesystem.  Basic tilde expansion is done (`~`, `~/foo/bar`), but not any more advanced tilde expansion (`~someuser/foo`).

<dt> init(<i>string</i>) </dt>
<dd> Creates the path, doing basic tilde expansion. </dd>

<dt> basename, base-name </dt>
<dd> Returns the last component of the path. </dd>

<dt> dirname, dir-name </dt>
<dd> Returns the path without its last component. <dd>

<dt> exists </dt>
<dd> Returns whether a file or directory exists at the path. </dd>

<dt> is-file, is-dir, is-symlink </dt>
<dd> Returns whether a filesystem entry exists at the path and is of that type. </dd>

<dt> can-read, can-write, can-execute </dt>
<dd> Returns whether the current user has permission to do the given operation. </dd>

<dt> size </dt>
<dd> For a file, returns the size of the file in bytes. </dd>


## ByteArray

<dl>

<dt> init(<i>[size]</i>) <dt>
<dd> If size is given, the new ByteArray has the given size.  If not, it starts out empty. </dd>

<dt> size </dt>
<dd> Returns the number of bytes in the ByteArray. </dd>

<dt> [](<i>index</i>) </dt>
<dd> Returns the integer value of the byte at <i>index</i>. </dd>

<dt> []=(<i>index</i>, <i>value</i>) </dt>
<dd> Sets the byte at <i>index</i>. Grows the ByteArray if needed. </dd>

<dt> as-string </dt>
<dd> Returns a String with the bytes in the ByteArray.  Assumes UTF-8 encoding. </dd>

<dt> slice(<i>start</i>, <i>size</i>) </dt>
<dd> Returns a new ByteArray that's a slice of the original ByteArray.  Setting a byte within the slice will modify the original ByteArray, but setting a byte outside it will cause the slice to be "detached" and become a copy. </dd>

<dt> is-valid-utf8 </dt>
<dd> Returns whether the bytes represent valid UTF-8. </dd>

<dt> decode-8851-1 </dt>
<dd> Returns a String (in normal UTF-8 encoding) with the ByteArray's bytes interpreted as being encoded in ISO-8851-1. </dd>
	
</dl>



