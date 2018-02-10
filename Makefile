.phony all:
all: diskInfo diskList diskGet diskPut

diskInfo: diskinfo.c tools.o
	gcc diskinfo.c tools.o -g -std=c99 -o diskinfo

diskList: disklist.c tools.o
	gcc disklist.c tools.o -g -std=c99 -o disklist

diskGet: diskget.c tools.o
	gcc diskget.c tools.o -g -std=c99 -o diskget

diskPut: diskput.c tools.o
	gcc diskput.c tools.o -g -std=c99 -o diskput

tools.o: tools.c tools.h
	gcc -g -std=c99 -c tools.c tools.h


.PHONY clean:
clean:
	-rm -rf *.o *.exe
