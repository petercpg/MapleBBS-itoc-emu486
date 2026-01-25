/*-------------------------------------------------------*/
/* tetris.c	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : 俄羅斯方塊					 */
/* create :   /  /  					 */
/* update : 02/10/15					 */
/* author : zhch.bbs@bbs.nju.edu.cn			 */
/* modify : hightman.bbs@bbs.hightman.net		 */
/* recast : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_GAME

#define MAX_MAP_WIDTH	10	/* 棋盤的寬度共 10 行 */
#define MAX_MAP_HEIGHT	20	/* 棋盤的高度共 20 列 */
#define MAX_STYLE	7	/* 總共有 7 種類型的方塊 */


/* 每種方塊都是 4*4 的，然後可以轉 4 個方向 */

static int style_x[MAX_STYLE][4][4] =
{
  0, 1, 0, 1,    0, 1, 0, 1,   0, 1, 0, 1,   0, 1, 0, 1,
  0, 1, 2, 3,    1, 1, 1, 1,   0, 1, 2, 3,   1, 1, 1, 1,
  0, 1, 1, 2,    1, 0, 1, 0,   0, 1, 1, 2,   1, 0, 1, 0,
  1, 2, 0, 1,    0, 0, 1, 1,   1, 2, 0, 1,   0, 0, 1, 1,
  0, 1, 2, 0,    0, 1, 1, 1,   2, 0, 1, 2,   0, 0, 0, 1,
  0, 1, 2, 2,    1, 1, 0, 1,   0, 0, 1, 2,   0, 1, 0, 0,
  0, 1, 2, 1,    1, 0, 1, 1,   1, 0, 1, 2,   0, 0, 1, 0
};

static int style_y[MAX_STYLE][4][4] =
{
  0, 0, 1, 1,    0, 0, 1, 1,   0, 0, 1, 1,   0, 0, 1, 1,
  1, 1, 1, 1,    0, 1, 2, 3,   1, 1, 1, 1,   0, 1, 2, 3,
  0, 0, 1, 1,    0, 1, 1, 2,   0, 0, 1, 1,   0, 1, 1, 2,
  0, 0, 1, 1,    0, 1, 1, 2,   0, 0, 1, 1,   0, 1, 1, 2,
  0, 0, 0, 1,    0, 0, 1, 2,   0, 1, 1, 1,   0, 1, 2, 2,
  0, 0, 0, 1,    0, 1, 2, 2,   0, 1, 1, 1,   0, 0, 1, 2,
  0, 0, 0, 1,    0, 1, 1, 2,   0, 1, 1, 1,   0, 1, 1, 2
};


static int map[MAX_MAP_HEIGHT + 1][MAX_MAP_WIDTH + 2];

static int my_lines;		/* 總消去條數 */
static int my_scores;		/* 總得分 */
static int level;		/* 關卡 */
static int delay;		/* 方塊自動下跌的時間間距 (單位:usec) */
static int cx, cy;		/* 目前所在 (x, y) */
static int style;		/* 目前方塊的類型 */
static int dir;			/* 目前方塊的方向 */
static int last_dir;		/* 上次方向的方向 */


static void
move_map(x, y)
  int x, y;
{
  move(x + 2, 2 * y);
}


static void
tetris_welcome()
{
  move(4, 33);
  /* 按鍵說明： */
  outs("\xAB\xF6\xC1\xE4\xBB\xA1\xA9\xFA\xA1\x47");
  move(5, 35);
  /* ↓←→       移動方向 */
  outs("\xA1\xF5\xA1\xF6\xA1\xF7       \xB2\xBE\xB0\xCA\xA4\xE8\xA6\x56");
  move(6, 35);
  /* Space        快速降下 */
  outs("Space        \xA7\xD6\xB3\x74\xAD\xB0\xA4\x55");
  move(7, 35);
  /* ↑           180 度旋轉 */
  outs("\xA1\xF4           180 \xAB\xD7\xB1\xDB\xC2\xE0");
  move(8, 35);
  /* k            順時針旋轉 */
  outs("k            \xB6\xB6\xAE\xC9\xB0\x77\xB1\xDB\xC2\xE0");
  move(9, 35);
  /* j            逆時針旋轉 */
  outs("j            \xB0\x66\xAE\xC9\xB0\x77\xB1\xDB\xC2\xE0");
  move(10, 35);
  /* ^S           暫停遊戲 */
  outs("^S           \xBC\xC8\xB0\xB1\xB9\x43\xC0\xB8");
  move(11, 35);
  /* q            離開遊戲 */
  outs("q            \xC2\xF7\xB6\x7D\xB9\x43\xC0\xB8");

  move(13, 35);
  /* 若游標有移動錯誤的現象 */
  outs("\xAD\x59\xB4\xE5\xBC\xD0\xA6\xB3\xB2\xBE\xB0\xCA\xBF\xF9\xBB\x7E\xAA\xBA\xB2\x7B\xB6\x48");
  move(14, 35);
  /* 暫時將偵測方向鍵全形關閉即可 */
  outs("\xBC\xC8\xAE\xC9\xB1\x4E\xB0\xBB\xB4\xFA\xA4\xE8\xA6\x56\xC1\xE4\xA5\xFE\xA7\xCE\xC3\xF6\xB3\xAC\xA7\x59\xA5\x69");

  move(16, 33);
  /* 每消３０條升一級 */
  outs("\xA8\x43\xAE\xF8\xA2\xB2\xA2\xAF\xB1\xF8\xA4\xC9\xA4\x40\xAF\xC5");
  move(17, 33);
  /* 消一行得１分、二行３分、三行７分、四行１５分 */
  outs("\xAE\xF8\xA4\x40\xA6\xE6\xB1\x6F\xA2\xB0\xA4\xC0\xA1\x42\xA4\x47\xA6\xE6\xA2\xB2\xA4\xC0\xA1\x42\xA4\x54\xA6\xE6\xA2\xB6\xA4\xC0\xA1\x42\xA5\x7C\xA6\xE6\xA2\xB0\xA2\xB4\xA4\xC0");

  vmsg(NULL);
}


static void
tetris_init()		/* initialize map[][] */
{
  int i, j, line;

  line = MAX_MAP_HEIGHT - level;	/* 依等級來決定有幾行有東西 */

  /* map[][] 裡面的值 0:空白(  ) 1:佔滿(█) 2:左右界(│) 3:底界(─) 4:左角落界(└) 5:右角落界(┘) */

  for (i = 0; i < MAX_MAP_HEIGHT; i++)
  {
    for (j = 1; j <= MAX_MAP_WIDTH; j++)
    {
      if (i >= line)
	map[i][j] = rnd(2);			/* 佔滿 */
      else
	map[i][j] = 0;				/* 空白 */
    }
    map[i][0] = map[i][MAX_MAP_WIDTH + 1] = 2;	/* 左右界 */
  }

  for (j = 1; j <= MAX_MAP_WIDTH; j++)
    map[MAX_MAP_HEIGHT][j] = 3;			/* 底界 */

  map[MAX_MAP_HEIGHT][0] = 4;			/* 左角落界 */
  map[MAX_MAP_HEIGHT][MAX_MAP_WIDTH + 1] = 5;	/* 左角落界 */
}


static void
tetris_mapshow()
{
  int i, j;
  /* █ */
  /* │ */
  /* ─ */
  /* └ */
  /* ┘ */
  char piece[6][3] = {"  ", "\xA2\x69", "\xA2\x78", "\xA2\x77", "\xA2\x7C", "\xA2\x7D"};

  /* map[][] 裡面的值 0:空白(  ) 1:佔滿(█) 2:左右界(│) 3:底界(─) 4:左角落界(└) 5:右角落界(┘) */

  for (i = 0; i <= MAX_MAP_HEIGHT; i++)
  {
    move_map(i, 0);
    for (j = 0; j <= MAX_MAP_WIDTH + 1; j++)
      outs(piece[map[i][j]]);
  }
}


static void
tetris_blineshow()
{
  move(b_lines, 0);
  clrtoeol();
  /* 等級：\033[1;32m%d\033[m  總消去條數：\033[1;32m%d\033[m  總得分：\033[1;32m%d\033[m */
  prints("\xB5\xA5\xAF\xC5\xA1\x47\033[1;32m%d\033[m  \xC1\x60\xAE\xF8\xA5\x68\xB1\xF8\xBC\xC6\xA1\x47\033[1;32m%d\033[m  \xC1\x60\xB1\x6F\xA4\xC0\xA1\x47\033[1;32m%d\033[m",
    level, my_lines, my_scores);
}


static void
block_show(x, y, s, d, f)
  int x, y;		/* (x, y) */
  int s;		/* style */
  int d;		/* dir */
  int f;		/* 1:加上方塊 0:移除方塊 */
{
  int n;

  if (d == -1)
    return;

  for (n = 0; n <= 3; n++)
  {
    move_map(x + style_x[s][d][n], y + style_y[s][d][n]);
    if (f)
      /* █ */
      outs("\xA2\x69");	/* piece[1] */
    else
      outs("  ");	/* piece[0] */
  }
  move(1, 0);
}


static void
block_move()
{
  static int last_x = -1;
  static int last_y = -1;
  static int last_style = -1;

  if (last_x == cx && last_y == cy && last_style == style && last_dir == dir)
    return;

  block_show(last_x, last_y, last_style, last_dir, 0);
  last_x = cx;
  last_y = cy;
  last_style = style;
  last_dir = dir;
  block_show(cx, cy, style, dir, 1);
}


static void
tune_delay()
{
  /* delay 要在 1 ~ 999999 之間 */
  delay = 999999 / (level + 1);
}


static void
check_lines()		/* 檢查看是否能消去一條 */
{
  int i, j, s;

  s = 1;
  for (i = 0; i < MAX_MAP_HEIGHT; i++)
  {
    for (j = 1; j <= MAX_MAP_WIDTH; j++)
    {
      if (map[i][j] == 0)	/* 空白 */
	break;
    }

    if (j == MAX_MAP_WIDTH + 1)
    {
      int n;

      s *= 2;
      /* 消去條上部全部下移 */
      for (n = i; n > 0; n--)
      {
	for (j = 1; j <= MAX_MAP_WIDTH; j++)
	  map[n][j] = map[n - 1][j];
      }
      for (j = 1; j <= MAX_MAP_WIDTH; j++)
	map[0][j] = 0;

      if (++my_lines % 30 == 0)	/* 每消 30 條關卡加一 */
      {
	level++;
	tune_delay();
      }
    }
  }

  if (--s > 0)	/* 有消去至少一條 */
  {
    s = s * (10 + level) / 10;
    my_scores += s;
    tetris_mapshow();
    tetris_blineshow();
  }
}


static int	/* 1:碰到障礙物 0:通行無阻 */
crash(x, y, s, d)
  int x, y;	/* (x, y) */
  int s;	/* style */
  int d;	/* dir */
{
  int n;

  for (n = 0; n <= 3; n++)
  {
    if (map[x + style_x[s][d][n]][y + style_y[s][d][n]])	/* 已佔滿 */
      return 1;
  }
  return 0;
}


static void
arrived()
{
  int n;

  for (n = 0; n <= 3; n++)
    map[cx + style_x[style][dir][n]][cy + style_y[style][dir][n]] = 1;	/* 佔滿 */

  check_lines();
}


static int
getkey()
{
  int fd;
  struct timeval tv;

  fd = 1;
  tv.tv_sec = 0;
  tv.tv_usec = delay;

  /* 若有按鍵，回傳所按的鍵；若 delay 的時間到了仍沒有按鍵，回傳 0 */

  if (select(1, (fd_set *) &fd, NULL, NULL, &tv) > 0)
    return vkey();

  return 0;
}


int
main_tetris()
{
  int ch;
  int next_style;

  /* 俄羅斯方塊 */
  vs_bar("\xAB\x58\xC3\xB9\xB4\xB5\xA4\xE8\xB6\xF4");
  tetris_welcome();

start_game:

  /* 從第幾級開始玩(0-9)？[0]  */
  level = vans("\xB1\x71\xB2\xC4\xB4\x58\xAF\xC5\xB6\x7D\xA9\x6C\xAA\xB1(0-9)\xA1\x48[0] ") - '0';
  if (level < 0 || level > 9)
    level = 0;

  tetris_init();
  tetris_mapshow();

  /* 遊戲開始 */
  vmsg("\xB9\x43\xC0\xB8\xB6\x7D\xA9\x6C");
  tetris_blineshow();

  style = 0;
  next_style = rnd(MAX_STYLE);
  my_lines = my_scores = 0;
  tune_delay();

  while (1)
  {
    style = next_style;
    next_style = rnd(MAX_STYLE);/* 亂數決定下一個出來的方塊類型 */
    last_dir = -1;		/* 每次新方塊出來都要重設為 -1 */
    dir = 0;			/* 方塊一出來是朝上的 */
    cx = 0;			/* 方塊一出來的位置 */
    cy = MAX_MAP_WIDTH / 2;

    /* 把上一個已出來的清除，把下一個要出來的畫在右上角 */
    block_show(0, MAX_MAP_WIDTH + 2, style, dir, 0);
    block_show(0, MAX_MAP_WIDTH + 2, next_style, dir, 1);

    block_move();

    if (crash(cx, cy, style, dir))	/* 新方塊一出來就 crash，game over */
      break;

    for (;;)
    {
      switch (ch = getkey())
      {
      case Ctrl('S'):
	/* 遊戲暫停，按任意鍵繼續玩 */
	vmsg("\xB9\x43\xC0\xB8\xBC\xC8\xB0\xB1\xA1\x41\xAB\xF6\xA5\xF4\xB7\x4E\xC1\xE4\xC4\x7E\xC4\xF2\xAA\xB1");
	tetris_blineshow();
	break;

      case KEY_LEFT:
	if (!crash(cx, cy - 1, style, dir))
	{
	  cy--;
	  block_move();
	}
	break;

      case KEY_RIGHT:
	if (!crash(cx, cy + 1, style, dir))
	{
	  cy++;
	  block_move();
	}
	break;

      case KEY_DOWN:
	if (!crash(cx + 1, cy, style, dir))
	{
	  cx++;
	  block_move();
	}
	else
	{
	  arrived();
	  ch = '#';
	}
	break;

      case KEY_UP:
	if (!crash(cx, cy, style, (dir + 2) % 4))
	{
	  dir = (dir + 2) % 4;
	  block_move();
	}
	break;

      case 'k':
	if (!crash(cx, cy, style, (dir + 1) % 4))
	{
	  dir = (dir + 1) % 4;
	  block_move();
	}
	break;

      case 'j':
	if (!crash(cx, cy, style, (dir + 3) % 4))
	{
	  dir = (dir + 3) % 4;
	  block_move();
	}
	break;

      case ' ':
	while (!crash(cx + 1, cy, style, dir))
	  cx++;
	block_move();
	arrived();
	ch = '#';
	break;

      case 0:		/* 隨 delay 時間一到會自動往下跳 */
	if (!crash(cx + 1, cy, style, dir))
	{
	  cx++;
	  block_move();
	}
	else
	{
	  arrived();
	  ch = '#';
	  break;
	}
	break;
      }

      if (ch == 'q' || ch == '#')
	break;

      refresh();
    }		/* end of for (;;) */

    if (ch == 'q')
      break;
  }		/* end of while (1) */

  /* 本局結束！您還要繼續玩嗎(Y/N)？[N]  */
  if (vans("\xA5\xBB\xA7\xBD\xB5\xB2\xA7\xF4\xA1\x49\xB1\x7A\xC1\xD9\xAD\x6E\xC4\x7E\xC4\xF2\xAA\xB1\xB6\xDC(Y/N)\xA1\x48[N] ") == 'y')
    goto start_game;

  return 0;
}
#endif	/* HAVE_GAME */
