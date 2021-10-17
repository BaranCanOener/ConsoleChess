#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <windows.h>
#include "pieces.h"
#include "board.h"
#include "engine.h"

#include <ctime>

const char CONV_ERROR = 127;

char numToLetter(char coord) {
	switch (coord)
	{
	case 0:
		return 'a';
		break;
	case 1:
		return 'b';
		break;
	case 2:
		return 'c';
		break;
	case 3:
		return 'd';
		break;
	case 4:
		return 'e';
		break;
	case 5:
		return 'f';
		break;
	case 6:
		return 'g';
		break;
	case 7:
		return 'h';
		break;
	default:
		return CONV_ERROR;
	}
}

char letterToNum(char coord) {
	switch (coord)
	{
	case 'a':
		return 0;
		break;
	case 'b':
		return 1;
		break;
	case 'c':
		return 2;
		break;
	case 'd':
		return 3;
		break;
	case 'e':
		return 4;
		break;
	case 'f':
		return 5;
		break;
	case 'g':
		return 6;
		break;
	case 'h':
		return 7;
		break;
	default:
		return CONV_ERROR;
	}
}

std::string numberToString(int Number) {
	std::ostringstream ss;
	ss << Number;
	return ss.str();
}

void printUpdate(Engine* engine, ChessBoard* board) {
	int xOrig = std::get<0>(engine->optimalTurnSequence.at(0));
	int yOrig = std::get<1>(engine->optimalTurnSequence.at(0));
	int xDest = std::get<2>(engine->optimalTurnSequence.at(0));
	int yDest = std::get<3>(engine->optimalTurnSequence.at(0));
	if (engine->seachAborted()) std::cout << "\r" << "DONE. ";
	else std::cout << "\r" << "CALC..";
	std::cout << "opt=[" <<
		numToLetter(xOrig) << numberToString(yOrig + 1) << numToLetter(xDest) << numberToString(yDest + 1) <<
		"]@val " << engine->optimalValue << " || " << "Depth " << int(engine->depthLimit) + int(engine->quiescenceLimit) << "="
		<< int(engine->depthLimit) << "n+"<< int(engine->quiescenceLimit)<<"h, " << int(board->transpos_table.getHashHits() / 1000) << "k Hash hits, "
		<< (engine->getNodes() + engine->getQuiescenceNodes()) / 1000 << " kN="<< int(engine->getNodes()/1000) << "n+" << int(engine->getQuiescenceNodes()/1000)
		<< "h, " << int(((engine->getNodes() + engine->getQuiescenceNodes()) / 1000) / engine->getTimePassed())
		<< " kN/s, T=" << engine->getTimePassed() << "s    ";
}

void printBoard(ChessBoard* board, std::vector<std::tuple<char, char, char, char>>* moveList, MoveData* move) {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	int x, y;
	std::cout << "|";
	SetConsoleTextAttribute(hConsole, 114);
	std::cout << "    a  b  c  d  e  f  g  h    ";
	SetConsoleTextAttribute(hConsole, 15);
	std::cout << "|" << std::endl;
	for (y = 7; y >= 0; y--) {
		SetConsoleTextAttribute(hConsole, 15);
		std::cout << "|";
		SetConsoleTextAttribute(hConsole, 114);
		std::cout <<" "<< numberToString(y + 1) << " ";
		for (x = 0; x < 8; x++) {
			if ((x + y) % 2 == 0) {
				SetConsoleTextAttribute(hConsole, 47);
			}
			else {
				SetConsoleTextAttribute(hConsole, 143);
			}
			char moveAdj = ' ';
			if (moveList != nullptr) {
				for (unsigned int i = 0; i < moveList->size(); i++) {
					if ((std::get<2>((*moveList)[i]) == x) && ((std::get<3>((*moveList)[i]) == y))) {
						moveAdj = '*';
					}
				}
			}
			if (board->squares[x][y] != nullptr) {
				if ((move != nullptr) && (std::get<0>(move->dest) == x) && (std::get<1>(move->dest) == y)) {
					moveAdj = '<';
				}
				if (board->squares[x][y]->colour == Colour::Black) {
					if ((x + y) % 2 == 0) {
						SetConsoleTextAttribute(hConsole, 32);
					}
					else {
						SetConsoleTextAttribute(hConsole, 128);
					}
				}		
				std::cout << " " << board->squares[x][y]->getSymbol() << moveAdj;
			}
			else {
				if (move != nullptr) {
					if (move->pieceMoved->colour == Colour::Black) {
						if ((x + y) % 2 == 0) {
							SetConsoleTextAttribute(hConsole, 32);
						}
						else {
							SetConsoleTextAttribute(hConsole, 128);
						}
					}
					if ((std::get<0>(move->orig) == x) && (std::get<1>(move->orig) == y)) {
						moveAdj = '_';
					}
				}
				std::cout << " " << moveAdj << " ";
			}
		}
		SetConsoleTextAttribute(hConsole, 114);
		std::cout << " " << numberToString(y + 1) << " ";
		SetConsoleTextAttribute(hConsole, 15);
		std::cout << "|" << std::endl;
	}
	SetConsoleTextAttribute(hConsole, 15);
	std::cout << "|";
	SetConsoleTextAttribute(hConsole, 114);
	std::cout << "    a  b  c  d  e  f  g  h    ";
	SetConsoleTextAttribute(hConsole, 15);
	std::cout << "|" << std::endl;
}

Colour switchColour(Colour colour) {
	if (colour == Colour::White) {
		return Colour::Black;
	}
	else {
		return Colour::White;
	}
}

void printMoveType(MoveData* move) {
	if (move->isPromotion) {
		if (move->pieceMoved->colour == Colour::White) std::cout << "White promoted a Pawn!" << std::endl;
		else std::cout << "Black promoted a Pawn!" << std::endl;
	}
	if ((move->isWhiteLRookCastling) || (move->isWhiteRRookCastling))
		std::cout << "White castled!" << std::endl;
	if ((move->isBlackLRookCastling) || (move->isBlackRRookCastling))
		std::cout << "Black castled!" << std::endl;
	if (move->isEnPassant) {
		if (move->pieceMoved->colour == Colour::White) std::cout << "White captured en passant!" << std::endl;
		else std::cout << "Black captured en passant!" << std::endl;
	}
}

void printHelp() {
	std::cout << "Game Commands:" << std::endl;
	std::cout << "  'reset' restores the board to its initial state" << std::endl;
	std::cout << "  'help' displays the command list" << std::endl;
	std::cout << "Move Commands:" << std::endl;
	std::cout << "  'PQXY' moves the piece on PQ to XY (e.g. a4a5)" << std::endl;
	std::cout << "  'undo' reverts the last move" << std::endl;
	std::cout << "  'getmoves' displays all squares that can be moved to" << std::endl;
	std::cout << "  'getcaptures' displays all attacked opponent pieces" << std::endl;
	std::cout << "Engine Commands:" << std::endl;
	std::cout << "  'engine' makes the engine respond with a move" << std::endl;
	std::cout << "  'engine_loop' lets the black and white engines play each other" << std::endl;
	std::cout << "  'white_alloctime w' allocates a maximum computation time of w seconds to white (default: 2.5s)" << std::endl;
	std::cout << "  'black_alloctime b' does the same for black (default: 2.5s)" << std::endl;
	std::cout << "  'white_rnd' toggles from calculated moves by white to completely random moves" << std::endl;
	std::cout << "  'black_rnd' does the same for black" << std::endl;
	std::cout << "  'quiet_limit h' to set the maximum horizon search depth to h (default: h=2)" << std::endl;
	std::cout << "  'toggle_hashtable' to enable/disable hashtables (default: enabled)" << std::endl;
}

void printHeuristicVal(Engine* engine, ChessBoard* board) {
	engine->randomness = false;
	std::cout << "  " << engine->evalHeuristic(board) << "=Heuristic Valuation";
	engine->randomness = true;
}

int main()
{
	std::cout << "CONSOLE CHESS ENGINE v2021/10/17- baran.oener@gmail.com" << std::endl;
	std::cout << "kN = kilonode = 1000 positions // h = horizon nodes // n = normal nodes // White maximizes & Black minimizes valuation" << std::endl;
	printHelp();
	ChessBoard board;
	MoveData move;
	Engine engine;
	Colour colour = Colour::White;
	int moveCount = 1;
	const int MAX_MOVE_COUNT = 200;
	int gameCount = 0;
	int blackWins = 0;
	int whiteWins = 0;
	int draws = 0;
	engine.updateFct = printUpdate;
	float alloc_time_white = 1;
	float alloc_time_black = 1;
	bool white_rnd = false;
	bool black_rnd = false;
	std::string command = " ";
	printBoard(&board, nullptr, nullptr);
	std::cout << "Hash: " << std::hex << board.transpos_table.getHash() << std::dec << std::endl;
	while ((command != "exit") && (command != "")){

		//Prints the player whose turn it is and the heuristic valuation
		if (command != "skip_print") {
			std::cout << "[" << moveCount << "] ";
			if (colour == Colour::White) 
				std::cout << "White's turn.";
			else 
				std::cout << "Black's turn.";
			if (moveCount < 10) std::cout << " ";
			if (moveCount < 100) std::cout << " ";
			printHeuristicVal(&engine, &board);
			std::cout << std::endl;
		}

		//Get player command unless the engines are running a loop
		if (command != "engine_loop") {
			std::cout << ">";
			std::cin >> command;
		}

		//cycle through the different commands
		if (command == "undo") {
			if (move.validMove) {
				board.undoMove(move);
				colour = switchColour(colour);
				printBoard(&board, nullptr, nullptr);
			}
			move.validMove = false;
		}
		else if (command == "debug") {
			board.resetToDebugBoard();
			colour = Colour::White;
			printBoard(&board, nullptr, nullptr);
		}
		else if (command == "help") {
			printHelp();
			printBoard(&board, nullptr, nullptr);
			printHeuristicVal(&engine, &board);
		}
		else if (command == "reset") {
			board.resetBoard();
			colour = Colour::White;
			moveCount = 1;
			printBoard(&board, nullptr, nullptr);
		}
		else if (command == "exit") {
			continue;
		}
		else if (command == "black_alloctime") {
			std::cin >> alloc_time_black;
			command = "skip_print";
		}
		else if (command == "white_alloctime") {
			std::cin >> alloc_time_white;
			command = "skip_print";
		}
		else if (command == "white_rnd") {
			white_rnd = !white_rnd;
			if (white_rnd) std::cout << "The engine now randomizes moves for white" << std::endl;
			else std::cout << "Move randomization is switched off for white" << std::endl;
			command = "skip_print";
		}
		else if (command == "black_rnd") {
			black_rnd = !black_rnd;
			if (black_rnd) std::cout << "The engine now randomizes moves for black" << std::endl;
			else std::cout << "Move randomization is switched off for black" << std::endl;
			command = "skip_print";
		}
		else if (command == "toggle_hashtable") {
			engine.useHashtable = !engine.useHashtable;
			if (engine.useHashtable) std::cout << "Hashtables enabled" << std::endl;
			else std::cout << "Hashtables disabled" << std::endl;
			command = "skip_print";
		}
		else if (command == "quiet_limit") {
			std::cin >> engine.quiescenceLimit;
			command = "skip_print";
		}
		else if (command == "engine") {
			int val;
			if (colour == Colour::White) {
				if (white_rnd)
					engine.calculateMove_random(&board, colour);
				else {
					engine.timeLimit = alloc_time_white;
					val = engine.calculateMove_iterativeDeepening(&board, colour);
				}
			}
			else {
				if (black_rnd) {
					engine.calculateMove_random(&board, colour);
				}
				else {
					engine.timeLimit = alloc_time_black;
					val = engine.calculateMove_iterativeDeepening(&board, colour);
				}
			}
			std::cout << std::endl;
			int xOrig = std::get<0>(engine.optimalTurnSequence.at(0));
			int yOrig = std::get<1>(engine.optimalTurnSequence.at(0));
			int xDest = std::get<2>(engine.optimalTurnSequence.at(0));
			int yDest = std::get<3>(engine.optimalTurnSequence.at(0));
			move = board.moveTo(std::tuple<char, char>(xOrig, yOrig), std::tuple<char, char>(xDest, yDest));
			if (move.validMove) {
				if (((colour == Colour::White) && (white_rnd)) || ((colour == Colour::Black) && (black_rnd)))
					std::cout << "RND=" << numToLetter(xOrig) << numberToString(yOrig + 1) <<
					numToLetter(xDest) << numberToString(yDest + 1) << std::endl;
				else
					std::cout << std::endl;
				printBoard(&board, nullptr, &move);
				printMoveType(&move);
				colour = switchColour(colour);
				moveCount++;
			}
		}
		else if (command == "engine_d") {
			engine.depthLimit = 4;
			int val = engine.calculateMove_fixedDepth(&board, colour);
			std::cout << std::endl;
			int xOrig = std::get<0>(engine.optimalTurnSequence.at(0));
			int yOrig = std::get<1>(engine.optimalTurnSequence.at(0));
			int xDest = std::get<2>(engine.optimalTurnSequence.at(0));
			int yDest = std::get<3>(engine.optimalTurnSequence.at(0));
			move = board.moveTo(std::tuple<char, char>(xOrig, yOrig), std::tuple<char, char>(xDest, yDest));
			if (move.validMove) {
				printBoard(&board, nullptr, &move);
				printMoveType(&move);
				colour = switchColour(colour);
				moveCount++;
			}
			std::cout << "=> Engine Move: " << numToLetter(xOrig) << numberToString(yOrig + 1) << " " << numToLetter(xDest) << numberToString(yDest + 1) << std::endl;
		}
		else if (command == "engine_loop") {
			int val;
			int xOrig;
			int yOrig;
			int xDest;
			int yDest;
			if (colour == Colour::White) {
				if (white_rnd)
					engine.calculateMove_random(&board, colour);
				else {
					engine.timeLimit = alloc_time_white;

					val = engine.calculateMove_iterativeDeepening(&board, colour);
				}
				xOrig = std::get<0>(engine.optimalTurnSequence.at(0));
				yOrig = std::get<1>(engine.optimalTurnSequence.at(0));
				xDest = std::get<2>(engine.optimalTurnSequence.at(0));
				yDest = std::get<3>(engine.optimalTurnSequence.at(0));
			}
			else {
				if (black_rnd) {
					engine.calculateMove_random(&board, colour);
				}
				else {
					engine.timeLimit = alloc_time_black;

					val = engine.calculateMove_iterativeDeepening(&board, colour);
				}
				xOrig = std::get<0>(engine.optimalTurnSequence.at(0));
				yOrig = std::get<1>(engine.optimalTurnSequence.at(0));
				xDest = std::get<2>(engine.optimalTurnSequence.at(0));
				yDest = std::get<3>(engine.optimalTurnSequence.at(0));
			}

			move = board.moveTo(std::tuple<char, char>(xOrig, yOrig), std::tuple<char, char>(xDest, yDest));
			if (move.validMove) {
				if (((colour == Colour::White) && (white_rnd)) || ((colour == Colour::Black) && (black_rnd)))
					std::cout << "RND=" << numToLetter(xOrig) << numberToString(yOrig + 1) <<
					numToLetter(xDest) << numberToString(yDest + 1) << std::endl;
				else
					std::cout << std::endl;
				printBoard(&board, nullptr, &move);
				printMoveType(&move);
				std::cout << "Hash: " << std::hex << board.transpos_table.getHash() << std::dec << ", Endgame: " << engine.isEndgame(&board) << std::endl;
				colour = switchColour(colour);
				moveCount++;
			}
			if (moveCount > MAX_MOVE_COUNT) {
				std::cout << "Move limit of "<< MAX_MOVE_COUNT << " reached." << std::endl;
				draws++;
				std::cout << "White wins:  " << whiteWins << ", Black wins: " << blackWins << ", Draws: " << draws << std::endl;
				std::cout << "Resetting Board." << std::endl;
				board.resetBoard();
				moveCount = 1;
				colour = Colour::White;
				printBoard(&board, nullptr, nullptr);
				//DEBUG
				//command = " ";
			}
			if (board.drawBy50Moves()) {
				std::cout << "Draw by 50-move-rule."<< std::endl;
				draws++;
				std::cout << "White wins:  " << whiteWins << ", Black wins: " << blackWins << ", Draws: " << draws << std::endl;
				std::cout << "Resetting Board." << std::endl;
				board.resetBoard();
				moveCount = 1;
				colour = Colour::White;
				printBoard(&board, nullptr, nullptr);

				//DEBUG
				//command = " ";
			}
		}
		else if (command == "getcapturemoves") {
			//printBoard(&board, &board.getPossibleCaptures(colour), nullptr);
		}
		else if (command == "getmoves") {
			//printBoard(&board, &board.getPossibleMoves(colour), nullptr);
		}
		else if (command == "isattacked") {
			std::cin >> command;
			std::tuple<char, char> orig = std::tuple<char, char>(letterToNum(command.at(0)), (command.at(1) - '0') - 1); //Write error handler
			if ((std::get<0>(orig) != CONV_ERROR) && (std::get<1>(orig) > 0) && (std::get<1>(orig) < 9)) {
				std::cout << std::endl;
				if (board.squareAttackedBy(orig, colour)) {
					std::cout << "Attacked" << std::endl;
				}
				else {
					std::cout << "Not Attacked" << std::endl;
				}
			}
		}
		else if (command.length() == 4) {
			std::tuple<char, char> orig = std::tuple<char, char>(letterToNum(command.at(0)), (command.at(1) - '0') - 1); //Write error handler
			std::tuple<char, char> dest = std::tuple<char, char>(letterToNum(command.at(2)), (command.at(3) - '0') - 1);
			if ((std::get<0>(orig) != CONV_ERROR) && (std::get<1>(orig) >= 0) && (std::get<1>(orig) <= 7) && 
				(std::get<0>(dest) != CONV_ERROR) && (std::get<1>(dest) >= 0) && (std::get<1>(dest) <= 7) &&
				(board.squares[std::get<0>(orig)][std::get<1>(orig)]!= nullptr) &&
				(board.squares[std::get<0>(orig)][std::get<1>(orig)]->colour == colour)) {
				std::cout << std::endl;
				move = board.moveTo(orig, dest);
				if (!move.validMove) {
					std::cout << "Invalid move" << std::endl;
				}
				else {
					printBoard(&board, nullptr, &move);
					printMoveType(&move);
					colour = switchColour(colour);
				}
			}
			else std::cout << "Invalid coordinates" << std::endl;
		}
		else std::cout << "Invalid command" << std::endl;

		//Evaluate the game state for checks & checkmates
		if ((colour == Colour::White) && (board.getPossibleMoves(Colour::White).empty())) {
			if (board.isChecked(Colour::White)) {
				std::cout << "White is checkmate!" << std::endl;
				blackWins++;
				//printBoard(&board, &board.getPossibleMoves(Colour::Black), nullptr);
				std::cout << "White wins:  " << whiteWins << ", Black wins: " << blackWins << ", Draws: " << draws << std::endl;
				std::cout << "Resetting Board." << std::endl;
				board.resetBoard();
				colour = Colour::White;
				printBoard(&board, nullptr, nullptr);
				std::cout << "Hash: " << std:: hex << board.transpos_table.getHash() << std::dec << std::endl;
				moveCount = 1;
				//DEBUG
				//command = " ";
			}
			else {
				board.resetBoard();
				std::cout << "Draw!" << std::endl;
				draws++;
				std::cout << "White wins:  " << whiteWins << ", Black wins: " << blackWins << ", Draws: " << draws << std::endl;
				std::cout << "Resetting Board." << std::endl;
				colour = Colour::White;
				printBoard(&board, nullptr, nullptr);
				std::cout << "Hash: " << std::hex << board.transpos_table.getHash() << std::dec << std::endl;
				moveCount = 1;
			}
		}
		else if (board.isChecked(Colour::White)) {
			std::cout << "White is checked" << std::endl;
		}
		if ((colour == Colour::Black) && (board.getPossibleMoves(Colour::Black).empty())) {
			if (board.isChecked(Colour::Black)) {
				std::cout << "Black is checkmate!" << std::endl;
				whiteWins++;
				//printBoard(&board, &board.getPossibleMoves(Colour::White), nullptr);
				std::cout << "White wins:  " << whiteWins << ", Black wins: " << blackWins << ", Draws: " << draws << std::endl;
				std::cout << "Resetting Board." << std::endl;
				board.resetBoard();
				colour = Colour::White;
				printBoard(&board, nullptr, nullptr);
				std::cout << "Hash: " << std::hex << board.transpos_table.getHash() << std::dec << std::endl;
				moveCount = 1;
				//DEBUG
				//command = " ";
			}
			else {
				board.resetBoard();
				std::cout << "Draw!" << std::endl;
				draws++;
				std::cout << "White wins:  " << whiteWins << ", Black wins: " << blackWins << ", Draws: " << draws << std::endl;
				std::cout << "Resetting Board." << std::endl;
				colour = Colour::White;
				printBoard(&board, nullptr, nullptr);
				std::cout << "Hash: " << std::hex << board.transpos_table.getHash() << std::dec << std::endl;
				moveCount = 1;
			}
		}
		else if (board.isChecked(Colour::Black)) {
			std::cout << "Black is checked" << std::endl;
		}
	}
}