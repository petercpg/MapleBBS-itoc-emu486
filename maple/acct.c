/*-------------------------------------------------------*/
/* maple/acct.c         ( NTHU CS MapleBBS Ver 3.00 )    */
/*-------------------------------------------------------*/
/* target : account / administration routines	 	 */
/* create : 95/03/29				 	 */
/* update : 96/04/05				 	 */
/*-------------------------------------------------------*/


#define	_ADMIN_C_


#include "bbs.h"


extern BCACHE *bshm;


/* ----------------------------------------------------- */
/* (.ACCT) 使用者帳號 (account) subroutines		 */
/* ----------------------------------------------------- */


int
acct_load(acct, userid)
  ACCT *acct;
  char *userid;
{
  int fd;

  usr_fpath((char *) acct, userid, fn_acct);
  fd = open((char *) acct, O_RDONLY);
  if (fd >= 0)
  {
    /* Thor.990416: 特別注意, 有時 .ACCT的長度會是0 */
    read(fd, acct, sizeof(ACCT));
    close(fd);
  }
  return fd;
}


/* static */	/* itoc.010408: 給其他程式用 */
void
acct_save(acct)
  ACCT *acct;
{
  int fd;
  char fpath[64];

  /* itoc.010811: 若被站長鎖定，就不能寫回自己的檔案 */
  if ((acct->userno == cuser.userno) && HAS_STATUS(STATUS_DATALOCK) && !HAS_PERM(PERM_ALLACCT))
    return;

  usr_fpath(fpath, acct->userid, fn_acct);
  fd = open(fpath, O_WRONLY, 0600);	/* fpath 必須已經存在 */
  if (fd >= 0)
  {
    write(fd, acct, sizeof(ACCT));
    close(fd);
  }
}


int
acct_userno(userid)
  char *userid;
{
  int fd;
  int userno;
  char fpath[64];

  usr_fpath(fpath, userid, fn_acct);
  fd = open(fpath, O_RDONLY);
  if (fd >= 0)
  {
    read(fd, &userno, sizeof(userno));
    close(fd);
    return userno;
  }
  return 0;
}


/* ----------------------------------------------------- */
/* name complete for user ID				 */
/* ----------------------------------------------------- */
/* return value :					 */
/* 0 : 使用直接按 enter ==> cancel			 */
/* -1 : bad user id					 */
/* ow.: 傳回該 userid 之 userno				 */
/* ----------------------------------------------------- */


int
acct_get(msg, acct)
  char *msg;
  ACCT *acct;
{
  /* ★ 輸入首字母後，可以按空白鍵自動搜尋 */
  outz("\xA1\xB9 \xBF\xE9\xA4\x4A\xAD\xBA\xA6\x72\xA5\xC0\xAB\xE1\xA1\x41\xA5\x69\xA5\x48\xAB\xF6\xAA\xC5\xA5\xD5\xC1\xE4\xA6\xDB\xB0\xCA\xB7\x6A\xB4\x4D");
  
  if (!vget(1, 0, msg, acct->userid, IDLEN + 1, GET_USER))
    return 0;

  if (acct_load(acct, acct->userid) >= 0)
    return acct->userno;

  vmsg(err_uid);
  return -1;
}


/* ----------------------------------------------------- */
/* bit-wise display and setup				 */
/* ----------------------------------------------------- */


/* ■ */
#define BIT_ON		"\xA1\xBD"
/* □ */
#define BIT_OFF		"\xA1\xBC"


void
bitmsg(msg, str, level)
  char *msg, *str;
  int level;
{
  int cc;

  outs(msg);
  while (cc = *str)
  {
    outc((level & 1) ? cc : '-');
    level >>= 1;
    str++;
  }

  outc('\n');
}


usint
bitset(pbits, count, maxon, msg, perms)
  usint pbits;
  int count;			/* 共有幾個選項 */
  int maxon;			/* 最多可以 enable 幾項 */
  char *msg;
  char *perms[];
{
  int i, j, on;

  move(1, 0);
  clrtobot();
  move(3, 0);
  outs(msg);

  for (i = on = 0, j = 1; i < count; i++)
  {
    msg = BIT_OFF;
    if (pbits & j)
    {
      on++;
      msg = BIT_ON;
    }
    move(5 + (i & 15), (i < 16 ? 0 : 40));
    prints("%c %s %s", radix32[i], msg, perms[i]);
    j <<= 1;
  }

  /* 請按鍵切換設定，或按 [Return] 結束： */
  while (i = vans("\xBD\xD0\xAB\xF6\xC1\xE4\xA4\xC1\xB4\xAB\xB3\x5D\xA9\x77\xA1\x41\xA9\xCE\xAB\xF6 [Return] \xB5\xB2\xA7\xF4\xA1\x47"))
  {
    i -= '0';
    if (i >= 10)
      i -= 'a' - '0' - 10;

    if (i >= 0 && i < count)
    {
      j = 1 << i;
      if (pbits & j)
      {
	on--;
	msg = BIT_OFF;
      }
      else
      {
	if (on >= maxon)
	  continue;
	on++;
	msg = BIT_ON;
      }

      pbits ^= j;
      move(5 + (i & 15), (i < 16 ? 2 : 42));
      outs(msg);
    }
  }
  return (pbits);
}


static usint
setperm(level)
  usint level;
{
  if (HAS_PERM(PERM_SYSOP))
    return bitset(level, NUMPERMS, NUMPERMS, MSG_USERPERM, perm_tbl);

  /* [帳號管理員] 不能管 PERM_SYSOP */
  if (level & PERM_SYSOP)
    return level;

  /* [帳號管理員] 不能更改權限 PERM_ACCOUNTS CHATROOM BOARD SYSOP */
  return bitset(level, NUMPERMS - 4, NUMPERMS - 4, MSG_USERPERM, perm_tbl);
}


/* ----------------------------------------------------- */
/* 帳號管理						 */
/* ----------------------------------------------------- */


static void
bm_list(userid)			/* 顯示 userid 是哪些板的板主 */
  char *userid;
{
  int len;
  char *list;
  BRD *bhead, *btail;

  len = strlen(userid);
  /*   \033[32m擔任板主：\033[37m */
  outs("  \033[32m\xBE\xE1\xA5\xF4\xAA\x4F\xA5\x44\xA1\x47\033[37m");		/* itoc.010922: 換 user info 版面 */

  bhead = bshm->bcache;
  btail = bhead + bshm->number;

  do
  {
    list = bhead->BM;
    if (str_has(list, userid, len))
    {
      outs(bhead->brdname);
      outc(' ');
    }
  } while (++bhead < btail);

  outc('\n');
}


static void
adm_log(old, new)
  ACCT *old, *new;
{
  int i;
  usint bit, oldl, newl;
  char *userid, buf[80];

  userid = new->userid;
  /* 異動資料 */
  alog("\xB2\xA7\xB0\xCA\xB8\xEA\xAE\xC6", userid);

  if (strcmp(old->passwd, new->passwd))
    /* 異動密碼 */
    alog("\xB2\xA7\xB0\xCA\xB1\x4B\xBD\x58", userid);

  if ((old->money != new->money) || (old->gold != new->gold))
  {
    /* %-13s銀%d→%d 金%d→%d */
    sprintf(buf, "%-13s\xBB\xC8%d\xA1\xF7%d \xAA\xF7%d\xA1\xF7%d", userid, old->money, new->money, old->gold, new->gold);
    /* 異動錢幣 */
    alog("\xB2\xA7\xB0\xCA\xBF\xFA\xB9\xF4", buf);
  }

  /* Thor.990405: log permission modify */
  oldl = old->userlevel;
  newl = new->userlevel;
  for (i = 0, bit = 1; i < NUMPERMS; i++, bit <<= 1)
  {
    if ((newl & bit) != (oldl & bit))
    {
      sprintf(buf, "%-13s%s %s", userid, (newl & bit) ? BIT_ON : BIT_OFF, perm_tbl[i]);
      /* 異動權限 */
      alog("\xB2\xA7\xB0\xCA\xC5\x76\xAD\xAD", buf);
    }
  }
}


void
acct_show(u, adm)
  ACCT *u;
  int adm;			/* 0: user info  1: admin  2: reg-form */
{
  int diff;
  usint ulevel;
  char *uid, buf[80];

  clrtobot();

  /* itoc.010922: 換 user info 版面 */
  if (adm == 0)
  {
    /* \n        \033[30;41m┬┴┬┴┬┴\033[m  \033[45m╰╦╦╮╭╦═╮ */
    outs("\n        \033[30;41m\xA2\x73\xA2\x72\xA2\x73\xA2\x72\xA2\x73\xA2\x72\033[m  \033[45m\xF9\xFC\xF9\xDE\xF9\xDE\xF9\xFB\xF9\xFA\xF9\xDE\xF9\xF9\xF9\xFB"
      /* ╔╦═╮╭╦═╮\033[m  \033[30;41m┬┴┬┴┬┴\033[m\n */
      "\xF9\xDD\xF9\xDE\xF9\xF9\xF9\xFB\xF9\xFA\xF9\xDE\xF9\xF9\xF9\xFB\033[m  \033[30;41m\xA2\x73\xA2\x72\xA2\x73\xA2\x72\xA2\x73\xA2\x72\033[m\n"
      /*         \033[30;41m┴┬┴┬┴┬\033[m  \033[1;37;45m  ╠╣  ╠╣ */
      "        \033[30;41m\xA2\x72\xA2\x73\xA2\x72\xA2\x73\xA2\x72\xA2\x73\033[m  \033[1;37;45m  \xF9\xE0\xF9\xE2  \xF9\xE0\xF9\xE2"
      /*   ║╠╬╣  ╠╣  ║\033[m  \033[30;41m┴┬┴┬┴┬\033[m\n */
      "  \xF9\xF8\xF9\xE0\xF9\xE1\xF9\xE2  \xF9\xE0\xF9\xE2  \xF9\xF8\033[m  \033[30;41m\xA2\x72\xA2\x73\xA2\x72\xA2\x73\xA2\x72\xA2\x73\033[m\n"
      /*         \033[30;41m┬┴┬┴┬┴\033[m  \033[45m  ╠╣  ╠╣  ║ */
      "        \033[30;41m\xA2\x73\xA2\x72\xA2\x73\xA2\x72\xA2\x73\xA2\x72\033[m  \033[45m  \xF9\xE0\xF9\xE2  \xF9\xE0\xF9\xE2  \xF9\xF8"
      /* ╠╣╯  ╠╣  ║\033[m  \033[30;41m┬┴┬┴┬┴\033[m\n */
      "\xF9\xE0\xF9\xE2\xF9\xFD  \xF9\xE0\xF9\xE2  \xF9\xF8\033[m  \033[30;41m\xA2\x73\xA2\x72\xA2\x73\xA2\x72\xA2\x73\xA2\x72\033[m\n"
      /*         \033[30;41m┴┬┴┬┴┬\033[m  \033[1;30;45m╰╩╩╮╚╝ */
      "        \033[30;41m\xA2\x72\xA2\x73\xA2\x72\xA2\x73\xA2\x72\xA2\x73\033[m  \033[1;30;45m\xF9\xFC\xF9\xE4\xF9\xE4\xF9\xFB\xF9\xE3\xF9\xE5"
      /*   ╰╚╝    ╰╩═╯\033[m  \033[30;41m┴┬┴┬┴┬\033[m\n */
      "  \xF9\xFC\xF9\xE3\xF9\xE5    \xF9\xFC\xF9\xE4\xF9\xF9\xF9\xFD\033[m  \033[30;41m\xA2\x72\xA2\x73\xA2\x72\xA2\x73\xA2\x72\xA2\x73\033[m\n");
  }

  uid = u->userid;

  outs("\n\033[1m");

  /* itoc.010408: 新增金錢/生日/性別欄位 */

  if (adm != 2)
    /*   \033[32m英文代號：\033[37m%-35s\033[32m用戶編號：\033[37m%d\n */
    prints("  \033[32m\xAD\x5E\xA4\xE5\xA5\x4E\xB8\xB9\xA1\x47\033[37m%-35s\033[32m\xA5\xCE\xA4\xE1\xBD\x73\xB8\xB9\xA1\x47\033[37m%d\n", uid, u->userno);

  /*   \033[32m我的暱稱：\033[37m%-35s\033[32m擁有銀幣：\033[37m%d\n */
  prints("  \033[32m\xA7\xDA\xAA\xBA\xBC\xCA\xBA\xD9\xA1\x47\033[37m%-35s\033[32m\xBE\xD6\xA6\xB3\xBB\xC8\xB9\xF4\xA1\x47\033[37m%d\n", u->username, u->money);

  /*   \033[32m真實姓名：\033[37m%-35s\033[32m擁有金幣：\033[37m%d\n */
  prints("  \033[32m\xAF\x75\xB9\xEA\xA9\x6D\xA6\x57\xA1\x47\033[37m%-35s\033[32m\xBE\xD6\xA6\xB3\xAA\xF7\xB9\xF4\xA1\x47\033[37m%d\n", u->realname, u->gold);

  /*   \033[32m出生日期：\033[37m民國 %02d 年 %02d 月 %02d 日             \033[32m我的性別：\033[37m%.2s\n */
  /* ？♂♀ */
  prints("  \033[32m\xA5\x58\xA5\xCD\xA4\xE9\xB4\xC1\xA1\x47\033[37m\xA5\xC1\xB0\xEA %02d \xA6\x7E %02d \xA4\xEB %02d \xA4\xE9             \033[32m\xA7\xDA\xAA\xBA\xA9\xCA\xA7\x4F\xA1\x47\033[37m%.2s\n", u->year, u->month, u->day, "\xA1\x48\xA1\xF1\xA1\xF0" + (u->sex << 1));

  /*   \033[32m上站次數：\033[37m%-35d\033[32m文章篇數：\033[37m%d\n */
  prints("  \033[32m\xA4\x57\xAF\xB8\xA6\xB8\xBC\xC6\xA1\x47\033[37m%-35d\033[32m\xA4\xE5\xB3\xB9\xBD\x67\xBC\xC6\xA1\x47\033[37m%d\n", u->numlogins, u->numposts);

  /*   \033[32m郵件信箱：\033[37m%s\n */
  prints("  \033[32m\xB6\x6C\xA5\xF3\xAB\x48\xBD\x63\xA1\x47\033[37m%s\n", u->email);

  /*   \033[32m註冊日期：\033[37m%s\n */
  prints("  \033[32m\xB5\xF9\xA5\x55\xA4\xE9\xB4\xC1\xA1\x47\033[37m%s\n", Btime(u->firstlogin));

  /*   \033[32m光臨日期：\033[37m%s\n */
  prints("  \033[32m\xA5\xFA\xC1\x7B\xA4\xE9\xB4\xC1\xA1\x47\033[37m%s\n", Btime(u->lastlogin));

  ulevel = u->userlevel;

  if (ulevel & PERM_ALLDENY)
  {
    /* yiting: 顯示停權天數 */
    /*   \033[32m停權天數：\033[37m */
    outs("  \033[32m\xB0\xB1\xC5\x76\xA4\xD1\xBC\xC6\xA1\x47\033[37m");
    if ((diff = u->tvalid - time(0)) < 0)
    {
      /* 停權期限已到，可自行申請復權\n */
      outs("\xB0\xB1\xC5\x76\xB4\xC1\xAD\xAD\xA4\x77\xA8\xEC\xA1\x41\xA5\x69\xA6\xDB\xA6\xE6\xA5\xD3\xBD\xD0\xB4\x5F\xC5\x76\n");
    }
    else
    {
      /* 不滿一小時的部份加一小時計算，這樣顯示0小時就表示可以去復權了 */
      diff += 3600;
      /* 還有 %d 天 %d 小時\n */
      prints("\xC1\xD9\xA6\xB3 %d \xA4\xD1 %d \xA4\x70\xAE\xC9\n", diff / 86400, (diff % 86400) / 3600);
    }
  }
  else
  {
    /*   \033[32m身分認證：\033[37m%s\n */
    /* 請參考本站公佈欄進行確認，以提昇權限 */
    prints("  \033[32m\xA8\xAD\xA4\xC0\xBB\x7B\xC3\xD2\xA1\x47\033[37m%s\n", (ulevel & PERM_VALID) ? Btime(u->tvalid) : "\xBD\xD0\xB0\xD1\xA6\xD2\xA5\xBB\xAF\xB8\xA4\xBD\xA7\x47\xC4\xE6\xB6\x69\xA6\xE6\xBD\x54\xBB\x7B\xA1\x41\xA5\x48\xB4\xA3\xAA\x40\xC5\x76\xAD\xAD");
  }

  usr_fpath(buf, uid, fn_dir);
  /*   \033[32m個人信件：\033[37m%d 封\n */
  prints("  \033[32m\xAD\xD3\xA4\x48\xAB\x48\xA5\xF3\xA1\x47\033[37m%d \xAB\xCA\n", rec_num(buf, sizeof(HDR)));

  if (adm)
  {
    /*   \033[32m上站地點：\033[37m%-35s\033[32m發信次數：\033[37m%d\n */
    prints("  \033[32m\xA4\x57\xAF\xB8\xA6\x61\xC2\x49\xA1\x47\033[37m%-35s\033[32m\xB5\x6F\xAB\x48\xA6\xB8\xBC\xC6\xA1\x47\033[37m%d\n", u->lasthost, u->numemails);
    /*   \033[32m權限等級：\033[37m */
    bitmsg("  \033[32m\xC5\x76\xAD\xAD\xB5\xA5\xAF\xC5\xA1\x47\033[37m", STR_PERM, ulevel);
    /*   \033[32m習慣旗標：\033[37m */
    bitmsg("  \033[32m\xB2\xDF\xBA\x44\xBA\x58\xBC\xD0\xA1\x47\033[37m", STR_UFO, u->ufo);
  }
  else
  {
    diff = (time(0) - ap_start) / 60;
    /*   \033[32m停留期間：\033[37m%d 小時 %d 分\n */
    prints("  \033[32m\xB0\xB1\xAF\x64\xB4\xC1\xB6\xA1\xA1\x47\033[37m%d \xA4\x70\xAE\xC9 %d \xA4\xC0\n", diff / 60, diff % 60);
  }

  if (adm == 2)
    goto end_show;

  /* Thor: 想看看這個 user 是那些板的板主 */

  if (ulevel & PERM_BM)
    bm_list(uid);

#ifdef NEWUSER_LIMIT
  if (u->lastlogin - u->firstlogin < 3 * 86400)
    /* \n  \033[36m新手上路：三天後開放權限\n */
    outs("\n  \033[36m\xB7\x73\xA4\xE2\xA4\x57\xB8\xF4\xA1\x47\xA4\x54\xA4\xD1\xAB\xE1\xB6\x7D\xA9\xF1\xC5\x76\xAD\xAD\n");
#endif

end_show:
  outs("\033[m");
}


void
acct_setup(u, adm)
  ACCT *u;
  int adm;
{
  ACCT x;
  int i, num;
  char *str, buf[80], pass[PSWDLEN + 1];

  acct_show(u, adm);
  memcpy(&x, u, sizeof(ACCT));

  if (adm)
  {
    /* 設定 1)資料 2)權限 Q)取消 [Q]  */
    adm = vans("\xB3\x5D\xA9\x77 1)\xB8\xEA\xAE\xC6 2)\xC5\x76\xAD\xAD Q)\xA8\xFA\xAE\xF8 [Q] ");
    if (adm == '2')
      goto set_perm;

    if (adm != '1')
      return;
  }
  else
  {
    /* 修改資料(Y/N)？[N]  */
    if (vans("\xAD\xD7\xA7\xEF\xB8\xEA\xAE\xC6(Y/N)\xA1\x48[N] ") != 'y')
      return;
  }

  move(i = 3, 0);
  clrtobot();

  if (adm)
  {
    str = x.userid;
    for (;;)
    {
      /* itoc.010804.註解: 改使用者代號時請確定該 user 不在站上 */
      /* 使用者代號(不改請按 Enter)： */
      vget(i, 0, "\xA8\xCF\xA5\xCE\xAA\xCC\xA5\x4E\xB8\xB9(\xA4\xA3\xA7\xEF\xBD\xD0\xAB\xF6 Enter)\xA1\x47", str, IDLEN + 1, GCARRY);
      if (!str_cmp(str, u->userid) || !acct_userno(str))
	break;
      /* 錯誤！已有相同 ID 的使用者 */
      vmsg("\xBF\xF9\xBB\x7E\xA1\x49\xA4\x77\xA6\xB3\xAC\xDB\xA6\x50 ID \xAA\xBA\xA8\xCF\xA5\xCE\xAA\xCC");
    }
  }
  else
  {
    /* 請確認密碼： */
    vget(i, 0, "\xBD\xD0\xBD\x54\xBB\x7B\xB1\x4B\xBD\x58\xA1\x47", buf, PSWDLEN + 1, NOECHO);
    if (chkpasswd(u->passwd, buf))
    {
      /* 密碼錯誤 */
      vmsg("\xB1\x4B\xBD\x58\xBF\xF9\xBB\x7E");
      return;
    }
  }

  /* itoc.030223: 只有 PERM_SYSOP 能變更其他站務的密碼 */
  if (!adm || !(u->userlevel & PERM_ALLADMIN) || HAS_PERM(PERM_SYSOP))
  {
    i++;
    for (;;)
    {
      /* 設定新密碼(不改請按 Enter)： */
      if (!vget(i, 0, "\xB3\x5D\xA9\x77\xB7\x73\xB1\x4B\xBD\x58(\xA4\xA3\xA7\xEF\xBD\xD0\xAB\xF6 Enter)\xA1\x47", buf, PSWDLEN + 1, NOECHO))
	break;

      strcpy(pass, buf);
      /* 檢查新密碼： */
      vget(i + 1, 0, "\xC0\xCB\xAC\x64\xB7\x73\xB1\x4B\xBD\x58\xA1\x47", buf, PSWDLEN + 1, NOECHO);
      if (!strcmp(buf, pass))
      {
	str_ncpy(x.passwd, genpasswd(buf), sizeof(x.passwd));
	break;
      }
    }
  }

  i++;
  str = x.username;
  while (1)
  {
    /* 暱    稱： */
    if (vget(i, 0, "\xBC\xCA    \xBA\xD9\xA1\x47", str, UNLEN + 1, GCARRY))
      break;
  };

  /* itoc.010408: 新增生日/性別欄位，不強迫使用者填 (允許填 0) */
  i++;
  do
  {
    /* 生日－民國 %02d 年： */
    sprintf(buf, "\xA5\xCD\xA4\xE9\xA1\xD0\xA5\xC1\xB0\xEA %02d \xA6\x7E\xA1\x47", u->year);
    if (!vget(i, 0, buf, buf, 3, DOECHO))
      break;
    x.year = atoi(buf);
  } while (x.year < 0 || x.year > 99);
  do
  {
    /* 生日－ %02d 月： */
    sprintf(buf, "\xA5\xCD\xA4\xE9\xA1\xD0 %02d \xA4\xEB\xA1\x47", u->month);
    if (!vget(i, 0, buf, buf, 3, DOECHO))
      break;
    x.month = atoi(buf);
  } while (x.month < 0 || x.month > 12);
  do
  {
    /* 生日－ %02d 日： */
    sprintf(buf, "\xA5\xCD\xA4\xE9\xA1\xD0 %02d \xA4\xE9\xA1\x47", u->day);
    if (!vget(i, 0, buf, buf, 3, DOECHO))
      break;
    x.day = atoi(buf);
  } while (x.day < 0 || x.day > 31);

  i++;
  /* 性別 (0)中性 (1)男性 (2)女性：[%d]  */
  sprintf(buf, "\xA9\xCA\xA7\x4F (0)\xA4\xA4\xA9\xCA (1)\xA8\x6B\xA9\xCA (2)\xA4\x6B\xA9\xCA\xA1\x47[%d] ", u->sex);
  if (vget(i, 0, buf, buf, 3, DOECHO))
    x.sex = (*buf - '0') & 3;

  if (adm)
  {
    /* itoc.010317: 不讓 user 改姓名 */
    i++;
    str = x.realname;
    do
    {
      /* 真實姓名： */
      vget(i, 0, "\xAF\x75\xB9\xEA\xA9\x6D\xA6\x57\xA1\x47", str, RNLEN + 1, GCARRY);
    } while (strlen(str) < 4);

    sprintf(buf, "%d", u->userno);
    /* 用戶編號： */
    vget(++i, 0, "\xA5\xCE\xA4\xE1\xBD\x73\xB8\xB9\xA1\x47", buf, 10, GCARRY);
    if ((num = atoi(buf)) > 0)
      x.userno = num;

    sprintf(buf, "%d", u->numlogins);
    /* 上線次數： */
    vget(++i, 0, "\xA4\x57\xBD\x75\xA6\xB8\xBC\xC6\xA1\x47", buf, 10, GCARRY);
    if ((num = atoi(buf)) >= 0)
      x.numlogins = num;

    sprintf(buf, "%d", u->numposts);
    /* 文章篇數： */
    vget(++i, 0, "\xA4\xE5\xB3\xB9\xBD\x67\xBC\xC6\xA1\x47", buf, 10, GCARRY);
    if ((num = atoi(buf)) >= 0)
      x.numposts = num;

    /* itoc.010408: 新增金錢欄位 */
    sprintf(buf, "%d", u->money);
    /* 銀    幣： */
    vget(++i, 0, "\xBB\xC8    \xB9\xF4\xA1\x47", buf, 10, GCARRY);
    if ((num = atoi(buf)) >= 0)
      x.money = num;

    sprintf(buf, "%d", u->gold);
    /* 金    幣： */
    vget(++i, 0, "\xAA\xF7    \xB9\xF4\xA1\x47", buf, 10, GCARRY);
    if ((num = atoi(buf)) >= 0)
      x.gold = num;

    sprintf(buf, "%d", u->numemails);
    /* 發信次數： */
    vget(++i, 0, "\xB5\x6F\xAB\x48\xA6\xB8\xBC\xC6\xA1\x47", buf, 10, GCARRY);
    if ((num = atoi(buf)) >= 0)
      x.numemails = num;

    /* 上站地點： */
    vget(++i, 0, "\xA4\x57\xAF\xB8\xA6\x61\xC2\x49\xA1\x47", x.lasthost, sizeof(x.lasthost), GCARRY);
    /* 郵件信箱： */
    vget(++i, 0, "\xB6\x6C\xA5\xF3\xAB\x48\xBD\x63\xA1\x47", x.email, sizeof(x.email), GCARRY);

    /* 設定習慣(Y/N)？[N]  */
    if (vans("\xB3\x5D\xA9\x77\xB2\xDF\xBA\x44(Y/N)\xA1\x48[N] ") == 'y')
      x.ufo = bitset(x.ufo, NUMUFOS, NUMUFOS, MSG_USERUFO, ufo_tbl);

    /* 設定權限(Y/N)？[N]  */
    if (vans("\xB3\x5D\xA9\x77\xC5\x76\xAD\xAD(Y/N)\xA1\x48[N] ") == 'y')
    {
set_perm:

      i = setperm(num = x.userlevel);

      if (i == num)
      {
	/* 取消修改 */
	vmsg("\xA8\xFA\xAE\xF8\xAD\xD7\xA7\xEF");
	if (adm == '2')
	  return;
      }
      else
      {
	x.userlevel = i;

	/* itoc.011120: 站長放水加上認證通過權限，要附加改認證時間 */
	if ((i & PERM_VALID) && !(num & PERM_VALID))
	  time(&x.tvalid);

	/* itoc.050413: 如果站長手動停權，就要由站長才能來復權 */
	if ((i & PERM_ALLDENY) && (i & PERM_ALLDENY) != (num & PERM_ALLDENY))
	  x.tvalid = INT_MAX;
      }
    }
  }

  if (!memcmp(&x, u, sizeof(ACCT)) || vans(msg_sure_ny) != 'y')
    return;

  if (adm)
  {
    if (str_cmp(u->userid, x.userid))
    { /* Thor: 980806: 特別注意如果 usr每個字母不在同一partition的話會有問題 */
      char dst[80];

      usr_fpath(buf, u->userid, NULL);
      usr_fpath(dst, x.userid, NULL);
      rename(buf, dst);
      /* Thor.990416: 特別注意! .USR並未一併更新, 可能有部分問題 */
    }

    /* itoc.010811: 動態設定線上使用者 */
    /* 被站長改過資料的線上使用者(包括站長自己)，其 cutmp->status 會被加上 STATUS_DATALOCK
       這個旗標，就無法 acct_save()，於是站長便可以修改線上使用者資料 */
    /* 在站長修改過才上線的 ID 因為其 cutmp->status 沒有 STATUS_DATALOCK 的旗標，
       所以將可以繼續存取，所以線上如果同時有修改前、修改後的同一隻 ID multi-login，也是無妨。 */
    utmp_admset(x.userno, STATUS_DATALOCK | STATUS_COINLOCK);

    /* lkchu.981201: security log */
    adm_log(u, &x);
  }
  else
  {
    /* itoc.010804.註解: 線上的 userlevel/tvalid 是舊的，.ACCT 裡才是新的 */
    if (acct_load(u, x.userid) >= 0)
    {
      x.userlevel = u->userlevel;
      x.tvalid = u->tvalid;
    }
  }

  memcpy(u, &x, sizeof(ACCT));
  acct_save(u);
}


#if 0	/* itoc.010805.註解 */

  認證成功只加上 PERM_VALID，讓 user 在下次進站才自動得到 PERM_POST | PERM_PAGE | PERM_CHAT
  以免新手上路、停權的功能失效

  但重填 email 拿掉認證者需拿掉 PERM_VALID | PERM_POST | PERM_PAGE | PERM_CHAT
  否則 user 可以在下次進站前任意使用 bbs_post

#endif

#if 0	/* itoc.010831.註解 */

  因為線上 cuser.userlevel 並不是最新的，使用者如果在線上認證或是被停權，
  硬碟中的 .ACCT 寫的才是正確的 userlevel，
  所以要先讀出 .ACCT，加入 level 後再蓋回去。

  使用 acct_seperm(&acct, adm) 之前要先 acct_load(&acct, userid)，
  其中 &acct 不能是 &cuser。
  使用者要重新上站才會換成新的權限。

#endif

void
acct_setperm(u, levelup, leveldown)	/* itoc.000219: 加/減權限程式 */
  ACCT *u;
  usint levelup;		/* 加權限 */
  usint leveldown;		/* 減權限 */
{
  u->userlevel |= levelup;
  u->userlevel &= ~leveldown;

  acct_save(u);
}


/* ----------------------------------------------------- */
/* 增加金銀幣						 */
/* ----------------------------------------------------- */


void
addmoney(addend)
  int addend;
{
  if (addend < (INT_MAX - cuser.money))	/* 避免溢位 */
    cuser.money += addend;
  else
    cuser.money = INT_MAX;
}


void
addgold(addend)
  int addend;
{
  if (addend < (INT_MAX - cuser.gold))	/* 避免溢位 */
    cuser.gold += addend;
  else
    cuser.gold = INT_MAX;
}


/* ----------------------------------------------------- */
/* 看板管理						 */
/* ----------------------------------------------------- */


#ifndef HAVE_COSIGN
static
#endif
int			/* 1:合法的板名 */
valid_brdname(brd)
  char *brd;
{
  int ch;

  if (!is_alnum(*brd))
    return 0;

  while (ch = *++brd)
  {
    if (!is_alnum(ch) && ch != '.' && ch != '-' && ch != '_')
      return 0;
  }
  return 1;
}


static int
brd_set(brd, row)
  BRD *brd;
  int row;
{
  int i, BMlen, len;
  char *brdname, buf[80], userid[IDLEN + 2];
  ACCT acct;

  i = row;
  brdname = brd->brdname;
  strcpy(buf, brdname);

  for (;;)
  {
    if (!vget(i, 0, MSG_BID, brdname, BNLEN + 1, GCARRY))
    {
      if (i == 1)	/* 開新板若無輸入板名表示離開 */
	return -1;

      strcpy(brdname, buf);	/* Thor: 若是清空則設為原名稱 */
      continue;
    }

    if (!valid_brdname(brdname))
      continue;

    if (!str_cmp(buf, brdname))	/* Thor: 與舊板原名相同則跳過 */
      break;

    if (brd_bno(brdname) >= 0)
      /* \n錯誤！板名雷同 */
      outs("\n\xBF\xF9\xBB\x7E\xA1\x49\xAA\x4F\xA6\x57\xB9\x70\xA6\x50");
    else
      break;
  }

  /* 看板分類： */
  vget(++i, 0, "\xAC\xDD\xAA\x4F\xA4\xC0\xC3\xFE\xA1\x47", brd->class, BCLEN + 1, GCARRY);
  /* 看板主題： */
  vget(++i, 0, "\xAC\xDD\xAA\x4F\xA5\x44\xC3\x44\xA1\x47", brd->title, BTLEN + 1, GCARRY);

  /* vget(++i, 0, "板主名單：", brd->BM, BMLEN + 1, GCARRY); */

  /* itoc.010212: 開新板/修改看板自動加上板主權限. */
  /* 目前的作法是一輸入完 id 就加入板主權限，即使最後選擇不變動，
     如果因此多加了板主權限，在 reaper.c 中拿下 */

  i += 4;
  move(i - 2, 0);
  /* 目前板主為 %s\n請輸入新的板主名單，或按 [Return] 不改 */
  prints("\xA5\xD8\xAB\x65\xAA\x4F\xA5\x44\xAC\xB0 %s\n\xBD\xD0\xBF\xE9\xA4\x4A\xB7\x73\xAA\xBA\xAA\x4F\xA5\x44\xA6\x57\xB3\xE6\xA1\x41\xA9\xCE\xAB\xF6 [Return] \xA4\xA3\xA7\xEF", brd->BM);

  strcpy(buf, brd->BM);
  BMlen = strlen(buf);

  /* 請輸入板主，結束請按 Enter，清掉所有板主請打「無」： */
  while (vget(i, 0, "\xBD\xD0\xBF\xE9\xA4\x4A\xAA\x4F\xA5\x44\xA1\x41\xB5\xB2\xA7\xF4\xBD\xD0\xAB\xF6 Enter\xA1\x41\xB2\x4D\xB1\xBC\xA9\xD2\xA6\xB3\xAA\x4F\xA5\x44\xBD\xD0\xA5\xB4\xA1\x75\xB5\x4C\xA1\x76\xA1\x47", userid, IDLEN + 1, DOECHO))
  {
    /* 無 */
    if (!strcmp(userid, "\xB5\x4C"))
    {
      buf[0] = '\0';
      BMlen = 0;
    }
    else if (is_bm(buf, userid))	/* 刪除舊有的板主 */
    {
      len = strlen(userid);
      if (BMlen == len)
      {
	buf[0] = '\0';
      }
      else if (!str_cmp(buf + BMlen - len, userid) && buf[BMlen - len - 1] == '/')	/* 名單上最後一位，ID 後面不接 '/' */
      {
	buf[BMlen - len - 1] = '\0';			/* 刪除 ID 及前面的 '/' */
	len++;
      }
      else						/* ID 後面會接 '/' */
      {
	str_lower(userid, userid);
	strcat(userid, "/");
	len++;
	brdname = str_str(buf, userid);
        strcpy(brdname, brdname + len);
      }
      BMlen -= len;
    }
    else if (acct_load(&acct, userid) >= 0 && !is_bm(buf, userid))	/* 輸入新板主 */
    {
      len = strlen(userid);
      if (BMlen)
      {
	len++;		/* '/' + userid */
	if (BMlen + len > BMLEN)
	{
	  /* 板主名單過長，無法將這 ID 設為板主 */
	  vmsg("\xAA\x4F\xA5\x44\xA6\x57\xB3\xE6\xB9\x4C\xAA\xF8\xA1\x41\xB5\x4C\xAA\x6B\xB1\x4E\xB3\x6F ID \xB3\x5D\xAC\xB0\xAA\x4F\xA5\x44");
	  continue;
	}
	sprintf(buf + BMlen, "/%s", acct.userid);
	BMlen += len;
      }
      else
      {
	strcpy(buf, acct.userid);
	BMlen = len;
      }      

      acct_setperm(&acct, PERM_BM, 0);
    }
    else
      continue;

    move(i - 2, 0);
    /* 目前板主為 %s */
    prints("\xA5\xD8\xAB\x65\xAA\x4F\xA5\x44\xAC\xB0 %s", buf);
    clrtoeol();
  }
  strcpy(brd->BM, buf);


#ifdef HAVE_MODERATED_BOARD
  /* itoc.011208: 改用較便利的看板權限設定 */
  /* 看板權限 A)一般 B)自定 C)秘密 D)好友？[Q]  */
  switch (vget(++i, 0, "\xAC\xDD\xAA\x4F\xC5\x76\xAD\xAD A)\xA4\x40\xAF\xEB B)\xA6\xDB\xA9\x77 C)\xAF\xB5\xB1\x4B D)\xA6\x6E\xA4\xCD\xA1\x48[Q] ", buf, 3, LCECHO))
  {
  case 'c':
    brd->readlevel = PERM_SYSOP;	/* 秘密看板 */
    brd->postlevel = 0;
    brd->battr |= (BRD_NOSTAT | BRD_NOVOTE);
    break;

  case 'd':
    brd->readlevel = PERM_BOARD;	/* 好友看板 */
    brd->postlevel = 0;
    brd->battr |= (BRD_NOSTAT | BRD_NOVOTE);
    break;
#else
  /* 看板權限 A)一般 B)自定？[Q]  */
  switch (vget(++i, 0, "\xAC\xDD\xAA\x4F\xC5\x76\xAD\xAD A)\xA4\x40\xAF\xEB B)\xA6\xDB\xA9\x77\xA1\x48[Q] ", buf, 3, LCECHO))
  {
#endif

  case 'a':
    brd->readlevel = 0;
    brd->postlevel = PERM_POST;		/* 一般看板發表權限為 PERM_POST */
    brd->battr &= ~(BRD_NOSTAT | BRD_NOVOTE);	/* 拿掉好友＆秘密板屬性 */
    break;

  case 'b':
    /* 閱讀權限(Y/N)？[N]  */
    if (vget(++i, 0, "\xBE\x5C\xC5\xAA\xC5\x76\xAD\xAD(Y/N)\xA1\x48[N] ", buf, 3, LCECHO) == 'y')
    {
      brd->readlevel = bitset(brd->readlevel, NUMPERMS, NUMPERMS, MSG_READPERM, perm_tbl);
      move(2, 0);
      clrtobot();
      i = 1;
    }

    /* 發表權限(Y/N)？[N]  */
    if (vget(++i, 0, "\xB5\x6F\xAA\xED\xC5\x76\xAD\xAD(Y/N)\xA1\x48[N] ", buf, 3, LCECHO) == 'y')
    {
      brd->postlevel = bitset(brd->postlevel, NUMPERMS, NUMPERMS, MSG_POSTPERM, perm_tbl);
      move(2, 0);
      clrtobot();
      i = 1;
    }
    break;

  default:	/* 預設不變動 */
    break;
  }

  /* 設定屬性(Y/N)？[N]  */
  if (vget(++i, 0, "\xB3\x5D\xA9\x77\xC4\xDD\xA9\xCA(Y/N)\xA1\x48[N] ", buf, 3, LCECHO) == 'y')
    brd->battr = bitset(brd->battr, NUMBATTRS, NUMBATTRS, MSG_BRDATTR, battr_tbl);

  return 0;
}


int			/* 0:開板成功 -1:開板失敗 */
brd_new(brd)
  BRD *brd;
{
  int bno;
  char fpath[64];

  /* 建立新板 */
  vs_bar("\xAB\xD8\xA5\xDF\xB7\x73\xAA\x4F");

  if (brd_set(brd, 1))
    return -1;

  if (vans(msg_sure_ny) != 'y')
    return -1;

  if (brd_bno(brd->brdname) >= 0)
  {
    /* 錯誤！板名雷同，可能有其他站務剛開啟此板 */
    vmsg("\xBF\xF9\xBB\x7E\xA1\x49\xAA\x4F\xA6\x57\xB9\x70\xA6\x50\xA1\x41\xA5\x69\xAF\xE0\xA6\xB3\xA8\xE4\xA5\x4C\xAF\xB8\xB0\xC8\xAD\xE8\xB6\x7D\xB1\xD2\xA6\xB9\xAA\x4F");
    return -1;
  }

  time(&brd->bstamp);
  if ((bno = brd_bno("")) >= 0)
  {
    rec_put(FN_BRD, brd, sizeof(BRD), bno, NULL);
  }
  /* Thor.981102: 防止超過shm看板個數 */
  else if (bshm->number >= MAXBOARD)
  {
    /* 超過系統所能容納看板個數，請調整系統參數 */
    vmsg("\xB6\x57\xB9\x4C\xA8\x74\xB2\xCE\xA9\xD2\xAF\xE0\xAE\x65\xAF\xC7\xAC\xDD\xAA\x4F\xAD\xD3\xBC\xC6\xA1\x41\xBD\xD0\xBD\xD5\xBE\xE3\xA8\x74\xB2\xCE\xB0\xD1\xBC\xC6");
    return -1;
  }
  else if (rec_add(FN_BRD, brd, sizeof(BRD)) < 0)
  {
    /* 無法建立新板 */
    vmsg("\xB5\x4C\xAA\x6B\xAB\xD8\xA5\xDF\xB7\x73\xAA\x4F");
    return -1;
  }

  gem_fpath(fpath, brd->brdname, NULL);
  mak_dirs(fpath);
  mak_dirs(fpath + 4);

  bshm_reload();		/* force reload of bcache */

  brh_save();
  board_main();			/* reload brd_bits[] */

  return 0;
}


static void
brd_classchange(folder, oldname, newbrd)	/* itoc.020117: 異動 @Class 中的看板 */
  char *folder;
  char *oldname;
  BRD *newbrd;		/* 若為 NULL，表示要刪除看板 */
{
  int pos, xmode;
  char fpath[64];
  HDR hdr;

  pos = 0;
  while (!rec_get(folder, &hdr, sizeof(HDR), pos))
  {
    xmode = hdr.xmode & (GEM_BOARD | GEM_FOLDER);

    if (xmode == (GEM_BOARD | GEM_FOLDER))	/* 看板精華區捷徑 */
    {
      if (!strcmp(hdr.xname, oldname))
      {
	if (newbrd)	/* 看板更名 */
	{
	  brd2gem(newbrd, &hdr);
	  rec_put(folder, &hdr, sizeof(HDR), pos, NULL);
	}
	else		/* 看板刪除 */
	{
	  rec_del(folder, sizeof(HDR), pos, NULL);
	  continue;	/* rec_del 以後不需要 pos++ */
	}
      }
    }
    else if (xmode == GEM_FOLDER)		/* 分類 recursive 進去砍 */
    {
      hdr_fpath(fpath, folder, &hdr);
      brd_classchange(fpath, oldname, newbrd);
    }
    pos++;
  }
}


void
brd_edit(bno)
  int bno;
{
  BRD *bhdr, newbh;
  char *bname, src[64], dst[64];;

  /* 看板設定 */
  vs_bar("\xAC\xDD\xAA\x4F\xB3\x5D\xA9\x77");
  bhdr = bshm->bcache + bno;
  memcpy(&newbh, bhdr, sizeof(BRD));
  /* 看板名稱：%s\n看板說明：[%s] %s\n板主名單：%s\n */
  prints("\xAC\xDD\xAA\x4F\xA6\x57\xBA\xD9\xA1\x47%s\n\xAC\xDD\xAA\x4F\xBB\xA1\xA9\xFA\xA1\x47[%s] %s\n\xAA\x4F\xA5\x44\xA6\x57\xB3\xE6\xA1\x47%s\n",
    newbh.brdname, newbh.class, newbh.title, newbh.BM);

  bitmsg(MSG_READPERM, STR_PERM, newbh.readlevel);
  bitmsg(MSG_POSTPERM, STR_PERM, newbh.postlevel);
  bitmsg(MSG_BRDATTR, STR_BATTR, newbh.battr);

  /* (D)刪除 (E)設定 (Q)取消？[Q]  */
  switch (vget(8, 0, "(D)\xA7\x52\xB0\xA3 (E)\xB3\x5D\xA9\x77 (Q)\xA8\xFA\xAE\xF8\xA1\x48[Q] ", src, 3, LCECHO))
  {
  case 'd':

    if (vget(9, 0, msg_sure_ny, src, 3, LCECHO) != 'y')
    {
      vmsg(MSG_DEL_CANCEL);
    }
    else
    {
      bname = bhdr->brdname;
      if (*bname)	/* itoc.000512: 同時砍除同一個看板會造成精華區、看板全毀 */
      {
	/* 刪除看板 */
	alog("\xA7\x52\xB0\xA3\xAC\xDD\xAA\x4F", bname);

	gem_fpath(src, bname, NULL);
	f_rm(src);
	f_rm(src + 4);
	brd_classchange("gem/@/@"CLASS_INIFILE, bname, NULL);	/* itoc.020117: 刪除 @Class 中的看板精華區捷徑 */
	memset(&newbh, 0, sizeof(BRD));
	sprintf(newbh.title, "[%s] deleted by %s", bname, cuser.userid);
	memcpy(bhdr, &newbh, sizeof(BRD));
	rec_put(FN_BRD, &newbh, sizeof(BRD), bno, NULL);

	/* itoc.050531: 砍板會造成看板不是按字母排序，所以要修正 numberOld */
	if (bshm->numberOld > bno)
	  bshm->numberOld = bno;

	/* 刪板完畢 */
	vmsg("\xA7\x52\xAA\x4F\xA7\xB9\xB2\xA6");
      }
    }
    break;

  case 'e':

    move(9, 0);
    /* 直接按 [Return] 不修改該項設定 */
    outs("\xAA\xBD\xB1\xB5\xAB\xF6 [Return] \xA4\xA3\xAD\xD7\xA7\xEF\xB8\xD3\xB6\xB5\xB3\x5D\xA9\x77");

    if (!brd_set(&newbh, 11))
    {
      if (memcmp(&newbh, bhdr, sizeof(BRD)) && vans(msg_sure_ny) == 'y')
      {
	bname = bhdr->brdname;
	if (strcmp(bname, newbh.brdname))	/* 看板更名要移目錄 */
	{
	  /* Thor.980806: 特別注意如果看板不在同一partition裡的話會有問題 */
	  gem_fpath(src, bname, NULL);
	  gem_fpath(dst, newbh.brdname, NULL);
	  rename(src, dst);
	  rename(src + 4, dst + 4);
	  brd_classchange("gem/@/@"CLASS_INIFILE, bname, &newbh);/* itoc.050329: 異動 @Class 中的看板精華區捷徑 */

	  /* itoc.050520: 改了板名會造成看板不是按字母排序，所以要修正 numberOld */
	  if (bshm->numberOld > bno)
	    bshm->numberOld = bno;
	}
	memcpy(bhdr, &newbh, sizeof(BRD));
	rec_put(FN_BRD, &newbh, sizeof(BRD), bno, NULL);
      }
    }
    /* 設定完畢 */
    vmsg("\xB3\x5D\xA9\x77\xA7\xB9\xB2\xA6");
    break;
  }
}


void
brd_title(bno)		/* itoc.000312: 板主修改中文敘述 */
  int bno;
{
  BRD *bhdr, newbh;
  char *blist;

  bhdr = bshm->bcache + bno;
  memcpy(&newbh, bhdr, sizeof(BRD));

  blist = bhdr->BM;

  if (blist[0] > ' ' && is_bm(blist, cuser.userid))
  {
    /* 是否修改中文板名敘述(Y/N)？[N]  */
    if (vans("\xAC\x4F\xA7\x5F\xAD\xD7\xA7\xEF\xA4\xA4\xA4\xE5\xAA\x4F\xA6\x57\xB1\xD4\xAD\x7A(Y/N)\xA1\x48[N] ") == 'y')
    {
      /* 看板主題： */
      vget(b_lines, 0, "\xAC\xDD\xAA\x4F\xA5\x44\xC3\x44\xA1\x47", newbh.title, BTLEN + 1, GCARRY);
      memcpy(bhdr, &newbh, sizeof(BRD));
      rec_put(FN_BRD, &newbh, sizeof(BRD), bno, NULL);
    }
  }
}
