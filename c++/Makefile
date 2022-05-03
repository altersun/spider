
EXE = $(patsubst ./%.cpp,./%.exe,$(wildcard ./*.cpp))

.PHONY: all clean
all: $(EXE)

run: $(EXE)
	@$(EXE)

%.exe: %.cpp
	@$(CXX) -g --std=c++20 -o "$@" "$<"
	@echo "Built test"

clean:
	@rm -f $(EXE)	 	
	@echo "All clean!"
