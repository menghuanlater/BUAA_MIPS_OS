INCLUDES := -I../include

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

.PHONY: clean

all: init.o main.o code_a.o code_b.o code_c.o code_d.o code_e.o code_f.o code_g.o

clean:
	rm -rf *~ *.o


include ../include.mk

