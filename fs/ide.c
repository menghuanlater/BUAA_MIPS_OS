/*
 * operations on IDE disk.
 */

#include "fs.h"
#include "lib.h"
#include <mmu.h>

extern int read_sector(int diskno, int offset);
extern int write_sector(int diskno, int offset);

// Overview:
// 	read data from IDE disk.
//
// Parameters:
//	diskno: disk number.
// 	secno: start sector number.
// 	dst: destination for data read from IDE disk.
// 	nsecs: the number of sectors to read.
//
// Post-Condition:
// 	If error occurred during read the IDE disk, panic. 
void
ide_read(u_int diskno, u_int secno, void *dst, u_int nsecs)
{
	// 0x200: the size of a sector: 512 bytes.
	int offset_begin = secno * 0x200;
	int offset_end = offset_begin + nsecs * 0x200;
	int offset = 0;

	while (offset_begin + offset < offset_end) {
		if (read_sector(diskno, offset_begin + offset)) {
			// copy data from disk buffer(512 bytes, a sector) to destination array.
			user_bcopy((void *)0x93004000, dst + offset, 0x200);
			offset += 0x200;
		} else {
			// error occurred, then panic.
			user_panic("disk I/O error");
		}
	}
}


// Overview:
// 	write data to IDE disk.
//
// Parameters:
//	diskno: disk number.
//	secno: start sector number.
// 	src: the source data to write into IDE disk.
//	nsecs: the number of sectors to write.
//
// Post-Condition:
//	If error occurred during read the IDE disk, panic.
void
ide_write(u_int diskno, u_int secno, void *src, u_int nsecs)
{
	int offset_begin = secno * 0x200;
	int offset_end = offset_begin + nsecs * 0x200;
	int offset = 0;
	writef("diskno: %d\n", diskno);
	while ( offset_begin + offset < offset_end ) {
		//首先将要写入的数据写入设备缓冲区
		user_bcopy(src+offset,(void *)0x93004000,0x200);
		if(write_sector(diskno,offset_begin+offset)){
		// copy data from source array to disk buffer.
			offset+=0x200;//偏移量自增
		}else{
        // if error occur, then panic.	
			user_panic("disk I/O error\n");
		}
	}
}

