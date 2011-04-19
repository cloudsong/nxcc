#
# just check is it is okay ..
# @date:2011-4-17
#

CC=gcc
CFLAGS=-O2

SOURCES=nxc_vm.c nxcc.c

TARGET=nxcc_demo
all:
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)
clean:
	rm -f $(TARGET)

