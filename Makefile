CC = gcc
BISON = bison
FLEX = flex

SRC_DIR = src
BUILD_DIR = build

PARSER_C = $(BUILD_DIR)/parser.tab.c
PARSER_H = $(BUILD_DIR)/parser.tab.h
LEX_C = $(BUILD_DIR)/lex.yy.c
COMPILER_BIN = $(BUILD_DIR)/mamalangc

CFLAGS = -Wall -Wextra -O2

all: $(COMPILER_BIN)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(PARSER_C) $(PARSER_H): $(SRC_DIR)/parser.y | $(BUILD_DIR)
	$(BISON) -d -o $(PARSER_C) $(SRC_DIR)/parser.y

$(LEX_C): $(SRC_DIR)/lexer.l $(PARSER_H) | $(BUILD_DIR)
	$(FLEX) -o $(LEX_C) $(SRC_DIR)/lexer.l

$(COMPILER_BIN): $(PARSER_C) $(LEX_C)
	$(CC) $(CFLAGS) -I$(BUILD_DIR) $(PARSER_C) $(LEX_C) -o $(COMPILER_BIN)

run-sample: $(COMPILER_BIN)
	$(COMPILER_BIN) examples/sample.mama $(BUILD_DIR)/generated.c
	$(CC) $(CFLAGS) $(BUILD_DIR)/generated.c -o $(BUILD_DIR)/sample_program
	$(BUILD_DIR)/sample_program

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean run-sample
