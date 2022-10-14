EXE := "spider_test.exe"
OBJS = $(patsubst ./%.c,./%.o,$(wildcard ./*.c))
OBJS += $(patsubst ./%.cpp,./%.opp,$(wildcard ./*.cpp))
$(info $(OBJS))

.PHONY: all clean
all: $(EXE)

run: $(EXE)
	@./$(EXE)

# Make test exe
# TODO: Make threading (-lpthread) conditional
$(EXE): $(OBJS)
	@echo 'Linking source file(s) $(OBJS) together into $@...'
	@$(CXX) -o "$@" $(OBJS) -lpthread
	@echo "Built $@"

# Build cpp
%.opp: %.cpp 
	@echo 'Building source file $<...'
	@$(CXX) --std=c++20 -g -c -o  "$@" "$<" 
	@echo 'Built $@'
	@echo

# Build c
%.o: %.c 
	@echo 'Building source file $<...'
	@$(CC) -std=gnu18 -g -c -o "$@" "$<"
	@echo 'Built $@'
	@echo

clean:
	@rm -f *.o* *.exe	
	@echo "All clean!"

