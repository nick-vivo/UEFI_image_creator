.POSIX:
.PHONY: all clean

TARGET = write_gpt
SRC = write_gpt.c src/uefi_gpt.c src/uefi_lba.c src/uefi_mbr.c src/config.c src/uefi_fat32.c
INCLUDE = -Iinclude

CC = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -O2

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(INCLUDE) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET) *.img
