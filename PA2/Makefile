CXX=gcc
SRC=src/pa2.c
BIN=bin/pa2.o
TARGET=pa2.out


$(TARGET): $(BIN)
	$(CXX) -o $(TARGET) $(BIN)

$(BIN): $(SRC)
	$(CXX) -c -o $(BIN) $(SRC)


clean:
	rm $(TARGET) $(BIN)