TARGET = kernel.elf
OBJS = main.o

CXXFLAGS += -O2 -Wall -g --target=x86_64-elf -ffreestanding -mno-red-zone \
            -fno-exceptions -fno-rtti -std=c++17
LDFLAGS  += --entry KernelMain -z norelro --image-base 0x100000 --static


.PHONY: all
all: $(TARGET)

.PHONY: clean
clean:
	rm -rf *.o

kernel.elf: $(OBJS) Makefile
	/home/nonoka/workspace/clang+llvm-7.1.0-x86_64-linux-gnu-ubuntu-14.04/bin/ld.lld $(LDFLAGS) -o kernel.elf $(OBJS)

%.o: %.cpp Makefile
	/home/nonoka/workspace/clang+llvm-7.1.0-x86_64-linux-gnu-ubuntu-14.04/bin/clang++ $(CPPFLAGS) $(CXXFLAGS) -c $<
