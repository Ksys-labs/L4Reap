#
# Makefile for l4lib
#

# note: task_gen.o not needed for l4env version
lib-y := thread_gen.o irq_gen.o legacy.o
lib-$(CONFIG_L4_IRQ_SINGLE) += irq_timer.o
