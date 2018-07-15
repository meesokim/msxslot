OBJS = msxbus.o msx.o
TARGET = msx
LDFLAGS = -lbcm2835

.SUFFIXES: .c .o

all: $(TARGET)

msxbus.o: msxbus.c 
	$(CC) -c -g -o msxbus.o msxbus.c
	
msx.o: msx.c
	$(CC) -c -g -o msx.o msx.c

$(TARGET): $(OBJS)
	$(CC) -o $@ -g $(OBJS) $(LDFLAGS)

m: 
	$(CC) -o msx -D_MAIN -g msxbus.c $(LDFLAGS) 


clean:
	rm -f $(OBJS) $(TARGET) m
