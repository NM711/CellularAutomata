#include "../grid.hpp"
#include "../window.hpp"
#include <chrono>
#include <thread>

// Simple terrain generator. I plan on making it a tool I can use in some side personal games. Still a work in progress though.
// Multistate

enum State {
  DIRT,
  GRASS,
  WATER,
  STONE,
  TREE,
  BUSH,
};

struct NeighborCount {
    unsigned int grass;
    unsigned int dirt;
    unsigned int water;
    unsigned int stone;

    NeighborCount() {
      this->grass = 0;
      this->dirt = 0;
      this->water = 0;
      this->stone = 0;
    };
};

class TerrainGenerator {
  public:
    TerrainGenerator() {
      this->grid = GridUtil::InitGrid(this->GRID_ROWS, this->GRID_COLS);
    };

    void execute() {
      CellStateMap stateMap = {
        {State::DIRT, BROWN + std::string("&") + DEFAULT},
        {State::GRASS, GREEN + std::string("#") + DEFAULT},
        {State::WATER, BLUE + std::string("%") + DEFAULT},
        {State::STONE, GRAY + std::string("0") + DEFAULT}
      };

      this->seed();

      for (int i = 0; i < 80; ++i) {
        system("clear");
        GridPrinterUtil::PrintGrid(this->grid, stateMap);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        this->generateBaseTerrain();
      };
    };

  private:
    Grid grid;
    // Init the constants inline
    const int GRID_ROWS = 32;
    const int GRID_COLS = WindowUtil::getTermWindowInfo()->ws_col;

    // Seeds the terrain with water cells.

    void seed() {
      for (int i = 0; i < 12; ++i) {
        Entrypoint entrypoint = GridUtil::GenerateGridEntrypoint(this->GRID_ROWS, this->GRID_COLS);
        this->grid.at(entrypoint.row).at(entrypoint.col) = State::WATER;
      };

      for (int i = 0; i < 5; ++i) {
        Entrypoint entrypoint = GridUtil::GenerateGridEntrypoint(this->GRID_ROWS, this->GRID_COLS);
        this->grid.at(entrypoint.row).at(entrypoint.col) = State::STONE;
        this->grid.at(entrypoint.row).at(entrypoint.col + 1) = State::STONE;
      };
    };

    /*
      Generates trees and other things in the environment
    */

    void generateTerrainEnvironment() {
      Grid newGrid = GridUtil::InitGrid(this->GRID_ROWS, this->GRID_COLS);
      for (int row = 0; row < this->GRID_ROWS; ++row) {
        for (int col = 0; col < this->GRID_COLS; ++col) {};
      };
    };

    /*
      Generates the base terrain of our map.
    */

    void generateBaseTerrain() {
      Grid newGrid = GridUtil::InitGrid(this->GRID_ROWS, this->GRID_COLS);
      for (int row = 0; row < this->GRID_ROWS; ++row) {
        for (int col = 0; col < this->GRID_COLS; ++col) {
          NeighborCount neighbors = this->calculateNeighbors(row, col);
          int currentCell = this->grid.at(row).at(col);
          int *newGridCell = &newGrid.at(row).at(col);

          /*
            I will attempt to make the rules as logical as I can, in the sense of how plants move water and grow.

            Rules:

            VEGETATION:
            
              1. If current cell is dirt and neighbors are more than 4 dirt and one 1 water cells, we create a grass cell -
            NEW VEGETATION
              2. If current cell is dirt and neighbors are equal to 3 OR 4 grass cells, we create a grass cell - WATER
            TRANSPORTATION VIA ROOTS
              3. If a dirt or grass cell is encompassed by more than 4 neighboring water cells, convert the cell into a
            water cell. - Over Irrigation
              4. If current cell is dirt OR grass and water neighbors are equal to 1, there
            will be a 5% percent chance that the given cell turns into water and a 95% chance that it will remain the same.
            - RANDOM EROSION (THE DEGREDATION OF THE CELL)

            OTHER:

              5. If current cell is dirt and there is 1 or 2 neighbor stone cells, there is a 3%
            chance that we will create a new stone, and a 97% chance that everything stays the same

            ENVIRONMENT:
          */

          if (currentCell == State::DIRT && (neighbors.water == 1 && neighbors.dirt > 4)) {
            *newGridCell = State::GRASS;
          } else if (currentCell == State::DIRT && (neighbors.grass == 3 || neighbors.grass == 4)) {
            *newGridCell = State::GRASS;
          } else if ((currentCell == State::DIRT || currentCell == State::GRASS) && neighbors.water > 4) {
            *newGridCell = State::WATER;
          } else if ((currentCell == State::DIRT || currentCell == State::GRASS) && neighbors.water == 1 &&
                     GridUtil::GenerateRandomNumber(0, 100) < 5) {
            *newGridCell = State::WATER;
          } else if (currentCell == State::DIRT && neighbors.stone >= 1 &&
                     GridUtil::GenerateRandomNumber(0, 100) < 3) {
            *newGridCell = State::STONE;
          } else {
            *newGridCell = currentCell;
          };
        };
      };

      GridUtil::RepopulateGrid(this->grid, newGrid);
    };

    /*
      Given a cell state this method with increase the neighbor count for a given state.
    */

    void updateNeighbor(int cell, NeighborCount &neighborCount) {
      switch (cell) {
        case State::DIRT:
          ++neighborCount.dirt;
          break;
        case State::GRASS:
          ++neighborCount.grass;
          break;

        case State::WATER:
          ++neighborCount.water;
          break;

        case State::STONE:
          ++neighborCount.stone;
          break;
      };
    };

    NeighborCount calculateNeighbors(int row, int col) {
      NeighborCount neighborCount;
      // Edge checks
      bool validTop = false;
      bool validBottom = false;

      // TOP

      if (row > 0) {
        int top = this->grid.at(row - 1).at(col);
        this->updateNeighbor(top, neighborCount);
        validTop = true;
      };

      // BOTTOM

      if (row < this->GRID_ROWS - 1) {
        int bottom = this->grid.at(row + 1).at(col);
        this->updateNeighbor(bottom, neighborCount);
        validBottom = true;
      };

      // LEFT

      if (col > 0) {
        int left = this->grid.at(row).at(col - 1);
        this->updateNeighbor(left, neighborCount);

        if (validTop) {
          int topLeft = this->grid.at(row - 1).at(col - 1);
          this->updateNeighbor(topLeft, neighborCount);
        };

        if (validBottom) {
          int bottomLeft = this->grid.at(row + 1).at(col - 1);
          this->updateNeighbor(bottomLeft, neighborCount);
        };
      };

      if (col < this->GRID_COLS - 1) {
        int right = this->grid.at(row).at(col + 1);
        this->updateNeighbor(right, neighborCount);

        if (validTop) {
          int topRight = this->grid.at(row - 1).at(col + 1);
          this->updateNeighbor(topRight, neighborCount);
        };

        if (validBottom) {
          int bottomRight = this->grid.at(row + 1).at(col + 1);
          this->updateNeighbor(bottomRight, neighborCount);
        };
      };

      return neighborCount;
    };
};

int main() {
  TerrainGenerator generator;
  generator.execute();
  return 0;
};
