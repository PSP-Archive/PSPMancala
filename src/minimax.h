/* $Id: minimax.h,v 1.1.1.1 1995/07/25 11:55:21 sverrehu Exp $ */
#ifndef MINIMAX_H
#define MINIMAX_H

#include <limits.h>

#ifdef __cplusplus
  extern "C" {
#endif

struct  Move;
typedef struct Move *PMove;

typedef int Player;

typedef int Score;
#define INF_SCORE INT_MAX

PMove miniMax(
	      void   (*fncPushBoard)(void),
	      void   (*fncPopBoard)(void),
	      PMove *(*fncGetMoves)(Player, int *),
	      Player (*fncDoMove)(Player, PMove),
	      void   (*fncUndoMove)(Player, PMove),
	      Score  (*fncEvalBoard)(Player),
	      Player player,
	      int    maxPly
	      );

#ifdef __cplusplus
  }
#endif

#endif
