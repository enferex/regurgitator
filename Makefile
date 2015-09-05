APP=regurgitator
CC=gcc
OBJS=main.o
CFLAGS=-g3 -O0 -Wall -ldl

all: $(APP)

$(APP): $(OBJS)
	$(CC) $^ -o $@ $(CFLAGS)

clean:
	$(RM) $(APP) $(OBJS)
