/*-------------------------------------------------------*/
/* pip/pip_race.c       ( NTHU CS MapleBBS Ver 3.10 )    */
/*-------------------------------------------------------*/
/* target : 收穫季比賽                                   */
/* create :   /  /                                       */
/* update : 03/03/31                                     */
/* author : dsyan.bbs@forever.twbbs.org                  */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_GAME

#include "pip.h"


/*-------------------------------------------------------*/
/* 陪賽者名單	 				         */
/*-------------------------------------------------------*/


/* 武鬥大會 */
/* 藝術大展 */
/* 皇家舞會 */
/* 烹飪大賽 */
static char racename[4][9] = {"\xAA\x5A\xB0\xAB\xA4\x6A\xB7\x7C", "\xC3\xC0\xB3\x4E\xA4\x6A\xAE\x69", "\xAC\xD3\xAE\x61\xBB\x52\xB7\x7C", "\xB2\x69\xB6\xB9\xA4\x6A\xC1\xC9"};


/* name[13] attribute hp maxhp attack spirit magic armor dodge money exp pic */
/* 參考 badman_generate() 中的註解，有怪物等級的期望值 */
struct playrule racemanlist[] = 
{
  /* 茱麗葉塔 */
  /* 相當於等級 05 的怪物 */	"\xAF\xFC\xC4\x52\xB8\xAD\xB6\xF0", 0,   30,   30,  50,  50,  50,  50,  50,  50,  25, 001,
  /* 菲歐利娜 */
  /* 相當於等級 10 的怪物 */	"\xB5\xE1\xBC\xDA\xA7\x51\xAE\x52", 0,   95,   95,  99,  99,  99,  99,  99,  99,  50, 002,
  /* 阿妮斯白 */
  /* 相當於等級 20 的怪物 */	"\xAA\xFC\xA9\x67\xB4\xB5\xA5\xD5", 0,  300,  300, 350,  40,  40, 200, 200, 200, 100, 003, 
  /* 帕多雷西 */
  /* 相當於等級 40 的怪物 */	"\xA9\xAC\xA6\x68\xB9\x70\xA6\xE8", 0, 1200, 1200, 350, 600, 200, 600, 100, 400, 200, 004, 
  /* 卡美拉美 */
  /* 相當於等級 60 的怪物 */	"\xA5\x64\xAC\xFC\xA9\xD4\xAC\xFC", 0, 2700, 2700, 600, 600, 600, 600, 600, 600, 300, 005, 
  /* 尼古拉斯 */
  /* 相當於等級 90 的怪物 */ 	"\xA5\xA7\xA5\x6A\xA9\xD4\xB4\xB5", 0, 6100, 6100, 900, 799, 999, 799, 999, 900, 450, 006, 
};

static int player[3];		/* 三位陪賽選手的號碼，強度是 player[2] > player[1] > player[0] */


/*-------------------------------------------------------*/
/* 武鬥大會	 				         */
/*-------------------------------------------------------*/


/* itoc.030331.遊戲設計: 呼叫 pip_vs_man() 這函式進入戰鬥畫面，打贏多少人來算成績 */

static int			/* 回傳: 贏了幾個人 >=3:冠軍 2:亞軍 1:季軍 <=0:最後一名 */
pip_race_eventA()
{
  int i, winorlost;
  char buf[80];

  /* 從 racemanlist 六位中挑出三個陪賽者 */
  player[0] = rand() % 2;
  player[1] = rand() % 2 + 2;
  player[2] = rand() % 2 + 4;

  winorlost = 0;
  for (i = 0; i < 3; i++)
  {
    /* 您的第 %d 個對手是%s */
    sprintf(buf, "\xB1\x7A\xAA\xBA\xB2\xC4 %d \xAD\xD3\xB9\xEF\xA4\xE2\xAC\x4F%s", i + 1, racemanlist[player[i]].name);
    vmsg(buf);

    if (pip_vs_man(racemanlist[player[i]], 0))	/* 小雞對戰敵人 */
      winorlost++;	/* 獲勝 */
  }

  return winorlost;
}


/*-------------------------------------------------------*/
/* 藝術大展	 				         */
/*-------------------------------------------------------*/


/* itoc.030331.遊戲設計: 完全由 d.art 和 d.charm 來決定藝術大展的成績 */

static int			/* 回傳: 贏了幾個人 >=3:冠軍 2:亞軍 1:季軍 <=0:最後一名 */
pip_race_eventB()
{
  /* 從 racemanlist 六位中挑出三個陪賽者 */
  player[0] = rand() % 2 + 4;
  player[1] = rand() % 2;
  player[2] = rand() % 2 + 2;

  /* 直接看能力，沒有比賽過程 */
  return ((d.art * 2 + d.character) / 600);
}


/*-------------------------------------------------------*/
/* 皇家舞會	 				         */
/*-------------------------------------------------------*/


/* itoc.030331.遊戲設計: 完全由 d.art 和 d.charm 來決定皇家舞會的成績 */

static int			/* 回傳: 贏了幾個人 >=3:冠軍 2:亞軍 1:季軍 <=0:最後一名 */
pip_race_eventC()
{
  /* 從 racemanlist 六位中挑出三個陪賽者 */
  player[0] = rand() % 2 + 2;
  player[1] = rand() % 2 + 4;
  player[2] = rand() % 2;

  /* 直接看能力，沒有比賽過程 */
  return ((d.art * 2 + d.charm) / 600);
}


/*-------------------------------------------------------*/
/* 烹飪大賽	 				         */
/*-------------------------------------------------------*/


/* itoc.030331.遊戲設計: 原則上是由 d.cook 和 d.affect 來決定烹飪大賽的成績，
   但是如果菜色和目前的狀態吻合的話有加分效果，反之則有扣分效果 */

static int			/* 回傳: 贏了幾個人 >=3:冠軍 2:亞軍 1:季軍 <=0:最後一名 */
pip_race_eventD()
{
  int winorlost;

  /* 從 racemanlist 六位中挑出三個陪賽者 */
  player[0] = rand() % 2 + 4;
  player[1] = rand() % 2 + 2;
  player[2] = rand() % 2;

  /* 您想煮哪種口味的菜色？0)家常 1)酸 2)甜 3)苦 4)辣 [0]  */
  winorlost = ians(b_lines - 1, 0, "\xB1\x7A\xB7\x51\xB5\x4E\xAD\xFE\xBA\xD8\xA4\x66\xA8\xFD\xAA\xBA\xB5\xE6\xA6\xE2\xA1\x48""0)\xAE\x61\xB1\x60 1)\xBB\xC4 2)\xB2\xA2 3)\xAD\x57 4)\xBB\xB6 [0] ");
  if (winorlost == '1')
    winorlost = 70 - d.satisfy * 2;	/* 越不滿足煮出來的菜才越帶有醋味 (不滿足會吃醋) */
  else if (winorlost == '2')
    winorlost = d.happy * 2 - 130;	/* 越是快樂煮出來的菜才越帶有甜度 (很快樂會甜蜜) */
  else if (winorlost == '3')
    winorlost = d.shit * 2 - 130;	/* 越是骯髒煮出來的菜才越帶有苦處 (大便是苦的:p) */
  else if (winorlost == '4')
    winorlost = d.sick * 2 - 130;	/* 越是生病煮出來的菜才越帶有辣香 (生病味覺不靈) */
  else
    winorlost = 0;			/* 家常菜與狀態無關 */

  winorlost += d.cook * 2 + d.affect;

  return (winorlost / 600);
}


/*-------------------------------------------------------*/
/* 比賽結果	 				         */
/*-------------------------------------------------------*/


static void
pip_race_ending(winorlost, mode)
  int winorlost;		/* 贏了幾個人 >=3:冠軍 2:亞軍 1:季軍 <=0:最後一名 */
  int mode;			/* 參加哪一種比賽 */
{
  char *name1, *name2, *name3, *name4;
  char buf[80];

  if (winorlost <= 0)		/* 最後一名 */
  {
    name1 = racemanlist[player[2]].name;
    name2 = racemanlist[player[1]].name;
    name3 = racemanlist[player[0]].name;
    name4 = d.name;
  }
  else if (winorlost == 1)	/* 季軍獎金 2000 */
  {
    name1 = racemanlist[player[2]].name;
    name2 = racemanlist[player[1]].name;
    name3 = d.name;
    name4 = racemanlist[player[0]].name;
    d.money += 2000;
  }
  else if (winorlost == 2)	/* 亞軍獎金 5000 */
  {
    name1 = racemanlist[player[2]].name;
    name2 = d.name;
    name3 = racemanlist[player[1]].name;
    name4 = racemanlist[player[0]].name;
    d.money += 5000;
  }
  else				/* 冠軍獎金 10000 */
  {
    name1 = d.name;
    name2 = racemanlist[player[2]].name;
    name3 = racemanlist[player[1]].name;
    name4 = racemanlist[player[0]].name;
    d.money += 10000;
  }

  clear();
  move(6, 13);
  /* \033[1;37m～～～\033[32m本屆 %s 結果揭曉\033[37m～～～\033[m */
  prints("\033[1;37m\xA1\xE3\xA1\xE3\xA1\xE3\033[32m\xA5\xBB\xA9\xA1 %s \xB5\xB2\xAA\x47\xB4\xA6\xBE\xE5\033[37m\xA1\xE3\xA1\xE3\xA1\xE3\033[m", racename[mode - 1]);
  move(8, 15);
  /* \033[1;41m 冠軍 \033[0;1m～\033[1;33m%-10s\033[36m  獎金 %d\033[m */
  prints("\033[1;41m \xAB\x61\xAD\x78 \033[0;1m\xA1\xE3\033[1;33m%-10s\033[36m  \xBC\xFA\xAA\xF7 %d\033[m", name1, 10000);
  move(10, 15);
  /* \033[1;41m 亞軍 \033[0;1m～\033[1;33m%-10s\033[36m  獎金 %d\033[m */
  prints("\033[1;41m \xA8\xC8\xAD\x78 \033[0;1m\xA1\xE3\033[1;33m%-10s\033[36m  \xBC\xFA\xAA\xF7 %d\033[m", name2, 5000);
  move(12, 15);
  /* \033[1;41m 季軍 \033[0;1m～\033[1;33m%-10s\033[36m  獎金 %d\033[m */
  prints("\033[1;41m \xA9\x75\xAD\x78 \033[0;1m\xA1\xE3\033[1;33m%-10s\033[36m  \xBC\xFA\xAA\xF7 %d\033[m", name3, 2000);
  move(14, 15);
  /* \033[1;41m 最後 \033[0;1m～\033[1;33m%-10s\033[36m\033[0m */
  prints("\033[1;41m \xB3\xCC\xAB\xE1 \033[0;1m\xA1\xE3\033[1;33m%-10s\033[36m\033[0m", name4);
  /* 今年的%s結束囉 後年再來吧.. */
  sprintf(buf, "\xA4\xB5\xA6\x7E\xAA\xBA%s\xB5\xB2\xA7\xF4\xC5\x6F \xAB\xE1\xA6\x7E\xA6\x41\xA8\xD3\xA7\x61..", racename[mode - 1]);
  vmsg(buf);
}


/*-------------------------------------------------------*/
/* 收穫季比賽					         */
/*-------------------------------------------------------*/


int			/* !=0:參加的項目 0:不參加 */
pip_race_main()		/* 收穫季 */
{
  int ch;
  int winorlost;		/* 贏了幾個人 >=3:冠軍 2:亞軍 1:季軍 <=0:最後一名 */

  clear();
  move(10, 14);
  /* \033[1;33m叮咚叮咚～ 辛苦的郵差幫我們送信來了喔...\033[m */
  outs("\033[1;33m\xA5\x6D\xA9\x4E\xA5\x6D\xA9\x4E\xA1\xE3 \xA8\xAF\xAD\x57\xAA\xBA\xB6\x6C\xAE\x74\xC0\xB0\xA7\xDA\xAD\xCC\xB0\x65\xAB\x48\xA8\xD3\xA4\x46\xB3\xE1...\033[m");
  /* 嗯  把信打開看看吧... */
  vmsg("\xB6\xE2  \xA7\xE2\xAB\x48\xA5\xB4\xB6\x7D\xAC\xDD\xAC\xDD\xA7\x61...");

  show_resultshow_pic(0);

  move(b_lines - 2, 0);
  /* [A]%s [B]%s [C]%s [D]%s [Q]放棄： */
  prints("[A]%s [B]%s [C]%s [D]%s [Q]\xA9\xF1\xB1\xF3\xA1\x47", racename[0], racename[1], racename[2], racename[3]);
  do
  {
    ch = vkey();
  } while (ch != 'q' && (ch < 'a' || ch > 'd'));

  if (ch == 'q')
  {
    /* 今年不參加啦.....:( */
    vmsg("\xA4\xB5\xA6\x7E\xA4\xA3\xB0\xD1\xA5\x5B\xB0\xD5.....:(");
    d.happy -= rand() % 10 + 10;
    d.satisfy -= rand() % 10 + 10;
    d.relation -= rand() % 10;
    return 0;
  }

  ch -= 'a' - 1;
  show_resultshow_pic(ch);
  /* 今年共有四人參賽～現在比賽開始 */
  vmsg("\xA4\xB5\xA6\x7E\xA6\x40\xA6\xB3\xA5\x7C\xA4\x48\xB0\xD1\xC1\xC9\xA1\xE3\xB2\x7B\xA6\x62\xA4\xF1\xC1\xC9\xB6\x7D\xA9\x6C");

  switch (ch)
  {
  case 1:					/* 武鬥大會 */
    winorlost = pip_race_eventA();
    d.hexp += rand() % 10 + 20 * winorlost;
    d.exp += rand() % 10 + d.level * winorlost;
    break;

  case 2:					/* 藝術大展 */
    winorlost = pip_race_eventB();
    d.art += rand() % 10 + 20 * winorlost;
    d.character += rand() % 10 + 20 * winorlost;
    break;

  case 3:					/* 皇家舞會 */
    winorlost = pip_race_eventC();
    d.art += rand() % 10 + 20 * winorlost;
    d.charm += rand() % 10 + 20 * winorlost;
    break;

  case 4:					/* 烹飪大賽 */
    winorlost = pip_race_eventD();
    d.cook += rand() % 10 + 20 * winorlost;
    d.family += rand() % 10 + 20 * winorlost;
    break;
  }

  pip_race_ending(winorlost, ch);

  /* 如果參加的話，恢復所有屬性 */
  d.tired = 0;
  d.hp = d.maxhp;
  d.happy += rand() % 20;
  d.satisfy += rand() % 20;
  d.relation += rand() % 10;

  return ch;
}
#endif	/* HAVE_GAME */
