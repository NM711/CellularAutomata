#ifndef UTIL_WINDOW_HPP
#define UTIL_WINDOW_HPP

#include <iostream>
#include <sys/ioctl.h>

#define RED "\033[0;31m"
#define DARK_RED "\033[38;2;82;11;20m"
#define DARK_GREEN "\033[38;2;35;89;18m"
#define GREEN "\033[0;32m"
#define BLUE  "\033[0;34m"
#define BROWN "\033[0;33m"
#define GRAY "\033[0;90m"
#define LIGHT_YELLOW "\033[38;2;237;227;111m"
#define DEFAULT "\033[0m"

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
