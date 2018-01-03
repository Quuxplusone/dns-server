
#include "application.h"
#include "exception.h"

#include <iostream>

int main(int argc, char **argv)
{
    try {
        dns::Application application;
        application.parse_arguments(argc, argv);
        application.run();
    } catch (const dns::Exception& e) {
        std::cout << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}
