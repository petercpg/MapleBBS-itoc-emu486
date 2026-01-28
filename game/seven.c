/*-------------------------------------------------------*/
/* game/seven.c         ( NTHU CS MapleBBS Ver 3.10 )    */
/*-------------------------------------------------------*/
/* target : 賭城七張遊戲                                 */
/* create : 98/07/29                                     */
/* update : 01/04/26                                     */
/* author : weiren@mail.eki.com.tw                       */
/* recast : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"


#ifdef HAVE_GAME


/* 烏龍 */
/* 單胚 */
/* 兔胚 */
/* 三條 */
/* 順子 */
/* 同花 */
/* 葫蘆 */
/* 鐵支 */
/* 柳丁 */
static char *kind[9] = {"\xAF\x51\xC0\x73", "\xB3\xE6\xAD\x46", "\xA8\xDF\xAD\x46", "\xA4\x54\xB1\xF8", "\xB6\xB6\xA4\x6C", "\xA6\x50\xAA\xE1", "\xB8\xAC\xC4\xAA","\xC5\x4B\xA4\xE4", "\xAC\x68\xA4\x42"};
static char *poker[52] = 
{
  /* ２ */
  /* ２ */
  /* ２ */
  /* ２ */
  /* ３ */
  /* ３ */
  /* ３ */
  /* ３ */
  /* ４ */
  /* ４ */
  /* ４ */
  /* ４ */
  "\xA2\xB1", "\xA2\xB1", "\xA2\xB1", "\xA2\xB1", "\xA2\xB2", "\xA2\xB2", "\xA2\xB2", "\xA2\xB2", "\xA2\xB3", "\xA2\xB3", "\xA2\xB3", "\xA2\xB3", 
  /* ５ */
  /* ５ */
  /* ５ */
  /* ５ */
  /* ６ */
  /* ６ */
  /* ６ */
  /* ６ */
  /* ７ */
  /* ７ */
  /* ７ */
  /* ７ */
  "\xA2\xB4", "\xA2\xB4", "\xA2\xB4", "\xA2\xB4", "\xA2\xB5", "\xA2\xB5", "\xA2\xB5", "\xA2\xB5", "\xA2\xB6", "\xA2\xB6", "\xA2\xB6", "\xA2\xB6", 
  /* ８ */
  /* ８ */
  /* ８ */
  /* ８ */
  /* ９ */
  /* ９ */
  /* ９ */
  /* ９ */
  /* Ｔ */
  /* Ｔ */
  /* Ｔ */
  /* Ｔ */
  "\xA2\xB7", "\xA2\xB7", "\xA2\xB7", "\xA2\xB7", "\xA2\xB8", "\xA2\xB8", "\xA2\xB8", "\xA2\xB8", "\xA2\xE2", "\xA2\xE2", "\xA2\xE2", "\xA2\xE2", 
  /* Ｊ */
  /* Ｊ */
  /* Ｊ */
  /* Ｊ */
  /* Ｑ */
  /* Ｑ */
  /* Ｑ */
  /* Ｑ */
  /* Ｋ */
  /* Ｋ */
  /* Ｋ */
  /* Ｋ */
  "\xA2\xD8", "\xA2\xD8", "\xA2\xD8", "\xA2\xD8", "\xA2\xDF", "\xA2\xDF", "\xA2\xDF", "\xA2\xDF", "\xA2\xD9", "\xA2\xD9", "\xA2\xD9", "\xA2\xD9", 
  /* Ａ */
  /* Ａ */
  /* Ａ */
  /* Ａ */
  "\xA2\xCF", "\xA2\xCF", "\xA2\xCF", "\xA2\xCF"
};


static void
out_song()
{
  static int count = 0;

  /* 陳譯賢˙心跳 */
  uschar *msg[8] =
  {
    /* 你又近又遠  話隱隱約約  我猜不出  也探不到你的一切 */
    "\xA7\x41\xA4\x53\xAA\xF1\xA4\x53\xBB\xB7  \xB8\xDC\xC1\xF4\xC1\xF4\xAC\xF9\xAC\xF9  \xA7\xDA\xB2\x71\xA4\xA3\xA5\x58  \xA4\x5D\xB1\xB4\xA4\xA3\xA8\xEC\xA7\x41\xAA\xBA\xA4\x40\xA4\xC1",
    /* 夜半醒半睡  心苦苦甜甜  我防不到  已走進某一種危險 */
    "\xA9\x5D\xA5\x62\xBF\xF4\xA5\x62\xBA\xCE  \xA4\xDF\xAD\x57\xAD\x57\xB2\xA2\xB2\xA2  \xA7\xDA\xA8\xBE\xA4\xA3\xA8\xEC  \xA4\x77\xA8\xAB\xB6\x69\xAC\x59\xA4\x40\xBA\xD8\xA6\x4D\xC0\x49",
    /* 血液慢慢發熱臉發燙  呼吸彷彿要停了 */
    "\xA6\xE5\xB2\x47\xBA\x43\xBA\x43\xB5\x6F\xBC\xF6\xC1\x79\xB5\x6F\xBF\x53  \xA9\x49\xA7\x6C\xA7\xCF\xA9\xBB\xAD\x6E\xB0\xB1\xA4\x46",
    /* 明明白白不是單純喜歡  那是愛  那是愛嗎 */
    "\xA9\xFA\xA9\xFA\xA5\xD5\xA5\xD5\xA4\xA3\xAC\x4F\xB3\xE6\xAF\xC2\xB3\xDF\xC5\x77  \xA8\xBA\xAC\x4F\xB7\x52  \xA8\xBA\xAC\x4F\xB7\x52\xB6\xDC",
    /* 聽到自己心跳  為誰心跳  鼓聲般的震盪  緊纏著我不能放 */
    "\xC5\xA5\xA8\xEC\xA6\xDB\xA4\x76\xA4\xDF\xB8\xF5  \xAC\xB0\xBD\xD6\xA4\xDF\xB8\xF5  \xB9\xAA\xC1\x6E\xAF\xEB\xAA\xBA\xBE\x5F\xC0\xFA  \xBA\xF2\xC4\xF1\xB5\xDB\xA7\xDA\xA4\xA3\xAF\xE0\xA9\xF1",
    /* 多想  把心交換  你就能數算  你就會明瞭  心跳  分分秒 */
    "\xA6\x68\xB7\x51  \xA7\xE2\xA4\xDF\xA5\xE6\xB4\xAB  \xA7\x41\xB4\x4E\xAF\xE0\xBC\xC6\xBA\xE2  \xA7\x41\xB4\x4E\xB7\x7C\xA9\xFA\xC1\x41  \xA4\xDF\xB8\xF5  \xA4\xC0\xA4\xC0\xAC\xED",
    /* 血液慢慢發熱臉發燙  呼吸彷彿要停了 */
    "\xA6\xE5\xB2\x47\xBA\x43\xBA\x43\xB5\x6F\xBC\xF6\xC1\x79\xB5\x6F\xBF\x53  \xA9\x49\xA7\x6C\xA7\xCF\xA9\xBB\xAD\x6E\xB0\xB1\xA4\x46",
    /* 明明白白不是單純喜歡  莫非就是愛  莫非是愛 */
    "\xA9\xFA\xA9\xFA\xA5\xD5\xA5\xD5\xA4\xA3\xAC\x4F\xB3\xE6\xAF\xC2\xB3\xDF\xC5\x77  \xB2\xF6\xAB\x44\xB4\x4E\xAC\x4F\xB7\x52  \xB2\xF6\xAB\x44\xAC\x4F\xB7\x52"
  };
  move(b_lines - 2, 0);
  /* \033[1;3%dm%s\033[m  籌碼還有 %d 元 */
  prints("\033[1;3%dm%s\033[m  \xC4\x77\xBD\x58\xC1\xD9\xA6\xB3 %d \xA4\xB8", time(0) % 7, msg[count], cuser.money);
  clrtoeol();
  if (++count == 8)
    count = 0;
}


static inline int
find_pair(set)			/* 有 Pair 就傳回 1 */
  int set[6];
{
  int i, j;

  for (i = 0; i < set[5] - 1; i++)
  {
    for (j = i + 1; j < set[5]; j++)
    {
      if (set[j] / 4 == set[i] / 4)
	return 1;
    }
  }
  return 0;
}


static inline int
find_tpair(set)			/* Two Pair 傳回 1 */
  int set[6];
{
  int i, j, k;
  int z[13] = {0};

  for (i = 0; i < 13; i++)
  {
    for (j = 0; j < 5; j++)
    {
      if (set[j] / 4 == i)
	z[i]++;
    }
  }
  k = 0;
  for (i = 0; i < 13; i++)
  {
    if (z[i] >= 2)
      k++;
  }
  if (k == 2)
    return 1;
  return 0;
}


static inline int
find_triple(set)		/* 三條傳回 3, 鐵支傳回 4 */
  int set[6];
{
  int i, j, k;

  for (i = 0; i < 13; i++)
  {
    k = 0;
    for (j = 0; j < 5; j++)
    {
      if (set[j] / 4 == i)
	k++;
    }
    if (k == 4)
      return 4;
    if (k == 3)
      return 3;
  }
  return 0;
}


static inline int
find_dragon(set)		/* 順傳回 1, 否則傳回 0 */
  int set[6];
{
  int i;
  int test[6];

  for (i = 0; i < 5; i++)
    test[i] = set[i] / 4;

  for (i = 0; i < 3; i++)
  {
    if (test[i] + 1 != test[i + 1])
      return 0;
  }

  if (test[4] == 12 && test[0] == 0)
    return 1;			/* A2345 順 */

  if (test[3] + 1 == test[4])
    return 1;			/* 一般順 */
  return 0;
}


static inline int
find_flush(set)			/* 同花傳回 1, 否則傳回 0 */
  int set[6];
{
  int i;
  int test[6];

  for (i = 0; i < 5; i++)
    test[i] = set[i] % 4;

  for (i = 1; i < 5; i++)
  {
    if (test[0] != test[i])
      return 0;
  }

  return 1;
}


static int
find_all(set)
  int set[6];
{
  int i;
  int a[9];		/* 烏龍, 胚 , 兔胚, 三條, 順, 同花, 胡盧, 鐵支, 同花順 */

  a[0] = 1;		/* a[0]  1    2     3     4    5    6     7     a[8]   */

  for (i = 1; i < 9; i++)
    a[i] = 0;

  a[1] = find_pair(set);
  a[2] = find_tpair(set);

  switch (find_triple(set))
  {
  case 3:
    a[3] = 1;
    break;

  case 4:
    a[7] = 1;
    break;
  }

  a[4] = find_dragon(set);
  a[5] = find_flush(set);

  if (a[2] && a[3])
    a[6] = 1;			/* 兔胚 + 三條 = 胡盧 */
  if (a[4] && a[5])
    a[8] = 1;			/* 同花 + 順 = 同花順 */

  for (i = 8; i >= 0; i--)
    if (a[i])
      return i;
}


static inline int
diedragon(set, a, b)
  int set[6];
  int a, b;
{
  int card[13] = {0};
  int first[2];
  int i, z;

  first[0] = a;
  first[1] = b;
  z = find_all(set);

  if (!z)
  {				/* 第二堵烏龍 */
    if (first[0] / 4 == first[1] / 4)
      return 1;
    if (first[1] / 4 > set[4] / 4)
      return 1;
    if (first[1] / 4 == set[4] / 4)
    {
      if (first[0] / 4 > set[3] / 4)
	return 1;
    }				/* 倒龍 */
  }

  else if (z == 1)		/* 胚 */
  {
    for (i = 0; i < 5; i++)
      card[set[i] / 4]++;

    for (i = 0; i < 13; i++)
    {
      if (card[i] == 2 && first[0] / 4 == first[1] / 4 && first[0] / 4 > i)
	return 1;		/* 兩堵都單胚且倒龍 */
    }
  }
  return 0;
}


static inline int
bigsmall(h, g, key, gm)
  int h[7], g[7], key, gm[2];
{
  int hm[2];
  int i, j, k = 0, tmp = 0, tmp2 = 0, x, a, b;
  int duA = 0, duB = 0;		/* duA duB 是兩堵判定輸贏參數, 1 是電腦贏 */
  int hset[6], gset[6];	/* host, guest */
  int gc[13] = {0}, hc[13] = {0};

  for (i = 0; i < 6; i++)
  {
    for (j = i + 1; j < 7; j++)
    {
      if (key == k)
      {
	hm[0] = i;
	hm[1] = j;
      };
      k++;
    }
  }

  if (hm[1] < hm[0])
  {
    k = hm[1];
    hm[1] = hm[0];
    hm[0] = k;
  }

  if (gm[1] < gm[0])
  {
    k = gm[1];
    gm[1] = gm[0];
    gm[0] = k;
  }

  if (h[hm[0]] / 4 == h[hm[1]] / 4)
    tmp = 1;
  if (g[gm[0]] / 4 == g[gm[1]] / 4)
    tmp2 = 1;
  if (tmp == tmp2)
  {
    if (h[hm[1]] / 4 > g[gm[1]] / 4)
      duA = 1;			/* duA=1 表示第一堵莊家贏 */
    if (h[hm[1]] / 4 == g[gm[1]] / 4 && tmp == 1)
      duA = 1;			/* 第一堵都胚且平手, 莊家贏 */
    if (h[hm[1]] / 4 == g[gm[1]] / 4 && tmp == 0 && h[hm[0]] / 4 >= g[gm[0]] / 4)
      duA = 1;
  }
  if (tmp > tmp2)
    duA = 1;
  if (tmp < tmp2)
    duA = 0;
  k = 0;
  j = 0;

  for (i = 0; i < 7; i++)
  {
    if (i != hm[0] && i != hm[1])
    {
      hset[j] = h[i];
      j++;
    }
    if (i != gm[0] && i != gm[1])
    {
      gset[k] = g[i];
      k++;
    }
  }

  hset[5] = 5;
  gset[5] = 5;
  tmp = find_all(hset);
  tmp2 = find_all(gset);

  if (tmp > tmp2)
  {
    duB = 1;
  }
  else if (tmp == tmp2)
  {
    for (i = 0; i < 5; i++)
    {
      gc[gset[i] / 4]++;
      hc[hset[i] / 4]++;
    }
    switch (tmp)
    {
    case 0:
      i = 12;
      x = 0;
      duB = 1;			/* 兩方都是烏龍 */
      do
      {
	if (hc[i] > gc[i])
	{
	  duB = 1;
	  x = 1;
	}
	if (hc[i] < gc[i])
	{
	  duB = 0;
	  x = 1;
	}
	i--;
	if (i < 0)
	  x = 1;
      } while (x == 0);
      break;

    case 1:
      for (i = 0; i < 12; i++)
      {
	if (hc[i] == 2)
	  a = i;
	if (gc[i] == 2)
	  b = i;
      }
      if (a > b)
      {
	duB = 1;		/* 兩方都是胚 */
      }
      else if (a == b)
      {
	i = 12;
	j = 12;
	x = 0;
	duB = 1;
	do
	{
	  if (hc[i] == 2)
	    i--;
	  if (hc[j] == 2)
	    j--;
	  if (hc[i] > gc[j])
	  {
	    duB = 1;
	    x = 1;
	  }
	  if (hc[i] < gc[j])
	  {
	    duB = 0;
	    x = 1;
	  }
	  i--;
	  j--;
	  if (i < 0 || j < 0)
	    x = 1;
	} while (x == 0);
      }
      break;

    case 2:
      i = 12;
      x = 0;
      duB = 2;			/* 兩方都是兔胚 */
      do
      {
	if (hc[i] > gc[i] && hc[i] != 1)
	{
	  duB = 1;
	  x = 1;
	};
	if (hc[i] < gc[i] && gc[i] != 1)
	{
	  duB = 0;
	  x = 1;
	};
	i--;
	if (i < 0)
	  x = 1;
      } while (x == 0);
      if (duB == 2)
      {
	for (i = 0; i < 12; i++)
	{
	  if (hc[i] == 1)
	    a = i;
	  if (gc[i] == 1)
	    b = i;
	}
	duB = 1;
	if (a < b)
	  duB = 0;
      }
      break;

    case 3:
    case 6:
      for (i = 0; i < 12; i++)
      {
	if (hc[i] == 3)
	  a = i;
	if (gc[i] == 3)
	  b = i;
      }
      if (a > b)
	duB = 1;		/* 兩方都是三條(胡盧) */
      else if (a < b)
	duB = 0;
      break;

    case 4:
      i = 12;
      x = 0;
      a = 0;
      b = 0;			/* 兩方都是順子 */
      do
      {
	if (hc[i] > gc[i])
	{
	  duB = 1;
	  x = 1;
	}
	if (hc[i] < gc[i])
	{
	  duB = 0;
	  x = 1;
	}
	i--;
	if (i < 0)
	{
	  duB = 1;
	  x = 1;
	}
      } while (x == 0);

      if (hc[12] == hc[0] && hc[0] == 1)
	a = 1;
      if (gc[12] == gc[0] && gc[0] == 1)
	b = 1;
      if (a > b)
	duB = 0;
      if (a < b)
	duB = 1;
      if (a == b && b == 1)
	duB = 1;
      break;

    case 5:
      if (hset[0] % 4 > gset[0] % 4)
	duB = 1;		/* 兩方都是同花 */
      if (hset[0] % 4 < gset[0] % 4)
	duB = 0;
      if (hset[0] % 4 == gset[0] % 4)
      {
	if (hset[4] > gset[4])
	  duB = 1;
	if (hset[4] < gset[4])
	  duB = 0;
      }
      break;

    case 7:
      for (i = 0; i < 12; i++)
      {
	if (hc[i] == 4)
	  a = i;
	if (gc[i] == 4)
	  b = i;
      }
      if (a > b)
	duB = 1;		/* 兩方都是鐵支 */
      if (a < b)
	duB = 0;
      break;

    case 8:
      if (hset[0] % 4 > gset[0] % 4)
	duB = 1;		/* 兩方都是同花順 */
      if (hset[0] % 4 < gset[0] % 4)
	duB = 0;
      if (hset[0] % 4 == gset[0] % 4)
      {
	i = 12;
	x = 0;
	do
	{
	  if (hc[i] > gc[i])
	  {
	    duB = 1;
	    x = 1;
	  }
	  if (hc[i] < gc[i])
	  {
	    duB = 0;
	    x = 1;
	  }
	  i--;
	  if (i < 0)
	  {
	    duB = 1;
	    x = 1;
	  }
	} while (x == 0);
      }
      break;
    }
  }
  return 2 * duA + duB;
}


static void
print_Scard(card, x, y)
  int card, x, y;
{
  /* Ｃ */
  /* Ｄ */
  /* Ｈ */
  /* Ｓ */
  char *flower[4] = {"\xA2\xD1", "\xA2\xD2", "\xA2\xD6", "\xA2\xE1"};

  move(x, y);
  /* ╭───╮ */
  outs("\xA2\x7E\xA2\x77\xA2\x77\xA2\x77\xA2\xA1");
  move(x + 1, y);
  /* │%s    │ */
  prints("\xA2\x78%s    \xA2\x78", poker[card]);
  move(x + 2, y);
  /* │%s    │ */
  prints("\xA2\x78%s    \xA2\x78", flower[card % 4]);
  move(x + 3, y);
  /* │      │ */
  outs("\xA2\x78      \xA2\x78");
  move(x + 4, y);
  /* │      │ */
  outs("\xA2\x78      \xA2\x78");
  move(x + 5, y);
  /* │      │ */
  outs("\xA2\x78      \xA2\x78");
  move(x + 6, y);
  /* ╰───╯ */
  outs("\xA2\xA2\xA2\x77\xA2\x77\xA2\x77\xA2\xA3");
}


static inline void
print_hostcard(card, x)		/* x 為兩張的組合 key */
  int card[7];
  int x;
{
  int i, j, k = 0;
  int tmp, tmp2;
  int set[6];

  for (i = 1; i < 6; i++)
  {
    move(5 + i, 0);
    clrtoeol();
  }

  for (i = 0; i < 6; i++)
  {
    for (j = i + 1; j < 7; j++)
    {
      if (x == k)
      {
	tmp = i;
	tmp2 = j;
      };
      k++;
    }
  }

  print_Scard(card[tmp], 3, 0);
  print_Scard(card[tmp2], 3, 4);

  j = 0;
  for (i = 0; i < 7; i++)
  {
    if (i != tmp && i != tmp2)
    {
      print_Scard(card[i], 3, 16 + j * 4);
      set[j] = card[i];
      j++;
    }
  }
  set[5] = 5;
  move(7, 4);
  if (card[tmp] / 4 == card[tmp2] / 4)
    x = 1;
  /* \033[1;44;33m   %s%s   \033[m  │  │  \033[1;44;33m         %s         \033[m */
  prints("\033[1;44;33m   %s%s   \033[m  \xA2\x78  \xA2\x78  \033[1;44;33m         %s         \033[m",
    /* 胚 */
    poker[card[tmp]], x == 1 ? "\xAD\x46" : poker[card[tmp2]] ,kind[find_all(set)]);
}


static inline int
score(first, set)	/* 回傳分兩堵後的評分(AI), 電腦會把 21 種牌型都拆出來, 取評分高者 */
  int first[2], set[6];
{
  int i, z;
  int points = 0;
  int card[13] = {0};

  z = find_all(set);
  if (z == 0)
  {
    if (first[0] / 4 == first[1] / 4)
      return 0;
    if (first[1] / 4 >= set[4] / 4)
      return 0;			/* 倒龍 */
  }
  else if (z == 1)
  {
    for (i = 0; i < 5; i++)
      card[set[i] / 4]++;
    for (i = 0; i < 13; i++)
    {
      if (card[i] == 2 && first[0] / 4 == first[1] / 4 && first[0] / 4 >= i)
	return 0;		/* 兩堵都單胚且倒龍 */
    }
  }

  points = z + 2;		/* 第二堵烏龍就算兩分, 以上遞增 */
  if (points >= 5)
    points ++;			/* 第二堵若有順以上再加一分 */
  if (first[0] / 4 == first[1] / 4)
    points += 3;		/* 第一堵有胚分數加三 */
  if (first[0] / 4 != first[1] / 4 && first[1] / 4 >= 10)
    points++;
  /* 第一堵無胚有 Q 以上加一分 */
  if (first[0] / 4 == 12 || first[1] / 4 == 12)
    points += 1;		/* 第一堵有 A 分數再加一 */
  return points;
}


static inline int 
find_host(h)			/* 傳回兩張的組合 key */
  int h[7];
{
  int i, j, k, x = 0, z = 0;
  int tmp = 0, tmp2 = 0;  
  int result[21] = {0}, set[6] = {0}, first[2];

  for (i = 0; i < 6; i++)
  {
    for (j = i + 1; j < 7; j++)
    {
      first[0] = h[i];
      first[1] = h[j];
      x = 0;
      for (k = 0; k < 7; k++)
      {
	if (i != k && j != k)
	{
	  set[x] = h[k];
	  x++;
	}
      }
      set[5] = 5;
      result[z] = score(first, set);
      z++;
    }
  }
  for (i = 0; i < 21; i++)
  {
    if (result[i] >= tmp)
    {
      tmp = result[i];
      tmp2 = i;
    }
  }
  return tmp2;
}


static int
get_newcard(mode)
  int mode;			/* 0:重新洗牌  1:發牌 */
{
  static int card[14];	/* 最多只會用到 14 張牌 */
  static int now;	/* 發出第 now 張牌 */
  char num;
  int i;

  if (!mode)	/* 重新洗牌 */
  {
    now = 0;
    return -1;
  }

rand_num:		/* random 出一張和之前都不同的牌 */
  num = rnd(52);
  for (i = 0; i < now; i++)
  {
    if (num == card[i])	/* 這張牌以前 random 過了 */
      goto rand_num;
  }

  card[now] = num;
  now++;

  return num;
}


int 
main_seven()
{
  int money;		/* 押金 */
  int host_card[7];	/* 電腦的 7 張牌張 */
  int guest_card[7];	/* 玩家的 7 張牌張 */
  int mark[2];		/* 玩家標記用 */
  int set[6];

  int i, j, win;
  char buf[10];

  if (HAS_STATUS(STATUS_COINLOCK))
  {
    vmsg(msg_coinlock);
    return XEASY;
  }

  while (1)
  {
    /* 賭城七張 */
    vs_bar("\xBD\xE4\xAB\xB0\xA4\x43\xB1\x69");
    out_song();

    /* 請問要下注多少呢？(1 ~ 50000)  */
    vget(2, 0, "\xBD\xD0\xB0\xDD\xAD\x6E\xA4\x55\xAA\x60\xA6\x68\xA4\xD6\xA9\x4F\xA1\x48(1 ~ 50000) ", buf, 6, DOECHO);
    money = atoi(buf);
    if (money < 1 || money > 50000 || money > cuser.money)
      break;			/* 離開賭場 */

    cuser.money -= money;

    out_song();

    move(2, 0);
    clrtoeol();		/* 清掉「請問要下注多少」 */
    /* (按 ←→ 移動，按 ↑↓ 選取二張牌，選好按 enter 攤牌) */
    outs("(\xAB\xF6 \xA1\xF6\xA1\xF7 \xB2\xBE\xB0\xCA\xA1\x41\xAB\xF6 \xA1\xF4\xA1\xF5 \xBF\xEF\xA8\xFA\xA4\x47\xB1\x69\xB5\x50\xA1\x41\xBF\xEF\xA6\x6E\xAB\xF6 enter \xC5\x75\xB5\x50)");

    get_newcard(0);	/* 洗牌 */

    mark[0] = mark[1] = 123;	/* mark[?] = 123 表示沒有 mark */

    /* 發十四張牌 */
    for (i = 0; i < 7; i++)
    {
      host_card[i] = get_newcard(1);
      guest_card[i] = get_newcard(1);
    }

    /* 排序 */
    for (i = 0; i < 7; i++)
    {
      for (j = 0; j < (6 - i); j++)
      {
        /* 借用 win */
	if (guest_card[j] > guest_card[j + 1])
	{
	  win = guest_card[j];
	  guest_card[j] = guest_card[j + 1];
	  guest_card[j + 1] = win;
	}
	if (host_card[j] > host_card[j + 1])
	{
	  win = host_card[j];
	  host_card[j] = host_card[j + 1];
	  host_card[j + 1] = win;
	}
      }
    }

    /* 印出手上的牌 */
    move(3, 0);
    /* ╭─╭─╭─╭─╭─╭─╭───╮\n */
    outs("\xA2\x7E\xA2\x77\xA2\x7E\xA2\x77\xA2\x7E\xA2\x77\xA2\x7E\xA2\x77\xA2\x7E\xA2\x77\xA2\x7E\xA2\x77\xA2\x7E\xA2\x77\xA2\x77\xA2\x77\xA2\xA1\n");
    /* │  │  │  │  │  │  │      │\n */
    outs("\xA2\x78  \xA2\x78  \xA2\x78  \xA2\x78  \xA2\x78  \xA2\x78  \xA2\x78      \xA2\x78\n");
    /* │  │  │  │  │  │  │      │\n */
    outs("\xA2\x78  \xA2\x78  \xA2\x78  \xA2\x78  \xA2\x78  \xA2\x78  \xA2\x78      \xA2\x78\n");
    /* │  │  │  │  │  │  │      │\n */
    outs("\xA2\x78  \xA2\x78  \xA2\x78  \xA2\x78  \xA2\x78  \xA2\x78  \xA2\x78      \xA2\x78\n");
    /* │  │  │  │  │  │  │      │\n */
    outs("\xA2\x78  \xA2\x78  \xA2\x78  \xA2\x78  \xA2\x78  \xA2\x78  \xA2\x78      \xA2\x78\n");
    /* │  │  │  │  │  │  │      │\n */
    outs("\xA2\x78  \xA2\x78  \xA2\x78  \xA2\x78  \xA2\x78  \xA2\x78  \xA2\x78      \xA2\x78\n");
    /* ╰─╰─╰─╰─╰─╰─╰───╯ */
    outs("\xA2\xA2\xA2\x77\xA2\xA2\xA2\x77\xA2\xA2\xA2\x77\xA2\xA2\xA2\x77\xA2\xA2\xA2\x77\xA2\xA2\xA2\x77\xA2\xA2\xA2\x77\xA2\x77\xA2\x77\xA2\xA3");
    for (i = 0; i < 7; i++)
      print_Scard(guest_card[i], 11, 0 + 4 * i);

    i = j = 0;
    for (;;)	/* 選出二張牌 */
    {
      /* 在此迴圈內，i 是第幾張牌，j 是已選取了幾張牌 */
      move(15, 1 + i * 4);
      switch (vkey())
      {
      case KEY_RIGHT:
	if (i < 6)
	  i++;
	break;

      case KEY_LEFT:
	if (i > 0)
	  i--;
	break;

      case KEY_UP:		/* 選取這張牌 */
        if (j < 2 && mark[0] != i && mark[1] != i)	/* 不能重覆 mark 且最多 mark 二張 */
	{
	  if (mark[0] == 123)
	    mark[0] = i;
	  else
	    mark[1] = i;
	  j++;
	  move(15, 2 + i * 4);
	  /* ● */
	  outs("\xA1\xB4");
	}
	break;

      case KEY_DOWN:		/* 取消選取這張牌 */
	if (mark[0] == i)
	{
	  mark[0] = 123;
	  j--;
	  move(15, 2 + i * 4);
	  outs("  ");
	}
	else if (mark[1] == i)
	{
	  mark[1] = 123;
	  j--;
	  move(15, 2 + i * 4);
	  outs("  ");
	}
	break;

      case '\n':		/* 選出兩張後按 enter */
	if (j == 2)
	  goto end_choose;
	break;
      }
    }
  end_choose:

    if (mark[0] > mark[1])
    {
      i = mark[0];
      mark[0] = mark[1];
      mark[1] = i;
    }

    /* 印出玩家分好兩堵後的牌 */
    for (i = 1; i < 18; i++)
    {
      move(i, 0);
      clrtoeol();
    }
    print_Scard(guest_card[mark[0]], 11, 0);
    print_Scard(guest_card[mark[1]], 11, 4);

    j = 0;
    for (i = 0; i < 7; i++)
    {
      if (i != mark[0] && i != mark[1])
      {
	print_Scard(guest_card[i], 11, 16 + j * 4);
	set[j] = guest_card[i];
	j++;
      }
    }

    /* 判斷是否倒龍，若倒龍則結束 */
    set[5] = 5;
    if (diedragon(set, guest_card[mark[0]], guest_card[mark[1]]))
    {
      /* 倒龍 */
      vmsg("\xAD\xCB\xC0\x73");
      continue;
    }

    /* 判斷勝負 */
    i = find_host(host_card);
    print_hostcard(host_card, i);
    win = bigsmall(host_card, guest_card, i, mark);

    /* 秀出結果 */
    switch (win)
    {
      /* 借用 i 來當做 color1; 借用 j 來當做 color2 */

    case 0:	/* 玩家 duA duB 皆贏  */
      win = 2;
      i = 41;
      j = 41;
      break;

    case 1:	/* 玩家 duA 贏 duB 輸 */
      win = 1;
      i = 41;
      j = 47;
      break;

    case 2:	/* 玩家 duA 輸 duB 贏 */
      win = 1;
      i = 47;
      j = 41;
      break;

    case 3:	/* 玩家 duA duB 皆輸 */
      win = 0;
      i = 47;
      j = 47;
      break;
    }

    move(15, 4);
    /* \033[1;%d;%dm   %s%s   \033[m  │  │  \033[1;%d;%dm         %s         \033[m */
    prints("\033[1;%d;%dm   %s%s   \033[m  \xA2\x78  \xA2\x78  \033[1;%d;%dm         %s         \033[m",
      i, i == 41 ? 33 : 30, poker[guest_card[mark[0]]], 
      /* 胚 */
      (guest_card[mark[0]] / 4 == guest_card[mark[1]] / 4) ? "\xAD\x46" : poker[guest_card[mark[1]]],
      j, j == 41 ? 33 : 30, kind[find_all(set)]);

    switch (win)
    {
    case 2:
      /* 您贏了 */
      vmsg("\xB1\x7A\xC4\xB9\xA4\x46");
      money *= 2;
      break;

    case 1:
      /* 平手 */
      vmsg("\xA5\xAD\xA4\xE2");
      break;

    case 0:
      /* 您輸了 */
      vmsg("\xB1\x7A\xBF\xE9\xA4\x46");
      money = 0;
      break;
    }
    addmoney(money);
  }
  return 0;
}
#endif	/* HAVE_GAME */
