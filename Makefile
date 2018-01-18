TARGET=serialfilter
TARGET_BIN=$(TARGET).ko
obj-m += $(TARGET).o
$(TARGET)-objs := filter_module.o debug_proc.o tty_interceptor.o filter.o
EXTRA_CFLAGS += -std=gnu99 -Wno-declaration-after-statement

all:
	make -C $(KERNEL_SRC) M=$(PWD) modules
	cp -fv $(TARGET).ko /tmp/$(TARGET).ko

clean:
	make -C $(KERNEL_SRC) M=$(PWD) clean
	rm -rfv *.mod.c *.mod.o *.o *.ko modules.order

scp:
	scp $(TARGET_BIN) $(DEVICE_SCP_PATH)
