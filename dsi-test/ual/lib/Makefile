-include Makefile.specific
CONFIG_PCI ?= y
CONFIG_VME ?= n

CFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\"
CFLAGS += -Wall -Werror -ggdb -O2 -fPIC

LIB = libual.o
LIB_A = libual.a
LIB_SO = libual.so

LOBJ := bus.o
LOBJ += bus-pci.o
LOBJ += bus-rawmem.o
LOBJ += route.o
LOBJ += irq.o

LIB_DEP :=

ifeq ($(CONFIG_VME), y)
LIBVME_A = $(VMEBRIDGE)/lib
LIBVME_H = $(VMEBRIDGE)/include
CFLAGS += -DCONFIG_VME
CFLAGS += -I$(LIBVME_H) -I$(LIBVME_A)
LDFLAGS += -L$(LIBVME_A) -lvmebus
LOBJ += bus-vme.o
LIB_DEP += $(LIBVME_A)/libvmebus.a
endif

LIB_DEP += $(LOBJ)

CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
RANLIB = $(CROSS_COMPILE)ranlib
MV ?= mv

all: libs

libs: $(LIB_A) $(LIB_SO)

%.o:%.c
	$(CC) $(CFLAGS) -c $^ -o $@

$(LIB_A): $(LIB_DEP)
ifeq ($(CONFIG_VME), y)
	$(AR) r tmp$@ $(LOBJ)
# transform a thin library into a normal one
	echo -e "create $@\naddlib tmp$@\naddlib $(LIBVME_A)/libvmebus.a\nsave\nend" | $(AR) -M
	$(RM) tmp$@
else
	$(AR) r $@ $(LOBJ)
endif
	$(RANLIB) $@

$(LIB_SO): $(LIB_A)
	$(CC) -shared -o $@ -Wl,--whole-archive,-soname,$@.$(VERSION) $^ -Wl,--no-whole-archive

clean:
	-$(RM) -rf $(LIB) $(LIB_A) $(LIB_SO)
	-$(RM) -rf *.{a,so,o}
	-$(RM) -rf html Doxyfile Doxyfile.template

cleanall: clean
