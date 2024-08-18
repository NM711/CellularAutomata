#include "../grid.hpp"
#include "../window.hpp"
#include <chrono>
#include <thread>

// Simple terrain generator. I plan on making it a tool I can use in some side personal games. Still a work in progress
// though. Multistate

// TODO: Start refactoring some code, and begin building api.

/*
  Rules:

  VEGETATION:

    1. If current cell is dirt and neighbors are more than 4 dirt and one 1 water cells, we create a grass cell -
  NEW VEGETATION
    2. If current cell is dirt and neighbors are equal to 3 OR 4 grass cells, we create a grass cell - WATER
  TRANSPORTATION VIA ROOTS
    3. If a dirt or grass cell is encompassed by more than 4 neighboring water cells, convert the cell into a
  water cell. - Over Irrigation
    4. If current cell is dirt OR grass and water neighbors are equal to 1, there
  will be a 1% percent chance that the given cell turns into water and a 99% chance that it will remain the same.
  - RANDOM EROSION (THE DEGRADATION OF THE CELL)
    5. If a current cell is water and its neighbors are less than 6 waters cell and a combination of a total of
  more than 6 dirt OR grass cells are around it, there will be a 5% chance that the cell turns into dirt and a 95%
  chance nothing occurs - Evaporation

  ENVIRONMENT:

    6. If current cell is dirt and there is 1 or 2 neighbor stone cells, there is a 3%
  chance that we will create a new stone, and a 97% chance that everything stays the same

    7. If a current cell is grass and its neighbors are more than 4 grass cells, there will be a 10%
  chance we generate a tree, a 10% chance we generate a bush, a 10% chance we generate a rose, lastly a 70% chance
  nothing occurs. - Plant Growth

  8. If a current cell is stone, and its neighbors are more than 6 stone cells, there will be a 20% chance we generate a
  crystal. - Crystal Growth

*/

enum State {
  DIRT,
  GRASS,
  WATER,
  STONE,
  PLANT_TREE,
  PLANT_BUSH,
  PLANT_FLOWER,
  CRYSTAL
};

struct NeighborCount {
    unsigned int grass;
    unsigned int dirt;
    unsigned int water;
    unsigned int stone;
    unsigned int plant;
    unsigned int crystal;

    NeighborCount() {
      this->grass = 0;
      this->dirt = 0;
      this->water = 0;
      this->stone = 0;
      this->plant = 0;
      this->crystal = 0;
    };
};

/*
  Abstract class that can be used for the user of the library to define a new process method that is able process the
  received grid, for the given period of iterations.
*/

struct AbstractGridProcessor {
    virtual void process(Grid &grid, CellStateMap &map) const = 0;
};

// Example of what you can do, but given the grid and states people will be able to render their own visuals, given a
// overriden version of this.

struct ProcessASCIIGrid : AbstractGridProcessor {
    virtual void process(Grid &grid, CellStateMap &map) const override {
      system("clear");
      GridPrinterUtil::PrintGrid(grid, map);
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
    };
};

class TerrainGenerator {
  public:
    void setGridSize(int rows, int cols) {
      this->gridRows = rows;
      this->gridCols = cols;
    };

    // Use numbers for representation of ores and crystals, the bigger the number the rarer. At least thats the idea.

    void execute(AbstractGridProcessor *processor = nullptr) {
      this->grid = GridUtil::InitGrid(this->gridRows, this->gridCols);

      CellStateMap stateMap = {
        {State::DIRT, BROWN + std::string("&") + DEFAULT},
        {State::GRASS, DARK_GREEN + std::string("#") + DEFAULT},
        {State::WATER, BLUE + std::string("%") + DEFAULT},
        {State::STONE, GRAY + std::string("0") + DEFAULT},
        {State::PLANT_BUSH, GREEN + std::string("\"") + DEFAULT},
        {State::PLANT_TREE, GREEN + std::string("¥") + DEFAULT},
        {State::PLANT_FLOWER, DARK_RED + std::string("@") + DEFAULT},
        {State::CRYSTAL, LIGHT_YELLOW + std::string("1") + DEFAULT}
      };

      this->seed();

      for (int i = 0; i <= 500; ++i) {

        if (processor != nullptr) {
          processor->process(this->grid, stateMap);
        };

        if (i < 499) {
          this->generateBaseTerrain();
        } else {
          // Do one step environment growth once the terrain has been generated.
          this->generateTerrainEnvironment();
        };
      };
    };

  private:
    Grid grid;
    int gridRows;
    int gridCols;

    // Seeds the terrain with initial state cells.

    void seed() {
      // Kinda hacky way to keep track of a seed for later use, but we could append a string of entrypoints in the form of
      // "<row><col>" together per entry point.

      // Given entry row = 1 col = 5 and row = 9 and col = 3 we would create "1593";

      for (int i = 0; i <= 10; ++i) {
        Entrypoint entrypoint = GridUtil::GenerateGridEntrypoint(this->gridRows, this->gridCols);
        if (entrypoint.col > 1 && entrypoint.col < this->gridCols - 1 && entrypoint.row > 1 &&
            entrypoint.row < this->gridRows - 1) {
          if (i < 6) {
            this->grid.at(entrypoint.row).at(entrypoint.col) = State::WATER;
            this->grid.at(entrypoint.row).at(entrypoint.col - 1) = State::WATER;
            this->grid.at(entrypoint.row).at(entrypoint.col + 1) = State::WATER;
            this->grid.at(entrypoint.row - 1).at(entrypoint.col - 1) = State::WATER;
            this->grid.at(entrypoint.row - 1).at(entrypoint.col + 1) = State::WATER;
            this->grid.at(entrypoint.row + 1).at(entrypoint.col - 1) = State::WATER;
            this->grid.at(entrypoint.row + 1).at(entrypoint.col + 1) = State::WATER;
          } else {
            this->grid.at(entrypoint.row).at(entrypoint.col) = State::STONE;
            this->grid.at(entrypoint.row).at(entrypoint.col + 1) = State::STONE;
          };
        };
      };
    };
    /*
      Generates trees and other things in the environment
    */

    void generateTerrainEnvironment() {
      Grid newGrid = GridUtil::InitGrid(this->gridRows, this->gridCols);
      for (int row = 0; row < this->gridRows; ++row) {
        for (int col = 0; col < this->gridCols; ++col) {
          NeighborCount neighbors = this->calculateNeighbors(row, col);
          int currentCell = this->grid.at(row).at(col);
          int *newestCell = &newGrid.at(row).at(col);

          if (currentCell == State::GRASS && neighbors.grass > 4) {
            int outcome = GridUtil::GenerateRandomNumber(0, 100);

            if (outcome < 10) {
              *newestCell = State::PLANT_TREE;
            } else if (outcome > 10 && outcome < 20) {
              *newestCell = State::PLANT_BUSH;
            } else if (outcome > 20 && outcome < 30) {
              *newestCell = State::PLANT_FLOWER;
            } else {
              *newestCell = currentCell;
            };

          } else if (currentCell == State::STONE && neighbors.stone > 6 && GridUtil::GenerateRandomNumber(0, 100) < 20) {
            *newestCell = State::CRYSTAL;
          } else {
            *newestCell = currentCell;
          }
        };
      };

      GridUtil::UpdateGrid(this->grid, newGrid);
    };

    /*
      Generates the base terrain of our map.
    */

    void generateBaseTerrain() {
      Grid newGrid = GridUtil::InitGrid(this->gridRows, this->gridCols);
      for (int row = 0; row < this->gridRows; ++row) {
        for (int col = 0; col < this->gridCols; ++col) {
          NeighborCount neighbors = this->calculateNeighbors(row, col);
          int currentCell = this->grid.at(row).at(col);
          int *newestCell = &newGrid.at(row).at(col);

          if (currentCell == State::DIRT && (neighbors.water == 1 && neighbors.dirt > 4)) {
            *newestCell = State::GRASS;
          } else if (currentCell == State::DIRT && (neighbors.grass == 3 || neighbors.grass == 4)) {
            *newestCell = State::GRASS;
          } else if ((currentCell == State::DIRT || currentCell == State::GRASS) && neighbors.water > 4) {
            *newestCell = State::WATER;
          } else if ((currentCell == State::DIRT || currentCell == State::GRASS) && neighbors.water == 1 &&
                     GridUtil::GenerateRandomNumber(0, 100) < 1) {
            *newestCell = State::WATER;
          } else if (currentCell == State::WATER && neighbors.water < 6 && neighbors.dirt + neighbors.grass > 6 &&
                     GridUtil::GenerateRandomNumber(0, 100) < 5) {
            *newestCell = State::DIRT;
          } else if (currentCell == State::DIRT && neighbors.stone >= 1 && GridUtil::GenerateRandomNumber(0, 100) < 3) {
            *newestCell = State::STONE;
          } else {
            *newestCell = currentCell;
          };
        };
      };

      GridUtil::UpdateGrid(this->grid, newGrid);
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

        case State::PLANT_BUSH:
        case State::PLANT_FLOWER:
        case State::PLANT_TREE:
          ++neighborCount.plant;
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

      if (row < this->gridRows - 1) {
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

      if (col < this->gridCols - 1) {
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
  generator.setGridSize(WindowUtil::getTermWindowInfo()->ws_row, WindowUtil::getTermWindowInfo()->ws_col);
  ProcessASCIIGrid asciiProc;
  ProcessASCIIGrid *proc = &asciiProc;

  generator.execute(proc);
  return 0;
};
