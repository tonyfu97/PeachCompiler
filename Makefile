OBJECTS= ./build/compiler.o ./build/cprocess.o ./build/lexer.o ./build/lex_process.o ./build/token.o ./build/buffer.o ./build/vector.o
INCLUDES= -I./
FLAGS= -g #-Wall -Werror -std=c11

all: ${OBJECTS}
	gcc main.c -o ./main ${INCLUDES} ${OBJECTS} ${FLAGS}

./build/compiler.o: ./compiler.c
	gcc -c ./compiler.c -o ./build/compiler.o ${INCLUDES} ${FLAGS}

./build/cprocess.o: ./cprocess.c
	gcc -c ./cprocess.c -o ./build/cprocess.o ${INCLUDES} ${FLAGS}

./build/lexer.o: ./lexer.c
	gcc -c ./lexer.c -o ./build/lexer.o ${INCLUDES} ${FLAGS}

./build/lex_process.o: ./lex_process.c
	gcc -c ./lex_process.c -o ./build/lex_process.o ${INCLUDES} ${FLAGS}

./build/token.o : ./token.c
	gcc -c ./token.c -o ./build/token.o ${INCLUDES} ${FLAGS}

./build/buffer.o: ./helpers/buffer.c
	gcc -c ./helpers/buffer.c -o ./build/buffer.o ${INCLUDES} ${FLAGS}

./build/vector.o: ./helpers/vector.c
	gcc -c ./helpers/vector.c -o ./build/vector.o ${INCLUDES} ${FLAGS}

clean:
	rm ./main
	rm -rf ${OBJECTS}
