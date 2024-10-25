# Compiler
CC = clang

# Compiler flags
CFLAGS = -Wall -g -Ilib/cjson  # Use the local cJSON include directory

# Linker flags for pthread library (no need for system-wide cJSON lib)
LDFLAGS = -lpthread -lcurl


# Source files
SRV_SRC = $(wildcard uchat-server/src/network/*.c) $(wildcard uchat-server/src/requests/*.c) $(wildcard uchat-server/src/*.c) $(wildcard uchat-database/src/*.c)
CLI_SRC = $(wildcard uchat-client/src/*.c)

# cJSON source files
CJSON_SRC = lib/cjson/cJSON.c

# Executable names
SRV_BIN = uchat_server
CLI_BIN = uchat

# Default target: compile both server and client
all: $(SRV_BIN) $(CLI_BIN)

# Compile the server
$(SRV_BIN): $(SRV_SRC) $(CJSON_SRC)
	$(CC) $(CFLAGS) -Iuchat-server/inc $(SRV_SRC) $(CJSON_SRC) -o $(SRV_BIN) $(LDFLAGS)

# Compile the client
$(CLI_BIN): $(CLI_SRC) $(CJSON_SRC)
	$(CC) $(CFLAGS) -Iuchat-client/inc $(CLI_SRC) $(CJSON_SRC) -o $(CLI_BIN) $(LDFLAGS)

# Clean up
clean:
	rm -f $(SRV_BIN) $(CLI_BIN)
	rm -r uchat_server.dSYM
	rm -r uchat.dSYM
