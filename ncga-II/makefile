CC = g++
CFLAGS = -std=c++17 -Wall -Wextra
EXEC = ncga
SRCS = ncga.cpp GraphReader.cpp Model.cpp Greedy.cpp
OBJS = $(SRCS:.cpp=.o)
INSTANCE ?= instancias/100n.txt

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $(EXEC) $(OBJS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC)

run: $(EXEC)
	./$(EXEC) $(INSTANCE)

.PHONY: all clean run