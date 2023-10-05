include Makefile.files

CFLAGS=-Werror -Wall -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2 -g -O2
CXXFLAGS=$(CFLAGS)

all: complete obj simple syslog filter mutex minimal simple-disabled \
	complete-disabled

complete:
	gcc ${CFLAGS} ${HUGIN_SOURCES} main_complete.c -o hugin-$@ \
	 -DWITH_HUG_SYSLOG -DWITH_HUG_FILTER -DWITH_HUG_MUTEX
	(./hugin-$@ || (echo "Failed running $@"; false) ) 2>&1

	g++ ${CXXFLAGS} ${HUGIN_SOURCES} main_complete.c -o hugin-$@ \
	 -DWITH_HUG_SYSLOG -DWITH_HUG_FILTER -DWITH_HUG_MUTEX
	(./hugin-$@ || (echo "Failed running $@"; false) ) 2>&1

obj:
#	Compile hugin as c prog and link with c++ program (use hugin.hpp)
	g++ ${CXXFLAGS} main_simple.cc -c
	gcc ${CFLAGS} hugin.c -c
	g++ main_simple.o hugin.o -o hugin-$@
	(./hugin-$@ || (echo "Failed running $@"; false) ) 2>&1

#	Compile hugin as c++ prog and link with c++ program
	g++ ${CXXFLAGS} main_simple.c -c
	g++ ${CXXFLAGS} hugin.c -c
	g++ main_simple.o hugin.o -o hugin-$@
	(./hugin-$@ || (echo "Failed running $@"; false) ) 2>&1

simple:
	gcc ${CFLAGS} hugin.c main_simple.c -o hugin-$@
	(./hugin-$@ || (echo "Failed running $@"; false) ) 2>&1

	g++ ${CXXFLAGS} hugin.c main_simple.c -o hugin-$@
	(./hugin-$@ || (echo "Failed running $@"; false) ) 2>&1

minimal:
	gcc ${CFLAGS} hugin.c main_minimal.c -o hugin-$@
	(./hugin-$@ || (echo "Failed running $@"; false) ) 2>&1

	g++ ${CXXFLAGS} hugin.c main_minimal.c -o hugin-$@
	(./hugin-$@ || (echo "Failed running $@"; false) ) 2>&1

syslog:
	gcc ${CFLAGS} hugin.c hugin_syslog.c main_syslog.c \
	 -o hugin-$@ -DWITH_HUG_SYSLOG
	(./hugin-$@ || (echo "Failed running $@"; false) ) 2>&1

	g++ ${CXXFLAGS} hugin.c hugin_syslog.c main_syslog.c \
	 -o hugin-$@ -DWITH_HUG_SYSLOG
	(./hugin-$@ || (echo "Failed running $@"; false) ) 2>&1

filter:
	gcc ${CFLAGS} hugin.c hugin_filter.c main_filter.c -o hugin-$@ \
		-DWITH_HUG_FILTER
	(./hugin-$@ || (echo "Failed running $@"; false) ) 2>&1

	g++ ${CXXFLAGS} hugin.c hugin_filter.c main_filter.c -o hugin-$@ \
		-DWITH_HUG_FILTER
	(./hugin-$@ || (echo "Failed running $@"; false) ) 2>&1

mutex:
	gcc ${CFLAGS} hugin.c main_mutex.c -o hugin-$@ -DWITH_HUG_MUTEX
	(./hugin-$@ || (echo "Failed running $@"; false) ) 2>&1

	g++ ${CXXFLAGS} hugin.c main_mutex.c -o hugin-$@ -DWITH_HUG_MUTEX
	(./hugin-$@ || (echo "Failed running $@"; false) ) 2>&1

simple-disabled:
	gcc -DDISABLE_HUGIN ${CFLAGS} hugin.c main_simple.c -o hugin-$@
	(./hugin-$@ || (echo "Failed running $@"; false) ) 2>&1

	g++ -DDISABLE_HUGIN ${CXXFLAGS} hugin.c main_simple.c -o hugin-$@
	(./hugin-$@ || (echo "Failed running $@"; false) ) 2>&1


complete-disabled:
	gcc -DDISABLE_HUGIN ${CFLAGS} ${HUGIN_SOURCES} main_complete.c \
		-o hugin-$@ -DWITH_HUG_SYSLOG -DWITH_HUG_FILTER \
		-DWITH_HUG_MUTEX
	(./hugin-$@ || (echo "Failed running $@"; false) ) 2>&1

	g++ -DDISABLE_HUGIN ${CXXFLAGS} ${HUGIN_SOURCES} main_complete.c \
		-o hugin-$@ -DWITH_HUG_SYSLOG -DWITH_HUG_FILTER \
		-DWITH_HUG_MUTEX
	(./hugin-$@ || (echo "Failed running $@"; false) ) 2>&1

clean:
	rm -f hugin-{complete,complete-cpp,simple,syslog,filter,mutex}
	rm -f hugin-{minimal,simple-disabled,complete-disabled,obj}
