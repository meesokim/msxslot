TARGET=zmxbus

CC=g++
ifneq (,$(findstring mingw, $(CC)))
	LDFLAGS = -static -static-libgcc -static-libstdc++
	EXT = zxw
else
	LDFLAGS = -ldl
	EXT = zxl
endif
FLAGS = -fPIC -I.
CFLAGS = 
DEBUGFLAGS = -O0 -D _DEBUG -shared
RELEASEFLAGS = -O2 -fwhole-program -shared
OBJS = $(TARGET).o 

all: $(TARGET).$(EXT) zmxdump libinfo zmxclk example

zmxdump: zmxdump.o zmx.o
	$(CC) $(FLAGS) -o $@ $^ -ldl 

libinfo: libinfo.o
	$(CC) $(FLAGS) -o $@ $^ -ldl -g

zmxclk: zmxclk.o
	$(CC) $(FLAGS) -o $@ $^ -lbcm2835

example: example.o zmx.o
	$(CC) $(FLAGS) -o $@ $^ -ldl

%.o: %.cc 
	$(CC) $(FLAGS) -c -o $@ $^

%.o: %.c 
	$(CC) $(FLAGS) -c -o $@ $^


$(TARGET).$(EXT): $(OBJS)
	$(CC) $(FLAGS) $(CFLAGS) $(RELEASEFLAGS) -o $(TARGET).$(EXT) $(OBJS) -lbcm2835

clean:
	$(RM) $(OBJS) $(TARGET).$(EXT) libinfo zmxdump *.o
