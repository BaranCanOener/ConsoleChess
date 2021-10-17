# ConsoleChess
A chess game on the command prompt, including an engine to play against or to play itself.

This initially served both as a benchmark for the Python Chess script (https://github.com/BaranCanOener/PythonChess), as well as a little project to learn more about C++. As such, the general concept of the engine is largely the same, comprising of an iterative deepening search supported by alpha beta pruning and quiescence searches; the valuation function has been tweaked to reward castlings, and the valuation tables have been updated, along with some other minor improvements.

The difference in performance are quite significant, with PythonChess running at approx. 30kN /sec and ConsoleChess averaging 2.000 kN/sec on an i5-9300H. That being said, I can't rule out that the Python-version can be written more efficiently [while keeping the overall (object-oriented, rather than bitboard) approach the same].

Update Oct/2021: I implemented Zobrist key based hashtables earlier this year and the current commit contains this implementation - see the Hashtable class in board.h.

![Console Chess](https://github.com/BaranCanOener/ConsoleChess/blob/master/ConsoleChessv2.PNG)
