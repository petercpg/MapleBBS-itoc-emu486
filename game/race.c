/*-------------------------------------------------------*/
/* game/race.c          ( NTHU CS MapleBBS Ver 3.10 )    */
/*-------------------------------------------------------*/
/* target : 賽馬場遊戲                                   */
/* create : 98/12/17                                     */
/* update : 01/04/21                                     */
/* author : SugarII (u861838@Oz.nthu.edu.tw)             */
/* recast : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"


#ifdef HAVE_GAME


static int pace[5];		/* 五匹馬跑了多遠 */


static void
out_song()
{
  static int count = 0;  

  /* 張雨˙一言難盡 */
  uschar *msg[13] = 
  {
    /* 妳給我一場戲  妳看著我入迷 */
    "\xA9\x70\xB5\xB9\xA7\xDA\xA4\x40\xB3\xF5\xC0\xB8  \xA9\x70\xAC\xDD\xB5\xDB\xA7\xDA\xA4\x4A\xB0\x67",
    /* 被妳從心裡剝落的感情  痛的不知怎麼捨去 */
    "\xB3\x51\xA9\x70\xB1\x71\xA4\xDF\xB8\xCC\xAD\xE9\xB8\xA8\xAA\xBA\xB7\x50\xB1\xA1  \xB5\x68\xAA\xBA\xA4\xA3\xAA\xBE\xAB\xE7\xBB\xF2\xB1\xCB\xA5\x68",
    /* 遲遲不能相信這感覺  像自己和自己分離 */
    "\xBF\xF0\xBF\xF0\xA4\xA3\xAF\xE0\xAC\xDB\xAB\x48\xB3\x6F\xB7\x50\xC4\xB1  \xB9\xB3\xA6\xDB\xA4\x76\xA9\x4D\xA6\xDB\xA4\x76\xA4\xC0\xC2\xF7",
    /* 而信誓旦旦的愛情  在那裡 */
    "\xA6\xD3\xAB\x48\xBB\x7D\xA5\xB9\xA5\xB9\xAA\xBA\xB7\x52\xB1\xA1  \xA6\x62\xA8\xBA\xB8\xCC",
    /* 我一言難盡  忍不住傷心 */
    "\xA7\xDA\xA4\x40\xA8\xA5\xC3\xF8\xBA\xC9  \xA7\xD4\xA4\xA3\xA6\xED\xB6\xCB\xA4\xDF",
    /* 衡量不出愛或不愛之間的距離 */
    "\xBF\xC5\xB6\x71\xA4\xA3\xA5\x58\xB7\x52\xA9\xCE\xA4\xA3\xB7\x52\xA4\xA7\xB6\xA1\xAA\xBA\xB6\x5A\xC2\xF7",
    /* 妳說妳的心  不再溫熱如昔 */
    "\xA9\x70\xBB\xA1\xA9\x70\xAA\xBA\xA4\xDF  \xA4\xA3\xA6\x41\xB7\xC5\xBC\xF6\xA6\x70\xA9\xF5",
    /* 從那裡開始  從那裡失去 */
    "\xB1\x71\xA8\xBA\xB8\xCC\xB6\x7D\xA9\x6C  \xB1\x71\xA8\xBA\xB8\xCC\xA5\xA2\xA5\x68",
    /* 我一言難盡  忍不住傷心 */
    "\xA7\xDA\xA4\x40\xA8\xA5\xC3\xF8\xBA\xC9  \xA7\xD4\xA4\xA3\xA6\xED\xB6\xCB\xA4\xDF",
    /* 衡量不出愛或不愛之間的距離 */
    "\xBF\xC5\xB6\x71\xA4\xA3\xA5\x58\xB7\x52\xA9\xCE\xA4\xA3\xB7\x52\xA4\xA7\xB6\xA1\xAA\xBA\xB6\x5A\xC2\xF7",
    /* 隱隱約約中  明白妳的決定 */
    "\xC1\xF4\xC1\xF4\xAC\xF9\xAC\xF9\xA4\xA4  \xA9\xFA\xA5\xD5\xA9\x70\xAA\xBA\xA8\x4D\xA9\x77",
    /* 不敢勉強妳  只好為難自己 */
    "\xA4\xA3\xB4\xB1\xAB\x6A\xB1\x6A\xA9\x70  \xA5\x75\xA6\x6E\xAC\xB0\xC3\xF8\xA6\xDB\xA4\x76",
    /* 我為難我自己  我為難我自己 */
    "\xA7\xDA\xAC\xB0\xC3\xF8\xA7\xDA\xA6\xDB\xA4\x76  \xA7\xDA\xAC\xB0\xC3\xF8\xA7\xDA\xA6\xDB\xA4\x76"
  };
  move(b_lines - 2, 0);
  /* \033[1;3%dm%s\033[m  籌碼還有 %d 元 */
  prints("\033[1;3%dm%s\033[m  \xC4\x77\xBD\x58\xC1\xD9\xA6\xB3 %d \xA4\xB8", time(0) % 7, msg[count], cuser.money);
  clrtoeol();
  if (++count == 13)
    count = 0;
}


static int			/* -1: 還沒分出勝負 0~4:贏的那匹馬 */
race_path(run, j, step)
  int run, j, step;
{
  int i;

  if (!step)
  {
    return -1;
  }
  else if (step < 0)
  {
    if (j + step < 0)
      j = -step;
    move(run + 9, (j + step) * 2 + 8);
    clrtoeol();
    pace[run] += step * 100;
    if (pace[run] < 1)
      pace[run] = 1;
    return -1;
  }

  /* step > 0 */
  move(run + 9, j * 2 + 8);
  for (i = 0; i < step; i++)
    /* ■ */
    outs("\xA1\xBD");

  if (pace[run] + step * 100 > 3000)
    return run;
  return -1;
}


int
main_race()
{
  int money[5];			/* 五匹馬的押金 */
  int speed[5];			/* 五匹馬的速度 */
  int stop[5];			/* 五匹馬的暫停 */
  int bomb;			/* 是否使用炸彈 */
  int run;			/* 目前在計算的那匹 */
  int win;			/* 哪匹馬贏了 */
  int flag;			/* 事件發生次數 */
  int i, j, ch;
  char buf[60];
  /* 赤兔 */
  /* 的盧 */
  /* 爪黃 */
  /* 飛電 */
  /* 汗血 */
  char *racename[5] = {"\xA8\xAA\xA8\xDF", "\xAA\xBA\xBF\x63", "\xA4\xF6\xB6\xC0", "\xAD\xB8\xB9\x71", "\xA6\xBD\xA6\xE5"};

  if (HAS_STATUS(STATUS_COINLOCK))
  {
    vmsg(msg_coinlock);
    return XEASY;
  }

  while (1)
  {
    /* 賽馬場 */
    vs_bar("\xC1\xC9\xB0\xA8\xB3\xF5");
    out_song();
    bomb = 0;
    win = -1;
    flag = 0;
    for (i = 0; i < 5; i++)
    {
      pace[i] = 1;
      stop[i] = money[i] = 0;
      speed[i] = 100;
    }
    move(5, 0);
    /*   \033[1m馬名：\033[m */
    outs("  \033[1m\xB0\xA8\xA6\x57\xA1\x47\033[m");
    for (i = 0; i < 5; i++)
      prints("     %d. \033[1;3%dm%s\033[m", i + 1, i + 1, racename[i]);
    /* \n  \033[1m速度：\033[m\n  \033[1m賭金：\033[m\n\n */
    outs("\n  \033[1m\xB3\x74\xAB\xD7\xA1\x47\033[m\n  \033[1m\xBD\xE4\xAA\xF7\xA1\x47\033[m\n\n");

    for (i = 0; i < 5; i++)
      /* %d.\033[1;3%dm%s\033[m║\n */
      prints("%d.\033[1;3%dm%s\033[m\xF9\xF8\n", i + 1, i + 1, racename[i]);
    /* ───╨──┴──┴──┴──┴──┴──┴──┴──┴──┴──╜ */
    outs("\xA2\x77\xA2\x77\xA2\x77\xF9\xF6\xA2\x77\xA2\x77\xA2\x72\xA2\x77\xA2\x77\xA2\x72\xA2\x77\xA2\x77\xA2\x72\xA2\x77\xA2\x77\xA2\x72\xA2\x77\xA2\x77\xA2\x72\xA2\x77\xA2\x77\xA2\x72\xA2\x77\xA2\x77\xA2\x72\xA2\x77\xA2\x77\xA2\x72\xA2\x77\xA2\x77\xA2\x72\xA2\x77\xA2\x77\xF9\xF7");

    while (1)
    {
      /* 決定各匹馬賭注 */
      /* 您要押哪匹馬(1-5)？[S]開始 [Q]離開： */
      ch = vget(2, 0, "\xB1\x7A\xAD\x6E\xA9\xE3\xAD\xFE\xA4\xC7\xB0\xA8(1-5)\xA1\x48[S]\xB6\x7D\xA9\x6C [Q]\xC2\xF7\xB6\x7D\xA1\x47", buf, 3, DOECHO);
      i = buf[0];
      if (!ch || i == 's')
      {
	if (money[0] || money[1] || money[2] || money[3] || money[4])
	  break;
	addmoney(money[0] + money[1] + money[2] + money[3] + money[4]);	/* 還錢 */
	goto abort_game;
      }
      else if (i < '1' || i > '5')
      {
	addmoney(money[0] + money[1] + money[2] + money[3] + money[4]);	/* 還錢 */
	goto abort_game;
      }

      /* 要押多少賭金？ */
      ch = vget(3, 0, "\xAD\x6E\xA9\xE3\xA6\x68\xA4\xD6\xBD\xE4\xAA\xF7\xA1\x48", buf, 6, DOECHO);
      j = atoi(buf);
      if (!ch)
      {
	if (money[0] || money[1] || money[2] || money[3] || money[4])
	  break;
	addmoney(money[0] + money[1] + money[2] + money[3] + money[4]);	/* 還錢 */
	goto abort_game;
      }
      if (j < 1 || j > cuser.money)
      {
	addmoney(money[0] + money[1] + money[2] + money[3] + money[4]);	/* 還錢 */
	goto abort_game;
      }

      money[i - '1'] += j;
      cuser.money -= j;

      move(7, 15);
      clrtoeol();
      outs("\033[1m");
      for (i = 0; i < 5; i++)
	prints("     \033[3%dm%7d", i + 1, money[i]);
      outs("\033[m");
      out_song();
    }

    /* 開始遊戲 */
    move(3, 0);
    clrtoeol();
    move(2, 0);
    clrtoeol();
    /* -== 請按 \033[1;36mk\033[m 為您選的勁駒加油，按 \033[1;36mz\033[m 可丟出炸彈(只有一次機會) ==- */
    outs("-== \xBD\xD0\xAB\xF6 \033[1;36mk\033[m \xAC\xB0\xB1\x7A\xBF\xEF\xAA\xBA\xAB\x6C\xBE\x73\xA5\x5B\xAA\x6F\xA1\x41\xAB\xF6 \033[1;36mz\033[m \xA5\x69\xA5\xE1\xA5\x58\xAC\xB5\xBC\x75(\xA5\x75\xA6\xB3\xA4\x40\xA6\xB8\xBE\xF7\xB7\x7C) ==-");

    while (win < 0)
    {
      move(6, 15);
      clrtoeol();
      outs("\033[1m");
      for (i = 0; i < 5; i++)
      {
	if (stop[i] < 1)
	  speed[i] += rnd(20) - (speed[i] + 170) / 30;
	if (speed[i] < 0)
	  speed[i] = 0;
	prints("     \033[3%dm%7d", i + 1, speed[i]);
      }
      outs("\033[m");

      do
      {
	ch = igetch();
      } while (ch != 'k' && (ch != 'z' || bomb));

      run = rnd(5);		/* 選擇事件發生對象 */
      flag %= 5;		/* 列印事件於螢幕上 */
      move(15 + flag, 0);
      clrtoeol();

      if (ch == 'z')		/* 丟炸彈 */
      {
	stop[run] = 3;
	/* \033[1m保齡球砸到\033[3%dm%s\033[37m停止前進三次，速度 = 0\033[m */
	prints("\033[1m\xAB\x4F\xC4\xD6\xB2\x79\xAF\x7B\xA8\xEC\033[3%dm%s\033[37m\xB0\xB1\xA4\xEE\xAB\x65\xB6\x69\xA4\x54\xA6\xB8\xA1\x41\xB3\x74\xAB\xD7 = 0\033[m",
	  run + 1, racename[run]);
	speed[run] = 0;
	flag++;
	bomb = 1;
      }
      else if (rnd(12) == 0)	/* 特殊事件 */
      {
	prints("\033[1;3%dm%s\033[36m", run + 1, racename[run]);

	switch (rnd(14))
	{
	case 0:
	  /* 服下威而剛，速度 x1.5\033[m */
	  outs("\xAA\x41\xA4\x55\xAB\xC2\xA6\xD3\xAD\xE8\xA1\x41\xB3\x74\xAB\xD7 x1.5\033[m");
	  speed[run] *= 1.5;
	  break;
	case 1:
	  /* 使出熊的爆發力，前進五格\033[m */
	  outs("\xA8\xCF\xA5\x58\xBA\xB5\xAA\xBA\xC3\x7A\xB5\x6F\xA4\x4F\xA1\x41\xAB\x65\xB6\x69\xA4\xAD\xAE\xE6\033[m");
	  win = race_path(run, pace[run] / 100, 5);
	  pace[run] += 500;
	  break;
	case 2:
	  /* 踩到地雷，速度減半\033[m */
	  outs("\xBD\xF2\xA8\xEC\xA6\x61\xB9\x70\xA1\x41\xB3\x74\xAB\xD7\xB4\xEE\xA5\x62\033[m");
	  speed[run] /= 2;
	  break;
	case 3:
	  /* 踩到香蕉皮滑倒，暫停二次\033[m */
	  outs("\xBD\xF2\xA8\xEC\xAD\xBB\xBF\xBC\xA5\xD6\xB7\xC6\xAD\xCB\xA1\x41\xBC\xC8\xB0\xB1\xA4\x47\xA6\xB8\033[m");
	  stop[run] += 2;
	  break;
	case 4:
	  /* 請神上身，暫停四次，速度加倍\033[m */
	  outs("\xBD\xD0\xAF\xAB\xA4\x57\xA8\xAD\xA1\x41\xBC\xC8\xB0\xB1\xA5\x7C\xA6\xB8\xA1\x41\xB3\x74\xAB\xD7\xA5\x5B\xAD\xBF\033[m");
	  stop[run] += 4;
	  speed[run] *= 2;
	  break;
	case 5:
	  /* 唱出大魔法咒，使其他人暫停三次\033[m */
	  outs("\xB0\xDB\xA5\x58\xA4\x6A\xC5\x5D\xAA\x6B\xA9\x47\xA1\x41\xA8\xCF\xA8\xE4\xA5\x4C\xA4\x48\xBC\xC8\xB0\xB1\xA4\x54\xA6\xB8\033[m");
	  for (i = 0; i < 5 && i != run; i++)
	    stop[i] += 3;
	  break;
	case 6:
	  /* 聽見 badboy 的加油聲，速度 +100\033[m */
	  outs("\xC5\xA5\xA8\xA3 badboy \xAA\xBA\xA5\x5B\xAA\x6F\xC1\x6E\xA1\x41\xB3\x74\xAB\xD7 +100\033[m");
	  speed[run] += 100;
	  break;
	case 7:
	  /* 使出鋼鐵變身，前進三格，速度 +30\033[m */
	  outs("\xA8\xCF\xA5\x58\xBF\xFB\xC5\x4B\xC5\xDC\xA8\xAD\xA1\x41\xAB\x65\xB6\x69\xA4\x54\xAE\xE6\xA1\x41\xB3\x74\xAB\xD7 +30\033[m");
	  win = race_path(run, pace[run] / 100, 3);
	  speed[run] += 30;
	  break;
	case 8:
	  /* 衰神上身速度減半，旁邊暫停二次\033[m */
	  outs("\xB0\x49\xAF\xAB\xA4\x57\xA8\xAD\xB3\x74\xAB\xD7\xB4\xEE\xA5\x62\xA1\x41\xAE\xC7\xC3\xE4\xBC\xC8\xB0\xB1\xA4\x47\xA6\xB8\033[m");
	  speed[run] /= 2;
	  if (run > 0)
	    stop[run - 1] += 2;
	  if (run < 4)
	    stop[run + 1] += 2;
	  break;
	case 9:
	  /* 被詛咒，回到起點\033[m */
	  outs("\xB3\x51\xB6\x41\xA9\x47\xA1\x41\xA6\x5E\xA8\xEC\xB0\x5F\xC2\x49\033[m");
	  win = race_path(run, pace[run] / 100, -30);
	  break;
	case 10:
	  if (pace[0] + pace[1] + pace[2] + pace[3] + pace[4] > 6000)
	  {
	    /* \033[5m使出凱化薰陶，所有人回到起點\033[m */
	    outs("\033[5m\xA8\xCF\xA5\x58\xB3\xCD\xA4\xC6\xC2\xC8\xB3\xB3\xA1\x41\xA9\xD2\xA6\xB3\xA4\x48\xA6\x5E\xA8\xEC\xB0\x5F\xC2\x49\033[m");
	    for (i = 0; i < 5; i++)
	      win = race_path(i, pace[i] / 100, -30);
	  }
	  else
	  {
	    /* 使出企鵝彈跳，速度 x1.3，其他人減半\033[m */
	    outs("\xA8\xCF\xA5\x58\xA5\xF8\xC3\x5A\xBC\x75\xB8\xF5\xA1\x41\xB3\x74\xAB\xD7 x1.3\xA1\x41\xA8\xE4\xA5\x4C\xA4\x48\xB4\xEE\xA5\x62\033[m");
	    for (i = 0; i < 5 && i != run; i++)
	      speed[i] /= 2;
	    speed[run] *= 1.3;
	  }
	  break;
	case 11:
	  if (money[run])
	  {
	    /* 撿到很多錢，暫停一次\033[m */
	    outs("\xBE\xDF\xA8\xEC\xAB\xDC\xA6\x68\xBF\xFA\xA1\x41\xBC\xC8\xB0\xB1\xA4\x40\xA6\xB8\033[m");
	    addmoney(money[run]);
	    out_song();
	    stop[run]++;
	  }
	  else
	  {
	    /* 整匹馬爽起來了，速度 +50\033[m */
	    outs("\xBE\xE3\xA4\xC7\xB0\xA8\xB2\x6E\xB0\x5F\xA8\xD3\xA4\x46\xA1\x41\xB3\x74\xAB\xD7 +50\033[m");
	    speed[run] += 50;
	  }
	  break;
	case 12:
	  j = rnd(5);
	  /* 愛上了[3%dm%s[36m，速度跟牠一樣 */
	  prints("\xB7\x52\xA4\x57\xA4\x46[3%dm%s[36m\xA1\x41\xB3\x74\xAB\xD7\xB8\xF2\xA8\x65\xA4\x40\xBC\xCB", j + 1, racename[j]);
	  speed[run] = speed[j];
	  break;
	case 13:
	  if (money[run] > 0)
	  {
	    /* 的賭金 x1.5，賺啦！\033[m */
	    outs("\xAA\xBA\xBD\xE4\xAA\xF7 x1.5\xA1\x41\xC1\xC8\xB0\xD5\xA1\x49\033[m");
	    money[run] *= 1.5;
	    move(7, 15);
	    clrtoeol();
	    outs("\033[1m");
	    for (i = 0; i < 5; i++)
	      prints("     \033[3%dm%7d ", i + 1, money[i]);
	    outs("\033[m");
	  }
	  else
	  {
	    /* 鞋子掉了，退後三格\033[m */
	    outs("\xBE\x63\xA4\x6C\xB1\xBC\xA4\x46\xA1\x41\xB0\x68\xAB\xE1\xA4\x54\xAE\xE6\033[m");
	    race_path(run, pace[run] / 100, -3);
	  }
	  break;
	}
      }
      else		/* 往前跑 */
      {
	if (stop[run])
	{
	  /* \033[1;3%dm%s\033[37m 爬不起來\033[m */
	  prints("\033[1;3%dm%s\033[37m \xAA\xA6\xA4\xA3\xB0\x5F\xA8\xD3\033[m", run + 1, racename[run]);
	  stop[run]--;
	}
	else
	{
	  /* \033[1;3%dm%s\033[37m 拚命奔跑\033[m */
	  prints("\033[1;3%dm%s\033[37m \xA9\xE9\xA9\x52\xA9\x62\xB6\x5D\033[m", run + 1, racename[run]);
	  i = pace[run] / 100;
	  win = race_path(run, i, (pace[run] + speed[run]) / 100 - i);
	  pace[run] += speed[run];
	}
      }
      flag++;
    }
    move(b_lines - 1, 0);
    /* \033[1;35m★ \033[37m遊戲結束 \033[35m★ \033[37m獲勝的是\033[3%dm %s \033[m */
    prints("\033[1;35m\xA1\xB9 \033[37m\xB9\x43\xC0\xB8\xB5\xB2\xA7\xF4 \033[35m\xA1\xB9 \033[37m\xC0\xF2\xB3\xD3\xAA\xBA\xAC\x4F\033[3%dm %s \033[m",
      win + 1, racename[win]);
    if (money[win])
    {
      money[win] += money[win] * (pace[win] - (pace[0] + pace[1] + pace[2] + pace[3] + pace[4]) / 5) / 500;
      /* 恭喜您押中了，獲得獎金 %d 元 */
      sprintf(buf, "\xAE\xA5\xB3\xDF\xB1\x7A\xA9\xE3\xA4\xA4\xA4\x46\xA1\x41\xC0\xF2\xB1\x6F\xBC\xFA\xAA\xF7 %d \xA4\xB8", money[win]);
      addmoney(money[win]);
    }
    else
    {
      /* 抱歉...您沒押中ㄛ~~~ */
      strcpy(buf, "\xA9\xEA\xBA\x70...\xB1\x7A\xA8\x53\xA9\xE3\xA4\xA4\xA3\xAC~~~");
    }
    vmsg(buf);
  }

abort_game:
  return 0;
}
#endif				/* HAVE_GAME */
