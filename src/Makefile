TARGET=demoBin

SRC=timebase_demo.c

CC=gcc

CFLAGS=-I../include

LDFLAGS=-L../lib/

LDFLAGS+=\
		-lavutil  \
		-lavformat \
		-lavcodec \

all:$(TARGET)

$(TARGET):$(SRC)
		$(CC) $(SRC) $(CFLAGS) $(LDFLAGS) -o $(TARGET)

clean:
		rm $(TARGET) -rf

