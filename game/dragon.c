/*-------------------------------------------------------*/
/* dragon.c	( YZU WindTopBBS Ver 3.00 )		 */
/*-------------------------------------------------------*/
/* target : 接龍遊戲					 */
/* create : 01/01/12					 */
/* update : 03/07/23					 */
/* author : verit.bbs@bbs.yzu.edu.tw			 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_GAME

static int cards[52];
static int seven[7];		/* 每堆牌墩各還有幾張牌 */
static int points[7];


/*-------------------------------------------------------*/
/* 畫出牌						 */
/*-------------------------------------------------------*/


static void
draw_card(x, y, card)
  int x, y;
  int card;	/* >=0:印出整張牌及此張牌的號碼  -1:只印牌外殼 */
{
  /* Ｃ */
  /* Ｄ */
  /* Ｈ */
  /* Ｓ */
  char flower[4][3] = {"\xA2\xD1", "\xA2\xD2", "\xA2\xD6", "\xA2\xE1"};
  /* Ａ */
  /* ２ */
  /* ３ */
  /* ４ */
  /* ５ */
  /* ６ */
  /* ７ */
  /* ８ */
  /* ９ */
  /* Ｔ */
  /* Ｊ */
  /* Ｑ */
  /* Ｋ */
  char number[13][3] = {"\xA2\xCF", "\xA2\xB1", "\xA2\xB2", "\xA2\xB3", "\xA2\xB4", "\xA2\xB5", "\xA2\xB6", "\xA2\xB7", "\xA2\xB8", "\xA2\xE2", "\xA2\xD8", "\xA2\xDF", "\xA2\xD9"};

  move(x, y);
  /* ╭───╮ */
  outs("\xA2\x7E\xA2\x77\xA2\x77\xA2\x77\xA2\xA1");

  if (card < 0)
    return;

  move(x + 1, y);
  /* │%s　　│ */
  prints("\xA2\x78%s\xA1\x40\xA1\x40\xA2\x78", number[card % 13]);
  move(x + 2, y);
  /* │%s　　│ */
  prints("\xA2\x78%s\xA1\x40\xA1\x40\xA2\x78", flower[card % 4]);
  move(x + 3, y);
  /* │　　　│ */
  outs("\xA2\x78\xA1\x40\xA1\x40\xA1\x40\xA2\x78");
  move(x + 4, y);
  /* │　　　│ */
  outs("\xA2\x78\xA1\x40\xA1\x40\xA1\x40\xA2\x78");
  move(x + 5, y);
  /* ╰───╯ */
  outs("\xA2\xA2\xA2\x77\xA2\x77\xA2\x77\xA2\xA3");
}


/*-------------------------------------------------------*/
/* 遊戲說明						 */
/*-------------------------------------------------------*/


static int
draw_explain()
{
  /* 接龍遊戲 */
  vs_bar("\xB1\xB5\xC0\x73\xB9\x43\xC0\xB8");

  move(4, 10);
  /* 【遊戲說明】 */
  outs("\xA1\x69\xB9\x43\xC0\xB8\xBB\xA1\xA9\xFA\xA1\x6A");
  move(6, 15);
  /* (1) 在螢幕上方為牌墩，可以利用 ←、→ 切換。 */
  outs("(1) \xA6\x62\xBF\xC3\xB9\xF5\xA4\x57\xA4\xE8\xAC\xB0\xB5\x50\xBC\x5B\xA1\x41\xA5\x69\xA5\x48\xA7\x51\xA5\xCE \xA1\xF6\xA1\x42\xA1\xF7 \xA4\xC1\xB4\xAB\xA1\x43");
  move(8, 15);
  /* (2) 在螢幕左下方為持牌 , 可以利用 c 切換。 */
  outs("(2) \xA6\x62\xBF\xC3\xB9\xF5\xA5\xAA\xA4\x55\xA4\xE8\xAC\xB0\xAB\xF9\xB5\x50 , \xA5\x69\xA5\x48\xA7\x51\xA5\xCE c \xA4\xC1\xB4\xAB\xA1\x43");
  move(10, 15);
  /* (3) 當牌墩的牌是持牌的下一張或上一張，即可利用 Enter 吃牌。 */
  outs("(3) \xB7\xED\xB5\x50\xBC\x5B\xAA\xBA\xB5\x50\xAC\x4F\xAB\xF9\xB5\x50\xAA\xBA\xA4\x55\xA4\x40\xB1\x69\xA9\xCE\xA4\x57\xA4\x40\xB1\x69\xA1\x41\xA7\x59\xA5\x69\xA7\x51\xA5\xCE Enter \xA6\x59\xB5\x50\xA1\x43");
  move(11, 15);
  /*    (只看點數，不看花色) */
  outs("   (\xA5\x75\xAC\xDD\xC2\x49\xBC\xC6\xA1\x41\xA4\xA3\xAC\xDD\xAA\xE1\xA6\xE2)");
  move(13, 15);
  /* (4) 當牌墩的牌都吃完，及遊戲獲勝。 */
  outs("(4) \xB7\xED\xB5\x50\xBC\x5B\xAA\xBA\xB5\x50\xB3\xA3\xA6\x59\xA7\xB9\xA1\x41\xA4\xCE\xB9\x43\xC0\xB8\xC0\xF2\xB3\xD3\xA1\x43");
  move(15, 15);
  /* (5) 當持牌切換完且尚未吃完牌墩的牌，即遊戲失敗。 */
  outs("(5) \xB7\xED\xAB\xF9\xB5\x50\xA4\xC1\xB4\xAB\xA7\xB9\xA5\x42\xA9\x7C\xA5\xBC\xA6\x59\xA7\xB9\xB5\x50\xBC\x5B\xAA\xBA\xB5\x50\xA1\x41\xA7\x59\xB9\x43\xC0\xB8\xA5\xA2\xB1\xD1\xA1\x43");
  vmsg(NULL);
}


/*-------------------------------------------------------*/
/* 畫出遊戲牌的配置					 */
/*-------------------------------------------------------*/


static void
draw_screen()
{
  int i, j;
  /* 接龍遊戲 */
  vs_bar("\xB1\xB5\xC0\x73\xB9\x43\xC0\xB8");

  for (i = 0; i < 7; i++)
  {
    for (j = 0; j <= i; j++)
      draw_card(4 + j, 5 + i * 10, (i == j) ? cards[i] : -1);
  }
}


/*-------------------------------------------------------*/
/* 畫出游標						 */
/*-------------------------------------------------------*/


static void
draw_cursor(location, mode)
  int location;
  int mode;		/* 1:上色  0:清除 */
{
  int x, y;

  x = 8 + seven[location];
  y = 9 + location * 10;
  move(x, y);
  /* ● */
  /* 　 */
  outs(mode ? "\xA1\xB4" : "\xA1\x40");
  if (mode)
    move(x, y + 1);		/* 避免自動偵測全形 */
}


/*-------------------------------------------------------*/
/* 清除螢幕上的牌					 */
/*-------------------------------------------------------*/


static void 
clear_card(location)
  int location;
{
  move(9 + seven[location], 5 + location * 10);
  outs("          ");
}


/*-------------------------------------------------------*/
/* 遊戲參數初始化					 */
/*-------------------------------------------------------*/


static int
init_dragon()
{
  int i, j, num;

  for (i = 0; i < 52; i++)	/* 牌先一張一張排好，準備洗牌 */
    cards[i] = i;

  for (i = 0; i < 51; i++)
  {
    j = rnd(52 - i) + i;

    /* cards[j] 和 cards[i] 交換 */
    num = cards[i];
    cards[i] = cards[j];
    cards[j] = num;
  }

  for (i = 0; i < 7; i++)
  {
    seven[i] = i;
    points[i] = cards[i];
  }

  return 0;
}


/*-------------------------------------------------------*/
/* 判斷遊戲是否結束					 */
/*-------------------------------------------------------*/


static int	/* 1:成功 */
gameover()
{
  int i;

  for (i = 0; i < 7; i++)
  {
    if (seven[i] != -1)
      return 0;
  }
  return 1;
}


/*-------------------------------------------------------*/
/* 遊戲主程式						 */
/*-------------------------------------------------------*/


int			/* >=0:成功 -1:失敗 -2:離開 */
play_dragon()
{
  int i;
  int location = 0;	/* 目前游標的位置 */
  int now = 7;		/* 目前用到 cards[] 第幾張牌 */
  int have_card = 22;	/* 22 次換牌機會 */
  int point;		/* 目前手上的這張牌 */

  clear();

  draw_screen();
  draw_cursor(location, 1);
  point = cards[now];
  draw_card(14, 5, cards[now++]);
  move(19, 40);
  /* 您還有 %2d 次機會可以換牌 */
  prints("\xB1\x7A\xC1\xD9\xA6\xB3 %2d \xA6\xB8\xBE\xF7\xB7\x7C\xA5\x69\xA5\x48\xB4\xAB\xB5\x50", have_card);
  move(b_lines, 0);
  /* ★ 操作說明：(←)左移 (→)右移 (Enter)吃牌 (c)換牌 (q)離開 */
  outs("\xA1\xB9 \xBE\xDE\xA7\x40\xBB\xA1\xA9\xFA\xA1\x47(\xA1\xF6)\xA5\xAA\xB2\xBE (\xA1\xF7)\xA5\x6B\xB2\xBE (Enter)\xA6\x59\xB5\x50 (c)\xB4\xAB\xB5\x50 (q)\xC2\xF7\xB6\x7D");

  for (;;)
  {
    switch (vkey())
    {
    case 'c':
      if (have_card <= 0)
	return -1;
      have_card--;
      move(19, 47);
      prints("%2d", have_card);
      point = cards[now];
      draw_card(14, 5, cards[now++]);
      break;

    case KEY_RIGHT:
      draw_cursor(location, 0);
      do
      {
	location = (location + 1) % 7;
      } while (seven[location] == -1);
      draw_cursor(location, 1);
      break;

    case KEY_LEFT:
      draw_cursor(location, 0);
      do
      {
	location = (location == 0) ? 6 : location - 1;
      } while (seven[location] == -1);
      draw_cursor(location, 1);
      break;

    case '\n':
    case ' ':
      if (points[location] % 13 - point % 13 == 1 ||
	points[location] % 13 - point % 13 == -1 ||
	points[location] % 13 - point % 13 == 12 ||
	points[location] % 13 - point % 13 == -12)
      {
	point = points[location];
	draw_card(14, 5, point);
	clear_card(location);
	draw_cursor(location, 0);
	seven[location]--;
	if (seven[location] >= 0)
	{
	  points[location] = cards[now];
	  draw_card(4 + seven[location], 5 + location * 10, cards[now++]);
	  draw_cursor(location, 1);
	}
	else
	{
	  for (i = 0; i < 5; i++)
	  {
	    move(4 + i, 5 + location * 10);
	    outs("          ");
	  }
	  if (gameover() == 1)
	    return have_card;
	  do
	  {
	    location = (location + 1) % 7;
	  } while (seven[location] == -1);
	  draw_cursor(location, 1);
	}
      }
      break;

    case 'q':
      return -2;
    }
  }
}


int
main_dragon()
{
  draw_explain();

  while (1)
  {
    init_dragon();

    switch (play_dragon())
    {
    case -1:
      /* 挑戰失敗！ */
      vmsg("\xAC\x44\xBE\xD4\xA5\xA2\xB1\xD1\xA1\x49");
      break;

    case -2:
      vmsg(MSG_QUITGAME);
      return 0;

    default:
      /* 恭喜您過關啦！ */
      vmsg("\xAE\xA5\xB3\xDF\xB1\x7A\xB9\x4C\xC3\xF6\xB0\xD5\xA1\x49");
    }

    /* 是否要繼續玩(Y/N)？[N]  */
    if (vans("\xAC\x4F\xA7\x5F\xAD\x6E\xC4\x7E\xC4\xF2\xAA\xB1(Y/N)\xA1\x48[N] ") != 'y')
      break;
  }

  return 0;
}
#endif	/* HAVE_GAME */
