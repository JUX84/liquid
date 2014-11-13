BIN=bin/
OBJ=obj/
SRC=src/
INC=inc/
CXX=clang++ -std=c++14 -stdlib=libc++ -I$(INC)

all: $(BIN)server.bin $(BIN)client.bin
server: $(BIN)server.bin
client: $(BIN)client.bin
$(BIN)server.bin: $(OBJ)main.o $(OBJ)server.o $(OBJ)connectionHandler.o $(OBJ)requestHandler.o $(OBJ)parser.o $(OBJ)response.o $(OBJ)utility.o
	$(CXX) $(OBJ)*.o -o $(BIN)server.bin -lev -lz -lc++abi
$(BIN)client.bin: client/$(OBJ)client.o
	$(CXX) client/$(OBJ)client.o -o $(BIN)client.bin -lc++abi
client/$(OBJ)client.o: client/client.cpp
	$(CXX) -c client/client.cpp -o client/$(OBJ)client.o
$(OBJ)main.o: $(SRC)main.cpp
	$(CXX) -c $(SRC)main.cpp -o $(OBJ)main.o 
$(OBJ)server.o: $(SRC)server.cpp
	$(CXX) -c $(SRC)server.cpp -o $(OBJ)server.o
$(OBJ)connectionHandler.o: $(SRC)connectionHandler.cpp
	$(CXX) -c $(SRC)connectionHandler.cpp -o $(OBJ)connectionHandler.o
$(OBJ)requestHandler.o: $(SRC)requestHandler.cpp
	$(CXX) -c $(SRC)requestHandler.cpp -o $(OBJ)requestHandler.o
$(OBJ)utility.o: $(SRC)utility.cpp
	$(CXX) -c $(SRC)utility.cpp -o $(OBJ)utility.o
$(OBJ)response.o: $(SRC)response.cpp
	$(CXX) -c $(SRC)response.cpp -o $(OBJ)response.o
$(OBJ)parser.o: $(SRC)parser.cpp
	$(CXX) -c $(SRC)parser.cpp -o $(OBJ)parser.o
force: mrproper
	make all
mrproper: clean
	rm -f $(BIN)*.bin
clean:
	rm -f $(OBJ)*.o client/$(OBJ)*.o
