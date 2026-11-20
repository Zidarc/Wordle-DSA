// Entry point for the Wordle-DSA backend.
//
// main.cpp is kept small on purpose: it only starts the server.

#include "Server.h"

#include <iostream>

int main()
{
    std::cout << "========================================\n";
    std::cout << "       Wordle-DSA C++ REST Server       \n";
    std::cout << "========================================\n";

    Server server("data/words.txt", "data/scores.txt");
    std::cout << "Server starting on http://localhost:8080 ...\n";
    server.listen("0.0.0.0", 8080);

    return 0;
}
