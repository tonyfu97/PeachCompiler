OBJECTS= ./build/compiler.o ./build/cprocess.o ./build/lexer.o ./build/lex_process.o ./build/token.o ./build/parser.o ./build/node.o ./build/expressionable.o ./build/datatype.o ./build/scope.o ./build/symresolver.o ./build/buffer.o ./build/vector.o
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

./build/parser.o: ./parser.c
	gcc -c ./parser.c -o ./build/parser.o ${INCLUDES} ${FLAGS}

./build/node.o: ./node.c
	gcc -c ./node.c -o ./build/node.o ${INCLUDES} ${FLAGS}

./build/expressionable.o: ./expressionable.c
	gcc -c ./expressionable.c -o ./build/expressionable.o ${INCLUDES} ${FLAGS}

./build/datatype.o: ./datatype.c
	gcc -c ./datatype.c -o ./build/datatype.o ${INCLUDES} ${FLAGS}

./build/scope.o: ./scope.c
	gcc -c ./scope.c -o ./build/scope.o ${INCLUDES} ${FLAGS}

./build/symresolver.o: ./symresolver.c
	gcc -c ./symresolver.c -o ./build/symresolver.o ${INCLUDES} ${FLAGS}

./build/buffer.o: ./helpers/buffer.c
	gcc -c ./helpers/buffer.c -o ./build/buffer.o ${INCLUDES} ${FLAGS}

./build/vector.o: ./helpers/vector.c
	gcc -c ./helpers/vector.c -o ./build/vector.o ${INCLUDES} ${FLAGS}

clean:
	rm ./main
	rm -rf ${OBJECTS}
