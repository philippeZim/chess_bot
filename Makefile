CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -O2 -MMD -MP
CFLAGS_DEBUG = -Wall -Wextra -g -std=c11 -O0 -DDEBUG -fsanitize=address,undefined
LDFLAGS = 

SRC_DIR = src
TEST_DIR = test
BUILD_DIR = build
TEST_BUILD_DIR = build/test

SRCS = src/board/bitboard.c \
       src/board/board.c \
       src/move/movegen.c \
       src/move/move.c \
       src/transposition/zobrist.c

MAIN_SRC = src/main.c
OBJS = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRCS))
MAIN_OBJ = $(BUILD_DIR)/main.o

OBJS = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRCS))

TEST_OBJS = $(TEST_BUILD_DIR)/test_bitboard.o \
            $(TEST_BUILD_DIR)/test_board.o

TARGET = chess_bot
TEST_TARGET = chess_test

.PHONY: all test clean debug coverage benchmark

all: $(TARGET)

debug: CFLAGS = $(CFLAGS_DEBUG)
debug: LDFLAGS += -fsanitize=address,undefined
debug: $(TARGET)

$(TARGET): $(OBJS) $(MAIN_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: src/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(MAIN_OBJ): $(MAIN_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/board
	mkdir -p $(BUILD_DIR)/move
	mkdir -p $(BUILD_DIR)/transposition

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJS) $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(TEST_BUILD_DIR)/%.o: test/unit/%.c | $(TEST_BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $< -I$(SRC_DIR) -I$(SRC_DIR)/board

$(TEST_BUILD_DIR):
	mkdir -p $(TEST_BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(TEST_TARGET) $(TARGET)

coverage: CFLAGS += --coverage
coverage: LDFLAGS += --coverage
coverage: test
	gcov $(BUILD_DIR)/*.o
	lcov -c -d $(BUILD_DIR) -o coverage.info
	genhtml coverage.info -o coverage_html

benchmark: CFLAGS += -O3 -march=native
benchmark: $(TARGET)
	./$(TARGET) bench

-include $(OBJS:.o=.d)
-include $(TEST_OBJS:.o=.d)
