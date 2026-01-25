/*-------------------------------------------------------*/
/* pip_basic.c         ( NTHU CS MapleBBS Ver 3.10 )     */
/*-------------------------------------------------------*/
/* target : 基本選單                                     */
/* create :   /  /                                       */
/* update : 01/08/14                                     */
/* author : dsyan.bbs@forever.twbbs.org                  */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_GAME

#include "pip.h"


/*-------------------------------------------------------*/
/* 基本選單:餵食 清潔 親親 休息	換雞金			 */
/*-------------------------------------------------------*/


int				/* 1: 沒吃食物，放棄  0: 吃了 */
pip_basic_feed()		/* 餵食 */
{
  int ch;
  int nodone;			/* itoc.010731: 記錄是否有行動 */

  nodone = 1;

  do
  {
    /*  一般  */
    /*  [1]吃飯 [2]零食 [3]書本 [4]玩具 [5]讀物 [Q]跳出                        \033[m */
    out_cmd(COLOR1 " \xA4\x40\xAF\xEB " COLOR2 " [1]\xA6\x59\xB6\xBA [2]\xB9\x73\xAD\xB9 [3]\xAE\xD1\xA5\xBB [4]\xAA\xB1\xA8\xE3 [5]\xC5\xAA\xAA\xAB [Q]\xB8\xF5\xA5\x58                        \033[m", 
      /*  藥品  */
      /*  [a]大還 [b]靈芝 [c]補丸 [d]人蔘 [e]黑玉 [f]雪蓮 [Q]跳出                \033[m */
      COLOR1 " \xC3\xC4\xAB\x7E " COLOR2 " [a]\xA4\x6A\xC1\xD9 [b]\xC6\x46\xAA\xDB [c]\xB8\xC9\xA4\x59 [d]\xA4\x48\xE7\x78 [e]\xB6\xC2\xA5\xC9 [f]\xB3\xB7\xBD\xAC [Q]\xB8\xF5\xA5\x58                \033[m");

    switch (ch = vkey())
    {
    case '1':		/* 吃飯 */
      if (d.food <= 0)
      {
	/* 沒有食物囉..快去買吧！ */
	vmsg("\xA8\x53\xA6\xB3\xAD\xB9\xAA\xAB\xC5\x6F..\xA7\xD6\xA5\x68\xB6\x52\xA7\x61\xA1\x49");
	break;
      }
      d.food--;
      d.hp += 50;
      if (d.hp > d.maxhp)
      {
	d.hp = d.maxhp;
	d.weight += rand() % 2;
      }
      nodone = 0;
      if ((d.bbtime / 60 / 30) < 3)		/* 未滿三歲 */
	show_feed_pic(11);
      else
	show_feed_pic(12);
      /* 每吃一次食物會恢復體力50喔! */
      vmsg("\xA8\x43\xA6\x59\xA4\x40\xA6\xB8\xAD\xB9\xAA\xAB\xB7\x7C\xAB\xEC\xB4\x5F\xC5\xE9\xA4\x4F""50\xB3\xE1!");
      break;

    case '2':		/* 零食 */
      if (d.cookie <= 0)
      {
	/* 零食吃光囉..快去買吧！ */
	vmsg("\xB9\x73\xAD\xB9\xA6\x59\xA5\xFA\xC5\x6F..\xA7\xD6\xA5\x68\xB6\x52\xA7\x61\xA1\x49");
	break;
      }
      d.cookie--;
      d.hp += 100;
      if (d.hp > d.maxhp)
      {
	d.hp = d.maxhp;
	d.weight += rand() % 2 + 2;
      }
      else
      {
	d.weight += (rand() % 2 + 1);
      }
      d.happy += (rand() % 3 + 4);
      d.satisfy += rand() % 3 + 2;
      nodone = 0;
      if (rand() % 2)
	show_feed_pic(21);
      else
	show_feed_pic(22);
      /* 吃零食容易胖喔... */
      vmsg("\xA6\x59\xB9\x73\xAD\xB9\xAE\x65\xA9\xF6\xAD\x44\xB3\xE1...");
      break;

    case '3':		/* 書本 */
      if (d.book <= 0)
      {
	/* 沒有書本囉..快去買吧！ */
	vmsg("\xA8\x53\xA6\xB3\xAE\xD1\xA5\xBB\xC5\x6F..\xA7\xD6\xA5\x68\xB6\x52\xA7\x61\xA1\x49");
	break;
      }
      d.book--;
      d.happy -= 5;
      d.wisdom+= 20;
      d.art += 20;
      nodone = 0;
      show_feed_pic(31);
      /* 開卷有益 */
      vmsg("\xB6\x7D\xA8\xF7\xA6\xB3\xAF\x71");
      break;

    case '4':		/* 玩具 */
      if (d.toy <= 0)
      {
	/* 沒有玩具囉..快去買吧！ */
	vmsg("\xA8\x53\xA6\xB3\xAA\xB1\xA8\xE3\xC5\x6F..\xA7\xD6\xA5\x68\xB6\x52\xA7\x61\xA1\x49");
	break;
      }
      d.toy--;
      d.happy += 20;
      d.satisfy += 20;
      nodone = 0;
      show_feed_pic(41);
      /* 玩玩具的小孩不會變壞 */
      vmsg("\xAA\xB1\xAA\xB1\xA8\xE3\xAA\xBA\xA4\x70\xAB\xC4\xA4\xA3\xB7\x7C\xC5\xDC\xC3\x61");
      break;

    case '5':		/* 讀物 */
      if (d.playboy <= 0)
      {
	/* 沒有課外讀物囉..快去買吧！ */
	vmsg("\xA8\x53\xA6\xB3\xBD\xD2\xA5\x7E\xC5\xAA\xAA\xAB\xC5\x6F..\xA7\xD6\xA5\x68\xB6\x52\xA7\x61\xA1\x49");
	break;
      }
      if ((d.bbtime / 60 / 30) < 5)
      {
        /* itoc.010801: 五歲以後才能看 :p */
        /* 封面上寫著 5 禁 Ｘ */
        vmsg("\xAB\xCA\xAD\xB1\xA4\x57\xBC\x67\xB5\xDB 5 \xB8\x54 \xA2\xE6");
        break;
      }
      d.playboy--;
      /* itoc.010801: 增加罪惡，但快樂/滿意大量上升 */
      d.happy = 100;
      d.satisfy = 100;
      d.art += 5;
      d.sin += 30;
      nodone = 0;
      show_feed_pic(51);
      /* 呼呼，還好沒人看見 */
      vmsg("\xA9\x49\xA9\x49\xA1\x41\xC1\xD9\xA6\x6E\xA8\x53\xA4\x48\xAC\xDD\xA8\xA3");
      break;

    case 'a':		/* 大還 */
      if (d.pill <= 0)
      {
	/* 沒有大還丹囉..快去買吧！ */
	vmsg("\xA8\x53\xA6\xB3\xA4\x6A\xC1\xD9\xA4\xA6\xC5\x6F..\xA7\xD6\xA5\x68\xB6\x52\xA7\x61\xA1\x49");
	break;
      }
      d.pill--;
      d.hp += 1000;
      if (d.hp > d.maxhp)
	d.hp = d.maxhp;
      nodone = 0;
      show_feed_pic(61);
      /* 每吃一次大還丹會恢復體力 1000 喔! */
      vmsg("\xA8\x43\xA6\x59\xA4\x40\xA6\xB8\xA4\x6A\xC1\xD9\xA4\xA6\xB7\x7C\xAB\xEC\xB4\x5F\xC5\xE9\xA4\x4F 1000 \xB3\xE1!");
      break;

    case 'b':
      if (d.medicine <= 0)
      {
	/* 沒有靈芝囉..快去買吧！ */
	vmsg("\xA8\x53\xA6\xB3\xC6\x46\xAA\xDB\xC5\x6F..\xA7\xD6\xA5\x68\xB6\x52\xA7\x61\xA1\x49");
	break;
      }
      d.medicine--;
      d.mp += 1000;
      if (d.mp > d.maxmp)
	d.mp = d.maxmp;
      nodone = 0;
      show_feed_pic(71);
      /* 每吃一次靈芝會恢復法力 1000 喔! */
      vmsg("\xA8\x43\xA6\x59\xA4\x40\xA6\xB8\xC6\x46\xAA\xDB\xB7\x7C\xAB\xEC\xB4\x5F\xAA\x6B\xA4\x4F 1000 \xB3\xE1!");
      break;

    case 'c':		/* 補丸 */
      if (d.burger <= 0)
      {
	/* 沒有大補丸了耶! 快去買吧.. */
	vmsg("\xA8\x53\xA6\xB3\xA4\x6A\xB8\xC9\xA4\x59\xA4\x46\xAD\x43! \xA7\xD6\xA5\x68\xB6\x52\xA7\x61..");
	break;
      }
      d.burger--;
      d.vp += 1000;
      if (d.vp > d.maxvp)
	d.vp = d.maxvp;
      nodone = 0;
      show_feed_pic(81);
      /* 每吃一次補丸會恢復移動 1000 喔! */
      vmsg("\xA8\x43\xA6\x59\xA4\x40\xA6\xB8\xB8\xC9\xA4\x59\xB7\x7C\xAB\xEC\xB4\x5F\xB2\xBE\xB0\xCA 1000 \xB3\xE1!");
      break;

    case 'd':		/* 人蔘 */
      if (d.ginseng <= 0)
      {
	/* 沒有千年人蔘耶! 快去買吧.. */
	vmsg("\xA8\x53\xA6\xB3\xA4\x64\xA6\x7E\xA4\x48\xE7\x78\xAD\x43! \xA7\xD6\xA5\x68\xB6\x52\xA7\x61..");
	break;
      }
      d.ginseng--;
      d.sp += 1000;
      if (d.sp > d.maxsp)
        d.sp = d.maxsp;
      nodone = 0;
      show_feed_pic(91);
      /* 每吃一次人蔘會恢復內力 1000 喔! */
      vmsg("\xA8\x43\xA6\x59\xA4\x40\xA6\xB8\xA4\x48\xE7\x78\xB7\x7C\xAB\xEC\xB4\x5F\xA4\xBA\xA4\x4F 1000 \xB3\xE1!");
      break;

    case 'e':		/* 黑玉 */
      if (d.paste <= 0)
      {
	/* 沒有黑玉斷續膏耶! 快去買吧.. */
	vmsg("\xA8\x53\xA6\xB3\xB6\xC2\xA5\xC9\xC2\x5F\xC4\xF2\xBB\x49\xAD\x43! \xA7\xD6\xA5\x68\xB6\x52\xA7\x61..");
	break;
      }
      d.snowgrass--;
      d.hp = d.maxhp;
      d.tired = 0;
      d.sick = 0;
      nodone = 0;
      show_feed_pic(101);
      /* 黑玉斷續膏..超極棒的唷... */
      vmsg("\xB6\xC2\xA5\xC9\xC2\x5F\xC4\xF2\xBB\x49..\xB6\x57\xB7\xA5\xB4\xCE\xAA\xBA\xAD\xF2...");
      break;

    case 'f':		/* 雪蓮 */
      if (d.snowgrass <= 0)
      {
	/* 沒有天山雪蓮耶! 快去買吧.. */
	vmsg("\xA8\x53\xA6\xB3\xA4\xD1\xA4\x73\xB3\xB7\xBD\xAC\xAD\x43! \xA7\xD6\xA5\x68\xB6\x52\xA7\x61..");
	break;
      }
      d.snowgrass--;
      d.hp = d.maxhp;
      d.mp = d.maxmp;
      d.vp = d.maxvp;
      d.sp = d.maxsp;
      d.tired = 0;
      d.sick = 0;
      nodone = 0;
      show_feed_pic(111);
      /* 天山雪蓮..超極棒的唷... */
      vmsg("\xA4\xD1\xA4\x73\xB3\xB7\xBD\xAC..\xB6\x57\xB7\xA5\xB4\xCE\xAA\xBA\xAD\xF2...");
      break;
    }
  } while (ch != 'q' && ch != KEY_LEFT);

  return nodone;
}


int
pip_basic_takeshower()		/* 洗澡 */
{
  d.shit -= 20;
  if (d.shit < 0)
    d.shit = 0;

  d.hp -= rand() % 2 + 3;

  switch(rand() % 3)
  {
  case 0:
    show_usual_pic(1);
    /* 我是乾淨的小雞  cccc.... */
    vmsg("\xA7\xDA\xAC\x4F\xB0\xAE\xB2\x62\xAA\xBA\xA4\x70\xC2\xFB  cccc....");
    break;

  case 1:
    show_usual_pic(7);
    /* 馬桶 嗯～～ */
    vmsg("\xB0\xA8\xB1\xED \xB6\xE2\xA1\xE3\xA1\xE3");
    break;

  case 2: 
    show_usual_pic(2);
    /* 我愛洗澡 lalala.... */
    vmsg("\xA7\xDA\xB7\x52\xAC\x7E\xBE\xFE lalala....");
    break;
  }
  return 0;
}


int
pip_basic_takerest()		/* 休息 */
{
  count_tired(5, 20, 1, 100, 0);	/* 恢復疲勞 */
  d.shit++;
  d.hp += d.maxhp / 10;
  if (d.hp > d.maxhp)
    d.hp = d.maxhp;

  show_usual_pic(5);
  /* 再按一下我就起床囉.... */
  vmsg("\xA6\x41\xAB\xF6\xA4\x40\xA4\x55\xA7\xDA\xB4\x4E\xB0\x5F\xA7\xC9\xC5\x6F....");
  show_usual_pic(6);
  /* 喂喂喂..該起床囉...... */
  vmsg("\xB3\xDE\xB3\xDE\xB3\xDE..\xB8\xD3\xB0\x5F\xA7\xC9\xC5\x6F......");
  return 0;
}


int
pip_basic_kiss()		/* 親親 */
{
  if (rand() % 2)
  {
    d.happy += rand() % 3 + 4;
    d.satisfy += rand() % 2 + 1;
  }
  else
  {
    d.happy += rand() % 2 + 1;
    d.satisfy += rand() % 3 + 4;
  }
  count_tired(1, 2, 0, 100, 1);
  d.shit += rand() % 5 + 4;
  d.relation += rand() % 2;

  show_usual_pic(3);

  if (d.shit < 60)
    /* 來嘛! 啵一個..... */
    vmsg("\xA8\xD3\xB9\xC0! \xD4\x71\xA4\x40\xAD\xD3.....");
  else
    /* 親太多也是會髒死的喔.... */
    vmsg("\xBF\xCB\xA4\xD3\xA6\x68\xA4\x5D\xAC\x4F\xB7\x7C\xC5\xBC\xA6\xBA\xAA\xBA\xB3\xE1....");

  return 0;
}


int
pip_money()
{
  int money;
  char buf[80];

  if (HAS_STATUS(STATUS_COINLOCK))
  {
    vmsg(msg_coinlock);
    return 0;
  }

  /* itoc.註解: 之所以不提供小雞幣換銀幣的原因是因為小雞可以儲存/讀取進度 */

  clrfromto(6, 18);
  /* 您身上有 %d 銀幣,雞金 %d 元\n */
  prints("\xB1\x7A\xA8\xAD\xA4\x57\xA6\xB3 %d \xBB\xC8\xB9\xF4,\xC2\xFB\xAA\xF7 %d \xA4\xB8\n", cuser.money, d.money);
  /* \n一銀幣換一雞金唷！\n */
  outs("\n\xA4\x40\xBB\xC8\xB9\xF4\xB4\xAB\xA4\x40\xC2\xFB\xAA\xF7\xAD\xF2\xA1\x49\n");

  do
  {
    /* 要換多少銀幣？[Q]  */
    if (!vget(10, 0, "\xAD\x6E\xB4\xAB\xA6\x68\xA4\xD6\xBB\xC8\xB9\xF4\xA1\x48[Q] ", buf, 10, DOECHO))
      return 0;
    money = atol(buf);
  } while (money <= 0 || money > cuser.money);

  /* 是否要轉換 %d 銀幣 為 %d 雞金(Y/N)？[N]  */
  sprintf(buf, "\xAC\x4F\xA7\x5F\xAD\x6E\xC2\xE0\xB4\xAB %d \xBB\xC8\xB9\xF4 \xAC\xB0 %d \xC2\xFB\xAA\xF7(Y/N)\xA1\x48[N] ", money, money);
  if (ians(11, 0, buf) == 'y')
  {
    cuser.money -= money;
    d.money += money;
    /* 您身上有 %d 次銀幣,雞金 %d 元 */
    sprintf(buf, "\xB1\x7A\xA8\xAD\xA4\x57\xA6\xB3 %d \xA6\xB8\xBB\xC8\xB9\xF4,\xC2\xFB\xAA\xF7 %d \xA4\xB8", cuser.money, d.money);
    vmsg(buf);
    return 1;
  }
  /* 取消.... */
  vmsg("\xA8\xFA\xAE\xF8....");
  return 0;
}
#endif		/* HAVE_GAME */
