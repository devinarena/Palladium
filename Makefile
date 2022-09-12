
SRC = src
BIN = bin
CC = gcc

source := $(wildcard $(SRC)/*.c)
target := $(BIN)
output = palladium
alias = pal

all:
	@echo "Creating bin directory..."
	@mkdir -p $(target)
	@echo "Creating build target for Palladium..."
	$(CC) $(source) -o $(target)/$(output)
	@echo "Creating alias for Palladium..."
	cp $(target)/$(output) $(alias)
	@echo "Build complete."

clean:
	@echo "Cleaning up..."
	rm -rf $(target)
	rm -f $(alias)
	@echo "Cleanup complete."