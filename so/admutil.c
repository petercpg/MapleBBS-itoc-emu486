/*-------------------------------------------------------*/
/* admutil.c	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : 站長指令					 */
/* create : 95/03/29					 */
/* update : 01/03/01					 */
/*-------------------------------------------------------*/


#include "bbs.h"


extern BCACHE *bshm;
extern UCACHE *ushm;


/* ----------------------------------------------------- */
/* 站務指令						 */
/* ----------------------------------------------------- */


int
a_user()
{
  int ans;
  ACCT acct;

  move(1, 0);
  clrtobot();

  while (ans = acct_get(msg_uid, &acct))
  {
    if (ans > 0)
      acct_setup(&acct, 1);
  }
  return 0;
}


int
a_search()	/* itoc.010902: 暴力搜尋使用者 */
{
  ACCT acct;
  char c;
  char key[30];

  /* 請輸入關鍵字(姓名/暱稱/來源/信箱)： */
  if (!vget(b_lines, 0, "\xBD\xD0\xBF\xE9\xA4\x4A\xC3\xF6\xC1\xE4\xA6\x72(\xA9\x6D\xA6\x57/\xBC\xCA\xBA\xD9/\xA8\xD3\xB7\xBD/\xAB\x48\xBD\x63)\xA1\x47", key, 30, DOECHO))
    return XEASY;  

  /* itoc.010929.註解: 真是有夠暴力 :p 考慮先由 reaper 做出一個 .PASSWDS 再去找 */

  for (c = 'a'; c <= 'z'; c++)
  {
    char buf[64];
    struct dirent *de;
    DIR *dirp;

    sprintf(buf, "usr/%c", c);
    if (!(dirp = opendir(buf)))
      continue;

    while (de = readdir(dirp))
    {
      if (acct_load(&acct, de->d_name) < 0)
	continue;

      if (strstr(acct.realname, key) || strstr(acct.username, key) ||  
	strstr(acct.lasthost, key) || strstr(acct.email, key))
      {
	move(1, 0);
	acct_setup(&acct, 1);

	/* 是否繼續搜尋下一筆？[N]  */
	if (vans("\xAC\x4F\xA7\x5F\xC4\x7E\xC4\xF2\xB7\x6A\xB4\x4D\xA4\x55\xA4\x40\xB5\xA7\xA1\x48[N] ") != 'y')
	{
	  closedir(dirp);
 	  goto end_search;
 	}
      }
    }
    closedir(dirp);
  }
end_search:
  /* 搜尋完畢 */
  vmsg("\xB7\x6A\xB4\x4D\xA7\xB9\xB2\xA6");
  return 0;
}


int
a_editbrd()		/* itoc.010929: 修改看板選項 */
{
  int bno;
  BRD *brd;
  char bname[BNLEN + 1];

  if (brd = ask_board(bname, BRD_R_BIT, NULL))
  {
    bno = brd - bshm->bcache;
    brd_edit(bno);
  }
  else
  {
    vmsg(err_bid);
  }

  return 0;
}


int
a_xfile()		/* 設定系統檔案 */
{
  static char *desc[] =
  {
    /* 看板文章期限 */
    "\xAC\xDD\xAA\x4F\xA4\xE5\xB3\xB9\xB4\xC1\xAD\xAD",

    /* 身分認證信函 */
    "\xA8\xAD\xA4\xC0\xBB\x7B\xC3\xD2\xAB\x48\xA8\xE7",
    /* 認證通過通知 */
    "\xBB\x7B\xC3\xD2\xB3\x71\xB9\x4C\xB3\x71\xAA\xBE",
    /* 重新認證通知 */
    "\xAD\xAB\xB7\x73\xBB\x7B\xC3\xD2\xB3\x71\xAA\xBE",

#ifdef HAVE_DETECT_CROSSPOST
    /* 跨貼停權通知 */
    "\xB8\xF3\xB6\x4B\xB0\xB1\xC5\x76\xB3\x71\xAA\xBE",
#endif
    
    /* 不雅名單 */
    "\xA4\xA3\xB6\xAE\xA6\x57\xB3\xE6",
    /* 站務名單 */
    "\xAF\xB8\xB0\xC8\xA6\x57\xB3\xE6",

    /* 節日 */
    "\xB8\x60\xA4\xE9",

#ifdef HAVE_WHERE
    /* 故鄉 IP */
    "\xAC\x47\xB6\x6D IP",
    /* 故鄉 FQDN */
    "\xAC\x47\xB6\x6D FQDN",
#endif

#ifdef HAVE_TIP
    /* 每日小秘訣 */
    "\xA8\x43\xA4\xE9\xA4\x70\xAF\xB5\xB3\x5A",
#endif

#ifdef HAVE_LOVELETTER
    /* 情書產生器文庫 */
    "\xB1\xA1\xAE\xD1\xB2\xA3\xA5\xCD\xBE\xB9\xA4\xE5\xAE\x77",
#endif

    /* 認證白名單 */
    "\xBB\x7B\xC3\xD2\xA5\xD5\xA6\x57\xB3\xE6",
    /* 認證黑名單 */
    "\xBB\x7B\xC3\xD2\xB6\xC2\xA6\x57\xB3\xE6",

    /* 收信白名單 */
    "\xA6\xAC\xAB\x48\xA5\xD5\xA6\x57\xB3\xE6",
    /* 收信黑名單 */
    "\xA6\xAC\xAB\x48\xB6\xC2\xA6\x57\xB3\xE6",

#ifdef HAVE_LOGIN_DENIED
    /* 拒絕連線名單 */
    "\xA9\xDA\xB5\xB4\xB3\x73\xBD\x75\xA6\x57\xB3\xE6",
#endif

    NULL
  };

  static char *path[] =
  {
    FN_ETC_EXPIRE,

    FN_ETC_VALID,
    FN_ETC_JUSTIFIED,
    FN_ETC_REREG,

#ifdef HAVE_DETECT_CROSSPOST
    FN_ETC_CROSSPOST,
#endif
    
    FN_ETC_BADID,
    FN_ETC_SYSOP,

    FN_ETC_FEAST,

#ifdef HAVE_WHERE
    FN_ETC_HOST,
    FN_ETC_FQDN,
#endif

#ifdef HAVE_TIP
    FN_ETC_TIP,
#endif

#ifdef HAVE_LOVELETTER
    FN_ETC_LOVELETTER,
#endif

    TRUST_ACLFILE,
    UNTRUST_ACLFILE,

    MAIL_ACLFILE,
    UNMAIL_ACLFILE,

#ifdef HAVE_LOGIN_DENIED
    BBS_ACLFILE,
#endif
  };

  x_file(M_XFILES, desc, path);
  return 0;
}


int
a_resetsys()		/* 重置 */
{
  /* ◎ 系統重設 1)動態看板 2)分類群組 3)指名及擋信 4)全部：[Q]  */
  switch (vans("\xA1\xB7 \xA8\x74\xB2\xCE\xAD\xAB\xB3\x5D 1)\xB0\xCA\xBA\x41\xAC\xDD\xAA\x4F 2)\xA4\xC0\xC3\xFE\xB8\x73\xB2\xD5 3)\xAB\xFC\xA6\x57\xA4\xCE\xBE\xD7\xAB\x48 4)\xA5\xFE\xB3\xA1\xA1\x47[Q] "))
  {
  case '1':
    system("bin/camera");
    break;

  case '2':
    system("bin/account -nokeeplog");
    brh_save();
    board_main();
    break;

  case '3':
    system("kill -1 `cat run/bmta.pid`; kill -1 `cat run/bguard.pid`");
    break;

  case '4':
    system("kill -1 `cat run/bmta.pid`; kill -1 `cat run/bguard.pid`; bin/account -nokeeplog; bin/camera");
    brh_save();
    board_main();
    break;
  }

  return XEASY;
}


/* ----------------------------------------------------- */
/* 還原備份檔						 */
/* ----------------------------------------------------- */


static void
show_availability(type)		/* 將 BAKPATH 裡面所有可取回備份的目錄印出來 */
  char *type;
{
  int tlen, len, col;
  char *fname, fpath[64];
  struct dirent *de;
  DIR *dirp;
  FILE *fp;

  if (dirp = opendir(BAKPATH))
  {
    col = 0;
    tlen = strlen(type);

    sprintf(fpath, "tmp/restore.%s", cuser.userid);
    fp = fopen(fpath, "w");
    /* ※ 可供取回的備份有：\n\n */
    fputs("\xA1\xB0 \xA5\x69\xA8\xD1\xA8\xFA\xA6\x5E\xAA\xBA\xB3\xC6\xA5\xF7\xA6\xB3\xA1\x47\n\n", fp);

    while (de = readdir(dirp))
    {
      fname = de->d_name;
      if (!strncmp(fname, type, tlen))
      {
	len = strlen(fname) + 2;
	if (b_cols - col < len)
	{
	  fputc('\n', fp);
	  col = len;
	}
	else
	{
	  col += len;
	}
	fprintf(fp, "%s  ", fname);
      }
    }

    fputc('\n', fp);
    fclose(fp);
    closedir(dirp);

    more(fpath, (char *) -1);
    unlink(fpath);
  }
}


int
a_restore()
{
  int ch;
  char *type, *ptr;
  char *tpool[3] = {"brd", "gem", "usr"};
  char date[20], brdname[BNLEN + 1], src[64], cmd[256];
  ACCT acct;
  BPAL *bpal;

  /* ◎ 還原備份 1)看板 2)精華區 3)使用者：[Q]  */
  ch = vans("\xA1\xB7 \xC1\xD9\xAD\xEC\xB3\xC6\xA5\xF7 1)\xAC\xDD\xAA\x4F 2)\xBA\xEB\xB5\xD8\xB0\xCF 3)\xA8\xCF\xA5\xCE\xAA\xCC\xA1\x47[Q] ") - '1';
  if (ch < 0 || ch >= 3)
    return XEASY;

  type = tpool[ch];
  show_availability(type);

  /* 要取回的備份目錄： */
  if (vget(b_lines, 0, "\xAD\x6E\xA8\xFA\xA6\x5E\xAA\xBA\xB3\xC6\xA5\xF7\xA5\xD8\xBF\xFD\xA1\x47", date, 20, DOECHO))
  {
    /* 避免站長打了一個存在的目錄，但是和 type 不合 */
    if (strncmp(date, type, strlen(type)))
      return 0;

    sprintf(src, BAKPATH"/%s", date);
    if (!dashd(src))
      return 0;
    ptr = strchr(src, '\0');

    clear();
    move(3, 0);
    /* 欲還原備份的看板/使用者必須已存在。\n */
    outs("\xB1\xFD\xC1\xD9\xAD\xEC\xB3\xC6\xA5\xF7\xAA\xBA\xAC\xDD\xAA\x4F/\xA8\xCF\xA5\xCE\xAA\xCC\xA5\xB2\xB6\xB7\xA4\x77\xA6\x73\xA6\x62\xA1\x43\n"
      /* 若該看板/使用者已刪除，請先重新開設/註冊一個同名的看板/使用者。\n */
      "\xAD\x59\xB8\xD3\xAC\xDD\xAA\x4F/\xA8\xCF\xA5\xCE\xAA\xCC\xA4\x77\xA7\x52\xB0\xA3\xA1\x41\xBD\xD0\xA5\xFD\xAD\xAB\xB7\x73\xB6\x7D\xB3\x5D/\xB5\xF9\xA5\x55\xA4\x40\xAD\xD3\xA6\x50\xA6\x57\xAA\xBA\xAC\xDD\xAA\x4F/\xA8\xCF\xA5\xCE\xAA\xCC\xA1\x43\n"
      /* 還原備份時請確認該看板無人使用/使用者不在線上 */
      "\xC1\xD9\xAD\xEC\xB3\xC6\xA5\xF7\xAE\xC9\xBD\xD0\xBD\x54\xBB\x7B\xB8\xD3\xAC\xDD\xAA\x4F\xB5\x4C\xA4\x48\xA8\xCF\xA5\xCE/\xA8\xCF\xA5\xCE\xAA\xCC\xA4\xA3\xA6\x62\xBD\x75\xA4\x57");

    if (ch == 0 || ch == 1)
    {
      if (!ask_board(brdname, BRD_L_BIT, NULL))
	return 0;
      sprintf(ptr, "/%s%s.tgz", ch == 0 ? "" : "brd/", brdname);
    }
    else /* if (ch == 2) */
    {
      if (acct_get(msg_uid, &acct) <= 0)
	return 0;
      type = acct.userid;
      str_lower(type, type);
      sprintf(ptr, "/%c/%s.tgz", *type, type);
    }

    if (!dashf(src))
    {
      /* 檔案不存在，通常是因為備份點時該看板/使用者已被刪除，或是當時根本就還沒有該看板/使用者 */
      /* 備份檔案不存在，請試試其他時間點的備份 */
      vmsg("\xB3\xC6\xA5\xF7\xC0\xC9\xAE\xD7\xA4\xA3\xA6\x73\xA6\x62\xA1\x41\xBD\xD0\xB8\xD5\xB8\xD5\xA8\xE4\xA5\x4C\xAE\xC9\xB6\xA1\xC2\x49\xAA\xBA\xB3\xC6\xA5\xF7");
      return 0;
    }

    /* 還原備份後，目前所有資料都會流失，請務必確定(Y/N)？[N]  */
    if (vans("\xC1\xD9\xAD\xEC\xB3\xC6\xA5\xF7\xAB\xE1\xA1\x41\xA5\xD8\xAB\x65\xA9\xD2\xA6\xB3\xB8\xEA\xAE\xC6\xB3\xA3\xB7\x7C\xAC\x79\xA5\xA2\xA1\x41\xBD\xD0\xB0\xC8\xA5\xB2\xBD\x54\xA9\x77(Y/N)\xA1\x48[N] ") != 'y')
      return 0;

    /* 還原備份 */
    alog("\xC1\xD9\xAD\xEC\xB3\xC6\xA5\xF7", src);

    /* 解壓縮 */
    if (ch == 0)
      ptr = "brd";
    else if (ch == 1)
      ptr = "gem/brd";
    else /* if (ch == 2) */
      sprintf(ptr = date, "usr/%c", *type);
    sprintf(cmd, "tar xfz %s -C %s/", src, ptr);
    /* system(cmd); */

#if 1	/* 讓站長手動執行 */
    move(7, 0);
    /* \n請以 bbs 身分登入工作站，並於\033[1;36m家目錄\033[m執行\n\n\033[1;33m */
    outs("\n\xBD\xD0\xA5\x48 bbs \xA8\xAD\xA4\xC0\xB5\x6E\xA4\x4A\xA4\x75\xA7\x40\xAF\xB8\xA1\x41\xA8\xC3\xA9\xF3\033[1;36m\xAE\x61\xA5\xD8\xBF\xFD\033[m\xB0\xF5\xA6\xE6\n\n\033[1;33m");
    outs(cmd);
    outs("\033[m\n\n");
#endif

    /* tar 完以後，還要做的事 */
    /* ◎ 指令 Y)已成功執行以上指令 Q)放棄執行：[Q]  */
    if (vans("\xA1\xB7 \xAB\xFC\xA5\x4F Y)\xA4\x77\xA6\xA8\xA5\x5C\xB0\xF5\xA6\xE6\xA5\x48\xA4\x57\xAB\xFC\xA5\x4F Q)\xA9\xF1\xB1\xF3\xB0\xF5\xA6\xE6\xA1\x47[Q] ") == 'y')
    {
      if (ch == 0)	/* 還原看板時，要更新板友 */
      {
	if ((ch = brd_bno(brdname)) >= 0)
	{
	  brd_fpath(src, brdname, fn_pal);
	  bpal = bshm->pcache + ch;
	  bpal->pal_max = image_pal(src, bpal->pal_spool);
	}
      }
      else if (ch == 2)	/* 還原使用者時，不還原 userno */
      {
	ch = acct.userno;
	if (acct_load(&acct, type) >= 0)
	{
	  acct.userno = ch;
	  acct_save(&acct);
	}
      }
      /* 還原備份成功 */
      vmsg("\xC1\xD9\xAD\xEC\xB3\xC6\xA5\xF7\xA6\xA8\xA5\x5C");
      return 0;
    }
  }

  vmsg(msg_cancel);
  return 0;
}


#ifdef HAVE_REGISTER_FORM

/* ----------------------------------------------------- */
/* 處理 Register Form					 */
/* ----------------------------------------------------- */


static int
scan_register_form(fd)
  int fd;
{
  static char logfile[] = FN_RUN_RFORM_LOG;
  static char *reason[] = 
  {
    /* 輸入真實姓名 */
    /* 詳實填寫申請表 */
    /* 詳填住址資料 */
    /* 詳填連絡電話 */
    "\xBF\xE9\xA4\x4A\xAF\x75\xB9\xEA\xA9\x6D\xA6\x57", "\xB8\xD4\xB9\xEA\xB6\xF1\xBC\x67\xA5\xD3\xBD\xD0\xAA\xED", "\xB8\xD4\xB6\xF1\xA6\xED\xA7\x7D\xB8\xEA\xAE\xC6", "\xB8\xD4\xB6\xF1\xB3\x73\xB5\xB8\xB9\x71\xB8\xDC", 
    /* 詳填服務單位、或學校系級 */
    /* 用中文填寫申請單 */
    "\xB8\xD4\xB6\xF1\xAA\x41\xB0\xC8\xB3\xE6\xA6\xEC\xA1\x42\xA9\xCE\xBE\xC7\xAE\xD5\xA8\x74\xAF\xC5", "\xA5\xCE\xA4\xA4\xA4\xE5\xB6\xF1\xBC\x67\xA5\xD3\xBD\xD0\xB3\xE6", 
#ifdef EMAIL_JUSTIFY	/* waynesan.040327: 有 E-mail 認證才有此項 */
    /* 採用 E-mail 認證 */
    "\xB1\xC4\xA5\xCE E-mail \xBB\x7B\xC3\xD2", 
#endif
    NULL
  };

  ACCT acct;
  RFORM rform;
  HDR hdr;
  FILE *fout;

  int op, n;
  char buf[256], *agent, *userid, *str;
  char folder[64], fpath[64];

  /* 審核使用者註冊資料 */
  vs_bar("\xBC\x66\xAE\xD6\xA8\xCF\xA5\xCE\xAA\xCC\xB5\xF9\xA5\x55\xB8\xEA\xAE\xC6");
  agent = cuser.userid;

  while (read(fd, &rform, sizeof(RFORM)) == sizeof(RFORM))
  {
    userid = rform.userid;
    move(2, 0);
    /* 申請代號: %s (申請時間：%s)\n */
    prints("\xA5\xD3\xBD\xD0\xA5\x4E\xB8\xB9: %s (\xA5\xD3\xBD\xD0\xAE\xC9\xB6\xA1\xA1\x47%s)\n", userid, Btime(rform.rtime));
    /* 服務單位: %s\n */
    prints("\xAA\x41\xB0\xC8\xB3\xE6\xA6\xEC: %s\n", rform.career);
    /* 目前住址: %s\n */
    prints("\xA5\xD8\xAB\x65\xA6\xED\xA7\x7D: %s\n", rform.address);
    /* 連絡電話: %s\n%s\n */
    prints("\xB3\x73\xB5\xB8\xB9\x71\xB8\xDC: %s\n%s\n", rform.phone, msg_seperator);
    clrtobot();

    if ((acct_load(&acct, userid) < 0) || (acct.userno != rform.userno))
    {
      /* 查無此人 */
      vmsg("\xAC\x64\xB5\x4C\xA6\xB9\xA4\x48");
      op = 'd';
    }
    else
    {
      acct_show(&acct, 2);

#ifdef JUSTIFY_PERIODICAL
      if (acct.userlevel & PERM_VALID && acct.tvalid + VALID_PERIOD - INVALID_NOTICE_PERIOD >= acct.lastlogin)
#else
      if (acct.userlevel & PERM_VALID)
#endif
      {
	/* 此帳號已經完成註冊 */
	vmsg("\xA6\xB9\xB1\x62\xB8\xB9\xA4\x77\xB8\x67\xA7\xB9\xA6\xA8\xB5\xF9\xA5\x55");
	op = 'd';
      }
      else if (acct.userlevel & PERM_ALLDENY)
      {
	/* itoc.050405: 不能讓停權者重新認證，因為會改掉他的 tvalid (停權到期時間) */
	/* 此帳號目前被停權中 */
	vmsg("\xA6\xB9\xB1\x62\xB8\xB9\xA5\xD8\xAB\x65\xB3\x51\xB0\xB1\xC5\x76\xA4\xA4");
	op = 'd';
      }
      else
      {
	/* 是否接受(Y/N/Q/Del/Skip)？[S]  */
	op = vans("\xAC\x4F\xA7\x5F\xB1\xB5\xA8\xFC(Y/N/Q/Del/Skip)\xA1\x48[S] ");
      }
    }

    switch (op)
    {
    case 'y':

      /* 提升權限 */
      sprintf(buf, "REG: %s:%s:%s:by %s", rform.phone, rform.career, rform.address, agent);
      justify_log(acct.userid, buf);
      time(&acct.tvalid);
      /* itoc.041025: 這個 acct_setperm() 並沒有緊跟在 acct_load() 後面，中間隔了一個 vans()，
         這可能造成拿舊 acct 去覆蓋新 .ACCT 的問題。不過因為是站長才有的權限，所以就不改了 */
      acct_setperm(&acct, PERM_VALID, 0);

      /* 寄信通知使用者 */
      usr_fpath(folder, userid, fn_dir);
      hdr_stamp(folder, HDR_LINK, &hdr, FN_ETC_JUSTIFIED);
      strcpy(hdr.title, msg_reg_valid);
      strcpy(hdr.owner, str_sysop);
      rec_add(folder, &hdr, sizeof(HDR));

      strcpy(rform.agent, agent);
      rec_add(logfile, &rform, sizeof(RFORM));

      m_biff(rform.userno);

      break;

    case 'q':			/* 太累了，結束休息 */

      do
      {
	rec_add(FN_RUN_RFORM, &rform, sizeof(RFORM));
      } while (read(fd, &rform, sizeof(RFORM)) == sizeof(RFORM));

    case 'd':
      break;

    case 'n':

      move(9, 0);
      /* 請提出退回申請表原因，按 <enter> 取消\n\n */
      prints("\xBD\xD0\xB4\xA3\xA5\x58\xB0\x68\xA6\x5E\xA5\xD3\xBD\xD0\xAA\xED\xAD\xEC\xA6\x5D\xA1\x41\xAB\xF6 <enter> \xA8\xFA\xAE\xF8\n\n");
      for (n = 0; str = reason[n]; n++)
	/* %d) 請%s\n */
	prints("%d) \xBD\xD0%s\n", n, str);
      clrtobot();

      /* 退回原因： */
      if (op = vget(b_lines, 0, "\xB0\x68\xA6\x5E\xAD\xEC\xA6\x5D\xA1\x47", buf, 60, DOECHO))
      {
	int i;

	i = op - '0';
	if (i >= 0 && i < n)
	  strcpy(buf, reason[i]);

	usr_fpath(folder, acct.userid, fn_dir);
	if (fout = fdopen(hdr_stamp(folder, 0, &hdr, fpath), "w"))
	{
	  /* \t由於您提供的資料不夠詳實，無法確認身分， */
	  fprintf(fout, "\t\xA5\xD1\xA9\xF3\xB1\x7A\xB4\xA3\xA8\xD1\xAA\xBA\xB8\xEA\xAE\xC6\xA4\xA3\xB0\xF7\xB8\xD4\xB9\xEA\xA1\x41\xB5\x4C\xAA\x6B\xBD\x54\xBB\x7B\xA8\xAD\xA4\xC0\xA1\x41"
	    /* \n\n\t請重新填寫註冊表單：%s。\n */
	    "\n\n\t\xBD\xD0\xAD\xAB\xB7\x73\xB6\xF1\xBC\x67\xB5\xF9\xA5\x55\xAA\xED\xB3\xE6\xA1\x47%s\xA1\x43\n", buf);
	  fclose(fout);

	  strcpy(hdr.owner, agent);
	  /* [退件] 請您重新填寫註冊表單 */
	  strcpy(hdr.title, "[\xB0\x68\xA5\xF3] \xBD\xD0\xB1\x7A\xAD\xAB\xB7\x73\xB6\xF1\xBC\x67\xB5\xF9\xA5\x55\xAA\xED\xB3\xE6");
	  rec_add(folder, &hdr, sizeof(HDR));
	}

	strcpy(rform.reply, buf);	/* 理由 */
	strcpy(rform.agent, agent);
	rec_add(logfile, &rform, sizeof(RFORM));

	break;
      }

    default:			/* put back to regfile */

      rec_add(FN_RUN_RFORM, &rform, sizeof(RFORM));
    }
  }
}


int
a_register()
{
  int num;
  char buf[80];

  num = rec_num(FN_RUN_RFORM, sizeof(RFORM));
  if (num <= 0)
  {
    /* 目前並無新註冊資料 */
    zmsg("\xA5\xD8\xAB\x65\xA8\xC3\xB5\x4C\xB7\x73\xB5\xF9\xA5\x55\xB8\xEA\xAE\xC6");
    return XEASY;
  }

  /* 共有 %d 筆資料，開始審核嗎(Y/N)？[N]  */
  sprintf(buf, "\xA6\x40\xA6\xB3 %d \xB5\xA7\xB8\xEA\xAE\xC6\xA1\x41\xB6\x7D\xA9\x6C\xBC\x66\xAE\xD6\xB6\xDC(Y/N)\xA1\x48[N] ", num);
  num = XEASY;

  if (vans(buf) == 'y')
  {
    sprintf(buf, "%s.tmp", FN_RUN_RFORM);
    if (dashf(buf))
    {
      /* 其他 SYSOP 也在審核註冊申請單 */
      vmsg("\xA8\xE4\xA5\x4C SYSOP \xA4\x5D\xA6\x62\xBC\x66\xAE\xD6\xB5\xF9\xA5\x55\xA5\xD3\xBD\xD0\xB3\xE6");
    }
    else
    {
      int fd;

      rename(FN_RUN_RFORM, buf);
      fd = open(buf, O_RDONLY);
      if (fd >= 0)
      {
	scan_register_form(fd);
	close(fd);
	unlink(buf);
	num = 0;
      }
      else
      {
	/* 無法開啟註冊資料工作檔 */
	vmsg("\xB5\x4C\xAA\x6B\xB6\x7D\xB1\xD2\xB5\xF9\xA5\x55\xB8\xEA\xAE\xC6\xA4\x75\xA7\x40\xC0\xC9");
      }
    }
  }
  return num;
}


int
a_regmerge()			/* itoc.000516: 斷線時註冊單修復 */
{
  char fpath[64];
  FILE *fp;

  sprintf(fpath, "%s.tmp", FN_RUN_RFORM);
  if (dashf(fpath))
  {
    /* 請先確定已無其他站長在審核註冊單，以免發生嚴重意外！ */
    vmsg("\xBD\xD0\xA5\xFD\xBD\x54\xA9\x77\xA4\x77\xB5\x4C\xA8\xE4\xA5\x4C\xAF\xB8\xAA\xF8\xA6\x62\xBC\x66\xAE\xD6\xB5\xF9\xA5\x55\xB3\xE6\xA1\x41\xA5\x48\xA7\x4B\xB5\x6F\xA5\xCD\xC4\x59\xAD\xAB\xB7\x4E\xA5\x7E\xA1\x49");

    /* 確定要啟動註冊單修復功能(Y/N)？[N]  */
    if (vans("\xBD\x54\xA9\x77\xAD\x6E\xB1\xD2\xB0\xCA\xB5\xF9\xA5\x55\xB3\xE6\xAD\xD7\xB4\x5F\xA5\x5C\xAF\xE0(Y/N)\xA1\x48[N] ") == 'y')
    {
      if (fp = fopen(FN_RUN_RFORM, "a"))
      {
	f_suck(fp, fpath);
	fclose(fp);
	unlink(fpath);
      }
      /* 處理完畢，以後請小心！ */
      vmsg("\xB3\x42\xB2\x7A\xA7\xB9\xB2\xA6\xA1\x41\xA5\x48\xAB\xE1\xBD\xD0\xA4\x70\xA4\xDF\xA1\x49");
    }
  }
  else
  {
    /* 目前並無修復註冊單之必要 */
    zmsg("\xA5\xD8\xAB\x65\xA8\xC3\xB5\x4C\xAD\xD7\xB4\x5F\xB5\xF9\xA5\x55\xB3\xE6\xA4\xA7\xA5\xB2\xAD\x6E");
  }
  return XEASY;
}
#endif	/* HAVE_REGISTER_FORM */


/* ----------------------------------------------------- */
/* 寄信給全站使用者/板主				 */
/* ----------------------------------------------------- */


static void
add_to_list(list, id)
  char *list;
  char *id;		/* 未必 end with '\0' */
{
  char *i;

  /* 先檢查先前的 list 裡面是否已經有了，以免重覆加入 */
  for (i = list; *i; i += IDLEN + 1)
  {
    if (!strncmp(i, id, IDLEN))
      return;
  }

  /* 若之前的 list 沒有，那麼直接附加在 list 最後 */
  str_ncpy(i, id, IDLEN + 1);
}


static void
make_bm_list(list)
  char *list;
{
  BRD *head, *tail;
  char *ptr, *str, buf[BMLEN + 1];

  /* 去 bshm 中抓出所有 brd->BM */

  head = bshm->bcache;
  tail = head + bshm->number;
  do				/* 至少有 note 一板，不必對看板做檢查 */
  {
    ptr = buf;
    strcpy(ptr, head->BM);

    while (*ptr)	/* 把 brd->BM 中 bm1/bm2/bm3/... 各個 bm 抓出來 */
    {
      if (str = strchr(ptr, '/'))
	*str = '\0';
      add_to_list(list, ptr);
      if (!str)
	break;
      ptr = str + 1;
    }      
  } while (++head < tail);
}


static void
make_all_list(list)
  char *list;
{
  int fd;
  SCHEMA schema;

  if ((fd = open(FN_SCHEMA, O_RDONLY)) < 0)
    return;

  while (read(fd, &schema, sizeof(SCHEMA)) == sizeof(SCHEMA))
    add_to_list(list, schema.userid);

  close(fd);
}


static void
send_list(title, fpath, list)
  char *title;		/* 信件的標題 */
  char *fpath;		/* 信件的檔案 */
  char *list;		/* 寄信的名單 */
{
  char folder[64], *ptr;
  HDR hdr;

  for (ptr = list; *ptr; ptr += IDLEN + 1)
  {
    usr_fpath(folder, ptr, fn_dir);
    if (hdr_stamp(folder, HDR_LINK, &hdr, fpath) >= 0)
    {
      strcpy(hdr.owner, str_sysop);
      strcpy(hdr.title, title);
      hdr.xmode = 0;
      rec_add(folder, &hdr, sizeof(HDR));
    }
  }
}


static void
biff_bm()
{
  UTMP *utmp, *uceil;

  utmp = ushm->uslot;
  uceil = (void *) utmp + ushm->offset;
  do
  {
    if (utmp->pid && (utmp->userlevel & PERM_BM))
      utmp->status |= STATUS_BIFF;
  } while (++utmp <= uceil);
}


static void
biff_all()
{
  UTMP *utmp, *uceil;

  utmp = ushm->uslot;
  uceil = (void *) utmp + ushm->offset;
  do
  {
    if (utmp->pid)
      utmp->status |= STATUS_BIFF;
  } while (++utmp <= uceil);
}


int
m_bm()
{
  char *list, fpath[64];
  FILE *fp;
  int size;

  /* 要寄信給全站所有板主(Y/N)？[N]  */
  if (vans("\xAD\x6E\xB1\x48\xAB\x48\xB5\xB9\xA5\xFE\xAF\xB8\xA9\xD2\xA6\xB3\xAA\x4F\xA5\x44(Y/N)\xA1\x48[N] ") != 'y')
    return XEASY;

  /* [板主通告]  */
  strcpy(ve_title, "[\xAA\x4F\xA5\x44\xB3\x71\xA7\x69] ");
  /* 標題： */
  if (!vget(1, 0, "\xBC\xD0\xC3\x44\xA1\x47", ve_title, TTLEN + 1, GCARRY))
    return 0;

  usr_fpath(fpath, cuser.userid, "sysmail");
  if (fp = fopen(fpath, "w"))
  {
    /* ※ [板主通告] 站長通告，收信人：各板主\n */
    fprintf(fp, "\xA1\xB0 [\xAA\x4F\xA5\x44\xB3\x71\xA7\x69] \xAF\xB8\xAA\xF8\xB3\x71\xA7\x69\xA1\x41\xA6\xAC\xAB\x48\xA4\x48\xA1\x47\xA6\x55\xAA\x4F\xA5\x44\n");
    fprintf(fp, "-------------------------------------------------------------------------\n");
    fclose(fp);
  }

  curredit = EDIT_MAIL;
  *quote_file = '\0';
  if (vedit(fpath, 1) >= 0)
  {
    /* 需要一段蠻長的時間，請耐心等待 */
    vmsg("\xBB\xDD\xAD\x6E\xA4\x40\xAC\x71\xC6\x5A\xAA\xF8\xAA\xBA\xAE\xC9\xB6\xA1\xA1\x41\xBD\xD0\xAD\x40\xA4\xDF\xB5\xA5\xAB\xDD");

    size = (IDLEN + 1) * MAXBOARD * 4;	/* 假設每板四個板主已足夠 */
    if (list = (char *) malloc(size))
    {
      memset(list, 0, size);

      make_bm_list(list);
      send_list(ve_title, fpath, list);

      free(list);
      biff_bm();
    }
  }
  else
  {
    vmsg(msg_cancel);
  }

  unlink(fpath);

  return 0;
}


int
m_all()
{
  char *list, fpath[64];
  FILE *fp;
  int size;

  /* 要寄信給全站使用者(Y/N)？[N]  */
  if (vans("\xAD\x6E\xB1\x48\xAB\x48\xB5\xB9\xA5\xFE\xAF\xB8\xA8\xCF\xA5\xCE\xAA\xCC(Y/N)\xA1\x48[N] ") != 'y')
    return XEASY;    

  /* [系統通告]  */
  strcpy(ve_title, "[\xA8\x74\xB2\xCE\xB3\x71\xA7\x69] ");
  /* 標題： */
  if (!vget(1, 0, "\xBC\xD0\xC3\x44\xA1\x47", ve_title, TTLEN + 1, GCARRY))
    return 0;

  usr_fpath(fpath, cuser.userid, "sysmail");
  if (fp = fopen(fpath, "w"))
  {
    /* ※ [系統通告] 站長通告，收信人：全站使用者\n */
    fprintf(fp, "\xA1\xB0 [\xA8\x74\xB2\xCE\xB3\x71\xA7\x69] \xAF\xB8\xAA\xF8\xB3\x71\xA7\x69\xA1\x41\xA6\xAC\xAB\x48\xA4\x48\xA1\x47\xA5\xFE\xAF\xB8\xA8\xCF\xA5\xCE\xAA\xCC\n");
    fprintf(fp, "-------------------------------------------------------------------------\n");
    fclose(fp);
  }

  curredit = EDIT_MAIL;
  *quote_file = '\0';
  if (vedit(fpath, 1) >= 0)
  {
    /* 需要一段蠻長的時間，請耐心等待 */
    vmsg("\xBB\xDD\xAD\x6E\xA4\x40\xAC\x71\xC6\x5A\xAA\xF8\xAA\xBA\xAE\xC9\xB6\xA1\xA1\x41\xBD\xD0\xAD\x40\xA4\xDF\xB5\xA5\xAB\xDD");

    size = (IDLEN + 1) * rec_num(FN_SCHEMA, sizeof(SCHEMA));
    if (list = (char *) malloc(size))
    {
      memset(list, 0, size);

      make_all_list(list);
      send_list(ve_title, fpath, list);

      free(list);
      biff_all();
    }
  }
  else
  {
    vmsg(msg_cancel);
  }

  unlink(fpath);

  return 0;
}
