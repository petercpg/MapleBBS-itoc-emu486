/*-------------------------------------------------------*/
/* recall.c     ( NTHU CS MapleBBS Ver 3.10 )            */
/*-------------------------------------------------------*/
/* target : Memory Game routines                         */
/* create : 01/07/19                                     */
/* update :   /  /                                       */
/* author : einstein@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/

#include "bbs.h"

#ifdef HAVE_GAME

enum
{
  MG_XPOS = 4,
  MG_YPOS = 4,

  /* MAX_X * MAX_Y 必須是偶數 */
  MAX_X = 10,
  MAX_Y = 10,
};


static int cx, cy;
static int board[MAX_X][MAX_Y], isopen[MAX_X][MAX_Y];
/* Ａ */
/* Ｂ */
/* Ｃ */
/* Ｄ */
/* Ｅ */
/* Ｆ */
/* Ｇ */
/* Ｈ */
/* Ｉ */
/* Ｊ */
static char card[52][3] = {"\xA2\xCF", "\xA2\xD0", "\xA2\xD1", "\xA2\xD2", "\xA2\xD3", "\xA2\xD4", "\xA2\xD5", "\xA2\xD6", "\xA2\xD7", "\xA2\xD8", 
			   /* Ｋ */
			   /* Ｌ */
			   /* Ｍ */
			   /* Ｎ */
			   /* Ｏ */
			   /* Ｐ */
			   /* Ｑ */
			   /* Ｒ */
			   /* Ｓ */
			   /* Ｔ */
			   "\xA2\xD9", "\xA2\xDA", "\xA2\xDB", "\xA2\xDC", "\xA2\xDD", "\xA2\xDE", "\xA2\xDF", "\xA2\xE0", "\xA2\xE1", "\xA2\xE2", 
			   /* Ｕ */
			   /* Ｖ */
			   /* Ｗ */
			   /* Ｘ */
			   /* Ｙ */
			   /* Ｚ */
			   /* ａ */
			   /* ｂ */
			   /* ｃ */
			   /* ｄ */
			   "\xA2\xE3", "\xA2\xE4", "\xA2\xE5", "\xA2\xE6", "\xA2\xE7", "\xA2\xE8", "\xA2\xE9", "\xA2\xEA", "\xA2\xEB", "\xA2\xEC", 
			   /* ｅ */
			   /* ｆ */
			   /* ｇ */
			   /* ｈ */
			   /* ｉ */
			   /* ｊ */
			   /* ｋ */
			   /* ｌ */
			   /* ｍ */
			   /* ｎ */
			   "\xA2\xED", "\xA2\xEE", "\xA2\xEF", "\xA2\xF0", "\xA2\xF1", "\xA2\xF2", "\xA2\xF3", "\xA2\xF4", "\xA2\xF5", "\xA2\xF6", 			   
			   /* ｏ */
			   /* ｐ */
			   /* ｑ */
			   /* ｒ */
			   /* ｓ */
			   /* ｔ */
			   /* ｕ */
			   /* ｖ */
			   /* ｗ */
			   /* ｘ */
			   "\xA2\xF7", "\xA2\xF8", "\xA2\xF9", "\xA2\xFA", "\xA2\xFB", "\xA2\xFC", "\xA2\xFD", "\xA2\xFE", "\xA3\x40", "\xA3\x41", 
			   /* ｙ */
			   /* ｚ */
			   "\xA3\x42", "\xA3\x43"};



static inline void
init_board()
{
  int i, j, rx, ry, temp, count = 0;

  for (i = 0; i < MAX_X; i++)
  {
    for (j = 0; j < MAX_Y; j++)
    {
      board[i][j] = (count++) / 2;
      isopen[i][j] = 0;
    }
  }

  for (i = 0; i < MAX_X; i++)
  {
    for (j = 0; j < MAX_Y; j++)
    {
      rx = rnd(MAX_X);
      ry = rnd(MAX_Y);
      temp = board[i][j];
      board[i][j] = board[rx][ry];
      board[rx][ry] = temp;
    }
  }

  cx = 0;
  cy = 0;
}


static void
show_board()
{
  int i, j;

  /* 記憶遊戲 */
  vs_bar("\xB0\x4F\xBE\xD0\xB9\x43\xC0\xB8");

  for (i = 0; i < MAX_X; i++)
  {
    for (j = 0; j < MAX_Y; j++)
    {
      move(MG_XPOS + i, MG_YPOS + j * 2);
      if (isopen[i][j])
      {
	outs(card[board[i][j]]);
      }
      else
      {
	/* ■ */
	outs("\xA1\xBD");
      }
    }
  }

  move(3, 40);
  /* ↑↓←→         方向鍵 */
  outs("\xA1\xF4\xA1\xF5\xA1\xF6\xA1\xF7         \xA4\xE8\xA6\x56\xC1\xE4");
  move(5, 40);
  /* [Space][Enter]   翻開 */
  outs("[Space][Enter]   \xC2\xBD\xB6\x7D");
  move(7, 40);
  /* Q/q              離開 */
  outs("Q/q              \xC2\xF7\xB6\x7D");

  move(MG_XPOS + cx, MG_YPOS + cy * 2 + 1);
}


static inline int
valid_pos(x, y)
  int x, y;
{
  if (x < 0 || x >= MAX_X || y < 0 || y >= MAX_Y)
  {
    return 0;
  }
  return 1;
}


static void
get_pos(x, y)
  int *x, *y;
{
  char ch;
  while (1)
  {
    ch = vkey();
    if (ch == KEY_UP && valid_pos(cx - 1, cy))
    {
      cx -= 1;
      move(MG_XPOS + cx, MG_YPOS + cy * 2 + 1);
    }
    else if (ch == KEY_DOWN && valid_pos(cx + 1, cy))
    {
      cx += 1;
      move(MG_XPOS + cx, MG_YPOS + cy * 2 + 1);
    }
    else if (ch == KEY_LEFT && valid_pos(cx, cy - 1))
    {
      cy -= 1;
      move(MG_XPOS + cx, MG_YPOS + cy * 2 + 1);
    }
    else if (ch == KEY_RIGHT && valid_pos(cx, cy + 1))
    {
      cy += 1;
      move(MG_XPOS + cx, MG_YPOS + cy * 2 + 1);
    }
    else if (ch == 'q' || ch == 'Q')
    {
      vmsg(MSG_QUITGAME);
      *x = -1;
      break;
    }
    else if (ch == '\n' || ch == ' ')
    {
      *x = cx;
      *y = cy;
      break;
    }
  }
}


int
main_recall()
{
  int fx, fy, sx, sy, count = 0;

  init_board();
  show_board();

  while (1)
  {

    while (1)			/* 第一次 */
    {
      get_pos(&fx, &fy);
      if (fx < 0)
      {
	goto abort_game;
      }
      if (isopen[fx][fy])
      {
	continue;
      }
      move(MG_XPOS + fx, MG_YPOS + 2 * fy);
      outs(card[board[fx][fy]]);
      move(MG_XPOS + fx, MG_YPOS + 2 * fy + 1);
      isopen[fx][fy] = 1;
      break;
    }

    while (1)			/* 第二次 */
    {
      get_pos(&sx, &sy);
      if (sx < 0)
      {
	goto abort_game;
      }
      if (isopen[sx][sy])
      {
	continue;
      }
      move(MG_XPOS + sx, MG_YPOS + 2 * sy);
      outs(card[board[sx][sy]]);
      move(MG_XPOS + sx, MG_YPOS + 2 * sy + 1);
      isopen[sx][sy] = 1;
      if (board[fx][fy] == board[sx][sy])
      {
	count += 2;
      }
      else
      {
	/* 看清楚了沒？ */
	vmsg("\xAC\xDD\xB2\x4D\xB7\xA1\xA4\x46\xA8\x53\xA1\x48");
	move(b_lines, 0);
	clrtoeol();
	move(MG_XPOS + fx, MG_YPOS + 2 * fy);
	/* ■ */
	outs("\xA1\xBD");
	isopen[fx][fy] = 0;
	move(MG_XPOS + sx, MG_YPOS + 2 * sy);
	/* ■ */
	outs("\xA1\xBD");
	move(MG_XPOS + sx, MG_YPOS + 2 * sy + 1);
	isopen[sx][sy] = 0;
      }
      break;
    }

    if (count == MAX_X * MAX_Y)
    {
      /* 恭喜您成功了 */
      vmsg("\xAE\xA5\xB3\xDF\xB1\x7A\xA6\xA8\xA5\x5C\xA4\x46");
      break;
    }

  }
abort_game:
  return 0;
}
#endif	/* HAVE_GAME */
