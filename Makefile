# variables
COMP_C := g++
FLAGS_C := -Wall -O3 -std=c++11
TARGETDIR=.

main: main.o tecplotread.o
	$(COMP_C) $(FLAGS_C) -o main.bin main.o tecplotread.o

# Compile all source files .cpp into .o files
$(TARGETDIR)/%.o: $(TARGETDIR)/%.cpp
	$(COMP_C) $(FLAGS_C) -c $< -o $@

# Compile all source files .cpp into .o files
$(TARGETDIR)/%.o: $(TARGETDIR)/%.cpp $(TARGETDIR)/%.h
	$(COMP_C) $(FLAGS_C) -c $< -o $@

clean:
	rm -f *.o;
	rm -f main.bin;
