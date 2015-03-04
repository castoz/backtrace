export TOOLCHAIN=/workshop/backtrace/build_dir/staging_dir/toolchain-mips_r2_gcc-4.6-linaro_uClibc-0.9.33.2/bin
export CC=$(TOOLCHAIN)/mips-openwrt-linux-uclibc-gcc
export LD=$(TOOLCHAIN)/mips-openwrt-linux-uclibc-ld
export OBJDUMP=$(TOOLCHAIN)/mips-openwrt-linux-uclibc-objdump

CFLAGS =-I/workshop/backtrace/build_dir/staging_dir/target-mips_r2_uClibc-0.9.33.2/usr/include -Os -Wall -Werror --std=gnu99 -Wmissing-declarations
LDFLAGS=-L/workshop/backtrace/build_dir/staging_dir/target-mips_r2_uClibc-0.9.33.2/usr/lib
export STAGING_DIR=/workshop/backtrace/build_dir/staging_dir

OBJS = backtrace.o main.o

testbt.o: $(OBJS)
	$(LD) -r -o $@ $(OBJS)

%.o:%.c
	$(CC) $(CFLAGS) -c -o $@ $<

uallsyms: uallsyms.c
	gcc uallsyms.c -o uallsyms

all: testbt.o uallsyms
	@echo Compile $@
	nm -n testbt.o | ./uallsyms > tmp_uallsyms1.S
	$(CC) -c tmp_uallsyms1.S -o uallsyms1.o
	$(CC) $(LDFLAGS) $(OBJS) uallsyms1.o -o testbt1.o
	nm -n testbt1.o | ./uallsyms > tmp_uallsyms2.S
	$(CC) -c tmp_uallsyms2.S -o uallsyms2.o
	$(LD) -n -r $(OBJS) uallsyms2.o -o testbt2.o
	$(CC) $(LDFLAGS) testbt2.o -o testbt
	ls -l testbt

clean:
	rm -f *.S *.o testbt uallsyms

