OBJECTS= ./build/compiler.o ./build/cprocess.o ./build/buffer.o ./build/vector.o
INCLUDES= -I./

all: ${OBJECTS}
	gcc main.c -o ./main ${INCLUDES} ${OBJECTS}

./build/compiler.o: ./compiler.c
	gcc -c ./compiler.c -o ./build/compiler.o ${INCLUDES}

./build/cprocess.o: ./cprocess.c
	gcc -c ./cprocess.c -o ./build/cprocess.o ${INCLUDES}

./build/buffer.o: ./helpers/buffer.c
	gcc -c ./helpers/buffer.c -o ./build/buffer.o ${INCLUDES}

./build/vector.o: ./helpers/vector.c
	gcc -c ./helpers/vector.c -o ./build/vector.o ${INCLUDES}

clean:
	rm ./main
	rm -rf ${OBJECTS}
