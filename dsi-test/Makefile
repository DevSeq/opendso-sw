include ../config.mk

CC=$(CROSS_COMPILE)gcc
OBJS=dsi-test.o dsi_core.o
LDFLAGS=-Lual/lib -lual -static
CFLAGS=-Iual/lib

all: ual testprog

ual/lib/libual.a:
	CROSS_COMPILE=$(CROSS_COMPILE) make -C ual all 

ual: ual/lib/libual.a

testprog: $(OBJS) 
	${CC} -o dsi-test $(OBJS) $(LDFLAGS)

clean:
	rm -f $(OBJS) dsi-test 
	make -C ual/ clean

scp:	clean all
#	ssh-keygen -f "/home/twl/.ssh/known_hosts" -R "zynq-dev2"
	scp dsi-test root@zynq-dev2:/root