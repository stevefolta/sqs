" Vim filetype detect
" Language: sqs

fun! s:DetectSQS()
	if getline(1) == '#!/usr/bin/env sqs'
		set filetype=sqs
		setlocal iskeyword+=-
		setlocal nocindent
		endif
endfun

autocmd BufNewFile,BufRead * call s:DetectSQS()

