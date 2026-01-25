/* ----------------------------------------------------- */
/* pip_fight.c        ( NTHU CS MapleBBS Ver 3.10 )      */
/* ----------------------------------------------------- */
/* target : 戰鬥選單                                     */
/* create :   /  /                                       */
/* update : 01/08/14		  			 */
/* author : dsyan.bbs@forever.twbbs.org                  */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */  
/* ----------------------------------------------------- */


#include "bbs.h"

#ifdef HAVE_GAME

#include "pip.h"


/*-------------------------------------------------------*/
/* 升級函式                                              */
/*-------------------------------------------------------*/


void
pip_levelup(success)
  int success;		/* 1:成功解決升級任務 0:失敗 */
{
  int level;

  d.quest = 0;

  level = d.level;
  d.exp -= level * 100;
  d.level = ++level;

  /* itoc.010730: 為了增加戰鬥的必要性，maxhp maxmp maxvp maxsp 
     這些屬性應該只在 exp 增加升級後，才能大量增加 */

  /* itoc.010730: 在設定怪物時請注意：在每次都成功解決升級任務之下
     level 是 n 級的小雞，其 maxhp/maxmp/maxvp/maxsp 期望值約是 0.75*(n^2) */

  d.maxhp += rand() % level;
  d.maxmp += rand() % level;
  d.maxvp += rand() % level;
  d.maxsp += rand() % level;

  if (success)		/* 如果成功解決升級任務，血加比較多 */
  {
    d.maxhp += level;
    d.maxmp += level;
    d.maxvp += level;
    d.maxsp += level;
    /* 任務完成，等級提升了 */
    vmsg("\xA5\xF4\xB0\xC8\xA7\xB9\xA6\xA8\xA1\x41\xB5\xA5\xAF\xC5\xB4\xA3\xA4\xC9\xA4\x46");
  }
  else
  {
    /* 任務失敗，等級提升了 */
    vmsg("\xA5\xF4\xB0\xC8\xA5\xA2\xB1\xD1\xA1\x41\xB5\xA5\xAF\xC5\xB4\xA3\xA4\xC9\xA4\x46");
  }

  /* 升級後補滿血 */
  d.hp = d.maxhp;
  d.mp = d.maxmp;
  d.vp = d.maxvp;
  d.sp = d.maxsp;
}


/* itoc.010731: 檢查經驗值是否已經達升級標準 */
static void
pip_check_levelup()
{
  /* itoc.010731: 每升一級要 (等級 * 100) 的經驗值 */
  /* 等級 n 怪物的 exp = n*5 (原則上打20隻怪物升一級) */

  /* itoc.020114: 限制等級上限，因為居然有人玩到數百萬級，遊戲的設計都被破壞了 */
  
  if ((d.level < 100) && (d.exp >= d.level * 100))
  {
    /* itoc.021031: 達升級標準時會取得一個升級任務，解完任務才能升級 */
    if (d.quest)
      /* 已達升級標準，但您必須執行或放棄任務才能升級 */
      vmsg("\xA4\x77\xB9\x46\xA4\xC9\xAF\xC5\xBC\xD0\xB7\xC7\xA1\x41\xA6\xFD\xB1\x7A\xA5\xB2\xB6\xB7\xB0\xF5\xA6\xE6\xA9\xCE\xA9\xF1\xB1\xF3\xA5\xF4\xB0\xC8\xA4\x7E\xAF\xE0\xA4\xC9\xAF\xC5");
    else
      pip_quest_new();
  }
}


/*-------------------------------------------------------*/
/* 戰鬥特區                    				 */
/*-------------------------------------------------------*/


/* itoc.010731.註解: 以下所有類似 value * (110 - rand() % 20) / 100; 的東西
   就是範圍在 value 的 90% ~ 110%，其期望值為 value */

/* itoc.010731: 加強防禦 */
/* resistmore = 40 表示對方攻擊變為 60%，resistmore = -20 表示對方攻擊變為 120% */

static int d_resistmore;	/* 小雞加強防禦 */
static int m_resistmore;	/* 怪物加強防禦 */

static int d_nodone;		/* 1:小雞還沒動作 0:小雞已執行完畢 */


  /*-----------------------------------------------------*/
  /* 怪物產生器            				 */
  /*-----------------------------------------------------*/


/* m.name[13] attribute hp maxhp attack spirit magic armor dodge money exp pic */
static playrule m;		/* 記錄怪物 */


/* itoc.010731: 為了避免怪物的資料太龐大，吃太多資源，寫一支怪物產生器 */
static void
badman_generate(area)
  int area;		/* 傳入區域來產生怪物 */
{
  int level;		/* 怪物的等級 */

  level = rand();	/* 借用 level當亂數，因為同餘的數都不同，所以只用一次 rand() 就好了 */

  memset(&m, 0, sizeof(playrule));	/* 初始化 */

  /* [1]炎之洞窟 [2]北方冰原 [3]古代遺跡 [4]人工島嶼 [5]地獄之門 [6]金庸群俠 */
  /* itoc.010731: 各區怪物的 name/attribute/pic 不同，以及等級範圍也不同 */

  /* itoc.010731: 等級為 n 級的怪物，其期望 
     maxhp = 0.75*(n^2)   (和玩家一樣)
     attack/spirit/magic/armor/dodge = n*10
     money = n*10
     exp = n*5 (原則上打20隻怪物升一級) */

  /* itoc.010731: 注意 m.name 長度是 13 (六個中文字) */
  /* itoc.010731: 現在各地區都只有一張圖，所以 m.pic 只好用指定的 */

  switch (area)
  {
  case '1':
    {
      /* 龍人 */
      /* 炎魔 */
      /* 火焰 */
      char race[3][7] = {"\xC0\x73\xA4\x48", "\xAA\xA2\xC5\x5D", "\xA4\xF5\xB5\x4B"};
      /* 官兵 */
      /* 守衛 */
      /* 士兵 */
      /* 囉嘍 */
      char title[4][7] = {"\xA9\x78\xA7\x4C", "\xA6\x75\xBD\xC3", "\xA4\x68\xA7\x4C", "\xC5\x6F\xB9\xC6"};
     
      sprintf(m.name, "%s%s", race[level % 3], title[level % 4]);
      level = d.level - 10 + level % 5;		/* 難慶低 */
      if (level <= 5)
	level = 5;
      m.maxhp = level * level / 2 + 30;
      m.attack = level * 8;
      m.spirit = level * 8;
      m.magic = level * 8;
      m.armor = level * 8;
      m.dodge = level * 8;
      m.money = level * 8;
      m.exp = level * 3;			/* 所有屬性都比較差，當然經驗值比期望值少 */
      m.attribute = -4;				/* 炎系 */
      m.pic = 101 + rand() % 3;
    }
    break;

  case '2':
    {
      /* 黑 */
      /* 白 */
      /* 紅 */
      /* 綠 */
      /* 藍 */
      /* 金 */
      /* 上古 */
      char color[7][5] = {"\xB6\xC2", "\xA5\xD5", "\xAC\xF5", "\xBA\xF1", "\xC2\xC5", "\xAA\xF7", "\xA4\x57\xA5\x6A"};
      /* 冰魔 */
      /* 雪怪 */
      /* 冰蠶 */
      /* 長毛象 */
      char race[4][9] = {"\xA6\x42\xC5\x5D", "\xB3\xB7\xA9\xC7", "\xA6\x42\xC5\xFA", "\xAA\xF8\xA4\xF2\xB6\x48"};

      sprintf(m.name, "%s%s", color[level % 7], race[level % 4]);
      level = d.level - 10 + level % 10;		/* 難度低 */
      if (level <= 5)
	level = 5;
      m.maxhp = level * level + 30;			/* 血比期望值多 */
      m.attack = level * 10;
      m.spirit = level * 10;
      m.magic = level * 10;
      m.armor = level * 10;
      m.dodge = level * 10;
      m.money = level * 12;				/* 錢比較多 */
      m.exp = level * 5;
      m.attribute = -3;					/* 冰系 */
      m.pic = 201 + rand() % 3;
    }
    break;

  case '3':
    {
      /* 金 */
      /* 木 */
      /* 水 */
      /* 火 */
      /* 土 */
      char color[5][3] = {"\xAA\xF7", "\xA4\xEC", "\xA4\xF4", "\xA4\xF5", "\xA4\x67"};
      
      /* %s元素 */
      sprintf(m.name, "%s\xA4\xB8\xAF\xC0", color[level % 5]);
      level = d.level - 10 + level % 20;		/* 難度中 */
      if (level <= 5)
	level = 5;
      m.maxhp = level * level * 3 / 4 + 30;
      m.attack = level * 12;				/* 物理攻擊比期望值強 */
      m.spirit = level * 8;				/* 內力指數比期望值差 */
      m.magic = level * 15;				/* 魔法力量比期望值強很多 */
      m.armor = level * 8;				/* 護甲強度比期望值差 */
      m.dodge = level * 10;
      m.money = level * 10;
      m.exp = level * 6;				/* 經驗值比較多 */
      m.attribute = 0;
      m.pic = 301 + rand() % 3;
    }
    break;

  case '4':
    {
      /* 英勇 */
      /* 神武 */
      /* 百戰 */
      /* 常勝 */
      /* 萬能 */
      char title[5][5] = {"\xAD\x5E\xAB\x69", "\xAF\xAB\xAA\x5A", "\xA6\xCA\xBE\xD4", "\xB1\x60\xB3\xD3", "\xB8\x55\xAF\xE0"};
      /* 騎士 */
      /* 武士 */
      /* 忍者 */
      /* 劍客 */
      /* 盜賊 */
      /* 僧侶 */
      /* 巫師 */
      /* 牧師 */
      char race[8][5] = {"\xC3\x4D\xA4\x68", "\xAA\x5A\xA4\x68", "\xA7\xD4\xAA\xCC", "\xBC\x43\xAB\xC8", "\xB5\x73\xB8\xE9", "\xB9\xAC\xAB\x51", "\xA7\xC5\xAE\x76", "\xAA\xAA\xAE\x76"};

      sprintf(m.name, "%s%s", title[level % 5], race[level % 8]);
      level = d.level + level % 10;			/* 難度高 */
      m.maxhp = level * level * 2 + 30;			/* 血比期望值多很多 */
      m.attack = level * 10;
      m.spirit = level * 10;
      m.magic = level * 10;
      m.armor = level * 12;				/* 護甲強度比期望值高 */
      m.dodge = level * 12;				/* 閃避指數比期望值高 */
      m.money = level * 10;
      m.exp = level * 13 / 2;				/* 經驗值比較多 */
      m.attribute = 0;
      m.pic = 401 + rand() % 3;
    }
    break;

  case '5':
    {
      /* 貪吃的 */
      /* 路過的 */
      /* 愛玩的 */
      char title[3][7] = {"\xB3\x67\xA6\x59\xAA\xBA", "\xB8\xF4\xB9\x4C\xAA\xBA", "\xB7\x52\xAA\xB1\xAA\xBA"};
      /* 幽靈 */
      /* 吸血鬼 */
      /* 黑無常 */
      /* 白無常 */
      /* 小鬼 */
      char race[5][7] = {"\xAB\xD5\xC6\x46", "\xA7\x6C\xA6\xE5\xB0\xAD", "\xB6\xC2\xB5\x4C\xB1\x60", "\xA5\xD5\xB5\x4C\xB1\x60", "\xA4\x70\xB0\xAD"};

      sprintf(m.name, "%s%s", title[level % 3], race[level % 5]);
      level = d.level + level % 20;			/* 難度高 */
      m.maxhp = level * level * 3 / 2 + 30;		/* 全部都很強 */
      m.attack = level * 15;
      m.spirit = level * 15;
      m.magic = level * 15;
      m.armor = level * 15;
      m.dodge = level * 15;
      m.money = level * 15;				/* 錢多很多 */
      m.exp = level * 7;				/* 經驗值多很多 */
      m.attribute = 0;
      m.pic = 501 + rand() % 3;
    }
    break;

  case '6':
    {
      /* itoc.010814: 金庸群俠傳 */
      int num;
      char name[27][13] = 
      {
	/* 韋小寶 */
	/* 段譽 */
	/* 狄雲 */
	/* 游坦之 */
	/* 虛竹 */
	"\xAD\xB3\xA4\x70\xC4\x5F", "\xAC\x71\xC5\x41",   "\xA8\x66\xB6\xB3",   "\xB4\xE5\xA9\x5A\xA4\xA7", "\xB5\xEA\xA6\xCB", 
	/* 小龍女 */
	/* 胡斐 */
	/* 慕容復 */
	/* 莫大 */
	/* 岳不群 */
	"\xA4\x70\xC0\x73\xA4\x6B", "\xAD\x4A\xB4\xB4",   "\xBC\x7D\xAE\x65\xB4\x5F", "\xB2\xF6\xA4\x6A",   "\xA9\xA8\xA4\xA3\xB8\x73", 
	/* 袁承志 */
	/* 黃蓉 */
	/* 石破天 */
	/* 左冷禪 */
	/* 金輪法王 */
	"\xB0\x4B\xA9\xD3\xA7\xD3", "\xB6\xC0\xBB\x54",   "\xA5\xDB\xAF\x7D\xA4\xD1", "\xA5\xAA\xA7\x4E\xC1\x49", "\xAA\xF7\xBD\xFC\xAA\x6B\xA4\xFD", 
	/* 令狐沖 */
	/* 張無忌 */
	/* 一燈 */
	/* 楊過 */
	/* 洪七公 */
	"\xA5\x4F\xAA\xB0\xA8\x52", "\xB1\x69\xB5\x4C\xA7\xD2", "\xA4\x40\xBF\x4F",   "\xB7\xA8\xB9\x4C",   "\xAC\x78\xA4\x43\xA4\xBD", 
	/* 黃藥師 */
	/* 歐陽峰 */
	/* 喬峰 */
	/* 郭靖 */
	/* 任我行 */
	"\xB6\xC0\xC3\xC4\xAE\x76", "\xBC\xDA\xB6\xA7\xAE\x70", "\xB3\xEC\xAE\x70",   "\xB3\xA2\xB9\x74",   "\xA5\xF4\xA7\xDA\xA6\xE6", 
	/* 周伯通 */
	/* 東方不敗 */
	"\xA9\x50\xA7\x42\xB3\x71", "\xAA\x46\xA4\xE8\xA4\xA3\xB1\xD1"
      };

      /* m.attr: +1:護身 +2:blitz +3:吸精 +4:拳 +5:劍 +6:刀 +7:暗器 (參考 pip_attack_skill())
                 -1:治療 -2:雷 -3:冰 -4:炎 -5:土 -6:風 -7:究極 */

      int attr[27] = 
      {
	+7, +3, +6, +3, +4, 
	+5, +6, -7, +6, +3, 
	+5, -1, -4, -3, -5, 
	+5, +4, +4, +5, +4, 
	-1, +2, +4, +4, +3, 
	+4, -6
      };

      num = level % 27;			/* 指定一隻 */
      strcpy(m.name, name[num]);
      m.attribute = attr[num];		/* 指定擅長技能 */
      num++;				/* 避免待會同餘數為 0 */

      level = d.level + num + rand() % 20;		/* 越後面的人物，難度越高 */
      m.maxhp = level * level * 2 + 500 * (rand() % num);
      m.attack = level * (15 + (rand() % num));
      m.spirit = level * (15 + (rand() % num));
      m.magic = level * (15 + (rand() % num));
      m.armor = level * (15 + (rand() % num));
      m.dodge = level * (15 + (rand() % num));
      m.money = level * 20;				/* 錢多很多 */
      m.exp = level * 10;				/* 經驗值多很多 */
      m.pic = 101 + 100 * (rand() % 5) + rand() % 3;	/* 101~501 102~502 103~503 十五選一 */
    }
    break;
  }

  m.hp = m.maxhp;	/* 血補滿 */
}


  /*-----------------------------------------------------*/
  /* 戰鬥技能參數                                        */
  /*-----------------------------------------------------*/


/* skillset: smode sno sbasic name[13] needhp needmp needvp needsp addtired effect pic message[41] */

/* itoc.010820: 特殊雜項技能 */

struct skillset skillXYZlist[] =
{
  /* 雜項的效果是程式訂定             hp   mp   vp   sp  tir  eff  pic  message */
  /* 特殊技能 */
  /* 特殊技能 */
  +0, 0x0000,      7, "\xAF\x53\xAE\xED\xA7\xDE\xAF\xE0",      0,   0,   0,   0,   0,   0,   0, "\xAF\x53\xAE\xED\xA7\xDE\xAF\xE0", 
  /* 左右互搏 */
  /* 您在無聊之際，自創了左手和右手打架的方法 */
  +0, 0x0001, 0x0000, "\xA5\xAA\xA5\x6B\xA4\xAC\xB7\x69",      0,   0,   0,   0,   0,   0,   0, "\xB1\x7A\xA6\x62\xB5\x4C\xB2\xE1\xA4\xA7\xBB\xDA\xA1\x41\xA6\xDB\xB3\xD0\xA4\x46\xA5\xAA\xA4\xE2\xA9\x4D\xA5\x6B\xA4\xE2\xA5\xB4\xAC\x5B\xAA\xBA\xA4\xE8\xAA\x6B", 	/* d_dr 大量上升 */
  /* 力拔山河 */
  /* 您的體質能讓您集氣甚快 */
  +0, 0x0002, 0x0000, "\xA4\x4F\xA9\xDE\xA4\x73\xAA\x65",      0,   0,   0,   0,   0,   0,   0, "\xB1\x7A\xAA\xBA\xC5\xE9\xBD\xE8\xAF\xE0\xC5\xFD\xB1\x7A\xB6\xB0\xAE\xF0\xAC\xC6\xA7\xD6", 			/* d_sr 大量上升 */
  /* 魔喚精靈 */
  /* 您和天地精靈簽下契約，施展魔法威力大增 */
  +0, 0x0004, 0x0000, "\xC5\x5D\xB3\xEA\xBA\xEB\xC6\x46",      0,   0,   0,   0,   0,   0,   0, "\xB1\x7A\xA9\x4D\xA4\xD1\xA6\x61\xBA\xEB\xC6\x46\xC3\xB1\xA4\x55\xAB\xB4\xAC\xF9\xA1\x41\xAC\x49\xAE\x69\xC5\x5D\xAA\x6B\xAB\xC2\xA4\x4F\xA4\x6A\xBC\x57", 	/* d_mr 大量上升 */
  /* 快馬加鞭 */
  /* 您可在敵我之間快速穿梭，如入無人之地 */
  +0, 0x0008, 0x0000, "\xA7\xD6\xB0\xA8\xA5\x5B\xC3\x40",      0,   0,   0,   0,   0,   0,   0, "\xB1\x7A\xA5\x69\xA6\x62\xBC\xC4\xA7\xDA\xA4\xA7\xB6\xA1\xA7\xD6\xB3\x74\xAC\xEF\xB1\xF4\xA1\x41\xA6\x70\xA4\x4A\xB5\x4C\xA4\x48\xA4\xA7\xA6\x61", 	/* d_hr 大量上升 */

  /* 尋龍訣 */
  /* 尋龍堪嶼之學，今日全被您悟透了 */
  +0, 0x0010, 0x0000, "\xB4\x4D\xC0\x73\xB3\x5A",        0,   0,   0,   0,   0,   0,   0, "\xB4\x4D\xC0\x73\xB3\xF4\xC0\xAC\xA4\xA7\xBE\xC7\xA1\x41\xA4\xB5\xA4\xE9\xA5\xFE\xB3\x51\xB1\x7A\xAE\xA9\xB3\x7A\xA4\x46", 		/* 戰鬥路上可以回復血 */
  /* 得來速 */
  /* 您在麥當勞前體會了速食的技巧 */
  +0, 0x0020, 0x0000, "\xB1\x6F\xA8\xD3\xB3\x74",        0,   0,   0,   0,   0,   0,   0, "\xB1\x7A\xA6\x62\xB3\xC1\xB7\xED\xB3\xD2\xAB\x65\xC5\xE9\xB7\x7C\xA4\x46\xB3\x74\xAD\xB9\xAA\xBA\xA7\xDE\xA5\xA9", 		/* 戰鬥中吃東西不耗回合數 */
  /* 優先攻擊 */
  /* 從此以後，戰場上您總能快人一步 */
  +0, 0x0040, 0x0000, "\xC0\x75\xA5\xFD\xA7\xF0\xC0\xBB",      0,   0,   0,   0,   0,   0,   0, "\xB1\x71\xA6\xB9\xA5\x48\xAB\xE1\xA1\x41\xBE\xD4\xB3\xF5\xA4\x57\xB1\x7A\xC1\x60\xAF\xE0\xA7\xD6\xA4\x48\xA4\x40\xA8\x42", 		/* 每次戰鬥必先攻擊 */
};


/* itoc.010801: 強度設定: 由耗點 (mp/vp/sp) 總合來決定效果 (effect)，needmp->effect，對應如下
   20->10 30->20 50->40 70->100 100->150 250->350 400->600 600->900 900->1500 請適當使用內插法
   若該技能耗點平均在二種以上的點數，那麼效果要打個折扣，即純武功或純魔法比較強 */

/* 三種武功 skillA ~ skillC */

struct skillset skillAlist[] =
{
  /* 護身的效果是增加 d_resistmore    hp   mp   vp   sp  tir  eff  pic  message */
  /* 護身 */
  /* 護身列表 */
  +1, 0x0000,      4, "\xC5\x40\xA8\xAD",          0,   0,   0,   0,   0,   0,   0, "\xC5\x40\xA8\xAD\xA6\x43\xAA\xED", 
  /* 鐵布衫 */
  /* 全身上下圍繞著一陣金光 */
  +1, 0x0001, 0x0000, "\xC5\x4B\xA5\xAC\xAD\x6D",        0,   0,   0,  80,   6,  60, 100, "\xA5\xFE\xA8\xAD\xA4\x57\xA4\x55\xB3\xF2\xC2\xB6\xB5\xDB\xA4\x40\xB0\x7D\xAA\xF7\xA5\xFA", 
  /* 金鐘罩 */
  /* 少林金剛不壞之身 */
  +1, 0x0002, 0x0001, "\xAA\xF7\xC4\xC1\xB8\x6E",        0,   0,   0, 150,   7,  80, 100, "\xA4\xD6\xAA\x4C\xAA\xF7\xAD\xE8\xA4\xA3\xC3\x61\xA4\xA7\xA8\xAD", 

  /* 斗轉星移 */
  /* 斗轉星移，二儀化四象 */
  +1, 0x0004, 0x0000, "\xA4\xE6\xC2\xE0\xAC\x50\xB2\xBE",      0,   0,  90,   5,   6,  70, 100, "\xA4\xE6\xC2\xE0\xAC\x50\xB2\xBE\xA1\x41\xA4\x47\xBB\xF6\xA4\xC6\xA5\x7C\xB6\x48", 
  /* 乾坤大挪移 */
  /* 乾坤大挪移，四象化八卦 */
  +1, 0x0008, 0x0004, "\xB0\xAE\xA9\x5B\xA4\x6A\xAE\xBF\xB2\xBE",    0,   0, 120, 100,   7,  90, 100, "\xB0\xAE\xA9\x5B\xA4\x6A\xAE\xBF\xB2\xBE\xA1\x41\xA5\x7C\xB6\x48\xA4\xC6\xA4\x4B\xA8\xF6", 
};

struct skillset skillBlist[] =
{
  /* 輕功耗內力，回復移動力           hp   mp   vp   sp  tir  eff  pic  message */
  /* 輕功 */
  /* 輕功列表 */
  +2, 0x0000,      3, "\xBB\xB4\xA5\x5C",         0,   0,   0,   0,   0,   0,   0, "\xBB\xB4\xA5\x5C\xA6\x43\xAA\xED", 
  /* 武當蹤雲梯 */
  /* 武當弟子所擅長的蹤雲梯 */
  +2, 0x0001, 0x0000, "\xAA\x5A\xB7\xED\xC2\xDC\xB6\xB3\xB1\xE8",    0,   0, -30,  15,   2,   0, 110, "\xAA\x5A\xB7\xED\xA7\xCC\xA4\x6C\xA9\xD2\xBE\xD5\xAA\xF8\xAA\xBA\xC2\xDC\xB6\xB3\xB1\xE8", 
  /* 神行百變 */
  /* 我變我變我變變變 */
  +2, 0x0002, 0x0001, "\xAF\xAB\xA6\xE6\xA6\xCA\xC5\xDC",      0,   0,-100,  60,   3,   0, 110, "\xA7\xDA\xC5\xDC\xA7\xDA\xC5\xDC\xA7\xDA\xC5\xDC\xC5\xDC\xC5\xDC", 
  /* 凌波微步 */
  /* 按照八卦，您使出凌波微步 */
  +2, 0x0004, 0x0003, "\xAD\xE2\xAA\x69\xB7\x4C\xA8\x42",      0,   0,-200, 110,   3,   0, 110, "\xAB\xF6\xB7\xD3\xA4\x4B\xA8\xF6\xA1\x41\xB1\x7A\xA8\xCF\xA5\x58\xAD\xE2\xAA\x69\xB7\x4C\xA8\x42", 
};

struct skillset skillClist[] =
{
  /* 心法增加疲勞，回復內力           hp   mp   vp   sp  tir  eff  pic  message */
  /* 心法 */
  /* 心法列表 */
  +3, 0x0000,     12, "\xA4\xDF\xAA\x6B",          0,   0,   0,   0,   0,   0,   0, "\xA4\xDF\xAA\x6B\xA6\x43\xAA\xED", 
  /* 神照經 */
  /* 週身運起了神照真經 */
  +3, 0x0001, 0x0000, "\xAF\xAB\xB7\xD3\xB8\x67",        0,   0,   0, -20,  10,  50, 120, "\xB6\x67\xA8\xAD\xB9\x42\xB0\x5F\xA4\x46\xAF\xAB\xB7\xD3\xAF\x75\xB8\x67", 
  /* 紫霞神功 */
  /* 您運起紫霞神功，臉色紅潤 */
  +3, 0x0002, 0x0000, "\xB5\xB5\xC1\xF8\xAF\xAB\xA5\x5C",     0,   0,   0, -60,  20,  50, 120, "\xB1\x7A\xB9\x42\xB0\x5F\xB5\xB5\xC1\xF8\xAF\xAB\xA5\x5C\xA1\x41\xC1\x79\xA6\xE2\xAC\xF5\xBC\xED", 
  /* 九陰真經 */
  /* 王重陽的得意絕招－九陰真經 */
  +3, 0x0004, 0x0000, "\xA4\x45\xB3\xB1\xAF\x75\xB8\x67",      0,   0,   0,-200,  28,  50, 120, "\xA4\xFD\xAD\xAB\xB6\xA7\xAA\xBA\xB1\x6F\xB7\x4E\xB5\xB4\xA9\xDB\xA1\xD0\xA4\x45\xB3\xB1\xAF\x75\xB8\x67", 
  /* 九陽真經 */
  /* 九陽神功，隨心而發 */
  +3, 0x0008, 0x0000, "\xA4\x45\xB6\xA7\xAF\x75\xB8\x67",      0,   0,   0,-250,  30,  50, 120, "\xA4\x45\xB6\xA7\xAF\xAB\xA5\x5C\xA1\x41\xC0\x48\xA4\xDF\xA6\xD3\xB5\x6F", 

  /* 小無相功 */
  /* 小無相功，陰陽調和 */
  +3, 0x0010, 0x0000, "\xA4\x70\xB5\x4C\xAC\xDB\xA5\x5C",     0,   0,   0, -40,  15,  50, 120, "\xA4\x70\xB5\x4C\xAC\xDB\xA5\x5C\xA1\x41\xB3\xB1\xB6\xA7\xBD\xD5\xA9\x4D", 
  /* 洗髓經 */
  /* 您施展出傳說中的洗髓經 */
  +3, 0x0020, 0x0010, "\xAC\x7E\xC5\xE8\xB8\x67",        0,   0,   0, -80,  23,  50, 120, "\xB1\x7A\xAC\x49\xAE\x69\xA5\x58\xB6\xC7\xBB\xA1\xA4\xA4\xAA\xBA\xAC\x7E\xC5\xE8\xB8\x67", 
  /* 十八泥偶 */
  /* 您從十八尊泥偶中所參透的內功 */
  +3, 0x0040, 0x0030, "\xA4\x51\xA4\x4B\xAA\x64\xB0\xB8",      0,   0,   0,-180,  27,  50, 120, "\xB1\x7A\xB1\x71\xA4\x51\xA4\x4B\xB4\x4C\xAA\x64\xB0\xB8\xA4\xA4\xA9\xD2\xB0\xD1\xB3\x7A\xAA\xBA\xA4\xBA\xA5\x5C", 
  /* 易筋經 */
  /* 少林不傳之學－達摩易筋 */
  +3, 0x0080, 0x0070, "\xA9\xF6\xB5\xAC\xB8\x67",        0,   0,   0,-360,  35,  50, 120, "\xA4\xD6\xAA\x4C\xA4\xA3\xB6\xC7\xA4\xA7\xBE\xC7\xA1\xD0\xB9\x46\xBC\xAF\xA9\xF6\xB5\xAC", 

  /* 用其他換 sp */
  /* 神木王鼎 */
  /* 您使出毒，並伺機吸了對方一些內力 */
  +3, 0x0100, 0x0000, "\xAF\xAB\xA4\xEC\xA4\xFD\xB9\xA9",     15,   0,   0, -30,   1,  50, 120, "\xB1\x7A\xA8\xCF\xA5\x58\xAC\x72\xA1\x41\xA8\xC3\xA6\xF8\xBE\xF7\xA7\x6C\xA4\x46\xB9\xEF\xA4\xE8\xA4\x40\xA8\xC7\xA4\xBA\xA4\x4F", 
  /* 化功大法 */
  /* 您從敵手身上化來了一絲絲內力 */
  +3, 0x0200, 0x0100, "\xA4\xC6\xA5\x5C\xA4\x6A\xAA\x6B",     0,  35,   0, -60,   1,  50, 120, "\xB1\x7A\xB1\x71\xBC\xC4\xA4\xE2\xA8\xAD\xA4\x57\xA4\xC6\xA8\xD3\xA4\x46\xA4\x40\xB5\xB7\xB5\xB7\xA4\xBA\xA4\x4F", 
  /* 吸星大法 */
  /* 在吸納對方內力之餘，您得好好休息一番 */
  +3, 0x0400, 0x0300, "\xA7\x6C\xAC\x50\xA4\x6A\xAA\x6B",      0,  55,   0,-150,   7,  50, 120, "\xA6\x62\xA7\x6C\xAF\xC7\xB9\xEF\xA4\xE8\xA4\xBA\xA4\x4F\xA4\xA7\xBE\x6C\xA1\x41\xB1\x7A\xB1\x6F\xA6\x6E\xA6\x6E\xA5\xF0\xAE\xA7\xA4\x40\xB5\x66", 
  /* 北冥神功 */
  /* 無意之中，您從對方吸取了大量內力 */
  +3, 0x0800, 0x0700, "\xA5\x5F\xAD\xDF\xAF\xAB\xA5\x5C",     0, 100,   0,-180,   1,  50, 120, "\xB5\x4C\xB7\x4E\xA4\xA7\xA4\xA4\xA1\x41\xB1\x7A\xB1\x71\xB9\xEF\xA4\xE8\xA7\x6C\xA8\xFA\xA4\x46\xA4\x6A\xB6\x71\xA4\xBA\xA4\x4F", 
};

struct skillset skillDlist[] =
{
  /* 拳法主要耗內力                   hp   mp   vp   sp  tir  eff  pic  message */
  /* 拳法 */
  /* 拳法列表 */
  +4, 0x0000,     16, "\xAE\xB1\xAA\x6B",          0,   0,   0,   0,   0,   0,   0, "\xAE\xB1\xAA\x6B\xA6\x43\xAA\xED", 
  /* 少林長拳 */
  /* 少林長拳，虎虎生風 */
  +4, 0x0001, 0x0000, "\xA4\xD6\xAA\x4C\xAA\xF8\xAE\xB1",      0,   0,   0,  50,  10,  40, 130, "\xA4\xD6\xAA\x4C\xAA\xF8\xAE\xB1\xA1\x41\xAA\xEA\xAA\xEA\xA5\xCD\xAD\xB7", 		/* 少林系掌法全拼內力 */
  /* 羅漢拳 */
  /* 少林十八羅漢 */
  +4, 0x0002, 0x0001, "\xC3\xB9\xBA\x7E\xAE\xB1",        0,   0,   0,  70,  10, 100, 130, "\xA4\xD6\xAA\x4C\xA4\x51\xA4\x4B\xC3\xB9\xBA\x7E", 
  /* 伏虎拳 */
  /* 南山伏虎 */
  +4, 0x0004, 0x0003, "\xA5\xF1\xAA\xEA\xAE\xB1",        0,   0,   0, 100,  10, 150, 130, "\xAB\x6E\xA4\x73\xA5\xF1\xAA\xEA", 
  /* 般若掌 */
  /* 少林般若掌 */
  +4, 0x0008, 0x0007, "\xAF\xEB\xAD\x59\xB4\x78",        0,   0,   0, 150,  10, 200, 130, "\xA4\xD6\xAA\x4C\xAF\xEB\xAD\x59\xB4\x78", 
  /* 大力金剛指 */
  /* 少林大力金剛指 */
  +4, 0x0010, 0x000F, "\xA4\x6A\xA4\x4F\xAA\xF7\xAD\xE8\xAB\xFC",    0,   0,   0, 250,  10, 350, 130, "\xA4\xD6\xAA\x4C\xA4\x6A\xA4\x4F\xAA\xF7\xAD\xE8\xAB\xFC", 
  /* 無相劫指 */
  /* 少林無相劫指 */
  +4, 0x0020, 0x001F, "\xB5\x4C\xAC\xDB\xA7\x54\xAB\xFC",      0,   0,   0, 400,  10, 600, 130, "\xA4\xD6\xAA\x4C\xB5\x4C\xAC\xDB\xA7\x54\xAB\xFC", 
  /* 六脈神劍 */
  /* 大理段家六脈神劍 */
  +4, 0x0040, 0x001F, "\xA4\xBB\xAF\xDF\xAF\xAB\xBC\x43",      0,   0, 200, 100,   7, 400, 130, "\xA4\x6A\xB2\x7A\xAC\x71\xAE\x61\xA4\xBB\xAF\xDF\xAF\xAB\xBC\x43", 
  /* 黯然消魂掌 */
  /* 您傷心地擊出一掌 */
  +4, 0x0080, 0x0003, "\xC5\x66\xB5\x4D\xAE\xF8\xBB\xEE\xB4\x78",   30,   0,   0, 100,  12, 300, 130, "\xB1\x7A\xB6\xCB\xA4\xDF\xA6\x61\xC0\xBB\xA5\x58\xA4\x40\xB4\x78", 		/* 兼耗血! */

  /* 武當掌法 */
  /* 您打出武當派最基本的掌法 */
  +4, 0x0100, 0x0000, "\xAA\x5A\xB7\xED\xB4\x78\xAA\x6B",      0,   0,  30,  30,   6,  40, 130, "\xB1\x7A\xA5\xB4\xA5\x58\xAA\x5A\xB7\xED\xAC\xA3\xB3\xCC\xB0\xF2\xA5\xBB\xAA\xBA\xB4\x78\xAA\x6B", 	/* 武當系掌法兼耗移動力 */
  /* 太極拳 */
  /* 太極拳法，以柔克剛 */
  +4, 0x0200, 0x0100, "\xA4\xD3\xB7\xA5\xAE\xB1",        0,   0, 180,  80,   6, 350, 130, "\xA4\xD3\xB7\xA5\xAE\xB1\xAA\x6B\xA1\x41\xA5\x48\xAC\x58\xA7\x4A\xAD\xE8", 
  /* 醉拳 */
  /* 喝了一口酒，您膽子大了起來 */
  +4, 0x0400, 0x0100, "\xBE\x4B\xAE\xB1",          0,   0, 120,  60,  10, 200, 130, "\xB3\xDC\xA4\x46\xA4\x40\xA4\x66\xB0\x73\xA1\x41\xB1\x7A\xC1\x78\xA4\x6C\xA4\x6A\xA4\x46\xB0\x5F\xA8\xD3", 

  /* 天馬流星拳 */
  /* 啊啊，天馬流★拳 */
  +4, 0x0800, 0x0000, "\xA4\xD1\xB0\xA8\xAC\x79\xAC\x50\xAE\xB1",    0,  30,  30,  30,   8, 120, 130, "\xB0\xDA\xB0\xDA\xA1\x41\xA4\xD1\xB0\xA8\xAC\x79\xA1\xB9\xAE\xB1", 		/* 怪怪類掌法兼耗移動力及法力 */
  /* 龜派氣功 */
  /* 吃我一記龜派氣功 */
  +4, 0x1000, 0x0800, "\xC0\x74\xAC\xA3\xAE\xF0\xA5\x5C",     0,  60,  60,  60,   8, 200, 130, "\xA6\x59\xA7\xDA\xA4\x40\xB0\x4F\xC0\x74\xAC\xA3\xAE\xF0\xA5\x5C", 
  /* 元氣玉 */
  /* 一塊元氣飛出 */
  +4, 0x2000, 0x1800, "\xA4\xB8\xAE\xF0\xA5\xC9",        0, 120, 120, 120,   8, 450, 130, "\xA4\x40\xB6\xF4\xA4\xB8\xAE\xF0\xAD\xB8\xA5\x58", 
  /* 界王拳 */
  /* 看我五倍的界王拳 */
  +4, 0x4000, 0x3800, "\xAC\xC9\xA4\xFD\xAE\xB1",        0, 240, 240, 240,   8, 950, 130, "\xAC\xDD\xA7\xDA\xA4\xAD\xAD\xBF\xAA\xBA\xAC\xC9\xA4\xFD\xAE\xB1", 

  /* 降龍十八掌 */
  /* 亢龍有悔！ */
  +4, 0x8000, 0x7FFF, "\xAD\xB0\xC0\x73\xA4\x51\xA4\x4B\xB4\x78",    0,   0,   0, 900,  15, 1500,130, "\xA4\xAE\xC0\x73\xA6\xB3\xAE\xAC\xA1\x49", 			/* 終極拳法，內力要深厚啊 */
};

struct skillset skillElist[] =
{
  /* 劍法主要耗移動力，其次內力       hp   mp   vp   sp  tir  eff  pic  message */
  /* 劍法 */
  /* 劍法列表 */
  +5, 0x0000,     16, "\xBC\x43\xAA\x6B",          0,   0,   0,   0,   0,   0,   0, "\xBC\x43\xAA\x6B\xA6\x43\xAA\xED", 
  /* 武當劍法 */
  /* 武當劍法，聞名天下 */
  +5, 0x0001, 0x0000, "\xAA\x5A\xB7\xED\xBC\x43\xAA\x6B",      0,   0,  30,  20,   6,  35, 140, "\xAA\x5A\xB7\xED\xBC\x43\xAA\x6B\xA1\x41\xBB\x44\xA6\x57\xA4\xD1\xA4\x55", 		/* 武當系劍法，首重移動力 */
  /* 虛影劍法 */
  /* 虛影劍法，劍至影至 */
  +5, 0x0002, 0x0001, "\xB5\xEA\xBC\x76\xBC\x43\xAA\x6B",      0,   0,  50,  20,   6,  90, 140, "\xB5\xEA\xBC\x76\xBC\x43\xAA\x6B\xA1\x41\xBC\x43\xA6\xDC\xBC\x76\xA6\xDC", 
  /* 伏羲劍法 */
  /* 伏羲陰陽，劍在人在 */
  +5, 0x0004, 0x0003, "\xA5\xF1\xBF\xAA\xBC\x43\xAA\x6B",      0,   0,  70,  20,   6, 120, 140, "\xA5\xF1\xBF\xAA\xB3\xB1\xB6\xA7\xA1\x41\xBC\x43\xA6\x62\xA4\x48\xA6\x62", 
  /* 玄陽劍法 */
  /* 玄陽絕火，劍發氣發 */
  +5, 0x0008, 0x0007, "\xA5\xC8\xB6\xA7\xBC\x43\xAA\x6B",      0,   0, 100,  30,   6, 150, 140, "\xA5\xC8\xB6\xA7\xB5\xB4\xA4\xF5\xA1\x41\xBC\x43\xB5\x6F\xAE\xF0\xB5\x6F", 
  /* 沖虛劍法 */
  /* 沖虛劍法，一氣呵成 */
  +5, 0x0010, 0x000F, "\xA8\x52\xB5\xEA\xBC\x43\xAA\x6B",      0,   0, 150,  30,   6, 220, 140, "\xA8\x52\xB5\xEA\xBC\x43\xAA\x6B\xA1\x41\xA4\x40\xAE\xF0\xA8\xFE\xA6\xA8", 
  /* 兩儀劍法 */
  /* 兩儀劍法，兩儀四象 */
  +5, 0x0020, 0x001F, "\xA8\xE2\xBB\xF6\xBC\x43\xAA\x6B",      0,   0, 250,  30,   6, 365, 140, "\xA8\xE2\xBB\xF6\xBC\x43\xAA\x6B\xA1\x41\xA8\xE2\xBB\xF6\xA5\x7C\xB6\x48", 
  /* 太極劍法 */
  /* 太極劍法，以柔克剛 */
  +5, 0x0040, 0x003F, "\xA4\xD3\xB7\xA5\xBC\x43\xAA\x6B",      0,   0, 400,  30,   5, 620, 140, "\xA4\xD3\xB7\xA5\xBC\x43\xAA\x6B\xA1\x41\xA5\x48\xAC\x58\xA7\x4A\xAD\xE8", 
  /* 判官筆 */
  /* 判官之筆，絕不手軟 */
  +5, 0x0080, 0x003F, "\xA7\x50\xA9\x78\xB5\xA7",        0,   0, 200, 200,   7, 580, 140, "\xA7\x50\xA9\x78\xA4\xA7\xB5\xA7\xA1\x41\xB5\xB4\xA4\xA3\xA4\xE2\xB3\x6E", 

  /* 天女散花 */
  /* 天女散花，天花亂墜 */
  +5, 0x0100, 0x0000, "\xA4\xD1\xA4\x6B\xB4\xB2\xAA\xE1",     10,   0,  30,  20,   6,  40, 140, "\xA4\xD1\xA4\x6B\xB4\xB2\xAA\xE1\xA1\x41\xA4\xD1\xAA\xE1\xB6\xC3\xBC\x59", 
  /* 狂花劍法 */
  /* 狂花劍法，劍不虛發 */
  +5, 0x0200, 0x0100, "\xA8\x67\xAA\xE1\xBC\x43\xAA\x6B",     40,   0, 100,  40,   7, 250, 140, "\xA8\x67\xAA\xE1\xBC\x43\xAA\x6B\xA1\x41\xBC\x43\xA4\xA3\xB5\xEA\xB5\x6F", 
  /* 玄鐵劍法 */
  /* 玄劍重劍，力道十足 */
  +5, 0x0400, 0x0300, "\xA5\xC8\xC5\x4B\xBC\x43\xAA\x6B",     80,   0,  70, 120,   7, 350, 140, "\xA5\xC8\xBC\x43\xAD\xAB\xBC\x43\xA1\x41\xA4\x4F\xB9\x44\xA4\x51\xA8\xAC", 
  /* 獨孤九劍 */
  /* 獨孤九劍，求敗不能 */
  +5, 0x0800, 0x0700, "\xBF\x57\xA9\x74\xA4\x45\xBC\x43",      0,   0, 250,  70,  12, 400, 140, "\xBF\x57\xA9\x74\xA4\x45\xBC\x43\xA1\x41\xA8\x44\xB1\xD1\xA4\xA3\xAF\xE0", 

  /* 辟邪劍法 */
  /* 欲練神功，必先自宮 */
  +5, 0x1000, 0x0000, "\xB9\x40\xA8\xB8\xBC\x43\xAA\x6B",    100,   0, 100, 100,  15, 450, 140, "\xB1\xFD\xBD\x6D\xAF\xAB\xA5\x5C\xA1\x41\xA5\xB2\xA5\xFD\xA6\xDB\xAE\x63", 		/* 耗不少血，所以威力加強 */
  /* 葵花寶典 */
  /* 若不自宮，也能成功 */
  +5, 0x2000, 0x1000, "\xB8\xAA\xAA\xE1\xC4\x5F\xA8\xE5",    300,   0, 250, 200,  15, 1200,140, "\xAD\x59\xA4\xA3\xA6\xDB\xAE\x63\xA1\x41\xA4\x5D\xAF\xE0\xA6\xA8\xA5\x5C", 

  /* 無名劍訣 */
  /* 無名英雄，十分霸道 */
  +5, 0x4000, 0x3000, "\xB5\x4C\xA6\x57\xBC\x43\xB3\x5A",      0,  50, 250, 200,   6, 700, 140, "\xB5\x4C\xA6\x57\xAD\x5E\xB6\xAF\xA1\x41\xA4\x51\xA4\xC0\xC5\x51\xB9\x44", 
  /* 萬劍歸宗 */
  /* 此刻無劍勝有劍 */
  +5, 0x8000, 0x7FFF, "\xB8\x55\xBC\x43\xC2\x6B\xA9\x76",      0, 150, 700, 150,   5, 1500,140, "\xA6\xB9\xA8\xE8\xB5\x4C\xBC\x43\xB3\xD3\xA6\xB3\xBC\x43", 		/* 終極劍法，屬性要萬能啊 */
};

struct skillset skillFlist[] =
{
  /* 刀法兼耗移動力及內力             hp   mp   vp   sp  tir  eff  pic  message */
  /* 刀法 */
  /* 刀法列表 */
  +6, 0x0000,      5, "\xA4\x4D\xAA\x6B",          0,   0,   0,   0,   0,   0,   0, "\xA4\x4D\xAA\x6B\xA6\x43\xAA\xED", 
  /* 西瓜刀法 */
  /* 您由切西瓜中所體會的刀法 */
  +6, 0x0001, 0x0000, "\xA6\xE8\xA5\xCA\xA4\x4D\xAA\x6B",      0,   0,  40,  40,   6, 100, 150, "\xB1\x7A\xA5\xD1\xA4\xC1\xA6\xE8\xA5\xCA\xA4\xA4\xA9\xD2\xC5\xE9\xB7\x7C\xAA\xBA\xA4\x4D\xAA\x6B", 
  /* 血刀經 */
  /* 血刀沾著您的血，狂悍無比 */
  +6, 0x0002, 0x0000, "\xA6\xE5\xA4\x4D\xB8\x67",       15,   0,  70,  70,   7, 200, 152, "\xA6\xE5\xA4\x4D\xAA\x67\xB5\xDB\xB1\x7A\xAA\xBA\xA6\xE5\xA1\x41\xA8\x67\xAE\xAB\xB5\x4C\xA4\xF1", 
  /* 胡家刀法 */
  /* 胡家家傳刀法 */
  +6, 0x0004, 0x0000, "\xAD\x4A\xAE\x61\xA4\x4D\xAA\x6B",      0,   0, 170, 160,   6, 400, 152, "\xAD\x4A\xAE\x61\xAE\x61\xB6\xC7\xA4\x4D\xAA\x6B", 
  /* 軒轅菜刀 */
  /* 上古黃帝時代所流傳下來的刀法 */
  +6, 0x0008, 0x0000, "\xB0\x61\xC1\xD5\xB5\xE6\xA4\x4D",      0,   0, 210, 230,   6, 600, 151, "\xA4\x57\xA5\x6A\xB6\xC0\xAB\xD2\xAE\xC9\xA5\x4E\xA9\xD2\xAC\x79\xB6\xC7\xA4\x55\xA8\xD3\xAA\xBA\xA4\x4D\xAA\x6B", 
  /* 猛牛青龍斬 */
  /* 黑暗料理界最強絕技 */
  +6, 0x0010, 0x0000, "\xB2\x72\xA4\xFB\xAB\x43\xC0\x73\xB1\xD9",    0,  50, 400, 150,   7, 800, 153, "\xB6\xC2\xB7\x74\xAE\xC6\xB2\x7A\xAC\xC9\xB3\xCC\xB1\x6A\xB5\xB4\xA7\xDE", 
  /* 霹靂狂刀 */
  /* 霹靂狂刀，一流 */
  +6, 0x0020, 0x0000, "\xC5\x52\xC6\x45\xA8\x67\xA4\x4D",     25,  30, 300, 350,   7, 1000,152, "\xC5\x52\xC6\x45\xA8\x67\xA4\x4D\xA1\x41\xA4\x40\xAC\x79", 
};


struct skillset skillGlist[] =
{
  /* 暗器主要耗移動力，毒兼損一點血   hp   mp   vp   sp  tir  eff  pic  message */  
  /* 暗器的公式比較不一樣，適合低能力者使用，同樣耗點下，effect 比正常值高一些 */
  /* 暗器˙毒 */
  /* 暗器列表 */
  +7, 0x0000,      5, "\xB7\x74\xBE\xB9\xA3\xBB\xAC\x72",      0,   0,   0,   0,   0,   0,   0, "\xB7\x74\xBE\xB9\xA6\x43\xAA\xED", 
  /* 蒙汗藥 */
  /* 使人昏迷一段時間的蒙汗藥 */
  +7, 0x0001, 0x0000, "\xBB\x58\xA6\xBD\xC3\xC4",        5,   0,  20,   0,   5,  25, 160, "\xA8\xCF\xA4\x48\xA9\xFC\xB0\x67\xA4\x40\xAC\x71\xAE\xC9\xB6\xA1\xAA\xBA\xBB\x58\xA6\xBD\xC3\xC4", 
  /* 鴨片 */
  /* 使人上癮的鴨片 */
  +7, 0x0002, 0x0001, "\xC0\x6E\xA4\xF9",         10,   0,  40,   0,   5,  55, 160, "\xA8\xCF\xA4\x48\xA4\x57\xC5\x7D\xAA\xBA\xC0\x6E\xA4\xF9", 
  /* 搖頭丸 */
  /* 含著奶嘴的搖頭公子 */
  +7, 0x0004, 0x0003, "\xB7\x6E\xC0\x59\xA4\x59",       40,   0, 160,   0,   5, 350, 160, "\xA7\x74\xB5\xDB\xA5\xA4\xBC\x4C\xAA\xBA\xB7\x6E\xC0\x59\xA4\xBD\xA4\x6C", 
  /* 安非他命 */
  /* 俗稱冰塊的安公子 */
  +7, 0x0008, 0x0007, "\xA6\x77\xAB\x44\xA5\x4C\xA9\x52",     80,   0, 320,   0,   5, 650, 160, "\xAB\x55\xBA\xD9\xA6\x42\xB6\xF4\xAA\xBA\xA6\x77\xA4\xBD\xA4\x6C", 

  /* 袖裡劍 */
  /* 袖口中射出您事先藏好的快劍 */
  +7, 0x0010, 0x0000, "\xB3\x53\xB8\xCC\xBC\x43",        0,   0,  20,   5,   5,  25, 161, "\xB3\x53\xA4\x66\xA4\xA4\xAE\x67\xA5\x58\xB1\x7A\xA8\xC6\xA5\xFD\xC2\xC3\xA6\x6E\xAA\xBA\xA7\xD6\xBC\x43", 
  /* 飛蝗走石 */
  /* 您擲出滿天的死煌石 */
  +7, 0x0020, 0x0010, "\xAD\xB8\xBD\xC0\xA8\xAB\xA5\xDB",      0,   0,  40,  10,   5,  55, 161, "\xB1\x7A\xC2\x59\xA5\x58\xBA\xA1\xA4\xD1\xAA\xBA\xA6\xBA\xB7\xD7\xA5\xDB", 
  /* 笑裡藏刀 */
  /* 微笑之間，您突然捅出一刀 */
  +7, 0x0040, 0x0030, "\xAF\xBA\xB8\xCC\xC2\xC3\xA4\x4D",      0,   0, 160,  40,   5, 350, 161, "\xB7\x4C\xAF\xBA\xA4\xA7\xB6\xA1\xA1\x41\xB1\x7A\xAC\xF0\xB5\x4D\xD1\xB6\xA5\x58\xA4\x40\xA4\x4D", 
  /* 含沙射影 */
  /* 漫天飛沙之中，只見敵人中了您的暗器 */
  +7, 0x0080, 0x0070, "\xA7\x74\xA8\x46\xAE\x67\xBC\x76",      0,   0, 320,  80,   5, 650, 162, "\xBA\xA9\xA4\xD1\xAD\xB8\xA8\x46\xA4\xA7\xA4\xA4\xA1\x41\xA5\x75\xA8\xA3\xBC\xC4\xA4\x48\xA4\xA4\xA4\x46\xB1\x7A\xAA\xBA\xB7\x74\xBE\xB9", 
};


/* 七類法術 spellA ~ spellG */
/* 若 needhp <0  則表示補 hp，餘類推 */
/* itoc.010801: 各系的 Top 法術說明要押韻 :p */

struct skillset spellAlist[] =
{
  /* 治療法術                         hp   mp   vp   sp  tir  eff  pic  message */
  /* 治療法術 */
  /* 治療法術 */
  -1, 0x0000,     12, "\xAA\x76\xC0\xF8\xAA\x6B\xB3\x4E",      0,   0,   0,   0,   0,   0,   0, "\xAA\x76\xC0\xF8\xAA\x6B\xB3\x4E", 

  /* 補血魔法 */
  /* 基本氣療 */
  /* 您感覺到暖和了些 */
  -1, 0x0001, 0x0000, "\xB0\xF2\xA5\xBB\xAE\xF0\xC0\xF8",    -40,  50,   0,   0,   2,   0, 200, "\xB1\x7A\xB7\x50\xC4\xB1\xA8\xEC\xB7\x78\xA9\x4D\xA4\x46\xA8\xC7", 
  /* 凝神歸元 */
  /* 閉目養神，感覺好了一點 */
  -1, 0x0002, 0x0001, "\xBE\xAE\xAF\xAB\xC2\x6B\xA4\xB8",   -150, 100,   0,   0,   2,   0, 200, "\xB3\xAC\xA5\xD8\xBE\x69\xAF\xAB\xA1\x41\xB7\x50\xC4\xB1\xA6\x6E\xA4\x46\xA4\x40\xC2\x49", 
  /* 元靈歸心 */
  /* 精神專心了起來 */
  -1, 0x0004, 0x0002, "\xA4\xB8\xC6\x46\xC2\x6B\xA4\xDF",   -350, 250,   0,   0,   2,   0, 200, "\xBA\xEB\xAF\xAB\xB1\x4D\xA4\xDF\xA4\x46\xB0\x5F\xA8\xD3", 
  /* 五氣朝元 */
  /* 您吸收了大自然的力量 */
  -1, 0x0008, 0x0004, "\xA4\xAD\xAE\xF0\xB4\xC2\xA4\xB8",   -900, 600,   0,   0,   2,   0, 200, "\xB1\x7A\xA7\x6C\xA6\xAC\xA4\x46\xA4\x6A\xA6\xDB\xB5\x4D\xAA\xBA\xA4\x4F\xB6\x71", 

  /* 補移動力/疲勞魔法 */
  /* 清心魔咒 */
  /* 您的疲勞恢復了 */
  -1, 0x0010, 0x0000, "\xB2\x4D\xA4\xDF\xC5\x5D\xA9\x47",      0,  50,  -40,  0,  -5,   0, 200, "\xB1\x7A\xAA\xBA\xAF\x68\xB3\xD2\xAB\xEC\xB4\x5F\xA4\x46", 
  /* 女神庇祐 */
  /* 您感覺到有人偷偷地保護著您 */
  -1, 0x0020, 0x0010, "\xA4\x6B\xAF\xAB\xA7\xC8\xAF\xA7",      0, 100, -150,  0, -10,   0, 200, "\xB1\x7A\xB7\x50\xC4\xB1\xA8\xEC\xA6\xB3\xA4\x48\xB0\xBD\xB0\xBD\xA6\x61\xAB\x4F\xC5\x40\xB5\xDB\xB1\x7A", 
  /* 至聖光芒 */
  /* 一陣聖光圍繞身旁 */
  -1, 0x0040, 0x0030, "\xA6\xDC\xB8\x74\xA5\xFA\xA8\x7E",      0, 250, -350,  0, -15,   0, 200, "\xA4\x40\xB0\x7D\xB8\x74\xA5\xFA\xB3\xF2\xC2\xB6\xA8\xAD\xAE\xC7", 
  /* 天使加持 */
  /* 天使的力量加持在您身上 */
  -1, 0x0080, 0x0070, "\xA4\xD1\xA8\xCF\xA5\x5B\xAB\xF9",      0, 600, -900,  0, -20,   0, 200, "\xA4\xD1\xA8\xCF\xAA\xBA\xA4\x4F\xB6\x71\xA5\x5B\xAB\xF9\xA6\x62\xB1\x7A\xA8\xAD\xA4\x57", 

  /* 補血/移動力/疲勞魔法，要先學會前面二種對應的法術 */
  /* 聖佑 */
  /* 您得到來自上帝的祝福 */
  -1, 0x0100, 0x0011, "\xB8\x74\xA6\xF6",        -40,  120,  -40,  0, -5,   0, 200, "\xB1\x7A\xB1\x6F\xA8\xEC\xA8\xD3\xA6\xDB\xA4\x57\xAB\xD2\xAA\xBA\xAF\xAC\xBA\xD6", 
  /* 風雲 */
  /* 您得到來自風雲的祝福 */
  -1, 0x0200, 0x0033, "\xAD\xB7\xB6\xB3",       -150,  240, -150,  0, -5,   0, 200, "\xB1\x7A\xB1\x6F\xA8\xEC\xA8\xD3\xA6\xDB\xAD\xB7\xB6\xB3\xAA\xBA\xAF\xAC\xBA\xD6", 
  /* 星空 */
  /* 您得到來自星空的祝福 */
  -1, 0x0400, 0x0077, "\xAC\x50\xAA\xC5",       -350,  600, -350,  0, -5,   0, 200, "\xB1\x7A\xB1\x6F\xA8\xEC\xA8\xD3\xA6\xDB\xAC\x50\xAA\xC5\xAA\xBA\xAF\xAC\xBA\xD6", 
  /* 白虎 */
  /* 白虎天降，心之所向 */
  -1, 0x0800, 0x00FF, "\xA5\xD5\xAA\xEA",      -9999, 2000, -9999, 0, -999, 0, 200, "\xA5\xD5\xAA\xEA\xA4\xD1\xAD\xB0\xA1\x41\xA4\xDF\xA4\xA7\xA9\xD2\xA6\x56", 	/* 全滿 */
};

struct skillset spellBlist[] =
{
  /* 雷系法術                         hp   mp   vp   sp  tir  eff  pic  message */
  /* 雷系法術 */
  /* 雷系法術 */
  -2, 0x0000,      7, "\xB9\x70\xA8\x74\xAA\x6B\xB3\x4E",      0,   0,   0,   0,   0,   0,   0, "\xB9\x70\xA8\x74\xAA\x6B\xB3\x4E", 
  /* 雷咒 */
  /* 您施展了雷咒 */
  -2, 0x0001, 0x0000, "\xB9\x70\xA9\x47",          0,  30,   0,   0,   2,  20, 210, "\xB1\x7A\xAC\x49\xAE\x69\xA4\x46\xB9\x70\xA9\x47", 
  /* 五雷咒 */
  /* 您施展了五雷咒 */
  -2, 0x0002, 0x0001, "\xA4\xAD\xB9\x70\xA9\x47",        0,  50,   0,   0,   2,  40, 210, "\xB1\x7A\xAC\x49\xAE\x69\xA4\x46\xA4\xAD\xB9\x70\xA9\x47", 
  /* 天雷網 */
  /* 您施展了天雷網 */
  -2, 0x0004, 0x0003, "\xA4\xD1\xB9\x70\xBA\xF4",        0,  70,   0,   0,   2, 100, 210, "\xB1\x7A\xAC\x49\xAE\x69\xA4\x46\xA4\xD1\xB9\x70\xBA\xF4", 
  /* 瘋狂之雷 */
  /* 您施展了瘋狂之雷 */
  -2, 0x0008, 0x0007, "\xBA\xC6\xA8\x67\xA4\xA7\xB9\x70",      0, 100,   0,   0,   3, 150, 210, "\xB1\x7A\xAC\x49\xAE\x69\xA4\x46\xBA\xC6\xA8\x67\xA4\xA7\xB9\x70", 
  /* 雷神之舞 */
  /* 您施展了雷神之舞 */
  -2, 0x0010, 0x000F, "\xB9\x70\xAF\xAB\xA4\xA7\xBB\x52",      0, 150,   0,   0,   3, 200, 210, "\xB1\x7A\xAC\x49\xAE\x69\xA4\x46\xB9\x70\xAF\xAB\xA4\xA7\xBB\x52", 
  /* 爆雷降臨 */
  /* 您施展了爆雷降臨 */
  -2, 0x0020, 0x001F, "\xC3\x7A\xB9\x70\xAD\xB0\xC1\x7B",      0, 250,   0,   0,   3, 300, 210, "\xB1\x7A\xAC\x49\xAE\x69\xA4\x46\xC3\x7A\xB9\x70\xAD\xB0\xC1\x7B", 
  /* 神龍 */
  /* 神龍擺尾，雷電相隨 */
  -2, 0x0040, 0x003F, "\xAF\xAB\xC0\x73",          0, 400,   0,   0,   4, 600, 210, "\xAF\xAB\xC0\x73\xC2\x5C\xA7\xC0\xA1\x41\xB9\x70\xB9\x71\xAC\xDB\xC0\x48", 
};

struct skillset spellClist[] =
{
  /* 冰系法術                         hp   mp   vp   sp  tir  eff  pic  message */
  /* 冰系法術 */
  /* 冰系法術 */
  -3, 0x0000,      7, "\xA6\x42\xA8\x74\xAA\x6B\xB3\x4E",      0,   0,   0,   0,   0,   0,   0, "\xA6\x42\xA8\x74\xAA\x6B\xB3\x4E", 
  /* 冰咒 */
  /* 您施展了冰咒 */
  -3, 0x0001, 0x0000, "\xA6\x42\xA9\x47",          0,  30,   0,   0,   2,  20, 220, "\xB1\x7A\xAC\x49\xAE\x69\xA4\x46\xA6\x42\xA9\x47", 
  /* 寒冰咒 */
  /* 您施展了寒冰咒 */
  -3, 0x0002, 0x0001, "\xB4\x48\xA6\x42\xA9\x47",        0,  50,   0,   0,   2,  40, 220, "\xB1\x7A\xAC\x49\xAE\x69\xA4\x46\xB4\x48\xA6\x42\xA9\x47", 
  /* 玄冰咒 */
  /* 您施展了玄冰咒 */
  -3, 0x0004, 0x0003, "\xA5\xC8\xA6\x42\xA9\x47",        0,  70,   0,   0,   2, 100, 220, "\xB1\x7A\xAC\x49\xAE\x69\xA4\x46\xA5\xC8\xA6\x42\xA9\x47", 
  /* 冰風暴 */
  /* 您施展了冰風暴 */
  -3, 0x0008, 0x0007, "\xA6\x42\xAD\xB7\xBC\xC9",        0, 100,   0,   0,   3, 150, 220, "\xB1\x7A\xAC\x49\xAE\x69\xA4\x46\xA6\x42\xAD\xB7\xBC\xC9", 
  /* 風雷冰天 */
  /* 您施展了風雷冰天 */
  -3, 0x0010, 0x000F, "\xAD\xB7\xB9\x70\xA6\x42\xA4\xD1",      0, 150,   0,   0,   3, 200, 220, "\xB1\x7A\xAC\x49\xAE\x69\xA4\x46\xAD\xB7\xB9\x70\xA6\x42\xA4\xD1", 
  /* 絕對零度 */
  /* 您施展了絕對零度 */
  -3, 0x0020, 0x001F, "\xB5\xB4\xB9\xEF\xB9\x73\xAB\xD7",      0, 250,   0,   0,   3, 300, 220, "\xB1\x7A\xAC\x49\xAE\x69\xA4\x46\xB5\xB4\xB9\xEF\xB9\x73\xAB\xD7", 
  /* 冰神 */
  /* 冰神之舞，風雨無阻 */
  -3, 0x0040, 0x003F, "\xA6\x42\xAF\xAB",          0, 400,   0,   0,   4, 600, 220, "\xA6\x42\xAF\xAB\xA4\xA7\xBB\x52\xA1\x41\xAD\xB7\xAB\x42\xB5\x4C\xAA\xFD", 
};

struct skillset spellDlist[] =
{
  /* 炎系法術                         hp   mp   vp   sp  tir  eff  pic  message */
  /* 炎系法術 */
  /* 炎系法術 */
  -4, 0x0000,      7, "\xAA\xA2\xA8\x74\xAA\x6B\xB3\x4E",      0,   0,   0,   0,   0,   0,   0, "\xAA\xA2\xA8\x74\xAA\x6B\xB3\x4E", 
  /* 炎咒 */
  /* 您施展了炎咒 */
  -4, 0x0001, 0x0000, "\xAA\xA2\xA9\x47",          0,  30,   0,   0,   2,  20, 230, "\xB1\x7A\xAC\x49\xAE\x69\xA4\x46\xAA\xA2\xA9\x47", 
  /* 炎殺咒 */
  /* 您施展了炎殺咒 */
  -4, 0x0002, 0x0001, "\xAA\xA2\xB1\xFE\xA9\x47",        0,  50,   0,   0,   2,  40, 230, "\xB1\x7A\xAC\x49\xAE\x69\xA4\x46\xAA\xA2\xB1\xFE\xA9\x47", 
  /* 煉獄真火 */
  /* 您施展了煉獄真火 */
  -4, 0x0004, 0x0003, "\xB7\xD2\xBA\xBB\xAF\x75\xA4\xF5",      0,  70,   0,   0,   2, 100, 230, "\xB1\x7A\xAC\x49\xAE\x69\xA4\x46\xB7\xD2\xBA\xBB\xAF\x75\xA4\xF5", 
  /* 地獄業火 */
  /* 您施展了地獄業火 */
  -4, 0x0008, 0x0007, "\xA6\x61\xBA\xBB\xB7\x7E\xA4\xF5",      0, 100,   0,   0,   3, 150, 230, "\xB1\x7A\xAC\x49\xAE\x69\xA4\x46\xA6\x61\xBA\xBB\xB7\x7E\xA4\xF5", 
  /* 炎魔地獄 */
  /* 您施展了炎魔地獄 */
  -4, 0x0010, 0x000F, "\xAA\xA2\xC5\x5D\xA6\x61\xBA\xBB",      0, 150,   0,   0,   3, 200, 230, "\xB1\x7A\xAC\x49\xAE\x69\xA4\x46\xAA\xA2\xC5\x5D\xA6\x61\xBA\xBB", 
  /* 火龍招喚 */
  /* 您施展了火龍招喚 */
  -4, 0x0020, 0x001F, "\xA4\xF5\xC0\x73\xA9\xDB\xB3\xEA",      0, 250,   0,   0,   3, 300, 230, "\xB1\x7A\xAC\x49\xAE\x69\xA4\x46\xA4\xF5\xC0\x73\xA9\xDB\xB3\xEA", 
  /* 朱雀 */
  /* 朱雀展翅，來者必死 */
  -4, 0x0040, 0x003F, "\xA6\xB6\xB3\xB6",          0, 400,   0,   0,   4, 600, 230, "\xA6\xB6\xB3\xB6\xAE\x69\xAF\xCD\xA1\x41\xA8\xD3\xAA\xCC\xA5\xB2\xA6\xBA",
};

struct skillset spellElist[] =
{
  /* 土系法術                         hp   mp   vp   sp  tir  eff  pic  message */
  /* 土系法術 */
  /* 土系法術 */
  -5, 0x0000,      7, "\xA4\x67\xA8\x74\xAA\x6B\xB3\x4E",      0,   0,   0,   0,   0,   0,   0, "\xA4\x67\xA8\x74\xAA\x6B\xB3\x4E", 
  /* 土咒 */
  /* 您施展了土咒 */
  -5, 0x0001, 0x0000, "\xA4\x67\xA9\x47",          0,  30,   0,   0,   2,  20, 240, "\xB1\x7A\xAC\x49\xAE\x69\xA4\x46\xA4\x67\xA9\x47", 
  /* 飛岩術 */
  /* 您施展了飛岩術 */
  -5, 0x0002, 0x0001, "\xAD\xB8\xA9\xA5\xB3\x4E",        0,  50,   0,   0,   2,  40, 240, "\xB1\x7A\xAC\x49\xAE\x69\xA4\x46\xAD\xB8\xA9\xA5\xB3\x4E", 
  /* 地裂天崩 */
  /* 您施展了地裂天崩 */
  -5, 0x0004, 0x0003, "\xA6\x61\xB5\xF5\xA4\xD1\xB1\x59",      0,  70,   0,   0,   2, 100, 240, "\xB1\x7A\xAC\x49\xAE\x69\xA4\x46\xA6\x61\xB5\xF5\xA4\xD1\xB1\x59", 
  /* 泰山壓頂 */
  /* 您施展了泰山壓頂 */
  -5, 0x0008, 0x0007, "\xAE\xF5\xA4\x73\xC0\xA3\xB3\xBB",      0, 100,   0,   0,   3, 150, 240, "\xB1\x7A\xAC\x49\xAE\x69\xA4\x46\xAE\xF5\xA4\x73\xC0\xA3\xB3\xBB", 
  /* 土龍召喚 */
  /* 您施展了土龍召喚 */
  -5, 0x0010, 0x000F, "\xA4\x67\xC0\x73\xA5\x6C\xB3\xEA",      0, 150,   0,   0,   3, 200, 240, "\xB1\x7A\xAC\x49\xAE\x69\xA4\x46\xA4\x67\xC0\x73\xA5\x6C\xB3\xEA", 
  /* 土地神明 */
  /* 您施展了土地神明 */
  -5, 0x0020, 0x001F, "\xA4\x67\xA6\x61\xAF\xAB\xA9\xFA",      0, 250,   0,   0,   3, 300, 240, "\xB1\x7A\xAC\x49\xAE\x69\xA4\x46\xA4\x67\xA6\x61\xAF\xAB\xA9\xFA", 
  /* 玄武 */
  /* 玄武再現，攻擊無限 */
  -5, 0x0040, 0x003F, "\xA5\xC8\xAA\x5A",          0, 400,   0,   0,   4, 600, 240, "\xA5\xC8\xAA\x5A\xA6\x41\xB2\x7B\xA1\x41\xA7\xF0\xC0\xBB\xB5\x4C\xAD\xAD", 
};

struct skillset spellFlist[] =
{
  /* 風系法術                         hp   mp   vp   sp  tir  eff  pic  message */
  /* 風系法術 */
  /* 風系法術 */
  -6, 0x0000,      7, "\xAD\xB7\xA8\x74\xAA\x6B\xB3\x4E",      0,   0,   0,   0,   0,   0,   0, "\xAD\xB7\xA8\x74\xAA\x6B\xB3\x4E", 
  /* 風咒 */
  /* 您施展了風咒 */
  -6, 0x0001, 0x0000, "\xAD\xB7\xA9\x47",          0,  30,   0,   0,   2,  20, 250, "\xB1\x7A\xAC\x49\xAE\x69\xA4\x46\xAD\xB7\xA9\x47", 
  /* 旋風咒 */
  /* 您施展了旋風咒 */
  -6, 0x0002, 0x0001, "\xB1\xDB\xAD\xB7\xA9\x47",        0,  50,   0,   0,   2,  40, 250, "\xB1\x7A\xAC\x49\xAE\x69\xA4\x46\xB1\xDB\xAD\xB7\xA9\x47", 
  /* 狂風術 */
  /* 您施展了狂風術 */
  -6, 0x0004, 0x0003, "\xA8\x67\xAD\xB7\xB3\x4E",        0,  70,   0,   0,   2, 100, 250, "\xB1\x7A\xAC\x49\xAE\x69\xA4\x46\xA8\x67\xAD\xB7\xB3\x4E", 
  /* 龍捲風 */
  /* 您施展了龍捲風 */
  -6, 0x0008, 0x0007, "\xC0\x73\xB1\xB2\xAD\xB7",        0, 100,   0,   0,   3, 150, 250, "\xB1\x7A\xAC\x49\xAE\x69\xA4\x46\xC0\x73\xB1\xB2\xAD\xB7", 
  /* 風捲殘雲 */
  /* 您施展了風捲殘雲 */
  -6, 0x0010, 0x000F, "\xAD\xB7\xB1\xB2\xB4\xDD\xB6\xB3",      0, 150,   0,   0,   3, 200, 250, "\xB1\x7A\xAC\x49\xAE\x69\xA4\x46\xAD\xB7\xB1\xB2\xB4\xDD\xB6\xB3", 
  /* 風花雪月 */
  /* 您施展了風花雪月 */
  -6, 0x0020, 0x001F, "\xAD\xB7\xAA\xE1\xB3\xB7\xA4\xEB",      0, 250,   0,   0,   3, 300, 250, "\xB1\x7A\xAC\x49\xAE\x69\xA4\x46\xAD\xB7\xAA\xE1\xB3\xB7\xA4\xEB", 
  /* 青龍 */
  /* 青龍當中，神風特攻 */
  -6, 0x0040, 0x003F, "\xAB\x43\xC0\x73",          0, 400,   0,   0,   4, 600, 250, "\xAB\x43\xC0\x73\xB7\xED\xA4\xA4\xA1\x41\xAF\xAB\xAD\xB7\xAF\x53\xA7\xF0", 
};

struct skillset spellGlist[] =
{
  /* 究極法術                         hp   mp   vp   sp  tir  eff  pic  message */
  /* 究極法術 */
  /* 究極法術 */
  -7, 0x0000,      6, "\xA8\x73\xB7\xA5\xAA\x6B\xB3\x4E",      0,   0,   0,   0,   0,   0,   0, "\xA8\x73\xB7\xA5\xAA\x6B\xB3\x4E", 			/* 惡搞系的魔法 :p */
  /* 捐血中心 */
  /* 捐血一命，救人一袋 */
  -7, 0x0001, 0x0000, "\xAE\xBD\xA6\xE5\xA4\xA4\xA4\xDF",      0,2000, 9999, 9999, 0,   0, 260, "\xAE\xBD\xA6\xE5\xA4\x40\xA9\x52\xA1\x41\xB1\xCF\xA4\x48\xA4\x40\xB3\x55", 		/* 治療 */
  /* 手機 */
  /* T28 加中華電信，超強電磁波 */
  -7, 0x0002, 0x0000, "\xA4\xE2\xBE\xF7",          0, 800,   0,   0,   0, 1200,260, "T28 \xA5\x5B\xA4\xA4\xB5\xD8\xB9\x71\xAB\x48\xA1\x41\xB6\x57\xB1\x6A\xB9\x71\xBA\xCF\xAA\x69", 	/* 雷 */
  /* 水冷式風扇 */
  /* 超頻絕對沒問題 */
  -7, 0x0004, 0x0000, "\xA4\xF4\xA7\x4E\xA6\xA1\xAD\xB7\xAE\xB0",    0, 800,   0,   0,   0, 1200,260, "\xB6\x57\xC0\x57\xB5\xB4\xB9\xEF\xA8\x53\xB0\xDD\xC3\x44",		/* 冰 */
  /* 核彈融合 */
  /* 核子彈爆炸了 */
  -7, 0x0008, 0x0000, "\xAE\xD6\xBC\x75\xBF\xC4\xA6\x58",      0, 800,   0,   0,   0, 1200,260, "\xAE\xD6\xA4\x6C\xBC\x75\xC3\x7A\xAC\xB5\xA4\x46", 		/* 炎 */
  /* 土石流 */
  /* 台灣名產－土石流 */
  -7, 0x0010, 0x0000, "\xA4\x67\xA5\xDB\xAC\x79",        0, 800,   0,   0,   0, 1200,260, "\xA5\x78\xC6\x57\xA6\x57\xB2\xA3\xA1\xD0\xA4\x67\xA5\xDB\xAC\x79", 		/* 土 */
  /* 新竹風 */
  /* 造就了新竹米粉 */
  -7, 0x0020, 0x0000, "\xB7\x73\xA6\xCB\xAD\xB7",        0, 800,   0,   0,   0, 1200,260, "\xB3\x79\xB4\x4E\xA4\x46\xB7\x73\xA6\xCB\xA6\xCC\xAF\xBB", 		/* 風 */
};


  /*-----------------------------------------------------*/
  /* 技能學習函式                			 */
  /*-----------------------------------------------------*/


/* itoc.010801: 學到新技能 */
int				/* 0:沒有學到 1:學到 */
pip_learn_skill(smode)
  int smode;			/* skill mode  0:雜項  >0:武功  <0:魔法 */
{
  int num;
  char buf[80];

  /* itoc.010801: 先亂數決定該系技能的其中一項，然後檢查小雞是否已經會這技能了 */
  /* 如果不會而且已經學到此技能之基本技能，那麼將獲得此一新技能 */

  /* itoc.020129.設計技巧: 若 skill?list[].sno = skill?list[].sbasic，那麼使用者就無法從
     pip_learn_skill() 學到此技能，所以可以由特殊事件來學習 */

  switch (smode)
  {
  case 0:		/* 特殊 */
    num = rand() % skillXYZlist[0].sbasic + 1;
    if ((d.skillXYZ & skillXYZlist[num].sno) || ((d.skillXYZ & skillXYZlist[num].sbasic) != skillXYZlist[num].sbasic))
      return 0;
    d.skillXYZ |= skillXYZlist[num].sno;
    strcpy(buf, skillXYZlist[num].msg);
    break;  

  case 1:		/* 護身 */
    num = rand() % skillAlist[0].sbasic + 1;
    if ((d.skillA & skillAlist[num].sno) || ((d.skillA & skillAlist[num].sbasic) != skillAlist[num].sbasic))
      return 0;
    d.skillA |= skillAlist[num].sno;
    /* 您領悟了護身－%s */
    sprintf(buf, "\xB1\x7A\xBB\xE2\xAE\xA9\xA4\x46\xC5\x40\xA8\xAD\xA1\xD0%s", skillAlist[num].name);
    break;

  case 2:		/* 輕功 */
    num = rand() % skillBlist[0].sbasic + 1;
    if ((d.skillB & skillBlist[num].sno) || ((d.skillB & skillBlist[num].sbasic) != skillBlist[num].sbasic))
      return 0;
    d.skillB |= skillBlist[num].sno;
    /* 您領悟了輕功－%s */
    sprintf(buf, "\xB1\x7A\xBB\xE2\xAE\xA9\xA4\x46\xBB\xB4\xA5\x5C\xA1\xD0%s", skillBlist[num].name);
    break;  
    
  case 3:		/* 心法 */
    num = rand() % skillClist[0].sbasic + 1;
    if ((d.skillC & skillClist[num].sno) || ((d.skillC & skillClist[num].sbasic) != skillClist[num].sbasic))
      return 0;
    d.skillC |= skillClist[num].sno;
    /* 您領悟了心法－%s */
    sprintf(buf, "\xB1\x7A\xBB\xE2\xAE\xA9\xA4\x46\xA4\xDF\xAA\x6B\xA1\xD0%s", skillClist[num].name);
    break;

  case 4:		/* 拳法 */
    num = rand() % skillDlist[0].sbasic + 1;
    if ((d.skillD & skillDlist[num].sno) || ((d.skillD & skillDlist[num].sbasic) != skillDlist[num].sbasic))
      return 0;
    d.skillD |= skillDlist[num].sno;
    /* 您領悟了拳法－%s */
    sprintf(buf, "\xB1\x7A\xBB\xE2\xAE\xA9\xA4\x46\xAE\xB1\xAA\x6B\xA1\xD0%s", skillDlist[num].name);
    break;  
    
  case 5:		/* 劍法 */
    num = rand() % skillElist[0].sbasic + 1;
    if ((d.skillE & skillElist[num].sno) || ((d.skillE & skillElist[num].sbasic) != skillElist[num].sbasic))
      return 0;
    d.skillE |= skillElist[num].sno;
    /* 您領悟了劍法－%s */
    sprintf(buf, "\xB1\x7A\xBB\xE2\xAE\xA9\xA4\x46\xBC\x43\xAA\x6B\xA1\xD0%s", skillElist[num].name);
    break;

  case 6:		/* 刀法 */
    num = rand() % skillFlist[0].sbasic + 1;
    if ((d.skillF & skillFlist[num].sno) || ((d.skillF & skillFlist[num].sbasic) != skillFlist[num].sbasic))
      return 0;
    d.skillF |= skillFlist[num].sno;
    /* 您領悟了刀法－%s */
    sprintf(buf, "\xB1\x7A\xBB\xE2\xAE\xA9\xA4\x46\xA4\x4D\xAA\x6B\xA1\xD0%s", skillFlist[num].name);
    break;

  case 7:		/* 暗器 */
    num = rand() % skillGlist[0].sbasic + 1;
    if ((d.skillG & skillGlist[num].sno) || ((d.skillG & skillGlist[num].sbasic) != skillGlist[num].sbasic))
      return 0;
    d.skillG |= skillGlist[num].sno;
    /* 您領悟暗器－%s */
    sprintf(buf, "\xB1\x7A\xBB\xE2\xAE\xA9\xB7\x74\xBE\xB9\xA1\xD0%s", skillGlist[num].name);
    break;


  case -1:		/* 治療法術 */
    num = rand() % spellAlist[0].sbasic + 1;
    if ((d.spellA & spellAlist[num].sno) || ((d.spellA & spellAlist[num].sbasic) != spellAlist[num].sbasic))
      return 0;
    d.spellA |= spellAlist[num].sno;
    /* 您學會了白魔法－%s */
    sprintf(buf, "\xB1\x7A\xBE\xC7\xB7\x7C\xA4\x46\xA5\xD5\xC5\x5D\xAA\x6B\xA1\xD0%s", spellAlist[num].name);
    break;

  case -2:		/* 雷系法術 */
    num = rand() % spellBlist[0].sbasic + 1;
    if ((d.spellB & spellBlist[num].sno) || ((d.spellB & spellBlist[num].sbasic) != spellBlist[num].sbasic))
      return 0;
    d.spellB |= spellBlist[num].sno;
    /* 您學會了雷魔法－%s */
    sprintf(buf, "\xB1\x7A\xBE\xC7\xB7\x7C\xA4\x46\xB9\x70\xC5\x5D\xAA\x6B\xA1\xD0%s", spellBlist[num].name);
    break;
    
  case -3:		/* 冰系法術 */
    num = rand() % spellClist[0].sbasic + 1;
    if ((d.spellC & spellClist[num].sno) || ((d.spellC & spellClist[num].sbasic) != spellClist[num].sbasic))
      return 0;
    d.spellC |= spellClist[num].sno;
    /* 您學會了冰魔法－%s */
    sprintf(buf, "\xB1\x7A\xBE\xC7\xB7\x7C\xA4\x46\xA6\x42\xC5\x5D\xAA\x6B\xA1\xD0%s", spellClist[num].name);
    break;


  case -4:		/* 炎系法術 */
    num = rand() % spellDlist[0].sbasic + 1;
    if ((d.spellD & spellDlist[num].sno) || ((d.spellD & spellDlist[num].sbasic) != spellDlist[num].sbasic))
      return 0;
    d.spellD |= spellDlist[num].sno;
    /* 您學會了火魔法－%s */
    sprintf(buf, "\xB1\x7A\xBE\xC7\xB7\x7C\xA4\x46\xA4\xF5\xC5\x5D\xAA\x6B\xA1\xD0%s", spellDlist[num].name);
    break;
    
  case -5:		/* 土系法術 */
    num = rand() % spellElist[0].sbasic + 1;
    if ((d.spellE & spellElist[num].sno) || ((d.spellE & spellElist[num].sbasic) != spellElist[num].sbasic))
      return 0;
    d.spellE |= spellElist[num].sno;
    /* 您學會了土魔法－%s */
    sprintf(buf, "\xB1\x7A\xBE\xC7\xB7\x7C\xA4\x46\xA4\x67\xC5\x5D\xAA\x6B\xA1\xD0%s", spellElist[num].name);
    break;

  case -6:		/* 風系法術 */
    num = rand() % spellFlist[0].sbasic + 1;
    if ((d.spellF & spellFlist[num].sno) || ((d.spellF & spellFlist[num].sbasic) != spellFlist[num].sbasic))
      return 0;
    d.spellF |= spellFlist[num].sno;
    /* 您學會了風魔法－%s */
    sprintf(buf, "\xB1\x7A\xBE\xC7\xB7\x7C\xA4\x46\xAD\xB7\xC5\x5D\xAA\x6B\xA1\xD0%s", spellFlist[num].name);
    break;

  case -7:		/* 究極法術 */
    num = rand() % spellGlist[0].sbasic + 1;
    if ((d.spellG & spellGlist[num].sno) || ((d.spellG & spellGlist[num].sbasic) != spellGlist[num].sbasic))
      return 0;
    d.spellG |= spellGlist[num].sno;
    /* 您瞭解了黑魔法－%s */
    sprintf(buf, "\xB1\x7A\xC1\x41\xB8\xD1\xA4\x46\xB6\xC2\xC5\x5D\xAA\x6B\xA1\xD0%s", spellGlist[num].name);
    break;
  }

  vmsg(buf);
  return 1;
}


  /*-----------------------------------------------------*/
  /* 戰鬥技能選單                			 */
  /*-----------------------------------------------------*/


/* 有學會該技能，且血、法力夠才能施展 */
#define can_useskill(n)	((skill & p[n].sno) && (p[n].needhp < d.hp) && (p[n].needmp <= d.mp) && (p[n].needvp <= d.vp) && (p[n].needsp <= d.sp))


/* itoc.010729: 技能視窗 */
static void
pip_skill_doing_menu(p, dr, sr, mr, hr)		/* 技能畫面 */
  struct skillset *p;
  int dr;		/* 傷害力 damage rate */
  int sr;		/* 內力強度 spirit rate */
  int mr;		/* 魔法強度 magic rate */
  int hr;		/* 命中率 hit rate */
{
  int n, ch;
  char ans[5];
  usint skill;

  switch (p[0].smode)
  {
  case 1:		/* 護身 */
    skill = d.skillA;
    break;

  case 2:		/* 輕功 */
    skill = d.skillB;
    break;  
    
  case 3:		/* 心法 */
    skill = d.skillC;
    break;

  case 4:		/* 拳功 */
    skill = d.skillD;
    break;

  case 5:		/* 劍法 */
    skill = d.skillE;
    break;  
    
  case 6:		/* 刀法 */
    skill = d.skillF;
    break;

  case 7:		/* 暗器 */
    skill = d.skillG;
    break;

  case -1:		/* 治療法術 */
    skill = d.spellA;
    break;

  case -2:		/* 雷系法術 */
    skill = d.spellB;
    break;
    
  case -3:		/* 冰系法術 */
    skill = d.spellC;
    break;

  case -4:		/* 炎系法術 */
    skill = d.spellD;
    break;

  case -5:		/* 土系法術 */
    skill = d.spellE;
    break;
    
  case -6:		/* 風系法術 */
    skill = d.spellF;
    break;

  case -7:		/* 究極法術 */
    skill = d.spellG;
    break;

  default:		/* 以免意外發生 */
    skill = 0;
  }

  clrfromto(7, 16);
  /* \033[1;31m─────────────┤\033[37;41m   可用[%s]一覽表  \033[0;1;31m├─────────────\033[m */
  prints("\033[1;31m\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x74\033[37;41m   \xA5\x69\xA5\xCE[%s]\xA4\x40\xC4\xFD\xAA\xED  \033[0;1;31m\xA2\x75\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\033[m", p[0].name);

  n = 1;
  while (n <= p[0].sbasic)	/* p[0].sbasic 儲存有這系有幾個技能 */
  {
    if (can_useskill(n))
    {
      /* 分四欄，每欄八個，各系技能最多 32 個 */
      if (n <= 8)
	move(n + 7, 4);
      else if (n <= 16)
	move(n - 1, 20);
      else if (n <= 24)
	move(n - 9, 36);
      else
	move(n - 17, 52);

      prints("%2d.%s", n, p[n].name);
    }

    n++;
  }

  while (1)
  {
    /*     您想使用那一招呢？[Q]放棄： */
    if (vget(16, 0, "    \xB1\x7A\xB7\x51\xA8\xCF\xA5\xCE\xA8\xBA\xA4\x40\xA9\xDB\xA9\x4F\xA1\x48[Q]\xA9\xF1\xB1\xF3\xA1\x47", ans, 3, DOECHO))
    {
      if (ans[0] == 'q')
      {
	show_badman_pic(m.pic);
	return;
      }
      else
      {
	ch = atoi(ans);
	if (ch > 0 && ch < n && can_useskill(ch))
	{
	  break;
	}
      }
    }
  }

  d.hp -= p[ch].needhp;
  d.mp -= p[ch].needmp;
  d.vp -= p[ch].needvp;
  d.sp -= p[ch].needsp;
  d.tired += p[ch].addtired;

  /* itoc.010801: 只需要檢查是否爆上限，不必檢查是否 < 0，因為前面檢查過了 */
  if (d.hp > d.maxhp)
    d.hp = d.maxhp;
  if (d.mp > d.maxmp)
    d.mp = d.maxmp;
  if (d.vp > d.maxvp)
    d.vp = d.maxvp;
  if (d.sp > d.maxsp)
    d.sp = d.maxsp;
  if (d.tired < 0)
    d.tired = 0;

  /* itoc.010801: 比較 */
  /* 在怪物沒有防禦下，普通攻擊的期望傷害為 d_dr */
  /* 在怪物沒有防禦下，全力一擊的期望傷害為 120% * d_dr */
  /* 在怪物沒有防禦下，技能攻擊的期望傷害為 p[ch].effect + d_dr (或 d_mr d_sr d_hr)*/

  switch (p[ch].smode)
  {
    /* itoc.010729: random 變化越大的，就容易 miss */

  case 1:		/* 護身: 用了以後加強防禦 */
    /* 加強防禦是 d_resistmore + 40%，所以 p[ch].effect 應至少 > 40 */
    d_resistmore = p[ch].effect * (125 - rand() % 50) / 100;	/* 原效果的 75% ~ 125% */
    if (d_resistmore >= 100)
      d_resistmore = 99;	/* 若 d_resistmore > 100，反而變加血了! */
    break;

  case 2:		/* 輕功 */
    /* vp在前面已經加過 */
    break;

  case 3:		/* 心法 */
    /* sp在前面已經加過 */
    break;

  case 4:		/* 拳法 */
    m.hp -= p[ch].effect * (120 - rand() % 40) / 100 + (dr + sr) / 2;
    break;

  case 5:		/* 劍法 */
    m.hp -= p[ch].effect * (140 - rand() % 80) / 100 + (dr + hr) / 2;
    break;

  case 6:		/* 刀法 */
    m.hp -= p[ch].effect * (160 - rand() % 120) / 100 + (sr + hr) / 2;
    break;

  case 7:		/* 暗器 */
    /* 暗器與能力大致無關，適合低能力者使用 */
    if (hr > d.level * 5)
      m.hp -= p[ch].effect * (120 - rand() % 40) / 100;		/* 100% effect */
    else if (hr > 0)
      m.hp -= p[ch].effect * (80 - rand() % 40) / 100;		/* 60% effect */
    else
      m.hp -= p[ch].effect * (50 - rand() % 40) / 100;		/* 30% effect */
    break;


  case -1:		/* 治療法術 */
    /* hp在前面已經加過 */
    break;

  case -2:		/* 雷系法術 */
    m.hp -= p[ch].effect * (140 - rand() % 80) / 100 + mr;
    break;
    
  case -3:		/* 冰系法術 */
    m.hp -= p[ch].effect * (120 - rand() % 40) / 100 + mr;
    break;

  case -4:		/* 炎系法術 */
    m.hp -= p[ch].effect * (200 - rand() % 200) / 100 + mr;	/* 原效果的 1% ~ 200%，變化超大! */
    break;

  case -5:		/* 土系法術 */
    m.hp -= p[ch].effect * (110 - rand() % 20) / 100 + mr;
    break;

  case -6:		/* 風系法術 */
    m.hp -= p[ch].effect * (130 - rand() % 60) / 100 + mr;
    break;

  case -7:		/* 究極法術 */
    /* itoc.010801: 和一般魔法不同的是，究極法術不單看 mr 也看 dr sr hr */
    m.hp -= p[ch].effect * (140 - rand() % 80) / 100 + (mr * 2 + dr + sr + hr) / 5;
    break;
  }

  show_fight_pic(p[ch].pic);
  vmsg(p[ch].msg);
  d_nodone = 0;		/* 行動結束 */
}


/* itoc.010729: 進入使用技能選單 */
static void
pip_skill_menu(dr, sr, mr, hr)	/* 戰鬥中技能的應用 */
  int dr;		/* 傷害力 damage rate */
  int sr;		/* 內力強度 spirit rate */
  int mr;		/* 魔法強度 magic rate */
  int hr;		/* 命中率 hit rate */
{
  while (d_nodone)
  {   
    /*  武功選單  */
    /*  [1]護身 [2]輕功 [3]心法 [4]拳法 [5]劍法 [6]刀法 [7]暗器 [Q]放棄    \033[m */
    out_cmd(COLOR1 " \xAA\x5A\xA5\x5C\xBF\xEF\xB3\xE6 " COLOR2 " [1]\xC5\x40\xA8\xAD [2]\xBB\xB4\xA5\x5C [3]\xA4\xDF\xAA\x6B [4]\xAE\xB1\xAA\x6B [5]\xBC\x43\xAA\x6B [6]\xA4\x4D\xAA\x6B [7]\xB7\x74\xBE\xB9 [Q]\xA9\xF1\xB1\xF3    \033[m", 
      /*  法術選單  */
      /*  [A]治療 [B]雷系 [C]冰系 [D]炎系 [E]土系 [F]風系 [G]特殊 [Q]放棄    \033[m */
      COLOR1 " \xAA\x6B\xB3\x4E\xBF\xEF\xB3\xE6 " COLOR2 " [A]\xAA\x76\xC0\xF8 [B]\xB9\x70\xA8\x74 [C]\xA6\x42\xA8\x74 [D]\xAA\xA2\xA8\x74 [E]\xA4\x67\xA8\x74 [F]\xAD\xB7\xA8\x74 [G]\xAF\x53\xAE\xED [Q]\xA9\xF1\xB1\xF3    \033[m");

    switch (vkey())
    {
    case 'q':
      return;

    case '1':
      pip_skill_doing_menu(skillAlist, dr, sr, mr, hr);
      break;

    case '2':
      pip_skill_doing_menu(skillBlist, dr, sr, mr, hr);
      break;

    case '3':
      pip_skill_doing_menu(skillClist, dr, sr, mr, hr);
      break;

    case '4':
      pip_skill_doing_menu(skillDlist, dr, sr, mr, hr);
      break;

    case '5':
      pip_skill_doing_menu(skillElist, dr, sr, mr, hr);
      break;

    case '6':
      pip_skill_doing_menu(skillFlist, dr, sr, mr, hr);
      break;

    case '7':
      pip_skill_doing_menu(skillGlist, dr, sr, mr, hr);
      break;

    case 'a':
      pip_skill_doing_menu(spellAlist, dr, sr, mr, hr);
      break;

    case 'b':
      pip_skill_doing_menu(spellBlist, dr, sr, mr, hr);
      break;

    case 'c':
      pip_skill_doing_menu(spellClist, dr, sr, mr, hr);
      break;

    case 'd':
      pip_skill_doing_menu(spellDlist, dr, sr, mr, hr);
      break;

    case 'e':
      pip_skill_doing_menu(spellElist, dr, sr, mr, hr);
      break;

    case 'f':
      pip_skill_doing_menu(spellFlist, dr, sr, mr, hr);
      break;

    case 'g':
      pip_skill_doing_menu(spellGlist, dr, sr, mr, hr);
      break;
    }
  }
}


  /*-----------------------------------------------------*/
  /* 戰鬥怪物技能攻擊                                    */
  /*-----------------------------------------------------*/


/* itoc.010801: 由於怪物技能攻擊，不需要選單，所以要和小雞技能攻擊分開寫 */

static void
pip_attack_skill(dr, sr, mr, hr)	/* 怪物技能攻擊 */
  int dr;		/* 傷害力 damage rate */
  int sr;		/* 內力強度 spirit rate */
  int mr;		/* 魔法強度 magic rate */
  int hr;		/* 命中率 hit rate */
{
  int num, mankey;
  char buf[80];

  num = rand();	/* itoc.010801: 因為同餘的數不同，所以用同一個亂數來省資源 */

  /* itoc.010801: 怪物的技能不採用扣 mp/vp/sp 這種制度，而是由機率來施展 */

  if (m.attribute && (num % 2))
  {
    mankey = m.attribute;	/* 50% 怪物使用自己所擅長的技能 */
  }
  else
  {
    mankey = rand() % 14 - 7;	/* 如果沒有指定擅長技能，那麼就亂數產生一系技能 */
    if (!mankey)
      mankey = 7;
  }

  /* itoc.010801: 比較 */
  /* 在小雞沒有防禦下，怪物普通攻擊的期望傷害為 m_dr */
  /* 在小雞沒有防禦下，怪物全力一擊的期望傷害為 120% * m_dr */
  /* 在小雞沒有防禦下，怪物技能攻擊的期望傷害為 150% * m_mr (或 m_dr、m_sr) */

  /* itoc.010814: 怪物的技能大致上和小雞一樣，只是怪物沒有補 vp/sp，所以輕功/心法要換成別的 */

  switch (mankey)
  {
  case 1:		/* 護身 */
    /* 加強防禦是 m_resistmore + 40%，所以輕功 effect 應至少 > 40，且要 < 100 */
    m_resistmore = 40 + num % 60;
    /* 對方加強防禦，全身動了起來 */
    vmsg("\xB9\xEF\xA4\xE8\xA5\x5B\xB1\x6A\xA8\xBE\xBF\x6D\xA1\x41\xA5\xFE\xA8\xAD\xB0\xCA\xA4\x46\xB0\x5F\xA8\xD3");
    break;

  case 2:		/* 輕功 － 閃電襲擊 */
    if (hr * (100 - num % 30) / 10 > d.level)
    {
      /* 閃電奇襲的特色就是和怪物本身自己的能力並沒有關係(如果奇襲成功的話) */
      d.hp -= d.maxhp / 3;
      /* Ｂｌｉｔｚ！對方向您閃電襲擊 */
      vmsg("\xA2\xD0\xA2\xF4\xA2\xF1\xA2\xFC\xA3\x43\xA1\x49\xB9\xEF\xA4\xE8\xA6\x56\xB1\x7A\xB0\x7B\xB9\x71\xC5\xA7\xC0\xBB");
    }
    else
    {
      m.hp -= m.maxhp / 5;
      if (m.hp <= 0)
	m.hp = 1;
      /* 對方企圖發動閃電奇襲，但沒能成功，反而造成他自己的損傷 */
      vmsg("\xB9\xEF\xA4\xE8\xA5\xF8\xB9\xCF\xB5\x6F\xB0\xCA\xB0\x7B\xB9\x71\xA9\x5F\xC5\xA7\xA1\x41\xA6\xFD\xA8\x53\xAF\xE0\xA6\xA8\xA5\x5C\xA1\x41\xA4\xCF\xA6\xD3\xB3\x79\xA6\xA8\xA5\x4C\xA6\xDB\xA4\x76\xAA\xBA\xB7\x6C\xB6\xCB");      
    }
    break;

  case 3:		/* 心法 － 吸精 */
    /* itoc.010801: 怪物血越多，吸得越多 */
    m.hp += m.maxhp / 8;
    d.hp -= m.maxhp / 8;
    d.mp -= m.maxhp / 10;
    d.vp -= m.maxhp / 12;
    d.sp -= m.maxhp / 14;
    if (m.hp > m.maxhp)
      m.hp = m.maxhp;
    if (d.mp < 0)
      d.mp = 0;
    if (d.vp < 0)
      d.vp = 0;
    if (d.sp < 0)
      d.sp = 0;
    /* %s狠狠地咬了您一口，難道這就是傳說中的吸精術？ */
    sprintf(buf, "%s\xAC\xBD\xAC\xBD\xA6\x61\xAB\x72\xA4\x46\xB1\x7A\xA4\x40\xA4\x66\xA1\x41\xC3\xF8\xB9\x44\xB3\x6F\xB4\x4E\xAC\x4F\xB6\xC7\xBB\xA1\xA4\xA4\xAA\xBA\xA7\x6C\xBA\xEB\xB3\x4E\xA1\x48", m.name);    
    vmsg(buf);
    break;

  case 4:		/* 拳法 */
    /* 隔山打牛，對方打得您屁滾尿流 */
    vmsg("\xB9\x6A\xA4\x73\xA5\xB4\xA4\xFB\xA1\x41\xB9\xEF\xA4\xE8\xA5\xB4\xB1\x6F\xB1\x7A\xA7\xBE\xBA\x75\xA7\xBF\xAC\x79");
    d.hp -= (dr + sr) * (125 + num % 50) / 200;
    break;

  case 5:		/* 劍法 */
    if (num % 3 == 0)
    {
      /* itoc.010801: 做一些怪怪的效果 */
      /* 一斷貪嗔 */
      /* 二斷愛欲 */
      /* 三斷煩惱 */
      char name[3][9] = {"\xA4\x40\xC2\x5F\xB3\x67\xDC\xD2", "\xA4\x47\xC2\x5F\xB7\x52\xB1\xFD", "\xA4\x54\xC2\x5F\xB7\xD0\xB4\x6F"};

      for (num = 0; num < 3; num++)
      {
	/* 天遁神劍 %s */
	sprintf(buf, "\xA4\xD1\xB9\x50\xAF\xAB\xBC\x43 %s", name[num]);		/* itoc: 呂洞賓˙天遁神劍 */
	vmsg(buf);
      }
    }
    else
    {
      /* 滿天花雨，您中了對方一劍 */
      vmsg("\xBA\xA1\xA4\xD1\xAA\xE1\xAB\x42\xA1\x41\xB1\x7A\xA4\xA4\xA4\x46\xB9\xEF\xA4\xE8\xA4\x40\xBC\x43");
    }
    d.hp -= (dr + hr) * (125 + num % 50) / 200;
    break;

  case 6:		/* 刀法 */
    /* 您中了對手的天殘刀法！ */
    vmsg("\xB1\x7A\xA4\xA4\xA4\x46\xB9\xEF\xA4\xE2\xAA\xBA\xA4\xD1\xB4\xDD\xA4\x4D\xAA\x6B\xA1\x49");
    d.hp -= (sr + hr) * (125 + num % 50) / 200;
    break;

  case 7:		/* 暗器 */
    /* 疏忽之中，您中了對手的暗器！ */
    vmsg("\xB2\xA8\xA9\xBF\xA4\xA7\xA4\xA4\xA1\x41\xB1\x7A\xA4\xA4\xA4\x46\xB9\xEF\xA4\xE2\xAA\xBA\xB7\x74\xBE\xB9\xA1\x49");
    d.hp -= hr * (85 + num % 30) / 100;		/* 一般技能攻擊是 150% m_hr，但暗器比較差，只有 100% */
    break;

  case -1:		/* 治療法術 */
    m.hp += m.maxhp * (40 + rand() % 30) / 100;	/* 回復 40%~70% 的血 */
    if (m.hp > m.maxhp)
    {
      m.hp = m.maxhp;
      /* 一陣聖光照耀對方，他的狀況完全恢復了 */
      vmsg("\xA4\x40\xB0\x7D\xB8\x74\xA5\xFA\xB7\xD3\xC4\xA3\xB9\xEF\xA4\xE8\xA1\x41\xA5\x4C\xAA\xBA\xAA\xAC\xAA\x70\xA7\xB9\xA5\xFE\xAB\xEC\xB4\x5F\xA4\x46");
    }
    else
    {
      /* 對方使用魔法治療了自己 */
      vmsg("\xB9\xEF\xA4\xE8\xA8\xCF\xA5\xCE\xC5\x5D\xAA\x6B\xAA\x76\xC0\xF8\xA4\x46\xA6\xDB\xA4\x76");
    }
    break;

  case -2:		/* 雷系法術 */
  case -3:		/* 冰系法術 */
  case -4:		/* 炎系法術 */
  case -5:		/* 土系法術 */
  case -6:		/* 風系法術 */
    /* itoc.010814: 所有的攻擊法術合併在一起處理 */
    {
      /* itoc.010814: 每系法術各 2 隻以配合 mankey 來選召喚獸 */
      /* 電氣豬 */
      /* 雷光獸 */
      /* 寒冰鬼 */
      /* 凍水怪 */
      /* 熱火魔 */
      /* 火焰球 */
      /* 怒土虫 */
      /* 石頭怪 */
      /* 暴風妖 */
      /* 迷魂鬼 */
      char name[10][9] = {"\xB9\x71\xAE\xF0\xBD\xDE", "\xB9\x70\xA5\xFA\xC3\x7E", "\xB4\x48\xA6\x42\xB0\xAD", "\xAD\xE1\xA4\xF4\xA9\xC7", "\xBC\xF6\xA4\xF5\xC5\x5D", "\xA4\xF5\xB5\x4B\xB2\x79", "\xAB\xE3\xA4\x67\xA6\xE4", "\xA5\xDB\xC0\x59\xA9\xC7", "\xBC\xC9\xAD\xB7\xA7\xAF", "\xB0\x67\xBB\xEE\xB0\xAD"};

      num = mr * (125 + num % 50) / 100;	/* 借用 num 當傷害點數 */
      d.hp -= num;
      /* 對方招換了%s，您受傷了 %d 點 */
      sprintf(buf, "\xB9\xEF\xA4\xE8\xA9\xDB\xB4\xAB\xA4\x46%s\xA1\x41\xB1\x7A\xA8\xFC\xB6\xCB\xA4\x46 %d \xC2\x49", name[-2 * (mankey + 2) + (num % 2)], num);
      vmsg(buf);
    }
    break;

  case -7:		/* 究極法術 */
    /* 補滿血又攻擊! */
    m.hp = m.maxhp;
    d.hp -= (dr + sr + mr + hr) * (75 + num % 50) / 800;
    /* %s使用了天魔解體，您完全不是對手 */
    sprintf(buf, "%s\xA8\xCF\xA5\xCE\xA4\x46\xA4\xD1\xC5\x5D\xB8\xD1\xC5\xE9\xA1\x41\xB1\x7A\xA7\xB9\xA5\xFE\xA4\xA3\xAC\x4F\xB9\xEF\xA4\xE2", m.name);
    vmsg(buf);
    break;

  default:
    /* itoc.010814: 不該有不再上列範圍中的喔 :p */
    /* 請告訴站長，【%s】的屬性【%d】設定錯誤 */
    sprintf(buf, "\xBD\xD0\xA7\x69\xB6\x44\xAF\xB8\xAA\xF8\xA1\x41\xA1\x69%s\xA1\x6A\xAA\xBA\xC4\xDD\xA9\xCA\xA1\x69%d\xA1\x6A\xB3\x5D\xA9\x77\xBF\xF9\xBB\x7E", m.name, m.attribute);
    vmsg(buf);
    break;
  }
}


  /*-----------------------------------------------------*/
  /* 戰鬥物理攻擊                                        */
  /*-----------------------------------------------------*/


/* itoc.010731: 怪物和小雞的物理攻擊，可以寫成同一支函式 */

static void
pip_attack_normal(who, dr, hr)		/* itoc.010731: 普通攻擊 */
  int who;		/* 我是誰 1: 小雞下攻擊指令  0: 怪物下攻擊指令 */
  int dr;		/* 傷害力 damage rate */
  int hr;		/* 命中率 hit rate */
{
  int injure;
  char buf[80];

  injure = hr * (150 - rand() % 100) / 100;	/* 決定是否命中 */

  if (who)		/* 小雞攻擊 */
  {
    d_resistmore = 0;
    d_nodone = 0;
    d.tired += 1 + rand() % 2;

    if (hr > d.level * 10)
    {
      /* 在對方沒有防禦下，普通攻擊的期望傷害為 dr */
      injure = dr * (110 - rand() % 20) * (100 - m_resistmore) / 10000;

      if (injure > 0)
      {
	m.hp -= injure;
	d.hexp += rand() % 2 + 2;
	d.hskill += rand() % 2 + 1;
	/* 普通攻擊，%s生命力減低 %d */
	sprintf(buf, "\xB4\xB6\xB3\x71\xA7\xF0\xC0\xBB\xA1\x41%s\xA5\xCD\xA9\x52\xA4\x4F\xB4\xEE\xA7\x43 %d", m.name, injure);
      }
      else
      {
	/* 您的攻擊簡直是替%s抓癢 */
	sprintf(buf, "\xB1\x7A\xAA\xBA\xA7\xF0\xC0\xBB\xC2\xB2\xAA\xBD\xAC\x4F\xB4\xC0%s\xA7\xEC\xC4\x6F", m.name);
      }
    }
    else
    {
      /* 竟然沒打中 */
      strcpy(buf, "\xB3\xBA\xB5\x4D\xA8\x53\xA5\xB4\xA4\xA4");
    }
  }
  else			/* 怪物攻擊 */
  {
    m_resistmore = 0;

    if (injure > d.level * 10)
    {
      injure = dr * (110 - rand() % 20) * (100 - d_resistmore) / 10000;

      if (injure > 0)
      {
	d.hp -= injure;
	/* %s普通攻擊，您生命力減低 %d */
	sprintf(buf, "%s\xB4\xB6\xB3\x71\xA7\xF0\xC0\xBB\xA1\x41\xB1\x7A\xA5\xCD\xA9\x52\xA4\x4F\xB4\xEE\xA7\x43 %d", m.name, injure);
      }
      else
      {
	/* 您完全看不起對方的攻擊 */
	strcpy(buf, "\xB1\x7A\xA7\xB9\xA5\xFE\xAC\xDD\xA4\xA3\xB0\x5F\xB9\xEF\xA4\xE8\xAA\xBA\xA7\xF0\xC0\xBB");
      }
    }
    else
    {
      /* 您閃躲過對方的攻擊 */
      strcpy(buf, "\xB1\x7A\xB0\x7B\xB8\xFA\xB9\x4C\xB9\xEF\xA4\xE8\xAA\xBA\xA7\xF0\xC0\xBB");
    }
  }

  vmsg(buf);
}


static void
pip_attack_aggressive(who, dr, hr)	/* itoc.010731: 全力一擊 */
  int who;		/* 我是誰 1: 小雞下攻擊指令  0: 怪物下攻擊指令 */
  int dr;		/* 傷害力 damage rate */
  int hr;		/* 命中率 hit rate */
{
  int injure;
  char buf[80];

  injure = hr * (200 - rand() % 200) / 100;	/* 決定是否命中，全力一擊的亂數影響比較大 */

  if (who)		/* 小雞攻擊 */
  {
    d_resistmore = 0;
    d_nodone = 0;
    d.hp -= 5;			/* 全力一擊要扣血且比較容易累 */
    d.tired += 1 + rand() % 3;

    if (hr > d.level * 10)
    {
      /* 在對方沒有防禦下，全力攻擊的期望傷害為 120% * dr */
      injure = dr * (130 - rand() % 20) * (100 - m_resistmore) / 10000;

      if (injure > 0)
      {
	m.hp -= injure;
	d.hexp += rand() % 3 + 2;
	d.hskill += rand() % 3 + 1;
	/* 您全力一擊，%s生命力減低 %d */
	sprintf(buf, "\xB1\x7A\xA5\xFE\xA4\x4F\xA4\x40\xC0\xBB\xA1\x41%s\xA5\xCD\xA9\x52\xA4\x4F\xB4\xEE\xA7\x43 %d", m.name, injure);
      }
      else
      {
	/* 您的攻擊簡直是替%s抓癢 */
	sprintf(buf, "\xB1\x7A\xAA\xBA\xA7\xF0\xC0\xBB\xC2\xB2\xAA\xBD\xAC\x4F\xB4\xC0%s\xA7\xEC\xC4\x6F", m.name);
      }
    }
    else
    {
      /* 竟然沒打中 */
      strcpy(buf, "\xB3\xBA\xB5\x4D\xA8\x53\xA5\xB4\xA4\xA4");
    }
  }
  else			/* 怪物攻擊 */
  {
    m_resistmore = 0;

    if (injure > d.level * 10)
    {
      injure = dr * (130 - rand() % 20) * (100 - d_resistmore) / 10000;

      if (injure > 0)
      {
	d.hp -= injure;
	/* %s全力一擊，您生命力減低 %d */
	sprintf(buf, "%s\xA5\xFE\xA4\x4F\xA4\x40\xC0\xBB\xA1\x41\xB1\x7A\xA5\xCD\xA9\x52\xA4\x4F\xB4\xEE\xA7\x43 %d", m.name, injure);
      }
      else
      {
	/* 您完全看不起對方的攻擊 */
	strcpy(buf, "\xB1\x7A\xA7\xB9\xA5\xFE\xAC\xDD\xA4\xA3\xB0\x5F\xB9\xEF\xA4\xE8\xAA\xBA\xA7\xF0\xC0\xBB");
      }
    }
    else
    {
      /* 您閃躲過對方的攻擊 */
      strcpy(buf, "\xB1\x7A\xB0\x7B\xB8\xFA\xB9\x4C\xB9\xEF\xA4\xE8\xAA\xBA\xA7\xF0\xC0\xBB");
    }
  }

  vmsg(buf);
}



/*-------------------------------------------------------*/
/* 對戰主函式                                            */
/*-------------------------------------------------------*/


static void
pip_vs_showing()
{
  int color;
  char inbuf1[20], inbuf2[20];

  clear();
  move(0, 0);

  /*  ～\033[32m%s\033[37m%-13s                                            \033[m\n */
  prints("\033[1;41m  " BBSNAME PIPNAME " \xA1\xE3\033[32m%s\033[37m%-13s                                            \033[m\n", 
    /* ♂ */
    /* ♀ */
    /* ？ */
    d.sex == 1 ? "\xA1\xF1" : (d.sex == 2 ? "\xA1\xF0" : "\xA1\x48"), d.name);

  /* itoc.010801: 螢幕上方秀出小雞的資料 */

  if (d.tired >= 80)
    color = 31;
  else if (d.tired >= 60 && d.tired < 80)
    color = 33;
  else
    color = 37;

  sprintf(inbuf1, "%d%s/%d%s", d.hp > 1000 ? d.hp / 1000 : d.hp,
    d.hp > 1000 ? "K" : "", d.maxhp > 1000 ? d.maxhp / 1000 : d.maxhp,
    d.maxhp > 1000 ? "K" : "");
  sprintf(inbuf2, "%d%s/%d%s", d.mp > 1000 ? d.mp / 1000 : d.mp,
    d.mp > 1000 ? "K" : "", d.maxmp > 1000 ? d.maxmp / 1000 : d.maxmp,
    d.maxmp > 1000 ? "K" : "");

  /* \033[1;31m┌──────────────────────────────────────┐\033[m\n */
  outs("\033[1;31m\xA2\x7A\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x7B\033[m\n");
  /* \033[1;31m│\033[33m生  命:\033[37m%-12s\033[33m法  力:\033[37m%-12s\033[33m疲  勞:\033[%dm%-12d\033[33m金  錢:\033[37m%-12d\033[31m│\033[m\n */
  prints("\033[1;31m\xA2\x78\033[33m\xA5\xCD  \xA9\x52:\033[37m%-12s\033[33m\xAA\x6B  \xA4\x4F:\033[37m%-12s\033[33m\xAF\x68  \xB3\xD2:\033[%dm%-12d\033[33m\xAA\xF7  \xBF\xFA:\033[37m%-12d\033[31m\xA2\x78\033[m\n", inbuf1, inbuf2, color, d.tired, d.money);
  /* \033[1;31m│\033[33m攻  擊:\033[37m%-12d\033[33m防  禦:\033[37m%-12d\033[33m速  度:\033[37m%-12d\033[33m經  驗:\033[37m%-12d\033[31m│\033[m\n */
  prints("\033[1;31m\xA2\x78\033[33m\xA7\xF0  \xC0\xBB:\033[37m%-12d\033[33m\xA8\xBE  \xBF\x6D:\033[37m%-12d\033[33m\xB3\x74  \xAB\xD7:\033[37m%-12d\033[33m\xB8\x67  \xC5\xE7:\033[37m%-12d\033[31m\xA2\x78\033[m\n", d.attack, d.resist, d.speed, d.exp);
  /* \033[1;31m│\033[33m食  物:\033[37m%-12d\033[33m大補丸:\033[37m%-12d\033[33m零  食:\033[37m%-12d\033[33m靈  芝:\033[37m%-12d\033[31m│\033[m\n */
  prints("\033[1;31m\xA2\x78\033[33m\xAD\xB9  \xAA\xAB:\033[37m%-12d\033[33m\xA4\x6A\xB8\xC9\xA4\x59:\033[37m%-12d\033[33m\xB9\x73  \xAD\xB9:\033[37m%-12d\033[33m\xC6\x46  \xAA\xDB:\033[37m%-12d\033[31m\xA2\x78\033[m\n", d.food, d.burger, d.cookie, d.medicine);
  /* \033[1;31m└──────────────────────────────────────┘\033[m */
  outs("\033[1;31m\xA2\x7C\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x7D\033[m");

  /* itoc.010801: 螢幕中間 7~16 列 秀出怪物的圖檔 */
  show_badman_pic(m.pic);

  /* itoc.010801: 螢幕下方秀出怪物的資料 */

  sprintf(inbuf1, "%d%s/%d%s", m.hp > 1000 ? m.hp / 1000 : m.hp,
    m.hp > 1000 ? "K" : "", m.maxhp > 1000 ? m.maxhp / 1000 : m.maxhp,
    m.maxhp > 1000 ? "K" : "");

  move(18, 0);
  /* \033[1;34m┌──────────────────────────────────────┐\033[m\n */
  outs("\033[1;34m\xA2\x7A\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x7B\033[m\n");
  /* \033[1;34m│\033[32m姓  名:\033[37m%-12s\033[32m生  命:\033[37m%-12s\033[32m防  護:\033[37m%-12d\033[32m閃  避:\033[37m%-12d\033[34m│\033[m\n */
  prints("\033[1;34m\xA2\x78\033[32m\xA9\x6D  \xA6\x57:\033[37m%-12s\033[32m\xA5\xCD  \xA9\x52:\033[37m%-12s\033[32m\xA8\xBE  \xC5\x40:\033[37m%-12d\033[32m\xB0\x7B  \xC1\xD7:\033[37m%-12d\033[34m\xA2\x78\033[m\n", m.name, inbuf1, m.armor, m.dodge);
  /* \033[1;34m│\033[32m攻  擊:\033[37m%-12d\033[32m內  力:\033[37m%-12d\033[32m魔  法:\033[37m%-12d\033[32m金  錢:\033[37m%-12d\033[34m│\033[m\n */
  prints("\033[1;34m\xA2\x78\033[32m\xA7\xF0  \xC0\xBB:\033[37m%-12d\033[32m\xA4\xBA  \xA4\x4F:\033[37m%-12d\033[32m\xC5\x5D  \xAA\x6B:\033[37m%-12d\033[32m\xAA\xF7  \xBF\xFA:\033[37m%-12d\033[34m\xA2\x78\033[m\n", m.attack, m.spirit, m.magic, m.money);
  /* \033[1;34m└──────────────────────────────────────┘\033[m\n */
  outs("\033[1;34m\xA2\x7C\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x7D\033[m\n");

  /*  戰鬥命令  */
  /*  [1]普通 [2]全力 [3]技能 [4]防禦 [5]補充 [6]煉妖 [A]自動 [Q]逃命    \033[m */
  out_cmd("", COLOR1 " \xBE\xD4\xB0\xAB\xA9\x52\xA5\x4F " COLOR2 " [1]\xB4\xB6\xB3\x71 [2]\xA5\xFE\xA4\x4F [3]\xA7\xDE\xAF\xE0 [4]\xA8\xBE\xBF\x6D [5]\xB8\xC9\xA5\x52 [6]\xB7\xD2\xA7\xAF [A]\xA6\xDB\xB0\xCA [Q]\xB0\x6B\xA9\x52    \033[m");
}


static void
pip_vs_ending(winorlost, area)
  int winorlost;		/* 2:K死怪物  1:怪物逃跑  -1:小雞逃跑  -2:小雞被K死 */
  char area;			/* !=0: K怪物  0:收穫季比賽 */
{
  int mode;

  mode = winorlost;
  if (!area)
    mode += 8;

  clrfromto(7, 16);
  move(8, 0);

  /* itoc.010731: 收穫季比賽若是輸了，則恢復一些血；若是贏了，恢復全部血 */
  /* itoc.010731: 戰勝加經驗值/錢，戰敗扣錢/屬性；但收穫季戰敗不扣錢/屬性 */

  switch (mode)
  {
  case 10:		/* 收穫季 KO */
    d.tired = 0;
    d.hp = d.maxhp;
    d.hexp += rand() % 3 + 2;
    d.mexp += rand() % 3 + 2;
    d.exp += m.exp;
    /*            \033[1;31m┌──────────────────────┐\033[m\n */
    outs("           \033[1;31m\xA2\x7A\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x7B\033[m\n");
    /*            \033[1;31m│ \033[37m武術大會的小雞\033[33m%-13s                \033[31m│\033[m\n */
    prints("           \033[1;31m\xA2\x78 \033[37m\xAA\x5A\xB3\x4E\xA4\x6A\xB7\x7C\xAA\xBA\xA4\x70\xC2\xFB\033[33m%-13s                \033[31m\xA2\x78\033[m\n", d.name);
    /*            \033[1;31m│ \033[37m打敗了強勁的對手\033[32m%-13s              \033[31m│\033[m\n */
    prints("           \033[1;31m\xA2\x78 \033[37m\xA5\xB4\xB1\xD1\xA4\x46\xB1\x6A\xAB\x6C\xAA\xBA\xB9\xEF\xA4\xE2\033[32m%-13s              \033[31m\xA2\x78\033[m\n", m.name);
    /*            \033[1;31m│ \033[37m勇敢和經驗都上升了不少                     \033[31m│\033[m\n */
    outs("           \033[1;31m\xA2\x78 \033[37m\xAB\x69\xB4\xB1\xA9\x4D\xB8\x67\xC5\xE7\xB3\xA3\xA4\x57\xA4\xC9\xA4\x46\xA4\xA3\xA4\xD6                     \033[31m\xA2\x78\033[m\n");
    /*            \033[1;31m└──────────────────────┘\033[m */
    outs("           \033[1;31m\xA2\x7C\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x7D\033[m");
    /* 您打敗了一個強硬的傢伙 */
    vmsg("\xB1\x7A\xA5\xB4\xB1\xD1\xA4\x46\xA4\x40\xAD\xD3\xB1\x6A\xB5\x77\xAA\xBA\xB3\xC3\xA5\xEB");
    break;

  case 9:		/* 收穫季 win */
    d.tired = 0;
    d.hp = d.maxhp;
    d.hexp += rand() % 2 + 1;
    d.mexp += rand() % 2 + 1;
    d.exp += m.exp * (50 + rand() % 20) / 100;		/* 得 60% 的經驗值 */
    /*            \033[1;31m┌──────────────────────┐\033[m\n */
    outs("           \033[1;31m\xA2\x7A\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x7B\033[m\n");
    /*            \033[1;31m│ \033[37m武術大會的小雞\033[33m%-13s                \033[31m│\033[m\n */
    prints("           \033[1;31m\xA2\x78 \033[37m\xAA\x5A\xB3\x4E\xA4\x6A\xB7\x7C\xAA\xBA\xA4\x70\xC2\xFB\033[33m%-13s                \033[31m\xA2\x78\033[m\n", d.name);
    /*            \033[1;31m│ \033[37m打敗了強勁的對手\033[32m%-13s              \033[31m│\033[m\n */
    prints("           \033[1;31m\xA2\x78 \033[37m\xA5\xB4\xB1\xD1\xA4\x46\xB1\x6A\xAB\x6C\xAA\xBA\xB9\xEF\xA4\xE2\033[32m%-13s              \033[31m\xA2\x78\033[m\n", m.name);
    /*            \033[1;31m│ \033[37m勇敢和經驗都上升了一些                     \033[31m│\033[m\n */
    outs("           \033[1;31m\xA2\x78 \033[37m\xAB\x69\xB4\xB1\xA9\x4D\xB8\x67\xC5\xE7\xB3\xA3\xA4\x57\xA4\xC9\xA4\x46\xA4\x40\xA8\xC7                     \033[31m\xA2\x78\033[m\n");
    /*            \033[1;31m└──────────────────────┘\033[m */
    outs("           \033[1;31m\xA2\x7C\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x7D\033[m");
    /* 不負眾望地，您解決了這個難纏的敵手 */
    vmsg("\xA4\xA3\xAD\x74\xB2\xB3\xB1\xE6\xA6\x61\xA1\x41\xB1\x7A\xB8\xD1\xA8\x4D\xA4\x46\xB3\x6F\xAD\xD3\xC3\xF8\xC4\xF1\xAA\xBA\xBC\xC4\xA4\xE2");
    break;

  case 7:		/* 收穫季 lose */
    d.tired = 50;
    d.hp = d.maxhp / 3;
    d.hexp -= rand() % 2 + 1;
    d.mexp -= rand() % 2 + 1;
    if (d.hexp < 0)
      d.hexp = 0;
    if (d.mexp < 0)
      d.mexp = 0;
    /*            \033[1;31m┌──────────────────────┐\033[m\n */
    outs("           \033[1;31m\xA2\x7A\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x7B\033[m\n");
    /*            \033[1;31m│ \033[37m武術大會的小雞\033[33m%-13s                \033[31m│\033[m\n */
    prints("           \033[1;31m\xA2\x78 \033[37m\xAA\x5A\xB3\x4E\xA4\x6A\xB7\x7C\xAA\xBA\xA4\x70\xC2\xFB\033[33m%-13s                \033[31m\xA2\x78\033[m\n", d.name);
    /*            \033[1;31m│ \033[37m被\033[32m%-13s\033[37m對手打得落花流水            \033[31m│\033[m\n */
    prints("           \033[1;31m\xA2\x78 \033[37m\xB3\x51\033[32m%-13s\033[37m\xB9\xEF\xA4\xE2\xA5\xB4\xB1\x6F\xB8\xA8\xAA\xE1\xAC\x79\xA4\xF4            \033[31m\xA2\x78\033[m\n", m.name);
    /*            \033[1;31m│ \033[37m決定回家好好再鍛練                         \033[31m│\033[m\n */
    outs("           \033[1;31m\xA2\x78 \033[37m\xA8\x4D\xA9\x77\xA6\x5E\xAE\x61\xA6\x6E\xA6\x6E\xA6\x41\xC1\xEB\xBD\x6D                         \033[31m\xA2\x78\033[m\n");
    /*            \033[1;31m└──────────────────────┘\033[m */
    outs("           \033[1;31m\xA2\x7C\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x7D\033[m");
    /* 落跑的您心中相當不是味道 */
    vmsg("\xB8\xA8\xB6\x5D\xAA\xBA\xB1\x7A\xA4\xDF\xA4\xA4\xAC\xDB\xB7\xED\xA4\xA3\xAC\x4F\xA8\xFD\xB9\x44");
    break;

  case 6:		/* 收穫季 KO-ed */
    d.tired = 50;
    d.hp = d.maxhp / 3;
    d.hexp -= rand() % 3 + 2;
    d.mexp -= rand() % 3 + 2;
    if (d.hexp < 0)
      d.hexp = 0;
    if (d.mexp < 0)
      d.mexp = 0;
    /*            \033[1;31m┌──────────────────────┐\033[m\n */
    outs("           \033[1;31m\xA2\x7A\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x7B\033[m\n");
    /*            \033[1;31m│ \033[37m武術大會的小雞\033[33m%-13s                \033[31m│\033[m\n */
    prints("           \033[1;31m\xA2\x78 \033[37m\xAA\x5A\xB3\x4E\xA4\x6A\xB7\x7C\xAA\xBA\xA4\x70\xC2\xFB\033[33m%-13s                \033[31m\xA2\x78\033[m\n", d.name);
    /*            \033[1;31m│ \033[37m完全不是\033[32m%-13s\033[37m的對手                \033[31m│\033[m\n */
    prints("           \033[1;31m\xA2\x78 \033[37m\xA7\xB9\xA5\xFE\xA4\xA3\xAC\x4F\033[32m%-13s\033[37m\xAA\xBA\xB9\xEF\xA4\xE2                \033[31m\xA2\x78\033[m\n", m.name);
    /*            \033[1;31m│ \033[37m發誓明年還要捲土重來                       \033[31m│\033[m\n */
    outs("           \033[1;31m\xA2\x78 \033[37m\xB5\x6F\xBB\x7D\xA9\xFA\xA6\x7E\xC1\xD9\xAD\x6E\xB1\xB2\xA4\x67\xAD\xAB\xA8\xD3                       \033[31m\xA2\x78\033[m\n");
    /*            \033[1;31m└──────────────────────┘\033[m */
    outs("           \033[1;31m\xA2\x7C\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x7D\033[m");
    /* 您躺在地上奄奄一息 */
    vmsg("\xB1\x7A\xBD\xF6\xA6\x62\xA6\x61\xA4\x57\xA9\x61\xA9\x61\xA4\x40\xAE\xA7");
    break;

  case 2:		/* KO 怪物 */
    d.money += m.money;
    d.exp += m.exp;
    d.exp += m.exp;
    d.brave += rand() % 4 + 3;
    /*            \033[1;31m┌──────────────────────┐\033[m\n */
    outs("           \033[1;31m\xA2\x7A\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x7B\033[m\n");
    /*            \033[1;31m│ \033[37m英勇的小雞\033[33m%-13s                    \033[31m│\033[m\n */
    prints("           \033[1;31m\xA2\x78 \033[37m\xAD\x5E\xAB\x69\xAA\xBA\xA4\x70\xC2\xFB\033[33m%-13s                    \033[31m\xA2\x78\033[m\n", d.name);
    /*            \033[1;31m│ \033[37m打敗了邪惡的怪物\033[32m%-13s              \033[31m│\033[m\n */
    prints("           \033[1;31m\xA2\x78 \033[37m\xA5\xB4\xB1\xD1\xA4\x46\xA8\xB8\xB4\x63\xAA\xBA\xA9\xC7\xAA\xAB\033[32m%-13s              \033[31m\xA2\x78\033[m\n", m.name);
    /*            \033[1;31m│ \033[37m勇敢和經驗都上升了不少                     \033[31m│\033[m\n */
    outs("           \033[1;31m\xA2\x78 \033[37m\xAB\x69\xB4\xB1\xA9\x4D\xB8\x67\xC5\xE7\xB3\xA3\xA4\x57\xA4\xC9\xA4\x46\xA4\xA3\xA4\xD6                     \033[31m\xA2\x78\033[m\n");
    /*            \033[1;31m└──────────────────────┘\033[m */
    outs("           \033[1;31m\xA2\x7C\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x7D\033[m");
    /* 對方死掉了啊，所以您獲勝了喔 */
    vmsg("\xB9\xEF\xA4\xE8\xA6\xBA\xB1\xBC\xA4\x46\xB0\xDA\xA1\x41\xA9\xD2\xA5\x48\xB1\x7A\xC0\xF2\xB3\xD3\xA4\x46\xB3\xE1");
    break;

  case 1:		/* 怪物逃跑 */
    d.money += m.money * (30 + rand() % 20) / 100;	/* 得 40% 的錢 */
    d.exp += m.exp * (50 + rand() % 20) / 100;		/* 得 60% 的經驗值 */
    d.brave += rand() % 3 + 2;
    /*            \033[1;31m┌──────────────────────┐\033[m\n */
    outs("           \033[1;31m\xA2\x7A\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x7B\033[m\n");
    /*            \033[1;31m│ \033[37m英勇的小雞\033[33m%-13s                    \033[31m│\033[m\n */
    prints("           \033[1;31m\xA2\x78 \033[37m\xAD\x5E\xAB\x69\xAA\xBA\xA4\x70\xC2\xFB\033[33m%-13s                    \033[31m\xA2\x78\033[m\n", d.name);
    /*            \033[1;31m│ \033[37m打敗了邪惡的怪物\033[32m%-13s              \033[31m│\033[m\n */
    prints("           \033[1;31m\xA2\x78 \033[37m\xA5\xB4\xB1\xD1\xA4\x46\xA8\xB8\xB4\x63\xAA\xBA\xA9\xC7\xAA\xAB\033[32m%-13s              \033[31m\xA2\x78\033[m\n", m.name);
    /*            \033[1;31m│ \033[37m勇敢和經驗都上升了一些                     \033[31m│\033[m\n */
    outs("           \033[1;31m\xA2\x78 \033[37m\xAB\x69\xB4\xB1\xA9\x4D\xB8\x67\xC5\xE7\xB3\xA3\xA4\x57\xA4\xC9\xA4\x46\xA4\x40\xA8\xC7                     \033[31m\xA2\x78\033[m\n");
    /*            \033[1;31m└──────────────────────┘\033[m */
    outs("           \033[1;31m\xA2\x7C\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x7D\033[m");
    /* 對方先閃了..但掉了一些錢給您... */
    vmsg("\xB9\xEF\xA4\xE8\xA5\xFD\xB0\x7B\xA4\x46..\xA6\xFD\xB1\xBC\xA4\x46\xA4\x40\xA8\xC7\xBF\xFA\xB5\xB9\xB1\x7A...");
    break;

  case -1:		/* 小雞逃跑 */
    d.money -= d.level * 5 + rand() % 100;
    d.brave -= rand() % 3 + 2;
    if (d.money < 0)
      d.money = 0;
    if (d.brave < 0)
      d.brave = 0;
    /*            \033[1;31m┌──────────────────────┐\033[m\n */
    outs("           \033[1;31m\xA2\x7A\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x7B\033[m\n");
    /*            \033[1;31m│ \033[37m可憐的小雞\033[33m%-13s                    \033[31m│\033[m\n */
    prints("           \033[1;31m\xA2\x78 \033[37m\xA5\x69\xBC\xA6\xAA\xBA\xA4\x70\xC2\xFB\033[33m%-13s                    \033[31m\xA2\x78\033[m\n", d.name);
    /*            \033[1;31m│ \033[37m在與\033[32m%-13s\033[37m的戰鬥中                  \033[31m│\033[m\n */
    prints("           \033[1;31m\xA2\x78 \033[37m\xA6\x62\xBB\x50\033[32m%-13s\033[37m\xAA\xBA\xBE\xD4\xB0\xAB\xA4\xA4                  \033[31m\xA2\x78\033[m\n", m.name);
    /*            \033[1;31m│ \033[37m不幸地打輸了，在此特別難過..........       \033[31m│\033[m\n */
    outs("           \033[1;31m\xA2\x78 \033[37m\xA4\xA3\xA9\xAF\xA6\x61\xA5\xB4\xBF\xE9\xA4\x46\xA1\x41\xA6\x62\xA6\xB9\xAF\x53\xA7\x4F\xC3\xF8\xB9\x4C..........       \033[31m\xA2\x78\033[m\n");
    /*            \033[1;31m└──────────────────────┘\033[m */
    outs("           \033[1;31m\xA2\x7C\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x7D\033[m");
    /* 小雞打輸了.... */
    vmsg("\xA4\x70\xC2\xFB\xA5\xB4\xBF\xE9\xA4\x46....");
    break;

  case -2:		/* 小雞 KO-ed */
    /*            \033[1;31m┌──────────────────────┐\033[m\n */
    outs("           \033[1;31m\xA2\x7A\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x7B\033[m\n");
    /*            \033[1;31m│ \033[37m可憐的小雞\033[33m%-13s                    \033[31m│\033[m\n */
    prints("           \033[1;31m\xA2\x78 \033[37m\xA5\x69\xBC\xA6\xAA\xBA\xA4\x70\xC2\xFB\033[33m%-13s                    \033[31m\xA2\x78\033[m\n", d.name);
    /*            \033[1;31m│ \033[37m在與\033[32m%-13s\033[37m的戰鬥中                  \033[31m│\033[m\n */
    prints("           \033[1;31m\xA2\x78 \033[37m\xA6\x62\xBB\x50\033[32m%-13s\033[37m\xAA\xBA\xBE\xD4\xB0\xAB\xA4\xA4                  \033[31m\xA2\x78\033[m\n", m.name);
    /*            \033[1;31m│ \033[37m不幸地陣亡了，在此特別默哀..........       \033[31m│\033[m\n */
    outs("           \033[1;31m\xA2\x78 \033[37m\xA4\xA3\xA9\xAF\xA6\x61\xB0\x7D\xA4\x60\xA4\x46\xA1\x41\xA6\x62\xA6\xB9\xAF\x53\xA7\x4F\xC0\x71\xAB\x73..........       \033[31m\xA2\x78\033[m\n");
    /*            \033[1;31m└──────────────────────┘\033[m */
    outs("           \033[1;31m\xA2\x7C\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x7D\033[m");
    /* 小雞陣亡了.... */
    vmsg("\xA4\x70\xC2\xFB\xB0\x7D\xA4\x60\xA4\x46....");
    /* \033[1;31m戰鬥中被打死了...\033[m   */
    pipdie("\033[1;31m\xBE\xD4\xB0\xAB\xA4\xA4\xB3\x51\xA5\xB4\xA6\xBA\xA4\x46...\033[m  ", 1);
    break;
  }
}


/* static */			/* itoc.010731: 給收穫季對戰用 */
int				/* 1: 小雞 win  0: 小雞 lose */
pip_vs_man(p, area)
  struct playrule p;		/* 傳入怪物資料 */
  int area;			/* !=0: K怪物時的區域  0: 收穫季比賽 */
{
  int d_dr;			/* 小雞打怪物 物理攻擊指數 damage rate */
  int d_sr;			/* 小雞打怪物 武功攻擊指數 spirit rate */
  int d_mr;			/* 小雞打怪物 魔法攻擊指數 magic rate */
  int d_hr;			/* 小雞打怪物 物理/武功/魔法攻擊命中率 hit rate */

  int m_dr;			/* 怪物打小雞 物理攻擊指數 damage rate */
  int m_sr;			/* 怪物打小雞 武功攻擊指數 spirit rate */
  int m_mr;			/* 怪物打小雞 魔法攻擊指數 magic rate */
  int m_hr;			/* 怪物打小雞 物理/武功/魔法攻擊命中率 hit rate */

  int randnum;			/* random number */
  int pipkey;			/* 小雞下的指令 */
  int mankey;			/* 怪物下的指令 */
  char buf[80];

  /* 產生怪物資料 */
  if (area)		/* K怪物 */
  {
    badman_generate(area);
  }
  else			/* 收穫季比賽 */
  { 
    strcpy(m.name, p.name);
    m.attribute = p.attribute;
    m.hp = p.hp;
    m.maxhp = p.maxhp;
    m.attack = p.attack;
    m.spirit = p.spirit;
    m.magic = p.magic;
    m.armor = p.armor;
    m.dodge =  p.dodge;
    m.money = p.money;
    m.pic = p.pic;
  }

  /* itoc.010731: 一開始就算好一些參數，然後等一下直接傳入函數求傷害值 */
  d_dr = d.attack + (d.hskill + d.hexp) / 10 - m.armor;
  d_sr = (d.attack + d.brave) / 2 + (d.hskill + d.hexp) / 10 - m.armor;
  d_mr = d.mskill + (d.immune + d.mexp) / 10 - m.armor;
  d_hr = d.speed + (d.hskill + d.hexp) / 10 - m.dodge;
  m_dr = m.attack - d.resist;
  m_sr = m.spirit - (d.speed + d.resist) / 2;
  m_mr = m.magic - d.immune;
  m_hr = m.attack - d.speed;

  /* itoc.020718: 要檢查這些參數，確定是正的 */
  if (d_dr <= 0)
    d_dr = 1;
  if (d_sr <= 0)
    d_sr = 1;
  if (d_mr <= 0)
    d_mr = 1;
  if (d_hr <= 0)
    d_hr = 1;
  if (m_dr <= 0)
    m_dr = 1;
  if (m_sr <= 0)
    m_sr = 1;
  if (m_mr <= 0)
    m_mr = 1;
  if (m_hr <= 0)
    m_hr = 1;

  /* itoc.010820: 如果有特殊技能，可以再加成 */
  if (d.skillXYZ & 0x0001)		/* 左右互搏 */
    d_dr += d.level << 3;
  else if (d.skillXYZ & 0x0002)		/* 力拔山河 */
    d_sr += d.level << 3;
  else if (d.skillXYZ & 0x0004)		/* 魔喚精靈 */
    d_mr += d.level << 3;
  else if (d.skillXYZ & 0x0008)		/* 快馬加鞭 */
    d_hr += d.level << 3;

  /* itoc.010730: 決定誰先攻擊 */
  if (d.skillXYZ & 0x0040)		/* 優先攻擊 */
  {
    d_nodone = 1;
  }
  else
  {
    d_nodone = rand() % 10 > 2;		/* 30% 由怪物先攻擊 */
    if (!d_nodone)
    {
      /* 您被%s偷襲了 */
      sprintf(buf, "\xB1\x7A\xB3\x51%s\xB0\xBD\xC5\xA7\xA4\x46", m.name);
      vmsg(buf);
    }
  }

  d_resistmore = 0;
  m_resistmore = 0;
  pipkey = 0;

  for (;;)		/* 無窮迴圈 */
  {
    /* 秀出戰鬥主畫面 */
    pip_vs_showing();

    while (d_nodone)
    {
      if (pipkey != 'a')	/* itoc.010820: 自動攻擊 */
        pipkey = vkey();

      switch (pipkey)		/* 小雞下指令 */
      {
      case 'a':		/* 自動攻擊只執行普通攻擊 */
      case '1':		/* 普通攻擊 */
	show_fight_pic(1);
	pip_attack_normal(1, d_dr, d_hr);
	break;

      case '2':		/* 全力一擊 */
	if (d.hp > 5)
	{
	  show_fight_pic(2);
	  pip_attack_aggressive(1, d_dr, d_hr);
	}
	else
	{
	  /* 您體力這麼差了還想全力一擊 */
	  vmsg("\xB1\x7A\xC5\xE9\xA4\x4F\xB3\x6F\xBB\xF2\xAE\x74\xA4\x46\xC1\xD9\xB7\x51\xA5\xFE\xA4\x4F\xA4\x40\xC0\xBB");
	}
	break;

      case '3':		/* 技能: 武功/魔法 */
	pip_skill_menu(d_dr, d_sr, d_mr, d_hr);
	if (d_nodone)	/* 進入 skill_menu 出來若還是 d_nodone，表示放棄使用技能，要重繪指令 */
	  /*  戰鬥命令  */
	  /*  [1]普通 [2]全力 [3]技能 [4]防禦 [5]補充 [6]煉妖 [A]自動 [Q]逃命    \033[m */
	  out_cmd("", COLOR1 " \xBE\xD4\xB0\xAB\xA9\x52\xA5\x4F " COLOR2 " [1]\xB4\xB6\xB3\x71 [2]\xA5\xFE\xA4\x4F [3]\xA7\xDE\xAF\xE0 [4]\xA8\xBE\xBF\x6D [5]\xB8\xC9\xA5\x52 [6]\xB7\xD2\xA7\xAF [A]\xA6\xDB\xB0\xCA [Q]\xB0\x6B\xA9\x52    \033[m");
	break;

      case '4':		/* 防禦 */
	d_resistmore = 40;		/* 怪物攻擊減少 40% */
	d.tired += rand() % 2 + 1;
	d_nodone = 0;
	show_fight_pic(3);
	/* 小雞加強防禦啦.... */
	vmsg("\xA4\x70\xC2\xFB\xA5\x5B\xB1\x6A\xA8\xBE\xBF\x6D\xB0\xD5....");
	break;

      case '5':		/* 吃補品 */
	d_nodone = pip_basic_feed();	/* feed 完傳回 0 表示吃完了，傳回 1 表示放棄沒吃 */
	pip_vs_showing();		/* itoc.010801: 需要完整重繪 */

	if (d.skillXYZ & 0x0020)	/* 得來速 */
	  d_nodone = 1;
	break;

      case '6':		/* 煉妖 */
	if (m.hp < m.maxhp * (d.level - rand() % 150) / 100)
	{
	  d.mexp += 8;
	  m.hp = 0;
	  /* 捕捉成功，名聲增加 */
	  vmsg("\xAE\xB7\xAE\xBB\xA6\xA8\xA5\x5C\xA1\x41\xA6\x57\xC1\x6E\xBC\x57\xA5\x5B");
	}
	else
	{
	  /* 捕捉失敗 */
	  vmsg("\xAE\xB7\xAE\xBB\xA5\xA2\xB1\xD1");
	}
	d_nodone = 0;
	break;

      case 'q':		/* 逃跑 */
  	pip_vs_ending(-1, area);
	return 0;
      }
    }

    /* 小雞攻擊後，需要檢查小雞/怪物狀態 */
    if (m.hp <= 0)			/* 怪物死掉了 */
    {
      pip_vs_ending(2, area);
      return 1;
    }
    else if (d.tired >= 100)
    {
      /* 小雞累死了，防禦力大降 */
      /* 您已經太疲累了，防禦力大幅降低 */
      vmsg("\xB1\x7A\xA4\x77\xB8\x67\xA4\xD3\xAF\x68\xB2\xD6\xA4\x46\xA1\x41\xA8\xBE\xBF\x6D\xA4\x4F\xA4\x6A\xB4\x54\xAD\xB0\xA7\x43");
      d_resistmore = -100;	/* 怪物攻擊加 100% */
    }


    /* 到此小雞已下完指令，該怪物下指令 */

    /* 決定怪物下的指令 */
    randnum = rand() % 100;
    if (m.attribute)		/* 有特殊的技能 */
    {
      if (randnum < 40)
	mankey = 1;		/* 40% 普通攻擊 */
      else if (randnum < 50)
	mankey = 2;		/* 10% 全力攻擊 */
      else if (randnum < 90)
	mankey = 3;		/* 40% 技能攻擊 */
      else if (randnum < 98)
	mankey = 4;		/* 8% 加強防禦 */
      else
	mankey = 5;		/* 2% 逃之夭夭 */
    }
    else
    {
      if (randnum < 40)
	mankey = 1;		/* 40% 普通攻擊 */
      else if (randnum < 65)
	mankey = 2;		/* 25% 全力攻擊 */
      else if (randnum < 90)
	mankey = 3;		/* 25% 技能攻擊 */
      else if (randnum < 98)
	mankey = 4;		/* 8% 加強防禦 */
      else
	mankey = 5;		/* 2% 逃之夭夭 */
    }

    switch (mankey)
    {
    case 2:		/* 怪物全力攻擊 */
      if (m.hp > 5)		/* 若怪物血不到五點，變普通攻擊 */
      {
	show_fight_pic(52);
	pip_attack_aggressive(0, m_dr, m_hr);
	break;
      }

    case 1:		/* 怪物普通攻擊 */
      show_fight_pic(51);
      pip_attack_normal(0, m_dr, m_hr);
      break;

    case 3:		/* 怪物技能攻擊 */
      pip_attack_skill(m_dr, m_sr, m_mr, m_hr);
      break;

    case 4:		/* 怪物防禦 */
      m_resistmore = 40;	/* 小雞攻擊減少 40% */
      break;

    case 5:		/* 怪物逃跑 */
      pip_vs_ending(1, area);
      return 1;
    }

    /* 怪物攻擊後，只需要檢查小雞狀態 */

    if (d.hp <= 0)	/* 小雞死掉了 */
    {
      pip_vs_ending(-2, area);
      return 0;
    }

    d_nodone = 1;	/* 又該小雞了 */

  }		/* 結束 for 迴圈，戰鬥結束 */
}


/*-------------------------------------------------------*/
/* 地圖產生器                                            */
/*-------------------------------------------------------*/


/* itoc.041017: 為了多樣性，寫一個地圖產生器 */

/* 在這間房間裡有哪些方向可以前進 */
#define MAP_EAST	0x01
#define MAP_WEST	0x02
#define MAP_SOUTH	0x04
#define MAP_NORTH	0x08


static int		/* 傳回本房間有哪些方向可以走 */
map_generate()
{
  int direction;
  int map[16] = 
  {
    MAP_EAST | MAP_WEST | MAP_SOUTH | MAP_NORTH,
    MAP_EAST, MAP_WEST, MAP_SOUTH, MAP_NORTH, 
    MAP_EAST | MAP_WEST, MAP_EAST | MAP_SOUTH, MAP_EAST | MAP_NORTH, MAP_WEST | MAP_SOUTH, MAP_WEST | MAP_NORTH, MAP_SOUTH | MAP_NORTH, 
    MAP_EAST | MAP_WEST | MAP_SOUTH, MAP_EAST | MAP_WEST | MAP_NORTH, MAP_EAST | MAP_SOUTH | MAP_NORTH, MAP_WEST | MAP_SOUTH | MAP_NORTH, 
    MAP_EAST | MAP_WEST | MAP_SOUTH | MAP_NORTH
  };

  /* 決定這間房間有哪些出口 */

  direction = map[rand() & 15];	/* 決定這間房間有哪些出口 */
  move(16, 0);
  /*   出口：\033[1m%s%s%s%s\033[m */
  prints("  \xA5\x58\xA4\x66\xA1\x47\033[1m%s%s%s%s\033[m", 
    /* \033[31m東  */
    direction & MAP_EAST ? "\033[31m\xAA\x46 " : "",
    /* \033[32m西  */
    direction & MAP_WEST ? "\033[32m\xA6\xE8 " : "",
    /* \033[33m南  */
    direction & MAP_SOUTH ? "\033[33m\xAB\x6E " : "",
    /* \033[34m北 */
    direction & MAP_NORTH ? "\033[34m\xA5\x5F" : "");

  return direction;
}


/*-------------------------------------------------------*/
/* 戰鬥主選單                                            */
/*-------------------------------------------------------*/


int
pip_fight_menu()
{
  int ch, area, direction;

  show_badman_pic(0);
  /*  甲區  */
  /*  [1]炎之洞窟 [2]北方冰原 [3]古代遺跡 [Q]離開                            \033[m */
  out_cmd(COLOR1 " \xA5\xD2\xB0\xCF " COLOR2 " [1]\xAA\xA2\xA4\xA7\xAC\x7D\xB8\x5D [2]\xA5\x5F\xA4\xE8\xA6\x42\xAD\xEC [3]\xA5\x6A\xA5\x4E\xBF\xF2\xB8\xF1 [Q]\xC2\xF7\xB6\x7D                            \033[m", 
    /*  乙區  */
    /*  [4]人工島嶼 [5]地獄之門 [6]金庸群俠 [Q]離開                            \033[m */
    COLOR1 " \xA4\x41\xB0\xCF " COLOR2 " [4]\xA4\x48\xA4\x75\xAE\x71\xC0\xAC [5]\xA6\x61\xBA\xBB\xA4\xA7\xAA\xF9 [6]\xAA\xF7\xB1\x65\xB8\x73\xAB\x4C [Q]\xC2\xF7\xB6\x7D                            \033[m");

  while (1)
  {
    area = vkey();
    if (area == 'q')
      return 0;
    if (area >= '1' && area <= '6')
      break;
  }

  clrfromto(7, 17);

  while (d.hp > 0)
  {
    direction = map_generate();
    /*  區域  */
    /*  [F]餵食 [E/W/S/N]東/西/南/北 [Q]回家                                   \033[m */
    out_cmd("", COLOR1 " \xB0\xCF\xB0\xEC " COLOR2 " [F]\xC1\xFD\xAD\xB9 [E/W/S/N]\xAA\x46/\xA6\xE8/\xAB\x6E/\xA5\x5F [Q]\xA6\x5E\xAE\x61                                   \033[m");

re_key:
    ch = vkey();

    if (ch == 'q')
      break;

    if (ch == 'f')	/* 餵食 */
    {
      pip_basic_feed();
      continue;
    }


    /* 其他按鍵都是方向 */

    ch = ch == 'e' ? MAP_EAST : ch == 'w' ? MAP_WEST : ch == 's' ? MAP_SOUTH : ch == 'n' ? MAP_NORTH : 0;
    if (!(ch & direction))	/* 這個方向不能前進 */
      goto re_key;		/* 不需要重新產生地圖 */

    if ((ch == 'e' && !(direction & MAP_EAST)) || (ch == 'e' && !(direction & MAP_EAST)) ||
      (ch == 'e' && !(direction & MAP_EAST)) || (ch == 'e' && !(direction & MAP_EAST)))

    if (d.vp < 10)	/* 戰鬥地圖中移動扣移動力，至少 10 才能移動 */
    {
      /* 您已累得無法再行動 */
      vmsg("\xB1\x7A\xA4\x77\xB2\xD6\xB1\x6F\xB5\x4C\xAA\x6B\xA6\x41\xA6\xE6\xB0\xCA");
      continue;		/* 可以用餵食繼續 */
    }

    d.vp -= 2;

    if (rand() % 3)	/* 三分之二的機會遇到敵人 */
    {
      pip_vs_man(m, area);	/* 進入戰鬥 */
    }
    else
    {
      if (d.skillXYZ & 0x0010)	/* 尋龍訣 */
      {
	d.hp += (d.maxhp >> 2);
	if (d.hp > d.maxhp)
	  d.hp = d.maxhp;
	/* 您沿路為人妙演天機 */
	vmsg("\xB1\x7A\xAA\x75\xB8\xF4\xAC\xB0\xA4\x48\xA7\xAE\xBA\x74\xA4\xD1\xBE\xF7");
      }
      else
      {
	/* 沒發生任何事！ */
	vmsg("\xA8\x53\xB5\x6F\xA5\xCD\xA5\xF4\xA6\xF3\xA8\xC6\xA1\x49");
      }
    }
  }

  /* itoc.010730: 在離開戰鬥選單時才一併檢查是否升級 */
  pip_check_levelup();

  return 0;
}
#endif	/* HAVE_GAME */
