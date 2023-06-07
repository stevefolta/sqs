" Vim syntax file
" Language: sqs

if exists("b:current_syntax")
	finish
	endif

syn keyword sqsBoolean	true false
syn keyword sqsNil	nil

syn keyword sqsStatement	if else elif while for with fn class
syn keyword sqsStatement	return break continue
syn match sqsStatement	"\$"

syn match sqsComment	'#.*'

syn region sqsString matchgroup=sqsStringDelimiter start='"' skip='\\"' end='"' contains=sqsInterpolation
syn region sqsString matchgroup=sqsStringDelimiter start="'" skip="\\'" end="'" contains=sqsInterpolation
syn region sqsString matchgroup=sqsStringDelimiter start='`' skip='\\`' end='`' contains=sqsInterpolation
syn region sqsInterpolation matchgroup=sqsInterpolationDelimter start="{" end="}" contained

syn region sqsRawString matchgroup=sqsStringDelimiter start='r"' skip='\\"' end='"'
syn region sqsRawString matchgroup=sqsStringDelimiter start="r'" skip="\\'" end="'"
syn region sqsRawString matchgroup=sqsStringDelimiter start='r`' skip='\\`' end='`'

hi def link sqsStatement	Keyword
hi def link sqsBoolean	Boolean
hi def link sqsNil	Constant
hi def link sqsComment	Comment
hi def link sqsStringDelimiter	Delimiter
hi def link sqsString	String
hi def link sqsRawString	String
hi def link sqsInterpolation	Macro
hi def link sqsInterpolationDelimter	Delimiter

