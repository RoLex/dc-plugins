# This is a rudimentary Makefile that compiles files from the pluginsdk & src directories into a
# shared library. Adapt to your needs.

# Rename to your plugin's name.
TARGET = ScriptPlugin

CC = gcc
CXX = g++

CPPFLAGS += -Wall -Wextra -g -pipe -O3 -I../../
CXXFLAGS += -std=gnu++0x
LINKFLAGS += -static-libgcc -g -O3 -shared -Wl,--add-stdcall-alias
LINKXXFLAGS += -static-libstdc++

VPATH = ../../

OUTPUT_DIR = build

OBJS = \
	pluginsdk/Config.o \
	pluginsdk/Connections.o \
	pluginsdk/Core.o \
	pluginsdk/Hooks.o \
	pluginsdk/Hubs.o \
	pluginsdk/Logger.o \
	pluginsdk/Queue.o \
	pluginsdk/Tagger.o \
	pluginsdk/UI.o \
	pluginsdk/Util.o \
	src/LuaManager.o \
	src/main.o \
	src/Plugin.o \
	src/ScriptInstance.o \
	src/stdafx.o

COMPILER_SPEC = $(shell $(CC) -dumpmachine)

ifeq ($(findstring mingw, $(COMPILER_SPEC)),)
	BINEXT =
	LIBEXT = .so
else
	BINEXT = .exe
	CPPFLAGS += -D_WIN32_WINNT=0x502 -DWINVER=0x502 -D_WIN32_IE=0x600 \
				-DNOMINMAX -DSTRICT -DWIN32_LEAN_AND_MEAN \
				-DWIN32 -D_WIN32 -DUNICODE -D_UNICODE
	LIBEXT = .dll
	OBJS += src/resource.o
	OUTPUT_DIR := $(OUTPUT_DIR)-mingw
endif

ifeq ($(findstring x86_64, $(COMPILER_SPEC)),)
	CPPFLAGS += -march=i686
	OUTPUT_DIR := $(OUTPUT_DIR)-x86
else
	OUTPUT_DIR := $(OUTPUT_DIR)-x64
endif

ifeq ($(OS), Windows_NT)
	ifeq ($(findstring Cygwin, $(SHELL)),)
		# todo: donesnt work on modern cygwin
		MKDIR = if not exist $(subst /,\,$1) md $(subst /,\,$1)
		RMDIR = if exist $(subst /,\,$1) rd /s /q $(subst /,\,$1)
	else
		MKDIR = mkdir -p $1
		RMDIR = $(RM) -r $1
	endif
else
	MKDIR = mkdir -p $1
	RMDIR = $(RM) -r $1
endif

# boost
CPPFLAGS += -I../../boost
ifneq ($(findstring mingw, $(COMPILER_SPEC)),)
	CPPFLAGS += -DBOOST_ALL_NO_LIB -DBOOST_USE_WINDOWS_H
endif
OBJS += boost/libs/system/src/error_code.o

# lua
CPPFLAGS += -I../../lua
ifeq ($(findstring mingw, $(COMPILER_SPEC)),)
	CPPFLAGS += -DLUA_USE_LINUX=1
	LINKFLAGS += -lm -ldl
endif
OBJS += lua/lapi.o lua/lauxlib.o lua/lbaselib.o lua/lcode.o lua/lcorolib.o \
	lua/lctype.o lua/ldblib.o lua/ldebug.o lua/ldo.o lua/ldump.o lua/lfunc.o lua/lgc.o \
	lua/linit.o lua/liolib.o lua/llex.o lua/lmathlib.o lua/lmem.o lua/loadlib.o lua/lobject.o \
	lua/lopcodes.o lua/loslib.o lua/lparser.o lua/lstate.o lua/lstring.o lua/lstrlib.o \
	lua/ltable.o lua/ltablib.o lua/ltm.o lua/lundump.o lua/lutf8lib.o lua/lvm.o lua/lzio.o

ifeq ($(findstring mingw, $(COMPILER_SPEC)),)
	PDB =
else
	PDB := $(OUTPUT_DIR)/$(TARGET).pdb
endif

TARGET := $(OUTPUT_DIR)/$(TARGET)$(LIBEXT)
OBJS := $(addprefix $(OUTPUT_DIR)/, $(OBJS))

PACKAGER = $(OUTPUT_DIR)/packaging/packager$(BINEXT)
PACK_INFO = ../../packaging/info_generated.xml

all: $(TARGET) $(PDB) $(PACK_INFO)

$(TARGET): $(OBJS)
	$(CXX) $^ $(LINKFLAGS) $(LINKXXFLAGS) $(OUTPUT_OPTION)

ifneq ($(findstring mingw, $(COMPILER_SPEC)),)
$(PDB): $(TARGET)
	strip --only-keep-debug $< $(OUTPUT_OPTION) && strip $<
endif

$(PACKAGER): $(OUTPUT_DIR)/packaging/packager/packager.o
	$(CXX) $^ $(subst -shared,,$(LINKFLAGS) $(LINKXXFLAGS)) $(OUTPUT_OPTION)

$(PACK_INFO): $(PACKAGER)
	$(PACKAGER) $(PACK_INFO)

$(OUTPUT_DIR)/%.o: %.c
	$(COMPILE.c) $< $(OUTPUT_OPTION)

$(OUTPUT_DIR)/%.o: %.cpp
	$(COMPILE.cpp) $< $(OUTPUT_OPTION)

$(OUTPUT_DIR)/%.o: %.rc
	windres $< $(OUTPUT_OPTION)

$(OBJS): | $(OUTPUT_DIR)

$(PACKAGER): | $(OUTPUT_DIR)

$(OUTPUT_DIR):
	$(call MKDIR,$(OUTPUT_DIR)/packaging/packager)
	$(call MKDIR,$(OUTPUT_DIR)/pluginsdk)
	$(call MKDIR,$(OUTPUT_DIR)/src)
	$(call MKDIR,$(OUTPUT_DIR)/boost/libs/system/src)
	$(call MKDIR,$(OUTPUT_DIR)/lua)

clean:
	$(call RMDIR,$(OUTPUT_DIR))
