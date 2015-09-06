APP=regurgitator
CC=gcc
OBJS=main.o
CFLAGS=-g3 -O0 -Wall -ldl -Wno-unused-value

all: $(APP)

$(APP): $(OBJS)
	$(CC) $^ -o $@ $(CFLAGS)

test: $(APP)
	./$(APP)

clean:
	$(RM) $(APP) $(OBJS)
