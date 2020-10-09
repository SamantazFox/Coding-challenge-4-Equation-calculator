
## User parameters ##

PRODUCT = calc


## Sources & objects ##

SRCS = \
	alloc_free.c \
	calculator.c \
	compute.c \
	parser.c \
	functions.c

OBJ_DIR = objs
OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRCS))


## Search paths ##

VPATH  = ./include
VPATH += ./src


## Build flags ##

INCLUDES = $(addprefix -I, $(VPATH))

CFLAGS = -std=c11 -Wall -pedantic -Wno-unused-function
LIBS   = -lm


## Compiler & linker config ##

CC = gcc
LD = gcc


## Main targets ##

$(PRODUCT): $(OBJS)
	@echo "LD -o $@ $(notdir $^)"
	@$(LD) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	@echo "CC -o $@ $(notdir $<)"
	@$(CC) -c $(CFLAGS) $(INCLUDES) -o $@ $< $(LIBS)


## Miscellanous targets ##

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm $(PRODUCT)
