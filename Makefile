all: $(OBJ)
	make -C lib
	make -C sample
clean:
	make -C lib clean
	make -C sample clean
