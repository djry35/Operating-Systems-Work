# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 2.8

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list

# Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/djry35/OSF15_Exercise2

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/djry35/OSF15_Exercise2/build

# Include any dependencies generated for this target.
include bs/CMakeFiles/block_store.dir/depend.make

# Include the progress variables for this target.
include bs/CMakeFiles/block_store.dir/progress.make

# Include the compile flags for this target's objects.
include bs/CMakeFiles/block_store.dir/flags.make

bs/CMakeFiles/block_store.dir/src/block_store.c.o: bs/CMakeFiles/block_store.dir/flags.make
bs/CMakeFiles/block_store.dir/src/block_store.c.o: ../bs/src/block_store.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/djry35/OSF15_Exercise2/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object bs/CMakeFiles/block_store.dir/src/block_store.c.o"
	cd /home/djry35/OSF15_Exercise2/build/bs && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/block_store.dir/src/block_store.c.o   -c /home/djry35/OSF15_Exercise2/bs/src/block_store.c

bs/CMakeFiles/block_store.dir/src/block_store.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/block_store.dir/src/block_store.c.i"
	cd /home/djry35/OSF15_Exercise2/build/bs && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/djry35/OSF15_Exercise2/bs/src/block_store.c > CMakeFiles/block_store.dir/src/block_store.c.i

bs/CMakeFiles/block_store.dir/src/block_store.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/block_store.dir/src/block_store.c.s"
	cd /home/djry35/OSF15_Exercise2/build/bs && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/djry35/OSF15_Exercise2/bs/src/block_store.c -o CMakeFiles/block_store.dir/src/block_store.c.s

bs/CMakeFiles/block_store.dir/src/block_store.c.o.requires:
.PHONY : bs/CMakeFiles/block_store.dir/src/block_store.c.o.requires

bs/CMakeFiles/block_store.dir/src/block_store.c.o.provides: bs/CMakeFiles/block_store.dir/src/block_store.c.o.requires
	$(MAKE) -f bs/CMakeFiles/block_store.dir/build.make bs/CMakeFiles/block_store.dir/src/block_store.c.o.provides.build
.PHONY : bs/CMakeFiles/block_store.dir/src/block_store.c.o.provides

bs/CMakeFiles/block_store.dir/src/block_store.c.o.provides.build: bs/CMakeFiles/block_store.dir/src/block_store.c.o

# Object files for target block_store
block_store_OBJECTS = \
"CMakeFiles/block_store.dir/src/block_store.c.o"

# External object files for target block_store
block_store_EXTERNAL_OBJECTS =

bs/libblock_store.so: bs/CMakeFiles/block_store.dir/src/block_store.c.o
bs/libblock_store.so: bs/CMakeFiles/block_store.dir/build.make
bs/libblock_store.so: /usr/local/lib/libbitmap.so
bs/libblock_store.so: bs/CMakeFiles/block_store.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C shared library libblock_store.so"
	cd /home/djry35/OSF15_Exercise2/build/bs && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/block_store.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
bs/CMakeFiles/block_store.dir/build: bs/libblock_store.so
.PHONY : bs/CMakeFiles/block_store.dir/build

bs/CMakeFiles/block_store.dir/requires: bs/CMakeFiles/block_store.dir/src/block_store.c.o.requires
.PHONY : bs/CMakeFiles/block_store.dir/requires

bs/CMakeFiles/block_store.dir/clean:
	cd /home/djry35/OSF15_Exercise2/build/bs && $(CMAKE_COMMAND) -P CMakeFiles/block_store.dir/cmake_clean.cmake
.PHONY : bs/CMakeFiles/block_store.dir/clean

bs/CMakeFiles/block_store.dir/depend:
	cd /home/djry35/OSF15_Exercise2/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/djry35/OSF15_Exercise2 /home/djry35/OSF15_Exercise2/bs /home/djry35/OSF15_Exercise2/build /home/djry35/OSF15_Exercise2/build/bs /home/djry35/OSF15_Exercise2/build/bs/CMakeFiles/block_store.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : bs/CMakeFiles/block_store.dir/depend

