VPATH = src
objects = chipo-eighto.o chip.o media.o utils.o sys.o args.o

chipo8o: $(objects)
	cc -g3 --std=c99 -Wall -o chipo8o $(objects) -lraylib -lm

chipo-eighto.o: chipo-eighto.c 
chip.o: chip.h
media.o: media.h
utils.o: utils.h chip.h sys.h media.h
sys.o: sys.h
args.o: args.h

.PHONY: clean
clean:
	-rm $(objects)
