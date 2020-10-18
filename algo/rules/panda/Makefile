# include common definitions of DMT
include ../../../Makefile.include

# my own makefile
##CXXFLAGS := -O0 -g -Wall --pedantic `getconf LFS_CFLAGS` #-D_VERBOSE_ #-D_V_DEBUG_
CXXFLAGS := -Wall --pedantic -O3 -finline-functions -DREENTRANT -fmessage-length=0 $(shell getconf LFS_CFLAGS 2>/dev/null)

SOURCES  := $(filter-out costcompute.cc hdata.cc,$(wildcard *.cc))
OBJS_DIR := build
OBJS     := $(foreach file, $(SOURCES), $(OBJS_DIR)/$(subst .cc,.o,$(file)) )


INCS = $(DMT_INC)


LIBS = $(DMT_C_LIB_DIR)/chronos.o \
	$(DMT_C_LIB_DIR)/logger.o \
	$(DMT_C_LIB_DIR)/iohandler.o \
	$(DMT_C_LIB_DIR)/basicstruct.o \
	$(DMT_C_LIB_DIR)/nullmodel.o

# a number of ifdef would be useful
TARGETDIR = $(DMT_BIN_DIR)

TARGET = panda
COSTCOMPUTE = costcompute

all: DMT_COMMON $(TARGETDIR)/$(TARGET) $(TARGETDIR)/$(COSTCOMPUTE)

$(TARGET): all

$(OBJS_DIR):
	mkdir $(OBJS_DIR)
	
$(OBJS_DIR)/%.o: %.cc
	$(CXX) -c $(CXXFLAGS) -I$(INCS) $< -o $@

$(TARGETDIR)/$(TARGET): $(LIBS) $(OBJS_DIR) $(OBJS)
	$(CXX) -o $(TARGETDIR)/$(TARGET) $(LIBS) $(OBJS)
	strip $(TARGETDIR)/$(TARGET)

$(TARGETDIR)/$(COSTCOMPUTE): $(LIBS) $(OBJS_DIR) $(OBJS) $(OBJS_DIR)/costcompute.o
	$(CXX) -o $(TARGETDIR)/$(COSTCOMPUTE) $(LIBS) $(filter-out $(OBJS_DIR)/topk.o,$(OBJS)) $(OBJS_DIR)/costcompute.o
	strip $(TARGETDIR)/$(COSTCOMPUTE)


clean:
	@echo "### Cleaning $(TARGET) ..."
	rm -rf $(OBJS_DIR) *~ *.bak
	
# to be removed / updated

depend:
	makedepend $(CXXFLAGS) $(SOURCES)

pack:
	tar zcvf $(DMT_ALGO_DIR)/$(TARGET).tgz ./
