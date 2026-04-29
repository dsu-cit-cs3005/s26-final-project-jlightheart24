#include <iostream>

#include "Arena.h"

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <config-file>\n";
        return 1;
    }

    Arena arena;

    if (!arena.load_config(argv[1]))
    {
        std::cerr << "Failed to load config file.\n";
        return 1;
    }

    if (!arena.initialize())
    {
        std::cerr << "Failed to initialize arena.\n";
        return 1;
    }

    arena.run();
    return 0;
}
