# See ~/gitsrc/googletest/googletest/make/Makefile
# Points to the root of Google Test, relative to where this file is.
# Remember to tweak this if you move this file.
GTEST_DIR = $(HOME)/gitsrc/googletest
# Wrong: do not include $(GTEST_DIR)/include/gtest/internal/*.h
GTEST_HEADERS = $(GTEST_DIR)/googletest/include
GTESTLIBPATH=$(GTEST_DIR)/build/lib
# See ~/gitsrc/googletest/googletest/README.md.
# export GTEST_DIR=/home/alison/gitsrc/googletest/
# g++ -std=c++11 -isystem ${GTEST_DIR}/include -I${GTEST_DIR} -pthread -c ${GTEST_DIR}/src/gtest-all.cc
# cd make; make all
# 'make' in the README.md above doesn't create libgtest_main.a.  'make all' does.
GMOCK_HEADERS = $(GTEST_DIR)/googlemock/include
# Reordering the list below will cause a linker failure.  libgmock_main.a must apparently appear before libgmock.a.
GMOCKLIBS=$(GTESTLIBPATH)/libgmock_main.a $(GTESTLIBPATH)/libgmock.a  $(GTESTLIBPATH)/libgtest.a

# https://github.com/linux-rt/librtpi.git
LIBRTPI_PATH=$(HOME)/gitsrc/librtpi
LIBRTPI_HEADERS=$(LIBRTPI_PATH)/src
LIBRTPI_LIBS=$(LIBRTPI_PATH)/src/.libs/librtpi.a

# Note -pthread, not -lpthread.   Without this option, Googletest does not compile.
# How to allow users to add -DDEBUG (for example) to the cmdline:
# make CXXFLAGS:='-DDEBUG' as described in
#      https://www.gnu.org/software/make/manual/make.html#Appending
# "suppose you always want the ‘-g’ switch when you run the C compiler, but you
# would like to allow the user to specify the other switches with a command
# argument just as usual. You could use this override directive:
#     override CFLAGS += -g
#"
override CXXFLAGS+= -std=c++17 -ggdb -Wall -Wextra -Werror -g -O0 -fno-inline -fsanitize=address,undefined -I$(GTEST_HEADERS)
CXXFLAGS-NOTEST= -std=c++17 -ggdb -Wall -Wextra -Werror -g -O0 -fno-inline -fsanitize=address,undefined
CXXFLAGS-NOSANITIZE= -std=c++17 -ggdb -Wall -Wextra -Werror -g -O0 -fno-inline -I$(GTEST_HEADERS)
CXXFLAGS-TSAN= -std=c++17 -ggdb -Wall -Wextra -Werror -g -O0 -fno-inline -fsanitize=thread -I$(GTEST_HEADERS)

LDFLAGS= -ggdb -g -fsanitize=address -L$(GTESTLIBPATH)
LDFLAGS-NOSANITIZE= -ggdb -g -L$(GTESTLIBPATH)
LDFLAGS-NOTEST= -ggdb -g -fsanitize=address
# gcc and clang won't automatically link .cc files against the standard library.
CXX=/usr/bin/g++
#CXX=/usr/bin/clang++

atomic-stack_lib_test-gcc-nosanitize: atomic-stack.impl.hh atomic-stack_lib_test.cc
	g++ -std=c++20 -ggdb -Wall -Wextra -Werror -g -O0 -fno-inline -I$(GTEST_HEADERS) -I$(GMOCK_HEADERS) atomic-stack_lib_test.cc $(GMOCKLIBS) -o $@

atomic-stack_lib_test-tsan: atomic-stack.impl.hh atomic-stack_lib_test.cc
	g++ -std=c++20 -ggdb -Wall -Wextra -Werror -g -O0 -fno-inline -fsanitize=thread,undefined -I$(GTEST_HEADERS) -I$(GMOCK_HEADERS) atomic-stack_lib_test.cc $(GMOCKLIBS) -o $@

# -lc++ specifies the LLVM libc
atomic-stack_lib_test-clang-nosanitize: atomic-stack.impl.hh atomic-stack_lib_test.cc
	clang++ -lc++ -std=c++20 -ggdb -Wall -Wextra -Werror -g -O0 -fno-inline -I$(GTEST_HEADERS) -I$(GMOCK_HEADERS) atomic-stack_lib_test.cc $(GMOCKLIBS) -o $@

atomic-stack_lib_test-clangtidy: atomic-stack.impl.hh atomic-stack_lib_test.cc
	$(CLANG_TIDY_BINARY) $(CLANG_TIDY_OPTIONS) -checks=$(CLANG_TIDY_CHECKS) $^ -- -std=c++20 -ggdb -Wall -Wextra -Werror -g -O0 -fno-inline -I$(GTEST_HEADERS) -I$(GMOCK_HEADERS)  $(GMOCKLIBS)  -o $@

atomic-mutex_lib_test-gcc: atomic-mutex.hh atomic-mutex_lib_test.cc
	g++ -std=c++20 -ggdb -Wall -Wextra -Werror -g -O0 -fno-inline -fsanitize=address,undefined -I$(GTEST_HEADERS) -I$(GMOCK_HEADERS) atomic-mutex_lib_test.cc $(GMOCKLIBS) -o $@

atomic-mutex_lib_test-tsan: atomic-mutex.hh atomic-mutex_lib_test.cc
	g++ -std=c++20 -ggdb -Wall -Wextra -Werror -g -O0 -fno-inline -fsanitize=thread,undefined -I$(GTEST_HEADERS) -I$(GMOCK_HEADERS) atomic-mutex_lib_test.cc $(GMOCKLIBS) -o $@

atomic-mutex_lib_test-gcc-nosanitize: atomic-mutex.hh atomic-mutex_lib_test.cc
	g++ -std=c++20 -ggdb -Wall -Wextra -Werror -g -O0 -fno-inline  -I$(GTEST_HEADERS) -I$(GMOCK_HEADERS) atomic-mutex_lib_test.cc $(GMOCKLIBS) -o $@

# -lc++ specifies the LLVM libc
atomic-mutex_lib_test-clang-nosanitize: atomic-mutex.hh atomic-mutex_lib_test.cc
	clang++ -lc++ -std=c++20 -ggdb -Wall -Wextra -Werror -g -O0 -fno-inline -I$(GTEST_HEADERS) -I$(GMOCK_HEADERS) atomic-mutex_lib_test.cc $(GMOCKLIBS) -o $@

# Sanitizers produce a lot of uninteresting strace output.
# https://stackoverflow.com/questions/20673370/why-do-we-write-d-reentrant-while-compiling-c-code-using-threads
# "the recommended way to compile with threads in GCC is using the -pthread option. It is equivalent to -lpthread -D_REENTRANT"
# -D_REENTRANT tells the compiler to use the declarations (functions, types, ...) necessary for thread usage.
condvar-pthread_lib_test-nosanitize: condvar-pthread.hh condvar-pthread_lib.cc condvar-pthread_lib_test.cc
	$(CXX)  $(CXXFLAGS-NOSANITIZE) $(LDFLAGS-NOSANITIZE) -I$(GTEST_HEADERS) -I$(GMOCK_HEADERS) condvar-pthread_lib.cc condvar-pthread_lib_test.cc $(GMOCKLIBS)  -pthread -o $@

# ASAN and UBSAN, but not TSAN
condvar-pthread_lib_test: condvar-pthread.hh condvar-pthread_lib.cc condvar-pthread_lib_test.cc
	$(CXX)  $(CXXFLAGS) $(LDFLAGS) -I$(GTEST_HEADERS) -I$(GMOCK_HEADERS) condvar-pthread_lib.cc condvar-pthread_lib_test.cc $(GMOCKLIBS)  -pthread -o $@

condvar-pthread_lib_test-tsan: condvar-pthread.hh condvar-pthread_lib.cc condvar-pthread_lib_test.cc
	$(CXX)  $(CXXFLAGS-TSAN) $(LDFLAGS-NOSANITIZE) -I$(GTEST_HEADERS) -I$(GMOCK_HEADERS) condvar-pthread_lib.cc condvar-pthread_lib_test.cc $(GMOCKLIBS)  -pthread -o $@

condvar-stl_lib_test: condvar-stl.hh condvar-stl_lib.cc condvar-stl_lib_test.cc
	$(CXX)  $(CXXFLAGS) $(LDFLAGS) -I$(GTEST_HEADERS) -I$(GMOCK_HEADERS) condvar-stl_lib.cc condvar-stl_lib_test.cc $(GMOCKLIBS) -pthread -o $@

condvar-stl_lib_test-nosanitize: condvar-stl.hh condvar-stl_lib.cc condvar-stl_lib_test.cc
	$(CXX)  $(CXXFLAGS-NOSANITIZE) $(LDFLAGS-NOSANITIZE) -I$(GTEST_HEADERS) -I$(GMOCK_HEADERS) condvar-stl_lib.cc condvar-stl_lib_test.cc $(GMOCKLIBS) -pthread -o $@

condvar-stl_lib_test-tsan: condvar-stl.hh condvar-stl_lib.cc condvar-stl_lib_test.cc
	$(CXX)  $(CXXFLAGS-TSAN) $(LDFLAGS-NOSANITIZE) -I$(GTEST_HEADERS) -I$(GMOCK_HEADERS) condvar-stl_lib.cc condvar-stl_lib_test.cc $(GMOCKLIBS) -pthread -o $@

atomic-flag_lib_test: atomic-flag.hh atomic-flag_lib.cc atomic-flag_lib_test.cc
	g++ -std=c++20 -ggdb -Wall -Wextra -Werror -g -O0 -fno-inline -fsanitize=address,undefined -I$(GTEST_HEADERS) -I$(GMOCK_HEADERS) atomic-flag_lib.cc atomic-flag_lib_test.cc $(GMOCKLIBS) -o $@

atomic-flag_lib_test-tsan: atomic-flag.hh atomic-flag_lib.cc atomic-flag_lib_test.cc
	g++ -std=c++20 -ggdb -Wall -Wextra -Werror -g -O0 -fno-inline -fsanitize=thread,undefined -I$(GTEST_HEADERS) -I$(GMOCK_HEADERS) atomic-flag_lib.cc atomic-flag_lib_test.cc $(GMOCKLIBS) -o $@

atomic-flag_lib_test-nosanitize: atomic-flag.hh atomic-flag_lib.cc atomic-flag_lib_test.cc
	g++ -std=c++20 -ggdb -Wall -Wextra -Werror -g -O0 -fno-inline -I$(GTEST_HEADERS) -I$(GMOCK_HEADERS) atomic-flag_lib.cc atomic-flag_lib_test.cc $(GMOCKLIBS) -o $@

condvar-rtpi_lib_test: condvar-rtpi.hh condvar-rtpi_lib.cc condvar-rtpi_lib_test.cc
	$(CXX)  $(CXXFLAGS) $(LDFLAGS) -I$(GTEST_HEADERS) -I$(GMOCK_HEADERS) -I$(LIBRTPI_HEADERS) condvar-rtpi_lib.cc condvar-rtpi_lib_test.cc $(GMOCKLIBS) $(LIBRTPI_LIBS) -o $@

condvar-rtpi_lib_test-nosanitize: condvar-rtpi.hh condvar-rtpi_lib.cc condvar-rtpi_lib_test.cc
	$(CXX)  $(CXXFLAGS-NOSANITIZE) $(LDFLAGS-NOSANITIZE) -I$(GTEST_HEADERS) -I$(GMOCK_HEADERS) -I$(LIBRTPI_HEADERS) condvar-rtpi_lib.cc condvar-rtpi_lib_test.cc $(GMOCKLIBS) $(LIBRTPI_LIBS)  -o $@

condvar-rtpi_lib_test-tsan: condvar-rtpi.hh condvar-rtpi_lib.cc condvar-rtpi_lib_test.cc
	$(CXX)  $(CXXFLAGS-TSAN) $(LDFLAGS-NOSANITIZE) -I$(GTEST_HEADERS) -I$(GMOCK_HEADERS) -I$(LIBRTPI_HEADERS) condvar-rtpi_lib.cc condvar-rtpi_lib_test.cc $(GMOCKLIBS) $(LIBRTPI_LIBS) -o $@

BINARY_LIST = atomic-stack_lib_test-clang-nosanitize atomic-stack_lib_test-gcc-nosanitize atomic-stack_lib_test-tsan atomic-mutex_lib_test-gcc atomic-mutex_lib_test-tsan atomic-mutex_lib_test-gcc-nosanitize atomic-mutex_lib_test-clang-nosanitize condvar-pthread_lib_test-nosanitize condvar-pthread_lib_test-tsan condvar-stl_lib_test condvar-stl_lib_test-nosanitize condvar-stl_lib_test-tsan atomic-flag_lib_test atomic-flag_lib_test-nosanitize atomic-flag_lib_test-tsan condvar-rtpi_lib_test condvar-rtpi_lib_test-nosanitize condvar-rtpi_lib_test-tsan

clean:
	rm -rf *.o *~ $(BINARY_LIST) *_test *-coverage *-valgrind *.gcda *.gcov *.gcno *.info *_output *css *html a.out *rej *orig

all:
	make clean
	make $(BINARY_LIST)

.SILENT: *.o
