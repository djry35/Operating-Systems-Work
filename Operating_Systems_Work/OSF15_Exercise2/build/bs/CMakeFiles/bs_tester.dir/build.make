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
include bs/CMakeFiles/bs_tester.dir/depend.make

# Include the progress variables for this target.
include bs/CMakeFiles/bs_tester.dir/progress.make

# Include the compile flags for this target's objects.
include bs/CMakeFiles/bs_tester.dir/flags.make

bs/CMakeFiles/bs_tester.dir/test/test.c.o: bs/CMakeFiles/bs_tester.dir/flags.make
bs/CMakeFiles/bs_tester.dir/test/test.c.o: ../bs/test/test.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/djry35/OSF15_Exercise2/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object bs/CMakeFiles/bs_tester.dir/test/test.c.o"
	cd /home/djry35/OSF15_Exercise2/build/bs && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/bs_tester.dir/test/test.c.o   -c /home/djry35/OSF15_Exercise2/bs/test/test.c

bs/CMakeFiles/bs_tester.dir/test/test.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/bs_tester.dir/test/test.c.i"
	cd /home/djry35/OSF15_Exercise2/build/bs && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/djry35/OSF15_Exercise2/bs/test/test.c > CMakeFiles/bs_tester.dir/test/test.c.i

bs/CMakeFiles/bs_tester.dir/test/test.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/bs_tester.dir/test/test.c.s"
	cd /home/djry35/OSF15_Exercise2/build/bs && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/djry35/OSF15_Exercise2/bs/test/test.c -o CMakeFiles/bs_tester.dir/test/test.c.s

bs/CMakeFiles/bs_tester.dir/test/test.c.o.requires:
.PHONY : bs/CMakeFiles/bs_tester.dir/test/test.c.o.requires

bs/CMakeFiles/bs_tester.dir/test/test.c.o.provides: bs/CMakeFiles/bs_tester.dir/test/test.c.o.requires
	$(MAKE) -f bs/CMakeFiles/bs_tester.dir/build.make bs/CMakeFiles/bs_tester.dir/test/test.c.o.provides.build
.PHONY : bs/CMakeFiles/bs_tester.dir/test/test.c.o.provides

bs/CMakeFiles/bs_tester.dir/test/test.c.o.provides.build: bs/CMakeFiles/bs_tester.dir/test/test.c.o

# Object files for target bs_tester
bs_tester_OBJECTS = \
"CMakeFiles/bs_tester.dir/test/test.c.o"

# External object files for target bs_tester
bs_tester_EXTERNAL_OBJECTS =

bs/bs_tester: bs/CMakeFiles/bs_tester.dir/test/test.c.o
bs/bs_tester: bs/CMakeFiles/bs_tester.dir/build.make
bs/bs_tester: /usr/local/lib/libbitmap.so
bs/bs_tester: bs/CMakeFiles/bs_tester.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable bs_tester"
	cd /home/djry35/OSF15_Exercise2/build/bs && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/bs_tester.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
bs/CMakeFiles/bs_tester.dir/build: bs/bs_tester
.PHONY : bs/CMakeFiles/bs_tester.dir/build

bs/CMakeFiles/bs_tester.dir/requires: bs/CMakeFiles/bs_tester.dir/test/test.c.o.requires
.PHONY : bs/CMakeFiles/bs_tester.dir/requires

bs/CMakeFiles/bs_tester.dir/clean:
	cd /home/djry35/OSF15_Exercise2/build/bs && $(CMAKE_COMMAND) -P CMakeFiles/bs_tester.dir/cmake_clean.cmake
.PHONY : bs/CMakeFiles/bs_tester.dir/clean

bs/CMakeFiles/bs_tester.dir/depend:
	cd /home/djry35/OSF15_Exercise2/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/djry35/OSF15_Exercise2 /home/djry35/OSF15_Exercise2/bs /home/djry35/OSF15_Exercise2/build /home/djry35/OSF15_Exercise2/build/bs /home/djry35/OSF15_Exercise2/build/bs/CMakeFiles/bs_tester.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : bs/CMakeFiles/bs_tester.dir/depend

