file_name := "please give file name"
mode := "nodebug"

SRC := src/*
BIN := builds/compiler

run: $(BIN)
	$(BIN) $(file_name) $(mode)

$(BIN): $(SRC)
	g++  src/main.cpp -o builds/compiler

clean:
	rm -rf builds/*
