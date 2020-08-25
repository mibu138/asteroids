CC = gcc
CFLAGS = -Wall 
LIBS = -lvulkan -lxcb -lxcb-keysyms -lm
O = build
GLSL = shaders
SPV  = shaders/spv

NAME = as

DEPS =                 \
		d_display.h    \
		r_render.h     \
		r_commands.h   \
		r_pipeline.h   \
		g_game.h       \
		m_math.h       \
		w_world.h      \
		w_collision.h  \
		w_create.h     \
		z_memory.h     \
		i_input.h      \
		utils.h        \
		def.h

OBJS = 					    \
		$(O)/d_display.o    \
		$(O)/r_render.o     \
		$(O)/r_pipeline.o   \
		$(O)/r_commands.o   \
		$(O)/g_game.o       \
		$(O)/m_math.o       \
		$(O)/w_world.o      \
		$(O)/w_collision.o  \
		$(O)/w_create.o     \
		$(O)/z_memory.o     \
		$(O)/i_input.o      \
		$(O)/utils.o

SHADERS =                      \
		$(SPV)/simple-vert.spv \
		$(SPV)/simple-frag.spv \

debug: CFLAGS += -g -DVERBOSE=1
debug: all

release: CFLAGS += -DNDEBUG -O2
release: all

all: $(O)/$(NAME) tags

shaders: $(SHADERS)

clean: 
	rm -f $(O)/*

tags:
	ctags -R .
	
$(O)/$(NAME): main.c $(OBJS) $(DEPS)
	$(CC) $(CFLAGS) $(OBJS) $< -o $@ $(LIBS)

$(O)/%.o:  %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

$(SPV)/%-vert.spv: $(GLSL)/%.vert
	glslc $< -o $@
