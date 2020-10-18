# include common definitions of DMT
include ../Makefile.include

	
# -------------------------------
# variales and targets
CXXFLAGS := -Wall --pedantic -O6 -finline-functions -DREENTRANT -fmessage-length=0 $(shell getconf LFS_CFLAGS 2>/dev/null)
C_OBJS_DIR := $(DMT_C_LIB_DIR)
C_SOURCES := $(wildcard $(DMT_COMMON_DIR)/c/src/*.cc)
C_HEADERS := $(wildcard $(DMT_COMMON_DIR)/c/inc/*.hh)
C_OBJECTS := $(subst $(DMT_COMMON_DIR)/c/src,$(C_OBJS_DIR), $(subst .cc,.o,$(C_SOURCES)) )

PY_BUILD_DIR := tmp-python-build
PY_TARGETS := $(DMT_PY_LIB_DIR)/_$(DMT_PY_LIB_NAME).so

UNAME := $(shell uname)
PYVER := $(shell python -V 2>&1 | sed 's/Python \([0-9]\.[0-9]\).*/\1/')

all: $(C_OBJS_DIR) $(C_OBJECTS) $(PY_BUILD_DIR) $(DMT_PY_LIB_DIR) $(PY_TARGETS)
	rm -rf $(PY_BUILD_DIR)

$(C_OBJS_DIR):
	mkdir -p $(C_OBJS_DIR)

# -------------------------------
# C build process

$(C_OBJS_DIR)/%.o: $(DMT_COMMON_DIR)/c/src/%.cc $(DMT_COMMON_DIR)/c/inc/%.hh
	$(CXX) -c $(CXXFLAGS) $< -o $@

# -------------------------------
#  Python build process

# Mac OS X
# swig -c++ -I../inc/ -module basicstruct -python basicstruct.swig
# g++ -I../inc -c basicstruct_wrap.cxx ../src/basicstruct.cc -I/usr/include/python2.6
# g++ -bundle -flat_namespace -undefined suppress  basicstruct.o basicstruct_wrap.o -o _basicstruct.so 

$(PY_BUILD_DIR):
	mkdir -p $(PY_BUILD_DIR)

$(DMT_PY_LIB_DIR):
	mkdir -p $(DMT_PY_LIB_DIR)

$(PY_BUILD_DIR)/%.hh: $(C_HEADERS)
	cat $? > $@

$(PY_BUILD_DIR)/%.cc: $(C_SOURCES)
	cat $? > $@

$(PY_BUILD_DIR)/%.swig: $(PY_BUILD_DIR)/%.hh $(PY_BUILD_DIR)/%.cc
	@echo "generate $@"
	@rm -rf $@ # clean up old swigs 
	@echo "%module" $(basename $(notdir $@)) >> $@
	@echo " %{" >> $@
	@echo " #include \"$<\"" >> $@  
	@echo " %}" >> $@
	@#echo "#undef __DMT_$(shell echo $(basename $(notdir $@)) | tr a-z A-Z)_H" >> $@
	@echo " %include \"$<\"" >> $@  

$(PY_BUILD_DIR)/%_wrap.cxx: $(PY_BUILD_DIR)/%.swig
	swig -c++ -I$(DMT_INC) -python -o $@ $<
	@# this also creates the .py file

$(PY_BUILD_DIR)/%_wrap.o: $(PY_BUILD_DIR)/%_wrap.cxx
	$(CXX) -fPIC $(filter-out --pedantic, $(CXXFLAGS)) -I$(DMT_INC) -I$(PY_BUILD_DIR)/.. -I/usr/include/python$(PYVER) -c $< -o $@   

$(PY_BUILD_DIR)/%.o: $(PY_BUILD_DIR)/%.cc #$(DMT_COMMON_DIR)/c/inc/%.hh
	$(CXX) -fPIC -c $(CXXFLAGS) -Ic/inc $< -o $@

$(DMT_PY_LIB_DIR)/_%.so: $(PY_BUILD_DIR)/%.o $(PY_BUILD_DIR)/%_wrap.o
ifeq ($(UNAME),Darwin)
	$(CXX) $(filter-out --pedantic, $(CXXFLAGS)) -bundle -flat_namespace -undefined suppress $(PY_BUILD_DIR)/$*_wrap.o $(PY_BUILD_DIR)/$*.o -o $@
else
	$(CXX) $(filter-out --pedantic, $(CXXFLAGS)) -shared $(PY_BUILD_DIR)/$*_wrap.o $(PY_BUILD_DIR)/$*.o  -o $@
endif
	cp $(PY_BUILD_DIR)/$(basename $(notdir $<)).py $(DMT_PY_LIB_DIR)


#PY_TARGETS := $(subst $(DMT_COMMON_DIR)/src/,$(DMT_PY_LIB_DIR)/_, $(subst .cc,.so,$(C_SOURCES)) )
#SWIGS := $(subst $(DMT_COMMON_DIR)/src,$(PY_BUILD_DIR), $(subst .cc,.swig,$(C_SOURCES)) )
#WRAPPERS := $(subst $(DMT_COMMON_DIR)/src,$(PY_BUILD_DIR), $(subst .cc,_wrap.cxx,$(C_SOURCES)) )
#WRAPPERS_O := $(subst $(DMT_COMMON_DIR)/src,$(PY_BUILD_DIR), $(subst .cc,_wrap.o,$(C_SOURCES)) )
##.SECONDARY: $(SWIGS) $(WRAPPERS) $(WRAPPERS_O)
#
#
#$(PY_BUILD_DIR)/%.swig: $(DMT_INC)/%.hh
#	@echo "generate $@"
#	@rm -rf $@ # clean up old swigs 
#	@echo "%module" $(basename $(notdir $@)) >> $@
#	@echo " %{" >> $@
#	@echo " #include \"$(DMT_INC)/$(basename $(notdir $@)).hh\"" >> $@  
#	@echo " %}" >> $@
#	@echo "#undef __DMT_$(shell echo $(basename $(notdir $@)) | tr a-z A-Z)_H" >> $@
#	@echo " %include \"$<\"" >> $@  
#
#$(PY_BUILD_DIR)/%_wrap.cxx: $(PY_BUILD_DIR)/%.swig
#	swig -c++ -I$(DMT_INC) -python -o $@ $<
#	@#MAC OS X: swig -c++ -I$(DMT_INC) -python -module $(basename $(notdir $<)) -o $@ $<
#	@# swig -c++ -I../inc/ -module basicstruct -python basicstruct.swig
#	@# this also creates the .py file
#
#$(PY_BUILD_DIR)/%_wrap.o: $(PY_BUILD_DIR)/%_wrap.cxx
#	$(CXX) -fPIC $(filter-out --pedantic, $(CXXFLAGS)) -I$(DMT_INC) -I/usr/include/python2.6 -c $< -o $@   
#	@#MAX OS X: $(CXX) $(filter-out --pedantic, $(CXXFLAGS)) -I$(DMT_INC) -I/usr/include/python2.6 -c $< -o $@   
#	@# g++ -I../inc -c basicstruct_wrap.cxx ../src/basicstruct.cc -I/usr/include/python2.6
#	
#$(DMT_PY_LIB_DIR)/_%.so: $(C_OBJS_DIR)/%.o $(PY_BUILD_DIR)/%_wrap.o
#	$(CXX) $(filter-out --pedantic, $(CXXFLAGS)) -shared $(PY_BUILD_DIR)/$*_wrap.o $(C_OBJS_DIR)/$*.o  -o $@
#	@#$(CXX) $(filter-out --pedantic, $(CXXFLAGS)) -shared $^ -o $@
#	@#MAC OS X: $(CXX) $(filter-out --pedantic, $(CXXFLAGS)) -bundle -flat_namespace -undefined suppress $? -o $@
#	@# g++ -bundle -flat_namespace -undefined suppress  basicstruct.o basicstruct_wrap.o -o _basicstruct.so
#	cp $(PY_BUILD_DIR)/$(basename $(notdir $<)).py $(DMT_PY_LIB_DIR) 
	
clean:
	@echo "### -----------------------------------"
	@echo "### Cleaning Common Libs ..."
	rm -rf $(DMT_COMMON_DIR)/c/inc/*~ $(DMT_COMMON_DIR)/c/inc/*.bak
	rm -rf $(DMT_COMMON_DIR)/c/src/*~ $(DMT_COMMON_DIR)/c/src/*.bak
	rm -rf $(C_OBJS_DIR)/*
	rm -rf $(PY_BUILD_DIR)
	rm -rf $(DMT_PY_LIB_DIR)/*
	
