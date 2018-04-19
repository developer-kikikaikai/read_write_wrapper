TARGET=libfstream_wrapper.so
SRC=$(shell find . -maxdepth 1 -name "*.c")
#CFLAG=-O2 -shared -fPIC
CFLAG=-O0 -g -shared -fPIC
INCLUDE=-I ../include
all:
	gcc $(CFLAG) $(INCLUDE) $(OBJ) $(LIBS) -o $(TARGET) $(SRC)

clean:
	rm -f $(OBJ) $(TARGET);
