/*-------------------------------------------------------*/
/* so/bank.c            ( NTHU CS MapleBBS Ver 3.10 )    */
/*-------------------------------------------------------*/
/* target : 銀行、購買權限功能				 */
/* create : 01/07/16					 */
/* update :   /  /  					 */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_BUY

static void
x_give()
{
  int way, dollar;
  char userid[IDLEN + 1], buf[80];
  char folder[64], fpath[64], reason[40];
  HDR hdr;
  FILE *fp;
  time_t now;
  PAYCHECK paycheck;

  /* 您要把錢轉給誰呢？ */
  if (!vget(13, 0, "\xB1\x7A\xAD\x6E\xA7\xE2\xBF\xFA\xC2\xE0\xB5\xB9\xBD\xD6\xA9\x4F\xA1\x48", userid, IDLEN + 1, DOECHO))
    return;

  if (acct_userno(userid) <= 0)
  {
    vmsg(err_uid);
    return;
  }

  /* 轉帳 1)轉銀幣 2)轉金幣： */
  way = vget(15, 0, "\xC2\xE0\xB1\x62 1)\xC2\xE0\xBB\xC8\xB9\xF4 2)\xC2\xE0\xAA\xF7\xB9\xF4\xA1\x47", buf, 3, DOECHO) - '1';
  if (way < 0 || way > 1)
    return;

  do
  {
    /* 要轉多少錢過去？ */
    if (!vget(17, 0, "\xAD\x6E\xC2\xE0\xA6\x68\xA4\xD6\xBF\xFA\xB9\x4C\xA5\x68\xA1\x48", buf, 9, DOECHO))	/* 最多轉 99999999 避免溢位 */
      return;

    dollar = atoi(buf);

    if (!way)
    {
      if (dollar > cuser.money)
	dollar = cuser.money;	/* 全轉過去 */
    }
    else
    {
      if (dollar > cuser.gold)
	dollar = cuser.gold;	/* 全轉過去 */
    }
  } while (dollar <= 1);	/* 不能只轉 1，會全變手續費 */

  /* 請輸入理由： */
  if (!vget(19, 0, "\xBD\xD0\xBF\xE9\xA4\x4A\xB2\x7A\xA5\xD1\xA1\x47", reason, 40, DOECHO))
    /* 錢太多 */
    strcpy(reason, "\xBF\xFA\xA4\xD3\xA6\x68");

  /* 是否要轉帳給 %s %s幣 %d (Y/N)？[N]  */
  /* 銀 */
  /* 金 */
  sprintf(buf, "\xAC\x4F\xA7\x5F\xAD\x6E\xC2\xE0\xB1\x62\xB5\xB9 %s %s\xB9\xF4 %d (Y/N)\xA1\x48[N] ", userid, !way ? "\xBB\xC8" : "\xAA\xF7", dollar);
  if (vget(21, 0, buf, fpath, 3, LCECHO) == 'y')
  {
    if (!way)
      cuser.money -= dollar;
    else
      cuser.gold -= dollar;

    dollar -= dollar / 10 + ((dollar % 10) ? 1 : 0);	/* 10% 手續費 */

    /* itoc.020831: 加入匯錢記錄 */
    time(&now);
    /* %-13s轉給 %-13s計 %d %s (%s)\n */
    sprintf(buf, "%-13s\xC2\xE0\xB5\xB9 %-13s\xAD\x70 %d %s (%s)\n",
      /* 銀 */
      /* 金 */
      cuser.userid, userid, dollar, !way ? "\xBB\xC8" : "\xAA\xF7", Btime(now));
    f_cat(FN_RUN_BANK_LOG, buf);

    usr_fpath(folder, userid, fn_dir);
    if (fp = fdopen(hdr_stamp(folder, 0, &hdr, fpath), "w"))
    {
      /* %s %s (%s)\n標題: 轉帳通知\n時間: %s\n\n */
      fprintf(fp, "%s %s (%s)\n\xBC\xD0\xC3\x44: \xC2\xE0\xB1\x62\xB3\x71\xAA\xBE\n\xAE\xC9\xB6\xA1: %s\n\n", 
	str_author1, cuser.userid, cuser.username, Btime(now));
      /* %s\n他的理由是：%s\n\n請您至金融中心將支票兌現 */
      fprintf(fp, "%s\n\xA5\x4C\xAA\xBA\xB2\x7A\xA5\xD1\xAC\x4F\xA1\x47%s\n\n\xBD\xD0\xB1\x7A\xA6\xDC\xAA\xF7\xBF\xC4\xA4\xA4\xA4\xDF\xB1\x4E\xA4\xE4\xB2\xBC\xA7\x49\xB2\x7B", buf, reason);
      fclose(fp);      

      /* 轉帳通知 */
      strcpy(hdr.title, "\xC2\xE0\xB1\x62\xB3\x71\xAA\xBE");
      strcpy(hdr.owner, cuser.userid);
      rec_add(folder, &hdr, sizeof(HDR));
    }

    memset(&paycheck, 0, sizeof(PAYCHECK));
    time(&paycheck.tissue);
    if (!way)
      paycheck.money = dollar;
    else
      paycheck.gold = dollar;
    /* [轉帳] %s */
    sprintf(paycheck.reason, "[\xC2\xE0\xB1\x62] %s", cuser.userid);
    usr_fpath(fpath, userid, FN_PAYCHECK);
    rec_add(fpath, &paycheck, sizeof(PAYCHECK));

    /* 您身上有銀幣 %d 元，金幣 %d 元 */
    sprintf(buf, "\xB1\x7A\xA8\xAD\xA4\x57\xA6\xB3\xBB\xC8\xB9\xF4 %d \xA4\xB8\xA1\x41\xAA\xF7\xB9\xF4 %d \xA4\xB8", cuser.money, cuser.gold);
    vmsg(buf);
  }
  else
  {
    /* 取消交易 */
    vmsg("\xA8\xFA\xAE\xF8\xA5\xE6\xA9\xF6");
  }
}


#define GOLD2MONEY	900000	/* 金幣→銀幣 匯率 */
#define MONEY2GOLD	1100000	/* 銀幣→金幣 匯率 */

static void
x_exchange()
{
  int way, gold, money;
  char buf[80], ans[8];

  move(13, 0);
  /* 銀幣→金幣 = %d：1  金幣→銀幣 = 1：%d */
  prints("\xBB\xC8\xB9\xF4\xA1\xF7\xAA\xF7\xB9\xF4 = %d\xA1\x47""1  \xAA\xF7\xB9\xF4\xA1\xF7\xBB\xC8\xB9\xF4 = 1\xA1\x47%d", MONEY2GOLD, GOLD2MONEY);

  /* 匯兌 1)銀幣→金幣 2)金幣→銀幣： */
  way = vget(15, 0, "\xB6\xD7\xA7\x49 1)\xBB\xC8\xB9\xF4\xA1\xF7\xAA\xF7\xB9\xF4 2)\xAA\xF7\xB9\xF4\xA1\xF7\xBB\xC8\xB9\xF4\xA1\x47", ans, 3, DOECHO) - '1';

  if (!way)
    money = cuser.money / MONEY2GOLD;
  else if (way == 1)
    money = cuser.gold;
  else
    return;

  if (!way)
    /* 您要將銀幣兌換成多少個金幣呢？[1 - %d]  */
    sprintf(buf, "\xB1\x7A\xAD\x6E\xB1\x4E\xBB\xC8\xB9\xF4\xA7\x49\xB4\xAB\xA6\xA8\xA6\x68\xA4\xD6\xAD\xD3\xAA\xF7\xB9\xF4\xA9\x4F\xA1\x48[1 - %d] ", money);
  else
    /* 您要兌換多少個金幣成為銀幣呢？[1 - %d]  */
    sprintf(buf, "\xB1\x7A\xAD\x6E\xA7\x49\xB4\xAB\xA6\x68\xA4\xD6\xAD\xD3\xAA\xF7\xB9\xF4\xA6\xA8\xAC\xB0\xBB\xC8\xB9\xF4\xA9\x4F\xA1\x48[1 - %d] ", money);
    
  if (!vget(17, 0, buf, ans, 4, DOECHO))	/* 長度比較短，避免溢位 */
    return;

  gold = atoi(ans);
  if (gold <= 0 || gold > money)
    return;

  if (!way)
  {
    if (gold > (INT_MAX - cuser.gold))
    {
      /* 您換太多錢囉～會溢位的！ */
      vmsg("\xB1\x7A\xB4\xAB\xA4\xD3\xA6\x68\xBF\xFA\xC5\x6F\xA1\xE3\xB7\x7C\xB7\xB8\xA6\xEC\xAA\xBA\xA1\x49");
      return;
    }
    money = gold * MONEY2GOLD;
    /* 是否要兌換銀幣 %d 元 為金幣 %d (Y/N)？[N]  */
    sprintf(buf, "\xAC\x4F\xA7\x5F\xAD\x6E\xA7\x49\xB4\xAB\xBB\xC8\xB9\xF4 %d \xA4\xB8 \xAC\xB0\xAA\xF7\xB9\xF4 %d (Y/N)\xA1\x48[N] ", money, gold);
  }
  else
  {
    money = gold * GOLD2MONEY;
    if (money > (INT_MAX - cuser.money))
    {
      /* 您換太多錢囉～會溢位的！ */
      vmsg("\xB1\x7A\xB4\xAB\xA4\xD3\xA6\x68\xBF\xFA\xC5\x6F\xA1\xE3\xB7\x7C\xB7\xB8\xA6\xEC\xAA\xBA\xA1\x49");
      return;
    }
    /* 是否要兌換金幣 %d 元 為銀幣 %d (Y/N)？[N]  */
    sprintf(buf, "\xAC\x4F\xA7\x5F\xAD\x6E\xA7\x49\xB4\xAB\xAA\xF7\xB9\xF4 %d \xA4\xB8 \xAC\xB0\xBB\xC8\xB9\xF4 %d (Y/N)\xA1\x48[N] ", gold, money);
  }

  if (vget(19, 0, buf, ans, 3, LCECHO) == 'y')
  {
    if (!way)
    {
      cuser.money -= money;
      addgold(gold);
    }
    else
    {
      cuser.gold -= gold;
      addmoney(money);
    }
    /* 您身上有銀幣 %d 元，金幣 %d 元 */
    sprintf(buf, "\xB1\x7A\xA8\xAD\xA4\x57\xA6\xB3\xBB\xC8\xB9\xF4 %d \xA4\xB8\xA1\x41\xAA\xF7\xB9\xF4 %d \xA4\xB8", cuser.money, cuser.gold);
    vmsg(buf);
  }
  else
  {
    /* 取消交易 */
    vmsg("\xA8\xFA\xAE\xF8\xA5\xE6\xA9\xF6");
  }
}


static void
x_cash()
{
  int fd, money, gold;
  char fpath[64], buf[64];
  FILE *fp;
  PAYCHECK paycheck;

  usr_fpath(fpath, cuser.userid, FN_PAYCHECK);
  if ((fd = open(fpath, O_RDONLY)) < 0)
  {
    /* 您目前沒有支票未兌現 */
    vmsg("\xB1\x7A\xA5\xD8\xAB\x65\xA8\x53\xA6\xB3\xA4\xE4\xB2\xBC\xA5\xBC\xA7\x49\xB2\x7B");
    return;
  }

  usr_fpath(buf, cuser.userid, "cashed");
  fp = fopen(buf, "w");
  /* 以下是您的支票兌換清單：\n\n */
  fputs("\xA5\x48\xA4\x55\xAC\x4F\xB1\x7A\xAA\xBA\xA4\xE4\xB2\xBC\xA7\x49\xB4\xAB\xB2\x4D\xB3\xE6\xA1\x47\n\n", fp);

  money = gold = 0;
  while (read(fd, &paycheck, sizeof(PAYCHECK)) == sizeof(PAYCHECK))
  {
    if (paycheck.money < (INT_MAX - money))	/* 避免溢位 */
      money += paycheck.money;
    else
      money = INT_MAX;
    if (paycheck.gold < (INT_MAX - gold))	/* 避免溢位 */
      gold += paycheck.gold;
    else
      gold = INT_MAX;

    /* %s %s %d 銀 %d 金\n */
    fprintf(fp, "%s %s %d \xBB\xC8 %d \xAA\xF7\n", 
      Btime(paycheck.tissue), paycheck.reason, paycheck.money, paycheck.gold);
  }
  close(fd);
  unlink(fpath);

  /* \n您共兌現 %d 銀 %d 金\n */
  fprintf(fp, "\n\xB1\x7A\xA6\x40\xA7\x49\xB2\x7B %d \xBB\xC8 %d \xAA\xF7\n", money, gold);
  fclose(fp);

  addmoney(money);
  addgold(gold);

  more(buf, NULL);
  unlink(buf);
}


int
x_bank()
{
  char ans[3];

  if (HAS_STATUS(STATUS_COINLOCK))
  {
    vmsg(msg_coinlock);
    return XEASY;
  }

  /* 信託銀行 */
  vs_bar("\xAB\x48\xB0\x55\xBB\xC8\xA6\xE6");
  move(2, 0);

  /* itoc.011208: 以防萬一 */
  if (cuser.money < 0)
    cuser.money = 0;
  if (cuser.gold < 0)
    cuser.gold = 0;

  /* \033[1;36m  ╭═════════════════════════════╮\n */
  outs("\033[1;36m  \xF9\xFA\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xFB\n");
  /*   ║\033[32m您現在有銀幣 \033[33m%12d\033[32m 元，金幣 \033[33m%12d\033[32m 元\033[36m        ║\n */
  prints("  \xF9\xF8\033[32m\xB1\x7A\xB2\x7B\xA6\x62\xA6\xB3\xBB\xC8\xB9\xF4 \033[33m%12d\033[32m \xA4\xB8\xA1\x41\xAA\xF7\xB9\xF4 \033[33m%12d\033[32m \xA4\xB8\033[36m        \xF9\xF8\n", 
    cuser.money, cuser.gold);
  /*   ╠═════════════════════════════╣\n */
  outs("  \xF9\xE0\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xE2\n"
    /*   ║ 目前銀行提供下列幾項服務：                               ║\n */
    "  \xF9\xF8 \xA5\xD8\xAB\x65\xBB\xC8\xA6\xE6\xB4\xA3\xA8\xD1\xA4\x55\xA6\x43\xB4\x58\xB6\xB5\xAA\x41\xB0\xC8\xA1\x47                               \xF9\xF8\n"
    /*   ║\033[33m1.\033[37m 轉帳 -- 轉帳給其他人   (抽取 10% 手續費) \033[36m              ║\n */
    "  \xF9\xF8\033[33m1.\033[37m \xC2\xE0\xB1\x62 -- \xC2\xE0\xB1\x62\xB5\xB9\xA8\xE4\xA5\x4C\xA4\x48   (\xA9\xE2\xA8\xFA 10% \xA4\xE2\xC4\xF2\xB6\x4F) \033[36m              \xF9\xF8\n"
    /*   ║\033[33m2.\033[37m 匯兌 -- 銀幣/金幣 兌換 (抽取 10% 手續費) \033[36m              ║\n */
    "  \xF9\xF8\033[33m2.\033[37m \xB6\xD7\xA7\x49 -- \xBB\xC8\xB9\xF4/\xAA\xF7\xB9\xF4 \xA7\x49\xB4\xAB (\xA9\xE2\xA8\xFA 10% \xA4\xE2\xC4\xF2\xB6\x4F) \033[36m              \xF9\xF8\n"
    /*   ║\033[33m3.\033[37m 兌現 -- 支票兌現                         \033[36m              ║\n */
    "  \xF9\xF8\033[33m3.\033[37m \xA7\x49\xB2\x7B -- \xA4\xE4\xB2\xBC\xA7\x49\xB2\x7B                         \033[36m              \xF9\xF8\n"
    /*   ╰═════════════════════════════╯\033[m */
    "  \xF9\xFC\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xF9\xFD\033[m");

  /* 請輸入您需要的服務： */
  vget(11, 0, "\xBD\xD0\xBF\xE9\xA4\x4A\xB1\x7A\xBB\xDD\xAD\x6E\xAA\xBA\xAA\x41\xB0\xC8\xA1\x47", ans, 3, DOECHO);
  if (ans[0] == '1')
    x_give();
  else if (ans[0] == '2')
    x_exchange();
  else if (ans[0] == '3')
    x_cash();

  return 0;
}


int
b_invis()
{
  if (HAS_STATUS(STATUS_COINLOCK))
  {
    vmsg(msg_coinlock);
    return XEASY;
  }

  if (cuser.ufo & UFO_CLOAK)
  {
    /* 是否現身(Y/N)？[N]  */
    if (vans("\xAC\x4F\xA7\x5F\xB2\x7B\xA8\xAD(Y/N)\xA1\x48[N] ") != 'y')
      return XEASY; 
    /* 現身免費 */
  }
  else
  {
    if (HAS_PERM(PERM_CLOAK))
    {
      /* 是否隱形(Y/N)？[N]  */
      if (vans("\xAC\x4F\xA7\x5F\xC1\xF4\xA7\xCE(Y/N)\xA1\x48[N] ") != 'y')
	return XEASY;
      /* 有無限隱形權限者免費 */
    }
    else
    {
      if (cuser.gold < 10)
      {
	/* 要 10 金幣才能隱形喔 */
	vmsg("\xAD\x6E 10 \xAA\xF7\xB9\xF4\xA4\x7E\xAF\xE0\xC1\xF4\xA7\xCE\xB3\xE1");
	return XEASY;
      }
      /* 是否花 10 金幣隱形(Y/N)？[N]  */
      if (vans("\xAC\x4F\xA7\x5F\xAA\xE1 10 \xAA\xF7\xB9\xF4\xC1\xF4\xA7\xCE(Y/N)\xA1\x48[N] ") != 'y')
	return XEASY;
      cuser.gold -= 10;
    }
  }

  cuser.ufo ^= UFO_CLOAK;
  cutmp->ufo ^= UFO_CLOAK;	/* ufo 要同步 */

  return XEASY;
}


static void
buy_level(userlevel)		/* itoc.010830: 只存 level 欄位，以免變動到在線上更動的認證欄位 */
  usint userlevel;
{
  if (!HAS_STATUS(STATUS_DATALOCK))	/* itoc.010811: 要沒有被站長鎖定，才能寫入 */
  {
    int fd;
    char fpath[80];
    ACCT tuser;

    usr_fpath(fpath, cuser.userid, fn_acct);
    fd = open(fpath, O_RDWR);
    if (fd >= 0)
    {
      if (read(fd, &tuser, sizeof(ACCT)) == sizeof(ACCT))
      {
	tuser.userlevel |= userlevel;
	lseek(fd, (off_t) 0, SEEK_SET);
	write(fd, &tuser, sizeof(ACCT));
	/* 您已經獲得權限，請重新上站 */
	vmsg("\xB1\x7A\xA4\x77\xB8\x67\xC0\xF2\xB1\x6F\xC5\x76\xAD\xAD\xA1\x41\xBD\xD0\xAD\xAB\xB7\x73\xA4\x57\xAF\xB8");
      }
      close(fd);
    }
  }
}


int
b_cloak()
{
  if (HAS_STATUS(STATUS_COINLOCK))
  {
    vmsg(msg_coinlock);
    return XEASY;
  }

  if (HAS_PERM(PERM_CLOAK))
  {
    /* 您已經能無限隱形了 */
    vmsg("\xB1\x7A\xA4\x77\xB8\x67\xAF\xE0\xB5\x4C\xAD\xAD\xC1\xF4\xA7\xCE\xA4\x46");
  }
  else
  {
    if (cuser.gold < 1000)
    {
      /* 要 1000 金幣才能購買無限隱形權限喔 */
      vmsg("\xAD\x6E 1000 \xAA\xF7\xB9\xF4\xA4\x7E\xAF\xE0\xC1\xCA\xB6\x52\xB5\x4C\xAD\xAD\xC1\xF4\xA7\xCE\xC5\x76\xAD\xAD\xB3\xE1");
    }
    /* 是否花 1000 金幣購買無限隱形權限(Y/N)？[N]  */
    else if (vans("\xAC\x4F\xA7\x5F\xAA\xE1 1000 \xAA\xF7\xB9\xF4\xC1\xCA\xB6\x52\xB5\x4C\xAD\xAD\xC1\xF4\xA7\xCE\xC5\x76\xAD\xAD(Y/N)\xA1\x48[N] ") == 'y')
    {
      cuser.gold -= 1000;
      buy_level(PERM_CLOAK);
    }
  }

  return XEASY;
}


int
b_mbox()
{
  if (HAS_STATUS(STATUS_COINLOCK))
  {
    vmsg(msg_coinlock);
    return XEASY;
  }

  if (HAS_PERM(PERM_MBOX))
  {
    /* 您的信箱已經沒有上限了 */
    vmsg("\xB1\x7A\xAA\xBA\xAB\x48\xBD\x63\xA4\x77\xB8\x67\xA8\x53\xA6\xB3\xA4\x57\xAD\xAD\xA4\x46");
  }
  else
  {
    if (cuser.gold < 1000)
    {
      /* 要 1000 金幣才能購買信箱無限權限喔 */
      vmsg("\xAD\x6E 1000 \xAA\xF7\xB9\xF4\xA4\x7E\xAF\xE0\xC1\xCA\xB6\x52\xAB\x48\xBD\x63\xB5\x4C\xAD\xAD\xC5\x76\xAD\xAD\xB3\xE1");
    }
    /* 是否花 1000 金幣購買信箱無限權限(Y/N)？[N]  */
    else if (vans("\xAC\x4F\xA7\x5F\xAA\xE1 1000 \xAA\xF7\xB9\xF4\xC1\xCA\xB6\x52\xAB\x48\xBD\x63\xB5\x4C\xAD\xAD\xC5\x76\xAD\xAD(Y/N)\xA1\x48[N] ") == 'y')
    {
      cuser.gold -= 1000;
      buy_level(PERM_MBOX);
    }
  }

  return XEASY;
}


int
b_xempt()
{
  if (HAS_STATUS(STATUS_COINLOCK))
  {
    vmsg(msg_coinlock);
    return XEASY;
  }

  if (HAS_PERM(PERM_XEMPT))
  {
    /* 您的帳號已經永久保留了 */
    vmsg("\xB1\x7A\xAA\xBA\xB1\x62\xB8\xB9\xA4\x77\xB8\x67\xA5\xC3\xA4\x5B\xAB\x4F\xAF\x64\xA4\x46");
  }
  else
  {
    if (cuser.gold < 1000)
    {
      /* 要 1000 金幣才能購買帳號永久保留權限喔 */
      vmsg("\xAD\x6E 1000 \xAA\xF7\xB9\xF4\xA4\x7E\xAF\xE0\xC1\xCA\xB6\x52\xB1\x62\xB8\xB9\xA5\xC3\xA4\x5B\xAB\x4F\xAF\x64\xC5\x76\xAD\xAD\xB3\xE1");
    }
    /* 是否花 1000 金幣購買帳號永久保留權限(Y/N)？[N]  */
    else if (vans("\xAC\x4F\xA7\x5F\xAA\xE1 1000 \xAA\xF7\xB9\xF4\xC1\xCA\xB6\x52\xB1\x62\xB8\xB9\xA5\xC3\xA4\x5B\xAB\x4F\xAF\x64\xC5\x76\xAD\xAD(Y/N)\xA1\x48[N] ") == 'y')
    {
      cuser.gold -= 1000;
      buy_level(PERM_XEMPT);
    }
  }

  return XEASY;
}


#if 0	/* 不提供購買自殺功能 */
int
b_purge()
{
  if (HAS_STATUS(STATUS_COINLOCK))
  {
    vmsg(msg_coinlock);
    return XEASY;
  }

  if (HAS_PERM(PERM_PURGE))
  {
    /* 系統在下次定期清帳號時，將清除此 ID */
    vmsg("\xA8\x74\xB2\xCE\xA6\x62\xA4\x55\xA6\xB8\xA9\x77\xB4\xC1\xB2\x4D\xB1\x62\xB8\xB9\xAE\xC9\xA1\x41\xB1\x4E\xB2\x4D\xB0\xA3\xA6\xB9 ID");
  }
  else
  {
    if (cuser.gold < 1000)
    {
      /* 要 1000 金幣才能自殺喔 */
      vmsg("\xAD\x6E 1000 \xAA\xF7\xB9\xF4\xA4\x7E\xAF\xE0\xA6\xDB\xB1\xFE\xB3\xE1");
    }
    /* 是否花 1000 金幣自殺(Y/N)？[N]  */
    else if (vans("\xAC\x4F\xA7\x5F\xAA\xE1 1000 \xAA\xF7\xB9\xF4\xA6\xDB\xB1\xFE(Y/N)\xA1\x48[N] ") == 'y')
    {
      cuser.gold -= 1000;
      buy_level(PERM_PURGE);
    }
  }

  return XEASY;
}
#endif
#endif	/* HAVE_BUY */
