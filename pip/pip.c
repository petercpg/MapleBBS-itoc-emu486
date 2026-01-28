/*-------------------------------------------------------*/
/* pip/pip.c            ( NTHU CS MapleBBS Ver 3.10 )    */
/*-------------------------------------------------------*/
/* target : 養小雞遊戲                                   */
/* create :   /  /                                       */
/* update : 01/08/14                                     */
/* author : dsyan.bbs@forever.twbbs.org                  */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_GAME


#define	_PIPMAIN_C_	/* 先 define _PIPMAIN_C_ 再引進 pip.h */

#include "pip.h"


/*-------------------------------------------------------*/
/* 主程式                                                */
/*-------------------------------------------------------*/

#define ALIVE   (9/10)          /* 復活以後的能力是原來的 90% */

static void
pip_live_again()
{
  /* 小雞復活手術中 */
  vs_head("\xA4\x70\xC2\xFB\xB4\x5F\xAC\xA1\xA4\xE2\xB3\x4E\xA4\xA4", str_site);

  /* itoc.010814: 復活以後要讓玩家還玩的下去，所以不做「破壞性」
     的降屬性，像 maxhp..等不變 */

  /* 基本的資料 */
  d.death = 0;
  d.liveagain++;

  /* 狀態的數值 */
  d.relation = 0;	/* 人和寵物的互動惡化 */  
  d.happy = 20;		/* 變得不快樂 */
  d.satisfy = 20;	/* 變得不滿意 */

  /* 身體的參數 */
  d.weight = (rand() % 10) + 55 + (d.bbtime / 180);
  d.tired = 20;
  d.sick = 20;
  d.shit = 20;

  /* 評價減少 */
  d.social = d.social * ALIVE;
  d.family = d.family * ALIVE;
  d.hexp = d.hexp * ALIVE;
  d.mexp = d.mexp * ALIVE;

  /* 歸零 */
  d.hp = d.maxhp;
  d.mp = 0;
  d.vp = 0;
  d.sp = 0;

  /* 小雞又復活了，不要再把牠養死了喔！ */
  vmsg("\xA4\x70\xC2\xFB\xA4\x53\xB4\x5F\xAC\xA1\xA4\x46\xA1\x41\xA4\xA3\xAD\x6E\xA6\x41\xA7\xE2\xA8\x65\xBE\x69\xA6\xBA\xA4\x46\xB3\xE1\xA1\x49");

  pip_write_file();
}


static int			/* 1:申請成功 0:失敗 */
pip_apply()			/* 新小雞申請 */
{
  time_t now;
  struct tm *ptime;
  
  memset(&d, 0, sizeof(d));

  vs_head(BBSNAME PIPNAME, str_site);

  /* 小雞命名 */
  /*    幫小雞取個好聽的名字吧： */
  if (!vget(2, 0, "   \xC0\xB0\xA4\x70\xC2\xFB\xA8\xFA\xAD\xD3\xA6\x6E\xC5\xA5\xAA\xBA\xA6\x57\xA6\x72\xA7\x61\xA1\x47", d.name, IDLEN + 1, DOECHO))
    return 0;

  /* 1:公 2:母 */
  /* 性別：(1) 小公雞♂  (2) 小母雞♀ (1/2)？[1]  */
  d.sex = (ians(4, 3, "\xA9\xCA\xA7\x4F\xA1\x47(1) \xA4\x70\xA4\xBD\xC2\xFB\xA1\xF1  (2) \xA4\x70\xA5\xC0\xC2\xFB\xA1\xF0 (1/2)\xA1\x48[1] ") == '2') ? 2 : 1;

  move(6, 0);
  /* 的遊戲現今分成兩種玩法\n */
  outs("   " BBSNAME PIPNAME "\xAA\xBA\xB9\x43\xC0\xB8\xB2\x7B\xA4\xB5\xA4\xC0\xA6\xA8\xA8\xE2\xBA\xD8\xAA\xB1\xAA\x6B\n");
  /*    選有結局會在小雞 20 歲時結束遊戲，並告知小雞後續的發展\n */
  outs("   \xBF\xEF\xA6\xB3\xB5\xB2\xA7\xBD\xB7\x7C\xA6\x62\xA4\x70\xC2\xFB 20 \xB7\xB3\xAE\xC9\xB5\xB2\xA7\xF4\xB9\x43\xC0\xB8\xA1\x41\xA8\xC3\xA7\x69\xAA\xBE\xA4\x70\xC2\xFB\xAB\xE1\xC4\xF2\xAA\xBA\xB5\x6F\xAE\x69\n");
  /*    選沒有結局則一直養到小雞死亡才結束遊戲.... */
  outs("   \xBF\xEF\xA8\x53\xA6\xB3\xB5\xB2\xA7\xBD\xAB\x68\xA4\x40\xAA\xBD\xBE\x69\xA8\xEC\xA4\x70\xC2\xFB\xA6\xBA\xA4\x60\xA4\x7E\xB5\xB2\xA7\xF4\xB9\x43\xC0\xB8....");

  /* 1:不要且未婚  4:要且未婚 */
  /*    您希望小雞遊戲是否要有 20 歲結局(Y/N)？[Y]  */
  d.wantend = (ians(9, 0, "   \xB1\x7A\xA7\xC6\xB1\xE6\xA4\x70\xC2\xFB\xB9\x43\xC0\xB8\xAC\x4F\xA7\x5F\xAD\x6E\xA6\xB3 20 \xB7\xB3\xB5\xB2\xA7\xBD(Y/N)\xA1\x48[Y] ") == 'n') ? 1 : 4;

  /* 開頭畫面 */
  show_basic_pic(0);
  /* 小雞終於誕生了，請好好愛他.... */
  vmsg("\xA4\x70\xC2\xFB\xB2\xD7\xA9\xF3\xBD\xCF\xA5\xCD\xA4\x46\xA1\x41\xBD\xD0\xA6\x6E\xA6\x6E\xB7\x52\xA5\x4C....");

  /* 開頭設定：沒有設定的欄位都是預設 0 */
  time(&now);
  ptime = localtime(&now);
  sprintf(d.birth, "%02d/%02d/%02d", ptime->tm_year % 100, ptime->tm_mon + 1, ptime->tm_mday);

  /* 基本資料 */
  d.year = ptime->tm_year;
  d.month = ptime->tm_mon + 1;
  d.day = ptime->tm_mday;

  /* 身體參數 */
  d.weight = rand() % 10 + 50;

  /* 評價參數 : 預設 0 */

  /* 戰鬥參數 */
  d.level = 1;
  d.hp = d.maxhp = rand() % 20 + 40;
  d.mp = d.maxmp = rand() % 20 + 40;
  d.vp = d.maxvp = rand() % 20 + 40;
  d.sp = d.maxsp = rand() % 20 + 40;

  /* 技能參數 : 預設 0 */
  /* 武器參數 : 預設 0 */
  /* 能力參數 : 預設 0 */

  /* 狀態數值 */
  d.happy = rand() % 10 + 45;
  d.satisfy = rand() % 10 + 45;

  /* 食物參數 */
  d.food = 20;
  d.cookie = 2;

  /* 物品參數 : 預設 0 */
  d.money = 1000;

  /* 猜拳參數 : 預設 0 */

  /* 參見王臣 */
  d.seeroyalJ = 1;

  /* 接受求婚愛人 : 預設 0 */
  /* 工作指數 : 預設 0 */

  /* 一生只能學得一項特殊技能 */
  pip_learn_skill(0);

  pip_write_file();
  return 1;
}


static int
pip_reborn()			/* 小雞重生 */
{
  vs_head(BBSNAME PIPNAME, str_site);
  move(4, 0);
  /*    歡迎來到\033[1;33m */
  /* 生物科技研究院\033[m\n\n */
  outs("   \xC5\x77\xAA\xEF\xA8\xD3\xA8\xEC\033[1;33m" BBSNAME "\xA5\xCD\xAA\xAB\xAC\xEC\xA7\xDE\xAC\xE3\xA8\x73\xB0\x7C\033[m\n\n");
  /*    經我們調查顯示  先前您有養過小雞喔  可是被您養死了\n\n */
  outs("   \xB8\x67\xA7\xDA\xAD\xCC\xBD\xD5\xAC\x64\xC5\xE3\xA5\xDC  \xA5\xFD\xAB\x65\xB1\x7A\xA6\xB3\xBE\x69\xB9\x4C\xA4\x70\xC2\xFB\xB3\xE1  \xA5\x69\xAC\x4F\xB3\x51\xB1\x7A\xBE\x69\xA6\xBA\xA4\x46\n\n");

  /* 您要我們讓牠重生嗎(Y/N)？[N]  */
  if (ians(7, 3, "\xB1\x7A\xAD\x6E\xA7\xDA\xAD\xCC\xC5\xFD\xA8\x65\xAD\xAB\xA5\xCD\xB6\xDC(Y/N)\xA1\x48[N] ") == 'y')
  {
    pip_live_again();
    return 1;
  }	
  else
  {
    if (pip_apply())
      return 1;
  }

  return 0;
}  


/* 遊戲主程式 */
int
main_pip()
{
  int ch;
  char fpath[64];

  /* more(PIP_PICHOME "pip.welcome", NULL); */
  /* 電子養小雞 */
  vs_head("\xB9\x71\xA4\x6C\xBE\x69\xA4\x70\xC2\xFB", str_site);

  usr_fpath(fpath, cuser.userid, fn_pip);

  if (!dashf(fpath))	/* 之前沒玩過小雞 */
  {
    show_system_pic(11);

    if (vkey() == 'q')
      return 0;

    if (!pip_apply())
      return 0;
  }
  else
  {
    show_system_pic(12);

    if ((ch = vkey()) == 'r')
    {
      if (!pip_read_backup())
        return 0;
    }
    else if (ch == 'q')
    {
      return 0;
    }
    else
    {
      pip_read_file(cuser.userid, &d);

      if (d.death == 1)		/* 玩死了可選擇重生 */
      {
        if (!pip_reborn())
          return 0;
      }
      else if (d.death)		/* 放生或結束的話要重新開始 */
      {
        if (!pip_apply())
          return 0;
      }
    }
  }

  start_time = time(0);
  last_time = start_time;

  /* itoc.010816: 由於第一次進入主選單沒有中間的小雞圖，所以要另外給 */
  clrfromto(5, 18);
  /* \033[34m┌─────────────────────────────────────┐\033[m */
  outs("\033[34m\xA2\x7A\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x7B\033[m");
  show_basic_pic(21);
  move(18, 0);
  /* \033[34m└─────────────────────────────────────┘\033[m */
  outs("\033[34m\xA2\x7C\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x7D\033[m");

  pip_main_menu();

  pip_write_file();		/* 離開遊戲自動存檔 */
  return 0;
}
#endif		/* HAVE_GAME */
