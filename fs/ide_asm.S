#include <asm/regdef.h>
#include <asm/cp0regdef.h>
#include <asm/asm.h>

// more info about emulated IDE disks provided by Gexmul, see at http://gxemul.sourceforge.net/gxemul-stable/doc/experiments.html.

// Default physical address:  0x13000000
// Offset:     			Effect:
// 0x0000				Write: Set the offset (in bytes) from the beginning of the disk image. This offset will be used for the next read/write operation.
// 0x0008				Write: Set the high 32 bits of the offset (in bytes). (*)
// 0x0010				Write: Select the IDE ID to be used in the next read/write operation.
// 0x0020				Write: Start a read or write operation. (Writing 0 means a Read operation, a 1 means a Write operation.)
// 0x0030				Read: Get status of the last operation. (Status 0 means failure, non-zero means success.)
// 0x4000-0x41ff   		Read/Write: 512 bytes data buffer.

// read sector at specified offset from the beginning of the disk image.
LEAF(read_sector)
	sw  a0, 0x93000010 	// select the IDE id.
	sw	a1, 0x93000000  // offset.
	li	t0, 0
	sb	t0, 0x93000020  // start read.
	lw  v0, 0x93000030
	nop
	jr	ra
	nop
END(read_sector)

// write
LEAF(write_sector)
	// select the IDE id.
    // offset.

    // start write.
	// get status as return value.
END(write_sector)
