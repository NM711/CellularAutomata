#include <iostream>
#include <thread>
#include <vector>
#include "../window.hpp"

class Automaton {
  public:
    Automaton() {
      this->windowInfo = WindowUtil::getTermWindowInfo();
    };
    void execute() {
      const unsigned NEIGHBORHOOD_SIZE = windowInfo->ws_col;

      std::vector<int> neighborhood = std::vector(NEIGHBORHOOD_SIZE, 0);
      int random = rand() % NEIGHBORHOOD_SIZE;
      neighborhood.at(random) = 1;

      for (int generation = 0; generation < 80; ++generation) {
        this->printStates(neighborhood, NEIGHBORHOOD_SIZE);

        // We create a new array in order to store the new generation.
        // If we decided to update the original neighborhood directly with the outcome state of the rule in that iteration,
        // We would be modifying the intended outcome of the entire original neighborhood vector because that one updated
        // value will Deviate future iterations from what we expect.

        // Each cell state will ultimately be dependant on original states of its neighbors.

        std::vector<int> newNeighborhood = std::vector(NEIGHBORHOOD_SIZE, 0);
        this->populateNewGeneration(newNeighborhood, neighborhood, NEIGHBORHOOD_SIZE);
        
        for (int i = 0; i < NEIGHBORHOOD_SIZE; ++i) {
          neighborhood.at(i) = newNeighborhood.at(i);
        };
      };
    };

  ~Automaton() {
    delete this->windowInfo;
  };
  
  private:
    winsize *windowInfo;
    
    void populateNewGeneration(std::vector<int> &newGeneration, std::vector<int> &oldGeneration, int size) {
      for (int i = 1; i < size - 1; ++i) {
        int prev = oldGeneration.at(i - 1);
        int current = oldGeneration.at(i);
        int next = oldGeneration.at(i + 1);

        int state = this->rule30(prev, current, next);
        newGeneration.at(i) = state;
      };
    };

    void printStates(std::vector<int> &neighborhood, int size) {
      for (int i = 0; i < size; ++i) {
        int cell = neighborhood.at(i);

        if (cell == 0) {
          std::cout << ".";
        } else {
          std::cout << "@";
        };
      };

      std::cout << std::endl;
    };

    // I will implement Wolframs rule 30 so I can get some interesting visuals.
    // 111 = DEAD
    // 110 = DEAD
    // 101 = DEAD
    // 100 = ALIVE
    // 011 = ALIVE
    // 010 = ALIVE
    // 001 = ALIVE
    // 000 = DEAD

    int rule30(int left, int center, int right) {
      if (left == 1 && center == 1 && right == 1) {
        return 0;
      } else if (left == 1 && center == 1 && right == 0) {
        return 0;
      } else if (left == 1 && center == 0 && right == 1) {
        return 0;
      } else if (left == 1 && center == 0 && right == 0) {
        return 1;
      } else if (left == 0 && center == 1 && right == 1) {
        return 1;
      } else if (left == 0 && center == 1 && right == 0) {
        return 1;
      } else if (left == 0 && center == 0 && right == 1) {
        return 1;
      } else if (left == 0 && center == 0 && right == 0) {
        return 0;
      } else {
        return 0;
      };
    };
};

int main() {
  srand(time(nullptr));
  Automaton automaton;

  while (true) {
    automaton.execute();
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  // automaton.execute();
  
  return 0;
};
