/* ----------------------------------------------------- */
/* pip_quest.c        ( NTHU CS MapleBBS Ver 3.10 )      */
/* ----------------------------------------------------- */
/* target : 戰鬥選單                                     */
/* create : 01/12/22                                     */
/* update :   /  /  		  			 */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */  
/* ----------------------------------------------------- */


#include "bbs.h"

#ifdef HAVE_GAME

#include "pip.h"


/*-------------------------------------------------------*/
/* 所有任務: 回傳 1 表示完成   回傳 0 表示沒有完成       */
/*-------------------------------------------------------*/


/* 寫任務非常簡單：寫一個函式 pip_quest_??()，並加入 quest_cb，
   並改 PIPQUEST_NUM，最後編輯一個 etc/game/pip/quest/pic?? 即可 */


  /*-----------------------------------------------------*/
  /* pip_quest_1~99 取得物品的任務			 */
  /*-----------------------------------------------------*/


static int
pip_quest_1()		/* 取得基礎裝備 */
{
  /* 全身每個部位都穿著裝備就算合格 */
  if (d.weaponhead && d.weaponhand && d.weaponshield && d.weaponbody && d.weaponfoot)
  {
    /* 原裝備移除 */
    d.weaponhead = d.weaponhand = d.weaponshield = d.weaponbody = d.weaponfoot = 0;
    d.equiphead[0] = d.equiphand[0] = d.equipshield[0] = d.equipbody[0] = d.equipfoot[0] = '\0';

    d.tired = 0;
    /* 您不再感到疲憊了 */
    vmsg("\xB1\x7A\xA4\xA3\xA6\x41\xB7\x50\xA8\xEC\xAF\x68\xBE\xCE\xA4\x46");
    return 1;
  }

  return 0;
}


static int
pip_quest_2()		/* 取得黃金裝備 */
{
  /* 全身每個部位的裝備名稱都包含「金」這字就算合格 */
  /* 金 */
  /* 金 */
  if (strstr(d.equiphead, "\xAA\xF7") && strstr(d.equiphand, "\xAA\xF7") &&
    /* 金 */
    /* 金 */
    /* 金 */
    strstr(d.equipshield, "\xAA\xF7") && strstr(d.equipbody, "\xAA\xF7") && strstr(d.equipfoot, "\xAA\xF7"))
  {
    /* 原裝備移除 */
    d.weaponhead = d.weaponhand = d.weaponshield = d.weaponbody = d.weaponfoot = 0;
    d.equiphead[0] = d.equiphand[0] = d.equipshield[0] = d.equipbody[0] = d.equipfoot[0] = '\0';

    d.happy = 100;
    d.tired = 0;
    /* 您的精神百倍 */
    vmsg("\xB1\x7A\xAA\xBA\xBA\xEB\xAF\xAB\xA6\xCA\xAD\xBF");
    return 1;
  }

  return 0;
}


static int
pip_quest_3()		/* 取得藥材 */
{
  /* 取得大還丹、靈芝、大補丸、千年人蔘、黑玉斷續膏、天山雪蓮各一 */
  if (d.pill && d.medicine && d.burger && d.ginseng && d.paste && d.snowgrass)
  {
    d.pill--;
    d.medicine--;
    d.burger--;
    d.ginseng--;
    d.paste--;
    d.snowgrass--;
    d.happy = 100;
    /* 助人為快樂之本 */
    vmsg("\xA7\x55\xA4\x48\xAC\xB0\xA7\xD6\xBC\xD6\xA4\xA7\xA5\xBB");
    return 1;
  }

  return 0;
}


  /*-----------------------------------------------------*/
  /* pip_quest_101~199 屬性達成的任務			 */
  /*-----------------------------------------------------*/


static int
pip_quest_101()		/* 完全健康 */
{
  /* 達到不疲累、不生病、不骯髒 */
  if (d.tired == 0 && d.sick == 0 && d.shit == 0)
  {
    d.food++;
    d.cookie++;
    /* 真棒，女神賜您一些食物 */
    vmsg("\xAF\x75\xB4\xCE\xA1\x41\xA4\x6B\xAF\xAB\xBD\xE7\xB1\x7A\xA4\x40\xA8\xC7\xAD\xB9\xAA\xAB");
    return 1;
  }

  return 0;
}


  /*-----------------------------------------------------*/
  /* pip_quest_201~299 打敗怪物的任務			 */
  /*-----------------------------------------------------*/


static int
pip_quest_201()		/* 打倒病毒 */
{
  /* 打倒一隻能力比自己高的怪物 */

  int level;
  playrule m;

  /* 變種病毒 */
  strcpy(m.name, "\xC5\xDC\xBA\xD8\xAF\x66\xAC\x72");  
  level = d.level + 5;
  m.hp = m.maxhp = 100 + level * level;
  m.attack = m.spirit = m.magic = m.armor = m.dodge = level * 15;
  m.money = 0;
  m.exp = 0;
  m.attribute = +7;		/* 專長: 究極法術 */ 
  m.pic = 004;

  if (pip_vs_man(m, 0))		/* 小雞對戰敵人 (借用武術大會) */
  {
    d.hexp += d.level;
    d.mexp += d.level;
    /* 您打敗了這隻變種病毒，評價提升 */
    vmsg("\xB1\x7A\xA5\xB4\xB1\xD1\xA4\x46\xB3\x6F\xB0\xA6\xC5\xDC\xBA\xD8\xAF\x66\xAC\x72\xA1\x41\xB5\xFB\xBB\xF9\xB4\xA3\xA4\xC9");
    return 1;
  }

  return 0;
}


static int
pip_quest_202()		/* 十大惡人 */
{
  /* 依序打倒數隻怪物 */
  /* 若打敗十大惡人，名聲提高；反之，練成嫁衣神功 */

  int i;
  struct playrule badmanlist[] =	/* 十大惡人相當於等級 40 ~ 50 的怪物 */
  {
    /* name[13] attribute hp maxhp attack spirit magic armor dodge money exp pic */
    /* 李大嘴 */
    /* 愛用吸精 */	"\xA7\xF5\xA4\x6A\xBC\x4C", +3, 1875, 1875, 500, 460, 350, 500, 500, 0, 220, 001,
    /* 陰九幽 */
    /* 愛用 blitz */	"\xB3\xB1\xA4\x45\xAB\xD5", +2, 1850, 1850, 520, 470, 320, 420, 630, 0, 220, 002,
    /* 哈哈兒 */
    /* 愛用暗器 */	"\xAB\xA2\xAB\xA2\xA8\xE0", +7, 1750, 1750, 450, 450, 370, 400, 520, 0, 220, 003,
    /* 屠嬌嬌 */
    /* 愛用炎系魔法 */	"\xB1\x4F\xBC\x62\xBC\x62", -4, 1635, 1635, 430, 340, 640, 360, 470, 0, 220, 004,
    /* 杜  殺 */
    /* 攻擊力特強 */	"\xA7\xF9  \xB1\xFE",  0, 2400, 2400, 610, 570, 280, 550, 500, 0, 250, 005,
  };

  /* 十大惡人看起來超強，您要找幫手嗎(Y/N)？[Y]  */
  if (ians(b_lines - 1, 0, "\xA4\x51\xA4\x6A\xB4\x63\xA4\x48\xAC\xDD\xB0\x5F\xA8\xD3\xB6\x57\xB1\x6A\xA1\x41\xB1\x7A\xAD\x6E\xA7\xE4\xC0\xB0\xA4\xE2\xB6\xDC(Y/N)\xA1\x48[Y] ") != 'n')
  {
    /* 請誰當幫手？(1)燕東天 (2)燕西天 (3)燕南天 (4)燕北天  */
    if (ians(b_lines - 1, 0, "\xBD\xD0\xBD\xD6\xB7\xED\xC0\xB0\xA4\xE2\xA1\x48(1)\xBF\x50\xAA\x46\xA4\xD1 (2)\xBF\x50\xA6\xE8\xA4\xD1 (3)\xBF\x50\xAB\x6E\xA4\xD1 (4)\xBF\x50\xA5\x5F\xA4\xD1 ") == '3')
    {
      /* itoc.050320: 若等級太低時接到這個任務會打不贏，所以要提供賤招 :p */
      /* 在大俠燕南天的幫助之下，您成功地除去十大惡人 */
      vmsg("\xA6\x62\xA4\x6A\xAB\x4C\xBF\x50\xAB\x6E\xA4\xD1\xAA\xBA\xC0\xB0\xA7\x55\xA4\xA7\xA4\x55\xA1\x41\xB1\x7A\xA6\xA8\xA5\x5C\xA6\x61\xB0\xA3\xA5\x68\xA4\x51\xA4\x6A\xB4\x63\xA4\x48");
      return 1;
    }
    else
    {
      /* 他拒絕了您的請求，看來您只好自己上了 */
      vmsg("\xA5\x4C\xA9\xDA\xB5\xB4\xA4\x46\xB1\x7A\xAA\xBA\xBD\xD0\xA8\x44\xA1\x41\xAC\xDD\xA8\xD3\xB1\x7A\xA5\x75\xA6\x6E\xA6\xDB\xA4\x76\xA4\x57\xA4\x46");
    }
  }

  for (i = 0; i< 5; i++)
  {
    if (!pip_vs_man(badmanlist[i], 0))	/* 小雞對戰敵人 (借用武術大會) */
    {
      /* 您被十大惡人圍攻，全身精脈俱斷 */
      vmsg("\xB1\x7A\xB3\x51\xA4\x51\xA4\x6A\xB4\x63\xA4\x48\xB3\xF2\xA7\xF0\xA1\x41\xA5\xFE\xA8\xAD\xBA\xEB\xAF\xDF\xAD\xD1\xC2\x5F");
      d.hp = 1;
      d.mp = 0;
      d.vp = 0;
      d.sp = 0;

      /* 嫁衣神功: 拿 maxmp 去換 maxsp */
      i = rand() % 10;
      if (d.maxmp > i)
      {
        d.maxmp -= i;
        d.maxsp += i;
        /* 在神醫萬春流的治療之下，您反練成嫁衣神功 */
        vmsg("\xA6\x62\xAF\xAB\xC2\xE5\xB8\x55\xAC\x4B\xAC\x79\xAA\xBA\xAA\x76\xC0\xF8\xA4\xA7\xA4\x55\xA1\x41\xB1\x7A\xA4\xCF\xBD\x6D\xA6\xA8\xB6\xF9\xA6\xE7\xAF\xAB\xA5\x5C");
      }

      return 0;
    }
  }

  d.hexp += 100;
  d.mexp += 100;
  /* 您成功地除去惡人谷的所有壞蛋，名聲大幅提升 */
  vmsg("\xB1\x7A\xA6\xA8\xA5\x5C\xA6\x61\xB0\xA3\xA5\x68\xB4\x63\xA4\x48\xA8\xA6\xAA\xBA\xA9\xD2\xA6\xB3\xC3\x61\xB3\x4A\xA1\x41\xA6\x57\xC1\x6E\xA4\x6A\xB4\x54\xB4\xA3\xA4\xC9");
  return 1;
}


  /*-----------------------------------------------------*/
  /* pip_quest_301~399 數學計算的任務			 */
  /*-----------------------------------------------------*/


static int
pip_quest_301()		/* 分配珠寶 */
{
  char ans[3];

  /* 我應該可以分到幾個珠寶呢？ */
  vget(b_lines, 0, "\xA7\xDA\xC0\xB3\xB8\xD3\xA5\x69\xA5\x48\xA4\xC0\xA8\xEC\xB4\x58\xAD\xD3\xAF\x5D\xC4\x5F\xA9\x4F\xA1\x48", ans, 3, DOECHO);
  if (atoi(ans) == 2)
  {
    d.social += 10;
    /* 讓我想想看啊！噫，沒錯，您實在太聰明了 */
    vmsg("\xC5\xFD\xA7\xDA\xB7\x51\xB7\x51\xAC\xDD\xB0\xDA\xA1\x49\xBE\xB3\xA1\x41\xA8\x53\xBF\xF9\xA1\x41\xB1\x7A\xB9\xEA\xA6\x62\xA4\xD3\xC1\x6F\xA9\xFA\xA4\x46");
    return 1;
  }

  return 0;
}


static int
pip_quest_302()		/* 0.9999... 循環小數 */
{
  int num;
  char ans1[4], ans2[4];

  /* 循環小數 0.9999... 化為分數，分母是  */
  if (vget(b_lines, 0, "\xB4\x60\xC0\xF4\xA4\x70\xBC\xC6 0.9999... \xA4\xC6\xAC\xB0\xA4\xC0\xBC\xC6\xA1\x41\xA4\xC0\xA5\xC0\xAC\x4F ", ans1, 4, DOECHO) &&
    /* 循環小數 0.9999... 化為分數，分子是  */
    vget(b_lines, 0, "\xB4\x60\xC0\xF4\xA4\x70\xBC\xC6 0.9999... \xA4\xC6\xAC\xB0\xA4\xC0\xBC\xC6\xA1\x41\xA4\xC0\xA4\x6C\xAC\x4F ", ans2, 4, DOECHO))
  {
    if ((num = atoi(ans1)) && (num == atoi(ans2)))	/* 分母不能為 0 */
    {
      d.wisdom += 10;
      /* 沒錯，0.9999... 就是 1 */
      vmsg("\xA8\x53\xBF\xF9\xA1\x41""0.9999... \xB4\x4E\xAC\x4F 1");
      return 1;
    } 
  }

  return 0;
}


  /*-----------------------------------------------------*/
  /* pip_quest_401~499 格言欣賞的任務			 */
  /*-----------------------------------------------------*/


static int
pip_quest_401()		/* 態度百分百 */
{
  /* 1)知識 2)努力 3)態度  */
  if (ians(b_lines - 1, 0, "1)\xAA\xBE\xC3\xD1 2)\xA7\x56\xA4\x4F 3)\xBA\x41\xAB\xD7 ") == '3')
  {
    d.affect += 5;
    d.toman += 5;
    /* 是的，唯有百分百的態度才能獲得眾人的尊敬 */
    vmsg("\xAC\x4F\xAA\xBA\xA1\x41\xB0\xDF\xA6\xB3\xA6\xCA\xA4\xC0\xA6\xCA\xAA\xBA\xBA\x41\xAB\xD7\xA4\x7E\xAF\xE0\xC0\xF2\xB1\x6F\xB2\xB3\xA4\x48\xAA\xBA\xB4\x4C\xB7\x71");
    return 1;
  }

  return 0;
}


  /*-----------------------------------------------------*/
  /* quest_cb[] 任務列表			 	 */
  /*-----------------------------------------------------*/


#define PIPQUEST_NUM	9


/* static KeyFunc quest_cb[] = */
static KeyFunc quest_cb[PIPQUEST_NUM + 1] =	/* 把 PIPQUEST_NUM 指定進去，如果有錯，可以在 compile 中看出 */
{
  /* 尋找物品 */
  1, pip_quest_1,
  2, pip_quest_2,
  3, pip_quest_3,

  /* 屬性達成 */
  101, pip_quest_101,

  /* 打敗怪物 */
  201, pip_quest_201,
  202, pip_quest_202,

  /* 數學問題 */
  301, pip_quest_301,
  302, pip_quest_302,

  /* 格言欣賞 */
  401, pip_quest_401,
  
  0, NULL		/* 結束任務列表 */
};
    

/*-------------------------------------------------------*/
/* 任務函式                                              */
/*-------------------------------------------------------*/


static int		/* 1:查詢任務  0:沒有任務 */
pip_quest_query(quest)	/* 查詢舊任務 */
  int quest;		/* 任務編號 */
{
  if (!quest && !(quest = d.quest))
  {
    /* 您目前沒有任務在身 */
    vmsg("\xB1\x7A\xA5\xD8\xAB\x65\xA8\x53\xA6\xB3\xA5\xF4\xB0\xC8\xA6\x62\xA8\xAD");
    return 0;
  }
  show_quest_pic(quest);
  return 1;
}


int			/* 1:取得新任務  0:取消取得或已有任務 */
pip_quest_new()		/* 取得新任務 */
{
  /* 您已達升級標準，願意接受長老指派任務嗎(Y/N)？[N]  */
  if (ians(b_lines - 1, 0, "\xB1\x7A\xA4\x77\xB9\x46\xA4\xC9\xAF\xC5\xBC\xD0\xB7\xC7\xA1\x41\xC4\x40\xB7\x4E\xB1\xB5\xA8\xFC\xAA\xF8\xA6\xD1\xAB\xFC\xAC\xA3\xA5\xF4\xB0\xC8\xB6\xDC(Y/N)\xA1\x48[N] ") == 'y')
  {
    d.quest = quest_cb[rand() % PIPQUEST_NUM].key;
    pip_quest_query(d.quest);
    /* 去吧，執行這個任務結束後我就賦予您更高的能力 */
    vmsg("\xA5\x68\xA7\x61\xA1\x41\xB0\xF5\xA6\xE6\xB3\x6F\xAD\xD3\xA5\xF4\xB0\xC8\xB5\xB2\xA7\xF4\xAB\xE1\xA7\xDA\xB4\x4E\xBD\xE1\xA4\xA9\xB1\x7A\xA7\xF3\xB0\xAA\xAA\xBA\xAF\xE0\xA4\x4F");
    return 1;
  }
  return 0;
}


static int		/* 1:任務完成  0:任務失敗 */
pip_quest_done()	/* 完成任務 */
{
  KeyFunc *cb;
  int key;

  if (!d.quest)
  {
    /* 您目前沒有任務在身 */
    vmsg("\xB1\x7A\xA5\xD8\xAB\x65\xA8\x53\xA6\xB3\xA5\xF4\xB0\xC8\xA6\x62\xA8\xAD");
    return 0;
  }

  /* itoc.註解: 是不是考慮換 binary search? */
  for (cb = quest_cb; (key = cb->key); cb++)
  {
    if (key == d.quest)
    {
      key = (*(cb->func)) ();	/* 1:完成 0:失敗 */
      pip_levelup(key);
      return key;
    }
  }

  /* 請告訴站長，找不到此任務的程式 */
  vmsg("\xBD\xD0\xA7\x69\xB6\x44\xAF\xB8\xAA\xF8\xA1\x41\xA7\xE4\xA4\xA3\xA8\xEC\xA6\xB9\xA5\xF4\xB0\xC8\xAA\xBA\xB5\x7B\xA6\xA1");	/* 應該不可能出現 */
  return 0;
}


static int		/* 1:放棄舊任務  0:取消放棄或沒有任務 */
pip_quest_abort()	/* 放棄舊任務 */
{
  if (!d.quest)
  {
    /* 您目前沒有任務在身 */
    vmsg("\xB1\x7A\xA5\xD8\xAB\x65\xA8\x53\xA6\xB3\xA5\xF4\xB0\xC8\xA6\x62\xA8\xAD");
  }
  /* 您確定要放棄現有的任務嗎(Y/N)？[N]  */
  else if (ians(b_lines - 1, 0, "\xB1\x7A\xBD\x54\xA9\x77\xAD\x6E\xA9\xF1\xB1\xF3\xB2\x7B\xA6\xB3\xAA\xBA\xA5\xF4\xB0\xC8\xB6\xDC(Y/N)\xA1\x48[N] ") == 'y')
  {  
    pip_levelup(0);
    return 1;
  }
  else
  {
    /* 還是不要放棄好了 */
    vmsg("\xC1\xD9\xAC\x4F\xA4\xA3\xAD\x6E\xA9\xF1\xB1\xF3\xA6\x6E\xA4\x46");
  }
  return 0;
}


/*-------------------------------------------------------*/
/* 任務主選單                                            */
/*-------------------------------------------------------*/


int
pip_quest_menu()
{
  while (1)
  {
    /*  任務  */
    /*  [1]完成 [2]查詢 [3]放棄 [Q]跳出                                        \033[m */
    out_cmd("", COLOR1 " \xA5\xF4\xB0\xC8 " COLOR2 " [1]\xA7\xB9\xA6\xA8 [2]\xAC\x64\xB8\xDF [3]\xA9\xF1\xB1\xF3 [Q]\xB8\xF5\xA5\x58                                        \033[m");

    switch (vkey())
    {
      case 'q':
        return 0;

      case '1':
	pip_quest_done();
	break;
	
      case '2':
	pip_quest_query(0);
	break;

      case '3':
	pip_quest_abort();
	break;
    }
  }
}
#endif	/* HAVE_GAME */
