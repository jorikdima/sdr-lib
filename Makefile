ifeq ($(OS),Windows_NT)
    SYSTEM = WIN
    ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
        ARCHITECTURE = x86_64
    else
        ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
            ARCHITECTURE = x86_64
        endif
        ifeq ($(PROCESSOR_ARCHITECTURE),x86)
            ARCHITECTURE = x86
        endif
    endif
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        SYSTEM = LINUX
    endif
    ifeq ($(UNAME_S),Darwin)
        SYSTEM = OSX
    endif
    UNAME_P := $(shell uname -p)
    ifeq ($(UNAME_P),x86_64)
        ARCHITECTURE = x86_64
    endif
    ifneq ($(filter %86,$(UNAME_P)),)
       ARCHITECTURE = x86
    endif
    ifneq ($(filter arm%,$(UNAME_P)),)
        ARCHITECTURE = ARM
    endif
endif

ifeq ($(SYSTEM), WIN)
# === Windows ===
ifeq ($(ARCHITECTURE), x86_64)
	CROSS_COMPILE := x86_64-w64-mingw32-
else
	CROSS_COMPILE := i686-w64-mingw32-
endif
AS = $(CROSS_COMPILE)as
LD = $(CROSS_COMPILE)ld
CC = $(CROSS_COMPILE)gcc-posix
CXX = $(CROSS_COMPILE)g++-posix
OBJDUMP = $(CROSS_COMPILE)objdump

CXXLIBS = -static -lstdc++
# === End of Windows ===
else
# === Linux & macOS ===
ifeq ($(SYSTEM), OSX)
# == macOS ==
ARCH=-m64
CXXLIBS = -lc++
# == End of macOS ==
else
# == Linux ==
ifeq ($(ARCHITECTURE), x86_64)
ARCH=-m64
else
ARCH=-m32
endif
LIBS += -pthread -lrt
CXXLIBS = -lstdc++
# == End of Linux ==
endif
# === End of Linux & macOS ===
endif


LIBS += -L ./ftdi/linux-x86_64 -lftd3xx
COMMON_FLAGS = -ffunction-sections -fmerge-all-constants $(ARCH)
COMMON_CFLAGS = -g -O3 -Wall -Wextra $(COMMON_FLAGS)
CFLAGS = -std=c99  $(COMMON_CFLAGS) -D_POSIX_C_SOURCE
CXXFLAGS = -std=c++11 $(COMMON_CFLAGS)

INCLUDES_PATH=inc
SRC_PATH=src
BUILD_PATH=build
TARGET=test


all: clean $(TARGET)
	
$(TARGET): streamer.o
	$(info "Building for: " $(SYSTEM) $(ARCHITECTURE))
	$(CC) $(COMMON_FLAGS) -o $(BUILD_PATH)/$@ $(BUILD_PATH)/$^ $(CXXLIBS) $(LIBS)	
		
%.o: $(SRC_PATH)/%.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) -I $(INCLUDES_PATH) -o $(BUILD_PATH)/$@ $^
		
clean:
	-rm -f $(BUILD_PATH)/*
