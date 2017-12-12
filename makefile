SHELL := /bin/bash

export PATH := $(PATH):$(shell pwd)/cross/bin

CC = x86_64-elf-gcc
CXX = x86_64-elf-g++
CC_FLAGS = -mno-red-zone -Wreturn-type #-g #-Wall -Werror #-mgeneral-regs-only
# -fno-rtti is no runtime type information since we don't have libstdc++
CXX_FLAGS = -fno-exceptions -mno-red-zone -Wreturn-type -fno-rtti -mcmodel=large #-g
NASM = nasm -f elf64 -g
GAS = x86_64-elf-as
LD = x86_64-elf-ld

LOOP_ONE = /dev/loop7
LOOP_TWO = /dev/loop8

KERNEL_SOURCE_DIR = src/kernel
KERNEL_BUILD_DIR = build/kernel
KERNEL_SOURCES_CXX = $(shell find $(KERNEL_SOURCE_DIR) -name "*.cc")
KERNEL_SOURCES_C = $(shell find $(KERNEL_SOURCE_DIR) -name "*.c")
KERNEL_SOURCES_GAS = $(shell find $(KERNEL_SOURCE_DIR) -name "*.s")
KERNEL_SOURCES_NASM = $(shell find $(KERNEL_SOURCE_DIR) -name "*.asm")
KERNEL_OBJECTS_CXX = $(addprefix $(KERNEL_BUILD_DIR)/,$(KERNEL_SOURCES_CXX:$(KERNEL_SOURCE_DIR)/%.cc=%.o))
KERNEL_OBJECTS_C = $(addprefix $(KERNEL_BUILD_DIR)/,$(KERNEL_SOURCES_C:$(KERNEL_SOURCE_DIR)/%.c=%.o))
KERNEL_OBJECTS_GAS = $(addprefix $(KERNEL_BUILD_DIR)/,$(KERNEL_SOURCES_GAS:$(KERNEL_SOURCE_DIR)/%.s=%.o))
KERNEL_OBJECTS_NASM = $(addprefix $(KERNEL_BUILD_DIR)/,$(KERNEL_SOURCES_NASM:$(KERNEL_SOURCE_DIR)/%.asm=%.o))
KERNEL_OBJECTS = $(KERNEL_OBJECTS_CXX) $(KERNEL_OBJECTS_C) $(KERNEL_OBJECTS_GAS) $(KERNEL_OBJECTS_NASM)

SHARED_SOURCE_DIR = src/shared
SHARED_BUILD_DIR = build/shared
SHARED_SOURCES_CXX = $(shell find $(SHARED_SOURCE_DIR) -name "*.cc")
SHARED_SOURCES_C = $(shell find $(SHARED_SOURCE_DIR) -name "*.c")
SHARED_SOURCES_GAS = $(shell find $(SHARED_SOURCE_DIR) -name "*.s")
SHARED_SOURCES_NASM = $(shell find $(SHARED_SOURCE_DIR) -name "*.asm")
SHARED_OBJECTS_CXX = $(addprefix $(SHARED_BUILD_DIR)/,$(SHARED_SOURCES_CXX:$(SHARED_SOURCE_DIR)/%.cc=%.o))
SHARED_OBJECTS_C = $(addprefix $(SHARED_BUILD_DIR)/,$(SHARED_SOURCES_C:$(SHARED_SOURCE_DIR)/%.c=%.o))
SHARED_OBJECTS_GAS = $(addprefix $(SHARED_BUILD_DIR)/,$(SHARED_SOURCES_GAS:$(SHARED_SOURCE_DIR)/%.s=%.o))
SHARED_OBJECTS_NASM = $(addprefix $(SHARED_BUILD_DIR)/,$(SHARED_SOURCES_NASM:$(SHARED_SOURCE_DIR)/%.asm=%.o))
SHARED_OBJECTS = $(SHARED_OBJECTS_CXX) $(SHARED_OBJECTS_C) $(SHARED_OBJECTS_GAS) $(SHARED_OBJECTS_NASM)

USER_SOURCE_DIR = src/user
USER_BUILD_DIR = build/user
USER_SOURCES_CXX = $(shell find $(USER_SOURCE_DIR) -name "*.cc")
USER_SOURCES_C = $(shell find $(USER_SOURCE_DIR) -name "*.c")
USER_SOURCES_GAS = $(shell find $(USER_SOURCE_DIR) -name "*.s")
USER_SOURCES_NASM = $(shell find $(USER_SOURCE_DIR) -name "*.asm")
USER_OBJECTS_CXX = $(addprefix $(USER_BUILD_DIR)/,$(USER_SOURCES_CXX:$(USER_SOURCE_DIR)/%.cc=%.o))
USER_OBJECTS_C = $(addprefix $(USER_BUILD_DIR)/,$(USER_SOURCES_C:$(USER_SOURCE_DIR)/%.c=%.o))
USER_OBJECTS_GAS = $(addprefix $(USER_BUILD_DIR)/,$(USER_SOURCES_GAS:$(USER_SOURCE_DIR)/%.s=%.o))
USER_OBJECTS_NASM = $(addprefix $(USER_BUILD_DIR)/,$(USER_SOURCES_NASM:$(USER_SOURCE_DIR)/%.asm=%.o))
USER_OBJECTS = $(USER_OBJECTS_CXX) $(USER_OBJECTS_C) $(USER_OBJECTS_GAS) $(USER_OBJECTS_NASM)

TEST_SOURCE_DIR = src/test
TEST_BUILD_DIR = build/test
TEST_SOURCES_CXX = $(shell find $(TEST_SOURCE_DIR) -name "*.cc")
TEST_OBJECTS = $(addprefix $(TEST_BUILD_DIR)/,$(TEST_SOURCES_CXX:$(TEST_SOURCE_DIR)/%.cc=%.o))
TEST_EXECS = $(addprefix $(TEST_BUILD_DIR)/,$(TEST_SOURCES_CXX:$(TEST_SOURCE_DIR)/%.cc=%.out))

UTIL_SOURCE_DIR = src/util
UTIL_BUILD_DIR = build/util
UTIL_SOURCES_CXX = $(shell find $(UTIL_SOURCE_DIR) -name "*.cc")
UTIL_OBJECTS = $(addprefix $(UTIL_BUILD_DIR)/,$(UTIL_SOURCES_CXX:$(UTIL_SOURCE_DIR)/%.cc=%.o))

all: run

.PHONY: run
run: os.img
	# TODO: -d int
	qemu-system-x86_64 -curses -drive format=raw,file=os.img -s
	#qemu-system-x86_64 -curses -serial stdio -drive format=raw,file=os.img -s
	#qemu-system-x86_64 -serial stdio -drive format=raw,file=os.img -s

os.img: image/boot/kernel.bin image/boot/grub/grub.cfg image/user/init
	-sudo umount /mnt/fatgrub
	-sudo losetup -d $(LOOP_TWO)
	-sudo losetup -d $(LOOP_ONE)
	-rm -f os.img build/os.img
	#cp build/kernel.bin image/boot/kernel.bin
	dd if=/dev/zero of=build/os.img bs=512 count=32768
	parted build/os.img mklabel msdos
	parted build/os.img mkpart primary fat32 2048s 30720s
	parted build/os.img set 1 boot on
	sudo losetup $(LOOP_ONE) build/os.img
	sudo losetup $(LOOP_TWO) build/os.img -o 1048576
	#sudo mkdosfs -F32 -f 2 /dev/loop1
	sudo mkdosfs -F32 -f 2 -S 512 -s 1 $(LOOP_TWO)
	sudo mkdir -p /mnt/fatgrub
	sudo mount $(LOOP_TWO) /mnt/fatgrub
	sudo grub-install --root-directory=/mnt/fatgrub --no-floppy --modules="normal part_msdos ext2 multiboot" $(LOOP_ONE)
	sudo cp -r image/* /mnt/fatgrub
	-sudo umount /mnt/fatgrub
	-sudo losetup -d $(LOOP_TWO)
	-sudo losetup -d $(LOOP_ONE)
	mv build/os.img os.img


image/boot/kernel.bin: $(KERNEL_SOURCE_DIR)/linker.ld $(KERNEL_OBJECTS) $(SHARED_OBJECTS) $(UTIL_OBJECTS)
	$(LD) -n -o $@ -T $< $(KERNEL_OBJECTS) $(SHARED_OBJECTS) $(UTIL_OBJECTS)

$(KERNEL_BUILD_DIR)/%.o: $(KERNEL_SOURCE_DIR)/%.asm
	$(NASM) $< -o $@ -g

$(KERNEL_BUILD_DIR)/%.o: $(KERNEL_SOURCE_DIR)/%.s
	$(GAS) -g -c $< -o $@

$(KERNEL_BUILD_DIR)/%.o: $(KERNEL_SOURCE_DIR)/%.c $(KERNEL_SOURCE_DIR)/*.h $(SHARED_SOURCE_DIR)/*.h
	$(CC) $(CC_FLAGS) -c $< -o $@ -I $(SHARED_SOURCE_DIR) -g -DKERNEL

$(KERNEL_BUILD_DIR)/%.o: $(KERNEL_SOURCE_DIR)/%.cc $(KERNEL_SOURCE_DIR)/*.h $(SHARED_SOURCE_DIR)/*.h
	$(CXX) $(CXX_FLAGS) -c $< -o $@ -I $(SHARED_SOURCE_DIR) -g -DKERNEL


# TODO support multiple user executables
image/user:
	mkdir image/user
image/user/init: $(USER_SOURCE_DIR)/linker.ld $(USER_OBJECTS) $(SHARED_OBJECTS) image/user $(UTIL_OBJECTS)
	$(LD) -n -o $@ -T $< $(USER_OBJECTS) $(SHARED_OBJECTS) $(UTIL_OBJECTS)

$(USER_BUILD_DIR)/%.o: $(USER_SOURCE_DIR)/%.asm
	$(NASM) $< -o $@

$(USER_BUILD_DIR)/%.o: $(USER_SOURCE_DIR)/%.s
	$(GAS) -g -c $< -o $@

$(USER_BUILD_DIR)/%.o: $(USER_SOURCE_DIR)/%.c $(USER_SOURCE_DIR)/*.h $(SHARED_SOURCE_DIR)/*.h
	$(CC) $(CC_FLAGS) -c $< -o $@ -I $(SHARED_SOURCE_DIR)

$(USER_BUILD_DIR)/%.o: $(USER_SOURCE_DIR)/%.cc $(USER_SOURCE_DIR)/*.h $(SHARED_SOURCE_DIR)/*.h
	$(CXX) $(CXX_FLAGS) -c $< -o $@ -I $(SHARED_SOURCE_DIR)


$(SHARED_BUILD_DIR)/%.o: $(SHARED_SOURCE_DIR)/%.asm
	$(NASM) $< -o $@

$(SHARED_BUILD_DIR)/%.o: $(SHARED_SOURCE_DIR)/%.s
	$(GAS) -g -c $< -o $@

$(SHARED_BUILD_DIR)/%.o: $(SHARED_SOURCE_DIR)/%.c $(SHARED_SOURCE_DIR)/*.h
	$(CC) $(CC_FLAGS) -c $< -o $@

$(SHARED_BUILD_DIR)/%.o: $(SHARED_SOURCE_DIR)/%.cc $(SHARED_SOURCE_DIR)/*.h
	$(CXX) $(CXX_FLAGS) -c $< -o $@


$(UTIL_BUILD_DIR)/%.o: $(UTIL_SOURCE_DIR)/%.cc
	$(CXX) $(CXX_FLAGS) -c $< -o $@


src/multi_interrupt_handlers.asm: build/multi_interrupt_handlers_generate
	build/multi_interrupt_handlers_generate > src/multi_interrupt_handlers.asm

src/irq_handlers.h: build/irq_handlers_generate
	build/irq_handlers_generate > src/irq_handlers.h

build/multi_interrupt_handlers_generate: src/multi_interrupt_handlers_generate.c
	gcc src/multi_interrupt_handlers_generate.c -o build/multi_interrupt_handlers_generate

build/irq_handlers_generate: src/irq_handlers_generate.c
	gcc src/irq_handlers_generate.c -o build/irq_handlers_generate


.PHONY: test
test: $(TEST_EXECS)

# TODO make this depend on all headers in all source dirs?
$(TEST_BUILD_DIR)/%.o: $(TEST_SOURCE_DIR)/%.cc
	$(CXX) $(CXX_FLAGS) -c $< -o $@ -I src/

$(TEST_BUILD_DIR)/%.out: $(TEST_BUILD_DIR)/%.o $(KERNEL_OBJECTS) $(SHARED_OBJECTS)
	#$(LD) -o $@ $< $(KERNEL_OBJECTS) $(SHARED_OBJECTS)
	#$(CXX) -o $@ $< $(KERNEL_OBJECTS) $(SHARED_OBJECTS)
	g++ -o $@ $< $(KERNEL_OBJECTS) $(SHARED_OBJECTS)
	./$@


.PHONY: clean
clean:
	-rm -f os.img build/os.img $(KERNEL_BUILD_DIR)/*.o $(USER_BUILD_DIR)/*.o $(SHARED_BUILD_DIR)/*.o $(TEST_BUILD_DIR)/*.o $(TEST_BUILD_DIR)/*.out
	-sudo umount /mnt/fatgrub
	-sudo losetup -d $(LOOP_TWO)
	-sudo losetup -d $(LOOP_ONE)
