/*-------------------------------------------------------*/
/* game/liteon.c        ( NTHU CS MapleBBS Ver 3.10 )    */
/*-------------------------------------------------------*/
/* target : 開燈遊戲					 */
/* create : 02/05/23					 */
/* update :   /  /                                       */
/* author : Gein.bbs@csdc.twbbs.org			 */
/* recast : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#include "bbs.h"


#ifdef HAVE_GAME

#define MAX_LEVEL	(b_lines - 3)

enum
{
  TL_XPOS = 2,
  TL_YPOS = 5,

  /* 用 bitwise operators */
  TILE_BLANK = 0,	/* 暗區 */
  TILE_LIGHT = 1	/* 亮區 */
};


static int cx, cy;	/* 目前所在游標 */
static int level;	/* 等級，同時也是 board 的邊長 */
static int onturn;	/* 有幾個燈打開了 */
static int candle;	/* 點了幾次蠟燭 */
static int tl_board[T_LINES - 4][T_LINES - 4];


static void 
tl_setb()			/* set board all 0 */
{
  int i, j;

  move(1, 0);
  clrtobot();

  for (i = 0; i < level; i++)
  {
    move(i + TL_XPOS, TL_YPOS);
    for (j = 0; j < level; j++)
    {
      tl_board[i][j] = TILE_BLANK;
      /* ○ */
      outs("\xA1\xB3");
    }
  }

  cx = cy = onturn = candle = 0;
  move(TL_XPOS, TL_YPOS + 1);	/* move back to (0, 0) */
}


static void
tl_draw(x, y)			/* set/reset and draw a tile */
  int x, y;
{
  tl_board[x][y] ^= TILE_LIGHT;
  move(x + TL_XPOS, y * 2 + TL_YPOS);
  if (tl_board[x][y] == TILE_BLANK)	/* on-turn -> off-turn */
  {
    onturn--;
    /* ○ */
    outs("\xA1\xB3");
  }
  else					/* off-turn -> on-turn */
  {
    onturn++;
    /* ● */
    outs("\xA1\xB4");
  }
}


static void 
tl_turn()			/* turn light and light arround it */
{
  tl_draw(cx, cy);

  if (cx > 0)
    tl_draw(cx - 1, cy);

  if (cx < level - 1)
    tl_draw(cx + 1, cy);

  if (cy > 0)
    tl_draw(cx, cy - 1);

  if (cy < level - 1)
    tl_draw(cx, cy + 1);
}


static void 
tl_candle()			/* cheat: use candle */
{
  /* itoc.註解: 因為大家都破不了這遊戲，所以提供一下作弊用的點蠟燭 */
  tl_draw(cx, cy);
  candle++;
}

static int			/* 1:win 0:lose */
tl_play()			/* play turn_light */
{
  tl_setb();

  while (onturn != level * level)
  {
    switch (vkey())
    {
    case KEY_LEFT:
      cy--;
      if (cy < 0)
	cy = level - 1;
      break;

    case KEY_RIGHT:
      cy++;
      if (cy == level)
	cy = 0;
      break;

    case KEY_UP:
      cx--;
      if (cx < 0)
	cx = level - 1;
      break;

    case KEY_DOWN:
      cx++;
      if (cx == level)
	cx = 0;
      break;

    case 'c':
      tl_candle();
      break;

    case ' ':
    case '\n':
      tl_turn();
      break;

    case 'r':
      tl_setb();
      break;

    case 'q':
      return 0;
    }
    move(cx + TL_XPOS, cy * 2 + TL_YPOS + 1);	/* move back to current (x, y) */
  }
  return 1;
}


int
main_liteon()
{
  char ans[5], buf[80];

  /* 請選擇等級(1～%d)，或按 [Q] 離開： */
  sprintf(buf, "\xBD\xD0\xBF\xEF\xBE\xDC\xB5\xA5\xAF\xC5(1\xA1\xE3%d)\xA1\x41\xA9\xCE\xAB\xF6 [Q] \xC2\xF7\xB6\x7D\xA1\x47", MAX_LEVEL);
  level = vget(b_lines, 0, buf, ans, 3, DOECHO);
  if (level == 'q' || level == 'Q')
  {
    return XEASY;
  }
  else
  {
    level = atoi(ans);
    if (level < 1 || level > MAX_LEVEL)
      return XEASY;
  }

  /* 開燈遊戲 */
  vs_bar("\xB6\x7D\xBF\x4F\xB9\x43\xC0\xB8");
  move(4, 13);
  /* 前情提要： */
  outs("\xAB\x65\xB1\xA1\xB4\xA3\xAD\x6E\xA1\x47");
  move(5, 15);
  /* 有一天，小建回到家發現燈都被關了。 */
  outs("\xA6\xB3\xA4\x40\xA4\xD1\xA1\x41\xA4\x70\xAB\xD8\xA6\x5E\xA8\xEC\xAE\x61\xB5\x6F\xB2\x7B\xBF\x4F\xB3\xA3\xB3\x51\xC3\xF6\xA4\x46\xA1\x43");
  move(6, 15);
  /* 可是他家的燈有一個特性，那就是： */
  outs("\xA5\x69\xAC\x4F\xA5\x4C\xAE\x61\xAA\xBA\xBF\x4F\xA6\xB3\xA4\x40\xAD\xD3\xAF\x53\xA9\xCA\xA1\x41\xA8\xBA\xB4\x4E\xAC\x4F\xA1\x47");
  move(7, 15);
  /* 當一盞燈被按下開關以後，他周圍的燈 */
  outs("\xB7\xED\xA4\x40\xB7\xF8\xBF\x4F\xB3\x51\xAB\xF6\xA4\x55\xB6\x7D\xC3\xF6\xA5\x48\xAB\xE1\xA1\x41\xA5\x4C\xA9\x50\xB3\xF2\xAA\xBA\xBF\x4F");
  move(8, 15);
  /* 原本亮的，就會變暗，原本暗的，就會變亮。 -____-# */
  outs("\xAD\xEC\xA5\xBB\xAB\x47\xAA\xBA\xA1\x41\xB4\x4E\xB7\x7C\xC5\xDC\xB7\x74\xA1\x41\xAD\xEC\xA5\xBB\xB7\x74\xAA\xBA\xA1\x41\xB4\x4E\xB7\x7C\xC5\xDC\xAB\x47\xA1\x43 -____-#");
  move(9, 15);
  /* 現在就請聰明的您幫他把所有燈打開吧！ */
  outs("\xB2\x7B\xA6\x62\xB4\x4E\xBD\xD0\xC1\x6F\xA9\xFA\xAA\xBA\xB1\x7A\xC0\xB0\xA5\x4C\xA7\xE2\xA9\xD2\xA6\xB3\xBF\x4F\xA5\xB4\xB6\x7D\xA7\x61\xA1\x49");

  move(11, 13);
  /* 按鍵說明： */
  outs("\xAB\xF6\xC1\xE4\xBB\xA1\xA9\xFA\xA1\x47");
  move(12, 15);
  /* ↑↓←→     移動方向 */
  outs("\xA1\xF4\xA1\xF5\xA1\xF6\xA1\xF7     \xB2\xBE\xB0\xCA\xA4\xE8\xA6\x56");
  move(13, 15);
  /* Enter/Space  切換開關 */
  outs("Enter/Space  \xA4\xC1\xB4\xAB\xB6\x7D\xC3\xF6");
  move(14, 15);
  /* c            點燃蠟燭 [密技] */
  outs("c            \xC2\x49\xBF\x55\xC4\xFA\xC0\xEB [\xB1\x4B\xA7\xDE]");
  move(15, 15);
  /* r            重新來過 */
  outs("r            \xAD\xAB\xB7\x73\xA8\xD3\xB9\x4C");
  move(16, 15);
  /* q            離開遊戲 */
  outs("q            \xC2\xF7\xB6\x7D\xB9\x43\xC0\xB8");

  vmsg(NULL);

  if (tl_play())		/* if win */
  {
    /* 恭喜您成功了  (用了 %d 根蠟燭) */
    sprintf(buf, "\xAE\xA5\xB3\xDF\xB1\x7A\xA6\xA8\xA5\x5C\xA4\x46  (\xA5\xCE\xA4\x46 %d \xAE\xDA\xC4\xFA\xC0\xEB)", candle);
    vmsg(buf);
  }

  return 0;
}
#endif				/* HAVE_GAME */
