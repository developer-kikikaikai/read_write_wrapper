TARGET=libfstream_wrapper
RMTARGET=$(TARGET).*
SRC=$(shell find . -maxdepth 1 -name "*.c")
CFLAG=-O2 -shared -fPIC

#if I want to check memory leak test, change folliwing option and use valgring
#CFLAG=-O0 -g -shared -fPIC
#CFLAG+=-DDBGFLAG
INCLUDE=-I ../include
all:
	gcc $(CFLAG) $(INCLUDE) $(OBJ) $(LIBS) -o $(TARGET).a $(SRC)
	gcc $(CFLAG) $(INCLUDE) $(OBJ) $(LIBS) -o $(TARGET).so $(SRC)

clean:
	rm -f $(OBJ) $(RMTARGET);
