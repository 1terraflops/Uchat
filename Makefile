CC = clang
FLAGS = -std=c11 -Wall -Wextra -Werror -Wpedantic -D_GNU_SOURCE -pthread
INC = -Iinc
GTK_CFLAGS = `pkg-config --cflags gtk+-3.0`
GTK_LIBS = `pkg-config --libs gtk+-3.0`
SRC_CLIENT = $(wildcard src/client/*.c)
OBJ_CLIENT = $(SRC_CLIENT:src/client/%.c=obj/client/%.o)
SRC_SERVER = $(wildcard src/server/*.c)
OBJ_SERVER = $(SRC_SERVER:src/server/%.c=obj/server/%.o)
LIBS = libs/libmx.a

all: uchat uchat_server

# Compile the client side
uchat: $(OBJ_CLIENT) $(LIBS)
	$(CC) $(FLAGS) $(INC) libs/sqlite3.c $(OBJ_CLIENT) $(LIBS) $(GTK_LIBS) -o uchat

obj/client/%.o: src/client/%.c | obj
	$(CC) $(FLAGS) $(INC) $(GTK_CFLAGS) -c $< -o $@

# Compile the server side
uchat_server: $(OBJ_SERVER) $(LIBS)
	$(CC) $(FLAGS) $(INC) libs/sqlite3.c $(OBJ_SERVER) $(LIBS) -o uchat_server

obj/server/%.o: src/server/%.c | obj
	$(CC) $(FLAGS) $(INC) $(GTK_CFLAGS) -c $< -o $@

obj:
	mkdir -p obj/client
	mkdir -p obj/server

clean:
	rm -rf obj

uninstall:
	rm -rf obj uchat uchat_server

reinstall:
	make uninstall
	make
