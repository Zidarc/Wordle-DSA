#ifndef WORDLE_DSA_BOARD_H
#define WORDLE_DSA_BOARD_H

// The game grid: a dynamic 2D array with one row per attempt and one column
// per letter (8 / 6 / 5 rows by difficulty, 5 columns).
//
//   letters_[row][col]  the guessed letter ('\0' while the row is empty)
//   states_[row][col]   its colour
//
// Both grids are allocated with new[] as an array of row pointers, so the
// class follows the Rule of Three.

#include "LetterState.h"

#include <string>

using namespace std;

class Board
{
public:
    // Throws an error if rows or cols is not positive.
    explicit Board(int rows, int cols = 5);

    Board(const Board& other);
    Board& operator=(const Board& other);
    ~Board();

    // Writes a guess and its feedback into the next empty row.
    // Throws an error if the board is full,
    // throws an error if the guess length does not match cols().
    void addRow(const string& guess, const LetterState* feedback);

    // Throw an error for a row or column outside the grid.
    char letterAt(int row, int col) const;
    LetterState stateAt(int row, int col) const;
    string rowWord(int row) const; // empty string for a row not yet filled

    int rows() const { return rows_; }
    int cols() const { return cols_; }
    int filledRows() const { return filledRows_; }
    bool isFull() const { return filledRows_ == rows_; }

private:
    int rows_;
    int cols_;
    int filledRows_;
    char** letters_;
    LetterState** states_;

    void allocate();
    void release();
    void copyCellsFrom(const Board& other);
    void checkCell(int row, int col) const;
};

#endif // WORDLE_DSA_BOARD_H
