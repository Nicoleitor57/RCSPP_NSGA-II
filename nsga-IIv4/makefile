# Compilador y flags
CC = gcc
CFLAGS = -Wall -O0 -g -mconsole
LDFLAGS = -lm

# Archivos fuente y objeto
SRC = nsga-II.c GraphReader.c model.c greedy.c allocate.c rand.c initialize.c \
 		decode.c eval.c rank.c dominance.c list.c crowddist.c sort.c report.c \
 		tourselect.c mutation.c crossover.c merge.c fillnds.c\

OBJ = $(SRC:.c=.o)

# Ejecutable
EXEC = nsga-II

# Regla principal
all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

# Limpieza de archivos intermedios
clean:
	rm -f *.o $(EXEC)

