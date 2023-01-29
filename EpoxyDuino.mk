# Include this Makefile to compile an Arduino *.ino file on Linux or MacOS.

# vim: ts=4:noexpandtab
#
# Create a 'Makefile' in the sketch folder. For example, for the
# Blink/Blink.ino program, the makefile will be 'Blink/Makefile'.
# The content will look like this:
#
#      APP_NAME := {name of *.ino file}
#      ARDUINO_LIBS := AUnit AceTime {... additional Arduino libraries}
#      include ../../../EpoxyDuino/EpoxyDuino.mk
#
# The 2 required parameters are:
#
#   * APP_NAME
#       * Base name of the Arduino sketch file, e.g. 'Blink' not 'Blink.ino'
#   * ARDUINO_LIBS
#       * List of dependent Arduino libraries in sibling directories to
#         EpoxyDuino (e.g. AUnit).
#       * The EpoxyDuino directory is automatically included.
#
# Optional parameters are:
#
#    * ARDUINO_LIB_DIRS
#       * List of additional locations of Arduino libs
#       * For example, $(arduino_ide_dir)/libraries,
#         $(arduino_ide_dir)/hardware/arduino/avr/libraries,
#         $(arduino_ide_dir)/portable/packages/arduino/hardware/avr/1.8.2/libraries.
#       * (The $(arduino_ide_dir) is an example temporary variable containing
#         the install location of the Arduino IDE. It is not used by
#         EpoxyDuino.mk.)
#   * OBJS
#       * Additional object (*.o) files needed by the binary
#	* DEPS
#		* Additional source (*.h, *.cpp) files required by the final executable.
#		* These are additional source files which should trigger a recompile
#		  when they are modified.
#   * GENERATED
#       * A list of files which are generated by a script, and therefore can be
#          deleted by 'make clean'
#   * MORE_CLEAN
#       * Optional user-supplied make-target that performs additional cleanup
#          (i.e. removing generated files or directories).
#   * EPOXY_CORE
#       * The C macro to select a specific core. Valid options are:
#			* EPOXY_CORE_AVR (default)
#           * EPOXY_CORE_ESP8266
#	* EPOXY_CORE_PATH
#       * Select the alternate Core given by this full path.
#       * Default: $(EPOXY_DUINO_DIR)/cores/epoxy
#
# Type 'make -n' to verify.
#
# Type 'make' to create the $(APP_NAME).out program.
#
# Type 'make clean' to remove intermediate files.

# Detect Linux or MacOS
UNAME := $(shell uname)

# EpoxyDuino directory.
EPOXY_DUINO_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
# Look for libraries under ./EpoxyDuino/libraries/
EPOXY_DUINO_LIB_DIR := $(abspath $(EPOXY_DUINO_DIR)/libraries)
# Look for libraries which are siblings to ./EpoxyDuino/
EPOXY_DUINO_PARENT_DIR := $(abspath $(EPOXY_DUINO_DIR)/..)

# List of Arduino IDE library folders, both built-in to the Arduino IDE
# and those downloaded later, e.g. in the portable/ directory or .arduino15/
# directory.
ARDUINO_LIB_DIRS ?=

# CPP macro to select a specific core. Valid options are:
# EPOXY_CORE_AVR (default), and EPOXY_CORE_ESP8266.
EPOXY_CORE ?= EPOXY_CORE_AVR

# Define the directory where the <Arduino.h> and other core API files are
# located. The default is $(EPOXY_DUINO_DIR)/cores/epoxy.
EPOXY_CORE_PATH ?= $(EPOXY_DUINO_DIR)/cores/epoxy

# Find the directory paths of the libraries listed in ARDUINO_LIBS by looking
# at the following:
# 1) under the directory given by EPOXY_DUINO_LIB_DIR,
# 2) the sibling directories under EPOXY_DUINO_PARENT_DIR, and
# 3) each directory listed in ARDUINO_LIB_DIRS.
EPOXY_MODULES := $(foreach lib,$(ARDUINO_LIBS),${EPOXY_DUINO_LIB_DIR}/${lib})
EPOXY_MODULES += $(foreach lib,$(ARDUINO_LIBS),${EPOXY_DUINO_PARENT_DIR}/${lib})
EPOXY_MODULES += \
	$(foreach lib_dir,$(ARDUINO_LIB_DIRS),\
		$(foreach lib,$(ARDUINO_LIBS),\
			${lib_dir}/${lib}\
		)\
	)

# Compiler settings that depend on the OS (Linux, MacOS, FreeBSD). I'm not 100%
# sure that these flags are correct, but they seem to work. Previously, I was
# using just -Wall to catch the warnings, but some Arduino compilers seem to
# enable -Wextra when "enable warnings" is enabled. So let's add -Wextra in
# EpoxyDuino to help catch warnings on desktop machines as well.
#
# The difference between -stdlib=libstdc++ and -stdlib=libc++ seems to be that
# libstdc++ was created by the GNU team and libc++ was created by the LLVM
# team. Apple no longer distributes the latest version of the libstdc++, so we
# are forced to use libc++ on Macs.
#
# The Linux g++ flags came from looking at the verbose compiler output of the
# Arduino IDE. The g++ compiler automatically uses -stdlib=libstdc++. I copied
# the -fno-exceptions, -fno-threadsafe-statics, and -flto flags from the
# Arduino compiler output. But I'm not entirely sure that those flags are
# actually doing the right without linking to a version of the libstdc++ that
# was also compiled with the same flag. I don't know, things seem to work for
# now.
#
# The Mac clang++ flags came from... I can't remember. Some trial and error,
# and maybe this StackOverflow https://stackoverflow.com/questions/19774778).
# It looks like the clang++ compiler supports the -std=gnu++11 flag, but I
# suspect that it requires using the latest libstdc++ library, which isn't
# available on Macs. So I'll be conservative and use just -std=c++11.
#
# The FreeBSD clang++ flags came from copying the Mac version, since it seems
# like c++ on FreeBSD is actualy clang++. It's possible that FreeBSD has the
# latest GNU version of libstdc++, but I'm not sure.
#
# The CFLAGS assume that any C files are C11 files (not C99) because my
# AceTimeC library requires C11. It may be possible to override that by passing
# the `-std=c99` flag in the EXTRA_CFLAGS variable (which overrides the
# `-std=c11` flag), but this has not been tested.
#
# I added EXTRA_CXXFLAGS and EXTRA_CFLAGS to allow end-user Makefiles to
# specify additional flags to CXXFLAGS and CFLAGS.
ifeq ($(UNAME), Linux)
CXX ?= g++
CVER ?=14
CXXFLAGS ?= -Wextra -Wall -std=c++$(CVER) -flto \
	-fno-exceptions -fno-threadsafe-statics
CFLAGS ?= -Wextra -Wall -std=c$(CVER) -flto
else ifeq ($(UNAME), Darwin)
CXX ?= clang++
CXXFLAGS ?= -Wextra -Wall -std=c++$(CVER) -stdlib=libc++
CFLAGS ?= -Wextra -Wall -std=c$(CVER)
else ifeq ($(UNAME), FreeBSD)
CXX ?= clang++
CXXFLAGS ?= -Wextra -Wall -std=c++($C) -stdlib=libc++
CFLAGS ?= -Wextra -Wall -std=c$(CVER)
endif
CXXFLAGS += $(EXTRA_CXXFLAGS)
CFLAGS += $(EXTRA_CFLAGS)

# Pre-processor flags (-I, -D, etc), mostly for header files.
CPPFLAGS += $(EXTRA_CPPFLAGS)
# Define a macro to indicate that EpoxyDuino is being used. Defined here
# instead of Arduino.h so that files like 'compat.h' can determine the
# compile-time environment without having to include <Arduino.h>.
# Also define UNIX_HOST_DUINO for backwards compatibility.
CPPFLAGS += -D ARDUINO=100 -D UNIX_HOST_DUINO -D EPOXY_DUINO -D $(EPOXY_CORE)
# Add the header files for the Core files.
CPPFLAGS += -I$(EPOXY_CORE_PATH)
# Add the header files for libraries. Old Arduino libraries place the header
# and source files right at the top. New Arduino libraries tend to use the
# ./src/ subdirectory. We need to support both.
CPPFLAGS_EXPANSION = -I$(module) -I$(module)/src
CPPFLAGS += $(foreach module,$(EPOXY_MODULES),$(CPPFLAGS_EXPANSION))

# Linker settings (e.g. -lm).
LDFLAGS ?= -lpthread

# Collect list of C and C++ srcs to compile.
#
# 1) Collect the source files in the Epoxy Core directory. Support subdirectory
# expansions up to 3 levels below the given target. (There might be a better
# way to do this using GNU Make but I can't find a mechanism that doesn't barf
# when the 'src/' directory doesn't exist.)
EPOXY_SRCS := $(wildcard $(EPOXY_CORE_PATH)/*.cpp) \
	$(wildcard $(EPOXY_CORE_PATH)/*/*.cpp) \
	$(wildcard $(EPOXY_CORE_PATH)/*/*/*.cpp) \
	$(wildcard $(EPOXY_CORE_PATH)/*/*/*/*.cpp)
# 2) Collect the source files of the libraries referenced by the EPOXY_MODULES
# parameter. Old Arduino libraries place the source files at the top level.
# Later Arduino libraries put the source files under the src/ directory. Also
# support 3 levels of subdirectories. Support both C and C++ libraries files.
MODULE_EXPANSION_CPP = $(wildcard $(module)/*.cpp) \
	$(wildcard $(module)/src/*.cpp) \
	$(wildcard $(module)/src/*/*.cpp) \
	$(wildcard $(module)/src/*/*/*.cpp) \
	$(wildcard $(module)/src/*/*/*/*.cpp)
MODULE_EXPANSION_C = $(wildcard $(module)/*.c) \
	$(wildcard $(module)/src/*.c) \
	$(wildcard $(module)/src/*/*.c) \
	$(wildcard $(module)/src/*/*/*.c) \
	$(wildcard $(module)/src/*/*/*/*.c)
MODULE_SRCS_CPP += $(foreach module,$(EPOXY_MODULES),$(MODULE_EXPANSION_CPP))
MODULE_SRCS_C += $(foreach module,$(EPOXY_MODULES),$(MODULE_EXPANSION_C))
# 3) Collect the source files in the application directory, also 3 levels down.
APP_SRCS_CPP += $(wildcard *.cpp) \
	$(wildcard */*.cpp) \
	$(wildcard */*/*.cpp) \
	$(wildcard */*/*/*.cpp)
APP_SRCS_C += $(wildcard *.c) \
	$(wildcard */*.c) \
	$(wildcard */*/*.c) \
	$(wildcard */*/*/*.c)

# Generate the list of object files from the *.cpp, *.c, and *.ino files.
OBJS += $(EPOXY_SRCS:%.cpp=%.o)
OBJS += $(MODULE_SRCS_CPP:%.cpp=%.o)
OBJS += $(MODULE_SRCS_C:%.c=%.o)
OBJS += $(APP_SRCS_CPP:%.cpp=%.o)
OBJS += $(APP_SRCS_C:%.c=%.o)
OBJS +=$(APP_NAME).o

# Finally the rule to generate the *.out binary file for the application.
$(APP_NAME).out: $(OBJS)
	echo "    Linking $(compiler) $<"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# We need to add a rule to treat .ino file as just a  normal .cpp.
$(APP_NAME).o: $(APP_NAME).ino $(DEPS)
	echo "    Compiling $(compiler) $<"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -x c++ -c $<

# We don't need this rule because the implicit GNU Make rules for converting
# *.c and *.cpp into *.o files are sufficient.
#
%.o: %.cpp
	echo "    Compiling $(compiler) $<"
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# The following simple rules do not capture all header dependencies of a given
# *.cpp or *.c file. It's probably better to make each *.cpp and *.c to depend
# on all headers of a given module, and force a recompilation of all the files.
# As far as I understand, this is what the Arduino IDE does upon each compile
# iteration. But this is good enough for now, but has the disadvantage that we
# have to run `make clean` more often than necessary to reset all the
# dependencies.
%.cpp: %.h
%.c: %.h

.PHONY: all clean run $(MORE_CLEAN)

# Use 'make all' or just 'make' to compile the binary.
all: $(APP_NAME).out

# Use 'make run' to run the binary created by 'make all'. This is useful when
# the (APP_NAME).out is an AUnit unit test. You can execute ': make run' inside
# the vim editor. The error message of AUnit (as of v1.7) is compatible with
# the quickfix errorformat of vim, so vim will automatically detect assertion
# errors and jump directly to the line where the assertion failed.
run:
	echo "------------[ running ./$(APP_NAME).out $(TESTS) ]---------------"
	./$(APP_NAME).out $(TESTS); \
	echo "Return code: $$?"

debug:
	gdb --args ./$(APP_NAME).out $(TESTS)

# Use 'make clean' to remove intermediate '*.o' files, the target '*.out' file,
# and any generated files defined by $(GENERATED).
clean: $(MORE_CLEAN)
	rm -f $(OBJS) $(APP_NAME).out $(GENERATED)
