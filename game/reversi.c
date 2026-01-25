/*-------------------------------------------------------*/
/* reversi.c	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : 黑白棋遊戲					 */
/* create : 01/07/24					 */
/* update :   /  /					 */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_GAME

enum
{
  /* GRAY_XPOS + MAP_X 要小於 b_lines - 2 = 21    *
   * GRAY_YPOS + MAP_Y * 2 要小於 STRLEN - 1 = 79 *
   * GRAY_YPOS 要足夠使 out_prompt() 放入 */

  GRAY_XPOS = 2,	/* ↓ x 方向 */
  GRAY_YPOS = 17,	/* → y 方向 */

  MAP_X = 8,		/* 要是偶數 */
  MAP_Y = 8,		/* 要是偶數 */

  /* These are flags for "map, tile" bitwise operators */
  TILE_BLANK= 0,	/* 沒有標記 */
  TILE_CPU = 1,		/* 電腦擁有 */
  TILE_USR = 2,		/* 玩家擁有 */

  /* These are flags for "level" bitwise operators */
  LEVEL_USR_FIRST = 1,	/* 玩家先下 */
  LEVEL_1 = 2,		/* 一級 */
  LEVEL_2 = 4,		/* 二級 */
  LEVEL_3 = 8		/* 三級 */
};


/* □ */
/* ● */
/* ○ */
static char piece[3][3] = {"\xA1\xBC", "\xA1\xB4", "\xA1\xB3"};
static char map[MAP_X][MAP_Y];	/* 地圖上每格的擁有者 */
static int cx, cy;		/* current (x, y) */
static int EndGame;		/* -1: 離開遊戲 1: 遊戲結束 0: 還在玩 */

#define	mouts(x,y,t)	{ move(GRAY_XPOS + x, GRAY_YPOS + (y) * 2); outs(piece[t]); }


/* int count##(x, y, tile)     */
/* ##     : 測試的方向 N E W S */
/* x, y   : 放置的 (x,y) 座標  */
/* tile   : 誰下的子	       */
/* return : 能吃幾子	       */


static int
countN(x, y, tile)
  int x, y;
  int tile;
{
  int i;

  for (i = x - 1; i > 0;)
  {
    if (map[i][y] == TILE_BLANK || map[i][y] & tile)
      break;
    i--;
  }

  if (i != x - 1 && map[i][y] & tile)
    return x - i - 1;
  return 0;
}


static int
countS(x, y, tile)
  int x, y;
  int tile;
{
  int i;

  for (i = x + 1; i < MAP_X - 1;)
  {
    if (map[i][y] == TILE_BLANK || map[i][y] & tile)
      break;
    i++;
  }

  if (i != x + 1 && map[i][y] & tile)
    return i - x - 1;
  return 0;
}


static int
countE(x, y, tile)
  int x, y;
  int tile;
{
  int j;

  for (j = y + 1; j < MAP_Y - 1;)
  {
    if (map[x][j] == TILE_BLANK || map[x][j] & tile)
      break;
    j++;
  }

  if (j != y + 1 && map[x][j] & tile)
    return j - y - 1;
  return 0;
}


static int
countW(x, y, tile)
  int x, y;
  int tile;
{
  int j;

  for (j = y - 1; j > 0;)
  {
    if (map[x][j] == TILE_BLANK || map[x][j] & tile)
      break;
    j--;
  }

  if (j != y - 1 && map[x][j] & tile)
    return y - j - 1;
  return 0;
}


static int
countNE(x, y, tile)
  int x, y;
  int tile;
{
  int i, j;

  for (i = x - 1, j = y + 1; i > 0 && j < MAP_Y - 1;)
  {
    if (map[i][j] == TILE_BLANK || map[i][j] & tile)
      break;
    i--;
    j++;
  }

  if (i != x - 1 && map[i][j] & tile)
    return x - i - 1;
  return 0;
}


static int
countNW(x, y, tile)
  int x, y;
  int tile;
{
  int i, j;

  for (i = x - 1, j = y - 1; i > 0 && j > 0;)
  {
    if (map[i][j] == TILE_BLANK || map[i][j] & tile)
      break;
    i--;
    j--;
  }

  if (i != x - 1 && map[i][j] & tile)
    return x - i - 1;
  return 0;
}


static int
countSE(x, y, tile)
  int x, y;
  int tile;
{
  int i, j;

  for (i = x + 1, j = y + 1; i < MAP_X - 1 && j < MAP_Y - 1;)
  {
    if (map[i][j] == TILE_BLANK || map[i][j] & tile)
      break;
    i++;
    j++;
  }

  if (i != x + 1 && map[i][j] & tile)
    return i - x - 1;
  return 0;
}


static int
countSW(x, y, tile)
  int x, y;
  int tile;
{
  int i, j;

  for (i = x + 1, j = y - 1; i < MAP_X - 1 && j > 0;)
  {
    if (map[i][j] == TILE_BLANK || map[i][j] & tile)
      break;
    i++;
    j--;
  }

  if (i != x + 1 && map[i][j] & tile)
    return i - x - 1;
  return 0;
}


static int 		/* 總共可以吃幾子 0: 不能吃 */
do_count(x, y, tile)
{
  if (map[x][y] != TILE_BLANK)
    return 0;

  return countN(x, y, tile) + countS(x, y, tile) + countE(x, y, tile) + countW(x, y, tile) + 
    countNE(x, y, tile) + countNW(x, y, tile) + countSE(x, y, tile) + countSW(x, y, tile);
}


/* void eat##(x, y, tile, num) */
/* ##   : 欲吃的方向 N E W S   */
/* x, y : 放置的 (x,y) 座標    */
/* tile : 誰下的子	       */
/* num  : 吃幾子	       */


static inline void
eatN(x, y, tile, num)
  int x, y;
  int tile;
  int num;
{
  int i;

  for (i = x - 1; i >= x - num; i--)
  {
    map[i][y] = tile;
    mouts(i, y, tile);
  }
}


static inline void
eatS(x, y, tile, num)
  int x, y;
  int tile;
  int num;
{
  int i;

  for (i = x + 1; i <= x + num; i++)
  {
    map[i][y] = tile;
    mouts(i, y, tile);
  }
}


static inline void
eatE(x, y, tile, num)
  int x, y;
  int tile;
  int num;
{
  int j;

  for (j = y + 1; j <= y + num; j++)
  {
    map[x][j] = tile;
    mouts(x, j, tile);
  }
}


static inline void
eatW(x, y, tile, num)
  int x, y;
  int tile;
  int num;
{
  int j;

  for (j = y - 1; j >= y - num; j--)
  {
    map[x][j] = tile;
    mouts(x, j, tile);
  }
}


static inline void
eatNE(x, y, tile, num)
  int x, y;
  int tile;
  int num;
{
  int i, j;

  for (i = x - 1, j = y + 1; i >= x - num; i--, j++)
  {
    map[i][j] = tile;
    mouts(i, j, tile);
  }
}


static inline void
eatNW(x, y, tile, num)
  int x, y;
  int tile;
  int num;
{
  int i, j;

  for (i = x - 1, j = y - 1; i >= x - num; i--, j--)
  {
    map[i][j] = tile;
    mouts(i, j, tile);
  }
}


static inline void
eatSE(x, y, tile, num)
  int x, y;
  int tile;
  int num;
{
  int i, j;

  for (i = x + 1, j = y + 1; i <= x + num; i++, j++)
  {
    map[i][j] = tile;
    mouts(i, j, tile);
  }
}


static inline void
eatSW(x, y, tile, num)
  int x, y;
  int tile;
  int num;
{
  int i, j;

  for (i = x + 1, j = y - 1; i <= x + num; i++, j--)
  {
    map[i][j] = tile;
    mouts(i, j, tile);
  }
}


static void
do_eat(x, y, tile)
{
  /* 吃各方向能吃的 */
  eatN(x, y, tile, countN(x, y, tile));
  eatS(x, y, tile, countS(x, y, tile));
  eatE(x, y, tile, countE(x, y, tile));
  eatW(x, y, tile, countW(x, y, tile));
  eatNE(x, y, tile, countNE(x, y, tile));
  eatNW(x, y, tile, countNW(x, y, tile));
  eatSE(x, y, tile, countSE(x, y, tile));
  eatSW(x, y, tile, countSW(x, y, tile));

  /* 吃所下的這格 */
  map[x][y] = tile;
  mouts(x, y, tile);
}


/* 評分制度，很粗淺的人工智慧，待改善 */

/* 算能吃到幾個邊 */
static int
count_edge(x, y, tile)
  int x, y;
  int tile;
{
  /* 本身是邊，才也可能吃到上下或左右的邊 */
  if (x == 0 || x == MAP_X - 1)
  {
    return 1 + countE(x, y, tile) + countW(x, y, tile);	/* 包括自己一個邊 */
  }
  if (y == 0 || y == MAP_Y - 1)
  {
    return 1 + countN(x, y, tile) + countS(x, y, tile);	/* 包括自己一個邊 */
  }
  return 0;
}


static inline int
find_best(x, y, level)	/* 傳回 (x, y) 電腦所放置最好的位置 */
  int *x, *y;
  int level;
{
  int i, j, bestx, besty, tmp;
  int score = 0;

  if (level & LEVEL_1)			/* 一級: 吃越多越好 */
  {
    for (i = 0; i < MAP_X; i++)
    {
      for (j = 0; j < MAP_Y; j++)
      {
	if (tmp = do_count(i, j, TILE_CPU))
	{
	  if (tmp > score)
	  {
	    score = tmp;
	    bestx = i;
	    besty = j;
	  }
	}
      }
    }
  }
  else if (level & LEVEL_2)		/* 二級: 簡化的金角銀邊 */
  {
    for (i = 0; i < MAP_X; i++)
    {
      for (j = 0; j < MAP_Y; j++)
      {
	if (tmp = do_count(i, j, TILE_CPU))
	{
	  /* 角 +100  邊 +50  一般 +1 */
	  if (i == 0 || i == MAP_X - 1)
	  {
	    if (j == 0 || j == MAP_Y - 1)
	      tmp += 100;
	    else
	      tmp += 50;
	  }
	  else if (j == 0 || j == MAP_Y - 1)
	  {
	    tmp += 50;
	  }

	  if (tmp > score)
	  {
	    score = tmp;
	    bestx = i;
	    besty = j;
	  }
	}
      }
    }	  
  }
  else /* if (level & LEVEL_3) */	/* 三級: 金角銀邊 */
  {
    for (i = 0; i < MAP_X; i++)
    {
      for (j = 0; j < MAP_Y; j++)
      {
	if (tmp = do_count(i, j, TILE_CPU))
	{
	  /* 角 +100  一個邊 +10  一般 +1 */
	  if (i == 0 || i == MAP_X - 1)
	  {
	    if (j == 0 || j == MAP_Y - 1)
	      tmp += 100;
	    else
	      tmp += 10 * count_edge(i, j, TILE_CPU);
	  }
	  else if (j == 0 || j == MAP_Y - 1)
	  {
	    tmp += 10 * count_edge(i, j, TILE_CPU);
	  }

	  if (tmp > score)
	  {
	    score = tmp;
	    bestx = i;
	    besty = j;
	  }
	}
      }
    }	  
  }

  *x = bestx;
  *y = besty;
  return score;
}


/* 設定棋盤 */

static inline void
init_map()
{
  int i, j;

  for (i = 0; i < MAP_X; i++)
  {
    for (j = 0; j < MAP_Y; j++)
    {
      map[i][j] = TILE_BLANK;
    }
  }

  map[MAP_X / 2 - 1][MAP_Y / 2 - 1] = TILE_CPU;
  map[MAP_X / 2][MAP_Y / 2] = TILE_CPU;
  map[MAP_X / 2 - 1][MAP_Y / 2] = TILE_USR;
  map[MAP_X / 2][MAP_Y / 2 - 1] = TILE_USR;
}



/* 螢幕控制 */

static inline void
out_prompt()
{
  /* 不得超過 GRAY_YPOS，否則會錯亂 */
  move(3, 0);
  /* 按鍵說明： */
  outs("\xAB\xF6\xC1\xE4\xBB\xA1\xA9\xFA\xA1\x47");
  move(5, 0);
  /* 移動     方向鍵 */
  outs("\xB2\xBE\xB0\xCA     \xA4\xE8\xA6\x56\xC1\xE4");
  move(6, 0);
  /* 佔領     空白鍵 */
  outs("\xA6\xFB\xBB\xE2     \xAA\xC5\xA5\xD5\xC1\xE4");
  move(7, 0);
  /* 佔領     Enter */
  outs("\xA6\xFB\xBB\xE2     Enter");
  move(8, 0);
  /* 離開     Esc / q */
  outs("\xC2\xF7\xB6\x7D     Esc / q");
  move(10, 0);
  /* 玩家      */
  outs("\xAA\xB1\xAE\x61     ");
  outs(piece[TILE_USR]);
  move(11, 0);
  /* 電腦      */
  outs("\xB9\x71\xB8\xA3     ");
  outs(piece[TILE_CPU]);
}


static inline void
out_song()
{
  uschar *msg[5] = 
  {
    /* 二隻老虎  二隻老虎 */
    "\xA4\x47\xB0\xA6\xA6\xD1\xAA\xEA  \xA4\x47\xB0\xA6\xA6\xD1\xAA\xEA",
    /* 跑得快  跑得快 */
    "\xB6\x5D\xB1\x6F\xA7\xD6  \xB6\x5D\xB1\x6F\xA7\xD6",
    /* 一隻沒有眼睛 */
    "\xA4\x40\xB0\xA6\xA8\x53\xA6\xB3\xB2\xB4\xB7\xFA",
    /* 一隻沒有尾巴 */
    "\xA4\x40\xB0\xA6\xA8\x53\xA6\xB3\xA7\xC0\xA4\xDA",
    /* 真奇怪  真奇怪 */
    "\xAF\x75\xA9\x5F\xA9\xC7  \xAF\x75\xA9\x5F\xA9\xC7"
  };
  move(b_lines - 2, 0);
  prints("\033[1;3%dm%s\033[m", time(0) % 7, msg[time(0) % 5]);
  clrtoeol();
}


static inline void
out_map()
{
  int i, j;

  /* 黑白棋 */
  vs_bar("\xB6\xC2\xA5\xD5\xB4\xD1");

  out_prompt();
  out_song();

  for (i = 0; i < MAP_X; i++)
  {
    move(GRAY_XPOS + i, GRAY_YPOS);
    for (j = 0; j < MAP_Y; j++)
      outs(piece[TILE_BLANK]);
  }

  mouts(MAP_X / 2 - 1, MAP_Y / 2 - 1, TILE_CPU);
  mouts(MAP_X / 2, MAP_Y / 2, TILE_CPU);
  mouts(MAP_X / 2 - 1, MAP_Y / 2, TILE_USR);
  mouts(MAP_X / 2, MAP_Y / 2 - 1, TILE_USR);

  move(GRAY_XPOS + cx, GRAY_YPOS + cy * 2 + 1);	/* move to (0, 0) */
}


/* 遊戲主程式 */

static inline void
result(msg)
  char *msg;
{
  int i, j;
  int sumCPU, sumUSR;

  sumCPU = sumUSR = 0;
  for (i = 0; i < MAP_X; i++)
  {
    for (j = 0; j < MAP_Y; j++)
    {
      if (map[i][j] & TILE_CPU)
	sumCPU++;
      else if(map[i][j] & TILE_USR)
	sumUSR++;
    }
  }

  /* [%s] 玩家：電腦 = %d：%d */
  sprintf(msg, "[%s] \xAA\xB1\xAE\x61\xA1\x47\xB9\x71\xB8\xA3 = %d\xA1\x47%d", 
    /* 勝利 */
    /* 落敗 */
    /* 平手 */
    (sumUSR > sumCPU) ? "\xB3\xD3\xA7\x51" : (sumUSR < sumCPU ? "\xB8\xA8\xB1\xD1" : "\xA5\xAD\xA4\xE2"),
    sumUSR, sumCPU);
}


static inline void
play_reversi(level)
  int level;
{
  int i, j;
  int ch;
  int usr_turn;		/* 1: 該玩家  0: 該電腦 */
  int pass;		/* 0: 沒有人pass  1: 一個人pass  2:連續二個人pass */
  int bestx, besty;	/* 電腦最佳下子處 */

  pass = 0;
  if (!(level & LEVEL_USR_FIRST))
    goto cpu_first;

  while (!EndGame)
  {
    /* 先算玩家還有沒有子可以下 */
    for (i = 0; i < MAP_X; i++)
    {
      for (j = 0; j < MAP_Y; j++)
      {
	if (do_count(i, j, TILE_USR))
	{
	  usr_turn = 1;
	  i = MAP_X;	/* 離開 for 迴圈 */
	  j = MAP_Y;
	}
      }	
    }

    if (!usr_turn)
    {
      pass++;

      /* 檢查是否有二人 passout */
      if (pass == 2)
      {
	EndGame = 1;	/* 遊戲結束 */
	return;
      }
    }
    else if (pass)
    {
      /* 電腦無子可下，輪到您了 */
      vmsg("\xB9\x71\xB8\xA3\xB5\x4C\xA4\x6C\xA5\x69\xA4\x55\xA1\x41\xBD\xFC\xA8\xEC\xB1\x7A\xA4\x46");
      move(b_lines, 0);
      clrtoeol();	/* 消掉 vmsg() */
    }

    while (usr_turn && (ch = vkey()))	/* 該玩家下 */
    {
      switch (ch)
      {
      case KEY_ESC:
      case 'q':
      case 'Q':
	EndGame = -1;
	return;

      case KEY_UP:
	if (cx)
	{
	  cx--;
	  move(GRAY_XPOS + cx, GRAY_YPOS + cy * 2 + 1);
	}
	break;

      case KEY_DOWN:
	if (cx < MAP_X - 1)
	{
	  cx++;
	  move(GRAY_XPOS + cx, GRAY_YPOS + cy * 2 + 1);
	}
        break;

      case KEY_LEFT:
	if (cy)
	{
	  cy--;
	  move(GRAY_XPOS + cx, GRAY_YPOS + cy * 2 + 1);
	}
	break;

      case KEY_RIGHT:
	if (cy < MAP_Y - 1)
	{
	  cy++;
	  move(GRAY_XPOS + cx, GRAY_YPOS + cy * 2 + 1);
	}
	break;

      case '\n':
      case ' ':
	if (do_count(cx, cy, TILE_USR))
	{
          do_eat(cx, cy, TILE_USR);
          usr_turn = 0;
          pass = 0;
	}
	break;

      default:
        break;
      }
    }		/* 玩家下 while 迴圈結束 */

cpu_first:

    /* 該 CPU 下 */
    if (!find_best(&bestx, &besty, level))
    {
      pass++;
    }
    else
    {
      do_eat(bestx, besty, TILE_CPU);
      pass = 0;
      cx = bestx;		/* 移到 CPU 所下的位置 */
      cy = besty;
    }
    move(GRAY_XPOS + cx, GRAY_YPOS + cy * 2 + 1);

    /* 檢查是否有二人 passout */
    if (pass == 2)
      EndGame = 1;	/* 遊戲結束 */

  }	/* while (!EndGame) 迴圈結束 */
}


int
main_reversi()
{
  int level;

  /* 請選擇 1)易如反掌 2)非常簡單 3)普通難度，或按 [Q] 離開： */
  level = vans("\xBD\xD0\xBF\xEF\xBE\xDC 1)\xA9\xF6\xA6\x70\xA4\xCF\xB4\x78 2)\xAB\x44\xB1\x60\xC2\xB2\xB3\xE6 3)\xB4\xB6\xB3\x71\xC3\xF8\xAB\xD7\xA1\x41\xA9\xCE\xAB\xF6 [Q] \xC2\xF7\xB6\x7D\xA1\x47") - '1';
  if (level >= 0 && level <= 2)
  {
    level = LEVEL_1 << level;	/* 設定難度 */
    /* 玩家先下嗎(Y/N)？[Y]  */
    if (vans("\xAA\xB1\xAE\x61\xA5\xFD\xA4\x55\xB6\xDC(Y/N)\xA1\x48[Y] ") != 'n')
      level |= LEVEL_USR_FIRST;
  }
  else
  {
    /* vmsg(MSG_QUITGAME); */	/* itoc.010312: 不要了 */
    return XEASY;
  }

  cx = MAP_X / 2 - 1;
  cy = MAP_Y / 2 - 1;
  EndGame = 0;

  init_map();
  out_map();
  play_reversi(level);

  if (EndGame < 0)
  {
    vmsg(MSG_QUITGAME);
  }
  else
  {
    char buf[60];
    result(buf);
    vmsg(buf);
  }
  return 0;
}
#endif	/* HAVE_GAME */
