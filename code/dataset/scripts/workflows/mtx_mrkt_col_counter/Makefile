TARGET := print_pathNcol
SRC_DIR := src
OBJ_DIR := obj
MTX_MRKT_DIR := ../../../../mtx_mrkt_csr/include/matrix_market

SRC_DIRS := $(SRC_DIR) $(MTX_MRKT_DIR)
SRCS := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
OBJS := $(addprefix $(OBJ_DIR)/,$(notdir $(SRCS:.c=.o)))
DEPS := $(OBJS:.o=.d)

vpath %.c $(SRC_DIRS)

STD ?= c17

CFLAGS := -Wall -Wextra -std=$(STD)
CPPFLAGS := -MMD -MP

.PHONY: all clean

all: $(TARGET)

# link stage
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@

# compile object files
$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# removes both object files and binary
clean:
	rm -rf $(OBJ_DIR)
	rm $(TARGET)

-include $(DEPS)
