#ifndef __INCLUDE__ASM_L4__GENERIC__L4LIB_H__
#define __INCLUDE__ASM_L4__GENERIC__L4LIB_H__

#include <linux/stringify.h>

#ifdef ARCH_arm
#define L4_EXTERNAL_FUNC(func) \
	asm(".section \".data.l4externals.str\"                         \n" \
	    "9: .string \"" __stringify(func) "\"                       \n" \
	    ".previous                                                  \n" \
	    \
	    "7: .long 9b                                                \n" \
	    \
	    "8: .long " __stringify(func##_resolver) "                  \n" \
	    \
	    ".globl " __stringify(func) "                               \n" \
	    ".weak " __stringify(func) "                                \n" \
	    ".type " __stringify(func) ", #function                     \n" \
	    ".type " __stringify(func##_resolver) ", #function          \n" \
	    __stringify(func) ":             ldr pc, 8b                 \n" \
	    __stringify(func##_resolver) ":  adr ip, 8b                 \n" \
	    "                                push {ip}                  \n" \
	    "                                ldr ip, 7b                 \n" \
	    "                                push {ip}                  \n" \
            "                                ldr ip, [pc]               \n" \
	    "                                ldr pc, [ip]               \n" \
	    "                           .long __l4_external_resolver    \n" \
	   )

#else
#ifdef CONFIG_X86_32
#define PSTOR ".long"
#else
#define PSTOR ".quad"
#endif
#define L4_EXTERNAL_FUNC(func) \
	asm(".section \".data.l4externals.str\"                         \n" \
	    "9: .string \"" __stringify(func) "\"                       \n" \
	    ".previous                                                  \n" \
	    \
	    ".section \".data.l4externals.symtab\"                      \n" \
	    "7: "PSTOR" 9b                                              \n" \
	    ".previous                                                  \n" \
	    \
	    ".section \".data.l4externals.jmptbl\"                      \n" \
	    "8: "PSTOR" " __stringify(func##_resolver) "                \n" \
	    ".previous                                                  \n" \
	    \
	    ".section \"" __stringify(.text.l4externals.fu##nc) "\"     \n" \
	    ".globl " __stringify(func) "                               \n" \
	    ".weak " __stringify(func) "                                \n" \
	    ".type " __stringify(func) ", @function                     \n" \
	    ".type " __stringify(func##_resolver) ", @function          \n" \
	    __stringify(func) ":            jmp *8b                     \n" \
	    __stringify(func##_resolver) ": push $8b                    \n" \
            "                               push $7b                    \n" \
	    "                               jmp *__l4_external_resolver \n" \
	    ".previous                                                  \n" \
	   )
#endif

#endif /* __INCLUDE__ASM_L4__GENERIC__L4LIB_H__ */
