CC = gcc
CFLAGS = -Wall -g -DVERBOSE=1 
LIBS = -lvulkan -lxcb
O = build

NAME = as

DEPS =              \
		d_display.h \
		r_render.h  \
		def.h

OBJS = 					 \
		$(O)/d_display.o \
		$(O)/r_render.o  

all: $(O)/$(NAME)

clean: 
	rm -f $(O)/*

$(O)/$(NAME): main.c $(OBJS) $(DEPS)
	$(CC) $(CFLAGS) $(OBJS) $< -o $@ $(LIBS)

$(O)/%.o:  %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@
