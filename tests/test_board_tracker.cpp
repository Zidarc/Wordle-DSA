#include "TestFramework.h"
#include "Board.h"
#include "LetterTracker.h"

#include <stdexcept>
#include <string>

static const LetterState G = LetterState::Correct;
static const LetterState Y = LetterState::Present;
static const LetterState X = LetterState::Absent;

// ===========================================================================
// Board
// ===========================================================================

TEST_CASE(boardSizeMatchesEachDifficulty)
{
    const int attempts[] = {8, 6, 5}; // Easy, Normal, Hard
    for (int rows : attempts)
    {
        Board board(rows);
        CHECK_EQ(board.rows(), rows);
        CHECK_EQ(board.cols(), 5);
        CHECK_EQ(board.filledRows(), 0);
        CHECK(!board.isFull());
    }
}

TEST_CASE(boardRejectsNonPositiveSize)
{
    CHECK_THROWS(Board(0));
    CHECK_THROWS(Board(-1));
    CHECK_THROWS(Board(6, 0));
}

TEST_CASE(boardNewCellsAreEmpty)
{
    Board board(6);
    CHECK_EQ(board.letterAt(0, 0), '\0');
    CHECK(board.stateAt(5, 4) == LetterState::Unused);
    CHECK_EQ(board.rowWord(0), std::string(""));
}

TEST_CASE(boardAddRowFillsTheNextRow)
{
    Board board(6);
    const LetterState first[] = {X, G, Y, X, G};
    const LetterState second[] = {G, G, G, G, G};
    board.addRow("CRANE", first);
    board.addRow("TRACE", second);

    CHECK_EQ(board.filledRows(), 2);
    CHECK_EQ(board.rowWord(0), std::string("CRANE"));
    CHECK_EQ(board.rowWord(1), std::string("TRACE"));
    CHECK_EQ(board.letterAt(0, 2), 'A');
    CHECK(board.stateAt(0, 2) == Y);
    CHECK(board.stateAt(0, 4) == G);
    CHECK(board.stateAt(1, 0) == G);
    CHECK_EQ(board.rowWord(2), std::string(""));
}

TEST_CASE(boardOutOfRangeCellThrows)
{
    Board board(5);
    CHECK_THROWS(board.letterAt(5, 0));
    CHECK_THROWS(board.letterAt(0, 5));
    CHECK_THROWS(board.stateAt(-1, 0));
    CHECK_THROWS(board.rowWord(9));
}

TEST_CASE(boardFullBoardRejectsAnotherRow)
{
    Board board(5);
    const LetterState fb[] = {X, X, X, X, X};
    for (int i = 0; i < 5; ++i)
        board.addRow("SLATE", fb);
    CHECK(board.isFull());
    CHECK_THROWS(board.addRow("SLATE", fb));
}

TEST_CASE(boardRejectsWrongLengthGuess)
{
    Board board(6);
    const LetterState fb[] = {X, X, X, X, X};
    CHECK_THROWS(board.addRow("CRAN", fb));
    CHECK_THROWS(board.addRow("CRANES", fb));
    CHECK_EQ(board.filledRows(), 0);
}

TEST_CASE(boardCopyIsDeep)
{
    Board original(6);
    const LetterState fb[] = {G, X, Y, X, G};
    original.addRow("CRANE", fb);

    Board copy(original);
    copy.addRow("SLATE", fb);
    CHECK_EQ(original.filledRows(), 1);
    CHECK_EQ(copy.filledRows(), 2);
    CHECK_EQ(copy.rowWord(0), std::string("CRANE"));

    Board assigned(8);
    assigned = original;
    CHECK_EQ(assigned.rows(), 6);
    CHECK_EQ(assigned.rowWord(0), std::string("CRANE"));
    assigned.addRow("GHOST", fb);
    CHECK_EQ(original.rowWord(1), std::string(""));

    assigned = assigned; // self-assignment is safe
    CHECK_EQ(assigned.rowWord(1), std::string("GHOST"));
}

// ===========================================================================
// LetterTracker
// ===========================================================================

TEST_CASE(trackerStartsWithEveryLetterUnused)
{
    LetterTracker tracker;
    for (char c = 'A'; c <= 'Z'; ++c)
        CHECK(tracker.statusOf(c) == LetterState::Unused);
}

TEST_CASE(trackerUpdateRecordsFeedback)
{
    LetterTracker tracker;
    const LetterState fb[] = {X, G, Y, X, G};
    tracker.update("CRANE", fb);
    CHECK(tracker.statusOf('C') == X);
    CHECK(tracker.statusOf('R') == G);
    CHECK(tracker.statusOf('a') == Y); // lower case works too
    CHECK(tracker.statusOf('E') == G);
    CHECK(tracker.statusOf('Z') == LetterState::Unused);
}

TEST_CASE(trackerStatusOnlyUpgrades)
{
    LetterTracker tracker;
    const LetterState absent[] = {X, X, X, X, X};
    const LetterState present[] = {Y, Y, Y, Y, Y};
    const LetterState correct[] = {G, G, G, G, G};

    tracker.update("AAAAA", absent);
    CHECK(tracker.statusOf('A') == X);
    tracker.update("AAAAA", present);
    CHECK(tracker.statusOf('A') == Y);
    tracker.update("AAAAA", correct);
    CHECK(tracker.statusOf('A') == G);

    // Never downgrades.
    tracker.update("AAAAA", present);
    CHECK(tracker.statusOf('A') == G);
    tracker.update("AAAAA", absent);
    CHECK(tracker.statusOf('A') == G);
}

TEST_CASE(trackerRepeatedLetterKeepsBestStatus)
{
    // Guess PUPPY vs APPLE: P is yellow, green, gray in the same guess.
    LetterTracker tracker;
    const LetterState fb[] = {Y, X, G, X, X};
    tracker.update("PUPPY", fb);
    CHECK(tracker.statusOf('P') == G);
}

TEST_CASE(trackerRejectsNonLetters)
{
    LetterTracker tracker;
    CHECK_THROWS(tracker.statusOf('1'));
    CHECK_THROWS(tracker.statusOf(' '));
    CHECK_THROWS(tracker.update("CRANE", nullptr));
}

TEST_CASE(trackerResetClearsEverything)
{
    LetterTracker tracker;
    const LetterState fb[] = {G, G, G, G, G};
    tracker.update("CRANE", fb);
    tracker.reset();
    CHECK(tracker.statusOf('C') == LetterState::Unused);
}

TEST_CASE(trackerKeyboardRowsAreJagged)
{
    LetterTracker tracker;
    CHECK_EQ(tracker.keyboardRowCount(), 3);
    CHECK_EQ(tracker.keyboardRowLength(0), 10);
    CHECK_EQ(tracker.keyboardRowLength(1), 9);
    CHECK_EQ(tracker.keyboardRowLength(2), 7);

    CHECK_EQ(tracker.keyAt(0, 0), 'Q');
    CHECK_EQ(tracker.keyAt(0, 9), 'P');
    CHECK_EQ(tracker.keyAt(1, 0), 'A');
    CHECK_EQ(tracker.keyAt(1, 8), 'L');
    CHECK_EQ(tracker.keyAt(2, 0), 'Z');
    CHECK_EQ(tracker.keyAt(2, 6), 'M');

    CHECK_THROWS(tracker.keyAt(2, 7)); // row 2 is shorter
    CHECK_THROWS(tracker.keyAt(3, 0));
    CHECK_THROWS(tracker.keyboardRowLength(-1));
}

TEST_CASE(trackerKeyboardCoversEveryLetterOnce)
{
    LetterTracker tracker;
    int seen[26] = {0};
    int total = 0;
    for (int row = 0; row < tracker.keyboardRowCount(); ++row)
    {
        for (int i = 0; i < tracker.keyboardRowLength(row); ++i)
        {
            ++seen[tracker.keyAt(row, i) - 'A'];
            ++total;
        }
    }
    CHECK_EQ(total, 26);
    for (int i = 0; i < 26; ++i)
        CHECK_EQ(seen[i], 1);
}

TEST_CASE(trackerKeyStatusFollowsLetterStatus)
{
    LetterTracker tracker;
    const LetterState fb[] = {X, G, Y, X, G};
    tracker.update("CRANE", fb);
    CHECK(tracker.keyStatusAt(0, 3) == G);  // R
    CHECK(tracker.keyStatusAt(1, 0) == Y);  // A
    CHECK(tracker.keyStatusAt(2, 2) == X);  // C
    CHECK(tracker.keyStatusAt(0, 0) == LetterState::Unused); // Q
}

TEST_CASE(trackerCopyIsDeep)
{
    LetterTracker original;
    const LetterState fb[] = {G, G, G, G, G};
    original.update("CRANE", fb);

    LetterTracker copy(original);
    CHECK(copy.statusOf('C') == G);
    copy.reset();
    CHECK(original.statusOf('C') == G);

    LetterTracker assigned;
    assigned = original;
    CHECK(assigned.statusOf('E') == G);
    CHECK_EQ(assigned.keyAt(2, 6), 'M');
    assigned = assigned;
    CHECK(assigned.statusOf('E') == G);
}

int main()
{
    return tf::runAll("Board and LetterTracker");
}
