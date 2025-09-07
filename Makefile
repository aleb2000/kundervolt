obj-m += kundervolt.o
kundervolt-objs := module.o fp_util.o ftoa/ftoa.o
CFLAGS_fp_util.o := $(CC_FLAGS_FPU)
CFLAGS_REMOVE_fp_util.o += $(CC_FLAGS_NO_FPU)
CFLAGS_ftoa/ftoa.o := $(CC_FLAGS_FPU)
CFLAGS_REMOVE_ftoa/ftoa.o += $(CC_FLAGS_NO_FPU)

PWD := $(CURDIR)

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean

compile-commands: all
	python3 /usr/src/kernels/$(shell uname -r)/scripts/clang-tools/gen_compile_commands.py

install: all
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules_install
