CC = c++
CFLAGS = -std=c++98 -Wall -Werror -Wextra

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
EXEC = a.out

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f $(OBJS)
fclean:
	rm -f $(OBJS) $(EXEC)
re: fclean all

run: re clean
	./a.out 6667 asd 