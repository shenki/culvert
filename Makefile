# A simple makefile for building culvert
#
# Useful for when the distro can't support building with meson
#

SRC := $(wildcard src/*.c src/cmd/*.c src/uart/*.c src/bridge/*.c src/soc/*.c src/soc/uart/*.c)

CCAN_SRC := src/ccan/noerr/noerr.c src/ccan/ptr_valid/ptr_valid.c \
	    src/ccan/autodata/autodata.c src/ccan/str/debug.c src/ccan/str/str.c \
	    src/ccan/list/list.c

PPC64_SRC := arch/ppc64/lpc.c
AMD64_SRC := src/arch/x86_64/lpc.c

DTS := src/devicetree/g4.dts src/devicetree/g5.dts src/devicetree/g6.dts

ALL_SRC := $(SRC) $(CCAN_SRC)
OBJ := $(ALL_SRC:.c=.o) $(DTS:.dts=.dtb.o)

DTC := dtc
GIT_VERSION := "$(shell git describe --tags --dirty --always)"

CFLAGS = -std=gnu99 -Wall -O2 -Isrc/ -ggdb -Isrc/soc -Isrc/bridge -Isrc/cmd
LDLIBS = -lfdt

BIN := src/culvert

$(BIN): $(OBJ)

$(ALL_SRC): src/config.h

src/config.h: src/config.h.in
	cp $^ $@ && sed -i 's/@have_lpc@/0/' $@ && sed -i 's/@version@/$(GIT_VERSION)/' $@

%.dts.i: %.dts
	$(CC) -E -x assembler-with-cpp -nostdinc -Isrc $^ -o $@

%.dtb: %.dts.i
	$(DTC) $^ -O dtb -o $@

%.dtb.o: %.dtb
	$(LD) -z noexecstack -r -b binary $^ -o $@

.PHONY:
clean:
	$(RM) $(OBJ) src/config.h $(BIN)
