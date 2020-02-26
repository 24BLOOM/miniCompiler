SOURCEFILE = lex.yy.c minic.tab.c ast.cpp analysis.cpp
HEADFILE = minic.tab.h def.h
FLAGS = -w 
all: $(SOURCEFILE) $(HEADFILE) 
	g++ -o complier $(SOURCEFILE) $(FLAGS)  

.PHONY: all clean

lex.yy.c: minic.l
	flex minic.l

minic.tab.h: minic.y
	bison -d -v minic.y

minic.tab.c: minic.y
	bison -d -v minic.y

FILE = complier_ lex.yy.c minic.tab.h minic.tab.c minic.output

clean:
	rm -rf $(FILE)

