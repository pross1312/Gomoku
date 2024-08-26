FLAGS=-Wall -Wextra -ggdb
gomoku: BitBoard.cpp main.cpp Ui.cpp
	g++ -o gomoku $(FLAGS) BitBoard.cpp main.cpp Ui.cpp
