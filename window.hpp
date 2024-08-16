#ifndef UTIL_WINDOW_HPP
#define UTIL_WINDOW_HPP

#include <cstdio>
#include <cstdlib>
#include <sys/ioctl.h>

class WindowUtil {
  public:
    static winsize *getTermWindowInfo() {
      winsize *w = new winsize();
      ioctl(0, TIOCGWINSZ, w);

      if (ioctl(0, TIOCGWINSZ, w) == -1 || w == nullptr) {
        perror("Error getting terminal size");
        exit(-1);
      };
      return w;
    };
};

#endif
