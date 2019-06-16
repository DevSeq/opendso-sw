DIRS = lib tools

.PHONY: all clean $(DIRS)

clean: TARGET = clean

all clean: $(DIRS)
$(DIRS):
	CROSS_COMPILE=$(CROSS_COMPILE) $(MAKE) -C $@ $(TARGET) 
