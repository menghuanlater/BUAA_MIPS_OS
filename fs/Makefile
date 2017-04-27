INCLUDES = -I../include/ -I../user/

#CROSS_COMPILE := /usr/eldk/usr/bin/mips_4KC-
#CC			  := $(CROSS_COMPILE)gcc
#CFLAGS		  := -O -G 0 -mno-abicalls -fno-builtin -Wa,-xgot -Wall -fPIC
#LD			  := $(CROSS_COMPILE)ld
# For embedding one program in another

USERDIR := ../user/
USERLIB := ../user/printf.o \
		../user/print.o \
		../user/libos.o \
		../user/fork.o \
		../user/pgfault.o \
		../user/syscall_lib.o \
		../user/ipc.o \
		../user/string.o \
		../user/fd.o \
		../user/pageref.o \
		../user/file.o \
		../user/fsipc.o \
		../user/wait.o \
		../user/spawn.o \
		../user/pipe.o \
		../user/console.o \
		../user/fprintf.o

FSLIB :=	fs.o \
		ide.o \
		ide_asm.o \
		test.o

FSIMGFILES := 	motd \
			newmotd \
			../user/testfdsharing.b \
			../user/testarg.b \
			../user/init.b \
			../user/sh.b \
			../user/cat.b \
			../user/ls.b


CFLAGS += -nostdlib -static

all: serv.x fs.img serv.b

%.x: %.b.c
	echo cc1 $<
	$(CC) $(INCLUDES) $(CFLAGS) -c -o $@ $<

%.b.c: %.b
	echo create $@
	echo bintoc $* $< > $@~
	./bintoc $* $< > $@~ && mv -f $@~ $@
#	grep \. $@
	
%.b: ../user/entry.o ../user/syscall_wrap.o %.o $(USERLIB) $(FSLIB)
	echo ld $@
	$(LD) -o $@ $(LDFLAGS) -G 0 -static -n -nostdlib -T $(USERDIR)user.lds $^

%.o: %.c
	echo user1 cc $<
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

%.o: %.S
	echo as $<
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<


%.o: $(USERDIR)lib.h

fs.img: $(FSIMGFILES)
	dd if=/dev/zero of=../gxemul/fs.img bs=4096 count=1024 2>/dev/null
	./fsformat ../gxemul/fs.img $(FSIMGFILES)

.PHONY: clean

clean:
	rm -rf *~ *.o *.b.c *.x

include ../include.mk
