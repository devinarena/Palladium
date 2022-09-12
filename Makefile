
SRC = src
BIN = bin
CC = gcc
output = palladium
alias = pal
debug_file = file.pd

source := $(wildcard $(SRC)/*.c)
target := $(BIN)

all:
	@echo "Creating bin directory..."
	@mkdir -p $(target)
	@echo "Creating build target for Palladium..."
	$(CC) $(source) -g -o $(target)/$(output)
	@echo "Creating alias for Palladium..."
	cp $(target)/$(output) $(alias)
	@echo "Build complete."

debug: all
	@echo "Running debug file..."
	$(target)/$(output) $(debug_file)

clean:
	@echo "Cleaning up..."
	rm -rf $(target)
	rm -f $(alias)
	@echo "Cleanup complete."