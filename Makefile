ARCH_PREFIX_64 = x86_64-w64-mingw32-
ARCH_PREFIX_32 = i686-w64-mingw32-
CC=g++
CCFLAGS = -std=c++20
INCLUDES = -Isrc
LDFLAGS = -shared -static-libgcc -static-libstdc++
OPTIMIZEFLAGS = -s -Os -ffunction-sections -fdata-sections -Wl,--gc-sections
DEBUGFLAGS = -g -O0
DLL_NAME = eamio.dll

OUT_DIR = out
OUT_X64_DIR = $(OUT_DIR)/x64
OUT_X86_DIR = $(OUT_DIR)/x86

all: build-x64 build-x86

build-x64:
	mkdir -p $(OUT_X64_DIR)
	$(ARCH_PREFIX_64)$(CC) $(CCFLAGS) $(DLL_FLAGS) $(INCLUDES) $(LDFLAGS) $(OPTIMIZEFLAGS) -o $(OUT_X64_DIR)/$(DLL_NAME) src/eamio-proxy.cpp src/eamio-proxy.def

build-x86:
	mkdir -p $(OUT_DIR)/x86
	$(ARCH_PREFIX_32)$(CC) $(CCFLAGS) $(DLL_FLAGS) $(INCLUDES) $(LDFLAGS) $(OPTIMIZEFLAGS) -o $(OUT_X86_DIR)/$(DLL_NAME) src/eamio-proxy.cpp src/eamio-proxy.def

.PHONY: all clean build-x64 build-x86

clean:
	rm -rf $(OUT_DIR)
