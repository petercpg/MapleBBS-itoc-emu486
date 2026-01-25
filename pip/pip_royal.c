/* ----------------------------------------------------- */
/* pip_royal.c     ( NTHU CS MapleBBS Ver 3.10 )         */
/* ----------------------------------------------------- */
/* target : 小雞 royal                                   */
/* create :   /  /                                       */
/* update : 01/08/14                                     */
/* author : dsyan.bbs@forever.twbbs.org                  */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/* ----------------------------------------------------- */


#include "bbs.h"

#ifdef HAVE_GAME

#include "pip.h"


/* royalset:  num name needmode needvalue addtoman maxtoman words1 words2 */

struct royalset royallist[] = 
{
  /* 拜訪對象 */
  "T", "\xAB\xF4\xB3\x58\xB9\xEF\xB6\x48",   0,   0,   0,   0, NULL, NULL,
  /* 皇城騎兵連 */
  /* 您真好，來陪我聊天.. */
  /* 守衛星空的安全是很辛苦的.. */
  "A", "\xAC\xD3\xAB\xB0\xC3\x4D\xA7\x4C\xB3\x73", 1,  10,  15, 100, "\xB1\x7A\xAF\x75\xA6\x6E\xA1\x41\xA8\xD3\xB3\xAD\xA7\xDA\xB2\xE1\xA4\xD1..",		"\xA6\x75\xBD\xC3\xAC\x50\xAA\xC5\xAA\xBA\xA6\x77\xA5\xFE\xAC\x4F\xAB\xDC\xA8\xAF\xAD\x57\xAA\xBA..",
  /* ００７特務 */
  /* 真是禮貌的小雞..我喜歡.. */
  /* 特務就是秘密保護站長安全的人.. */
  "B", "\xA2\xAF\xA2\xAF\xA2\xB6\xAF\x53\xB0\xC8", 1, 100,  25, 200, "\xAF\x75\xAC\x4F\xC2\xA7\xBB\xAA\xAA\xBA\xA4\x70\xC2\xFB..\xA7\xDA\xB3\xDF\xC5\x77..",	"\xAF\x53\xB0\xC8\xB4\x4E\xAC\x4F\xAF\xB5\xB1\x4B\xAB\x4F\xC5\x40\xAF\xB8\xAA\xF8\xA6\x77\xA5\xFE\xAA\xBA\xA4\x48..",
  /* 鎮國大將軍 */
  /* 當年那個戰役很精彩喔.. */
  /* 您真是高貴優雅的小雞.. */
  "C", "\xC2\xED\xB0\xEA\xA4\x6A\xB1\x4E\xAD\x78", 1, 200,  30, 250, "\xB7\xED\xA6\x7E\xA8\xBA\xAD\xD3\xBE\xD4\xA7\xD0\xAB\xDC\xBA\xEB\xB1\x6D\xB3\xE1..",	"\xB1\x7A\xAF\x75\xAC\x4F\xB0\xAA\xB6\x51\xC0\x75\xB6\xAE\xAA\xBA\xA4\x70\xC2\xFB..",
  /* 參謀總務長 */
  /* 我幫站長管理這個國家唷.. */
  /* 您的聲音很好聽耶..我很喜歡喔..:) */
  "D", "\xB0\xD1\xBF\xD1\xC1\x60\xB0\xC8\xAA\xF8", 1, 300,  35, 300, "\xA7\xDA\xC0\xB0\xAF\xB8\xAA\xF8\xBA\xDE\xB2\x7A\xB3\x6F\xAD\xD3\xB0\xEA\xAE\x61\xAD\xF2..",	"\xB1\x7A\xAA\xBA\xC1\x6E\xAD\xB5\xAB\xDC\xA6\x6E\xC5\xA5\xAD\x43..\xA7\xDA\xAB\xDC\xB3\xDF\xC5\x77\xB3\xE1..:)",
  /* 管理副站長 */
  /* 您很有教養唷！很高興認識您.. */
  /* 優雅的您，請讓我幫您祈福.. */
  "E", "\xBA\xDE\xB2\x7A\xB0\xC6\xAF\xB8\xAA\xF8", 1, 400,  35, 300, "\xB1\x7A\xAB\xDC\xA6\xB3\xB1\xD0\xBE\x69\xAD\xF2\xA1\x49\xAB\xDC\xB0\xAA\xBF\xB3\xBB\x7B\xC3\xD1\xB1\x7A..",	"\xC0\x75\xB6\xAE\xAA\xBA\xB1\x7A\xA1\x41\xBD\xD0\xC5\xFD\xA7\xDA\xC0\xB0\xB1\x7A\xAC\xE8\xBA\xD6..",
  /* 系統站長 */
  /* 您好可愛喔..我喜歡您唷.. */
  /* 對啦..以後要多多來和我玩喔.. */
  "F", "\xA8\x74\xB2\xCE\xAF\xB8\xAA\xF8",   1, 500,  40, 350, "\xB1\x7A\xA6\x6E\xA5\x69\xB7\x52\xB3\xE1..\xA7\xDA\xB3\xDF\xC5\x77\xB1\x7A\xAD\xF2..",	"\xB9\xEF\xB0\xD5..\xA5\x48\xAB\xE1\xAD\x6E\xA6\x68\xA6\x68\xA8\xD3\xA9\x4D\xA7\xDA\xAA\xB1\xB3\xE1..",
  /* 程式站長 */
  /* 告訴您唷，跟您講話很快樂喔.. */
  /* 來，坐我膝蓋上，聽我講故事.. */
  "G", "\xB5\x7B\xA6\xA1\xAF\xB8\xAA\xF8",   1, 550,  40, 350, "\xA7\x69\xB6\x44\xB1\x7A\xAD\xF2\xA1\x41\xB8\xF2\xB1\x7A\xC1\xBF\xB8\xDC\xAB\xDC\xA7\xD6\xBC\xD6\xB3\xE1..",	"\xA8\xD3\xA1\x41\xA7\xA4\xA7\xDA\xBD\xA5\xBB\x5C\xA4\x57\xA1\x41\xC5\xA5\xA7\xDA\xC1\xBF\xAC\x47\xA8\xC6..",
  /* 一站之長責任重大呀..:).. */
  /* 謝謝您聽我講話..以後要多來喔.. */
  "H", SYSOPNICK,    1, 600,  50, 400, "\xA4\x40\xAF\xB8\xA4\xA7\xAA\xF8\xB3\x64\xA5\xF4\xAD\xAB\xA4\x6A\xA7\x72..:)..",	"\xC1\xC2\xC1\xC2\xB1\x7A\xC5\xA5\xA7\xDA\xC1\xBF\xB8\xDC..\xA5\x48\xAB\xE1\xAD\x6E\xA6\x68\xA8\xD3\xB3\xE1..",
  /* 瘋狂灌水群 */
  /* 不錯唷..蠻機靈的喔..很可愛.. */
  /* 來，我們一起來灌水吧.. */
  "I", "\xBA\xC6\xA8\x67\xC4\xE9\xA4\xF4\xB8\x73", 2,  60,  20, 150, "\xA4\xA3\xBF\xF9\xAD\xF2..\xC6\x5A\xBE\xF7\xC6\x46\xAA\xBA\xB3\xE1..\xAB\xDC\xA5\x69\xB7\x52..",	"\xA8\xD3\xA1\x41\xA7\xDA\xAD\xCC\xA4\x40\xB0\x5F\xA8\xD3\xC4\xE9\xA4\xF4\xA7\x61..",
  /* 青年帥武官 */
  /* 您好，我是武官，剛從邊境回來 */
  /* 希望下次還能見到您..:) */
  "J", "\xAB\x43\xA6\x7E\xAB\xD3\xAA\x5A\xA9\x78", 0,   0,   0,   0, "\xB1\x7A\xA6\x6E\xA1\x41\xA7\xDA\xAC\x4F\xAA\x5A\xA9\x78\xA1\x41\xAD\xE8\xB1\x71\xC3\xE4\xB9\xD2\xA6\x5E\xA8\xD3",	"\xA7\xC6\xB1\xE6\xA4\x55\xA6\xB8\xC1\xD9\xAF\xE0\xA8\xA3\xA8\xEC\xB1\x7A..:)",
  NULL, NULL,        0,   0,   0,   0, NULL, NULL
};


static int
pip_go_palace_screen(p)
  struct royalset *p;
{
  char inbuf1[128], inbuf2[20];
  /* 禮儀表現＞ */
  /* 談吐技巧＞ */
  char *needmode[3] = {"      ", "\xC2\xA7\xBB\xF6\xAA\xED\xB2\x7B\xA1\xD6", "\xBD\xCD\xA6\x52\xA7\xDE\xA5\xA9\xA1\xD6"};
  int n, a, b, choice, change;
  int save[11];

  /* 秀出所有可以拜訪的人 */
  clear();
  show_palace_pic(0);
  move(13, 0);
  /*     \033[1;31m┌──────┤\033[37;41m 來到總司令部了   請選擇您欲拜訪的對象\033[0;1;31m├──────┐\033[m\n */
  outs("    \033[1;31m\xA2\x7A\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x74\033[37;41m \xA8\xD3\xA8\xEC\xC1\x60\xA5\x71\xA5\x4F\xB3\xA1\xA4\x46   \xBD\xD0\xBF\xEF\xBE\xDC\xB1\x7A\xB1\xFD\xAB\xF4\xB3\x58\xAA\xBA\xB9\xEF\xB6\x48\033[0;1;31m\xA2\x75\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x7B\033[m\n");
  /*     \033[1;31m│                                                                  │\033[m\n */
  outs("    \033[1;31m\xA2\x78                                                                  \xA2\x78\033[m\n");

  for (n = 0; n < 5; n++)
  {
    a = 2 * n + 1;
    b = 2 * n + 2;
    sprintf(inbuf1, "%-10s%3d", needmode[p[a].needmode], p[a].needvalue);

    if (n == 4)	/* 王子 */
      sprintf(inbuf2, "%-10s", needmode[p[b].needmode]);
    else
      sprintf(inbuf2, "%-10s%3d", needmode[p[b].needmode], p[b].needvalue);

    if ((d.seeroyalJ == 1 && n == 4) || (n != 4))
    {
      /*     \033[1;31m│\033[36m(\033[37m%s\033[36m)\033[33m%-10s \033[37m%-14s    \033[36m(\033[37m%s\033[36m)\033[33m%-10s \033[37m%-14s      \033[31m│\033[m\n */
      prints("    \033[1;31m\xA2\x78\033[36m(\033[37m%s\033[36m)\033[33m%-10s \033[37m%-14s    \033[36m(\033[37m%s\033[36m)\033[33m%-10s \033[37m%-14s      \033[31m\xA2\x78\033[m\n",
	p[a].num, p[a].name, inbuf1, p[b].num, p[b].name, inbuf2);
    }
    else
    {
      /*     \033[1;31m│\033[36m(\033[37m%s\033[36m)\033[33m%-10s \033[37m%-14s                                        \033[31m│\033[0m\n */
      prints("    \033[1;31m\xA2\x78\033[36m(\033[37m%s\033[36m)\033[33m%-10s \033[37m%-14s                                        \033[31m\xA2\x78\033[0m\n", 
  	p[a].num, p[a].name, inbuf1);
    }
  }
  /*     \033[1;31m│                                                                  │\033[m\n */
  outs("    \033[1;31m\xA2\x78                                                                  \xA2\x78\033[m\n");
  /*     \033[1;31m└─────────────────────────────────┘\033[m */
  outs("    \033[1;31m\xA2\x7C\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x7D\033[m");

  while (1)
  {
    /*  [生命力] %6d/%6d  [疲勞度] %6d                                      \033[m */
    sprintf(inbuf1, COLOR1 " [\xA5\xCD\xA9\x52\xA4\x4F] %6d/%6d  [\xAF\x68\xB3\xD2\xAB\xD7] %6d                                      \033[m", d.hp, d.maxhp, d.tired);
    /*  參見選單  */
    /*  [字母]選擇欲拜訪的人物 [Q]離開總司令部                             \033[m */
    out_cmd(inbuf1, COLOR1 " \xB0\xD1\xA8\xA3\xBF\xEF\xB3\xE6 " COLOR2 " [\xA6\x72\xA5\xC0]\xBF\xEF\xBE\xDC\xB1\xFD\xAB\xF4\xB3\x58\xAA\xBA\xA4\x48\xAA\xAB [Q]\xC2\xF7\xB6\x7D\xC1\x60\xA5\x71\xA5\x4F\xB3\xA1                             \033[m");

    choice = vkey();
    if (choice == 'q' || choice == KEY_LEFT)
    {
      /* 離開 */
      /* 總司令部..... */
      vmsg("\xC2\xF7\xB6\x7D" BBSNAME "\xC1\x60\xA5\x71\xA5\x4F\xB3\xA1.....");
      return 0;
    }

    /* 將各人物已經給與的數值先儲存起來*/
    save[1] = d.royalA;		/* from守衛 */
    save[2] = d.royalB;		/* from近衛 */
    save[3] = d.royalC;		/* from將軍 */
    save[4] = d.royalD;		/* from大臣 */
    save[5] = d.royalE;		/* from祭司 */
    save[6] = d.royalF;		/* from寵妃 */
    save[7] = d.royalG;		/* from王妃 */
    save[8] = d.royalH;		/* from國王 */
    save[9] = d.royalI;		/* from小丑 */
    save[10] = d.royalJ;	/* from王子 */

    choice -= 'a' - 1;

    if ((choice >= 1 && choice <= 10 && d.seeroyalJ == 1) || (choice >= 1 && choice <= 9 && d.seeroyalJ == 0))
    {
      d.social += rand() % 3 + 3;
      d.hp -= rand() % 5 + 6;
      d.tired += rand() % 5 + 8;

      if ((p[choice].needmode == 0) || (p[choice].needmode == 1 && d.manners >= p[choice].needvalue) || 
        (p[choice].needmode == 2 && d.speech >= p[choice].needvalue))
      {
	if (choice >= 1 && choice <= 9 && save[choice] >= p[choice].maxtoman)
	{
	  /* 能和這麼偉大的您講話真是榮幸ㄚ... */
	  /* 很高興您來拜訪我，但我不能給您什麼了.. */
	  vmsg(rand() % 2 ? "\xAF\xE0\xA9\x4D\xB3\x6F\xBB\xF2\xB0\xB6\xA4\x6A\xAA\xBA\xB1\x7A\xC1\xBF\xB8\xDC\xAF\x75\xAC\x4F\xBA\x61\xA9\xAF\xA3\xAB..." : "\xAB\xDC\xB0\xAA\xBF\xB3\xB1\x7A\xA8\xD3\xAB\xF4\xB3\x58\xA7\xDA\xA1\x41\xA6\xFD\xA7\xDA\xA4\xA3\xAF\xE0\xB5\xB9\xB1\x7A\xA4\xB0\xBB\xF2\xA4\x46..");
	}
	else
	{
	  if (choice >= 1 && choice <= 8)	/* 晉見官員，增加待人接物 */
	  {
	    switch (choice)
	    {
	    case 1:
	      change = d.character / 5;
	      break;
	    case 2:
	      change = d.character / 8;
	      break;
	    case 3:
	      change = d.charm / 5;
	      break;
	    case 4:
	      change = d.wisdom / 10;
	      break;
	    case 5:
	      change = d.belief / 10;
	      break;
	    case 6:
	      change = d.speech / 10;
	      break;
	    case 7:
	      change = d.social / 10;
	      break;
	    case 8:
	      change = d.hexp / 10;
	      break;
	    }

	    if (change > p[choice].addtoman)		/* 如果大於每次的增加最大量 */
	      change = p[choice].addtoman;
	    else if ((change + save[choice]) >= p[choice].maxtoman)	/* 如果加上原先的之後大於所能給的所有值時 */
	      change = p[choice].maxtoman - save[choice];

	    save[choice] += change;
	    d.toman += change;
	  }
	  else if (choice == 9)			/* 找小丑 */
	  {
	    save[9] = 0;
	    d.social -= 13 + rand() % 4;
	    d.affect += 13 + rand() % 4;
	  }
	  else if (choice == 10 && d.seeroyalJ == 1)	/* 拜訪王子 */
	  {
	    save[10] += 15 + rand() % 4;
	    d.seeroyalJ = 0;
	  }

	  vmsg(rand() % 2 ? p[choice].words1 : p[choice].words2);
	}
      }
      else
      {
	/* 我不和這樣的雞談話.... */
	/* 沒教養的雞，再去學學禮儀吧.... */
	vmsg(rand() % 2 ? "\xA7\xDA\xA4\xA3\xA9\x4D\xB3\x6F\xBC\xCB\xAA\xBA\xC2\xFB\xBD\xCD\xB8\xDC...." : "\xA8\x53\xB1\xD0\xBE\x69\xAA\xBA\xC2\xFB\xA1\x41\xA6\x41\xA5\x68\xBE\xC7\xBE\xC7\xC2\xA7\xBB\xF6\xA7\x61....");
      }
    }

    d.royalA = save[1];
    d.royalB = save[2];
    d.royalC = save[3];
    d.royalD = save[4];
    d.royalE = save[5];
    d.royalF = save[6];
    d.royalG = save[7];
    d.royalH = save[8];
    d.royalI = save[9];
    d.royalJ = save[10];
  }
}


int
pip_go_palace()			/* 參見 */
{
  pip_go_palace_screen(royallist);
  return 0;
}
#endif	/* HAVE_GAME */
