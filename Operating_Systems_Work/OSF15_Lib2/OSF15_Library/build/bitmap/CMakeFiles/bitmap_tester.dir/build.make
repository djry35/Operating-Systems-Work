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
CMAKE_SOURCE_DIR = /home/djry35/OSF15_Lib2/OSF15_Library

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/djry35/OSF15_Lib2/OSF15_Library/build

# Include any dependencies generated for this target.
include bitmap/CMakeFiles/bitmap_tester.dir/depend.make

# Include the progress variables for this target.
include bitmap/CMakeFiles/bitmap_tester.dir/progress.make

# Include the compile flags for this target's objects.
include bitmap/CMakeFiles/bitmap_tester.dir/flags.make

bitmap/CMakeFiles/bitmap_tester.dir/test/test.c.o: bitmap/CMakeFiles/bitmap_tester.dir/flags.make
bitmap/CMakeFiles/bitmap_tester.dir/test/test.c.o: ../bitmap/test/test.c
	$(CMAKE_COMMAND) -E cmake_progress_report /home/djry35/OSF15_Lib2/OSF15_Library/build/CMakeFiles $(CMAKE_PROGRESS_1)
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Building C object bitmap/CMakeFiles/bitmap_tester.dir/test/test.c.o"
	cd /home/djry35/OSF15_Lib2/OSF15_Library/build/bitmap && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -o CMakeFiles/bitmap_tester.dir/test/test.c.o   -c /home/djry35/OSF15_Lib2/OSF15_Library/bitmap/test/test.c

bitmap/CMakeFiles/bitmap_tester.dir/test/test.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/bitmap_tester.dir/test/test.c.i"
	cd /home/djry35/OSF15_Lib2/OSF15_Library/build/bitmap && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -E /home/djry35/OSF15_Lib2/OSF15_Library/bitmap/test/test.c > CMakeFiles/bitmap_tester.dir/test/test.c.i

bitmap/CMakeFiles/bitmap_tester.dir/test/test.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/bitmap_tester.dir/test/test.c.s"
	cd /home/djry35/OSF15_Lib2/OSF15_Library/build/bitmap && /usr/bin/cc  $(C_DEFINES) $(C_FLAGS) -S /home/djry35/OSF15_Lib2/OSF15_Library/bitmap/test/test.c -o CMakeFiles/bitmap_tester.dir/test/test.c.s

bitmap/CMakeFiles/bitmap_tester.dir/test/test.c.o.requires:
.PHONY : bitmap/CMakeFiles/bitmap_tester.dir/test/test.c.o.requires

bitmap/CMakeFiles/bitmap_tester.dir/test/test.c.o.provides: bitmap/CMakeFiles/bitmap_tester.dir/test/test.c.o.requires
	$(MAKE) -f bitmap/CMakeFiles/bitmap_tester.dir/build.make bitmap/CMakeFiles/bitmap_tester.dir/test/test.c.o.provides.build
.PHONY : bitmap/CMakeFiles/bitmap_tester.dir/test/test.c.o.provides

bitmap/CMakeFiles/bitmap_tester.dir/test/test.c.o.provides.build: bitmap/CMakeFiles/bitmap_tester.dir/test/test.c.o

# Object files for target bitmap_tester
bitmap_tester_OBJECTS = \
"CMakeFiles/bitmap_tester.dir/test/test.c.o"

# External object files for target bitmap_tester
bitmap_tester_EXTERNAL_OBJECTS =

bitmap/bitmap_tester: bitmap/CMakeFiles/bitmap_tester.dir/test/test.c.o
bitmap/bitmap_tester: bitmap/CMakeFiles/bitmap_tester.dir/build.make
bitmap/bitmap_tester: bitmap/CMakeFiles/bitmap_tester.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --red --bold "Linking C executable bitmap_tester"
	cd /home/djry35/OSF15_Lib2/OSF15_Library/build/bitmap && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/bitmap_tester.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
bitmap/CMakeFiles/bitmap_tester.dir/build: bitmap/bitmap_tester
.PHONY : bitmap/CMakeFiles/bitmap_tester.dir/build

bitmap/CMakeFiles/bitmap_tester.dir/requires: bitmap/CMakeFiles/bitmap_tester.dir/test/test.c.o.requires
.PHONY : bitmap/CMakeFiles/bitmap_tester.dir/requires

bitmap/CMakeFiles/bitmap_tester.dir/clean:
	cd /home/djry35/OSF15_Lib2/OSF15_Library/build/bitmap && $(CMAKE_COMMAND) -P CMakeFiles/bitmap_tester.dir/cmake_clean.cmake
.PHONY : bitmap/CMakeFiles/bitmap_tester.dir/clean

bitmap/CMakeFiles/bitmap_tester.dir/depend:
	cd /home/djry35/OSF15_Lib2/OSF15_Library/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/djry35/OSF15_Lib2/OSF15_Library /home/djry35/OSF15_Lib2/OSF15_Library/bitmap /home/djry35/OSF15_Lib2/OSF15_Library/build /home/djry35/OSF15_Lib2/OSF15_Library/build/bitmap /home/djry35/OSF15_Lib2/OSF15_Library/build/bitmap/CMakeFiles/bitmap_tester.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : bitmap/CMakeFiles/bitmap_tester.dir/depend
