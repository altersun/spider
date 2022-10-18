EXE := "./spider_test.exe"
OBJS += $(patsubst ./%.cpp,./%.opp,$(wildcard ./*.cpp))
LIB := "../build/spider_cpp.so"

.PHONY: all clean
all: $(EXE)

$(EXE): $(OBJS) $(LIB)
	@echo 'Linking source file(s) $(OBJS) and lib $(LIB) together into $@...'
	@$(CXX) -o "$@" $(OBJS) -lpthread -L ../build/ -l:spider_cpp.so
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