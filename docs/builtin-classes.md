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

</dl>


## ByteArray

<dl>

<dt> size </dt>
<dd> Returns the number of bytes in the ByteArray. </dd>

<dt> [](<i>index</i>) </dt>
<dd> Returns the integer value of the byte at <i>index</i>. </dd>

<dt> []=(<i>index</i>, <i>value</i>) </dt>
<dd> Sets the byte at <i>index</i>. </dd>

<dt> as-string </dt>
<dd> Returns a String with the bytes in the ByteArray.  Assumes UTF-8 encoding. </dd>
	
</dl>



