
#include <net/checksum.h>
#include <asm/segment.h>
#include <asm/generic/memory.h>

/* #define DEBUG_PARTIAL_CSUM_FROM_USER */

#define MIN(a,b) (((a)<(b))?(a):(b))

inline unsigned int
add_with_carry(unsigned int x, unsigned int y)
{
     unsigned int temp;

     asm("adds	%1, %1, %0\n\t"
	 "addcs	%0, %0, #1\n\t"
	 : "=r" (temp)
	 : "r" (x), "0" (y)
	  );
     return temp;
}

__wsum csum_partial_copy_from_user(const void *src, void *dst, 
                                   int len, __wsum _sum, int *err_ptr)
{
     unsigned long page, offset;
     unsigned long len1, len2;
     unsigned long chksumgap;
     unsigned long sum = _sum;

     if (segment_eq(get_fs(),KERNEL_DS)) {
	  return csum_partial_copy_nocheck(src, dst, len, _sum);
     }

     page = parse_ptabs_read((unsigned long)src, &offset);
     if (page == -EFAULT)
	  goto exit_err;

     while (len) {
	  len1 = MIN(len, PAGE_SIZE - ((unsigned long) src & ~PAGE_MASK));
	  len2 = len - len1;

	  if (len2 && (len1 & 1)) {
#ifdef DEBUG_PARTIAL_CSUM_FROM_USER
	       herc_printf("unaligned checksumming src=%p, len=%x, len1=%x, len2=%x\n", src, len, len1, len2);
#endif
	       sum = csum_partial_copy_nocheck((char *)(page + offset), dst, len1 - 1, sum);
	       chksumgap = ((unsigned char*)page)[PAGE_SIZE - 1];
	    
	       src += len1 + 1;
	       dst += len1 + 1;
	    
	       page = parse_ptabs_read((unsigned long)src, &offset);
	       if (page == -EFAULT)
		    goto exit_err;
	    
	       chksumgap += ((unsigned char*)page)[0] << 8;
	       ((short*)dst)[-1] = chksumgap; /* copy data */

	       sum = add_with_carry(sum, chksumgap);
	       len -= len1 + 1;
	  } else {
	       sum = csum_partial_copy_nocheck((char *)(page + offset), dst, len1, sum);

	       src += len1;
	       dst += len1;
	       len -= len1;

	       if (len) {
		    page = parse_ptabs_read((unsigned long)src, &offset);
		    if (page == -EFAULT)
			 goto exit_err;
	       }
	  }
     }
     return sum;

 exit_err:
     *err_ptr = -EFAULT;
     return 0;
}

/*
 * Local Variables:
 * mode:c
 * c-file-style:"k&r"
 * c-basic-offset:8
 * End:
 */
