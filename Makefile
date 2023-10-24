all: 	
	flex --outfile=scanner.c scanner.l
	bison -v --yacc --defines --output=parser.c parser.y
	gcc -g -w scanner.c parser.c symbolTable.h 

run:
	./a.out < test.txt
	@echo "";
	cat Quads.txt
	@echo "";
	@echo "Quads to  Binary"
	@echo "";
	cat BinaryFile.txt



clean:
	rm -rf *.c
	rm -rf *.output
	rm -rf *.out
	rm -rf *.h.gch
	rm -rf *.abc
	rm -rf parser.h

