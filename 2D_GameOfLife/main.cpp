#include "../window.hpp"
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

const int GRID_ROWS = 32;
const int GRID_COLS = WindowUtil::getTermWindowInfo()->ws_col;
const int GENERATION_COUNT = 1000;

struct EntryPoint {
    int row;
    int col;
};

enum Pattern {
  OSCILLATOR_ANGEL_BLINKER,
  STILL_BLOCK,
  SPACESHIP_GLIDER
};

static int generateRandomNumber(int x, int y) {
  static std::mt19937 mt;
  std::uniform_int_distribution<int> distribution(x, y);
  return distribution(mt);
};

/*
  Game Rules:

  1. If a cell has less than 2 live neighbors it will die - Underpopulation
  2. If a cell has 2 OR 3 live neighbors it will survive onto the next generation - Survival
  3. If a cell has more than 3 neighbors it will die - Overpopulation
  4. If a dead cell has 3 live neighbors it will regain life - Reproduction
*/

typedef std::vector<std::vector<int>> Grid;

class GameOfLife {
  public:
    GameOfLife() {
      this->grid = this->initGrid();
    };

    void execute() {
      int generation = 0;

      while (true) {
        system("clear");
        std::cout << "Generation: " << generation << std::endl;
        std::cout << std::endl;
        this->printGrid();

        // render updated grid then check for extinction to end game.
        if (this->isExtinction()) {
          break;
        };

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        Grid newGeneration = this->initGrid();

        for (int row = 0; row < GRID_ROWS; ++row) {
          for (int col = 0; col < GRID_COLS; ++col) {

            int liveNeighbors = this->getNumOfNeighbors(row, col);
            int currentGenerationCell = this->grid.at(row).at(col);
            int *newGenerationCell = &newGeneration.at(row).at(col);

            if (currentGenerationCell == 1 && liveNeighbors < 2) {
              *newGenerationCell = 0;
            } else if (currentGenerationCell == 1 && (liveNeighbors == 2 || liveNeighbors == 3)) {
              *newGenerationCell = this->grid.at(row).at(col);
            } else if (currentGenerationCell == 1 && liveNeighbors > 3) {
              *newGenerationCell = 0;
            } else if (currentGenerationCell == 0 && liveNeighbors == 3) {
              *newGenerationCell = 1;
            };
          };
        };

        for (int row = 0; row < GRID_ROWS; ++row) {
          for (int col = 0; col < GRID_COLS; ++col) {
            this->grid.at(row).at(col) = newGeneration.at(row).at(col);
          };
        };

        ++generation;
      };

      this->logEntrypoints();

      std::cout << "Game Over!" << std::endl;
    };

    /*
      Given a random entry point cell you can create a premade pattern on the grid.
      If I want to implement custom patterns, I would need to implement the feature later on.
    */

    void createPattern(Pattern pattern) {
      EntryPoint entrypoint = this->seed();
      this->logEntrypoints();

      this->grid.at(entrypoint.row).at(entrypoint.col) = 1;

      // This could all be much better, but frick it xD

      switch (pattern) {
        case Pattern::STILL_BLOCK:

          if (entrypoint.row < GRID_ROWS - 1 && entrypoint.col < GRID_COLS - 1) {
            this->grid.at(entrypoint.row + 1).at(entrypoint.col) = 1;
            this->grid.at(entrypoint.row).at(entrypoint.col + 1) = 1;
          };
          break;

        case Pattern::SPACESHIP_GLIDER:
          //   @
          // @ @
          //  @@
          if (entrypoint.row < GRID_ROWS - 1 && entrypoint.col < GRID_COLS - 2) {
            this->grid.at(entrypoint.row + 1).at(entrypoint.col + 2) = 1;
            this->grid.at(entrypoint.row + 1).at(entrypoint.col + 1) = 1;
            // move two columns to the side, so we can make the gliders peak
            this->grid.at(entrypoint.row).at(entrypoint.col + 2) = 1;
            this->grid.at(entrypoint.row - 1).at(entrypoint.col + 2) = 1;
          };
          break;

        case Pattern::OSCILLATOR_ANGEL_BLINKER:
          std::cout << "OSCI" << std::endl;
          if ((entrypoint.row > 3 && entrypoint.row < GRID_ROWS - 4) &&
              (entrypoint.col < GRID_COLS - 2 && entrypoint.col > 1)) {
            // left hand side of angel
            this->grid.at(entrypoint.row - 1).at(entrypoint.col - 1) = 1;
            this->grid.at(entrypoint.row - 1).at(entrypoint.col - 2) = 1;
            this->grid.at(entrypoint.row - 2).at(entrypoint.col - 1) = 1;

            // right hand side
            this->grid.at(entrypoint.row - 1).at(entrypoint.col + 1) = 1;
            this->grid.at(entrypoint.row - 1).at(entrypoint.col + 2) = 1;
            this->grid.at(entrypoint.row - 2).at(entrypoint.col + 1) = 1;
            // bottom center
            this->grid.at(entrypoint.row - 3).at(entrypoint.col) = 1;
          };
          break;
      };
    };

  private:
    Grid grid;
    std::vector<EntryPoint> entrypoints;

    Grid initGrid() {
      // allocating a number of ELEMENTS, not index count.
      std::vector cols = std::vector(GRID_COLS, 0);
      return std::vector(GRID_ROWS, cols);
    };

    void logEntrypoints() {
      std::cout << "List of Seeded Entrypoints: " << std::endl;
      for (int i = 0; i < this->entrypoints.size() - 1; ++i) {
        std::cout << '\t' << i + 1 << ". "
                  << "(ROW: " << this->entrypoints.at(i).row << ", COL: " << this->entrypoints.at(i).col << ")" << std::endl;
      };
    };

    bool isExtinction() {

      bool isExtinct = true;

      for (int row = 0; row < GRID_ROWS; ++row) {
        for (int col = 0; col < GRID_COLS; ++col) {

          if (this->grid.at(row).at(col) == 1) {
            isExtinct = false;
          };
        };
      };

      return isExtinct;
    };

    void isAlive(int cell, int &liveCount) {
      if (cell == 1) {
        ++liveCount;
      };
    };

    int getNumOfNeighbors(int row, int col) {
      // In all honesty this is a series of checks which are written in a very ass manner.
      
      int liveNeighbors = 0;
      bool validTop = false;
      bool validBottom = false;

      // edge checking.

      // top check
      
      if (row > 0) {
        int top = this->grid.at(row - 1).at(col);
        this->isAlive(top, liveNeighbors);
        validTop = true;
      };

      // bottom check

      if (row < GRID_ROWS - 1) {
        int bottom = this->grid.at(row + 1).at(col);
        this->isAlive(bottom, liveNeighbors);
        validBottom = true;
      };

      // left checks
      
      if (col > 0) {
        int left = this->grid.at(row).at(col - 1);
        this->isAlive(left, liveNeighbors);

        if (validTop) {
          int topLeft = this->grid.at(row - 1).at(col - 1);
          this->isAlive(topLeft, liveNeighbors);
        };

        if (validBottom) {
          int bottomLeft = this->grid.at(row + 1).at(col - 1);
          this->isAlive(bottomLeft, liveNeighbors);
        };
      };

      // right checks

      if (col < GRID_COLS - 1) {
        int right = this->grid.at(row).at(col + 1);
        this->isAlive(right, liveNeighbors);

        if (validTop) {
          int topRight = this->grid.at(row - 1).at(col + 1);
          this->isAlive(topRight, liveNeighbors);
        };

        if (validBottom) {
          int bottomRight = this->grid.at(row + 1).at(col + 1);
          this->isAlive(bottomRight, liveNeighbors);
        };
      };

      return liveNeighbors;
    };

    EntryPoint seed() {
      int randomRow = generateRandomNumber(1, GRID_ROWS - 1);
      int randomCol = generateRandomNumber(1, GRID_COLS - 1);
      EntryPoint entrypoint = {randomRow, randomCol};
      this->entrypoints.push_back(entrypoint);
      return entrypoint;
    };

    void printGrid() {
      for (int row = 0; row < GRID_ROWS; ++row) {
        for (int col = 0; col < GRID_COLS; ++col) {
          const int cell = this->grid.at(row).at(col);
          if (cell == 1) {
            std::cout << "@";
          } else {
            std::cout << ".";
          };
        };
        // once done printing row, output newline
        std::cout << std::endl;
      };
    };
};

int main() {
  GameOfLife life;
  // create 30, remember some may generate some may not. It must fufill the drawing conditions.
  for (int i = 0; i < 30; ++i) {
    life.createPattern(Pattern::SPACESHIP_GLIDER);
    life.createPattern(Pattern::OSCILLATOR_ANGEL_BLINKER);
    life.createPattern(Pattern::STILL_BLOCK);
  };

  life.execute();

  return 0;
}
