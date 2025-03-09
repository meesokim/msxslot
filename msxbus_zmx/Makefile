TARGET=zmxbus

CC?=g++
ifeq ($(TARGET_OS),windows)
	CC=x86_64-w64-mingw32-gcc
	CXX=x86_64-w64-mingw32-g++
	ZEMMIX_EXT=zxw
	LIBINFO=
	FLAGS = 
	LDFLAGS=-static-libgcc -static
else
	CC=g++
	ZEMMIX_EXT=zxl
	LIBINFO=libinfo
	FLAGS = -fpermissive
	LDFLAGS=
endif
FLAGS += -fPIC -Iinclude -I.
CXXFLAGS = -fpermissive
SOURCES = $(shell find -maxdepth 1 -name "*.c")
COBJS = $(shell echo $(SOURCES) | sed -r 's/\.s|\.c|\.cpp|.cc/\.o/g') 
DEBUGFLAGS = -O0 -D _DEBUG -shared
RELEASEFLAGS = -O2 -D DEBUG -combine -fwhole-program -shared
OBJS = $(TARGET).o $(COBJS)

all: $(TARGET).$(ZEMMIX_EXT) example $(LIBINFO)

example: example.o zmx.o 
	$(CXX) $(FLAGS) $(LDFLAGS) -o $@ $^ 

libinfo: libinfo.o
	$(CC) $(FLAGS) -o $@ $^  -g

%.o: %.cc 
	$(CXX) $(FLAGS) $(CXXFLAGS) -c -o $@ $^

%.o: %.c 
	$(CC) $(FLAGS) -c -o $@ $<


$(TARGET).$(ZEMMIX_EXT): $(OBJS)
	# (CXX)
	$(CXX) $(FLAGS) $(DEBUGFLAGS) -o $(TARGET).$(ZEMMIX_EXT) $(OBJS) $(LDFLAGS)

clean:
	$(RM) $(OBJS) $(TARGET).$(ZEMMIX_EXT) libinfo example *.o
