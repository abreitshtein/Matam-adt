CC=gcc
OBJS=chessSystem.o games.o players.o tournament.o tests/chessSystemTestsExample.o
EXEC=chess
OBJ=chess.o
CFLAGS=-std=c99 -Wall -pedantic-errors -Werror $(DEBUG)

$(EXEC) : $(OBJ)
	$(CC) $(OBJ) -o $@ -L. -lmap

$(OBJ): $(OBJS)
	ld -r -o $(OBJ) $(OBJS)
chessSystem.o: chessSystem.c games.h players.h map.h chessSystem.h \
 tournament.h
games.o: games.c games.h players.h map.h chessSystem.h
players.o: players.c players.h map.h chessSystem.h
tournament.o: tournament.c tournament.h games.h players.h map.h \
 chessSystem.h
tests/chessSystemTestsExample.o: tests/chessSystemTestsExample.c
clean:
	rm -f $(OBJS) $(OBJ) $(EXEC)






