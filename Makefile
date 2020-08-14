CC = gcc
CFLAGS = -Wall -g -DVERBOSE=1 
LIBS = -lvulkan -lxcb
O = build

NAME = as

DEPS =              \
		d_display.h \
		r_render.h  \
		g_game.h    \
		m_math.h    \
		w_world.h   \
		z_memory.h  \
		utils.h     \
		def.h

OBJS = 					  \
		$(O)/d_display.o  \
		$(O)/r_render.o   \
		$(O)/r_pipeline.o \
		$(O)/g_game.o     \
		$(O)/w_world.o    \
		$(O)/z_memory.o   \
		$(O)/utils.o

all: $(O)/$(NAME) tags

clean: 
	rm -f $(O)/*

tags: 
	ctags .

$(O)/$(NAME): main.c $(OBJS) $(DEPS)
	$(CC) $(CFLAGS) $(OBJS) $< -o $@ $(LIBS)

$(O)/%.o:  %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@
