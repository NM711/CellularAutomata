#include "../grid.hpp"
#include "../window.hpp"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <vector>

enum Pattern {
  OSCILLATOR_ANGEL_BLINKER,
  STILL_BLOCK,
  SPACESHIP_GLIDER
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
      this->grid = GridUtil::InitGrid(GRID_ROWS, GRID_COLS);
    };

    void execute() {
      int generation = 0;
      CellStateMap stateMap = {
        {0, "."},
        {1, "@"}
      };

      while (true) {
        system("clear");
        std::cout << "Generation: " << generation << std::endl;
        std::cout << std::endl;
        GridPrinterUtil::PrintGrid(this->grid, stateMap);
        // render updated grid then check for extinction to end game.
        if (this->isExtinction()) {
          break;
        };

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        Grid newGeneration = GridUtil::InitGrid(GRID_ROWS, GRID_COLS);

        for (int row = 0; row < this->GRID_ROWS; ++row) {
          for (int col = 0; col < this->GRID_COLS; ++col) {

            int liveNeighbors = this->getNumOfNeighbors(row, col);
            int currentGenerationCell = this->grid.at(row).at(col);
            int *newGenerationCell = &newGeneration.at(row).at(col);

            if (currentGenerationCell == 1 && liveNeighbors < 2) {
              *newGenerationCell = 0;
            } else if (currentGenerationCell == 1 && (liveNeighbors == 2 || liveNeighbors == 3)) {
              *newGenerationCell = currentGenerationCell;
            } else if (currentGenerationCell == 1 && liveNeighbors > 3) {
              *newGenerationCell = 0;
            } else if (currentGenerationCell == 0 && liveNeighbors == 3) {
              *newGenerationCell = 1;
            } else {
              *newGenerationCell = currentGenerationCell;
            };
          };
        };

        GridUtil::UpdateGrid(this->grid, newGeneration);
        ++generation;
      };

      // If a game reaches a total en just print the entrypoints and the game over.
      // Might just honestly redirect the entrypoint output to a text file. Could be useful for debugging, possibly.

      GridPrinterUtil::PrintEntrypoints(this->entrypoints);
      std::cout << "Game Over!" << std::endl;
    };

    /*
      Given a random entry point cell you can create a premade pattern on the grid.
      If I want to implement custom patterns, I would need to implement the feature later on.
    */

    void createPattern(Pattern pattern) {
      Entrypoint entrypoint = GridUtil::GenerateGridEntrypoint(GRID_ROWS, GRID_COLS);
      this->entrypoints.push_back(entrypoint);

      this->grid.at(entrypoint.row).at(entrypoint.col) = 1;

      // This could all be much better, but frick it xD

      switch (pattern) {
        case Pattern::STILL_BLOCK:
          if (entrypoint.row < this->GRID_ROWS - 1 && entrypoint.col < this->GRID_COLS - 1) {
            this->grid.at(entrypoint.row + 1).at(entrypoint.col) = 1;
            this->grid.at(entrypoint.row).at(entrypoint.col + 1) = 1;
          };
          break;

        case Pattern::SPACESHIP_GLIDER:
          if (entrypoint.row < this->GRID_ROWS - 1 && entrypoint.col < this->GRID_COLS - 2) {
            this->grid.at(entrypoint.row + 1).at(entrypoint.col + 2) = 1;
            this->grid.at(entrypoint.row + 1).at(entrypoint.col + 1) = 1;
            // move two columns to the side, so we can make the gliders peak
            this->grid.at(entrypoint.row).at(entrypoint.col + 2) = 1;
            this->grid.at(entrypoint.row - 1).at(entrypoint.col + 2) = 1;
          };
          break;

        case Pattern::OSCILLATOR_ANGEL_BLINKER:
          std::cout << "OSCI" << std::endl;
          if ((entrypoint.row > 3 && entrypoint.row < this->GRID_ROWS - 4) &&
              (entrypoint.col < this->GRID_COLS - 2 && entrypoint.col > 1)) {
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
    std::vector<Entrypoint> entrypoints;
    const int GRID_ROWS = 32;
    const int GRID_COLS = WindowUtil::getTermWindowInfo()->ws_col;
    
    bool isExtinction() {

      bool isExtinct = true;

      for (int row = 0; row < this->GRID_ROWS; ++row) {
        for (int col = 0; col < this->GRID_COLS; ++col) {
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

      if (row < this->GRID_ROWS - 1) {
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

      if (col < this->GRID_COLS - 1) {
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
