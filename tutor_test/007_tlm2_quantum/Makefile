# SYSTEMC_HOME    = /usr/local/lib/systemc-2.2.0
SYSTEMC_HOME    = /usr/local/lib/systemc-2.3.0
TARGET_ARCH     = linux

SYSTEMC_INC_DIR = $(SYSTEMC_HOME)/include
SYSTEMC_LIB_DIR = $(SYSTEMC_HOME)/lib-$(TARGET_ARCH)

# LIBTOOL Setting
SHELL           = /bin/bash
LIBTOOL_DIR     = /home/kocha/library/systemc/install/systemc-2.3.0_pub_rev_20111121/objdir
LIBTOOL         = $(SHELL) $(LIBTOOL_DIR)/libtool

GXX             = g++
GCC             = gcc
LD              = g++

FLAGS           = -g -Wall -pedantic -Wno-long-long \
                 -DSC_INCLUDE_DYNAMIC_PROCESSES -fpermissive \
                 -I$(SYSTEMC_INC_DIR) 
# USEROPT         = -DDEBUG -DSIMPLE_MSG -DQUANTUM
USEROPT         = -DQUANTUM
LDFLAGS         = -L$(SYSTEMC_LIB_DIR) -lsystemc -lm

SRCS = sc_main.cpp
OBJS=$(SRCS:.cpp=.o)

# main
main: $(OBJS)
	$(LIBTOOL) --mode=link --tag=CXX g++ -o $@ $(OBJS) $(LDFLAGS)

.cpp.o:
	$(LIBTOOL) --mode=compile g++ -c $(FLAGS) $(USEROPT) $<

	# g++ -c $(FLAGS) $<

%.o: %.c
	$(LIBTOOL) --mode=compile gcc -Wall -c $<

clean:
	rm -f main *.o *.lo
	rm -rf .libs

