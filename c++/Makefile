BUILD_DIR := "./build"
SO := "$(BUILD_DIR)/spider_cpp.so"
OBJS += $(patsubst ./source/%.cpp,./build/%.opp,$(wildcard ./source/*.cpp))



.PHONY: all clean
all: $(SO)

# Make Shared lib
# TODO: Make threading (-lpthread) conditional
$(SO): $(OBJS)
	@echo 'Linking source file(s) $(OBJS) together into $@...'
	@$(CXX) -shared -fpic -o "$@" $(OBJS) -lpthread -Wl,-rpath,`realpath $(BUILD_DIR)`
	@echo "Built $@"

# Build cpp
./build/%.opp: ./source/%.cpp ./build
	@echo 'Building source file $<...'
	@$(CXX) --std=c++20 -g -c -Wall -fpic -o  "$@" "$<" 
	@echo 'Built $@'
	@echo

./build:
	mkdir -p ./build

clean:
	@rm -rf ./build/*.o* ./build/*.so ./build 
	@echo "All clean!"

outputs:
	@echo $(OBJS) $(SO)

test:
	@echo "Not ready yet!"

