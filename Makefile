# Compiler
CC = clang

# Compiler flags
CFLAGS = -g -Ilib/cjson $(shell pkg-config --cflags openssl)  # Use pkg-config for OpenSSL

# Linker flags for pthread library and OpenSSL
LDFLAGS = -lpthread $(shell pkg-config --libs openssl)  # Use pkg-config for OpenSSL

GSTREAMER_FLAGS = $(shell pkg-config --cflags gstreamer-1.0)
GSTREAMER_LIBS = $(shell pkg-config --libs gstreamer-1.0)

# Source files
SRV_SRC = $(wildcard uchat-server/src/network/*.c) $(wildcard uchat-server/src/requests/*.c) $(wildcard uchat-server/src/utils/*.c) $(wildcard uchat-server/src/*.c) $(wildcard uchat-database/src/*.c)
CLI_SRC = $(wildcard uchat-client/src/*.c) $(wildcard uchat-client/src/network/*.c) $(wildcard uchat-client/src/utils/*.c) $(wildcard uchat-client/src/responces/*.c) $(wildcard uchat-client/src/gui/*.c)

# Additional source files
CJSON_SRC = lib/cjson/cJSON.c
SHA256_SRC = lib/SHA256/sha256.c

# Executable names
SRV_BIN = uchat_server
CLI_BIN = uchat

# Default target: compile both server and client
all: $(SRV_BIN) $(CLI_BIN)

# Compile the server
$(SRV_BIN): $(SRV_SRC) $(CJSON_SRC) $(SHA256_SRC)
	$(CC) $(CFLAGS) -Iuchat-server/inc -Iuchat-database/inc -Ilib/SHA256 $(SRV_SRC) $(CJSON_SRC) $(SHA256_SRC) -o $(SRV_BIN) $(LDFLAGS) -lsqlite3

# Compile the client with GTK+3.0 support
$(CLI_BIN): $(CLI_SRC) $(CJSON_SRC)
	$(CC) $(CFLAGS) $(GSTREAMER_FLAGS) -Iuchat-client/inc $(CLI_SRC) $(CJSON_SRC) -o $(CLI_BIN) $(LDFLAGS) $(shell pkg-config --cflags --libs gtk+-3.0) $(GSTREAMER_LIBS)

# Clean up
clean:
	rm -f $(SRV_BIN) $(CLI_BIN)
	rm -rf uchat_server.dSYM
	rm -rf uchat.dSYM
	rm -f session.txt
	rm -rf cache