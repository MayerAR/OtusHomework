
/*! \mainpage Name or Title or the Software or the Project.
 *
 * \section intro_sec Introduction(or any thing)
 *
 * bla bla bla
 * more and more bla
 *
 * \section install_sec How to Install(or any thing)
 *
  * \subsection creating_sub_sec Compiling(or any thing)
  *
  * bla bla and bla
  * bla and bla bla
  *
  *
  */

#include "lib.h"

#include <iostream>

int main (int, char **) {
    std::cout << "Version: " << version() << std::endl;
    std::cout << "Hello, world!" << std::endl;
    std::cin.get();
    return 0;
}
