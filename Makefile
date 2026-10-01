# TinyOS - build with `make`, run with `make run`

# Use a proper cross-compiler if you have one, otherwise fall back to 32-bit host gcc.
ifneq ($(shell command -v i686-elf-g++ 2>/dev/null),)
  CXX    := i686-elf-g++
  LD     := i686-elf-ld
  ARCH   :=
  LDARCH :=
else
  CXX    := g++
  LD     := ld
  ARCH   := -m32
  LDARCH := -m elf_i386
endif
NASM := nasm

CXXFLAGS := $(ARCH) -std=c++17 -O2 -Wall -Wextra -Iinclude \
            -ffreestanding -fno-exceptions -fno-rtti -fno-stack-protector \
            -fno-pie -fno-pic -fno-threadsafe-statics -fno-use-cxa-atexit \
            -fno-asynchronous-unwind-tables -fno-tree-loop-distribute-patterns \
            -fcf-protection=none -mgeneral-regs-only
LDFLAGS  := $(LDARCH) -T linker.ld -z max-page-size=0x1000 --build-id=none -nostdlib

KERNEL := build/kernel.elf
ISO    := build/tinyos.iso

CPP_SRC := $(wildcard kernel/*.cpp drivers/*.cpp lib/*.cpp shell/*.cpp)
ASM_SRC := $(wildcard boot/*.asm)
OBJS    := $(patsubst %.cpp,build/%.o,$(CPP_SRC)) $(patsubst %.asm,build/%.o,$(ASM_SRC))
HEADERS := $(wildcard include/*.h)

.PHONY: all run iso run-iso test debug clean

all: $(KERNEL)

$(KERNEL): $(OBJS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
	@echo "Built $@"

build/%.o: %.cpp $(HEADERS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/%.o: %.asm
	@mkdir -p $(@D)
	$(NASM) -f elf32 $< -o $@

# Quickest way to try it: QEMU loads the Multiboot kernel directly.
run: $(KERNEL)
	qemu-system-i386 -kernel $(KERNEL) -serial stdio

# Bootable ISO (needs grub-mkrescue, xorriso, mtools)
iso: $(KERNEL)
	@mkdir -p build/isodir/boot/grub
	cp $(KERNEL) build/isodir/boot/kernel.elf
	cp iso/boot/grub/grub.cfg build/isodir/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO) build/isodir
	@echo "Built $(ISO)"

run-iso: iso
	qemu-system-i386 -cdrom $(ISO) -serial stdio

# Headless smoke test: boot, then check the serial log for the ready banner.
test: $(KERNEL)
	@rm -f build/serial.log
	@timeout 5 qemu-system-i386 -kernel $(KERNEL) -display none -serial file:build/serial.log -no-reboot || true
	@grep -q "TinyOS ready" build/serial.log && echo "PASS: kernel booted" || { echo "FAIL: kernel did not boot"; cat build/serial.log; exit 1; }

# Wait for gdb on localhost:1234:  gdb build/kernel.elf -ex "target remote :1234"
debug: $(KERNEL)
	qemu-system-i386 -kernel $(KERNEL) -serial stdio -s -S

clean:
	rm -rf build
