FLAGS=-Wall -Wextra -ggdb
gomoku: $(wildcard source/*)
	g++ -o gomoku $(FLAGS) source/*.cpp
