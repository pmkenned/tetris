CC=gcc
CFLAGS=-Wall -Werror -Wextra -Wpedantic
CPPFLAGS=-I/usr/include/SDL2
TARGET=tetris
BUILD_DIR=./build

UNAME := $(shell uname | grep -oE "MINGW32_NT|CYGWIN_NT|Linux|Darwin")

ifeq ($(OS),Windows_NT)
	KERNEL := Windows
else
	KERNEL := $(UNAME)
endif
ENV := $(UNAME)

ifeq ($(KERNEL),Windows)
	ifeq ($(ENV), CYGWIN_NT)
		CPPFLAGS += -D CYGWIN
	endif
	ifeq ($(ENV), MINGW32_NT)
		CPPFLAGS += -D MINGW -D _WIN32_WINNT=_WIN32_WINNT_WIN7 -Dmain=SDL_main
		LDLIBS += -lmingw32 -lSDL2main -lSDL2 -mwindows
		LDFLAGS += -L/usr/lib -mwindows
		LDFLAGS += -mconsole
	endif
	TARGET := $(TARGET).exe
endif
ifeq ($(KERNEL),Linux)
	CPPFLAGS += -D LINUX -D_REENTRANT
	LDLIBS += -lpthread -lSDL2
	LDFLAGS += -L/usr/lib
endif
ifeq ($(KERNEL),Darwin)
	CPPFLAGS += -D OSX
endif

SRC = main.c \
	  piece.c \
	  tetris.c
OBJ = $(SRC:%.c=$(BUILD_DIR)/%.o)
DEP = $(OBJ:%.o=%.d)

.PHONY: all clean

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJ)
	mkdir -p $(@D)
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)
	ln -sf $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(@D)
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -c $< -o $@

-include $(DEP)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
