PRODUCT = calc

SRCS = \
	alloc_free.c \
	calculator.c \
	compute.c \
	parser.c 

OBJS = $(SRCS:.c=.o)


INCLUDES = -I./include


CFLAGS = -std=c11 -Wall -pedantic
LIBS   = -lm



CC = gcc
LD = gcc


$(PRODUCT): $(OBJS)
	@echo "LD -o $@ $^"
	@$(LD) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

%.o: %.c
	@echo "CC -o $@ $<"
	@$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $< $(LIBS)


clean:
	rm *.o
