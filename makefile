OBJ_DIR = obj
SRC_DIR = src
INC_DIR = include
BIN_DIR = bin

OBJETOS = client.o main.o memory.o process.o proxy.o server.o synchronization.o

CC = gcc

vpath %.o $(OBJ_DIR)
vpath %.h $(INC_DIR)

CFLAGS = -Wall -I $(INC_DIR)

sovaccines : $(OBJETOS)
	$(CC) $(CFLAGS) $(addprefix $(OBJ_DIR)/,$(OBJETOS)) -o $(BIN_DIR)/$@ 

%.o: $(SRC_DIR)/%.c $($@) 
	$(CC) $(CFLAGS) -o $(OBJ_DIR)/$@ -c $<

clean :
	rm -f $(OBJ_DIR)/*.o $(BIN_DIR)/sovaccines

valgrind:
	valgrind --leak-check=full $(BIN_DIR)/sovaccines