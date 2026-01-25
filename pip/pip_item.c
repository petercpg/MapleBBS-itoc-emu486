/* ----------------------------------------------------- */
/* pip_item.c     ( NTHU CS MapleBBS Ver 3.10 )          */
/* ----------------------------------------------------- */
/* target : 小雞 item                                    */
/* create :   /  /                                       */
/* update : 01/08/14                                     */
/* author : dsyan.bbs@forever.twbbs.org                  */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */  
/* ----------------------------------------------------- */


#include "bbs.h"

#ifdef HAVE_GAME

#include "pip.h"


struct itemset pipfoodlist[] = 
{
  /*  name          msgbuy           msgfeed                        price */
  /* 物品名 */
  /* 購買須知 */
  /* 使用須知 */
  0, "\xAA\xAB\xAB\x7E\xA6\x57",     "\xC1\xCA\xB6\x52\xB6\xB7\xAA\xBE",      "\xA8\xCF\xA5\xCE\xB6\xB7\xAA\xBE",                        0, 
  /* 好吃的食物 */
  /* 體力恢復 50 */
  /* 每吃一次食物會恢復體力 50 喔 */
  1, "\xA6\x6E\xA6\x59\xAA\xBA\xAD\xB9\xAA\xAB", "\xC5\xE9\xA4\x4F\xAB\xEC\xB4\x5F 50",   "\xA8\x43\xA6\x59\xA4\x40\xA6\xB8\xAD\xB9\xAA\xAB\xB7\x7C\xAB\xEC\xB4\x5F\xC5\xE9\xA4\x4F 50 \xB3\xE1",   50, 
  /* 美味的零食 */
  /* 體力恢復 100 */
  /* 除了恢復體力，小雞也會更快樂 */
  2, "\xAC\xFC\xA8\xFD\xAA\xBA\xB9\x73\xAD\xB9", "\xC5\xE9\xA4\x4F\xAB\xEC\xB4\x5F 100",  "\xB0\xA3\xA4\x46\xAB\xEC\xB4\x5F\xC5\xE9\xA4\x4F\xA1\x41\xA4\x70\xC2\xFB\xA4\x5D\xB7\x7C\xA7\xF3\xA7\xD6\xBC\xD6",  120, 
  0, NULL, NULL, NULL, 0
};


struct itemset pipmedicinelist[] = 
{
  /*  name          msgbuy           msgfeed                        price */
  /* 物品名 */
  /* 購買須知 */
  /* 使用須知 */
  0, "\xAA\xAB\xAB\x7E\xA6\x57",     "\xC1\xCA\xB6\x52\xB6\xB7\xAA\xBE",      "\xA8\xCF\xA5\xCE\xB6\xB7\xAA\xBE",                        0, 
  /* 補血大還丹 */
  /* 體力恢復 1000 */
  /* 恢復大量流失體力的良方 */
  1, "\xB8\xC9\xA6\xE5\xA4\x6A\xC1\xD9\xA4\xA6", "\xC5\xE9\xA4\x4F\xAB\xEC\xB4\x5F 1000", "\xAB\xEC\xB4\x5F\xA4\x6A\xB6\x71\xAC\x79\xA5\xA2\xC5\xE9\xA4\x4F\xAA\xBA\xA8\x7D\xA4\xE8",       1000, 
  /* 珍貴的靈芝 */
  /* 法力恢復 1000 */
  /* 恢復大量流失法力的良方 */
  2, "\xAC\xC3\xB6\x51\xAA\xBA\xC6\x46\xAA\xDB", "\xAA\x6B\xA4\x4F\xAB\xEC\xB4\x5F 1000", "\xAB\xEC\xB4\x5F\xA4\x6A\xB6\x71\xAC\x79\xA5\xA2\xAA\x6B\xA4\x4F\xAA\xBA\xA8\x7D\xA4\xE8",       1000, 
  /* 好用大補丸 */
  /* 移動恢復 1000 */
  /* 恢復大量流失移動的良方 */
  3, "\xA6\x6E\xA5\xCE\xA4\x6A\xB8\xC9\xA4\x59", "\xB2\xBE\xB0\xCA\xAB\xEC\xB4\x5F 1000", "\xAB\xEC\xB4\x5F\xA4\x6A\xB6\x71\xAC\x79\xA5\xA2\xB2\xBE\xB0\xCA\xAA\xBA\xA8\x7D\xA4\xE8",       1000, 
  /* 千年人參王 */
  /* 內力恢復 1000 */
  /* 恢復大量流失內力的良方 */
  4, "\xA4\x64\xA6\x7E\xA4\x48\xB0\xD1\xA4\xFD", "\xA4\xBA\xA4\x4F\xAB\xEC\xB4\x5F 1000", "\xAB\xEC\xB4\x5F\xA4\x6A\xB6\x71\xAC\x79\xA5\xA2\xA4\xBA\xA4\x4F\xAA\xBA\xA8\x7D\xA4\xE8",       1000, 
  /* 黑玉斷續膏 */
  /* 體力完全恢復 */
  /* 傳說中能將所有受傷恢復的藥材 */
  5, "\xB6\xC2\xA5\xC9\xC2\x5F\xC4\xF2\xBB\x49", "\xC5\xE9\xA4\x4F\xA7\xB9\xA5\xFE\xAB\xEC\xB4\x5F",  "\xB6\xC7\xBB\xA1\xA4\xA4\xAF\xE0\xB1\x4E\xA9\xD2\xA6\xB3\xA8\xFC\xB6\xCB\xAB\xEC\xB4\x5F\xAA\xBA\xC3\xC4\xA7\xF7", 5000, 
  /* 天山雪蓮 */
  /* 狀態完全恢復 */
  /* 東北天山才有雪蓮子 */
  6, "\xA4\xD1\xA4\x73\xB3\xB7\xBD\xAC",   "\xAA\xAC\xBA\x41\xA7\xB9\xA5\xFE\xAB\xEC\xB4\x5F",  "\xAA\x46\xA5\x5F\xA4\xD1\xA4\x73\xA4\x7E\xA6\xB3\xB3\xB7\xBD\xAC\xA4\x6C",          10000, 
  0, NULL, NULL, NULL, 0
};


struct itemset pipotherlist[] = 
{
  /*  name          msgbuy           msgfeed                        price */
  /* 物品名 */
  /* 購買須知 */
  /* 使用須知 */
  0, "\xAA\xAB\xAB\x7E\xA6\x57",     "\xC1\xCA\xB6\x52\xB6\xB7\xAA\xBE",      "\xA8\xCF\xA5\xCE\xB6\xB7\xAA\xBE",                        0, 
  /* 百科全書 */
  /* 知識的來源 */
  /* 書本讓小雞更聰明更有氣質啦 */
  1, "\xA6\xCA\xAC\xEC\xA5\xFE\xAE\xD1",   "\xAA\xBE\xC3\xD1\xAA\xBA\xA8\xD3\xB7\xBD",    "\xAE\xD1\xA5\xBB\xC5\xFD\xA4\x70\xC2\xFB\xA7\xF3\xC1\x6F\xA9\xFA\xA7\xF3\xA6\xB3\xAE\xF0\xBD\xE8\xB0\xD5",   3000, 
  /* 樂高玩具組 */
  /* 快樂滿意度 */
  /* 玩具讓小雞更快樂啦 */
  2, "\xBC\xD6\xB0\xAA\xAA\xB1\xA8\xE3\xB2\xD5", "\xA7\xD6\xBC\xD6\xBA\xA1\xB7\x4E\xAB\xD7",    "\xAA\xB1\xA8\xE3\xC5\xFD\xA4\x70\xC2\xFB\xA7\xF3\xA7\xD6\xBC\xD6\xB0\xD5",            300, 
  /* 閣樓雜誌 */
  /* 滿足的快感 */
  /* 書中自有顏如玉啦 */
  3, "\xBB\xD5\xBC\xD3\xC2\xF8\xBB\x78",   "\xBA\xA1\xA8\xAC\xAA\xBA\xA7\xD6\xB7\x50",    "\xAE\xD1\xA4\xA4\xA6\xDB\xA6\xB3\xC3\x43\xA6\x70\xA5\xC9\xB0\xD5",              500, 
  0, NULL, NULL, NULL, 0
};


/* ------------------------------------------------------- */
/* 物品購買函式                                            */
/* ------------------------------------------------------- */


int
pip_buy_item(mode, p, oldnum)
  int mode;
  int oldnum[];
  struct itemset *p;
{
  /* 店名 */
  /* 便利商店 */
  /* 長春藥鋪 */
  /* 夜裡書局 */
  char *shopname[4] = {"\xA9\xB1\xA6\x57", "\xAB\x4B\xA7\x51\xB0\xD3\xA9\xB1", "\xAA\xF8\xAC\x4B\xC3\xC4\xBE\x51", "\xA9\x5D\xB8\xCC\xAE\xD1\xA7\xBD"};
  char buf[128], genbuf[20];
  int oldmoney;		/* 進商店前原有錢 */
  int total;		/* 購買/販賣個數 */
  int ch, choice;

  oldmoney = d.money;

  /* 秀出產品列表 */
  clrfromto(6, 18);
  /* \033[1;31m  ─\033[41;37m 編號\033[0;1;31m─\033[41;37m 商      品\033[0;1;31m──\033[41;37m 效            能\033[0;1;31m──\033[41;37m 價     格\033[0;1;31m─\033[37;41m 擁有數量\033[0;1;31m─\033[m\n\n */
  outs("\033[1;31m  \xA2\x77\033[41;37m \xBD\x73\xB8\xB9\033[0;1;31m\xA2\x77\033[41;37m \xB0\xD3      \xAB\x7E\033[0;1;31m\xA2\x77\xA2\x77\033[41;37m \xAE\xC4            \xAF\xE0\033[0;1;31m\xA2\x77\xA2\x77\033[41;37m \xBB\xF9     \xAE\xE6\033[0;1;31m\xA2\x77\033[37;41m \xBE\xD6\xA6\xB3\xBC\xC6\xB6\x71\033[0;1;31m\xA2\x77\033[m\n\n");
  for (ch = 1; ch <= oldnum[0]; ch++)
  {
    prints("    \033[1;35m[\033[37m%2d\033[35m]    \033[36m%-10s     \033[37m%-14s       \033[1;33m%-10d  \033[1;32m%-9d   \033[m\n",
      p[ch].num, p[ch].name, p[ch].msgbuy, p[ch].price, oldnum[ch]);
  }

  do
  {
    /*  採買  */
    /*  (%8s) [B]買入物品 [S]賣出物品 [Q]跳出                             \033[m */
    sprintf(buf, COLOR1 " \xB1\xC4\xB6\x52 " COLOR2 " (%8s) [B]\xB6\x52\xA4\x4A\xAA\xAB\xAB\x7E [S]\xBD\xE6\xA5\x58\xAA\xAB\xAB\x7E [Q]\xB8\xF5\xA5\x58                             \033[m", shopname[mode]);
    out_cmd("", buf);

    switch (ch = vkey())
    {
    case 'b':
      /* 想要買入啥呢？[0]放棄買入 [1～%d]物品商號： */
      sprintf(buf, "\xB7\x51\xAD\x6E\xB6\x52\xA4\x4A\xD4\xA3\xA9\x4F\xA1\x48[0]\xA9\xF1\xB1\xF3\xB6\x52\xA4\x4A [1\xA1\xE3%d]\xAA\xAB\xAB\x7E\xB0\xD3\xB8\xB9\xA1\x47", oldnum[0]);
      choice = ians(b_lines - 2, 0, buf) - '0';
      if (choice >= 1 && choice <= oldnum[0])
      {
	/* 您要買入物品 [%s] 多少個呢(1-%d)？[Q]  */
	sprintf(buf, "\xB1\x7A\xAD\x6E\xB6\x52\xA4\x4A\xAA\xAB\xAB\x7E [%s] \xA6\x68\xA4\xD6\xAD\xD3\xA9\x4F(1-%d)\xA1\x48[Q] ", p[choice].name, d.money / p[choice].price);
	vget(b_lines - 2, 0, buf, genbuf, 6, DOECHO);
	total = atoi(genbuf);

	if (total <= 0)
	{
	  /* 放棄買入... */
	  vmsg("\xA9\xF1\xB1\xF3\xB6\x52\xA4\x4A...");
	}
	else if (d.money < total * p[choice].price)
	{
	  /* 您的錢沒有那麼多喔.. */
	  vmsg("\xB1\x7A\xAA\xBA\xBF\xFA\xA8\x53\xA6\xB3\xA8\xBA\xBB\xF2\xA6\x68\xB3\xE1..");
	}
	else
	{
	  /* 確定買入總價為 %d 的物品 [%s] 數量 %d 個嗎(Y/N)？[N]  */
	  sprintf(buf, "\xBD\x54\xA9\x77\xB6\x52\xA4\x4A\xC1\x60\xBB\xF9\xAC\xB0 %d \xAA\xBA\xAA\xAB\xAB\x7E [%s] \xBC\xC6\xB6\x71 %d \xAD\xD3\xB6\xDC(Y/N)\xA1\x48[N] ", total * p[choice].price, p[choice].name, total);
	  if (ians(b_lines - 2, 0, buf) == 'y')
	  {
	    oldnum[choice] += total;
	    d.money -= total * p[choice].price;

	    /* itoc.010816: 更新擁有數量 */
	    move(7 + choice, 0);
	    prints("    \033[1;35m[\033[37m%2d\033[35m]    \033[36m%-10s     \033[37m%-14s       \033[1;33m%-10d  \033[1;32m%-9d   \033[m",
	      p[choice].num, p[choice].name, p[choice].msgbuy, p[choice].price, oldnum[choice]);

	    vmsg(p[choice].msguse);
	  }
	  else
	  {
	    /* 放棄買入... */
	    vmsg("\xA9\xF1\xB1\xF3\xB6\x52\xA4\x4A...");
	  }
	}
      }
      else
      {
	/* 放棄買入..... */
	sprintf(buf, "\xA9\xF1\xB1\xF3\xB6\x52\xA4\x4A.....");
	vmsg(buf);
      }
      break;

    case 's':
      /* 想要賣出啥呢？[0]放棄賣出 [1～%d]物品商號:  */
      sprintf(buf, "\xB7\x51\xAD\x6E\xBD\xE6\xA5\x58\xD4\xA3\xA9\x4F\xA1\x48[0]\xA9\xF1\xB1\xF3\xBD\xE6\xA5\x58 [1\xA1\xE3%d]\xAA\xAB\xAB\x7E\xB0\xD3\xB8\xB9: ", oldnum[0]);
      choice = ians(b_lines - 2, 0, buf) - '0';
      if (choice >= 1 && choice <= oldnum[0])
      {
	/* 您要賣出物品 [%s] 多少個呢(1-%d)？[Q]  */
	sprintf(buf, "\xB1\x7A\xAD\x6E\xBD\xE6\xA5\x58\xAA\xAB\xAB\x7E [%s] \xA6\x68\xA4\xD6\xAD\xD3\xA9\x4F(1-%d)\xA1\x48[Q] ", p[choice].name, oldnum[choice]);
	vget(b_lines - 2, 0, buf, genbuf, 6, DOECHO);
	total = atoi(genbuf);

	if (total <= 0)
	{
	  /* 放棄賣出... */
	  vmsg("\xA9\xF1\xB1\xF3\xBD\xE6\xA5\x58...");
	}
	else if (total > oldnum[choice])
	{
	  /* 您的 [%s] 沒有那麼多個喔 */
	  sprintf(buf, "\xB1\x7A\xAA\xBA [%s] \xA8\x53\xA6\xB3\xA8\xBA\xBB\xF2\xA6\x68\xAD\xD3\xB3\xE1", p[choice].name);
	  vmsg(buf);
	}
	else
	{
	  /* 確定賣出總價為 %d 的物品 [%s] 數量 %d 個嗎(Y/N)？[N]  */
	  sprintf(buf, "\xBD\x54\xA9\x77\xBD\xE6\xA5\x58\xC1\x60\xBB\xF9\xAC\xB0 %d \xAA\xBA\xAA\xAB\xAB\x7E [%s] \xBC\xC6\xB6\x71 %d \xAD\xD3\xB6\xDC(Y/N)\xA1\x48[N] ", total * p[choice].price * 4 / 5, p[choice].name, total);
	  if (ians(b_lines - 2, 0, buf) == 'y')
	  {
	    oldnum[choice] -= total;
	    d.money += total * p[choice].price * 8 / 10;

	    /* itoc.010816: 更新擁有數量 */
	    move(7 + choice, 0);
	    prints("    \033[1;35m[\033[37m%2d\033[35m]    \033[36m%-10s     \033[37m%-14s       \033[1;33m%-10d  \033[1;32m%-9d   \033[m",
	      p[choice].num, p[choice].name, p[choice].msgbuy, p[choice].price, oldnum[choice]);

	    /* 老闆拿走了您的 %d 個%s */
	    sprintf(buf, "\xA6\xD1\xC1\xF3\xAE\xB3\xA8\xAB\xA4\x46\xB1\x7A\xAA\xBA %d \xAD\xD3%s", total,  p[choice].name);
	    vmsg(buf);
	  }
	  else
	  {
	    /* 放棄賣出... */
	    vmsg("\xA9\xF1\xB1\xF3\xBD\xE6\xA5\x58...");
	  }
	}
      }
      else
      {
	/* 放棄賣出..... */
	sprintf(buf, "\xA9\xF1\xB1\xF3\xBD\xE6\xA5\x58.....");
	vmsg(buf);
      }
      break;

    case 'q':
    case KEY_LEFT:
      /* 金錢交易共 %d 元,離開 %s  */
      sprintf(buf, "\xAA\xF7\xBF\xFA\xA5\xE6\xA9\xF6\xA6\x40 %d \xA4\xB8,\xC2\xF7\xB6\x7D %s ", oldmoney - d.money, shopname[mode]);
      vmsg(buf);
      break;
    }

    /* itoc.010816: 消掉 ians() vget() 留下的殘骸 */
    move (b_lines - 2, 0);
    clrtoeol();

  } while (ch != 'q' && ch != KEY_LEFT);

  return 0;
}


/*-------------------------------------------------------*/
/* 商店選單:食物 零食 大補丸 玩具 書本			 */
/*-------------------------------------------------------*/

/*-------------------------------------------------------*/
/* 函式庫                      				 */
/*-------------------------------------------------------*/

int
pip_store_food()
{
  int num[3];
  num[0] = 2;
  num[1] = d.food;
  num[2] = d.cookie;
  pip_buy_item(1, pipfoodlist, num);
  d.food = num[1];
  d.cookie = num[2];
  return 0;
}


int
pip_store_medicine()
{
  int num[7];
  num[0] = 6;
  num[1] = d.pill;  
  num[2] = d.medicine;
  num[3] = d.burger;
  num[4] = d.ginseng;
  num[5] = d.paste;
  num[6] = d.snowgrass;
  pip_buy_item(2, pipmedicinelist, num);
  d.pill = num[1];
  d.medicine = num[2];
  d.burger = num[3];
  d.ginseng = num[4];
  d.paste = num[5];
  d.snowgrass = num[6];
  return 0;
}


int
pip_store_other()
{
  int num[4];
  num[0] = 3;
  num[1] = d.book;
  num[2] = d.toy;
  num[3] = d.playboy;
  pip_buy_item(3, pipotherlist, num);
  d.book = num[1];
  d.toy = num[2];
  d.playboy = num[3];
  return 0;
}
#endif		/* HAVE_GAME */
