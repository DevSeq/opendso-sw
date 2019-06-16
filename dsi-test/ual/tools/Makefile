-include Makefile.specific
LIBUAL = ..
CFLAGS += -Wall -Werror -g -ggdb
CFLAGS += -I$(LIBUAL)/lib
CFLAGS += $(EXTRACFLAGS)
LDLIBS += -Wl,-Bstatic -L$(LIBUAL)/lib -lual
LDLIBS += -Wl,-Bdynamic -lrt

CC=$(CROSS_COMPILE)gcc

PROGS := ualmem

ifeq ($(CONFIG_VME), y)
CFLAGS += -DCONFIG_VME
endif

all: $(PROGS)

clean:
	rm -f $(PROGS) *.o *~

.PHONY: all, clean
