OBJ_DIR = obj
SRC_DIR = src
INC_DIR = include
BIN_DIR = bin

OBJETOS = client.o main.o memory.o process.o proxy.o server.o synchronization.o

main.o = $(INC_DIR)/main.h  $(INC_DIR)/memory.h  $(INC_DIR)/synchronization.h  $(INC_DIR)/process.h
process.o = $(INC_DIR)/main.h  $(INC_DIR)/client.h $(INC_DIR)/server.h  $(INC_DIR)/proxy.h $(INC_DIR)/process.h

CC = gcc

vpath %.o $(OBJ_DIR)
vpath %.h $(INC_DIR)

CFLAGS = -Wall -I $(INC_DIR) -g

LIBS = -lrt -lpthread


sovaccines : $(OBJETOS)
	$(CC) $(CFLAGS) $(addprefix $(OBJ_DIR)/,$(OBJETOS)) -o $(BIN_DIR)/$@ $(LIBS)

%.o: $(SRC_DIR)/%.c $($@) 
	$(CC) $(CFLAGS) -o $(OBJ_DIR)/$@ -c $<

clean :
	rm -f $(OBJ_DIR)/*.o $(BIN_DIR)/sovaccines

valgrind:
	valgrind --leak-check=full --dsymutil=yes --show-leak-kinds=all $(BIN_DIR)/sovaccines 10 3 3 4 5

exec:
	$(BIN_DIR)/sovaccines 10 3 3 4 5