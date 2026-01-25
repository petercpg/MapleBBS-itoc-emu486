/*-------------------------------------------------------*/
/* pip_menu.c         ( NTHU CS MapleBBS Ver 3.10 )      */
/*-------------------------------------------------------*/
/* target : 選單函式                                     */
/* create :   /  /                                       */
/* update : 01/08/15                                     */
/* author : dsyan.bbs@forever.twbbs.org                  */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_GAME

#include "pip.h"


/*-------------------------------------------------------*/
/* 選單						 	 */
/*-------------------------------------------------------*/


/* 主選單 [1]基本 [2]逛街 [3]修行 [4]玩樂 [5]打工 [6]特殊 [7]系統 */

int pip_basic_menu(), pip_store_menu(), pip_practice_menu(), pip_play_menu();
int pip_job_menu(), pip_special_menu(), pip_system_menu();

static struct pipcommands pipmainlist[] = 
{
  pip_basic_menu,	'1', 
  pip_store_menu,	'2', 
  pip_practice_menu,	'3', 
  pip_play_menu,	'4', 
  pip_job_menu,		'5', 
  pip_special_menu,	'6', 
  pip_system_menu,	'7',
  NULL,			'\0'
};


/* 基本選單 [1]餵食 [2]清潔 [3]休息 [4]親親 [5]換錢 */

int pip_basic_feed(), pip_basic_takeshower(), pip_basic_takerest(), pip_basic_kiss(), pip_money();

static struct pipcommands pipbasiclist[] = 
{
  pip_basic_feed,	'1', 
  pip_basic_takeshower,	'2', 
  pip_basic_takerest,	'3', 
  pip_basic_kiss,	'4', 
  pip_money,		'5', 
  NULL,			'\0'
};


/* 逛街 【日常用品】[1]便利商店 [2]百草藥鋪 [3]夜裡書局 */
/* 選單 【武器百貨】[A]頭部裝備 [B]手部裝備 [C]盾牌裝備 [D]身體裝備 [E]腳部裝備 */

int pip_store_food(), pip_store_medicine(), pip_store_other();
int pip_store_weapon_head(), pip_store_weapon_hand(), pip_store_weapon_shield();
int pip_store_weapon_body(), pip_store_weapon_foot();

static struct pipcommands pipstorelist[] = 
{
  pip_store_food,	  '1', 
  pip_store_medicine,	  '2', 
  pip_store_other,	  '3', 
  pip_store_weapon_head,  'a', 
  pip_store_weapon_hand,  'b', 
  pip_store_weapon_shield,'c', 
  pip_store_weapon_body,  'd', 
  pip_store_weapon_foot,  'e', 
  NULL,			  '\0'
};


/* 修行 [A]科學(1) [B]詩詞(1) [C]神學(1) [D]軍學(1) [E]劍術(1) */
/* 選單 [F]格鬥(1) [G]魔法(1) [H]禮儀(1) [I]繪畫(1) [J]舞蹈(1) */

int pip_practice_classA(), pip_practice_classB(), pip_practice_classC(), pip_practice_classD(), pip_practice_classE();
int pip_practice_classF(), pip_practice_classG(), pip_practice_classH(), pip_practice_classI(), pip_practice_classJ();
  
static struct pipcommands pippracticelist[] = 
{
  pip_practice_classA,	'a', 
  pip_practice_classB,	'b', 
  pip_practice_classC,	'c', 
  pip_practice_classD,	'd', 
  pip_practice_classE,	'e', 
  pip_practice_classF,	'f', 
  pip_practice_classG,	'g', 
  pip_practice_classH,	'h', 
  pip_practice_classI,	'i', 
  pip_practice_classJ,	'j', 
  NULL, 		'\0'
};


/* 玩樂選單 [1]散步 [2]運動 [3]約會 [4]猜拳 [5]旅遊 [6]郊外 [7]唱歌 */

int pip_play_stroll(), pip_play_sport(), pip_play_date(), pip_play_guess();
int pip_play_outing(), pip_play_kite(), pip_play_KTV();

static struct pipcommands pipplaylist[] = 
{
  pip_play_stroll,	'1', 
  pip_play_sport,	'2', 
  pip_play_date,	'3', 
  pip_play_guess,	'4', 
  pip_play_outing,	'5', 
  pip_play_kite,	'6', 
  pip_play_KTV,		'7', 
  NULL,			'\0'
};


/* 打工 [A]家事 [B]保姆 [C]旅館 [D]農場 [E]餐廳 [F]教堂 [G]地攤 [H]伐木 */
/* 選單 [I]美髮 [J]獵人 [K]工地 [L]守墓 [M]家教 [N]酒家 [O]酒店 [P]夜總會 */

int pip_job_workA(), pip_job_workB(), pip_job_workC(), pip_job_workD();
int pip_job_workE(), pip_job_workF(), pip_job_workG(), pip_job_workH();
int pip_job_workI(), pip_job_workJ(), pip_job_workK(), pip_job_workL();
int pip_job_workM(), pip_job_workN(), pip_job_workO(), pip_job_workP();

static struct pipcommands pipjoblist[] = 
{
  pip_job_workA,	'a', 
  pip_job_workB,	'b', 
  pip_job_workC,	'c', 
  pip_job_workD,	'd', 
  pip_job_workE,	'e', 
  pip_job_workF,	'f', 
  pip_job_workG,	'g', 
  pip_job_workH,	'h', 
  pip_job_workI,	'i', 
  pip_job_workJ,	'j', 
  pip_job_workK,	'k', 
  pip_job_workL,	'l', 
  pip_job_workM,	'm', 
  pip_job_workN,	'n', 
  pip_job_workO,	'o', 
  pip_job_workP,	'p', 
  NULL,			'\0'
};


/* 特殊選單 [1]醫院 [2]整容 [3]戰鬥 [4]皇宮 [5]任務 [6]對戰 */

int pip_see_doctor(), pip_change_weight(), pip_fight_menu(), pip_go_palace(), pip_quest_menu(), pip_pk_menu();

static struct pipcommands pipspeciallist[] = 
{
  pip_see_doctor,	'1', 
  pip_change_weight,	'2', 
  pip_fight_menu,	'3', 
  pip_go_palace,	'4', 
  pip_quest_menu,	'5', 
  pip_pk_menu,		'6', 
  NULL,			'\0'
};


/* 系統選單  [1]詳細資料 [2]拜訪他人 [3]小雞放生 [4]特別服務 [S]儲存進度 [L]讀取進度 */

int pip_query_self(), pip_query(), pip_system_freepip(), pip_system_service();
int pip_write_backup(), pip_read_backup();

static struct pipcommands pipsystemlist[] = 
{
  pip_query_self,	'1', 
  pip_query,		'2', 
  pip_system_freepip,	'3',
  pip_system_service,	'4',
  pip_write_backup,	's', 
  pip_read_backup,	'l', 
  NULL,			'\0'
};


/*-------------------------------------------------------*/
/* 指令選單						 */
/*-------------------------------------------------------*/


/* 在 b_lines 和 b_lines - 1  這二行都是指令選單 */
/* 若指令第二行，則第一欄自白 */
/* 參考 global.h FEETER */

/* 僅供 pip_do_menu() 使用 */

static char *menuname[8][2] = 
{
  {"",
   /*  選單  */
   /*  [1]基本 [2]逛街 [3]修行 [4]玩樂 [5]打工 [6]特殊 [7]系統 [Q]離開        \033[m */
   COLOR1 " \xBF\xEF\xB3\xE6 " COLOR2 " [1]\xB0\xF2\xA5\xBB [2]\xB3\x7D\xB5\xF3 [3]\xAD\xD7\xA6\xE6 [4]\xAA\xB1\xBC\xD6 [5]\xA5\xB4\xA4\x75 [6]\xAF\x53\xAE\xED [7]\xA8\x74\xB2\xCE [Q]\xC2\xF7\xB6\x7D        \033[m"},
   
  {"",
   /*  基本  */
   /*  [1]餵食 [2]清潔 [3]休息 [4]親親 [5]換錢 [Q]跳出                        \033[m */
   COLOR1 " \xB0\xF2\xA5\xBB " COLOR2 " [1]\xC1\xFD\xAD\xB9 [2]\xB2\x4D\xBC\xE4 [3]\xA5\xF0\xAE\xA7 [4]\xBF\xCB\xBF\xCB [5]\xB4\xAB\xBF\xFA [Q]\xB8\xF5\xA5\x58                        \033[m"},

  /*  逛街  */
  /*  日常用品 [1]便利商店 [2]百草藥鋪 [3]夜裡書局 [Q]跳出                   \033[m */
  {COLOR1 " \xB3\x7D\xB5\xF3 " COLOR2 " \xA4\xE9\xB1\x60\xA5\xCE\xAB\x7E [1]\xAB\x4B\xA7\x51\xB0\xD3\xA9\xB1 [2]\xA6\xCA\xAF\xF3\xC3\xC4\xBE\x51 [3]\xA9\x5D\xB8\xCC\xAE\xD1\xA7\xBD [Q]\xB8\xF5\xA5\x58                   \033[m",
   /*  採購  */
   /*  武器百貨 [A]頭部裝備 [B]手部裝備 [C]盾牌裝備 [D]身體裝備 [E]腳部裝備   \033[m */
   COLOR1 " \xB1\xC4\xC1\xCA " COLOR2 " \xAA\x5A\xBE\xB9\xA6\xCA\xB3\x66 [A]\xC0\x59\xB3\xA1\xB8\xCB\xB3\xC6 [B]\xA4\xE2\xB3\xA1\xB8\xCB\xB3\xC6 [C]\xAC\xDE\xB5\x50\xB8\xCB\xB3\xC6 [D]\xA8\xAD\xC5\xE9\xB8\xCB\xB3\xC6 [E]\xB8\x7D\xB3\xA1\xB8\xCB\xB3\xC6   \033[m"},

  /*  修行  */
  /*  [A]科學 [B]詩詞 [C]神學 [D]軍學 [E]劍術                                \033[m */
  {COLOR1 " \xAD\xD7\xA6\xE6 " COLOR2 " [A]\xAC\xEC\xBE\xC7 [B]\xB8\xD6\xB5\xFC [C]\xAF\xAB\xBE\xC7 [D]\xAD\x78\xBE\xC7 [E]\xBC\x43\xB3\x4E                                \033[m",
   /*  苦練  */
   /*  [F]格鬥 [G]魔法 [H]禮儀 [I]繪畫 [J]舞蹈 [Q]跳出                        \033[m */
   COLOR1 " \xAD\x57\xBD\x6D " COLOR2 " [F]\xAE\xE6\xB0\xAB [G]\xC5\x5D\xAA\x6B [H]\xC2\xA7\xBB\xF6 [I]\xC3\xB8\xB5\x65 [J]\xBB\x52\xC1\xD0 [Q]\xB8\xF5\xA5\x58                        \033[m"},

  {"",
   /*  玩樂  */
   /*  [1]散步 [2]運動 [3]約會 [4]猜拳 [5]旅遊 [6]郊外 [7]唱歌 [Q]跳出        \033[m */
   COLOR1 " \xAA\xB1\xBC\xD6 " COLOR2 " [1]\xB4\xB2\xA8\x42 [2]\xB9\x42\xB0\xCA [3]\xAC\xF9\xB7\x7C [4]\xB2\x71\xAE\xB1 [5]\xAE\xC8\xB9\x43 [6]\xAD\xA5\xA5\x7E [7]\xB0\xDB\xBA\x71 [Q]\xB8\xF5\xA5\x58        \033[m"},

  /*  打工  */
  /*  [A]家事 [B]保姆 [C]旅館 [D]農場 [E]餐廳 [F]教堂 [G]地攤 [H]伐木 [Q]跳出\033[m */
  {COLOR1 " \xA5\xB4\xA4\x75 " COLOR2 " [A]\xAE\x61\xA8\xC6 [B]\xAB\x4F\xA9\x69 [C]\xAE\xC8\xC0\x5D [D]\xB9\x41\xB3\xF5 [E]\xC0\x5C\xC6\x55 [F]\xB1\xD0\xB0\xF3 [G]\xA6\x61\xC5\x75 [H]\xA5\xEF\xA4\xEC [Q]\xB8\xF5\xA5\x58\033[m",
   /*  賺錢  */
   /*  [I]美髮 [J]獵人 [K]工地 [L]守墓 [M]家教 [N]酒家 [O]酒店 [P]夜總會      \033[m */
   COLOR1 " \xC1\xC8\xBF\xFA " COLOR2 " [I]\xAC\xFC\xBE\x76 [J]\xC2\x79\xA4\x48 [K]\xA4\x75\xA6\x61 [L]\xA6\x75\xB9\xD3 [M]\xAE\x61\xB1\xD0 [N]\xB0\x73\xAE\x61 [O]\xB0\x73\xA9\xB1 [P]\xA9\x5D\xC1\x60\xB7\x7C      \033[m"},

  {"",
   /*  特殊  */
   /*  [1]醫院 [2]整容 [3]戰鬥 [4]皇宮 [5]任務 [6]對戰 [Q]跳出                \033[m */
   COLOR1 " \xAF\x53\xAE\xED " COLOR2 " [1]\xC2\xE5\xB0\x7C [2]\xBE\xE3\xAE\x65 [3]\xBE\xD4\xB0\xAB [4]\xAC\xD3\xAE\x63 [5]\xA5\xF4\xB0\xC8 [6]\xB9\xEF\xBE\xD4 [Q]\xB8\xF5\xA5\x58                \033[m"},

  /*  服務  */
  /*  [1]詳細資料 [2]拜訪他人 [3]小雞放生 [4]特別服務                        \033[m */
  {COLOR1 " \xAA\x41\xB0\xC8 " COLOR2 " [1]\xB8\xD4\xB2\xD3\xB8\xEA\xAE\xC6 [2]\xAB\xF4\xB3\x58\xA5\x4C\xA4\x48 [3]\xA4\x70\xC2\xFB\xA9\xF1\xA5\xCD [4]\xAF\x53\xA7\x4F\xAA\x41\xB0\xC8                        \033[m",
   /*  系統  */
   /*  [S]儲存進度 [L]讀取進度 [Q]跳出                                        \033[m */
   COLOR1 " \xA8\x74\xB2\xCE " COLOR2 " [S]\xC0\x78\xA6\x73\xB6\x69\xAB\xD7 [L]\xC5\xAA\xA8\xFA\xB6\x69\xAB\xD7 [Q]\xB8\xF5\xA5\x58                                        \033[m"}
};


/*-------------------------------------------------------*/
/* 選單函式						 */
/*-------------------------------------------------------*/


  /*-----------------------------------------------------*/
  /* 常駐函式						 */
  /*-----------------------------------------------------*/


#define PIP_CHECK_PERIOD	60		/* 每 60 秒檢查一次 */

static int			/* 回傳 歲數 */
pip_time_update()
{
  int oldtm, tm;

  /* 固定時間做的事 */

  if ((time(0) - last_time) >= PIP_CHECK_PERIOD)
  {
    do
    {
      d.shit += rand() % 3 + 3;		/* 不做事，還是會變髒的 */
      d.tired -= 2;			/* 不做事，疲勞當然減低啦 */
      d.sick += rand() % 4 - 2;		/* 不做事，病氣會隨機率增加減少或增加少許 */
      d.happy += rand() % 4 - 2;	/* 不做事，快樂會隨機率增加減少或增加少許 */
      d.satisfy += rand() % 4 - 2;	/* 不做事，滿足會隨機率增加減少或增加少許 */
      d.hp -= rand() % 3 + d.sick / 10;	/* 不做事，肚子也會餓咩，也會因生病降低一點 */

      last_time += PIP_CHECK_PERIOD;	/* 下次更新時間 */
    } while ((time(0) - last_time) >= PIP_CHECK_PERIOD);

    /* 檢查年齡 */

    oldtm = d.bbtime / 60 / 30;			/* 更新前幾歲了 */
    d.bbtime += time(0) - start_time;		/* 更新小雞的時間(年齡) */
    start_time = time(0);
    tm = d.bbtime / 60 / 30;			/* 更新後幾歲了 */

    /* itoc.010815.註解: 如果小雞一直在次選單中(例如戰鬥修行)，
       那麼會因為很久(超過30分鐘即一歲)沒有執行 pip_time_update() 而一次加好多歲 */

    /* itoc.010815: 一次過好多歲會少加好多次過生日的好處，不予修正，作為虐待小雞的處罰 :p */

    if (tm != oldtm)		/* 歲數更新前後如果不同，表示長大了 */
    {
      /* 長大時的增加改變值 */
      count_tired(1, 7, 0, 100, 0);	/* 恢復疲勞 */
      d.happy += rand() % 5 + 5;
      d.satisfy += rand() % 5;
      d.wisdom += 10;
      d.character += rand() % 5;
      d.money += 500;
      d.seeroyalJ = 1;			/* 一年可以見王子一次 */
      pip_write_file();			/* 自動儲存 */

      /* 電子養小雞 */
      vs_head("\xB9\x71\xA4\x6C\xBE\x69\xA4\x70\xC2\xFB", str_site);
      show_basic_pic(20);		/* 生日快樂 */
      /* 小雞過生日了 */
      vmsg("\xA4\x70\xC2\xFB\xB9\x4C\xA5\xCD\xA4\xE9\xA4\x46");

      /* 收穫季 */
      if (tm % 2 == 0)		/* 二年一次收穫季 */
        pip_race_main();

      /* 結局 */
      if (tm >= 21 && (d.wantend == 4 || d.wantend == 5 || d.wantend == 6))	/* 玩到 20 歲 */
        pip_ending_screen();
    }
  }
  else
  {
    tm = d.bbtime / 60 / 30;	/* 如果沒有 update，也要回傳 tm(歲數) */
  }

  /* 偶發事件 */

  oldtm = rand() % 2000;	/* 借用 oldtm 做亂數 */
  if (oldtm == 0 && tm >= 15 && d.charm >= 300 && d.character >= 300)
    pip_marriage_offer();	/* 商人來求婚 */
  else if (oldtm > 1998)
    pip_meet_divine();		/* 隨機遇到占卜師 */
  else if (oldtm > 1996)
    pip_meet_sysop();		/* 隨機遇到 SYSOP */
  else if (oldtm > 1994)
    pip_meet_smith();		/* 隨機遇到鐵匠 */

  /* 檢查一些常變動的值是否爆掉 */
  /* itoc.010815: 在 pip_time_update() 中不必檢查 shit/tired/sick >100 或 happy/satisfy/hp < 0
     而在 pip_refresh_screen() 檢查並順道宣告死亡 */

  if (d.shit < 0)
    d.shit = 0;
  if (d.tired < 0)
    d.tired = 0;
  if (d.sick < 0)
    d.sick = 0;

  if (d.happy > 100)
    d.happy = 100;
  if (d.satisfy > 100)
    d.satisfy = 100;
  if (d.hp > d.maxhp)
    d.hp = d.maxhp;

  return tm;
}


static int				/* -1:死亡  0:沒事 */
pip_refresh_screen(menunum, mode)	/* 重繪整個畫面 */
  int menunum;		/* 選單編號 */
  int mode;		/* 畫面種類 0:一般 1:餵食 2:打工 3:修行 */
{
  int tm, age, pic;
  char inbuf2[20], inbuf3[20], inbuf4[20], inbuf5[20], inbuf6[20], inbuf7[20];

  char yo[12][5] = 
  {
    /* 誕生 */
    /* 嬰兒 */
    /* 幼兒 */
    /* 兒童 */
    /* 少年 */
    /* 青年 */
    "\xBD\xCF\xA5\xCD", "\xC0\xA6\xA8\xE0", "\xA5\xAE\xA8\xE0", "\xA8\xE0\xB5\xA3", "\xA4\xD6\xA6\x7E", "\xAB\x43\xA6\x7E",
    /* 成年 */
    /* 壯年 */
    /* 更年 */
    /* 老年 */
    /* 古稀 */
    /* 神仙 */
    "\xA6\xA8\xA6\x7E", "\xA7\xA7\xA6\x7E", "\xA7\xF3\xA6\x7E", "\xA6\xD1\xA6\x7E", "\xA5\x6A\xB5\x7D", "\xAF\xAB\xA5\x50"
  };

  tm = pip_time_update();	/* 固定時間要做的事 */

  if (tm == 0)		/* 誕生 */
    age = 0;
  else if (tm == 1)	/* 嬰兒 */
    age = 1;
  else if (tm <= 5)	/* 幼兒 */
    age = 2;
  else if (tm <= 12)	/* 兒童 */
    age = 3;
  else if (tm <= 15)	/* 少年 */
    age = 4;
  else if (tm <= 18)	/* 青年 */
    age = 5;
  else if (tm <= 35)	/* 成年 */
    age = 6;
  else if (tm <= 45)	/* 壯年 */
    age = 7;
  else if (tm <= 60)	/* 更年 */
    age = 8;
  else if (tm <= 70)	/* 老年 */
    age = 9;
  else if (tm <= 100)	/* 古稀 */
    age = 10;
  else 			/* 神仙 */
    age = 11;

  sprintf(inbuf2, "%-4d/%4d", d.hp, d.maxhp);
  sprintf(inbuf3, "%-4d/%4d", d.mp, d.maxmp);
  sprintf(inbuf4, "%-4d/%4d", d.vp, d.maxvp);
  sprintf(inbuf5, "%-4d/%4d", d.sp, d.maxsp);
  sprintf(inbuf6, "%-4d/%4d", d.shit, d.sick);
  sprintf(inbuf7, "%-4d/%4d", d.happy, d.satisfy);

  if (menunum)
  {
    clear();
  }
  else	/* itoc.010816: 如果是進入主選單，不要清中間 5~18 列 */
  {
    clrfromto(0, 4);
    clrfromto(19, b_lines);
  }

  /* 螢幕上面 (0~4列) 顯示點數 */

  move(0, 0);
  /*  資料  */
  /* ♂ */
  /* ♀ */
  prints(COLOR1 " \xB8\xEA\xAE\xC6 " COLOR2 " %s            %-15s                                          \033[m\n", d.sex == 1 ? "\xA1\xF1" : "\xA1\xF0", d.name);

  /* itoc,010802: 為了看清楚一點，所以 prints() 裡面的引數就不斷行寫在該列最後 */
  /* \033[1;32m[狀  態] \033[37m%-9s\033[32m [生  日] \033[37m%-9s\033[32m [年  齡] \033[37m%-9d\033[32m [金  錢] \033[37m%-9d\033[m\n */
  prints("\033[1;32m[\xAA\xAC  \xBA\x41] \033[37m%-9s\033[32m [\xA5\xCD  \xA4\xE9] \033[37m%-9s\033[32m [\xA6\x7E  \xC4\xD6] \033[37m%-9d\033[32m [\xAA\xF7  \xBF\xFA] \033[37m%-9d\033[m\n", yo[age], d.birth, tm, d.money);
  /* \033[1;32m[生  命] \033[35m%-9s\033[32m [法  力] \033[34m%-9s\033[32m [移動力] \033[36m%-9s\033[32m [內  力] \033[31m%-9s\033[m\n */
  prints("\033[1;32m[\xA5\xCD  \xA9\x52] \033[35m%-9s\033[32m [\xAA\x6B  \xA4\x4F] \033[34m%-9s\033[32m [\xB2\xBE\xB0\xCA\xA4\x4F] \033[36m%-9s\033[32m [\xA4\xBA  \xA4\x4F] \033[31m%-9s\033[m\n", inbuf2, inbuf3, inbuf4, inbuf5);
  /* \033[1;32m[體  重] \033[37m%-9d\033[32m [疲  勞] \033[37m%-9d\033[32m [髒／病] \033[37m%-9s\033[32m [快／滿] \033[37m%-9s\033[m\n */
  prints("\033[1;32m[\xC5\xE9  \xAD\xAB] \033[37m%-9d\033[32m [\xAF\x68  \xB3\xD2] \033[37m%-9d\033[32m [\xC5\xBC\xA1\xFE\xAF\x66] \033[37m%-9s\033[32m [\xA7\xD6\xA1\xFE\xBA\xA1] \033[37m%-9s\033[m\n", d.weight, d.tired, inbuf6, inbuf7);

  if (mode == 0)		/* 一般畫面 */
  {
    char *hint[3] = 
    {
      /* \033[1;35m[站長曰]:\033[37m要多多注意小雞的疲勞度和病氣，以免累死病死\033[m\n */
      "\033[1;35m[\xAF\xB8\xAA\xF8\xA4\xEA]:\033[37m\xAD\x6E\xA6\x68\xA6\x68\xAA\x60\xB7\x4E\xA4\x70\xC2\xFB\xAA\xBA\xAF\x68\xB3\xD2\xAB\xD7\xA9\x4D\xAF\x66\xAE\xF0\xA1\x41\xA5\x48\xA7\x4B\xB2\xD6\xA6\xBA\xAF\x66\xA6\xBA\033[m\n", 
      /* \033[1;35m[站長曰]:\033[37m隨時注意小雞的生命數值唷！\033[m\n */
      "\033[1;35m[\xAF\xB8\xAA\xF8\xA4\xEA]:\033[37m\xC0\x48\xAE\xC9\xAA\x60\xB7\x4E\xA4\x70\xC2\xFB\xAA\xBA\xA5\xCD\xA9\x52\xBC\xC6\xAD\xC8\xAD\xF2\xA1\x49\033[m\n", 
      /* \033[1;35m[站長曰]:\033[37m快快樂樂的小雞才是幸福的小雞.....\033[m\n */
      "\033[1;35m[\xAF\xB8\xAA\xF8\xA4\xEA]:\033[37m\xA7\xD6\xA7\xD6\xBC\xD6\xBC\xD6\xAA\xBA\xA4\x70\xC2\xFB\xA4\x7E\xAC\x4F\xA9\xAF\xBA\xD6\xAA\xBA\xA4\x70\xC2\xFB.....\033[m\n"
    };
    outs(hint[rand() % 3]);
  }
  else if (mode == 1)		/* 餵食 */
  {
    sprintf(inbuf2, "%-4d/%4d", d.food, d.cookie);
    sprintf(inbuf3, "%-4d/%4d", d.pill, d.medicine);
    sprintf(inbuf4, "%-4d/%4d", d.burger, d.ginseng);
    sprintf(inbuf5, "%-4d/%4d", d.paste, d.snowgrass);
    /* \033[1;32m[食／零] \033[37m%-9s\033[32m [還／靈] \033[37m%-9s\033[32m [補／蔘] \033[37m%-9s\033[32m [膏／蓮] \033[37m%-9s\033[m\n */
    prints("\033[1;32m[\xAD\xB9\xA1\xFE\xB9\x73] \033[37m%-9s\033[32m [\xC1\xD9\xA1\xFE\xC6\x46] \033[37m%-9s\033[32m [\xB8\xC9\xA1\xFE\xE7\x78] \033[37m%-9s\033[32m [\xBB\x49\xA1\xFE\xBD\xAC] \033[37m%-9s\033[m\n", inbuf2, inbuf3, inbuf4, inbuf5);
  }
  else if (mode == 2)		/* 打工 */
  {
    /* \033[1;36m[愛心]\033[37m%-5d\033[36m[智慧]\033[37m%-5d\033[36m[氣質]\033[37m%-5d\033[36m[藝術]\033[37m%-5d\033[36m[道德]\033[37m%-5d\033[36m[勇敢]\033[37m%-5d\033[36m[家事]\033[37m%-5d\n\033[m */
    prints("\033[1;36m[\xB7\x52\xA4\xDF]\033[37m%-5d\033[36m[\xB4\xBC\xBC\x7A]\033[37m%-5d\033[36m[\xAE\xF0\xBD\xE8]\033[37m%-5d\033[36m[\xC3\xC0\xB3\x4E]\033[37m%-5d\033[36m[\xB9\x44\xBC\x77]\033[37m%-5d\033[36m[\xAB\x69\xB4\xB1]\033[37m%-5d\033[36m[\xAE\x61\xA8\xC6]\033[37m%-5d\n\033[m",
      d.love, d.wisdom, d.character, d.art, d.etchics, d.brave, d.homework);
  }
  else if (mode == 3)		/* 修行 */
  {
    /* \033[1;36m[智慧]\033[37m%-5d\033[36m[氣質]\033[37m%-5d\033[36m[藝術]\033[37m%-5d\033[36m[勇敢]\033[37m%-5d\033[36m[攻擊]\033[37m%-5d\033[36m[防禦]\033[37m%-5d\033[36m[速度]\033[37m%-5d\n\033[m */
    prints("\033[1;36m[\xB4\xBC\xBC\x7A]\033[37m%-5d\033[36m[\xAE\xF0\xBD\xE8]\033[37m%-5d\033[36m[\xC3\xC0\xB3\x4E]\033[37m%-5d\033[36m[\xAB\x69\xB4\xB1]\033[37m%-5d\033[36m[\xA7\xF0\xC0\xBB]\033[37m%-5d\033[36m[\xA8\xBE\xBF\x6D]\033[37m%-5d\033[36m[\xB3\x74\xAB\xD7]\033[37m%-5d\n\033[m",
      d.wisdom, d.character, d.art, d.brave, d.attack, d.resist, d.speed);
  }

  /* 螢幕中間 (5~18列) 顯示圖 */

  tm *= 10;

  if (menunum)		/* itoc.010816: 如果是進出主選單，不需要重繪中間的圖 */
  {
    /* 進出主選單不重繪中間的圖會有一個小 bug，就是第一次進來主選單沒有圖，不過可以省下大量重繪 */

    move(5, 0);
    /* \033[34m┌─────────────────────────────────────┐\033[m */
    outs("\033[34m\xA2\x7A\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x7B\033[m");

    /* 由年齡及體重來決定小雞日常生活起居的圖 */

    /* 體重的決定可以參考下面一點的程式說明 */
    if (d.weight < tm + 30)
      pic = 1;		/* 瘦 */
    else if (d.weight < tm + 90)
      pic = 2;		/* 中等 */
    else
      pic = 3;		/* 胖 */

    switch (age)
    {
    case 0:
    case 1:
    case 2:
      show_basic_pic(pic);	/* pic1~3 */
      break;

    case 3:
    case 4:
      show_basic_pic(pic + 3);	/* pic4~6 */
      break;

    case 5:
    case 6:
      show_basic_pic(pic + 6);	/* pic7~9 */
      break;

    case 7:
    case 8:
      show_basic_pic(pic + 9);	/* pic10~12 */
      break;

    case 9:
    case 10:
      show_basic_pic(pic + 12);	/* pic13~15 */
      break;

    case 11:
      show_basic_pic(pic + 15);	/* pic16~18 */      
      break;
    }

    move(18, 0);
    /* \033[34m└─────────────────────────────────────┘\033[m */
    outs("\033[34m\xA2\x7C\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x7D\033[m");
  }

  /* 螢幕下方 (19~b_lines列) 顯示目前狀態，並順便檢查是否死亡 */

  move(19, 0);
  /* \033[1;34m─\033[37;44m  狀 態 \033[0;1;34m─\033[m\n */
  outs("\033[1;34m\xA2\x77\033[37;44m  \xAA\xAC \xBA\x41 \033[0;1;34m\xA2\x77\033[m\n");

  /* itoc.010801: 借用 age 檢查狀態，由高一路檢查到中、由低一路檢查到中 */

  age = d.shit;			/* 糞便越少越好，理想 age = 0 */
  if (age >= 100)
  {
    /* \033[1;31m哇～臭死了\033[m   */
    pipdie("\033[1;31m\xAB\x7A\xA1\xE3\xAF\xE4\xA6\xBA\xA4\x46\033[m  ", 1);
    return -1;
  }
  else if (age >= 80)
  {
    /* \033[1;35m快臭死了\033[m   */
    outs("\033[1;35m\xA7\xD6\xAF\xE4\xA6\xBA\xA4\x46\033[m  ");
    d.sick += 4;
  }
  else if (age >= 60)
  {
    /* \033[1;33m很臭了說\033[m   */
    outs("\033[1;33m\xAB\xDC\xAF\xE4\xA4\x46\xBB\xA1\033[m  ");
  }
  else if (age >= 40)
  {
    /* 有點臭臭   */
    outs("\xA6\xB3\xC2\x49\xAF\xE4\xAF\xE4  ");
  }
  else if (age == 0)
  {
    /* 乾淨小雞   */
    outs("\xB0\xAE\xB2\x62\xA4\x70\xC2\xFB  ");
  }

  age = d.hp * 100 / d.maxhp;		/* age = 血滿的比例 % */
  if (age >= 90)
  {
    /* \033[1;33m撐撐的說\033[m   */
    outs("\033[1;33m\xBC\xB5\xBC\xB5\xAA\xBA\xBB\xA1\033[m  ");
  }
  else if (age >= 80)
  {
    /* 肚子飽飽   */
    outs("\xA8\x7B\xA4\x6C\xB9\xA1\xB9\xA1  ");
  }
  else if (age <= 40)
  {
    /* \033[1;33m想吃東西\033[m   */
    outs("\033[1;33m\xB7\x51\xA6\x59\xAA\x46\xA6\xE8\033[m  ");
  }
  else if (age <= 0)
  {
    /* \033[1;31m嗚～餓死了\033[m   */
    pipdie("\033[1;31m\xB6\xE3\xA1\xE3\xBE\x6A\xA6\xBA\xA4\x46\033[m  ", 1);
    return -1;
  }
  else if (age <= 20)
  {
    /* \033[1;35m快餓昏了\033[m   */
    outs("\033[1;35m\xA7\xD6\xBE\x6A\xA9\xFC\xA4\x46\033[m  ");
    d.sick += 3;
    d.happy -= 5;
    d.satisfy -= 3;
  }  

  age = d.tired;			/* 疲勞越低越好，理想 age = 0 */
  if (age >= 100)
  {
    /* \033[1;31mㄚ～累死了\033[m   */
    pipdie("\033[1;31m\xA3\xAB\xA1\xE3\xB2\xD6\xA6\xBA\xA4\x46\033[m  ", 1);
    return -1;
  }
  else if (age >= 80)
  {
    /* \033[1;35m真的很累\033[m   */
    outs("\033[1;35m\xAF\x75\xAA\xBA\xAB\xDC\xB2\xD6\033[m  ");
    d.sick += 5;
  }
  else if (age >= 60)
  {
    /* \033[1;33m有點小累\033[m   */
    outs("\033[1;33m\xA6\xB3\xC2\x49\xA4\x70\xB2\xD6\033[m  ");
  }
  else if (age <= 10)
  {
    /* 精神相當棒   */
    outs("\xBA\xEB\xAF\xAB\xAC\xDB\xB7\xED\xB4\xCE  ");
  }
  else if (age <= 20)
  {
    /* 精神很好   */
    outs("\xBA\xEB\xAF\xAB\xAB\xDC\xA6\x6E  ");
  }

  age = d.weight - tm;			/* 理想體重 age = 60 即 d.wight = 60 + 10 * tm */
					/* 注意此時 tm 已經 * 10 過了 */
  if (age >= 130)
  {
    /* \033[1;31m嗚～肥死了\033[m   */
    pipdie("\033[1;31m\xB6\xE3\xA1\xE3\xAA\xCE\xA6\xBA\xA4\x46\033[m  ", 1);
    return -1;
  }
  else if (age >= 110)
  {
    /* \033[1;35m太胖了啦\033[m   */
    outs("\033[1;35m\xA4\xD3\xAD\x44\xA4\x46\xB0\xD5\033[m  ");
    d.sick += 3;
    if (d.speed > 2)
      d.speed -= 2;
    else
      d.speed = 0;
  }
  else if (age >= 90)
  {
    /* \033[1;33m有點小胖\033[m   */
    outs("\033[1;33m\xA6\xB3\xC2\x49\xA4\x70\xAD\x44\033[m  ");    
  }
  else if (age <= -10)
  {
    /* \033[1;31m:~~ 瘦死了\033[m   */
    pipdie("\033[1;31m:~~ \xBD\x47\xA6\xBA\xA4\x46\033[m  ", 1);
    return -1;
  }
  else if (age <= 10)
  {
    /* \033[1;33m有點小瘦\033[m   */
    outs("\033[1;33m\xA6\xB3\xC2\x49\xA4\x70\xBD\x47\033[m  ");
  }
  else if (age <= 30)
  {
    /* \033[1;35m太瘦了喔\033[m  */
    outs("\033[1;35m\xA4\xD3\xBD\x47\xA4\x46\xB3\xE1\033[m ");
  }

  age = d.sick;				/* 疾病越低越好，理想 age = 0 */
  if (age >= 100)
  {
    /* \033[1;31m病死了啦 :~~\033[m   */
    pipdie("\033[1;31m\xAF\x66\xA6\xBA\xA4\x46\xB0\xD5 :~~\033[m  ", 1);
    return -1;
  }
  else if (age >= 75)
  {
    /* \033[1;35m正病重中\033[m   */
    outs("\033[1;35m\xA5\xBF\xAF\x66\xAD\xAB\xA4\xA4\033[m  ");
    d.sick += 5;
    count_tired(1, 15, 1, 100, 1);
  }
  else if (age >= 50)
  {
    /* \033[1;33m生病了啦\033[m   */
    outs("\033[1;33m\xA5\xCD\xAF\x66\xA4\x46\xB0\xD5\033[m  ");
    count_tired(1, 8, 1, 100, 1);
  }

  age = d.happy;			/* 快樂越高越好，理想 age = 100 */
  if (age >= 90)
  {
    /* 快樂啦..   */
    outs("\xA7\xD6\xBC\xD6\xB0\xD5..  ");
  }
  else if (age >= 80)
  {
    /* 快樂啦..   */
    outs("\xA7\xD6\xBC\xD6\xB0\xD5..  ");
  }
  else if (age <= 10)
  {
    /* \033[1;35m很不快樂\033[m   */
    outs("\033[1;35m\xAB\xDC\xA4\xA3\xA7\xD6\xBC\xD6\033[m  ");
  }
  else if (age <= 20)
  {
    /* \033[1;33m不太快樂\033[m   */
    outs("\033[1;33m\xA4\xA3\xA4\xD3\xA7\xD6\xBC\xD6\033[m  ");
  }

  age = d.satisfy;			/* 滿足越高越好，理想 age = 100 */
  if (age >= 90)
  {
    /* 滿足啦..   */
    outs("\xBA\xA1\xA8\xAC\xB0\xD5..  ");
  }
  else if (age >= 80)
  {
    /* 滿足啦..   */
    outs("\xBA\xA1\xA8\xAC\xB0\xD5..  ");
  }
  else if (age <= 10)
  {
    /* \033[1;35m很不滿足\033[m   */
    outs("\033[1;35m\xAB\xDC\xA4\xA3\xBA\xA1\xA8\xAC\033[m  ");
  }
  else if (age <= 20)
  {
    /* \033[1;33m不太滿足\033[m   */
    outs("\033[1;33m\xA4\xA3\xA4\xD3\xBA\xA1\xA8\xAC\033[m  ");
  }

  return 0;
}


  /*-----------------------------------------------------*/
  /* 選單主函式					 	 */
  /*-----------------------------------------------------*/

/* 類似 menu.c 的功能 */

static int
pip_do_menu(menunum, menumode, cmdtable)
  int menunum;		/* 哪一頁選單 */
  int menumode;		/* 哪一類畫面 */
  struct pipcommands cmdtable[];	/* 指令集 */
{
  int ch, key;
  struct pipcommands *cmd;

  while (1)
  {
    /* 判斷是否死亡，死掉即跳回上一層 */
    if (d.death)
      return 0;

    /* 畫面重繪，並判定後是否死亡，死掉即跳回上一層 */
    if (pip_refresh_screen(menunum, menumode))
      return 0;

    /* 印出最後二列指令列 */
    out_cmd(menuname[menunum][0], menuname[menunum][1]);

    switch (ch = vkey())
    {
    case KEY_LEFT:
    case 'q':
      return 0;

    default:
#if 0
      /* itoc.010815: 換小寫 */
      if (ch >= 'A' && ch <= 'Z')
	ch |= 0x20;
#endif

      cmd = cmdtable;
      for (; key = cmd->key; cmd++)
      {
	if (ch == key)
	{
	  cmd->fptr();
	  break;	/* itoc.010815: 執行完程式要重新按鍵 */
	}
      }
      break;
    }
  }

  return 0;
}


  /*-----------------------------------------------------*/
  /* 主選單: 基本 逛街 修行 玩樂 打工 特殊		 */
  /*-----------------------------------------------------*/

int
pip_main_menu()
{
  pip_do_menu(0, 0, pipmainlist);
  return 0;
}


  /*-----------------------------------------------------*/
  /* 基本選單: 餵食 清潔 親親 休息			 */
  /*-----------------------------------------------------*/

int
pip_basic_menu()
{
  pip_do_menu(1, 0, pipbasiclist);
  return 0;
}


  /*-----------------------------------------------------*/
  /* 商店選單: 食物 零食 大補丸 玩具 書本		 */
  /*-----------------------------------------------------*/

int
pip_store_menu()
{
  pip_do_menu(2, 1, pipstorelist);
  return 0;
}


  /*-----------------------------------------------------*/
  /* 修行選單: 念書 練武 修行     			 */
  /*-----------------------------------------------------*/

int
pip_practice_menu()
{
  pip_do_menu(3, 3, pippracticelist);
  return 0;
}


  /*-----------------------------------------------------*/
  /* 玩樂選單: 散步 旅遊 運動 約會 猜拳			 */
  /*-----------------------------------------------------*/

int
pip_play_menu()
{
  pip_do_menu(4, 0, pipplaylist);
  return 0;
}


  /*-----------------------------------------------------*/
  /* 打工選單: 家事 苦工 家教 地攤			 */
  /*-----------------------------------------------------*/

int
pip_job_menu()
{
  pip_do_menu(5, 2, pipjoblist);
  return 0;
}


  /*-----------------------------------------------------*/
  /* 特殊選單: 看病 減肥 戰鬥 拜訪 朝見			 */
  /*-----------------------------------------------------*/

int
pip_special_menu()
{
  pip_do_menu(6, 0, pipspeciallist);
  return 0;
}


  /*-----------------------------------------------------*/
  /* 系統選單: 個人資料 小雞放生 特別服務		 */
  /*-----------------------------------------------------*/

int
pip_system_menu()
{
  pip_do_menu(7, 0, pipsystemlist);
  return;
}
#endif		/* HAVE_GAME */
