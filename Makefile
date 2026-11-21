
obj-m += mymod.o

# Kernel build directory
KDIR ?= /home/student/rpi-buildroot-2026.08/output/build/linux-custom

# Build external kernel module
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# Clean generated files
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean