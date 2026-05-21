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
X64_DIR = x64
X86_DIR = x86
ZIP_NAME = eamio-proxy.zip

DOCKER_IMAGE = eamio-proxy:latest

all: package

build-x64:
	mkdir -p $(OUT_DIR)/$(X64_DIR)
	$(ARCH_PREFIX_64)$(CC) $(CCFLAGS) $(DLL_FLAGS) $(INCLUDES) $(LDFLAGS) $(OPTIMIZEFLAGS) -o $(OUT_DIR)/$(X64_DIR)/$(DLL_NAME) src/eamio-proxy.cpp src/eamio-proxy.def

build-x86:
	mkdir -p $(OUT_DIR)/$(X86_DIR)
	$(ARCH_PREFIX_32)$(CC) $(CCFLAGS) $(DLL_FLAGS) $(INCLUDES) $(LDFLAGS) $(OPTIMIZEFLAGS) -o $(OUT_DIR)/$(X86_DIR)/$(DLL_NAME) src/eamio-proxy.cpp src/eamio-proxy.def

package: build-x64 build-x86
	cd $(OUT_DIR) && zip -r $(ZIP_NAME) $(X64_DIR) $(X86_DIR)

docker-build:
	docker build -t $(DOCKER_IMAGE) --target builder .
	docker run -v $$(pwd):/app $(DOCKER_IMAGE)

format:
	find src -name '*.cpp' -o -name '*.hpp' -o -name '*.c' -o -name '*.h' | xargs clang-format -i

.PHONY: all clean build-x64 build-x86 docker-build package format

clean:
	rm -rf $(OUT_DIR)
