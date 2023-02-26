set SRC_FILES=chess.c main.c types.c 
set EXT_FILES=lib/glad/src/glad.c lib/stb_image/stb_image.c
set LIBS=-Llib/SDL/lib -lmingw32 -lSDL2main -lSDL2
set INCLUDES=-Ilib/SDL/include -Ilib/glad/include -Ilib/stb_image
set DEFINES=-DSDL_MAIN_HANDLED -D_DEBUG
gcc %SRC_FILES% %EXT_FILES% %LIBS% %INCLUDES% %DEFINES% -o chess.exe