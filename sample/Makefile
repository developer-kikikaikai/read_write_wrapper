TARGET=testread_write
SRC=$(shell find . -maxdepth 1 -name *.c)
CFLAG=-g -O0 
LIBS=-L ../lib/ -lfstream_wrapper
INC=-I ../include
OBJ=$(SRC:%.c=%.o)

all: $(OBJ)
	gcc $(CFLAG) $(OBJ) $(LIBS) -o $(TARGET)

%.o: %.c
	gcc $(INC) -o $@ -c $<

clean:
	rm -f $(OBJ) $(TARGET);
