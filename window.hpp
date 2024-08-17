#ifndef UTIL_WINDOW_HPP
#define UTIL_WINDOW_HPP

#include <iostream>
#include <sys/ioctl.h>

#define RED "\e[0;31m"
#define GREEN "\e[0;32m"
#define BLUE  "\e[0;34m"
#define BROWN "\033[0;33m"
#define GRAY "\033[0;90m"
#define DEFAULT "\e[0m"

class WindowUtil {
  public:
    static winsize *getTermWindowInfo() {
      winsize *w = new winsize();
      ioctl(0, TIOCGWINSZ, w);

      if (ioctl(0, TIOCGWINSZ, w) == -1 || w == nullptr) {
        std::cout << "Error getting the terminal size!" << std::endl;
        exit(-1);
      };
      return w;
    };
};

#endif
