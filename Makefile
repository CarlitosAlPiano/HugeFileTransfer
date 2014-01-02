C++ = g++

ifndef os
   os = OSX
endif

ifndef arch
   arch = POWERPC
endif

CCFLAGS = -Wall -D$(os) -Iudt4/src -finline-functions -O3

ifeq ($(arch), IA32)
   CCFLAGS += -DIA32 #-mcpu=pentiumpro -march=pentiumpro -mmmx -msse
endif

ifeq ($(arch), POWERPC)
   CCFLAGS += -mcpu=powerpc
endif

ifeq ($(arch), IA64)
   CCFLAGS += -DIA64
endif

ifeq ($(arch), SPARC)
   CCFLAGS += -DSPARC
endif

LDFLAGS = -Ludt4/src -ludt -lstdc++ -lpthread -lm

ifeq ($(os), UNIX)
   LDFLAGS += -lsocket
endif

ifeq ($(os), SUNOS)
   LDFLAGS += -lrt -lsocket
endif

DIR = $(shell pwd)

APP = server client

all: $(APP) install

%.o: %.cpp
	$(C++) $(CCFLAGS) $< -c

server: server.o common.o
	$(C++) $^ -o $@ $(LDFLAGS)
client: client.o common.o
	$(C++) $^ -o $@ $(LDFLAGS)

clean:
	rm -f *.o $(APP)

install:
	export PATH=$(DIR):$$PATH
	export DYLD_LIBRARY_PATH=$(DIR)/udt4/src:$$DYLD_LIBRARY_PATH
	rm -f *.o