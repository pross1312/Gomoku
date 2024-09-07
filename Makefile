OUT_DIR=./build
SRC_DIR=./source
CC=g++
FLAGS=-Wall -Wextra -Wswitch -Wswitch-enum -Wno-literal-suffix -ggdb -I./raylib-5.0_linux_amd64/include/ -Wl,-rpath=./raylib-5.0_linux_amd64/lib -L./raylib-5.0_linux_amd64/lib
LIBS=-l:libraylib.a

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(subst $(SRC_DIR)/,$(OUT_DIR)/,$(subst .cpp,.o,$(SOURCES)))

.PHONY: build_dir dist-clean

all: build_dir $(OBJECTS) gomoku

dist-clean:
	rm -rf $(OUT_DIR)

build_dir:
	mkdir -p $(OUT_DIR)

$(OBJECTS): $(OUT_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) -c -o $@ $(FLAGS) $<

gomoku: $(SOURCES)
	$(CC) -o gomoku $(FLAGS) $(SOURCES) $(LIBS)
