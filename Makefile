VPATH = src
objects = chipo-eighto.o chip.o media.o utils.o sys.o

chipo8o: $(objects)
	cc -g3 --std=c99 -Wall -o chipo8o $(objects) -lraylib

chipo-eighto.o: chipo-eighto.c 
chip.o: chip.h
media.o: media.h sys.h
utils.o: utils.h
sys.o: sys.h

.PHONY: clean
clean:
	-rm $(objects)
