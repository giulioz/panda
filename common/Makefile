# include common definitions of DMT
include ../Makefile.include

	
# -------------------------------
# variales and targets
CXXFLAGS := -Wall --pedantic -O6 -finline-functions -DREENTRANT -fmessage-length=0 $(shell getconf LFS_CFLAGS 2>/dev/null)
C_OBJS_DIR := $(DMT_C_LIB_DIR)
C_SOURCES := $(wildcard $(DMT_COMMON_DIR)/c/src/*.cc)
C_HEADERS := $(wildcard $(DMT_COMMON_DIR)/c/inc/*.hh)
C_OBJECTS := $(subst $(DMT_COMMON_DIR)/c/src,$(C_OBJS_DIR), $(subst .cc,.o,$(C_SOURCES)) )

UNAME := $(shell uname)
PYVER := $(shell python -V 2>&1 | sed 's/Python \([0-9]\.[0-9]\).*/\1/')

$(C_OBJS_DIR):
	mkdir -p $(C_OBJS_DIR)

# -------------------------------
# C build process

$(C_OBJS_DIR)/%.o: $(DMT_COMMON_DIR)/c/src/%.cc $(DMT_COMMON_DIR)/c/inc/%.hh
	$(CXX) -c $(CXXFLAGS) $< -o $@
	
all: $(C_OBJS_DIR) $(C_OBJECTS)
	
clean:
	@echo "### -----------------------------------"
	@echo "### Cleaning Common Libs ..."
	rm -rf $(DMT_COMMON_DIR)/c/inc/*~ $(DMT_COMMON_DIR)/c/inc/*.bak
	rm -rf $(DMT_COMMON_DIR)/c/src/*~ $(DMT_COMMON_DIR)/c/src/*.bak
	rm -rf $(C_OBJS_DIR)/*
	