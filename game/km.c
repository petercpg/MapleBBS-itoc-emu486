/*-------------------------------------------------------*/
/* game/km.c            ( NTHU CS MapleBBS Ver 3.10 )    */
/*-------------------------------------------------------*/
/* target : KongMing Chess routines                      */
/* create : 01/02/08                                     */
/* update : 01/05/09                                     */
/* author : einstein@bbs.tnfsh.tn.edu.tw                 */
/* recast : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"


#ifdef HAVE_GAME

#define RETRACT_CHESS	/* 是否提供悔棋功能 */

#ifdef RETRACT_CHESS
#  define LOG_KM	/* 是否提供記錄棋譜的功能 */
#endif


#if 0

棋盤在 etc/game/km 格式如下：

第一行放總共有幾盤棋盤(三位數)，從第三行開始則是一盤一盤的棋譜。

TILE_NOUSE 0 表示不能移動的格子
TILE_BLANK 1 表示空格
TILE_CHESS 2 表示棋子

#123混一宇內
0 0 2 2 2 0 0
0 0 2 2 2 0 0
2 2 2 2 2 2 2
2 2 2 1 2 2 2
2 2 2 2 2 2 2
0 0 2 2 2 0 0
0 0 2 2 2 0 0

#endif


enum
{
  KM_XPOS = 5,
  KM_YPOS = 5,
  MAX_X = 7,			/* 要是奇數 */
  MAX_Y = 7,			/* 要是奇數 */

  /* 用 bitwise operators & 來取代 == */
  TILE_NOUSE = 0,		/* 不能移動的格子 */
  TILE_BLANK = 1,		/* 空格 */
  TILE_CHESS = 2		/* 棋子 */
};


static int board[MAX_X][MAX_Y];
#ifdef LOG_KM
static int origin_board[MAX_X][MAX_Y];
#endif
static int cx, cy;
static int stage, NUM_TABLE;
/* 　 */
/* ○ */
/* ● */
/* ☆ */
static char piece[4][3] = {"\xA1\x40", "\xA1\xB3", "\xA1\xB4", "\xA1\xB8"};
static char title[20];		/* 棋譜名稱 */

#ifdef RETRACT_CHESS
static int route[MAX_X * MAX_Y][4];	/* 記錄 (fx, fy) -> (tx, ty)，悔棋步數不可能超過棋盤大小 */
static int step;
#endif


static void
out_song()
{
  /* itoc.註解: 每句話都弄成一樣長度，就不用 clrtoeol() :p */
  uschar *msg[8] = 
  {
    /* 您太強了，就是這樣！ */
    "\xB1\x7A\xA4\xD3\xB1\x6A\xA4\x46\xA1\x41\xB4\x4E\xAC\x4F\xB3\x6F\xBC\xCB\xA1\x49",
    /* 您怎麼可能想到這一步 */
    "\xB1\x7A\xAB\xE7\xBB\xF2\xA5\x69\xAF\xE0\xB7\x51\xA8\xEC\xB3\x6F\xA4\x40\xA8\x42",
    /* 這真是太神奇了，傑克 */
    "\xB3\x6F\xAF\x75\xAC\x4F\xA4\xD3\xAF\xAB\xA9\x5F\xA4\x46\xA1\x41\xB3\xC7\xA7\x4A",
    /* 我不知道該說些什麼了 */
    "\xA7\xDA\xA4\xA3\xAA\xBE\xB9\x44\xB8\xD3\xBB\xA1\xA8\xC7\xA4\xB0\xBB\xF2\xA4\x46",
    /* 這一著真是天人手筆呀 */
    "\xB3\x6F\xA4\x40\xB5\xDB\xAF\x75\xAC\x4F\xA4\xD1\xA4\x48\xA4\xE2\xB5\xA7\xA7\x72",
    /* 太佩服您了，這樣也行 */
    "\xA4\xD3\xA8\xD8\xAA\x41\xB1\x7A\xA4\x46\xA1\x41\xB3\x6F\xBC\xCB\xA4\x5D\xA6\xE6",
    /* 快完成了！加油加油！ */
    "\xA7\xD6\xA7\xB9\xA6\xA8\xA4\x46\xA1\x49\xA5\x5B\xAA\x6F\xA5\x5B\xAA\x6F\xA1\x49",
    /* 好的棋譜要告訴站長喔 */
    "\xA6\x6E\xAA\xBA\xB4\xD1\xC3\xD0\xAD\x6E\xA7\x69\xB6\x44\xAF\xB8\xAA\xF8\xB3\xE1"
  };
  move(21, 0);
  prints("\033[1;3%dm%s\033[m", time(0) % 7, msg[time(0) % 8]);
}


static void
show_board()
{
  int i, j;

  /* 孔明棋 */
  vs_bar("\xA4\xD5\xA9\xFA\xB4\xD1");
  move(2, KM_YPOS + MAX_Y - 6);		/* 置中顯示棋譜名稱 */
  outs(title);

  for (i = 0; i < MAX_X; i++)
  {
    for (j = 0; j < MAX_Y; j++)
    {
      move(KM_XPOS + i, KM_YPOS + j * 2);
      outs(piece[board[i][j]]);
    }
  }

  move(3, 40);
  /* ↑↓←→  方向鍵 */
  outs("\xA1\xF4\xA1\xF5\xA1\xF6\xA1\xF7  \xA4\xE8\xA6\x56\xC1\xE4");
  move(5, 40);
  /* [Enter]   選取/反選取 */
  outs("[Enter]   \xBF\xEF\xA8\xFA/\xA4\xCF\xBF\xEF\xA8\xFA");
  move(7, 40);
  /* Q/q       離開 */
  outs("Q/q       \xC2\xF7\xB6\x7D");
  move(9, 40);
  /* h         讀取棋譜範例 */
  outs("h         \xC5\xAA\xA8\xFA\xB4\xD1\xC3\xD0\xBD\x64\xA8\xD2");

#ifdef RETRACT_CHESS
  move(11, 40);
  /* r         悔棋 */
  outs("r         \xAE\xAC\xB4\xD1");
#endif

  move(13, 40);
  /* ○        空位 */
  outs("\xA1\xB3        \xAA\xC5\xA6\xEC");
  move(14, 40);
  /* ●        棋子 */
  outs("\xA1\xB4        \xB4\xD1\xA4\x6C");
  move(15, 40);
  /* ☆        選取 */
  outs("\xA1\xB8        \xBF\xEF\xA8\xFA");

  out_song();
  move(KM_XPOS + MAX_X / 2, KM_YPOS + MAX_Y / 2 * 2 + 1);	/* 一開始將游標置中 */
}


static int
read_board()
{
  int i, j, count;
  FILE *fp;
  char buf[40], ans[4];

  if (!(fp = fopen("etc/game/km", "r")))
    return 0;

  if (stage < 0)	/* 第一次進入遊戲 */
  {
    fgets(buf, 4, fp);
    NUM_TABLE = atoi(buf);	/* etc/game/km 第一行記錄棋譜數 */
    /* 請選擇編號 [1-%d]，[0] 隨機出題，或按 [Q] 離開： */
    sprintf(buf, "\xBD\xD0\xBF\xEF\xBE\xDC\xBD\x73\xB8\xB9 [1-%d]\xA1\x41[0] \xC0\x48\xBE\xF7\xA5\x58\xC3\x44\xA1\x41\xA9\xCE\xAB\xF6 [Q] \xC2\xF7\xB6\x7D\xA1\x47", NUM_TABLE);
    if (vget(b_lines, 0, buf, ans, 4, DOECHO) == 'q')
    {  
      fclose(fp);
      return 0;
    }  

    stage = atoi(ans) - 1;
    if (stage < 0 || stage >= NUM_TABLE)	/* 隨機出題 */
      stage = time(0) % NUM_TABLE;
  }

  fseek(fp, 4 + stage * (2 * MAX_X * MAX_Y + 14), SEEK_SET);
  /* 4: 第一行的三位數棋譜數目\n  14: \n#999棋譜名稱\n */

  fscanf(fp, "%s", &title);		/* 棋譜名稱 */

  count = 0;
  for (i = 0; i < MAX_X; i++)
  {
    for (j = 0; j < MAX_Y; j++)
    {
      fscanf(fp, "%d", &board[i][j]);
      if (board[i][j] & TILE_CHESS)
      {
	count++;
      }
    }
  }
  fclose(fp);

#ifdef LOG_KM
  memcpy(origin_board, board, sizeof(origin_board));
#endif

  return count;
}


static inline int
valid_pos(x, y)
  int x, y;
{
  if (x < 0 || x >= MAX_X || y < 0 || y >= MAX_Y || 
    board[x][y] == TILE_NOUSE)	/* TILE_NOUSE = 0 不能用 & operation */
  {
    return 0;
  }
  return 1;
}


static void
get_pos(x, y)
  int *x, *y;
{
  int ch;
  while (1)
  {
    ch = vkey();
    if (ch == KEY_UP && valid_pos(cx - 1, cy))
    {
      cx--;
      move(KM_XPOS + cx, KM_YPOS + cy * 2 + 1);
    }
    else if (ch == KEY_DOWN && valid_pos(cx + 1, cy))
    {
      cx++;
      move(KM_XPOS + cx, KM_YPOS + cy * 2 + 1);
    }
    else if (ch == KEY_LEFT && valid_pos(cx, cy - 1))
    {
      cy--;
      move(KM_XPOS + cx, KM_YPOS + cy * 2 + 1);
    }
    else if (ch == KEY_RIGHT && valid_pos(cx, cy + 1))
    {
      cy++;
      move(KM_XPOS + cx, KM_YPOS + cy * 2 + 1);
    }
    else if (ch == 'h')
    {
      more("etc/game/km.hlp", NULL);
      show_board();
      move(KM_XPOS + cx, KM_YPOS + cy * 2 + 1);
    }
    else if (ch == 'q' || ch == 'Q')
    {
      vmsg(MSG_QUITGAME);
      *x = -1;
      break;
    }
    else if (ch == '\n')
    {
      *x = cx;
      *y = cy;
      break;
    }
#ifdef RETRACT_CHESS
    else if (ch == 'r')
    {
      *x = -2;
      break;
    }
#endif
  }
}


static inline void
jump(fx, fy, tx, ty)
  int fx, fy, tx, ty;		/* From (fx, fy) To (tx, ty) */
{
  out_song();

  board[fx][fy] = TILE_BLANK;
  move(KM_XPOS + fx, KM_YPOS + fy * 2);
  outs(piece[1]);

  board[(fx + tx) / 2][(fy + ty) / 2] = TILE_BLANK;
  move(KM_XPOS + (fx + tx) / 2, KM_YPOS + (fy + ty));
  outs(piece[1]);

  board[tx][ty] = TILE_CHESS;
  move(KM_XPOS + tx, KM_YPOS + ty * 2);
  outs(piece[2]);
  move(KM_XPOS + tx, KM_YPOS + ty * 2 + 1);

#ifdef RETRACT_CHESS
  route[step][0] = fx;
  route[step][1] = fy;
  route[step][2] = tx;
  route[step][3] = ty;
  step++;
#endif
}


#ifdef RETRACT_CHESS
static inline void
retract()
{
  int fx, fy, tx, ty;

  out_song();

  step--; 
  ty = route[step][3];
  tx = route[step][2];
  fy = route[step][1];
  fx = route[step][0];

  board[tx][ty] = TILE_BLANK;
  move(KM_XPOS + tx, KM_YPOS + ty * 2);
  outs(piece[1]);

  board[(fx + tx) / 2][(fy + ty) / 2] = TILE_CHESS;
  move(KM_XPOS + (fx + tx) / 2, KM_YPOS + (fy + ty));
  outs(piece[2]);

  board[fx][fy] = TILE_CHESS;
  move(KM_XPOS + fx, KM_YPOS + fy * 2);
  outs(piece[2]);
  move(KM_XPOS + fx, KM_YPOS + fy * 2);
  cx = fx;
  cy = fy;
}
#endif


static inline int
check(fx, fy, tx, ty)
  int fx, fy, tx, ty;
{
  if ((board[(fx + tx) / 2][(fy + ty) / 2] & TILE_CHESS) &&
    ((abs(fx - tx) == 2 && fy == ty) || (fx == tx && abs(fy - ty) == 2)))
  {
    return 1;
  }
  return 0;
}


static inline int
live()
{
  int dir[4][2] = {1, 0, -1, 0, 0, 1, 0, -1};
  int i, j, k, nx, ny, nx2, ny2;
  for (i = 0; i < MAX_X; i++)
  {
    for (j = 0; j < MAX_Y; j++)
    {
      for (k = 0; k < 4; k++)
      {
	nx = i + dir[k][0];
	ny = j + dir[k][1];
	nx2 = nx + dir[k][0];
	ny2 = ny + dir[k][1];
	if (valid_pos(nx2, ny2) && (board[i][j] & TILE_CHESS) && 
	  (board[nx][ny] & TILE_CHESS) && (board[nx2][ny2] & TILE_BLANK))
	{
	  return 1;
	}
      }
    }
  }
  return 0;
}


#ifdef LOG_KM
static void
log_km()
{
  int s, i, j;
  int fx, fy, tx, ty;
  char fpath[64], buf[80];
  FILE *fp;

  usr_fpath(fpath, cuser.userid, "km.log");
  fp = fopen(fpath, "w");
  fprintf(fp, "%s %s (%s)\n", str_author1, cuser.userid, cuser.username);
  /* 標題: 孔明棋譜 %s 破解過程\n時間: %s\n\n */
  fprintf(fp, "\xBC\xD0\xC3\x44: \xA4\xD5\xA9\xFA\xB4\xD1\xC3\xD0 %s \xAF\x7D\xB8\xD1\xB9\x4C\xB5\x7B\n\xAE\xC9\xB6\xA1: %s\n\n", title, Now());
  fprintf(fp, "%s\n\n", title);

  memcpy(board, origin_board, sizeof(board));
  fx = fy = tx = ty = -1;

  for (s = 0;; s++)
  {
    for (i = 0; i < MAX_X; i++)
    {
      for (j = 0; j < MAX_Y; j++)
      {
#if 0	/* 加顏色好像沒比較清楚 */
	fprintf(fp, "%s%s%s", 
	  (i == fx && j == fy) ? "\033[1;43m" : (i == tx && j == ty) ? "\033[1;33m" : "", 
	  piece[board[i][j]], 
	  (i == fx && j == fy) || (i == tx && j == ty) ? str_ransi : "");
#endif
	fprintf(fp, "%s", piece[board[i][j]]);
      }
      fprintf(fp, "\n");
    }
    fprintf(fp, "\n");

    if (s >= step)
      break;

    fx = route[s][0];
    fy = route[s][1];
    tx = route[s][2];
    ty = route[s][3];
    board[fx][fy] = TILE_BLANK;
    board[(fx + tx) / 2][(fy + ty) / 2] = TILE_BLANK;
    board[tx][ty] = TILE_CHESS;
  }

  ve_banner(fp, 0);
  fclose(fp);

  /* 孔明棋譜 %s 破解過程 */
  sprintf(buf, "\xA4\xD5\xA9\xFA\xB4\xD1\xC3\xD0 %s \xAF\x7D\xB8\xD1\xB9\x4C\xB5\x7B", title);
  mail_self(fpath, cuser.userid, buf, MAIL_READ);

  unlink(fpath);
}
#endif


int
main_km()
{
  int fx, fy, tx, ty, count;

  stage = -1;

start_game:

  if (!(count = read_board()))
    return 0;

#ifdef RETRACT_CHESS
  step = 0;
#endif

  show_board();
  cx = MAX_X / 2;
  cy = MAX_Y / 2;

  while (1)
  {
    if (count == 1 && board[MAX_X / 2][MAX_Y / 2] & TILE_CHESS)
    {			/* 最後一子要在正中間 */
      /* 恭喜您成功了 */
      vmsg("\xAE\xA5\xB3\xDF\xB1\x7A\xA6\xA8\xA5\x5C\xA4\x46");

#ifdef LOG_KM
      /* 您是否要把完成的棋譜保存在信箱中(Y/N)？[Y]  */
      if (vans("\xB1\x7A\xAC\x4F\xA7\x5F\xAD\x6E\xA7\xE2\xA7\xB9\xA6\xA8\xAA\xBA\xB4\xD1\xC3\xD0\xAB\x4F\xA6\x73\xA6\x62\xAB\x48\xBD\x63\xA4\xA4(Y/N)\xA1\x48[Y] ") != 'n')
	log_km();
#endif

      /* 請選擇 1)繼續下一關 2)重新挑戰此關 Q)離開？[1]  */
      switch (vans("\xBD\xD0\xBF\xEF\xBE\xDC 1)\xC4\x7E\xC4\xF2\xA4\x55\xA4\x40\xC3\xF6 2)\xAD\xAB\xB7\x73\xAC\x44\xBE\xD4\xA6\xB9\xC3\xF6 Q)\xC2\xF7\xB6\x7D\xA1\x48[1] "))
      {
      case 'q':
        goto abort_game;
      case '2':
        stage--;
      default:
        if (++stage >= NUM_TABLE)
          stage = 0;
        goto start_game;
      }
    }
    if (!live())
    {
      /* 糟糕...沒棋了...@@ */
      vmsg("\xC1\x56\xBF\x7C...\xA8\x53\xB4\xD1\xA4\x46...@@");

      /* 請選擇 1)繼續下一關 2)重新挑戰此關 Q)離開？[2]  */
      switch (vans("\xBD\xD0\xBF\xEF\xBE\xDC 1)\xC4\x7E\xC4\xF2\xA4\x55\xA4\x40\xC3\xF6 2)\xAD\xAB\xB7\x73\xAC\x44\xBE\xD4\xA6\xB9\xC3\xF6 Q)\xC2\xF7\xB6\x7D\xA1\x48[2] "))
      {
      case 'q':
        goto abort_game;
      case '1':
        if (++stage >= NUM_TABLE)
          stage = 0;
      default:
        goto start_game;
      }
    }

    while (1)		/* 第一次 */
    {
      get_pos(&fx, &fy);
      if (fx < 0)
      {
#ifdef RETRACT_CHESS
	if (fx == -2)
	{
	  if (step)	/* 一步都還沒走，不能悔棋 */
	  {
	    retract();
	    count++;
	  }
	  continue;
	}
#endif
	goto abort_game;
      }
      if (!(board[fx][fy] & TILE_CHESS))
      {
	continue;
      }
      else		/* 選子 */
      {
	move(KM_XPOS + fx, KM_YPOS + fy * 2);
	outs(piece[3]);
	move(KM_XPOS + fx, KM_YPOS + fy * 2 + 1);
	break;
      }
    }

    while (1)		/* 第二次 */
    {
      get_pos(&tx, &ty);
      if (tx < 0)
      {
#ifdef RETRACT_CHESS
	if (tx == -2)
	{
	  continue;	/* 要取消選子才能悔棋 */
	}
#endif
	goto abort_game;
      }
      if (fx == tx && fy == ty)	/* 放棄選子 */
      {
	move(KM_XPOS + tx, KM_YPOS + ty * 2);
	outs(piece[2]);
	move(KM_XPOS + tx, KM_YPOS + ty * 2 + 1);
	break;
      }
      else if (!(board[tx][ty] & TILE_BLANK) || !check(fx, fy, tx, ty))	/* 選跳的地方不能跳 */
      {
	continue;
      }
      else		/* 跳到該地方 */
      {
	jump(fx, fy, tx, ty);
	count--;
	break;
      }
    }
  }
abort_game:
  return 0;
}

#endif	/* HAVE_GAME */
