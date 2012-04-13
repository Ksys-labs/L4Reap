#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/mm.h>

#include <asm-generic/sections.h>
#include <asm/e820.h>

#include <asm/generic/setup.h>

char * __init l4x_memory_setup(void)
{
	unsigned long mem_start, mem_size, isa_start, isa_size;
	unsigned long textbegin = (unsigned long)&_stext;

	l4x_setup_memory(boot_command_line, &mem_start, &mem_size,
	                 &isa_start, &isa_size);

	max_pfn_mapped = (mem_start + mem_size + ((1 << 12) - 1)) >> 12;

	e820.nr_map = 0;

        /* minimum 2 pages required */
	if (isa_size) {
		e820_add_region(0, isa_start,
		                E820_UNUSABLE);
		e820_add_region(isa_start, isa_size, E820_RAM);
		e820_add_region(isa_start + isa_size,
		                textbegin - (isa_start + isa_size),
		                E820_UNUSABLE);
	} else
		e820_add_region(0, textbegin, E820_RESERVED);

	if ((unsigned long)&_end > mem_start)
		printk("Uh, something looks strange.\n");
	e820_add_region(textbegin, (unsigned long)&_end - textbegin,
	                E820_RESERVED_KERN);
	e820_add_region((unsigned long)&_end,
	                mem_start - (unsigned long)&_end, E820_UNUSABLE);
	e820_add_region(mem_start, mem_size, E820_RAM);

	sanitize_e820_map(e820.map, ARRAY_SIZE(e820.map), &e820.nr_map);

	return "L4Lx-Memory";
}
