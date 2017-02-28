CC=gcc
CFLAGS=-g -lrados -lrbd -O0 -g3 -gdwarf-4

all:rados_write rados_read rados_rm rbd_read rbd_io
rados_write:rados_write.c
	$(CC) $< $(CFLAGS) -o $@
rados_read:rados_read.c
	$(CC) $< $(CFLAGS) -o $@
rados_rm:rados_rm.c
	$(CC) $< $(CFLAGS) -o $@
rbd_read:rbd_read.c
	$(CC) $< $(CFLAGS) -o $@
rbd_io:rbd_io.c
	$(CC) $< $(CFLAGS) -o $@

clean:
	rm -rf rados_write rados_read rados_rm rbd_read rbd_io
