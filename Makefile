# 1. Definiranje varijabli
CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -g -O2

# Popis svih objektnih datoteka koje ti trebaju
OBJS = evaluation.o graph.o greedy.o main.o output.o parser.o validation.o

# 2. PRVI CILJ - Ovo se pokreće kada upišeš samo 'make'
# Povezuje sve .o datoteke u konačni izvršni program 'main.exe'
main: $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o main

# 3. Eksplicitno pravilo za prevođenje .cpp u .o datoteke
# Sprječava 'make' da koristi svoja ugrađena 'cc' pravila
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 4. Čišćenje generiranih datoteka (za Windows)
clean:
	del /Q *.o main.exe 2>nul || rm -f *.o main
