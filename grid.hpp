#ifndef UTIL_GRID_HPP
#define UTIL_GRID_HPP

#include <iostream>
#include <random>
#include <unordered_map>
#include <vector>

typedef std::vector<std::vector<int>> Grid;

struct Entrypoint {
    int row;
    int col;
};

typedef std::unordered_map<unsigned int, std::string> CellStateMap;

class GridPrinterUtil {
  public:
    static void PrintEntrypoints(std::vector<Entrypoint> &entrypoints) {
      std::cout << "List of Generated Entrypoints: " << std::endl;
      for (int i = 0; i < entrypoints.size() - 1; ++i) {
        std::cout << '\t' << i + 1 << ". "
                  << "(ROW: " << entrypoints.at(i).row << ", COL: " << entrypoints.at(i).col << ")" << std::endl;
      };
    };

    /*
      Prints the given grid to the screen.
      Map of states and text representations must be provided.
      If state does not exist the program will exit.
    */

    static void PrintGrid(Grid &grid, CellStateMap &map) {
      for (int row = 0; row < grid.size(); ++row) {
        for (int col = 0; col < grid.at(row).size(); ++col) {
          int cell = grid.at(row).at(col);
          auto iterator = map.find(cell);
          if (iterator == map.end()) {
            std::cout << "Given cell state of \"" << cell << "\" does not exist in the provided map!" << std::endl;
            exit(1);
          };
          std::cout << map[cell];
        };
      };
      std::cout << std::endl;
    };
};

class GridUtil {
  public:
    /*
      Initializes a 2D Vector.
      Column Vectors will get populated with 0s, while Row Vectors will get populate with the Columns Vectors.
      Population value will default to 0 if none is provided.
    */

    static Grid InitGrid(int rowSize, int colSize, unsigned int popValue = 0) {
      std::vector<int> cols(colSize, popValue);
      Grid rows(rowSize, cols);
      return rows;
    };

    /*
      Generates a random grid entrypoint by computing a random row and column, starting from 1 to x - 1.
      Entrypoint can then be used to Seed the grid.
    */

    static Entrypoint GenerateGridEntrypoint(int rowSize, int colSize) {
      int randomRow = GridUtil::GenerateRandomNumber(1, rowSize - 1);
      int randomCol = GridUtil::GenerateRandomNumber(1, colSize - 1);
      Entrypoint entrypoint = {randomRow, randomCol};
      return entrypoint;
    };

    /*
      Populates original grid with the values of the most recent one.
    */

    static void RepopulateGrid(Grid &originalGrid, Grid &newGrid) {
      for (int row = 0; row < originalGrid.size(); ++row) {
        for (int col = 0; col < originalGrid.at(row).size(); ++col) {
          originalGrid.at(row).at(col) = newGrid.at(row).at(col);
        };
      };
    };
    
    /*
      Generates a random number starting from x (min) to y (max)
    */

    static int GenerateRandomNumber(int x, int y) {
      static std::mt19937 mt(std::random_device{}());
      std::uniform_int_distribution<int> distribution(x, y);
      return distribution(mt);
    };
};

#endif
