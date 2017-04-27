/*
 * Minimal PIO-based (non-interrupt-driven) IDE driver code.
 * For information about what all this IDE/ATA magic means,
 * see for example "The Guide to ATA/ATAPI documentation" at:
 *	http://www.stanford.edu/~csapuntz/ide.html
 */

#include "fs.h"
#include "lib.h"
#include <mmu.h>

void
ide_read(u_int diskno, u_int secno, void *dst, u_int nsecs)
{
	int offset_begin = secno * 0x200;
	int offset_end = offset_begin + nsecs * 0x200;
	int offset = 0;

	while(offset_begin + offset < offset_end)
	{
		//writef("ide.c: read_sector() offset=%x\n",offset_begin + offset);
		if (read_sector(offset_begin + offset))
		{
			user_bcopy( 0x93004000,dst + offset, 0x200);
			offset += 0x200;
			//user_panic("$$$$$$$$$$$$$$$$$$$$$$$");
			
		} else {
			user_panic("disk I/O error");
		}
	}
}

void
ide_write(u_int diskno, u_int secno, void *src, u_int nsecs)
{
	int offset_begin = secno * 0x200;
	int offset_end = offset_begin + nsecs * 0x200;
	int offset = 0;

	while (offset_begin + offset < offset_end)
	{
		//writef("ide_write(): offset_begin:%x offset:%x src:%x\n",offset_begin,offset,src);
		user_bcopy(src + offset,0x93004000,  0x200);
		if (write_sector(offset_begin + offset))
		{			
			offset += 0x200;
		} else {
			user_panic("disk I/O error");
		}
	}
}

