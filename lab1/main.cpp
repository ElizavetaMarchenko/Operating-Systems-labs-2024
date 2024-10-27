#include "daemon.hpp"
#include <iostream>
using namespace std;

int main(int argc, char** argv) 
{
    if (argc < 2) 
    {
        cerr << "Incorrect arguments, it requires two" << endl;
        return EXIT_FAILURE;
    }
    My_Daemon::get_one().initialize(argv[1]);
    My_Daemon::get_one().run();

    return EXIT_SUCCESS;
}