// Entry point for the Wordle-DSA backend.
//
// main.cpp is kept small on purpose: it only starts the server.

#include "Server.h"

#include <iostream>

using namespace std;

// GUIDE 3.1: startup - read Server::Server next
int main()
{
    cout << "========================================\n";
    cout << "       Wordle-DSA C++ REST Server       \n";
    cout << "========================================\n";

    Server server("data/words.txt", "data/scores.txt");
    cout << "Server starting on http://localhost:8080 ...\n";
    server.listen("0.0.0.0", 8080);

    return 0;
}
