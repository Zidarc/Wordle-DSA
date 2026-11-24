#include "Board.h"

#include <stdexcept>

Board::Board(int rows, int cols)
    : rows_(rows), cols_(cols), filledRows_(0), letters_(nullptr), states_(nullptr)
{
    if (rows <= 0 || cols <= 0)
        throw std::invalid_argument("Board: rows and cols must be positive");
    allocate();
}

Board::Board(const Board& other)
    : rows_(other.rows_), cols_(other.cols_), filledRows_(other.filledRows_), letters_(nullptr),
      states_(nullptr)
{
    allocate();
    copyCellsFrom(other);
}

Board& Board::operator=(const Board& other)
{
    if (this == &other)
        return *this;

    release();
    rows_ = other.rows_;
    cols_ = other.cols_;
    filledRows_ = other.filledRows_;
    allocate();
    copyCellsFrom(other);
    return *this;
}

Board::~Board()
{
    release();
}

// ---------------------------------------------------------------------------
// Rows
// ---------------------------------------------------------------------------

void Board::addRow(const std::string& guess, const LetterState* feedback)
{
    if (isFull())
        throw std::logic_error("Board::addRow: the board is full");
    if (static_cast<int>(guess.size()) != cols_)
        throw std::invalid_argument("Board::addRow: guess length does not match the board width");
    if (feedback == nullptr)
        throw std::invalid_argument("Board::addRow: feedback is null");

    for (int col = 0; col < cols_; ++col)
    {
        letters_[filledRows_][col] = guess[col];
        states_[filledRows_][col] = feedback[col];
    }
    ++filledRows_;
}

char Board::letterAt(int row, int col) const
{
    checkCell(row, col);
    return letters_[row][col];
}

LetterState Board::stateAt(int row, int col) const
{
    checkCell(row, col);
    return states_[row][col];
}

std::string Board::rowWord(int row) const
{
    checkCell(row, 0);
    if (row >= filledRows_)
        return "";
    return std::string(letters_[row], static_cast<std::size_t>(cols_));
}

// ---------------------------------------------------------------------------
// Memory
// ---------------------------------------------------------------------------

// GUIDE 1.11: array of row pointers = dynamic 2D array
void Board::allocate()
{
    letters_ = new char*[rows_];
    states_ = new LetterState*[rows_];
    for (int row = 0; row < rows_; ++row)
    {
        letters_[row] = new char[cols_];
        states_[row] = new LetterState[cols_];
        for (int col = 0; col < cols_; ++col)
        {
            letters_[row][col] = '\0';
            states_[row][col] = LetterState::Unused;
        }
    }
}

void Board::release()
{
    if (letters_ != nullptr)
    {
        for (int row = 0; row < rows_; ++row)
            delete[] letters_[row];
        delete[] letters_;
        letters_ = nullptr;
    }
    if (states_ != nullptr)
    {
        for (int row = 0; row < rows_; ++row)
            delete[] states_[row];
        delete[] states_;
        states_ = nullptr;
    }
}

void Board::copyCellsFrom(const Board& other)
{
    for (int row = 0; row < rows_; ++row)
    {
        for (int col = 0; col < cols_; ++col)
        {
            letters_[row][col] = other.letters_[row][col];
            states_[row][col] = other.states_[row][col];
        }
    }
}

void Board::checkCell(int row, int col) const
{
    if (row < 0 || row >= rows_ || col < 0 || col >= cols_)
        throw std::out_of_range("Board: cell (" + std::to_string(row) + ", " +
                                std::to_string(col) + ") is outside the grid");
}
