EXE := "./spider_test.exe"
OBJS += $(patsubst ./%.cpp,./%.opp,$(wildcard ./*.cpp))
LIB_PATH := $(shell dirname `pwd`)/build
LIB := "spider_cpp.so"
FULL_LIB := $(LIB_PATH)/$(LIB)

.PHONY: all clean
all: $(EXE)

$(EXE): $(OBJS) $(LIB)
	@echo 'Linking source file(s) $(OBJS) and lib $(LIB) together into $@...'
	@$(CXX) -o "$@" $(OBJS) -lpthread -L$(LIB_PATH) -l:$(LIB)
	@echo "Built $@"

$(LIB):
	@echo "Figure out how to call root makefile!"

# Build cpp
./%.opp: ./%.cpp 
	@echo 'Building source file $<...'
	@$(CXX) --std=c++20 -g -c -Wall -fpic -o  "$@" "$<" 
	@echo 'Built $@'
	@echo

clean:
	@rm -rf ./*.o* 
	@echo "All clean!"

outputs:
	@echo $(OBJS) $(EXE)

lib_path:
	@echo $(LIB_PATH)
	@echo $(LIB)
	@echo $(FULL_LIB)

run: $(EXE)
	@export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/$(LIB_PATH); $(EXE)