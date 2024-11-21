CC=g++
CFLAGS=-DDEBUG
OBJS=main.o
TARGET=main
RM=del

all: $(TARGET)

clean:
	$(RM) $(OBJS) $(TARGET).exe

.PHONY : clean

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

main.o: tree.h main.cpp
	$(CC) $(CFLAGS) -c main.cpp -o main.o
