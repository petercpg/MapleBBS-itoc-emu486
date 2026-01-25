/* ----------------------------------------------------- */
/* pip_weapon.c     ( NTHU CS MapleBBS Ver 3.10 )        */
/* ----------------------------------------------------- */
/* target : 小雞 weapon structure                        */
/* create :   /  /                                       */
/* update : 01/08/15                                     */
/* author : dsyan.bbs@forever.twbbs.org                  */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/* ----------------------------------------------------- */


#include "bbs.h"

#ifdef HAVE_GAME

#include "pip.h"


/* ------------------------------------------------------- */
/* 武器購買函式                                            */
/* ------------------------------------------------------- */


/* name[11] quality cost */
static weapon p[9];		/* 記錄武器 */


/* itoc.021031: 為了增加遊戲的多樣性，寫一支武器產生器 */
static void
weapon_generate(type)
  int type;			/* 哪一部分裝備 */
{
  int i, num;

  /* 損壞 */
  /* 再生 */
  /* 二手 */
  /* 絕版 */
  /* 塑膠 */
  /* 牛皮 */
  /* 鋼鐵 */
  /* 黃金 */
  /* 特級 */
  /* 屠龍 */
  /* 忘情 */
  /* 屠龍 */
  /* 飛天 */
  /* 傳奇 */
  char adje[14][5] = {"\xB7\x6C\xC3\x61", "\xA6\x41\xA5\xCD", "\xA4\x47\xA4\xE2", "\xB5\xB4\xAA\xA9", "\xB6\xEC\xBD\xA6", "\xA4\xFB\xA5\xD6", "\xBF\xFB\xC5\x4B", "\xB6\xC0\xAA\xF7", "\xAF\x53\xAF\xC5", "\xB1\x4F\xC0\x73", "\xA7\xD1\xB1\xA1", "\xB1\x4F\xC0\x73", "\xAD\xB8\xA4\xD1", "\xB6\xC7\xA9\x5F"};
  /* 破 */
  /* 爛 */
  /* 鳥 */
  /* 之 */
  /* 狂 */
  /* 烈 */
  /* 炫 */
  /* 聖 */
  /* 魔 */
  /* 寶 */
  /* 光 */
  /* 神 */
  char prep[13][3] = {"\xAF\x7D",   "\xC4\xEA",   "\xB3\xBE",   "\xA4\xA7",   "\xA8\x67",   "\xAF\x50",   "\xAC\xAF",   "\xB8\x74",   "\xC5\x5D",   "\xC4\x5F",   "\xA5\xFA",   "\xAF\xAB",   ""};
  char noun[5][9][5] =
  {
    /* 帽 */
    /* 頭盔 */
    /* 頭罩 */
    /* 頭巾 */
    /* 頭飾 */
    /* 耳機 */
    /* 眼鏡 */
    /* 髮箍 */
    /* 項鍊 */
    /* 頭部武器 */    "\xB4\x55",   "\xC0\x59\xB2\xAF", "\xC0\x59\xB8\x6E", "\xC0\x59\xA4\x79", "\xC0\x59\xB9\xA2", "\xA6\xD5\xBE\xF7", "\xB2\xB4\xC3\xE8", "\xBE\x76\xE3\x54", "\xB6\xB5\xC1\xE5", 
    /* 劍 */
    /* 刀 */
    /* 杖 */
    /* 棒 */
    /* 槍 */
    /* 矛 */
    /* 弓 */
    /* 鎚 */
    /* 扳手 */
    /* 手部武器 */    "\xBC\x43",   "\xA4\x4D",   "\xA7\xFA",   "\xB4\xCE",   "\xBA\x6A",   "\xA5\xD9",   "\xA4\x7D",   "\xC2\xF1",   "\xA7\xE6\xA4\xE2", 
    /* 錶 */
    /* 盾 */
    /* 戒指 */
    /* 手套 */
    /* 手環 */
    /* 臂章 */
    /* 盾牌 */
    /* 課本 */
    /* 講義 */
    /* 盾牌武器 */    "\xBF\xF6",   "\xAC\xDE",   "\xA7\xD9\xAB\xFC", "\xA4\xE2\xAE\x4D", "\xA4\xE2\xC0\xF4", "\xC1\x75\xB3\xB9", "\xAC\xDE\xB5\x50", "\xBD\xD2\xA5\xBB", "\xC1\xBF\xB8\x71", 
    /* 盔甲 */
    /* 冑甲 */
    /* 皮甲 */
    /* 披風 */
    /* 套裝 */
    /* 洋裝 */
    /* 衣服 */
    /* Ｔ恤 */
    /* 毛衣 */
    /* 身體武器 */    "\xB2\xAF\xA5\xD2", "\xAB\x60\xA5\xD2", "\xA5\xD6\xA5\xD2", "\xA9\xDC\xAD\xB7", "\xAE\x4D\xB8\xCB", "\xAC\x76\xB8\xCB", "\xA6\xE7\xAA\x41", "\xA2\xE2\xAB\xF2", "\xA4\xF2\xA6\xE7", 
    /* 鞋 */
    /* 靴 */
    /* 屐 */
    /* 履 */
    /* 雲 */
    /* 輪 */
    /* 襪 */
    /* 毯 */
    /* 踏 */
    /* 腳部武器 */    "\xBE\x63",   "\xB9\x75",   "\xAE\x6A",   "\xBC\x69",   "\xB6\xB3",   "\xBD\xFC",   "\xC4\xFB",   "\xB4\xE0",   "\xBD\xF1"
  };

  for (i = 0; i < 9; i++)
  {
    /* 依能力及手頭的錢來決定武器的好壞 */

    if (d.money < 12)
    {
      p[i].quality = 1;
      p[i].cost = d.money;
    }
    else
    {
      num = d.money / 1000 + 1;
      if (num > 300)
        num = 300;
      num = rand() % num + 1;
      p[i].quality = num;
      p[i].cost = 3 * num * num;
    }

    num = rand();	/* 用同一亂數來決定 adj+prep+noun，所以 mod 的數不要一樣 */
    /* 依哪一部分裝備來決定武器名稱，注意字串長度 */
    sprintf(p[i].name, "%s%s%s", adje[num % 14], prep[num % 13], noun[type][num % 9]);
  }
}


void
pip_weapon_wear(type, variance)	/* 裝備武器，計算能力的改變 */
  int type;			/* 哪一部分裝備 */
  int variance;			/* 新舊武器的品質差異 */
{
  /* 依裝備部位不同來改變指數 */
  if (type == 0)	/* 頭部武器 */
  {
    d.speed += variance;
    d.immune += variance;
  }
  else if (type == 1)	/* 手部武器 */
  {
    d.attack += variance;
    d.immune += variance;
  }
  else if (type == 2)	/* 盾牌武器 */
  {
    d.attack += variance;
    d.resist += variance;
  }
  else if (type == 3)	/* 身體武器 */
  {
    d.resist += variance;
    d.immune += variance;
  }
  else if (type == 4)	/* 腳部武器 */
  {
    d.attack += variance;
    d.speed += variance;
  }
}


static int
pip_weapon_doing_menu(quality, type, name)	/* 武器購買畫面 */
  int quality;			/* 傳入目前配戴 */
  int type;			/* 哪一部分裝備 */
  char *name;
{
  /* 頭部裝備區 */
  /* 手部裝備區 */
  /* 盾牌裝備區 */
  /* 身體裝備區 */
  /* 腳部裝備區 */
  char menutitle[5][11] = {"\xC0\x59\xB3\xA1\xB8\xCB\xB3\xC6\xB0\xCF", "\xA4\xE2\xB3\xA1\xB8\xCB\xB3\xC6\xB0\xCF", "\xAC\xDE\xB5\x50\xB8\xCB\xB3\xC6\xB0\xCF", "\xA8\xAD\xC5\xE9\xB8\xCB\xB3\xC6\xB0\xCF", "\xB8\x7D\xB3\xA1\xB8\xCB\xB3\xC6\xB0\xCF"};
  char buf[80];
  int n;

  /* 亂數產生武器 */
  weapon_generate(type);

  /* 印出武器列表 */
  vs_head(menutitle[type], str_site);
  show_weapon_pic(0);
  move(11, 0);
  /*   \033[1;37;41m [NO]  [武器名稱]  [品質]  [售價] \033[m\n */
  outs("  \033[1;37;41m [NO]  [\xAA\x5A\xBE\xB9\xA6\x57\xBA\xD9]  [\xAB\x7E\xBD\xE8]  [\xB0\xE2\xBB\xF9] \033[m\n");

  /* 印出武器單項 */    
  for (n = 0; n < 9; n++)
    prints("   %d     %-10s  %6d  %6d\n", n, p[n].name, p[n].quality, p[n].cost);

  /* 選單處理 */
  while (1)
  {
    /*  採買  */
    /*  (軍火販子) [B]購買武器 [E]強化武器 [D]拋棄武器 [Q]跳出                 \033[m */
    out_cmd("", COLOR1 " \xB1\xC4\xB6\x52 " COLOR2 " (\xAD\x78\xA4\xF5\xB3\x63\xA4\x6C) [B]\xC1\xCA\xB6\x52\xAA\x5A\xBE\xB9 [E]\xB1\x6A\xA4\xC6\xAA\x5A\xBE\xB9 [D]\xA9\xDF\xB1\xF3\xAA\x5A\xBE\xB9 [Q]\xB8\xF5\xA5\x58                 \033[m");

    switch (vkey())
    {
    case 'b':
      /* 您有 %d 元，想要購買啥呢？[Q]  */
      sprintf(buf, "\xB1\x7A\xA6\xB3 %d \xA4\xB8\xA1\x41\xB7\x51\xAD\x6E\xC1\xCA\xB6\x52\xD4\xA3\xA9\x4F\xA1\x48[Q] ", d.money);
      n = ians(b_lines - 2, 1, buf) - '0';

      if (n >= 0 && n < 9)
      {
	/* 確定要購買價值 %d 元 的%s嗎(Y/N)？[N]  */
	sprintf(buf, "\xBD\x54\xA9\x77\xAD\x6E\xC1\xCA\xB6\x52\xBB\xF9\xAD\xC8 %d \xA4\xB8 \xAA\xBA%s\xB6\xDC(Y/N)\xA1\x48[N] ", p[n].cost, p[n].name);
	if (ians(b_lines - 2, 1, buf) == 'y')
	{
	  /* 換武器 */
	  d.money -= p[n].cost;
	  strcpy(name, p[n].name);
	  pip_weapon_wear(type, p[n].quality - quality);
	  quality = p[n].quality;

	  /* 小雞已經裝配上%s了 */
	  sprintf(buf, "\xA4\x70\xC2\xFB\xA4\x77\xB8\x67\xB8\xCB\xB0\x74\xA4\x57%s\xA4\x46", name);
	  vmsg(buf);
	}
	else
	{
	  /* 放棄購買 */
	  vmsg("\xA9\xF1\xB1\xF3\xC1\xCA\xB6\x52");
	}
      }
      break;

    case 'e':
      n = quality * 100;
      if (quality && d.money >= n)
      {
        /* 確定要花 %d 元來提升%s的潛能嗎(Y/N)？[N]  */
        sprintf(buf, "\xBD\x54\xA9\x77\xAD\x6E\xAA\xE1 %d \xA4\xB8\xA8\xD3\xB4\xA3\xA4\xC9%s\xAA\xBA\xBC\xE7\xAF\xE0\xB6\xDC(Y/N)\xA1\x48[N] ", n, name);
        if (ians(b_lines - 2, 1, buf) == 'y')
        {
          /* 品質越好的武器強化收費越高 */
          d.money -= n;
          quality++;
          pip_weapon_wear(type, 1);
        }
      }
      break;

    case 'd':
      /* 確定要拋棄%s嗎(Y/N)？[N]  */
      sprintf(buf, "\xBD\x54\xA9\x77\xAD\x6E\xA9\xDF\xB1\xF3%s\xB6\xDC(Y/N)\xA1\x48[N] ", name);
      if (ians(b_lines - 2, 1, buf) == 'y')
      {
        pip_weapon_wear(type, -quality);
        name[0] = '\0';
        quality = 0;
      }
      break;

    case 'q':
    case KEY_LEFT:
      return quality;
    }

    /* itoc.010816: 消掉 ians() 留下的殘骸 */
    move (b_lines - 2, 0);
    clrtoeol();
  }
}


/*-------------------------------------------------------*/
/* 武器商店選單: 各部位                                  */
/*-------------------------------------------------------*/


int
pip_store_weapon_head()		/* 頭部武器 */
{
  d.weaponhead = pip_weapon_doing_menu(d.weaponhead, 0, d.equiphead);
  return 0;
}


int
pip_store_weapon_hand()		/* 手部武器 */
{
  d.weaponhand = pip_weapon_doing_menu(d.weaponhand, 1, d.equiphand);
  return 0;
}


int
pip_store_weapon_shield()	/* 盾牌武器 */
{
  d.weaponshield = pip_weapon_doing_menu(d.weaponshield, 2, d.equipshield);
  return 0;
}


int
pip_store_weapon_body()		/* 身體武器 */
{
  d.weaponbody = pip_weapon_doing_menu(d.weaponbody, 3, d.equipbody);
  return 0;
}


int
pip_store_weapon_foot()		/* 腳部武器 */
{
  d.weaponfoot = pip_weapon_doing_menu(d.weaponfoot, 4, d.equipfoot);
  return 0;
}
#endif	/* HAVE_GAME */
