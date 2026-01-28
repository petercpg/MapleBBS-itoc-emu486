/*-------------------------------------------------------*/
/* pip/pip_play.c       ( NTHU CS MapleBBS Ver 3.10 )    */
/*-------------------------------------------------------*/
/* target : 玩樂選單                                     */
/* create :   /  /                                       */
/* update : 01/08/15                                     */
/* author : dsyan.bbs@forever.twbbs.org                  */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_GAME

#include "pip.h"


/*-------------------------------------------------------*/
/* 玩樂選單:散步 旅遊 運動 約會 猜拳			 */
/*-------------------------------------------------------*/


int
pip_play_stroll()		/* 散步 */
{
  /* 預設改變值，若有偶發事件，另外加成於下 */
  count_tired(3, 3, 1, 100, 0);	/* 增加疲勞 */
  d.happy += rand() % 3 + 3;
  d.satisfy += rand() % 2 + 1;
  d.shit += rand() % 3 + 2;
  d.hp -= rand() % 3 + 2;

  switch (rand() % 10)
  {
  case 0:
    d.happy += 6;
    d.satisfy += 6;
    show_play_pic(1);
    /* 遇到朋友囉  真好.... ^_^ */
    vmsg("\xB9\x4A\xA8\xEC\xAA\x42\xA4\xCD\xC5\x6F  \xAF\x75\xA6\x6E.... ^_^");
    break;

  case 1:
    d.happy += 4;
    d.satisfy += 8;
    show_play_pic(2);
    /* 看到漂亮的女生囉  真好.... ^_^ */
    /* 看到英俊的男生囉  真好.... ^_^ */
    vmsg(d.sex == 1 ? "\xAC\xDD\xA8\xEC\xBA\x7D\xAB\x47\xAA\xBA\xA4\x6B\xA5\xCD\xC5\x6F  \xAF\x75\xA6\x6E.... ^_^" : "\xAC\xDD\xA8\xEC\xAD\x5E\xAB\x54\xAA\xBA\xA8\x6B\xA5\xCD\xC5\x6F  \xAF\x75\xA6\x6E.... ^_^");
    break;

  case 2:
    d.money += 100;
    d.happy += 4;
    show_play_pic(3);
    /* 撿到了100元了..耶耶耶.... */
    vmsg("\xBE\xDF\xA8\xEC\xA4\x46""100\xA4\xB8\xA4\x46..\xAD\x43\xAD\x43\xAD\x43....");
    break;

  case 3:
    d.happy -= 10;
    d.satisfy -= 3;
    show_play_pic(4);
    if (d.money > 50)
    {
      d.money -= 50;
      /* 掉了50元了..嗚嗚嗚.... */
      vmsg("\xB1\xBC\xA4\x46""50\xA4\xB8\xA4\x46..\xB6\xE3\xB6\xE3\xB6\xE3....");
    }
    else
    {
      d.money = 0;
      /* 錢掉光光了..嗚嗚嗚.... */
      vmsg("\xBF\xFA\xB1\xBC\xA5\xFA\xA5\xFA\xA4\x46..\xB6\xE3\xB6\xE3\xB6\xE3....");
    }
    break;

  case 4:
    d.happy += 3;
    show_play_pic(5);
    if (d.money > 50)
    {
      d.money -= 50;
      /* 用了50元了..不可以罵我喔.... */
      vmsg("\xA5\xCE\xA4\x46""50\xA4\xB8\xA4\x46..\xA4\xA3\xA5\x69\xA5\x48\xBD\x7C\xA7\xDA\xB3\xE1....");
    }
    else
    {
      d.money = 0;
      /* 錢被我偷用光光了..:p */
      vmsg("\xBF\xFA\xB3\x51\xA7\xDA\xB0\xBD\xA5\xCE\xA5\xFA\xA5\xFA\xA4\x46..:p");
    }
    break;

  case 5:
    d.toy++;
    show_play_pic(6);
    /* 好棒喔，撿到玩具了說..... */
    vmsg("\xA6\x6E\xB4\xCE\xB3\xE1\xA1\x41\xBE\xDF\xA8\xEC\xAA\xB1\xA8\xE3\xA4\x46\xBB\xA1.....");
    break;

  case 6:
    d.cookie++;
    show_play_pic(7);
    /* 好棒喔，撿到餅乾了說..... */
    vmsg("\xA6\x6E\xB4\xCE\xB3\xE1\xA1\x41\xBE\xDF\xA8\xEC\xBB\xE6\xB0\xAE\xA4\x46\xBB\xA1.....");
    break;

  case 7:
    d.satisfy -= 5;
    d.shit += 5;
    show_play_pic(9);
    /* 真是倒楣  可以去買愛國獎券 */
    vmsg("\xAF\x75\xAC\x4F\xAD\xCB\xB7\xB0  \xA5\x69\xA5\x48\xA5\x68\xB6\x52\xB7\x52\xB0\xEA\xBC\xFA\xA8\xE9");
    break;

  default:
    show_play_pic(8);
    /* 沒有特別的事發生啦..... */
    vmsg("\xA8\x53\xA6\xB3\xAF\x53\xA7\x4F\xAA\xBA\xA8\xC6\xB5\x6F\xA5\xCD\xB0\xD5.....");
    break;
  }

  if (d.happy > 100)
    d.happy = 100;
  if (d.satisfy > 100)
    d.satisfy = 100;

  return 0;
}


int
pip_play_sport()		/* 運動 */
{
  count_tired(3, 8, 1, 100, 1);
  d.speed += 2 + rand() % 3;
  d.weight -= rand() % 3 + 2;
  d.shit += rand() % 5 + 10;
  d.hp -= rand() % 2 + 8;
  d.satisfy += rand() % 2 + 3;
  if (d.satisfy > 100)
    d.satisfy = 100;

  show_play_pic(10);
  /* 運動好處多多啦... */
  vmsg("\xB9\x42\xB0\xCA\xA6\x6E\xB3\x42\xA6\x68\xA6\x68\xB0\xD5...");

  return 0;
}


int
pip_play_date()			/* 約會 */
{
  if (d.money < 150)
  {
    /* 錢不夠多啦！約會總得花點錢錢 */
    vmsg("\xBF\xFA\xA4\xA3\xB0\xF7\xA6\x68\xB0\xD5\xA1\x49\xAC\xF9\xB7\x7C\xC1\x60\xB1\x6F\xAA\xE1\xC2\x49\xBF\xFA\xBF\xFA");
  }
  else
  {
    count_tired(3, 6, 1, 100, 1);
    d.money -= 150;
    d.shit += rand() % 3 + 5;
    d.hp -= rand() % 4 + 8;
    d.character += rand() % 3 + 1;
    d.happy += rand() % 5 + 12;
    d.satisfy += rand() % 5 + 7;

    if (d.happy > 100)
      d.happy = 100;
    if (d.satisfy > 100)
      d.satisfy = 100;

    show_play_pic(11);
    /* 約會去  呼呼 */
    vmsg("\xAC\xF9\xB7\x7C\xA5\x68  \xA9\x49\xA9\x49");
  }
  return 0;
}


int
pip_play_outing()		/* 郊遊 */
{
  if (d.money < 250)
  {
    /* 錢不夠多啦！旅遊總得花點錢錢 */
    vmsg("\xBF\xFA\xA4\xA3\xB0\xF7\xA6\x68\xB0\xD5\xA1\x49\xAE\xC8\xB9\x43\xC1\x60\xB1\x6F\xAA\xE1\xC2\x49\xBF\xFA\xBF\xFA");
  }
  else
  {
    count_tired(10, 45, 0, 100, 0);
    d.money -= 250;
    d.weight += rand() % 2 + 1;
    d.hp -= rand() % 7 + 15;
    d.character += rand() % 5 + 5;
    d.happy += rand() % 10 + 12;
    d.satisfy += rand() % 10 + 10;

    if (d.happy > 100)
      d.happy = 100;
    if (d.satisfy > 100)
      d.satisfy = 100;

    switch (rand() % 4)
    {
    case 0:
      d.art += rand() % 2;
      show_play_pic(12);
      /* 心中有一股淡淡的感覺  好舒服喔.... */
      /* 雲水 閑情 心情好多了..... */
      vmsg(rand() % 2 ? "\xA4\xDF\xA4\xA4\xA6\xB3\xA4\x40\xAA\xD1\xB2\x48\xB2\x48\xAA\xBA\xB7\x50\xC4\xB1  \xA6\x6E\xB5\xCE\xAA\x41\xB3\xE1...." : "\xB6\xB3\xA4\xF4 \xB6\x7E\xB1\xA1 \xA4\xDF\xB1\xA1\xA6\x6E\xA6\x68\xA4\x46.....");
      break;

    case 1:
      d.art += rand() % 3;
      show_play_pic(13);
      /* 有山有水有落日  形成一幅美麗的畫.. */
      /* 看著看著  全身疲憊都不見囉.. */
      vmsg(rand() % 2 ? "\xA6\xB3\xA4\x73\xA6\xB3\xA4\xF4\xA6\xB3\xB8\xA8\xA4\xE9  \xA7\xCE\xA6\xA8\xA4\x40\xB4\x54\xAC\xFC\xC4\x52\xAA\xBA\xB5\x65.." : "\xAC\xDD\xB5\xDB\xAC\xDD\xB5\xDB  \xA5\xFE\xA8\xAD\xAF\x68\xBE\xCE\xB3\xA3\xA4\xA3\xA8\xA3\xC5\x6F..");
      break;

    case 2:
      d.love += rand() % 3;
      show_play_pic(14);
      /* 看  太陽快沒入水中囉... */
      /* 真是一幅美景 */
      vmsg(rand() % 2 ? "\xAC\xDD  \xA4\xD3\xB6\xA7\xA7\xD6\xA8\x53\xA4\x4A\xA4\xF4\xA4\xA4\xC5\x6F..." : "\xAF\x75\xAC\x4F\xA4\x40\xB4\x54\xAC\xFC\xB4\xBA");
      break;

    case 3:
      d.hp += d.maxhp;
      show_play_pic(15);
      /* 讓我們瘋狂在夜裡的海灘吧....呼呼.. */
      /* 涼爽的海風迎面襲來  最喜歡這種感覺了.... */
      vmsg(rand() % 2 ? "\xC5\xFD\xA7\xDA\xAD\xCC\xBA\xC6\xA8\x67\xA6\x62\xA9\x5D\xB8\xCC\xAA\xBA\xAE\xFC\xC5\x79\xA7\x61....\xA9\x49\xA9\x49.." : "\xB2\x44\xB2\x6E\xAA\xBA\xAE\xFC\xAD\xB7\xAA\xEF\xAD\xB1\xC5\xA7\xA8\xD3  \xB3\xCC\xB3\xDF\xC5\x77\xB3\x6F\xBA\xD8\xB7\x50\xC4\xB1\xA4\x46....");
    }

    /* 隨機遇到天使 */
    if (rand() % 301 == 0)
      pip_meet_angel();
  }

  return 0;
}


int
pip_play_kite()			/* 風箏 */
{
  count_tired(4, 4, 1, 100, 0);
  d.weight += (rand() % 2 + 2);
  d.shit += rand() % 5 + 6;
  d.hp -= rand() % 2 + 7;
  d.affect += rand() % 4 + 6;
  d.happy += rand() % 5 + 10;
  d.satisfy += rand() % 3 + 12;

  if (d.happy > 100)
    d.happy = 100;
  if (d.satisfy > 100)
    d.satisfy = 100;

  show_play_pic(16);
  /* 放風箏真好玩啦... */
  vmsg("\xA9\xF1\xAD\xB7\xBA\xE5\xAF\x75\xA6\x6E\xAA\xB1\xB0\xD5...");
  return 0;
}


int
pip_play_KTV()			/* KTV */
{
  if (d.money < 250)
  {
    /* 錢不夠多啦！唱歌總得花點錢錢 */
    vmsg("\xBF\xFA\xA4\xA3\xB0\xF7\xA6\x68\xB0\xD5\xA1\x49\xB0\xDB\xBA\x71\xC1\x60\xB1\x6F\xAA\xE1\xC2\x49\xBF\xFA\xBF\xFA");
  }
  else
  {
    count_tired(10, 10, 1, 100, 0);
    d.money -= 250;
    d.shit += rand() % 5 + 6;
    d.hp += rand() % 2 + 6;
    d.art += rand() % 4 + 3;
    d.happy += rand() % 3 + 20;
    d.satisfy += rand() % 2 + 20;

    if (d.happy > 100)
      d.happy = 100;
    if (d.satisfy > 100)
      d.satisfy = 100;

    show_play_pic(17);
    /* 二隻老虎..二隻老虎..跑得快..跑得快.. */
    vmsg("\xA4\x47\xB0\xA6\xA6\xD1\xAA\xEA..\xA4\x47\xB0\xA6\xA6\xD1\xAA\xEA..\xB6\x5D\xB1\x6F\xA7\xD6..\xB6\x5D\xB1\x6F\xA7\xD6..");
  }
  return 0;
}


static void
guess_pip_lose()
{
  d.winn++;
  d.shit += rand() % 3 + 2;
  d.hp -= rand() % 2 + 3;
  d.satisfy--;
  d.happy -= 2;
  /* 小雞輸了....~>_<~ */
  outs("\xA4\x70\xC2\xFB\xBF\xE9\xA4\x46....~>_<~");
  show_guess_pic(2);
}


static void
guess_pip_tie()
{
  d.tiee++;
  count_tired(2, 2, 1, 100, 1);
  d.shit += rand() % 3 + 2;
  d.hp -= rand() % 2 + 3;
  d.satisfy++;
  d.happy++;
  /* 平手........-_- */
  outs("\xA5\xAD\xA4\xE2........-_-");
  show_guess_pic(3);
}


static void
guess_pip_win()
{
  d.losee++;
  count_tired(2, 2, 1, 100, 1);
  d.shit += rand() % 3 + 2;
  d.hp -= rand() % 2 + 3;
  d.satisfy += rand() % 3 + 2;
  d.happy += rand() % 3 + 5;
  /* 小雞贏囉....*^_^* */
  outs("\xA4\x70\xC2\xFB\xC4\xB9\xC5\x6F....*^_^*");
  show_guess_pic(1);
}


int
pip_play_guess()		/* 猜拳程式 */
{
  int mankey;		/* 我出的手 */
  int pipkey;		/* 小雞出的手 */
  /* 剪刀 */
  /* 石頭 */
  /* 布   */
  char msg[3][5] = {"\xB0\xC5\xA4\x4D", "\xA5\xDB\xC0\x59", "\xA5\xAC  "};

  /*  猜拳  */
  /*  [1]我出剪刀 [2]我出石頭 [3]我出布啦 [Q]跳出                            \033[m */
  out_cmd("", COLOR1 " \xB2\x71\xAE\xB1 " COLOR2 " [1]\xA7\xDA\xA5\x58\xB0\xC5\xA4\x4D [2]\xA7\xDA\xA5\x58\xA5\xDB\xC0\x59 [3]\xA7\xDA\xA5\x58\xA5\xAC\xB0\xD5 [Q]\xB8\xF5\xA5\x58                            \033[m");

  /* itoc.010814: 可以一直猜拳 */
  while (1)
  {
    /* 我先出 */
    mankey = vkey() - '1';
    if (mankey < 0 || mankey > 2)
      return 0;

    /* 小雞再出 */
    pipkey = rand() % 3;

    /* 在 b_lines - 2 秀全部的勝負訊息 */
    move(b_lines - 2, 0);
    /* 您：%s   小雞：%s     */
    prints("\xB1\x7A\xA1\x47%s   \xA4\x70\xC2\xFB\xA1\x47%s    ", msg[mankey], msg[pipkey]);

    /* 判定勝負 */
    if (mankey == pipkey)	/* 平手 */
      guess_pip_tie();
    else if (pipkey == mankey + 1 || pipkey == mankey - 2)	/* 小雞勝 */
      guess_pip_win();
    else			/* 小雞敗 */
      guess_pip_lose();
  }
}
#endif		/* HAVE_GAME */
