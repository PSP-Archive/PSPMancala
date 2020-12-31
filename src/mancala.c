/* $Id: mancala.c,v 1.1.1.1 1995/07/25 11:55:21 sverrehu Exp $ */
/**************************************************************************
 *
 *  FILE            mancala.c
 *  MODULE OF       The board game Mancala.
 *
 *  DESCRIPTION     An implementation of the simple game called Mancala.
 *                  This is the backend.
 *
 *  WRITTEN BY      Sverre H. Huseby
 *
 **************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "psp_global.h"

#include "minimax.h"
#include "mancala.h"

/**************************************************************************
 *                                                                        *
 *                       P R I V A T E    D A T A                         *
 *                                                                        *
 **************************************************************************/

/*
 *  Define this if you want a more timeconsuming evaluation function.
 *  We're not sure it makes it any better -- it may even make it worse.
 */
#define FULL_EVAL

/*
 *  The board state stack, and the current stacklevel.
 */
static Board boardStack[STACK_LEVELS];
static int   idx = 0;

/*
 *  The game logic function requests an array of pointers to possible
 *  moves for a level of it's recursion. We need one array for each
 *  possible recursion level, so we don't mess things up. possibleMoveTable
 *  contains the real move variables, while possibleMove contains the
 *  pointers into the variable table.
 */
static Move  possibleMoveTable[STACK_LEVELS][MAX_HOLES];
static PMove possibleMove[STACK_LEVELS][MAX_HOLES];



/**************************************************************************
 *                                                                        *
 *                   P R I V A T E    F U N C T I O N S                   *
 *                                                                        *
 **************************************************************************/

/*------------------------------------------------------------------------*
 |                 Callbacks for the game logic function                  |
 *------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
 *
 *  NAME          pushBoard
 *
 *  FUNCTION      Save the current board state on a stack.
 *
 *  DESCRIPTION   This callback is called at the start of each recursion
 *                level. We use it to save the current board setup, and
 *                to make sure requests for new moves (getMoves()) don't
 *                mess up the array of moves currently being explored.
 */
static void 
pushBoard(void)
{
  ++idx;
  memcpy(&boardStack[idx], &boardStack[idx - 1], sizeof(Board));
}



/*-------------------------------------------------------------------------
 *
 *  NAME          popBoard
 *
 *  FUNCTION      Restore the current board stat from the stack.
 *
 *  DESCRIPTION   This callback is called at the end of each recursion
 *                level. We restore the board state (remove the
 *                `experimental' moves), and open for reuse of the array
 *                of possible moves.
 */
static void 
popBoard(void)
{
  --idx;
}

/*-------------------------------------------------------------------------
 *
 *  NAME          getMoves
 *
 *  FUNCTION      Make list of possible moves for the given player.
 *
 *  INPUT         player  the player to make movelist for.
 *
 *  OUTPUT        numMoves
 *                        number of moves in the returned array.
 *
 *  RETURNS       An array of pointers to legal moves, or NULL if no
 *                legal moves for this player.
 *
 *  DESCRIPTION   This callback is called for each recursion level to
 *                get a list of possible following moves. A legal move is
 *                the number of a non-empty hole for the player in question.
 *
 *                The current stacklevel is used to find a `free' array
 *                that we can fill with legal moves. We couldn't use the
 *                same array all the time, since the array we returned in
 *                the previous call is not fully examined (yeah, recursion
 *                gives some interresting problems...).
 */
static PMove *
getMoves(Player player, int *numMoves)
{
    int        q, n = 0;
    StoneCount *hole;
    PMove      *m, *ret;

    m = ret = possibleMove[idx];
    hole = boardStack[idx].hole[player];
    for (q = 0; q < MAX_HOLES; q++) {
      if (*hole) m[n++]->hole = q;
      ++hole;
    }
    if ((*numMoves = n) == 0)
    ret = NULL;
    return ret;
}



/*-------------------------------------------------------------------------
 *
 *  NAME          undoMove
 *
 *  FUNCTION      Update the board to the state before the given move.
 *
 *  INPUT         player  the player that did the move.
 *                move    pointer to the move to undo.
 *
 *  DESCRIPTION   This callback is called at the end of each recursion
 *                level. Since we really change the board in popBoard(),
 *                we don't need to do anything here.
 */
static void undoMove(Player player, PMove move)
{
    /* memcpy(&boardStack[idx], &boardStack[idx - 1], sizeof(Board)); */
}



/*-------------------------------------------------------------------------
 *
 *  NAME          evalBoard
 *
 *  FUNCTION      Evaluate the board after a player has moved.
 *
 *  INPUT         player  the player to evaluate the board for.
 *
 *  RETURNS       A measure indicating how good the current board is for
 *                the given player. Larger values are better. A positive
 *                value indicates that the given player is in the lead,
 *                while a negative value indicates the opposite.
 *
 *  DESCRIPTION   We do it simple; just see which player has captured
 *                most stones.
 */
static Score 
evalBoard(Player player)
{
    Board  *b;

#ifdef FULL_EVAL
    Player winner;
    /*
     *  The next thing that happens after the evaluation, is that the
     *  previous board is popped from the stack, so we can let the
     *  evaluation function mess the board up like this:
     */
    checkAndFixWin(&winner);
#endif
    b = &boardStack[idx];
    return b->mancala[player] - b->mancala[player ^ 1];
}



/**************************************************************************
 *                                                                        *
 *                    P U B L I C    F U N C T I O N S                    *
 *                                                                        *
 **************************************************************************/

/*-------------------------------------------------------------------------
 *
 *  NAME          initGame
 *
 *  FUNCTION      Set up variables for a new game.
 */
void 
initGame(void)
{
    int    q, w;
    Board  *b;
    Player player;
    
    /*
     *  An important step: Set up the possible move -pointers that the
     *  caller of getMoves() want. We make pointers into the table
     *  containing the real values.
     */
    for (q = 0; q < STACK_LEVELS; q++) {
      for (w = 0; w < MAX_HOLES; w++) {
        possibleMove[q][w] = &possibleMoveTable[q][w];
      }
    }

    /*
     *  Clear the game stack, and set up the board to initial state.
     *  The `real' board is actually at position 0 of the stack array.
     */
    idx = 0;
    b = &boardStack[idx];

    player = 0;
    b->mancala[player] = 0;
    for (w = 0; w < MAX_HOLES; w++) {
      b->hole[player][w] = MANCALA.board_array[(MANCALA_WIDTH-1) - w];
    }

    player = 1;
    b->mancala[player] = 0;
    for (w = 0; w < MAX_HOLES; w++) {
      b->hole[player][w] = MANCALA.board_array[(MANCALA_NUM_PITS-1) - w];
    }
}

int
psp_ai_get_best_move()
{
  PMove bestmove;
  int   hole;
  int   index;

  initGame();

  bestmove = getBestMove(MANCALA.to_move, MANCALA.mancala_level);

  hole  = bestmove->hole;
  if (MANCALA.to_move == MANCALA_NORTH) index = hole;
  else                                 index = (MANCALA_WIDTH -1) - hole;

  return index;
}

/*-------------------------------------------------------------------------
 *
 *  NAME          getHole
 *
 *  FUNCTION      Get number of stones in a hole on the board.
 *
 *  INPUT         player  the player to get number of stones for.
 *                hole    the hole number. 0 is the hole closest to
 *                        the mancala.
 *
 *  RETURNS       The number of stones in the given hole.
 */
StoneCount getHole(Player player, int hole)
{
    return boardStack[0].hole[player][hole];
}

//LUDO:
void
setHole(Player player, int hole, StoneCount stones)
{
  boardStack[0].hole[player][hole] = stones;
}


/*-------------------------------------------------------------------------
 *
 *  NAME          getMancala
 *
 *  FUNCTION      Get number of stones in the mancala of a player.
 *
 *  INPUT         player  the player to get number of stones for.
 *
 *  RETURNS       The number of stones in the given player's mancala.
 */
StoneCount getMancala(Player player)
{
    return boardStack[0].mancala[player];
}



/*-------------------------------------------------------------------------
 *
 *  NAME          legalMove
 *
 *  FUNCTION      Check if a human's move is legal.
 *
 *  INPUT         player  the player that wants to move.
 *                move    pointer to the move to check.
 *
 *  RETURNS       1 if legal move, 0 if illegal.
 *
 *  DESCRIPTION   This is used to check if a human player has chosen to
 *                do a legal move. The computer player will only do moves
 *                returned by getMoves(), and those are (or should) all be
 *                legal.
 */
int 
legalMove(Player player, PMove move)
{
    int   q;
    PMove *legalmove;     /* array of pointerts to legal moves */
    int   numMoves;       /* number of moves in this array */

    legalmove = getMoves(player, &numMoves);
    for (q = 0; q < numMoves; q++) {
      if ((*legalmove)->hole == move->hole) return 1;
      ++legalmove;
    }
    return 0;
}



/*-------------------------------------------------------------------------
 *
 *  NAME          checkAndFixWin
 *
 *  FUNCTION      Check if the game is ended, and in that case fix scores.
 *
 *  OUTPUT        winner  the player that won.
 *
 *  RETURNS       0 if game not finished, 1 if one winner, 2 if draw.
 *
 *  DESCRIPTION   The game is over if any of the players have no stones
 *                left to move. In that case, all remaining stones for the
 *                opposite player is moved to that player's mancala, and
 *                the winner is the one with more stones.
 */
int 
checkAndFixWin(Player *winner)
{
    int        q, ret = 0, sum[2];
    Board      *b;
    StoneCount *hole;
    Player     player;

    b = &boardStack[idx];
    for (player = 0; player < 2; player++) {
      hole = b->hole[player];
      sum[player] = 0;
      for (q = 0; q < MAX_HOLES; q++) {
        sum[player] += hole[q];
      }
    }
    if (sum[0] == 0 || sum[1] == 0) {
      ret = 1;
      for (player = 0; player < 2; player++) {
        b->mancala[player] += sum[player];
        hole = b->hole[player];
        for (q = 0; q < MAX_HOLES; q++) {
          hole[q] = 0;
        }
      }
      if (b->mancala[0] > b->mancala[1]) *winner = 0;
      else if (b->mancala[0] < b->mancala[1]) *winner = 1;
      else ret = 2;
   }
   return ret;
}

/*------------------------------------------------------------------------*
 |   The following function is a callback, but it's used for moving too.  |
 *------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
 *
 *  NAME          doMove
 *
 *  FUNCTION      Update the board with the given move.
 *
 *  INPUT         player  the player that does the move.
 *                move    pointer to the move to make.
 *
 *  RETURNS       The next player to move. For Mancala, this is the same
 *                player once again if the last stone ends in the mancala.
 *
 *  DESCRIPTION   This function is used both as a callback for finding the
 *                best moves, and as a `normal' function to update the
 *                board when a move is chosen.
 */
Player 
mancala_doMove(Player player, PMove move)
{
  int        seeds, holeNum, oppositeHoleNum;
  StoneCount *hole, *oppositeHole;
  Board      *b;
  Player     nextPlayer, holeOwner;

  nextPlayer = player ^ 1;
  b = &boardStack[idx];
  holeOwner = player;
  hole = b->hole[holeOwner];
  holeNum = move->hole;
  seeds = hole[holeNum];
  hole[holeNum] = 0;
  for (;;) {
    if (--holeNum < 0) {
      if (holeOwner == player) {
        ++b->mancala[player];
        if (!--seeds) {
            nextPlayer = player;
            break;
        }
      }
      holeOwner ^= 1;
      hole = b->hole[holeOwner];
      holeNum = MAX_HOLES - 1;
    }
    ++hole[holeNum];
    if (!--seeds) {
      if (holeOwner == player) {
        oppositeHoleNum = MAX_HOLES - 1 - holeNum;
        oppositeHole = b->hole[player ^ 1];
        if (hole[holeNum] == 1 && oppositeHole[oppositeHoleNum]) {
          b->mancala[player] += 1 + oppositeHole[oppositeHoleNum];
          oppositeHole[oppositeHoleNum] = 0;
          hole[holeNum] = 0;
        }
      }
      break;
    }
  }
  return nextPlayer;
}

Player 
awari_doMove(Player player, PMove move)
{
  int        seeds, holeNum;
  int        first_holeNum;
  StoneCount *hole;
  Board      *b;
  Player     nextPlayer, holeOwner;

  nextPlayer = player ^ 1;
  b = &boardStack[idx];
  holeOwner = player;
  hole = b->hole[holeOwner];
  holeNum = move->hole;
  first_holeNum = holeNum;
  seeds = hole[holeNum];
  hole[holeNum] = 0;
  for (;;) {
    if (--holeNum < 0) {
      holeOwner ^= 1;
      hole = b->hole[holeOwner];
      holeNum = MAX_HOLES - 1;
    }
    if ((holeOwner == player) && (holeNum == first_holeNum)) {
      /* we must skip initial hole */
    } else {
      ++hole[holeNum];
      if (!--seeds) break;
    }
  }

  while ((holeOwner != player) && (holeNum < MAX_HOLES)) {
   seeds = hole[holeNum];
   if ((seeds != 2) && (seeds != 3)) break;

   b->mancala[player] += seeds;
   hole[holeNum] = 0;

   holeNum++;
  }
  return nextPlayer;
}

/*-------------------------------------------------------------------------
 *
 *  NAME          getBestMove
 *
 *  FUNCTION      Calculate best move for player.
 *
 *  INPUT         player  the player to evaluate.
 *                maxPly  max recursion depth.
 *
 *  RETURNS       Pointer to the best move to make, or NULL if no move
 *                available.
 *
 *  DESCRIPTION   This is just a frontend to the real game logic function,
 *                hiding the static functions found in this file.
 */
PMove getBestMove(Player player, int maxPly)
{
  if (MANCALA.mancala_game_rules == MANCALA_AWARI) {
    return miniMax(pushBoard, popBoard,
       getMoves, awari_doMove, undoMove, evalBoard,
       player, maxPly);
  } else {
    return miniMax(pushBoard, popBoard,
       getMoves, mancala_doMove, undoMove, evalBoard,
       player, maxPly);
  }
}



