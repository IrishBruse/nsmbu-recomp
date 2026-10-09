# Builds the example guest mods: mod.c -> mod.elf (PowerPC relocatable ELF, platform independent).
# Needs a clang with the PowerPC target and ld.lld, e.g. Homebrew `llvm` + `lld` on macOS, the LLVM
# MSYS2 CLANG64 build on Windows or distro LLVM on Linux. Apple's clang has no PowerPC target.
CLANG ?= clang
SDK ?= ../../runtime/guest/include

ifndef LLD
  LLD := $(shell \
    d="$$(dirname "$$(command -v '$(CLANG)' 2>/dev/null)")/"; \
    if [ -x "$$d/ld.lld" ]; then echo "$$d/ld.lld"; exit 0; fi; \
    for p in /usr/lib/llvm-*/bin/ld.lld; do \
      if [ -x "$$p" ]; then echo "$$p"; exit 0; fi; \
    done; \
    for x in ld.lld ld.lld-21 ld.lld-20 ld.lld-19 ld.lld-18; do \
      p="$$(command -v $$x 2>/dev/null)"; \
      if [ -n "$$p" ]; then echo "$$p"; exit 0; fi; \
    done)
endif

ifeq ($(LLD),)
  $(error Guest mods need ld.lld. Debian/Ubuntu: sudo apt install clang lld. macOS: brew install llvm lld, then make CLANG=/opt/homebrew/opt/llvm/bin/clang LLD=/opt/homebrew/opt/lld/bin/ld.lld)
endif

CFLAGS = --target=powerpc-unknown-eabi -mcpu=750 -O2 -ffreestanding -fno-builtin -nostdlib \
         -fno-jump-tables -ffunction-sections -fdata-sections -I$(SDK)
MODS = play-scene-ticker smooth-step-replace

all: $(MODS:%=%/mod.elf)
%/mod.o: %/mod.c $(SDK)/nsmbu_guest.h $(wildcard $(SDK)/nsmbu/*.h)
	$(CLANG) $(CFLAGS) -c $< -o $@
%/mod.elf: %/mod.o
	$(LLD) -m elf32ppc -r $< -o $@
clean:
	rm -f $(MODS:%=%/mod.o) $(MODS:%=%/mod.elf)
