LIBS=`pkg-config --libs sdl2`
INCLUDES=-Ilib/SDL/include -Ilib/glad/include -Ilib/stb_image
EXT_FILES=lib/glad/src/glad.c lib/stb_image/stb_image.c

build:
	gcc -lm -ldl -O3 -Wall -Wextra  `pkg-config --cflags sdl2` $(EXT_FILES) $(LIBS) $(INCLUDES) -o chess main.c chess.c types.c

