CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -O2 -MMD -MP
CFLAGS_DEBUG = -Wall -Wextra -g -std=c11 -O0 -DDEBUG -fsanitize=address,undefined
LDFLAGS = -lm

SRC_DIR = src
TEST_DIR = test
BUILD_DIR = build
TEST_BUILD_DIR = build/test

SRCS = src/board/bitboard.c \
       src/board/board.c \
       src/move/movegen.c \
       src/move/move.c \
       src/transposition/zobrist.c \
       src/transposition/transposition.c \
       src/perft/perft.c \
       src/search/search.c \
       src/evaluation/evaluate.c \
       src/evaluation/piece_value.c \
       src/evaluation/pawn_table.c \
       src/evaluation/nnue.c \
       src/evaluation/nnue_io.c \
       src/evaluation/nnue_update.c \
       src/uci/uci.c \
       src/heuristics/history.c \
       src/heuristics/killer.c \
       src/pruning/futility.c \
       src/pruning/lmr.c \
       src/pruning/lmp.c \
       src/pruning/null_move.c \
       src/search/progressive.c \
       src/time/time_mgmt.c

MAIN_SRC = src/main.c
OBJS = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRCS))
MAIN_OBJ = $(BUILD_DIR)/main.o

OBJS = $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SRCS))

TEST_OBJS = $(TEST_BUILD_DIR)/test_main.o \
            $(TEST_BUILD_DIR)/test_bitboard.o \
            $(TEST_BUILD_DIR)/test_board.o \
            $(TEST_BUILD_DIR)/test_movegen.o \
            $(TEST_BUILD_DIR)/test_zobrist.o \
            $(TEST_BUILD_DIR)/test_perft.o \
            $(TEST_BUILD_DIR)/test_search.o \
            $(TEST_BUILD_DIR)/test_tt.o \
            $(TEST_BUILD_DIR)/test_qsearch.o \
            $(TEST_BUILD_DIR)/test_pv.o \
            $(TEST_BUILD_DIR)/test_history.o \
            $(TEST_BUILD_DIR)/test_killer.o \
            $(TEST_BUILD_DIR)/test_futility.o \
            $(TEST_BUILD_DIR)/test_lmr.o \
            $(TEST_BUILD_DIR)/test_lmp.o \
            $(TEST_BUILD_DIR)/test_null_move.o \
            $(TEST_BUILD_DIR)/test_progressive.o \
            $(TEST_BUILD_DIR)/test_time_mgmt.o \
            $(TEST_BUILD_DIR)/test_piece_value.o \
            $(TEST_BUILD_DIR)/test_pawn_table.o \
            $(TEST_BUILD_DIR)/test_nnue.o \
            $(TEST_BUILD_DIR)/test_eval.o

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
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(MAIN_OBJ): $(MAIN_SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)/board $(BUILD_DIR)/move $(BUILD_DIR)/utils \
	          $(BUILD_DIR)/transposition $(BUILD_DIR)/perft \
	          $(BUILD_DIR)/search $(BUILD_DIR)/evaluation \
	          $(BUILD_DIR)/heuristics $(BUILD_DIR)/pruning \
	          $(BUILD_DIR)/time $(BUILD_DIR)/uci

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJS) $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(TEST_BUILD_DIR)/%.o: test/unit/%.c | $(TEST_BUILD_DIR)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $< -I$(SRC_DIR) -I$(SRC_DIR)/board -I$(SRC_DIR)/move -I$(SRC_DIR)/evaluation -I$(SRC_DIR)/transposition

$(TEST_BUILD_DIR):
	mkdir -p $(TEST_BUILD_DIR)/board $(TEST_BUILD_DIR)/move \
	          $(TEST_BUILD_DIR)/transposition $(TEST_BUILD_DIR)/perft \
	          $(TEST_BUILD_DIR)/evaluation

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
