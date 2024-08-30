FLAGS=-Wall -Wextra -Ofast -I./raylib-5.0_linux_amd64/include/ -Wl,-rpath=./raylib-5.0_linux_amd64/lib -L./raylib-5.0_linux_amd64/lib
LIBS=-l:libraylib.a
gomoku: $(wildcard source/*)
	g++ -o gomoku $(FLAGS) source/*.cpp $(LIBS)
