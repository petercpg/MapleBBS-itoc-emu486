/*-------------------------------------------------------*/
/* pip/pip_stuff.c      ( NTHU CS MapleBBS Ver 3.10 )    */
/*-------------------------------------------------------*/
/* target : 升級、系統、特殊選單等雜七雜八函式           */
/* create :   /  /                                       */
/* update : 01/08/14                                     */
/* author : dsyan.bbs@forever.twbbs.org                  */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_GAME

#include "pip.h"


/*-------------------------------------------------------*/
/* 資料存取						 */
/*-------------------------------------------------------*/


void
pip_write_file()		/* 遊戲寫資料入檔案 */
{
  int fd;
  char fpath[64];

  usr_fpath(fpath, cuser.userid, fn_pip);
  fd = open(fpath, O_WRONLY | O_CREAT, 0600);	/* fpath 不必已經存在 */
  write(fd, &d, sizeof(CHICKEN));
  close(fd);
}


int		/* >=0:成功  <0:失敗 */
pip_read_file(userid, p)	/* 遊戲讀資料出檔案 */
  char *userid;
  struct CHICKEN *p;
{
  int fd;
  char fpath[64];

  usr_fpath(fpath, userid, fn_pip);
  fd = open(fpath, O_RDONLY);		/* fpath 必須已經存在 */
  if (fd >= 0)
  {
    read(fd, p, sizeof(CHICKEN));
    close(fd);
  }
  return fd;
}


int			/* 1: 成功寫入  0: 放棄 */
pip_write_backup()	/* 小雞進度備份 */
{
  /* 沒有 */
  /* 進度一 */
  /* 進度二 */
  /* 進度三 */
  char *files[4] = {"\xA8\x53\xA6\xB3", "\xB6\x69\xAB\xD7\xA4\x40", "\xB6\x69\xAB\xD7\xA4\x47", "\xB6\x69\xAB\xD7\xA4\x54"};
  char buf[80], fpath[64];
  int ch;

  show_basic_pic(101);

  /* 儲存 [1] 進度一 [2] 進度二 [3] 進度三 [Q] 放棄：[Q]  */
  ch = ians(b_lines - 2, 0, "\xC0\x78\xA6\x73 [1] \xB6\x69\xAB\xD7\xA4\x40 [2] \xB6\x69\xAB\xD7\xA4\x47 [3] \xB6\x69\xAB\xD7\xA4\x54 [Q] \xA9\xF1\xB1\xF3\xA1\x47[Q] ") - '0';

  if (ch < 1 || ch > 3)
  {
    /* 放棄儲存遊戲備份 */
    vmsg("\xA9\xF1\xB1\xF3\xC0\x78\xA6\x73\xB9\x43\xC0\xB8\xB3\xC6\xA5\xF7");
    return 0;
  }

  /* 確定要儲存於 [%s] 檔案嗎(Y/N)？[N]  */
  sprintf(buf, "\xBD\x54\xA9\x77\xAD\x6E\xC0\x78\xA6\x73\xA9\xF3 [%s] \xC0\xC9\xAE\xD7\xB6\xDC(Y/N)\xA1\x48[N] ", files[ch]);
  if (ians(b_lines - 2, 0, buf) != 'y')
  {
    /* 放棄儲存檔案 */
    vmsg("\xA9\xF1\xB1\xF3\xC0\x78\xA6\x73\xC0\xC9\xAE\xD7");
    return 0;
  }

  /* 儲存 [%s] 檔案完成了 */
  sprintf(buf, "\xC0\x78\xA6\x73 [%s] \xC0\xC9\xAE\xD7\xA7\xB9\xA6\xA8\xA4\x46", files[ch]);
  vmsg(buf);

  sprintf(buf, "%s.bak%d", fn_pip, ch);
  usr_fpath(fpath, cuser.userid, buf);
  ch = open(fpath, O_WRONLY | O_CREAT, 0600);	/* fpath 不必已經存在 */
  write(ch, &d, sizeof(CHICKEN));
  close(ch);

  return 1;
}


int			/* 1: 成功讀出  0: 放棄 */
pip_read_backup()	/* 小雞備份讀取 */
{
  /* 沒有 */
  /* 進度一 */
  /* 進度二 */
  /* 進度三 */
  char *files[4] = {"\xA8\x53\xA6\xB3", "\xB6\x69\xAB\xD7\xA4\x40", "\xB6\x69\xAB\xD7\xA4\x47", "\xB6\x69\xAB\xD7\xA4\x54"};
  char buf[80], fpath[64];
  int ch, fd;

  show_basic_pic(102);

  /* 讀取 [1] 進度一 [2] 進度二 [3] 進度三 [Q] 放棄：[Q]  */
  ch = ians(b_lines - 2, 0, "\xC5\xAA\xA8\xFA [1] \xB6\x69\xAB\xD7\xA4\x40 [2] \xB6\x69\xAB\xD7\xA4\x47 [3] \xB6\x69\xAB\xD7\xA4\x54 [Q] \xA9\xF1\xB1\xF3\xA1\x47[Q] ") - '0';

  if (ch < 1 || ch > 3)
  {
    /* 放棄讀取遊戲備份 */
    vmsg("\xA9\xF1\xB1\xF3\xC5\xAA\xA8\xFA\xB9\x43\xC0\xB8\xB3\xC6\xA5\xF7");
    return 0;
  }

  sprintf(buf, "%s.bak%d", fn_pip, ch);
  usr_fpath(fpath, cuser.userid, buf);

  fd = open(fpath, O_RDONLY);		/* fpath 必須已經存在 */
  if (fd >= 0)
  {
    /* 確定要讀取於 [%s] 檔案嗎(Y/N)？[N]  */
    sprintf(buf, "\xBD\x54\xA9\x77\xAD\x6E\xC5\xAA\xA8\xFA\xA9\xF3 [%s] \xC0\xC9\xAE\xD7\xB6\xDC(Y/N)\xA1\x48[N] ", files[ch]);
    if (ians(b_lines - 2, 0, buf) == 'y')
    {
      read(fd, &d, sizeof(CHICKEN));
      close(fd);
      /* 讀取 [%s] 檔案完成了 */
      sprintf(buf, "\xC5\xAA\xA8\xFA [%s] \xC0\xC9\xAE\xD7\xA7\xB9\xA6\xA8\xA4\x46", files[ch]);
      vmsg(buf);
      return 1;
    }
    /* 放棄讀取檔案 */
    vmsg("\xA9\xF1\xB1\xF3\xC5\xAA\xA8\xFA\xC0\xC9\xAE\xD7");
    close(fd);
    return 0;
  }
  else
  {
    /* 檔案 [%s] 不存在 */
    sprintf(buf, "\xC0\xC9\xAE\xD7 [%s] \xA4\xA3\xA6\x73\xA6\x62", files[ch]);
    vmsg(buf);
    return 0;
  }
}


/*-------------------------------------------------------*/
/* 小雞狀態函式					 	 */
/*-------------------------------------------------------*/


void
pipdie(msg, diemode)	/* 小雞死亡 */
  char *msg;
  int diemode;
{
  /* 電子養小雞 */
  vs_head("\xB9\x71\xA4\x6C\xBE\x69\xA4\x70\xC2\xFB", str_site);

  if (diemode == 1)
  {
    show_die_pic(1);
    /* 死神來帶走小雞了 */
    vmsg("\xA6\xBA\xAF\xAB\xA8\xD3\xB1\x61\xA8\xAB\xA4\x70\xC2\xFB\xA4\x46");
    /* 電子養小雞 */
    vs_head("\xB9\x71\xA4\x6C\xBE\x69\xA4\x70\xC2\xFB", str_site);
    show_die_pic(2);
    move(14, 20);
    /* 可憐的小雞\033[1;31m%s\033[m */
    prints("\xA5\x69\xBC\xA6\xAA\xBA\xA4\x70\xC2\xFB\033[1;31m%s\033[m", msg);
    /* 哀悼中.... */
    vmsg(BBSNAME "\xAB\x73\xB1\xA5\xA4\xA4....");
  }
  else if (diemode == 2)
  {
    show_die_pic(3);
    /* 嗚嗚嗚..我被丟棄了..... */
    vmsg("\xB6\xE3\xB6\xE3\xB6\xE3..\xA7\xDA\xB3\x51\xA5\xE1\xB1\xF3\xA4\x46.....");
  }
  else if (diemode == 3)
  {
    show_die_pic(0);
    /* 遊戲結束囉.. */
    vmsg("\xB9\x43\xC0\xB8\xB5\xB2\xA7\xF4\xC5\x6F..");
  }

  d.death = diemode;
  pip_write_file();
}


int
count_tired(prob, base, mode, mul, cal)		/* itoc.010803: 依照傳入的引數來增減疲勞度 */
  int prob;				/* 機率 */
  int base;				/* 底數 */
  int mode;				/* 類型 1:和年齡有關  0:和年齡無關  */
  int mul;				/* 加權 (以 % 來計 100->1) */
  int cal;				/* 1:加疲勞  0:減疲勞 */
{
  int tiredvary;       /* 改變值 */

  /* 先算改變量 */  
  tiredvary = rand() % prob + base;  

  if (mode)            /* 和年齡有關 */
  {
    int tm;            /* 年齡 */
    tm = d.bbtime / 60 / 30;

    /* itoc.010803: 年紀越小，加疲勞比較少，恢復疲勞也比較快 */
    /* 注意不能寫成 tiredvary *= 6 / 5; 喔 :p */

    if (tm <= 3)       /* 0~3 歲 */
    {
      tiredvary = cal ? tiredvary * 16 / 15 : tiredvary * 6 / 5;
    }
    else if (tm <= 7)  /* 4~7 歲 */
    {
      tiredvary = cal ? tiredvary * 11 / 10 : tiredvary * 8 / 7;
    }
    else if (tm <= 10) /* 8~10 歲 */
    {
      tiredvary = cal ? tiredvary * 8 / 7 : tiredvary * 11 / 10;
    }
    else               /* 11 歲以上 */
    {
      tiredvary = cal ? tiredvary * 6 / 5 : tiredvary * 16 / 15;
    }
  }

  /* 再算加權 */
  if (cal)
  {
    d.tired += tiredvary * mul / 100;
    if (d.tired > 100)
      d.tired = 100;
  }
  else
  {
    d.tired -= tiredvary;      /* 扣值不再加權了 */
    if (d.tired < 0)
      d.tired = 0;
  }
}


/*-------------------------------------------------------*/
/* 特殊選單:看病 減肥 				         */
/*-------------------------------------------------------*/


int				/* 1:看完醫生  0:沒病來醫院惡搞 */
pip_see_doctor()		/* 看醫生 */
{
  char buf[256];
  long savemoney;
  savemoney = d.sick * 25;
  if (d.sick <= 0)
  {
    /* 哇哩..沒病來醫院幹嘛..被罵了..嗚~~ */
    vmsg("\xAB\x7A\xAD\xF9..\xA8\x53\xAF\x66\xA8\xD3\xC2\xE5\xB0\x7C\xB7\x46\xB9\xC0..\xB3\x51\xBD\x7C\xA4\x46..\xB6\xE3~~");
    d.character -= rand() % 3 + 1;
    if (d.character < 0)
      d.character = 0;
    d.happy -= (rand() % 3 + 3);
    d.satisfy -= rand() % 3 + 2;
  }
  else if (d.money < savemoney)
  {
    /* 您的病要花 %d 元喔....您不夠錢啦... */
    sprintf(buf, "\xB1\x7A\xAA\xBA\xAF\x66\xAD\x6E\xAA\xE1 %d \xA4\xB8\xB3\xE1....\xB1\x7A\xA4\xA3\xB0\xF7\xBF\xFA\xB0\xD5...", savemoney);
    vmsg(buf);
  }
  else
  {
    d.tired -= rand() % 10 + 20;
    if (d.tired < 0)
      d.tired = 0;
    d.sick = 0;
    d.money = d.money - savemoney;
    move(4, 0);
    show_special_pic(1);
    /* 藥到病除..沒有副作用!! */
    vmsg("\xC3\xC4\xA8\xEC\xAF\x66\xB0\xA3..\xA8\x53\xA6\xB3\xB0\xC6\xA7\x40\xA5\xCE!!");
    return 1;
  }
  return 0;
}


int				/* 1:整容  0:沒有整容 */
pip_change_weight()		/* 增胖/減肥 */
{
  char buf[80];
  int weightmp;

  show_special_pic(2);

  /*  美容  */
  /*  [1]傳統增胖 [2]快速增胖 [3]傳統減肥 [4]快速減肥 [Q]跳出                \033[m */
  out_cmd("", COLOR1 " \xAC\xFC\xAE\x65 " COLOR2 " [1]\xB6\xC7\xB2\xCE\xBC\x57\xAD\x44 [2]\xA7\xD6\xB3\x74\xBC\x57\xAD\x44 [3]\xB6\xC7\xB2\xCE\xB4\xEE\xAA\xCE [4]\xA7\xD6\xB3\x74\xB4\xEE\xAA\xCE [Q]\xB8\xF5\xA5\x58                \033[m");

  switch (vkey())
  {
  case '1':
    if (d.money < 80)
    {
      /* 傳統增胖要80元喔....您不夠錢啦... */
      vmsg("\xB6\xC7\xB2\xCE\xBC\x57\xAD\x44\xAD\x6E""80\xA4\xB8\xB3\xE1....\xB1\x7A\xA4\xA3\xB0\xF7\xBF\xFA\xB0\xD5...");
    }
    else
    {
      /*  需花費 80 元（3～5公斤），確定嗎(Y/N)？[N]  */
      if (ians(b_lines - 1, 0, " \xBB\xDD\xAA\xE1\xB6\x4F 80 \xA4\xB8\xA1\x5D""3\xA1\xE3""5\xA4\xBD\xA4\xE7\xA1\x5E\xA1\x41\xBD\x54\xA9\x77\xB6\xDC(Y/N)\xA1\x48[N] ") == 'Y')
      {
	weightmp = 3 + rand() % 3;
	d.weight += weightmp;
	d.money -= 80;
	d.hp -= rand() % 2 + 3;
	show_special_pic(3);
	/* 總共增加了 %d 公斤 */
	sprintf(buf, "\xC1\x60\xA6\x40\xBC\x57\xA5\x5B\xA4\x46 %d \xA4\xBD\xA4\xE7", weightmp);
	vmsg(buf);
	return 1;
      }
      else
      {
	/* 回心轉意囉..... */
	vmsg("\xA6\x5E\xA4\xDF\xC2\xE0\xB7\x4E\xC5\x6F.....");
      }
    }
    break;

  case '2':
    /*  增一公斤要 30 元，您要增多少公斤呢？[請填數字]： */
    vget(b_lines - 1, 0, " \xBC\x57\xA4\x40\xA4\xBD\xA4\xE7\xAD\x6E 30 \xA4\xB8\xA1\x41\xB1\x7A\xAD\x6E\xBC\x57\xA6\x68\xA4\xD6\xA4\xBD\xA4\xE7\xA9\x4F\xA1\x48[\xBD\xD0\xB6\xF1\xBC\xC6\xA6\x72]\xA1\x47", buf, 4, DOECHO);
    weightmp = atoi(buf);
    if (weightmp <= 0)
    {
      /* 輸入有誤..放棄囉... */
      vmsg("\xBF\xE9\xA4\x4A\xA6\xB3\xBB\x7E..\xA9\xF1\xB1\xF3\xC5\x6F...");
    }
    else if (d.money > (weightmp * 30))
    {
      /*  增加 %d 公斤，總共需花費了 %d 元，確定嗎(Y/N)？[N]  */
      sprintf(buf, " \xBC\x57\xA5\x5B %d \xA4\xBD\xA4\xE7\xA1\x41\xC1\x60\xA6\x40\xBB\xDD\xAA\xE1\xB6\x4F\xA4\x46 %d \xA4\xB8\xA1\x41\xBD\x54\xA9\x77\xB6\xDC(Y/N)\xA1\x48[N] ", weightmp, weightmp * 30);      
      if (ians(b_lines - 1, 0, buf) == 'y')
      {
	d.money -= weightmp * 30;
	d.weight += weightmp;
	count_tired(5, 8, 0, 100, 1);
	d.hp -= (rand() % 2 + 3);
	d.sick += rand() % 10 + 5;
	show_special_pic(3);
	/* 總共增加了 %d 公斤 */
	sprintf(buf, "\xC1\x60\xA6\x40\xBC\x57\xA5\x5B\xA4\x46 %d \xA4\xBD\xA4\xE7", weightmp);
	vmsg(buf);
	return 1;
      }
      else
      {
	/* 回心轉意囉..... */
	vmsg("\xA6\x5E\xA4\xDF\xC2\xE0\xB7\x4E\xC5\x6F.....");
      }
    }
    else
    {
      /* 您錢沒那麼多啦....... */
      vmsg("\xB1\x7A\xBF\xFA\xA8\x53\xA8\xBA\xBB\xF2\xA6\x68\xB0\xD5.......");
    }
    break;

  case '3':
    if (d.money < 80)
    {
      /* 傳統減肥要80元喔....您不夠錢啦... */
      vmsg("\xB6\xC7\xB2\xCE\xB4\xEE\xAA\xCE\xAD\x6E""80\xA4\xB8\xB3\xE1....\xB1\x7A\xA4\xA3\xB0\xF7\xBF\xFA\xB0\xD5...");
    }
    else
    {
      /* 需花費 80元(3～5公斤)，確定嗎(Y/N)？[N]  */
      if (ians(b_lines - 1, 0, "\xBB\xDD\xAA\xE1\xB6\x4F 80\xA4\xB8(3\xA1\xE3""5\xA4\xBD\xA4\xE7)\xA1\x41\xBD\x54\xA9\x77\xB6\xDC(Y/N)\xA1\x48[N] ") == 'y')
      {
	weightmp = 3 + rand() % 3;
	d.weight -= weightmp;
	if (d.weight <= 0)
	  d.weight = 1;
	d.money -= 100;
	d.hp -= rand() % 2 + 3;
	show_special_pic(4);
	/* 總共減少了 %d 公斤 */
	sprintf(buf, "\xC1\x60\xA6\x40\xB4\xEE\xA4\xD6\xA4\x46 %d \xA4\xBD\xA4\xE7", weightmp);
	vmsg(buf);
	return 1;
      }
      else
      {
	/* 回心轉意囉..... */
	vmsg("\xA6\x5E\xA4\xDF\xC2\xE0\xB7\x4E\xC5\x6F.....");
      }
    }
    break;

  case '4':
    /*  減一公斤要 30 元，您要減多少公斤呢？[請填數字]： */
    vget(b_lines - 1, 0, " \xB4\xEE\xA4\x40\xA4\xBD\xA4\xE7\xAD\x6E 30 \xA4\xB8\xA1\x41\xB1\x7A\xAD\x6E\xB4\xEE\xA6\x68\xA4\xD6\xA4\xBD\xA4\xE7\xA9\x4F\xA1\x48[\xBD\xD0\xB6\xF1\xBC\xC6\xA6\x72]\xA1\x47", buf, 4, DOECHO);
    weightmp = atoi(buf);
    if (weightmp <= 0)
    {
      /* 輸入有誤..放棄囉... */
      vmsg("\xBF\xE9\xA4\x4A\xA6\xB3\xBB\x7E..\xA9\xF1\xB1\xF3\xC5\x6F...");
    }
    else if (d.weight <= weightmp)
    {
      /* 您沒那麼重喔..... */
      vmsg("\xB1\x7A\xA8\x53\xA8\xBA\xBB\xF2\xAD\xAB\xB3\xE1.....");
    }
    else if (d.money > (weightmp * 30))
    {
      /*  減少 %d 公斤，總共需花費了 %d 元，確定嗎(Y/N)？[N]  */
      sprintf(buf, " \xB4\xEE\xA4\xD6 %d \xA4\xBD\xA4\xE7\xA1\x41\xC1\x60\xA6\x40\xBB\xDD\xAA\xE1\xB6\x4F\xA4\x46 %d \xA4\xB8\xA1\x41\xBD\x54\xA9\x77\xB6\xDC(Y/N)\xA1\x48[N] ", weightmp, weightmp * 30);
      if (ians(b_lines - 1, 0, buf) == 'y')
      {
	d.money -= weightmp * 30;
	d.weight -= weightmp;
	count_tired(5, 8, 0, 100, 1);
	d.hp -= (rand() % 2 + 3);
	d.sick += rand() % 10 + 5;
	show_special_pic(4);
	/* 總共減少了 %d 公斤 */
	sprintf(buf, "\xC1\x60\xA6\x40\xB4\xEE\xA4\xD6\xA4\x46 %d \xA4\xBD\xA4\xE7", weightmp);
	vmsg(buf);
	return 1;
      }
      else
      {
	/* 回心轉意囉..... */
	vmsg("\xA6\x5E\xA4\xDF\xC2\xE0\xB7\x4E\xC5\x6F.....");
      }
    }
    else
    {
      /* 您錢沒那麼多啦....... */
      vmsg("\xB1\x7A\xBF\xFA\xA8\x53\xA8\xBA\xBB\xF2\xA6\x68\xB0\xD5.......");
    }
    break;
  }
  return 0;
}


/*-------------------------------------------------------*/
/* 系統選單: 個人資料 拜訪 小雞放生  特別服務		 */
/*-------------------------------------------------------*/


static int			/* 0: 沒有養小雞  1: 有養小雞 */
pip_data_list(userid)		/* 看某人小雞詳細資料 */
  char *userid;
{
  char buf1[20], buf2[20], buf3[20], buf4[20];
  int ch, page;
  struct CHICKEN chicken;

  if (!strcmp(cuser.userid, userid))	/* itoc.021031: 如果查詢自己，從記憶體叫現在值 */
  {
    memcpy(&chicken, &d, sizeof(CHICKEN));
  }
  else if (pip_read_file(userid, &chicken) < 0)
  {
    /* 他沒有養小雞喔 */
    vmsg("\xA5\x4C\xA8\x53\xA6\xB3\xBE\x69\xA4\x70\xC2\xFB\xB3\xE1");
    return 0;
  }

  page = 1;

  do
  {
    clear();
    move(1, 0);

    /* itoc,010802: 為了看清楚一點，所以 prints() 裡面的引數就不斷行寫在該列最後 */

    if (page == 1)
    {
      /* \033[1;31m ╭┤\033[41;37m 基本資料 \033[0;1;31m├────────────────────────────╮\033[m\n */
      outs("\033[1;31m \xA2\x7E\xA2\x74\033[41;37m \xB0\xF2\xA5\xBB\xB8\xEA\xAE\xC6 \033[0;1;31m\xA2\x75\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\xA1\033[m\n");
      /* \033[1;31m │\033[33m﹟姓    名 : \033[37m%-10s\033[33m﹟生    日 : \033[37m%-10s\033[33m﹟性    別 : \033[37m%-10s\033[31m │\033[m\n */
      /* ♂ */
      /* ♀ */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xA9\x6D    \xA6\x57 : \033[37m%-10s\033[33m\xA1\xCC\xA5\xCD    \xA4\xE9 : \033[37m%-10s\033[33m\xA1\xCC\xA9\xCA    \xA7\x4F : \033[37m%-10s\033[31m \xA2\x78\033[m\n", chicken.name, chicken.birth, chicken.sex == 1 ? "\xA1\xF1" : "\xA1\xF0");
      /* \033[1;31m │\033[33m﹟狀    態 : \033[37m%-10s\033[33m﹟復活次數 : \033[37m%-10d\033[33m﹟年    齡 : \033[37m%-10d\033[31m │\033[m\n */
      /* 死亡 */
      /* 拋棄 */
      /* 結束 */
      /* 正常 */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xAA\xAC    \xBA\x41 : \033[37m%-10s\033[33m\xA1\xCC\xB4\x5F\xAC\xA1\xA6\xB8\xBC\xC6 : \033[37m%-10d\033[33m\xA1\xCC\xA6\x7E    \xC4\xD6 : \033[37m%-10d\033[31m \xA2\x78\033[m\n", chicken.death == 1 ? "\xA6\xBA\xA4\x60" : chicken.death == 2 ? "\xA9\xDF\xB1\xF3" : chicken.death == 3 ? "\xB5\xB2\xA7\xF4" : "\xA5\xBF\xB1\x60", chicken.liveagain, chicken.bbtime / 60 / 30);

      /* \033[1;31m ├┤\033[41;37m 狀態指數 \033[0;1;31m├────────────────────────────┤\033[m\n */
      outs("\033[1;31m \xA2\x75\xA2\x74\033[41;37m \xAA\xAC\xBA\x41\xAB\xFC\xBC\xC6 \033[0;1;31m\xA2\x75\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x74\033[m\n");
      /* \033[1;31m │\033[33m﹟親子關係 : \033[37m%-10d\033[33m﹟快 樂 度 : \033[37m%-10d\033[33m﹟滿 意 度 : \033[37m%-10d\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xBF\xCB\xA4\x6C\xC3\xF6\xAB\x59 : \033[37m%-10d\033[33m\xA1\xCC\xA7\xD6 \xBC\xD6 \xAB\xD7 : \033[37m%-10d\033[33m\xA1\xCC\xBA\xA1 \xB7\x4E \xAB\xD7 : \033[37m%-10d\033[31m \xA2\x78\033[m\n", chicken.relation, chicken.happy, chicken.satisfy);
      /* \033[1;31m │\033[33m﹟戀愛指數 : \033[37m%-10d\033[33m﹟信    仰 : \033[37m%-10d\033[33m﹟罪    孽 : \033[37m%-10d\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xC5\xCA\xB7\x52\xAB\xFC\xBC\xC6 : \033[37m%-10d\033[33m\xA1\xCC\xAB\x48    \xA5\xF5 : \033[37m%-10d\033[33m\xA1\xCC\xB8\x6F    \xC4\x5E : \033[37m%-10d\033[31m \xA2\x78\033[m\n", chicken.fallinlove, chicken.belief, chicken.sin);
      /* \033[1;31m │\033[33m﹟感    受 : \033[37m%-10d\033[33m﹟         : \033[37m%-10s\033[33m﹟         : \033[37m%-10s\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xB7\x50    \xA8\xFC : \033[37m%-10d\033[33m\xA1\xCC         : \033[37m%-10s\033[33m\xA1\xCC         : \033[37m%-10s\033[31m \xA2\x78\033[m\n", chicken.affect, "", "");

      /* \033[1;31m ├┤\033[41;37m 健康指數 \033[0;1;31m├────────────────────────────┤\033[m\n */
      outs("\033[1;31m \xA2\x75\xA2\x74\033[41;37m \xB0\xB7\xB1\x64\xAB\xFC\xBC\xC6 \033[0;1;31m\xA2\x75\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x74\033[m\n");
      /* \033[1;31m │\033[33m﹟體    重 : \033[37m%-10d\033[33m﹟疲 勞 度 : \033[37m%-10d\033[33m﹟病    氣 : \033[37m%-10d\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xC5\xE9    \xAD\xAB : \033[37m%-10d\033[33m\xA1\xCC\xAF\x68 \xB3\xD2 \xAB\xD7 : \033[37m%-10d\033[33m\xA1\xCC\xAF\x66    \xAE\xF0 : \033[37m%-10d\033[31m \xA2\x78\033[m\n", chicken.weight, chicken.tired, chicken.sick);
      /* \033[1;31m │\033[33m﹟清 潔 度 : \033[37m%-10d\033[33m﹟         : \033[37m%-10s\033[33m﹟         : \033[37m%-10s\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xB2\x4D \xBC\xE4 \xAB\xD7 : \033[37m%-10d\033[33m\xA1\xCC         : \033[37m%-10s\033[33m\xA1\xCC         : \033[37m%-10s\033[31m \xA2\x78\033[m\n", chicken.shit, "", "");

      /* \033[1;31m ├┤\033[41;37m 遊戲背景 \033[0;1;31m├────────────────────────────┤\033[m\n */
      outs("\033[1;31m \xA2\x75\xA2\x74\033[41;37m \xB9\x43\xC0\xB8\xAD\x49\xB4\xBA \033[0;1;31m\xA2\x75\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x74\033[m\n");
      /* \033[1;31m │\033[33m﹟猜 拳 贏 : \033[37m%-10d\033[33m﹟猜 拳 輸 : \033[37m%-10d\033[33m﹟猜拳平手 : \033[37m%-10d\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xB2\x71 \xAE\xB1 \xC4\xB9 : \033[37m%-10d\033[33m\xA1\xCC\xB2\x71 \xAE\xB1 \xBF\xE9 : \033[37m%-10d\033[33m\xA1\xCC\xB2\x71\xAE\xB1\xA5\xAD\xA4\xE2 : \033[37m%-10d\033[31m \xA2\x78\033[m\n", chicken.winn, chicken.losee, chicken.tiee);

      /* \033[1;31m ├┤\033[41;37m 評價參數 \033[0;1;31m├────────────────────────────┤\033[m\n */
      outs("\033[1;31m \xA2\x75\xA2\x74\033[41;37m \xB5\xFB\xBB\xF9\xB0\xD1\xBC\xC6 \033[0;1;31m\xA2\x75\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x74\033[m\n");
      /* \033[1;31m │\033[33m﹟社交評價 : \033[37m%-10d\033[33m﹟家事評價 : \033[37m%-10d\033[33m﹟戰鬥評價 : \033[37m%-10d\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xAA\xC0\xA5\xE6\xB5\xFB\xBB\xF9 : \033[37m%-10d\033[33m\xA1\xCC\xAE\x61\xA8\xC6\xB5\xFB\xBB\xF9 : \033[37m%-10d\033[33m\xA1\xCC\xBE\xD4\xB0\xAB\xB5\xFB\xBB\xF9 : \033[37m%-10d\033[31m \xA2\x78\033[m\n", chicken.social, chicken.family, chicken.hexp);
      /* \033[1;31m │\033[33m﹟魔法評價 : \033[37m%-10d\033[33m﹟         : \033[37m%-10s\033[33m﹟         : \033[37m%-10s\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xC5\x5D\xAA\x6B\xB5\xFB\xBB\xF9 : \033[37m%-10d\033[33m\xA1\xCC         : \033[37m%-10s\033[33m\xA1\xCC         : \033[37m%-10s\033[31m \xA2\x78\033[m\n", chicken.hexp, "", "");

      /* \033[1;31m ╰───────────────────────────────────╯\033[m\n */
      outs("\033[1;31m \xA2\xA2\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\xA3\033[m\n");
      move(b_lines - 2, 0);
      /*                                                              \033[1;36m第一頁\033[37m/\033[36m共三頁\033[m\n */
      outs("                                                             \033[1;36m\xB2\xC4\xA4\x40\xAD\xB6\033[37m/\033[36m\xA6\x40\xA4\x54\xAD\xB6\033[m\n");
    }
    else if (page == 2)
    {
      /* \033[1;31m ╭┤\033[41;37m 能力參數 \033[0;1;31m├────────────────────────────╮\033[m\n */
      outs("\033[1;31m \xA2\x7E\xA2\x74\033[41;37m \xAF\xE0\xA4\x4F\xB0\xD1\xBC\xC6 \033[0;1;31m\xA2\x75\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\xA1\033[m\n");
      /* \033[1;31m │\033[33m﹟待人接物 : \033[37m%-10d\033[33m﹟氣 質 度 : \033[37m%-10d\033[33m﹟愛    心 : \033[37m%-10d\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xAB\xDD\xA4\x48\xB1\xB5\xAA\xAB : \033[37m%-10d\033[33m\xA1\xCC\xAE\xF0 \xBD\xE8 \xAB\xD7 : \033[37m%-10d\033[33m\xA1\xCC\xB7\x52    \xA4\xDF : \033[37m%-10d\033[31m \xA2\x78\033[m\n", chicken.toman, chicken.character, chicken.love);
      /* \033[1;31m │\033[33m﹟智    力 : \033[37m%-10d\033[33m﹟藝術能力 : \033[37m%-10d\033[33m﹟道    德 : \033[37m%-10d\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xB4\xBC    \xA4\x4F : \033[37m%-10d\033[33m\xA1\xCC\xC3\xC0\xB3\x4E\xAF\xE0\xA4\x4F : \033[37m%-10d\033[33m\xA1\xCC\xB9\x44    \xBC\x77 : \033[37m%-10d\033[31m \xA2\x78\033[m\n", chicken.wisdom, chicken.art, chicken.etchics);
      /* \033[1;31m │\033[33m﹟勇    敢 : \033[37m%-10d\033[33m﹟掃地洗衣 : \033[37m%-10d\033[33m﹟魅    力 : \033[37m%-10d\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xAB\x69    \xB4\xB1 : \033[37m%-10d\033[33m\xA1\xCC\xB1\xBD\xA6\x61\xAC\x7E\xA6\xE7 : \033[37m%-10d\033[33m\xA1\xCC\xBE\x79    \xA4\x4F : \033[37m%-10d\033[31m \xA2\x78\033[m\n", chicken.brave, chicken.homework, chicken.charm);
      /* \033[1;31m │\033[33m﹟禮    儀 : \033[37m%-10d\033[33m﹟談    吐 : \033[37m%-10d\033[33m﹟烹    飪 : \033[37m%-10d\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xC2\xA7    \xBB\xF6 : \033[37m%-10d\033[33m\xA1\xCC\xBD\xCD    \xA6\x52 : \033[37m%-10d\033[33m\xA1\xCC\xB2\x69    \xB6\xB9 : \033[37m%-10d\033[31m \xA2\x78\033[m\n", chicken.manners, chicken.speech, chicken.cook);
      /* \033[1;31m │\033[33m﹟攻 擊 力 : \033[37m%-10d\033[33m﹟防 禦 力 : \033[37m%-10d\033[33m﹟速    度 : \033[37m%-10d\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xA7\xF0 \xC0\xBB \xA4\x4F : \033[37m%-10d\033[33m\xA1\xCC\xA8\xBE \xBF\x6D \xA4\x4F : \033[37m%-10d\033[33m\xA1\xCC\xB3\x74    \xAB\xD7 : \033[37m%-10d\033[31m \xA2\x78\033[m\n", chicken.attack, chicken.resist, chicken.speed);
      /* \033[1;31m │\033[33m﹟戰鬥技術 : \033[37m%-10d\033[33m﹟魔法技術 : \033[37m%-10d\033[33m﹟抗魔能力 : \033[37m%-10d\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xBE\xD4\xB0\xAB\xA7\xDE\xB3\x4E : \033[37m%-10d\033[33m\xA1\xCC\xC5\x5D\xAA\x6B\xA7\xDE\xB3\x4E : \033[37m%-10d\033[33m\xA1\xCC\xA7\xDC\xC5\x5D\xAF\xE0\xA4\x4F : \033[37m%-10d\033[31m \xA2\x78\033[m\n", chicken.hskill, chicken.mskill, chicken.immune);

      sprintf(buf1, "%d%s/%d%s", chicken.hp > 1000 ? chicken.hp / 1000 : chicken.hp, chicken.hp > 1000 ? "K" : "",	/* HP */
	chicken.maxhp > 1000 ? chicken.maxhp / 1000 : chicken.maxhp, chicken.maxhp > 1000 ? "K" : "");
      sprintf(buf2, "%d%s/%d%s", chicken.mp > 1000 ? chicken.mp / 1000 : chicken.mp, chicken.mp > 1000 ? "K" : "",	/* MP */      
	chicken.maxmp > 1000 ? chicken.maxmp / 1000 : chicken.maxmp, chicken.maxmp > 1000 ? "K" : "");
      sprintf(buf3, "%d%s/%d%s", chicken.vp > 1000 ? chicken.vp / 1000 : chicken.vp, chicken.vp > 1000 ? "K" : "",	/* VP */
	chicken.maxvp > 1000 ? chicken.maxvp / 1000 : chicken.maxvp, chicken.maxvp > 1000 ? "K" : "");
      sprintf(buf4, "%d%s/%d%s", chicken.sp > 1000 ? chicken.sp / 1000 : chicken.sp, chicken.sp > 1000 ? "K" : "",	/* SP */      
	chicken.maxsp > 1000 ? chicken.maxsp / 1000 : chicken.maxsp, chicken.maxsp > 1000 ? "K" : "");            

      /* \033[1;31m ├┤\033[41;37m 戰鬥指標 \033[0;1;31m├────────────────────────────┤\033[m\n */
      outs("\033[1;31m \xA2\x75\xA2\x74\033[41;37m \xBE\xD4\xB0\xAB\xAB\xFC\xBC\xD0 \033[0;1;31m\xA2\x75\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x74\033[m\n");
      /* \033[1;31m │\033[33m﹟等    級 : \033[37m%-10d\033[33m﹟經 驗 值 : \033[37m%-10d\033[33m﹟   血    : \033[37m%-10s\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xB5\xA5    \xAF\xC5 : \033[37m%-10d\033[33m\xA1\xCC\xB8\x67 \xC5\xE7 \xAD\xC8 : \033[37m%-10d\033[33m\xA1\xCC   \xA6\xE5    : \033[37m%-10s\033[31m \xA2\x78\033[m\n", chicken.level, chicken.exp, buf1);
      /* \033[1;31m │\033[33m﹟法    力 : \033[37m%-10s\033[33m﹟移 動 力 : \033[37m%-10s\033[33m﹟內    力 : \033[37m%-10s\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xAA\x6B    \xA4\x4F : \033[37m%-10s\033[33m\xA1\xCC\xB2\xBE \xB0\xCA \xA4\x4F : \033[37m%-10s\033[33m\xA1\xCC\xA4\xBA    \xA4\x4F : \033[37m%-10s\033[31m \xA2\x78\033[m\n", buf2, buf3, buf4);

      /* \033[1;31m ├┤\033[41;37m 食物庫存 \033[0;1;31m├────────────────────────────┤\033[m\n */
      outs("\033[1;31m \xA2\x75\xA2\x74\033[41;37m \xAD\xB9\xAA\xAB\xAE\x77\xA6\x73 \033[0;1;31m\xA2\x75\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x74\033[m\n");
      /* \033[1;31m │\033[33m﹟食    物 : \033[37m%-10d\033[33m﹟零    食 : \033[37m%-10d\033[33m﹟         : \033[37m%-10s\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xAD\xB9    \xAA\xAB : \033[37m%-10d\033[33m\xA1\xCC\xB9\x73    \xAD\xB9 : \033[37m%-10d\033[33m\xA1\xCC         : \033[37m%-10s\033[31m \xA2\x78\033[m\n", chicken.food, chicken.cookie, "");
      /* \033[1;31m │\033[33m﹟大 還 丹 : \033[37m%-10d\033[33m﹟靈    芝 : \033[37m%-10d\033[33m﹟大 補 丸 : \033[37m%-10d\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xA4\x6A \xC1\xD9 \xA4\xA6 : \033[37m%-10d\033[33m\xA1\xCC\xC6\x46    \xAA\xDB : \033[37m%-10d\033[33m\xA1\xCC\xA4\x6A \xB8\xC9 \xA4\x59 : \033[37m%-10d\033[31m \xA2\x78\033[m\n", chicken.pill, chicken.medicine, chicken.burger);
      /* \033[1;31m │\033[33m﹟人    蔘 : \033[37m%-10d\033[33m﹟斷 續 膏 : \033[37m%-10d\033[33m﹟雪    蓮 : \033[37m%-10d\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xA4\x48    \xE7\x78 : \033[37m%-10d\033[33m\xA1\xCC\xC2\x5F \xC4\xF2 \xBB\x49 : \033[37m%-10d\033[33m\xA1\xCC\xB3\xB7    \xBD\xAC : \033[37m%-10d\033[31m \xA2\x78\033[m\n", chicken.ginseng, chicken.paste, chicken.snowgrass);

      /* \033[1;31m ├┤\033[41;37m 物品庫存 \033[0;1;31m├────────────────────────────┤\033[m\n */
      outs("\033[1;31m \xA2\x75\xA2\x74\033[41;37m \xAA\xAB\xAB\x7E\xAE\x77\xA6\x73 \033[0;1;31m\xA2\x75\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x74\033[m\n");
      /* \033[1;31m │\033[33m﹟金    錢 : \033[37m%-10d\033[33m﹟         : \033[37m%-10s\033[33m﹟         : \033[37m%-10s\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xAA\xF7    \xBF\xFA : \033[37m%-10d\033[33m\xA1\xCC         : \033[37m%-10s\033[33m\xA1\xCC         : \033[37m%-10s\033[31m \xA2\x78\033[m\n", chicken.money, "", "");
      /* \033[1;31m │\033[33m﹟書    本 : \033[37m%-10d\033[33m﹟玩    具 : \033[37m%-10d\033[33m﹟課外讀物 : \033[37m%-10d\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xAE\xD1    \xA5\xBB : \033[37m%-10d\033[33m\xA1\xCC\xAA\xB1    \xA8\xE3 : \033[37m%-10d\033[33m\xA1\xCC\xBD\xD2\xA5\x7E\xC5\xAA\xAA\xAB : \033[37m%-10d\033[31m \xA2\x78\033[m\n", chicken.book, chicken.toy, chicken.playboy);

 
      /* \033[1;31m ╰───────────────────────────────────╯\033[m\n */
      outs("\033[1;31m \xA2\xA2\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\xA3\033[m\n");
      move(b_lines - 2, 0);
      /*                                                              \033[1;36m第二頁\033[37m/\033[36m共三頁\033[m\n */
      outs("                                                             \033[1;36m\xB2\xC4\xA4\x47\xAD\xB6\033[37m/\033[36m\xA6\x40\xA4\x54\xAD\xB6\033[m\n");
    }
    else /* if (page == 3) */
    {
      /* \033[1;31m ╭┤\033[41;37m 參見王臣 \033[0;1;31m├────────────────────────────╮\033[m\n */
      outs("\033[1;31m \xA2\x7E\xA2\x74\033[41;37m \xB0\xD1\xA8\xA3\xA4\xFD\xA6\xDA \033[0;1;31m\xA2\x75\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\xA1\033[m\n");
      /* \033[1;31m │\033[33m﹟守衛好感 : \033[37m%-10d\033[33m﹟近衛好感 : \033[37m%-10d\033[33m﹟將軍好感 : \033[37m%-10d\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xA6\x75\xBD\xC3\xA6\x6E\xB7\x50 : \033[37m%-10d\033[33m\xA1\xCC\xAA\xF1\xBD\xC3\xA6\x6E\xB7\x50 : \033[37m%-10d\033[33m\xA1\xCC\xB1\x4E\xAD\x78\xA6\x6E\xB7\x50 : \033[37m%-10d\033[31m \xA2\x78\033[m\n", chicken.royalA, chicken.royalB, chicken.royalC);
      /* \033[1;31m │\033[33m﹟大臣好感 : \033[37m%-10d\033[33m﹟祭司好感 : \033[37m%-10d\033[33m﹟寵妃好感 : \033[37m%-10d\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xA4\x6A\xA6\xDA\xA6\x6E\xB7\x50 : \033[37m%-10d\033[33m\xA1\xCC\xB2\xBD\xA5\x71\xA6\x6E\xB7\x50 : \033[37m%-10d\033[33m\xA1\xCC\xC3\x64\xA6\x6D\xA6\x6E\xB7\x50 : \033[37m%-10d\033[31m \xA2\x78\033[m\n", chicken.royalD, chicken.royalE, chicken.royalF);
      /* \033[1;31m │\033[33m﹟王妃好感 : \033[37m%-10d\033[33m﹟國王好感 : \033[37m%-10d\033[33m﹟小丑好感 : \033[37m%-10d\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xA4\xFD\xA6\x6D\xA6\x6E\xB7\x50 : \033[37m%-10d\033[33m\xA1\xCC\xB0\xEA\xA4\xFD\xA6\x6E\xB7\x50 : \033[37m%-10d\033[33m\xA1\xCC\xA4\x70\xA4\xA1\xA6\x6E\xB7\x50 : \033[37m%-10d\033[31m \xA2\x78\033[m\n", chicken.royalG, chicken.royalH, chicken.royalI);

      /* \033[1;31m ├┤\033[41;37m 工作次數 \033[0;1;31m├────────────────────────────┤\033[m\n */
      outs("\033[1;31m \xA2\x75\xA2\x74\033[41;37m \xA4\x75\xA7\x40\xA6\xB8\xBC\xC6 \033[0;1;31m\xA2\x75\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x74\033[m\n");
      /* \033[1;31m │\033[33m﹟家事次數 : \033[37m%-10d\033[33m﹟保姆次數 : \033[37m%-10d\033[33m﹟旅店次數 : \033[37m%-10d\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xAE\x61\xA8\xC6\xA6\xB8\xBC\xC6 : \033[37m%-10d\033[33m\xA1\xCC\xAB\x4F\xA9\x69\xA6\xB8\xBC\xC6 : \033[37m%-10d\033[33m\xA1\xCC\xAE\xC8\xA9\xB1\xA6\xB8\xBC\xC6 : \033[37m%-10d\033[31m \xA2\x78\033[m\n", chicken.workA, chicken.workB, chicken.workC);
      /* \033[1;31m │\033[33m﹟農場次數 : \033[37m%-10d\033[33m﹟餐廳次數 : \033[37m%-10d\033[33m﹟教堂次數 : \033[37m%-10d\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xB9\x41\xB3\xF5\xA6\xB8\xBC\xC6 : \033[37m%-10d\033[33m\xA1\xCC\xC0\x5C\xC6\x55\xA6\xB8\xBC\xC6 : \033[37m%-10d\033[33m\xA1\xCC\xB1\xD0\xB0\xF3\xA6\xB8\xBC\xC6 : \033[37m%-10d\033[31m \xA2\x78\033[m\n", chicken.workD, chicken.workE, chicken.workF);
      /* \033[1;31m │\033[33m﹟地攤次數 : \033[37m%-10d\033[33m﹟伐木次數 : \033[37m%-10d\033[33m﹟美髮次數 : \033[37m%-10d\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xA6\x61\xC5\x75\xA6\xB8\xBC\xC6 : \033[37m%-10d\033[33m\xA1\xCC\xA5\xEF\xA4\xEC\xA6\xB8\xBC\xC6 : \033[37m%-10d\033[33m\xA1\xCC\xAC\xFC\xBE\x76\xA6\xB8\xBC\xC6 : \033[37m%-10d\033[31m \xA2\x78\033[m\n", chicken.workG, chicken.workH, chicken.workI);
      /* \033[1;31m │\033[33m﹟獵人次數 : \033[37m%-10d\033[33m﹟工地次數 : \033[37m%-10d\033[33m﹟守墓次數 : \033[37m%-10d\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xC2\x79\xA4\x48\xA6\xB8\xBC\xC6 : \033[37m%-10d\033[33m\xA1\xCC\xA4\x75\xA6\x61\xA6\xB8\xBC\xC6 : \033[37m%-10d\033[33m\xA1\xCC\xA6\x75\xB9\xD3\xA6\xB8\xBC\xC6 : \033[37m%-10d\033[31m \xA2\x78\033[m\n", chicken.workJ, chicken.workK, chicken.workL);
      /* \033[1;31m │\033[33m﹟家教次數 : \033[37m%-10d\033[33m﹟酒家次數 : \033[37m%-10d\033[33m﹟酒店次數 : \033[37m%-10d\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xAE\x61\xB1\xD0\xA6\xB8\xBC\xC6 : \033[37m%-10d\033[33m\xA1\xCC\xB0\x73\xAE\x61\xA6\xB8\xBC\xC6 : \033[37m%-10d\033[33m\xA1\xCC\xB0\x73\xA9\xB1\xA6\xB8\xBC\xC6 : \033[37m%-10d\033[31m \xA2\x78\033[m\n", chicken.workM, chicken.workN, chicken.workO);
      /* \033[1;31m │\033[33m﹟夜 總 會 : \033[37m%-10d\033[33m﹟         : \033[37m%-10s\033[33m﹟         : \033[37m%-10s\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xA9\x5D \xC1\x60 \xB7\x7C : \033[37m%-10d\033[33m\xA1\xCC         : \033[37m%-10s\033[33m\xA1\xCC         : \033[37m%-10s\033[31m \xA2\x78\033[m\n", chicken.workP, "", "");

      /* \033[1;31m ├┤\033[41;37m 上課次數 \033[0;1;31m├────────────────────────────┤\033[m\n */
      outs("\033[1;31m \xA2\x75\xA2\x74\033[41;37m \xA4\x57\xBD\xD2\xA6\xB8\xBC\xC6 \033[0;1;31m\xA2\x75\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x74\033[m\n");
      /* \033[1;31m │\033[33m﹟自然科學 : \033[37m%-10d\033[33m﹟唐詩宋詞 : \033[37m%-10d\033[33m﹟神學教育 : \033[37m%-10d\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xA6\xDB\xB5\x4D\xAC\xEC\xBE\xC7 : \033[37m%-10d\033[33m\xA1\xCC\xAD\xF0\xB8\xD6\xA7\xBA\xB5\xFC : \033[37m%-10d\033[33m\xA1\xCC\xAF\xAB\xBE\xC7\xB1\xD0\xA8\x7C : \033[37m%-10d\033[31m \xA2\x78\033[m\n", chicken.classA, chicken.classB, chicken.classC);
      /* \033[1;31m │\033[33m﹟軍學教育 : \033[37m%-10d\033[33m﹟劍道技術 : \033[37m%-10d\033[33m﹟格鬥戰技 : \033[37m%-10d\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xAD\x78\xBE\xC7\xB1\xD0\xA8\x7C : \033[37m%-10d\033[33m\xA1\xCC\xBC\x43\xB9\x44\xA7\xDE\xB3\x4E : \033[37m%-10d\033[33m\xA1\xCC\xAE\xE6\xB0\xAB\xBE\xD4\xA7\xDE : \033[37m%-10d\033[31m \xA2\x78\033[m\n", chicken.classD, chicken.classE, chicken.classF);
      /* \033[1;31m │\033[33m﹟魔法教育 : \033[37m%-10d\033[33m﹟禮儀教育 : \033[37m%-10d\033[33m﹟繪畫技巧 : \033[37m%-10d\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xC5\x5D\xAA\x6B\xB1\xD0\xA8\x7C : \033[37m%-10d\033[33m\xA1\xCC\xC2\xA7\xBB\xF6\xB1\xD0\xA8\x7C : \033[37m%-10d\033[33m\xA1\xCC\xC3\xB8\xB5\x65\xA7\xDE\xA5\xA9 : \033[37m%-10d\033[31m \xA2\x78\033[m\n", chicken.classG, chicken.classH, chicken.classI);
      /* \033[1;31m │\033[33m﹟舞蹈技巧 : \033[37m%-10d\033[33m﹟         : \033[37m%-10s\033[33m﹟         : \033[37m%-10s\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xBB\x52\xC1\xD0\xA7\xDE\xA5\xA9 : \033[37m%-10d\033[33m\xA1\xCC         : \033[37m%-10s\033[33m\xA1\xCC         : \033[37m%-10s\033[31m \xA2\x78\033[m\n", chicken.classJ, "", "");

      /* \033[1;31m ├┤\033[41;37m 裝備列表 \033[0;1;31m├────────────────────────────┤\033[m\n */
      outs("\033[1;31m \xA2\x75\xA2\x74\033[41;37m \xB8\xCB\xB3\xC6\xA6\x43\xAA\xED \033[0;1;31m\xA2\x75\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x74\033[m\n");
      /* \033[1;31m │\033[33m﹟頭部裝備 : \033[37m%-10s\033[33m﹟手部裝備 : \033[37m%-10s\033[33m﹟盾牌裝備 : \033[37m%-10s\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xC0\x59\xB3\xA1\xB8\xCB\xB3\xC6 : \033[37m%-10s\033[33m\xA1\xCC\xA4\xE2\xB3\xA1\xB8\xCB\xB3\xC6 : \033[37m%-10s\033[33m\xA1\xCC\xAC\xDE\xB5\x50\xB8\xCB\xB3\xC6 : \033[37m%-10s\033[31m \xA2\x78\033[m\n", chicken.equiphead, chicken.equiphand, chicken.equipshield);
      /* \033[1;31m │\033[33m﹟身體裝備 : \033[37m%-10s\033[33m﹟腳部裝備 : \033[37m%-10s\033[33m﹟           \033[37m%-10s\033[31m │\033[m\n */
      prints("\033[1;31m \xA2\x78\033[33m\xA1\xCC\xA8\xAD\xC5\xE9\xB8\xCB\xB3\xC6 : \033[37m%-10s\033[33m\xA1\xCC\xB8\x7D\xB3\xA1\xB8\xCB\xB3\xC6 : \033[37m%-10s\033[33m\xA1\xCC           \033[37m%-10s\033[31m \xA2\x78\033[m\n", chicken.equipbody, chicken.equipfoot, "");

      /* \033[1;31m ╰───────────────────────────────────╯\033[m\n */
      outs("\033[1;31m \xA2\xA2\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\xA3\033[m\n");
      move(b_lines - 2, 0);
      /*                                                              \033[1;36m第三頁\033[37m/\033[36m共三頁\033[m\n */
      outs("                                                             \033[1;36m\xB2\xC4\xA4\x54\xAD\xB6\033[37m/\033[36m\xA6\x40\xA4\x54\xAD\xB6\033[m\n");
    }

    /*  查詢  */
    /*  [↑/PgUP]往上一頁 [↓/PgDN]往下一頁 [Q]離開                            \033[m */
    out_cmd("", COLOR1 " \xAC\x64\xB8\xDF " COLOR2 " [\xA1\xF4/PgUP]\xA9\xB9\xA4\x57\xA4\x40\xAD\xB6 [\xA1\xF5/PgDN]\xA9\xB9\xA4\x55\xA4\x40\xAD\xB6 [Q]\xC2\xF7\xB6\x7D                            \033[m");

    switch (ch = vkey())
    {
    case KEY_UP:
    case KEY_PGUP:
      if (page > 1)
        page--;
      break;

    default:
      if (page < 3)
	page++;
      break;
    }
  } while (ch != 'q' && ch != KEY_LEFT);

  return 1;
}


int
pip_query_self()		/* 查詢自己 */
{
  pip_data_list(cuser.userid);
  return 0;
}


int				/* 1:拜訪成功  0:沒這隻雞 */
pip_query()			/* 拜訪小雞 */
{
  int uno;
  char uid[IDLEN + 1];

  /* 拜訪同伴 */
  vs_bar("\xAB\xF4\xB3\x58\xA6\x50\xA6\xF1");
  if (vget(1, 0, msg_uid, uid, IDLEN + 1, GET_USER))
  {
    move(2, 0);
    if (uno = acct_userno(uid))
    {
      pip_data_list(uid);
      return 1;
    }
    else
    {
      outs(err_uid);
      clrtoeol();
    }
  }
  return 0;
}


int				/* 1:放生  0:續養 */
pip_system_freepip()
{
  char buf[80];

  /* 真的要放生嗎(Y/N)？[N]  */
  if (ians(b_lines - 2, 0, "\xAF\x75\xAA\xBA\xAD\x6E\xA9\xF1\xA5\xCD\xB6\xDC(Y/N)\xA1\x48[N] ") == 'y')
  {
    /* \033[1;31m%s 被狠心的 %s 丟掉了~\033[m */
    sprintf(buf, "\033[1;31m%s \xB3\x51\xAC\xBD\xA4\xDF\xAA\xBA %s \xA5\xE1\xB1\xBC\xA4\x46~\033[m", d.name, cuser.userid);
    pipdie(buf, 2);
    return 1;
  }
  return 0;
}


int
pip_system_service()
{
  int choice;
  char buf[128];

  /*  服務  */
  /*  [1]命名大師 [2]變性手術 [3]結局設局 [Q]離開                            \033[m */
  out_cmd("", COLOR1 " \xAA\x41\xB0\xC8 " COLOR2 " [1]\xA9\x52\xA6\x57\xA4\x6A\xAE\x76 [2]\xC5\xDC\xA9\xCA\xA4\xE2\xB3\x4E [3]\xB5\xB2\xA7\xBD\xB3\x5D\xA7\xBD [Q]\xC2\xF7\xB6\x7D                            \033[m");

  switch (vkey())
  {
  case '1':
    /* 幫小雞重新取個好名字： */
    vget(b_lines - 2, 0, "\xC0\xB0\xA4\x70\xC2\xFB\xAD\xAB\xB7\x73\xA8\xFA\xAD\xD3\xA6\x6E\xA6\x57\xA6\x72\xA1\x47", buf, 11, DOECHO);
    if (!buf[0])
    {
      /* 等一下想好再來好了  :) */
      vmsg("\xB5\xA5\xA4\x40\xA4\x55\xB7\x51\xA6\x6E\xA6\x41\xA8\xD3\xA6\x6E\xA4\x46  :)");
    }
    else
    {
      strcpy(d.name, buf);
      /* 嗯嗯  換一個新的名字喔... */
      vmsg("\xB6\xE2\xB6\xE2  \xB4\xAB\xA4\x40\xAD\xD3\xB7\x73\xAA\xBA\xA6\x57\xA6\x72\xB3\xE1...");
    }
    break;

  case '2':			/* 變性 */
    if (d.sex == 1)	/* 1:公 2:母 */
    {
      choice = 2;		/* 公-->母 */
      /* 將小雞由♂變性成♀的嗎(Y/N)？[N]  */
      sprintf(buf, "\xB1\x4E\xA4\x70\xC2\xFB\xA5\xD1\xA1\xF1\xC5\xDC\xA9\xCA\xA6\xA8\xA1\xF0\xAA\xBA\xB6\xDC(Y/N)\xA1\x48[N] ");
    }
    else
    {
      choice = 1;		/* 母-->公 */
      /* 將小雞由♀變性成♂的嗎(Y/N)？[N]  */
      sprintf(buf, "\xB1\x4E\xA4\x70\xC2\xFB\xA5\xD1\xA1\xF0\xC5\xDC\xA9\xCA\xA6\xA8\xA1\xF1\xAA\xBA\xB6\xDC(Y/N)\xA1\x48[N] ");
    }
    if (ians(b_lines - 2, 0, buf) == 'y')
    {
      d.sex = choice;
      /* 變性手術完畢... */
      vmsg("\xC5\xDC\xA9\xCA\xA4\xE2\xB3\x4E\xA7\xB9\xB2\xA6...");
    }
    break;

  case '3':
    /* 1:不要且未婚 4:要且未婚 */
    if (d.wantend == 1 || d.wantend == 2 || d.wantend == 3)
    {
      choice = 3;		/* 沒有-->有 */
      /* 將小雞遊戲改成【有20歲結局】(Y/N)？[N]  */
      sprintf(buf, "\xB1\x4E\xA4\x70\xC2\xFB\xB9\x43\xC0\xB8\xA7\xEF\xA6\xA8\xA1\x69\xA6\xB3""20\xB7\xB3\xB5\xB2\xA7\xBD\xA1\x6A(Y/N)\xA1\x48[N] ");
    }
    else
    {
      choice = -3;		/* 有-->沒有 */
      /* 將小雞遊戲改成【沒有20歲結局】(Y/N)？[N]  */
      sprintf(buf, "\xB1\x4E\xA4\x70\xC2\xFB\xB9\x43\xC0\xB8\xA7\xEF\xA6\xA8\xA1\x69\xA8\x53\xA6\xB3""20\xB7\xB3\xB5\xB2\xA7\xBD\xA1\x6A(Y/N)\xA1\x48[N] ");
    }
    if (ians(b_lines - 2, 0, buf) == 'y')
    {
      d.wantend += choice;
      /* 遊戲結局設定完畢... */
      vmsg("\xB9\x43\xC0\xB8\xB5\xB2\xA7\xBD\xB3\x5D\xA9\x77\xA7\xB9\xB2\xA6...");
    }
    break;
  }
  return 0;
}
#endif	/* HAVE_GAME */
