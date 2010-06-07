.text
.global __pthread_new_thread_entry
__pthread_new_thread_entry:
	mov 16(%rsp), %rdi
	ret
