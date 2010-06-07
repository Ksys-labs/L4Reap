.text
.global __pthread_new_thread_entry
__pthread_new_thread_entry:
	ldr r0, [sp, #8]
	ldmia sp!, {pc}
