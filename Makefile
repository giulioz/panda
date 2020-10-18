# include common definitions
include Makefile.include

# The tools in the package
DMT_TOOLS_DIRS := $(filter-out ./ ./xalgo/%,$(dir $(shell find . -iname Makefile -print) ) )
DMT_TOOLS := $(foreach dir,$(DMT_TOOLS_DIRS), $(shell basename $(dir)))

# always run the make command
#.PHONY: $(DMT_TOOLS) clean

# default target
all: $(DMT_BIN_DIR) $(DMT_TOOLS)

# build every package - except common
$(filter-out common, $(DMT_TOOLS)): # DMT_COMMON
	@echo "### -----------------------------------"
	@echo "### Building - $@ - :"
	$(MAKE) -e -C $(filter %/$@/, $(DMT_TOOLS_DIRS)) all
	@echo "### End Building -" $@ "- ."
	@echo "### -----------------------------------"

$(DMT_BIN_DIR):
	mkdir $(DMT_BIN_DIR)

# clean every package
clean:
	@echo "### -----------------------------------"
	@echo "### DMT cleaning $(CURDIR) ..."
	rm -rf *~ *.bak
	@$(foreach dir, $(DMT_TOOLS_DIRS), $(MAKE) -e -C $(dir) clean;)


%.pack: clean
	@echo $(DMT_TOOLS)
	@echo $(DMT_TOOLS_DIRS)
	@echo $@
	@echo $*
	@tar --exclude=*svn* -zcvf $*.tgz $(filter %$*/, $(DMT_TOOLS_DIRS)) Makefile Makefile.include common/c common/Makefile

