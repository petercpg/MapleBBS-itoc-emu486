/*-------------------------------------------------------*/
/* menu.c	( NTHU CS MapleBBS Ver 3.00 )		 */
/*-------------------------------------------------------*/
/* target : menu/help/movie routines		 	 */
/* create : 95/03/29				 	 */
/* update : 97/03/29				 	 */
/*-------------------------------------------------------*/


#include "bbs.h"


extern UCACHE *ushm;
extern FCACHE *fshm;


#ifndef ENHANCED_VISIT
extern time_t brd_visit[];
#endif


/* ----------------------------------------------------- */
/* 離開 BBS 站						 */
/* ----------------------------------------------------- */


#define	FN_RUN_NOTE_PAD	"run/note.pad"
#define	FN_RUN_NOTE_TMP	"run/note.tmp"


typedef struct
{
  time_t tpad;
  char msg[400];
}      Pad;


int
pad_view()
{
  int fd, count;
  Pad *pad;

  if ((fd = open(FN_RUN_NOTE_PAD, O_RDONLY)) < 0)
    return XEASY;

  count = 0;
  mgets(-1);

  for (;;)
  {
    pad = mread(fd, sizeof(Pad));
    if (!pad)
    {
      vmsg(NULL);
      break;
    }
    else if (!(count % 5))	/* itoc.020122: 有 pad 才印 */
    {
      clear();
      move(0, 23);
      /* 【 酸 甜 苦 辣 留 言 板 】                第 %d 頁\n\n */
      prints("\xA1\x69 \xBB\xC4 \xB2\xA2 \xAD\x57 \xBB\xB6 \xAF\x64 \xA8\xA5 \xAA\x4F \xA1\x6A                \xB2\xC4 %d \xAD\xB6\n\n", count / 5 + 1);
    }

    outs(pad->msg);
    count++;

    if (!(count % 5))
    {
      move(b_lines, 0);
      /* 請按 [SPACE] 繼續觀賞，或按其他鍵結束：  */
      outs("\xBD\xD0\xAB\xF6 [SPACE] \xC4\x7E\xC4\xF2\xC6\x5B\xBD\xE0\xA1\x41\xA9\xCE\xAB\xF6\xA8\xE4\xA5\x4C\xC1\xE4\xB5\xB2\xA7\xF4\xA1\x47 ");
      /* itoc.010127: 修正在偵測左右鍵全形下，按左鍵會跳離二層選單的問題 */

      if (vkey() != ' ')
	break;
    }
  }

  close(fd);
  return 0;
}


static int
pad_draw()
{
  int i, cc, fdr, color;
  FILE *fpw;
  Pad pad;
  char *str, buf[3][71];

  /* itoc.註解: 不想用高彩度，太花 */
  static char pcolors[6] = {31, 32, 33, 34, 35, 36};

  /* itoc.010309: 留言板提供不同的顏色 */
  /* 心情顏色 1) \033[41m  \033[m 2) \033[42m  \033[m 3) \033[43m  \033[m  */
  color = vans("\xA4\xDF\xB1\xA1\xC3\x43\xA6\xE2 1) \033[41m  \033[m 2) \033[42m  \033[m 3) \033[43m  \033[m "
    "4) \033[44m  \033[m 5) \033[45m  \033[m 6) \033[46m  \033[m [Q] ");

  if (color < '1' || color > '6')
    return XEASY;
  else
    color -= '1';

  do
  {
    buf[0][0] = buf[1][0] = buf[2][0] = '\0';
    move(MENU_XPOS, 0);
    clrtobot();
    /* \n請留言 (至多三行)，按[Enter]結束 */
    outs("\n\xBD\xD0\xAF\x64\xA8\xA5 (\xA6\xDC\xA6\x68\xA4\x54\xA6\xE6)\xA1\x41\xAB\xF6[Enter]\xB5\xB2\xA7\xF4");
    for (i = 0; (i < 3) &&
      /* ： */
      vget(16 + i, 0, "\xA1\x47", buf[i], 71, DOECHO); i++);
    /* (S)存檔觀賞 (E)重新來過 (Q)算了？[S]  */
    cc = vans("(S)\xA6\x73\xC0\xC9\xC6\x5B\xBD\xE0 (E)\xAD\xAB\xB7\x73\xA8\xD3\xB9\x4C (Q)\xBA\xE2\xA4\x46\xA1\x48[S] ");
    if (cc == 'q' || i == 0)
      return 0;
  } while (cc == 'e');

  time(&pad.tpad);

  /* itoc.020812.註解: 改版面的時候要注意 struct Pad.msg[] 是否夠大 */
  str = pad.msg;
  /* ╭┤\033[1;46m %s － %s \033[m├ */
  sprintf(str, "\xA2\x7E\xA2\x74\033[1;46m %s \xA1\xD0 %s \033[m\xA2\x75", cuser.userid, cuser.username);

  for (cc = strlen(str); cc < 60; cc += 2)
    /* ─ */
    strcpy(str + cc, "\xA2\x77");
  if (cc == 60)
    str[cc++] = ' ';

  sprintf(str + cc,
    /* \033[1;44m %s \033[m╮\n */
    "\033[1;44m %s \033[m\xA2\xA1\n"
    /* │  \033[1;%dm%-70s\033[m  │\n */
    "\xA2\x78  \033[1;%dm%-70s\033[m  \xA2\x78\n"
    /* │  \033[1;%dm%-70s\033[m  │\n */
    "\xA2\x78  \033[1;%dm%-70s\033[m  \xA2\x78\n"
    /* ╰  \033[1;%dm%-70s\033[m  ╯\n */
    "\xA2\xA2  \033[1;%dm%-70s\033[m  \xA2\xA3\n",
    Btime(pad.tpad),
    pcolors[color], buf[0],
    pcolors[color], buf[1],
    pcolors[color], buf[2]);

  f_cat(FN_RUN_NOTE_ALL, str);

  if (!(fpw = fopen(FN_RUN_NOTE_TMP, "w")))
    return 0;

  fwrite(&pad, sizeof(pad), 1, fpw);

  if ((fdr = open(FN_RUN_NOTE_PAD, O_RDONLY)) >= 0)
  {
    Pad *pp;

    i = 0;
    cc = pad.tpad - NOTE_DUE * 60 * 60;
    mgets(-1);
    while (pp = mread(fdr, sizeof(Pad)))
    {
      fwrite(pp, sizeof(Pad), 1, fpw);
      if ((++i > NOTE_MAX) || (pp->tpad < cc))
	break;
    }
    close(fdr);
  }

  fclose(fpw);

  rename(FN_RUN_NOTE_TMP, FN_RUN_NOTE_PAD);
  pad_view();
  return 0;
}


static int
goodbye()
{
  /* itoc.010803: 秀張離站的圖 */
  clear();
  film_out(FILM_GOODBYE, 0);

  /* G)隨風而逝 M)報告站長 N)留言板 Q)取消？[Q]  */
  switch (vans("G)\xC0\x48\xAD\xB7\xA6\xD3\xB3\x75 M)\xB3\xF8\xA7\x69\xAF\xB8\xAA\xF8 N)\xAF\x64\xA8\xA5\xAA\x4F Q)\xA8\xFA\xAE\xF8\xA1\x48[Q] "))
  {
  /* lkchu.990428: 內定改為不離站 */
  case 'g':
  case 'y':
    break;

  case 'm':
    m_sysop();
    break;

  case 'n':
    /* if (cuser.userlevel) */
    if (HAS_PERM(PERM_POST)) /* Thor.990118: 要能post才能留言, 提高門檻 */
      pad_draw();
    break;

  case 'q':
  default:
    /* return XEASY; */
    return 0;	/* itoc.010803: 秀了 FILM_GOODBYE 要重繪 */
  }

#ifdef LOG_BMW
  bmw_log();			/* lkchu.981201: 水球記錄處理 */
#endif

  if (!(cuser.ufo & UFO_MOTD))	/* itoc.000407: 離站畫面一併簡化 */
  {
    clear();
    /* 親愛的 \033[32m%s(%s)\033[m，別忘了再度光臨【 %s 】\n */
    prints("\xBF\xCB\xB7\x52\xAA\xBA \033[32m%s(%s)\033[m\xA1\x41\xA7\x4F\xA7\xD1\xA4\x46\xA6\x41\xAB\xD7\xA5\xFA\xC1\x7B\xA1\x69 %s \xA1\x6A\n"
      /* 以下是您在站內的註冊資料：\n */
      "\xA5\x48\xA4\x55\xAC\x4F\xB1\x7A\xA6\x62\xAF\xB8\xA4\xBA\xAA\xBA\xB5\xF9\xA5\x55\xB8\xEA\xAE\xC6\xA1\x47\n",
      cuser.userid, cuser.username, str_site);
    acct_show(&cuser, 0);
    vmsg(NULL);
  }

  u_exit("EXIT ");
  exit(0);
}


/* ----------------------------------------------------- */
/* help & menu processring				 */
/* ----------------------------------------------------- */


void
vs_head(title, mid)
  char *title, *mid;
{
  char buf[(T_COLS - 1) - 79 + 69 + 1];		/* d_cols 最大可能是 (T_COLS - 1) */
  char ttl[(T_COLS - 1) - 79 + 69 + 1];
  int spc, len;

  if (mid)	/* xxxx_head() 都是用 vs_head(title, str_site); */
  {
    clear();
  }
  else		/* menu() 中才用 vs_head(title, NULL); 選單中無需 clear() */
  {
    move(0, 0);
    clrtoeol();
    mid = str_site;
  }

  len = d_cols + 69 - strlen(title) - strlen(currboard);	/* len: 中間還剩下多長的空間 */

  if (HAS_STATUS(STATUS_BIFF))
  {
    /* \033[5;41m 郵差來按鈴了 \033[m */
    mid = "\033[5;41m \xB6\x6C\xAE\x74\xA8\xD3\xAB\xF6\xB9\x61\xA4\x46 \033[m";
    spc = 14;
  }
  else
  {
    if ((spc = strlen(mid)) > len)	/* 空間不夠擺下原本要擺的 mid，只好把 mid 截斷 */
    {
      spc = len;
      memcpy(ttl, mid, spc);
      mid = ttl;
      mid[spc] = '\0';
    }
  }

  spc = 2 + len - spc;		/* 擺完 mid 以後，中間還有 spc 格空間，在 mid 左右各放 spc/2 長的空白 */
  len = 1 - spc & 1;
  memset(buf, ' ', spc >>= 1);
  buf[spc] = '\0';

#ifdef COLOR_HEADER
  spc = (time(0) % 7) + '1';
  /* \033[1;4%cm【%s】%s\033[33m%s\033[1;37;4%cm%s《%s》\033[m\n */
  prints("\033[1;4%cm\xA1\x69%s\xA1\x6A%s\033[33m%s\033[1;37;4%cm%s\xA1\x6D%s\xA1\x6E\033[m\n",
    spc, title, buf, mid, spc, buf + len, currboard);
#else
  /* \033[1;44m【%s】%s\033[33m%s\033[1;37;44m%s《%s》\033[m\n */
  prints("\033[1;44m\xA1\x69%s\xA1\x6A%s\033[33m%s\033[1;37;44m%s\xA1\x6D%s\xA1\x6E\033[m\n",
    title, buf, mid, buf + len, currboard);
#endif
}


/* ------------------------------------- */
/* 動畫處理				 */
/* ------------------------------------- */


static char feeter[160];


/* itoc.010403: 把 feeter 的 status 獨立出來，預備供其他 function 叫用 */

static void
status_foot()
{
  static int orig_flag = -1;
  static time_t uptime = -1;
  static int orig_money = -1;
  static int orig_gold = -1;
  static char flagmsg[16];
  static char coinmsg[20];

  int ufo;
  time_t now;

  ufo = cuser.ufo;
  time(&now);

  /* Thor: 同時 顯示 呼叫器 上站通知 隱身 */

#ifdef HAVE_ALOHA
  ufo &= UFO_PAGER | UFO_ALOHA | UFO_CLOAK | UFO_QUIET;
  if (orig_flag != ufo)
  {
    orig_flag = ufo;
    sprintf(flagmsg,
      "%s%s%s%s",
      /* 關 */
      /* 開 */
      (ufo & UFO_PAGER) ? "\xC3\xF6" : "\xB6\x7D",
      /* 上 */
      (ufo & UFO_ALOHA) ? "\xA4\x57" : "  ",
      /* 靜 */
      (ufo & UFO_QUIET) ? "\xC0\x52" : "  ",
      /* 隱 */
      (ufo & UFO_CLOAK) ? "\xC1\xF4" : "  ");
  }
#else
  ufo &= UFO_PAGER | UFO_CLOAK | UFO_QUIET;
  if (orig_flag != ufo)
  {
    orig_flag = ufo;
    sprintf(flagmsg,
      "%s%s%s  ",
      /* 關 */
      /* 開 */
      (ufo & UFO_PAGER) ? "\xC3\xF6" : "\xB6\x7D",
      /* 靜 */
      (ufo & UFO_QUIET) ? "\xC0\x52" : "  ",
      /* 隱 */
      (ufo & UFO_CLOAK) ? "\xC1\xF4" : "  ");
  }
#endif

  if (now > uptime)	/* 過了子夜要更新生日旗標 */
  {
    struct tm *ptime;

    ptime = localtime(&now);

    if (cuser.day == ptime->tm_mday && cuser.month == ptime->tm_mon + 1)
      cutmp->status |= STATUS_BIRTHDAY;
    else
      cutmp->status &= ~STATUS_BIRTHDAY;

    uptime = now + 86400 - ptime->tm_hour * 3600 - ptime->tm_min * 60 - ptime->tm_sec;
  }

  if (cuser.money != orig_money)
  {
    orig_money = cuser.money;
    /* 銀%4d%c */
    sprintf(coinmsg, "\xBB\xC8%4d%c",
      (orig_money & 0x7FF00000) ? (orig_money >> 20) : (orig_money & 0x7FFFFC00) ? (orig_money >> 10) : orig_money,
      (orig_money & 0x7FF00000) ? 'M' : (orig_money & 0x7FFFFC00) ? 'K' : ' ');
    coinmsg[7] = ' ';
  }
  if (cuser.gold != orig_gold)
  {
    orig_gold = cuser.gold;
    /* 金%4d%c  */
    sprintf(coinmsg + 8, "\xAA\xF7%4d%c ",
      (orig_gold & 0x7FF00000) ? (orig_gold >> 20) : (orig_gold & 0x7FFFFC00) ? (orig_gold >> 10) : orig_gold,
      (orig_gold & 0x7FF00000) ? 'M' : (orig_gold & 0x7FFFFC00) ? 'K' : ' ');
  }

  /* Thor.980913.註解: 最常見呼叫 status_foot() 的時機是每次更新 film，在 60 秒以上，
                       故不需針對 hh:mm 來特別作一字串儲存以加速 */

  ufo = (now - (uptime - 86400)) / 60;	/* 借用 ufo 來做時間(分) */

  /* itoc.010717: 改一下 feeter 使長度和 FEETER_XXX 一致 */
  /*  人數 %-4d 我是 %-12s %s [呼叫]%-9s  */
  sprintf(feeter, COLOR1 " %8.8s %02d:%02d " COLOR2 " \xA4\x48\xBC\xC6 %-4d \xA7\xDA\xAC\x4F %-12s %s [\xA9\x49\xA5\x73]%-9s ",
    fshm->today, ufo / 60, ufo % 60, total_user, cuser.userid, coinmsg, flagmsg);
  outf(feeter);
}


void
movie()
{
  /* Thor: it is depend on which state */

  if ((bbsmode <= M_XMENU) && (cuser.ufo & UFO_MOVIE))
    film_out(FILM_MOVIE, MENU_XNOTE);

  /* itoc.010403: 把 feeter 的 status 獨立出來 */
  status_foot();
}


typedef struct
{
  void *func;
  /* int (*func) (); */
  usint level;
  int umode;
  char *desc;
}      MENU;


#define	MENU_LOAD	1
#define	MENU_DRAW	2
#define	MENU_FILM	4


#define	PERM_MENU	PERM_PURGE


static MENU menu_main[];


/* ----------------------------------------------------- */
/* administrator's maintain menu			 */
/* ----------------------------------------------------- */


static MENU menu_admin[] =
{
  "bin/admutil.so:a_user", PERM_ALLACCT, - M_SYSTEM,
  /* User       ◤ 顧客資料 ◢ */
  "User       \xA2\xAB \xC5\x55\xAB\xC8\xB8\xEA\xAE\xC6 \xA2\xA8",

  "bin/admutil.so:a_search", PERM_ALLACCT, - M_SYSTEM,
  /* Hunt       ◤ 搜尋引擎 ◢ */
  "Hunt       \xA2\xAB \xB7\x6A\xB4\x4D\xA4\xDE\xC0\xBA \xA2\xA8",

  "bin/admutil.so:a_editbrd", PERM_ALLBOARD, - M_SYSTEM,
  /* QSetBoard  ◤ 設定看板 ◢ */
  "QSetBoard  \xA2\xAB \xB3\x5D\xA9\x77\xAC\xDD\xAA\x4F \xA2\xA8",

  "bin/innbbs.so:a_innbbs", PERM_ALLBOARD, - M_SYSTEM,
  /* InnBBS     ◤ 轉信設定 ◢ */
  "InnBBS     \xA2\xAB \xC2\xE0\xAB\x48\xB3\x5D\xA9\x77 \xA2\xA8",

#ifdef HAVE_REGISTER_FORM
  "bin/admutil.so:a_register", PERM_ALLREG, - M_SYSTEM,
  /* Register   ◤ 審註冊單 ◢ */
  "Register   \xA2\xAB \xBC\x66\xB5\xF9\xA5\x55\xB3\xE6 \xA2\xA8",

  "bin/admutil.so:a_regmerge", PERM_ALLREG, - M_SYSTEM,
  /* Merge      ◤ 復原審核 ◢ */
  "Merge      \xA2\xAB \xB4\x5F\xAD\xEC\xBC\x66\xAE\xD6 \xA2\xA8",
#endif

  "bin/admutil.so:a_xfile", PERM_ALLADMIN, - M_XFILES,
  /* Xfile      ◤ 系統檔案 ◢ */
  "Xfile      \xA2\xAB \xA8\x74\xB2\xCE\xC0\xC9\xAE\xD7 \xA2\xA8",

  "bin/admutil.so:a_resetsys", PERM_ALLADMIN, - M_SYSTEM,
  /* BBSreset   ◤ 重置系統 ◢ */
  "BBSreset   \xA2\xAB \xAD\xAB\xB8\x6D\xA8\x74\xB2\xCE \xA2\xA8",

  "bin/admutil.so:a_restore", PERM_SYSOP, - M_SYSTEM,
  /* TRestore   ◤ 還原備份 ◢ */
  "TRestore   \xA2\xAB \xC1\xD9\xAD\xEC\xB3\xC6\xA5\xF7 \xA2\xA8",

  menu_main, PERM_MENU + Ctrl('A'), M_AMENU,
  /* 系統維護 */
  "\xA8\x74\xB2\xCE\xBA\xFB\xC5\x40"
};


/* ----------------------------------------------------- */
/* mail menu						 */
/* ----------------------------------------------------- */


static int
XoMbox()
{
  xover(XZ_MBOX);
  return 0;
}


static MENU menu_mail[] =
{
  XoMbox, PERM_BASIC, M_RMAIL,
  /* Read       ├ 閱讀信件 ┤ */
  "Read       \xA2\x75 \xBE\x5C\xC5\xAA\xAB\x48\xA5\xF3 \xA2\x74",

  m_send, PERM_LOCAL, M_SMAIL,
  /* Mail       ├ 站內寄信 ┤ */
  "Mail       \xA2\x75 \xAF\xB8\xA4\xBA\xB1\x48\xAB\x48 \xA2\x74",

#ifdef MULTI_MAIL  /* Thor.981009: 防止愛情幸運信 */
  m_list, PERM_LOCAL, M_SMAIL,
  /* List       ├ 群組寄信 ┤ */
  "List       \xA2\x75 \xB8\x73\xB2\xD5\xB1\x48\xAB\x48 \xA2\x74",
#endif

  m_internet, PERM_INTERNET, M_SMAIL,
  /* Internet   ├ 寄依妹兒 ┤ */
  "Internet   \xA2\x75 \xB1\x48\xA8\xCC\xA9\x66\xA8\xE0 \xA2\x74",

#ifdef HAVE_SIGNED_MAIL
  m_verify, 0, M_XMODE,
  /* Verify     ├ 驗證信件 ┤ */
  "Verify     \xA2\x75 \xC5\xE7\xC3\xD2\xAB\x48\xA5\xF3 \xA2\x74",
#endif

#ifdef HAVE_MAIL_ZIP
  m_zip, PERM_INTERNET, M_SMAIL,
  /* Zip        ├ 打包資料 ┤ */
  "Zip        \xA2\x75 \xA5\xB4\xA5\x5D\xB8\xEA\xAE\xC6 \xA2\x74",
#endif

  m_sysop, 0, M_SMAIL,
  /* Yes Sir!   ├ 投書站長 ┤ */
  "Yes Sir!   \xA2\x75 \xA7\xEB\xAE\xD1\xAF\xB8\xAA\xF8 \xA2\x74",

  "bin/admutil.so:m_bm", PERM_ALLADMIN, - M_SMAIL,
  /* BM All     ├ 板主通告 ┤ */
  "BM All     \xA2\x75 \xAA\x4F\xA5\x44\xB3\x71\xA7\x69 \xA2\x74",	/* itoc.000512: 新增 m_bm */

  "bin/admutil.so:m_all", PERM_ALLADMIN, - M_SMAIL,
  /* User All   ├ 全站通告 ┤ */
  "User All   \xA2\x75 \xA5\xFE\xAF\xB8\xB3\x71\xA7\x69 \xA2\x74",	/* itoc.000512: 新增 m_all */

  menu_main, PERM_MENU + Ctrl('A'), M_MMENU,	/* itoc.020829: 怕 guest 沒選項 */
  /* 電子郵件 */
  "\xB9\x71\xA4\x6C\xB6\x6C\xA5\xF3"
};


/* ----------------------------------------------------- */
/* talk menu						 */
/* ----------------------------------------------------- */


static int
XoUlist()
{
  xover(XZ_ULIST);
  return 0;
}


static MENU menu_talk[];


  /* --------------------------------------------------- */
  /* list menu						 */
  /* --------------------------------------------------- */

static MENU menu_list[] =
{
  t_pal, PERM_BASIC, M_PAL,
  /* Pal        → 朋友名單 ← */
  "Pal        \xA1\xF7 \xAA\x42\xA4\xCD\xA6\x57\xB3\xE6 \xA1\xF6",

#ifdef HAVE_LIST
  t_list, PERM_BASIC, M_PAL,
  /* List       → 特別名單 ← */
  "List       \xA1\xF7 \xAF\x53\xA7\x4F\xA6\x57\xB3\xE6 \xA1\xF6",
#endif

#ifdef HAVE_ALOHA
  "bin/aloha.so:t_aloha", PERM_PAGE, - M_PAL,
  /* Aloha      → 上站通知 ← */
  "Aloha      \xA1\xF7 \xA4\x57\xAF\xB8\xB3\x71\xAA\xBE \xA1\xF6",
#endif

#ifdef LOGIN_NOTIFY
  t_loginNotify, PERM_PAGE, M_PAL,
  /* Notify     → 系統協尋 ← */
  "Notify     \xA1\xF7 \xA8\x74\xB2\xCE\xA8\xF3\xB4\x4D \xA1\xF6",
#endif

  menu_talk, PERM_MENU + 'P', M_TMENU,
  /* 各類名單 */
  "\xA6\x55\xC3\xFE\xA6\x57\xB3\xE6"
};


static MENU menu_talk[] =
{
  XoUlist, 0, M_LUSERS,
  /* Users      → 遊客名單 ← */
  "Users      \xA1\xF7 \xB9\x43\xAB\xC8\xA6\x57\xB3\xE6 \xA1\xF6",

  menu_list, PERM_BASIC, M_TMENU,
  /* ListMenu   → 設定名單 ← */
  "ListMenu   \xA1\xF7 \xB3\x5D\xA9\x77\xA6\x57\xB3\xE6 \xA1\xF6",

  t_pager, PERM_BASIC, M_XMODE,
  /* Pager      → 切換呼叫 ← */
  "Pager      \xA1\xF7 \xA4\xC1\xB4\xAB\xA9\x49\xA5\x73 \xA1\xF6",

  t_cloak, PERM_CLOAK, M_XMODE,
  /* Invis      → 隱身密法 ← */
  "Invis      \xA1\xF7 \xC1\xF4\xA8\xAD\xB1\x4B\xAA\x6B \xA1\xF6",

  t_query, 0, M_QUERY,
  /* Query      → 查詢網友 ← */
  "Query      \xA1\xF7 \xAC\x64\xB8\xDF\xBA\xF4\xA4\xCD \xA1\xF6",

  t_talk, PERM_PAGE, M_PAGE,
  /* Talk       → 情話綿綿 ← */
  "Talk       \xA1\xF7 \xB1\xA1\xB8\xDC\xBA\xF8\xBA\xF8 \xA1\xF6",

  /* Thor.990220: 改採外掛 */
  "bin/chat.so:t_chat", PERM_CHAT, - M_CHAT,
  /* ChatRoom   → 眾口鑠金 ← */
  "ChatRoom   \xA1\xF7 \xB2\xB3\xA4\x66\xC5\xE0\xAA\xF7 \xA1\xF6",

  t_display, PERM_BASIC, M_BMW,
  /* Display    → 瀏覽水球 ← */
  "Display    \xA1\xF7 \xC2\x73\xC4\xFD\xA4\xF4\xB2\x79 \xA1\xF6",

  t_bmw, PERM_BASIC, M_BMW,
  /* Write      → 回顧水球 ← */
  "Write      \xA1\xF7 \xA6\x5E\xC5\x55\xA4\xF4\xB2\x79 \xA1\xF6",

  menu_main, PERM_MENU + 'U', M_TMENU,
  /* 休閒聊天 */
  "\xA5\xF0\xB6\xA2\xB2\xE1\xA4\xD1"
};


/* ----------------------------------------------------- */
/* user menu						 */
/* ----------------------------------------------------- */


static MENU menu_user[];


  /* --------------------------------------------------- */
  /* register menu                                      */
  /* --------------------------------------------------- */

static MENU menu_register[] =
{
  u_addr, PERM_BASIC, M_XMODE,
  /* Address    《 電子信箱 》 */
  "Address    \xA1\x6D \xB9\x71\xA4\x6C\xAB\x48\xBD\x63 \xA1\x6E",

#ifdef HAVE_REGISTER_FORM
  u_register, PERM_BASIC, M_UFILES,
  /* Register   《 填註冊單 》 */
  "Register   \xA1\x6D \xB6\xF1\xB5\xF9\xA5\x55\xB3\xE6 \xA1\x6E",
#endif

#ifdef HAVE_REGKEY_CHECK
  u_verify, PERM_BASIC, M_UFILES,
  /* Verify     《 填認證碼 》 */
  "Verify     \xA1\x6D \xB6\xF1\xBB\x7B\xC3\xD2\xBD\x58 \xA1\x6E",
#endif

  u_deny, PERM_BASIC, M_XMODE,
  /* Perm       《 恢復權限 》 */
  "Perm       \xA1\x6D \xAB\xEC\xB4\x5F\xC5\x76\xAD\xAD \xA1\x6E",

  menu_user, PERM_MENU + 'A', M_UMENU,
  /* 註冊選單 */
  "\xB5\xF9\xA5\x55\xBF\xEF\xB3\xE6"
};


static MENU menu_user[] =
{
  u_info, PERM_BASIC, M_XMODE,
  /* Info       《 個人資料 》 */
  "Info       \xA1\x6D \xAD\xD3\xA4\x48\xB8\xEA\xAE\xC6 \xA1\x6E",

  u_setup, 0, M_UFILES,
  /* Habit      《 喜好模式 》 */
  "Habit      \xA1\x6D \xB3\xDF\xA6\x6E\xBC\xD2\xA6\xA1 \xA1\x6E",

  menu_register, PERM_BASIC, M_UMENU,
  /* Register   《 註冊選單 》 */
  "Register   \xA1\x6D \xB5\xF9\xA5\x55\xBF\xEF\xB3\xE6 \xA1\x6E",

  pad_view, 0, M_READA,
  /* Note       《 觀看留言 》 */
  "Note       \xA1\x6D \xC6\x5B\xAC\xDD\xAF\x64\xA8\xA5 \xA1\x6E",

  /* itoc.010309: 不必離站可以寫留言板 */
  pad_draw, PERM_POST, M_POST,
  /* Pad        《 心情塗鴉 》 */
  "Pad        \xA1\x6D \xA4\xDF\xB1\xA1\xB6\xEE\xBE\x7E \xA1\x6E",

  u_lock, PERM_BASIC, M_IDLE,
  /* Lock       《 鎖定螢幕 》 */
  "Lock       \xA1\x6D \xC2\xEA\xA9\x77\xBF\xC3\xB9\xF5 \xA1\x6E",

  u_xfile, PERM_BASIC, M_UFILES,
  /* Xfile      《 個人檔案 》 */
  "Xfile      \xA1\x6D \xAD\xD3\xA4\x48\xC0\xC9\xAE\xD7 \xA1\x6E",

  u_log, PERM_BASIC, M_UFILES,
  /* ViewLog    《 上站記錄 》 */
  "ViewLog    \xA1\x6D \xA4\x57\xAF\xB8\xB0\x4F\xBF\xFD \xA1\x6E",

  menu_main, PERM_MENU + 'H', M_UMENU,
  /* 個人設定 */
  "\xAD\xD3\xA4\x48\xB3\x5D\xA9\x77"
};


#ifdef HAVE_EXTERNAL

/* ----------------------------------------------------- */
/* tool menu						 */
/* ----------------------------------------------------- */


static MENU menu_tool[];


#ifdef HAVE_SONG
  /* --------------------------------------------------- */
  /* song menu						 */
  /* --------------------------------------------------- */

static MENU menu_song[] =
{
  "bin/song.so:XoSongLog", 0, - M_XMODE,
  /* KTV        ♂ 點歌紀錄 ♀ */
  "KTV        \xA1\xF1 \xC2\x49\xBA\x71\xAC\xF6\xBF\xFD \xA1\xF0",

  "bin/song.so:XoSongMain", 0, - M_XMODE,
  /* Book       ♂ 唱所欲言 ♀ */
  "Book       \xA1\xF1 \xB0\xDB\xA9\xD2\xB1\xFD\xA8\xA5 \xA1\xF0",

  "bin/song.so:XoSongSub", 0, - M_XMODE,
  /* Note       ♂ 歌本投稿 ♀ */
  "Note       \xA1\xF1 \xBA\x71\xA5\xBB\xA7\xEB\xBD\x5A \xA1\xF0",

  menu_tool, PERM_MENU + 'K', M_XMENU,
  /* 玩點歌機 */
  "\xAA\xB1\xC2\x49\xBA\x71\xBE\xF7"
};
#endif


#ifdef HAVE_GAME

#if 0

  itoc.010426.註解:
  益智遊戲不用賭金制度，讓玩家玩好玩的，只加錢，不減錢。

  itoc.010714.註解:
  (a) 每次玩遊戲的總期望值應在 1.01，一個晚上約可玩 100 次遊戲，
      若將總財產投入去玩遊戲，則 1.01^100 = 2.7 倍/每玩一個晚上。
  (b) 若各項機率不均等，也應維持在 1.0 ~ 1.02 之間，讓玩家一定能賺錢，
      且若一直押最高期望值的那一項，也不會賺得過於離譜。
  (c) 原則上，機率越低者其期望值應為 1.02，機率較高者其期望值應為 1.01。

  itoc.011011.註解:
  為了避免 user multi-login 玩來洗錢，
  所以在玩遊戲的開始就要檢查是否重覆 login 即 if (HAS_STATUS(STATUS_COINLOCK))。

#endif

  /* --------------------------------------------------- */
  /* game menu						 */
  /* --------------------------------------------------- */

static MENU menu_game[];

static MENU menu_game1[] =
{
  "bin/liteon.so:main_liteon", 0, - M_GAME,
  /* 0LightOn   ♂ 房間開燈 ♀ */
  "0LightOn   \xA1\xF1 \xA9\xD0\xB6\xA1\xB6\x7D\xBF\x4F \xA1\xF0",

  "bin/guessnum.so:guessNum", 0, - M_GAME,
  /* 1GuessNum  ♂ 玩猜數字 ♀ */
  "1GuessNum  \xA1\xF1 \xAA\xB1\xB2\x71\xBC\xC6\xA6\x72 \xA1\xF0",

  "bin/guessnum.so:fightNum", 0, - M_GAME,
  /* 2FightNum  ♂ 互猜數字 ♀ */
  "2FightNum  \xA1\xF1 \xA4\xAC\xB2\x71\xBC\xC6\xA6\x72 \xA1\xF0",

  "bin/km.so:main_km", 0, - M_GAME,
  /* 3KongMing  ♂ 孔明棋譜 ♀ */
  "3KongMing  \xA1\xF1 \xA4\xD5\xA9\xFA\xB4\xD1\xC3\xD0 \xA1\xF0",

  "bin/recall.so:main_recall", 0, - M_GAME,
  /* 4Recall    ♂ 回憶之卵 ♀ */
  "4Recall    \xA1\xF1 \xA6\x5E\xBE\xD0\xA4\xA7\xA7\x5A \xA1\xF0",

  "bin/mine.so:main_mine", 0, - M_GAME,
  /* 5Mine      ♂ 亂踩地雷 ♀ */
  "5Mine      \xA1\xF1 \xB6\xC3\xBD\xF2\xA6\x61\xB9\x70 \xA1\xF0",

  "bin/fantan.so:main_fantan", 0, - M_GAME,
  /* 6Fantan    ♂ 番攤接龍 ♀ */
  "6Fantan    \xA1\xF1 \xB5\x66\xC5\x75\xB1\xB5\xC0\x73 \xA1\xF0",

  "bin/dragon.so:main_dragon", 0, - M_GAME,
  /* 7Dragon    ♂ 接龍遊戲 ♀ */
  "7Dragon    \xA1\xF1 \xB1\xB5\xC0\x73\xB9\x43\xC0\xB8 \xA1\xF0",

  "bin/nine.so:main_nine", 0, - M_GAME,
  /* 8Nine      ♂ 天地九九 ♀ */
  "8Nine      \xA1\xF1 \xA4\xD1\xA6\x61\xA4\x45\xA4\x45 \xA1\xF0",

  menu_game, PERM_MENU + '0', M_XMENU,
  /* 益智空間 */
  "\xAF\x71\xB4\xBC\xAA\xC5\xB6\xA1"
};

static MENU menu_game2[] =
{
  "bin/dice.so:main_dice", 0, - M_GAME,
  /* 0Dice      ♂ 狂擲骰子 ♀ */
  "0Dice      \xA1\xF1 \xA8\x67\xC2\x59\xBB\xEB\xA4\x6C \xA1\xF0",

  "bin/gp.so:main_gp", 0, - M_GAME,
  /* 1GoldPoker ♂ 金牌撲克 ♀ */
  "1GoldPoker \xA1\xF1 \xAA\xF7\xB5\x50\xBC\xB3\xA7\x4A \xA1\xF0",

  "bin/bj.so:main_bj", 0, - M_GAME,
  /* 2BlackJack ♂ 二十一點 ♀ */
  "2BlackJack \xA1\xF1 \xA4\x47\xA4\x51\xA4\x40\xC2\x49 \xA1\xF0",

  "bin/chessmj.so:main_chessmj", 0, - M_GAME,
  /* 3ChessMJ   ♂ 象棋麻將 ♀ */
  "3ChessMJ   \xA1\xF1 \xB6\x48\xB4\xD1\xB3\xC2\xB1\x4E \xA1\xF0",

  "bin/seven.so:main_seven", 0, - M_GAME,
  /* 4Seven     ♂ 賭城七張 ♀ */
  "4Seven     \xA1\xF1 \xBD\xE4\xAB\xB0\xA4\x43\xB1\x69 \xA1\xF0",

  "bin/race.so:main_race", 0, - M_GAME,
  /* 5Race      ♂ 進賽馬場 ♀ */
  "5Race      \xA1\xF1 \xB6\x69\xC1\xC9\xB0\xA8\xB3\xF5 \xA1\xF0",

  "bin/bingo.so:main_bingo", 0, - M_GAME,
  /* 6Bingo     ♂ 賓果大戰 ♀ */
  "6Bingo     \xA1\xF1 \xBB\xAB\xAA\x47\xA4\x6A\xBE\xD4 \xA1\xF0",

  "bin/marie.so:main_marie", 0, - M_GAME,
  /* 7Marie     ♂ 大小瑪莉 ♀ */
  "7Marie     \xA1\xF1 \xA4\x6A\xA4\x70\xBA\xBF\xB2\xFA \xA1\xF0",

  "bin/bar.so:main_bar", 0, - M_GAME,
  /* 8Bar       ♂ 吧台瑪莉 ♀ */
  "8Bar       \xA1\xF1 \xA7\x61\xA5\x78\xBA\xBF\xB2\xFA \xA1\xF0",

  menu_game, PERM_MENU + '0', M_XMENU,
  /* 遊戲樂園 */
  "\xB9\x43\xC0\xB8\xBC\xD6\xB6\xE9"
};

static MENU menu_game3[] =
{
  "bin/pip.so:main_pip", PERM_BASIC, - M_GAME,
  /* 0Chicken   ♂ 電子小雞 ♀ */
  "0Chicken   \xA1\xF1 \xB9\x71\xA4\x6C\xA4\x70\xC2\xFB \xA1\xF0",

  "bin/pushbox.so:main_pushbox", 0, - M_GAME,
  /* 1PushBox   ♂ 倉庫番番 ♀ */
  "1PushBox   \xA1\xF1 \xAD\xDC\xAE\x77\xB5\x66\xB5\x66 \xA1\xF0",

  "bin/tetris.so:main_tetris", 0, - M_GAME,
  /* 2Tetris    ♂ 俄羅斯塊 ♀ */
  "2Tetris    \xA1\xF1 \xAB\x58\xC3\xB9\xB4\xB5\xB6\xF4 \xA1\xF0",

  "bin/reversi.so:main_reversi", 0, - M_GAME,
  /* 3Reversi   ♂ 淺灰大戰 ♀ */
  "3Reversi   \xA1\xF1 \xB2\x4C\xA6\xC7\xA4\x6A\xBE\xD4 \xA1\xF0",

  menu_game, PERM_MENU + '0', M_XMENU,
  /* 反斗特區 */
  "\xA4\xCF\xA4\xE6\xAF\x53\xB0\xCF"
};

static MENU menu_game[] =
{
  menu_game1, PERM_BASIC, M_XMENU,
  /* 1Game      【 益智天堂 】 */
  "1Game      \xA1\x69 \xAF\x71\xB4\xBC\xA4\xD1\xB0\xF3 \xA1\x6A",

  menu_game2, PERM_BASIC, M_XMENU,
  /* 2Game      【 遊戲樂園 】 */
  "2Game      \xA1\x69 \xB9\x43\xC0\xB8\xBC\xD6\xB6\xE9 \xA1\x6A",

  menu_game3, PERM_BASIC, M_XMENU,
  /* 3Game      【 反斗特區 】 */
  "3Game      \xA1\x69 \xA4\xCF\xA4\xE6\xAF\x53\xB0\xCF \xA1\x6A",

  menu_tool, PERM_MENU + '1', M_XMENU,
  /* 遊戲人生 */
  "\xB9\x43\xC0\xB8\xA4\x48\xA5\xCD"
};
#endif


#ifdef HAVE_BUY
  /* --------------------------------------------------- */
  /* buy menu						 */
  /* --------------------------------------------------- */

static MENU menu_buy[] =
{
  "bin/bank.so:x_bank", PERM_BASIC, - M_GAME,
  /* Bank       ♂ 信託銀行 ♀ */
  "Bank       \xA1\xF1 \xAB\x48\xB0\x55\xBB\xC8\xA6\xE6 \xA1\xF0",

  "bin/bank.so:b_invis", PERM_BASIC, - M_GAME,
  /* Invis      ♂ 隱形現身 ♀ */
  "Invis      \xA1\xF1 \xC1\xF4\xA7\xCE\xB2\x7B\xA8\xAD \xA1\xF0",

  "bin/bank.so:b_cloak", PERM_BASIC, - M_GAME,
  /* Cloak      ♂ 無限隱形 ♀ */
  "Cloak      \xA1\xF1 \xB5\x4C\xAD\xAD\xC1\xF4\xA7\xCE \xA1\xF0",

  "bin/bank.so:b_mbox", PERM_BASIC, - M_GAME,
  /* Mbox       ♂ 信箱無限 ♀ */
  "Mbox       \xA1\xF1 \xAB\x48\xBD\x63\xB5\x4C\xAD\xAD \xA1\xF0",

  "bin/bank.so:b_xempt", PERM_BASIC, - M_GAME,
  /* Xempt      ♂ 永久保留 ♀ */
  "Xempt      \xA1\xF1 \xA5\xC3\xA4\x5B\xAB\x4F\xAF\x64 \xA1\xF0",

  menu_tool, PERM_MENU + 'B', M_XMENU,
  /* 金融市場 */
  "\xAA\xF7\xBF\xC4\xA5\xAB\xB3\xF5"
};
#endif


  /* --------------------------------------------------- */
  /* other tools menu					 */
  /* --------------------------------------------------- */

static MENU menu_other[] =
{
  "bin/vote.so:vote_all", PERM_BASIC, - M_VOTE,	/* itoc.010414: 投票中心 */
  /* VoteAll    ♂ 投票中心 ♀ */
  "VoteAll    \xA1\xF1 \xA7\xEB\xB2\xBC\xA4\xA4\xA4\xDF \xA1\xF0",

#ifdef HAVE_TIP
  "bin/xyz.so:x_tip", 0, - M_READA,
  /* Tip        ♂ 教學精靈 ♀ */
  "Tip        \xA1\xF1 \xB1\xD0\xBE\xC7\xBA\xEB\xC6\x46 \xA1\xF0",
#endif

#ifdef HAVE_LOVELETTER
  "bin/xyz.so:x_loveletter", 0, - M_READA,
  /* LoveLetter ♂ 情書撰寫 ♀ */
  "LoveLetter \xA1\xF1 \xB1\xA1\xAE\xD1\xBC\xB6\xBC\x67 \xA1\xF0",
#endif

  "bin/xyz.so:x_password", PERM_VALID, - M_XMODE,
  /* Password   ♂ 忘記密碼 ♀ */
  "Password   \xA1\xF1 \xA7\xD1\xB0\x4F\xB1\x4B\xBD\x58 \xA1\xF0",

#ifdef HAVE_CLASSTABLE
  "bin/classtable.so:main_classtable", PERM_BASIC, - M_XMODE,
  /* ClassTable ♂ 功課時段 ♀ */
  "ClassTable \xA1\xF1 \xA5\x5C\xBD\xD2\xAE\xC9\xAC\x71 \xA1\xF0",
#endif

#ifdef HAVE_CREDIT
  "bin/credit.so:main_credit", PERM_BASIC, - M_XMODE,
  /* MoneyNote  ♂ 記帳手札 ♀ */
  "MoneyNote  \xA1\xF1 \xB0\x4F\xB1\x62\xA4\xE2\xA5\xBE \xA1\xF0",
#endif

#ifdef HAVE_CALENDAR
  "bin/todo.so:main_todo", PERM_BASIC, - M_XMODE,
  /* XTodo      ♂ 個人行程 ♀ */
  "XTodo      \xA1\xF1 \xAD\xD3\xA4\x48\xA6\xE6\xB5\x7B \xA1\xF0",

  "bin/calendar.so:main_calendar", 0, - M_XMODE,
  /* YCalendar  ♂ 萬年月曆 ♀ */
  "YCalendar  \xA1\xF1 \xB8\x55\xA6\x7E\xA4\xEB\xBE\xE4 \xA1\xF0",
#endif

  menu_tool, PERM_MENU + Ctrl('A'), M_XMENU,	/* itoc.020829: 怕 guest 沒選項 */
  /* 其他功能 */
  "\xA8\xE4\xA5\x4C\xA5\x5C\xAF\xE0"
};


static MENU menu_tool[] =
{
#ifdef HAVE_SONG
  menu_song, 0, M_XMENU,
  /* KTV        【 真情點歌 】 */
  "KTV        \xA1\x69 \xAF\x75\xB1\xA1\xC2\x49\xBA\x71 \xA1\x6A",
#endif

#ifdef HAVE_COSIGN
  "bin/newbrd.so:XoNewBoard", PERM_VALID, - M_XMODE,
  /* Join       【 看板連署 】 */
  "Join       \xA1\x69 \xAC\xDD\xAA\x4F\xB3\x73\xB8\x70 \xA1\x6A",
#endif

#ifdef HAVE_GAME
  menu_game, PERM_BASIC, M_XMENU,
  /* Game       【 遊戲人生 】 */
  "Game       \xA1\x69 \xB9\x43\xC0\xB8\xA4\x48\xA5\xCD \xA1\x6A",
#endif

#ifdef HAVE_BUY
  menu_buy, PERM_BASIC, M_XMENU,
  /* Market     【 金融市場 】 */
  "Market     \xA1\x69 \xAA\xF7\xBF\xC4\xA5\xAB\xB3\xF5 \xA1\x6A",
#endif

  menu_other, 0, M_XMENU,
  /* Other      【 雜七雜八 】 */
  "Other      \xA1\x69 \xC2\xF8\xA4\x43\xC2\xF8\xA4\x4B \xA1\x6A",

  menu_main, PERM_MENU + Ctrl('A'), M_XMENU,	/* itoc.020829: 怕 guest 沒選項 */
  /* 個人工具 */
  "\xAD\xD3\xA4\x48\xA4\x75\xA8\xE3"
};

#endif	/* HAVE_EXTERNAL */


/* ----------------------------------------------------- */
/* main menu						 */
/* ----------------------------------------------------- */


static int
Gem()
{
  /* itoc.001109: 看板總管在 (A)nnounce 下有 GEM_X_BIT，方便開板 */
  /* 精華佈告欄 */
  XoGem("gem/"FN_DIR, "\xBA\xEB\xB5\xD8\xA7\x47\xA7\x69\xC4\xE6", (HAS_PERM(PERM_ALLBOARD) ? (GEM_W_BIT | GEM_X_BIT | GEM_M_BIT) : 0));
  return 0;
}


static MENU menu_main[] =
{
  menu_admin, PERM_ALLADMIN, M_AMENU,
  /* 0Admin    Φ 系統維護區 Φ */
  "0Admin    \xA3\x58 \xA8\x74\xB2\xCE\xBA\xFB\xC5\x40\xB0\xCF \xA3\x58",

  Gem, 0, M_GEM,
  /* Announce  ξ 精華公佈欄 ξ */
  "Announce  \xA3\x69 \xBA\xEB\xB5\xD8\xA4\xBD\xA7\x47\xC4\xE6 \xA3\x69",

  Boards, 0, M_BOARD,
  /* Boards    Ω 佈告討論區 Ω */
  "Boards    \xA3\x5B \xA7\x47\xA7\x69\xB0\x51\xBD\xD7\xB0\xCF \xA3\x5B",

  Class, 0, M_BOARD,
  /* Class     φ 分組討論集 φ */
  "Class     \xA3\x70 \xA4\xC0\xB2\xD5\xB0\x51\xBD\xD7\xB6\xB0 \xA3\x70",

#ifdef MY_FAVORITE
  MyFavorite, PERM_BASIC, M_MF,
  /* Favorite  η 我的最愛群 η */
  "Favorite  \xA3\x62 \xA7\xDA\xAA\xBA\xB3\xCC\xB7\x52\xB8\x73 \xA3\x62",
#endif

  menu_mail, 0, M_MMENU,
  /* Mail      μ 信件典藏盒 μ */
  "Mail      \xA3\x67 \xAB\x48\xA5\xF3\xA8\xE5\xC2\xC3\xB2\xB0 \xA3\x67",

  menu_talk, 0, M_TMENU,
  /* Talk      ω 休閒聊天地 ω */
  "Talk      \xA3\x73 \xA5\xF0\xB6\xA2\xB2\xE1\xA4\xD1\xA6\x61 \xA3\x73",

  menu_user, 0, M_UMENU,
  /* User      π 個人工具坊 π */
  "User      \xA3\x6B \xAD\xD3\xA4\x48\xA4\x75\xA8\xE3\xA7\x7B \xA3\x6B",

#ifdef HAVE_EXTERNAL
  menu_tool, 0, M_XMENU,
  /* Xyz       θ 特殊招待所 θ */
  "Xyz       \xA3\x63 \xAF\x53\xAE\xED\xA9\xDB\xAB\xDD\xA9\xD2 \xA3\x63",
#endif

#if 0	/* itoc.010209: 選單按 s 直接進入 Select() 減少選單長度 */
  Select, 0, M_BOARD,
  /* Select    σ 選擇主看板 σ */
  "Select    \xA3\x6D \xBF\xEF\xBE\xDC\xA5\x44\xAC\xDD\xAA\x4F \xA3\x6D",
#endif

  goodbye, 0, M_XMODE,
  /* Goodbye   δ 下次再會吧 δ */
  "Goodbye   \xA3\x5F \xA4\x55\xA6\xB8\xA6\x41\xB7\x7C\xA7\x61 \xA3\x5F",

  NULL, PERM_MENU + 'B', M_0MENU,
  /* 主功能表 */
  "\xA5\x44\xA5\x5C\xAF\xE0\xAA\xED"
};


void
menu()
{
  MENU *menu, *mptr, *table[12];
  usint level, mode;
  int cc, cx;			/* current / previous cursor position */
  int max, mmx;			/* current / previous menu max */
  int cmd, depth;
  char *str;

  mode = MENU_LOAD | MENU_DRAW | MENU_FILM;
  menu = menu_main;
  level = cuser.userlevel;
  depth = mmx = 0;

  for (;;)
  {
    if (mode & MENU_LOAD)
    {
      for (max = -1;; menu++)
      {
	cc = menu->level;
	if (cc & PERM_MENU)
	{

#ifdef	MENU_VERBOSE
	  if (max < 0)		/* 找不到適合權限之功能，回上一層功能表 */
	  {
	    menu = (MENU *) menu->func;
	    continue;
	  }
#endif

	  break;
	}
	if (cc && !(cc & level))	/* 有該權限才秀出 */
	  continue;

	table[++max] = menu;
      }

      if (mmx < max)
	mmx = max;

      if ((depth == 0) && HAS_STATUS(STATUS_BIFF))	/* 第一次上站若有新信，進入 Mail 選單 */
	cmd = 'M';
      else
	cmd = cc ^ PERM_MENU;	/* default command */
      utmp_mode(menu->umode);
    }

    if (mode & MENU_DRAW)
    {
      if (mode & MENU_FILM)
      {
	clear();
	movie();
	cx = -1;
      }

      vs_head(menu->desc, NULL);

      mode = 0;
      do
      {
	move(MENU_XPOS + mode, MENU_YPOS + 2);
	if (mode <= max)
	{
	  mptr = table[mode];
	  str = mptr->desc;
	  prints("(\033[1;36m%c\033[m)", *str++);
	  outs(str);
	}
	clrtoeol();
      } while (++mode <= mmx);

      mmx = max;
      mode = 0;
    }

    switch (cmd)
    {
    case KEY_DOWN:
      cc = (cc == max) ? 0 : cc + 1;
      break;

    case KEY_UP:
      cc = (cc == 0) ? max : cc - 1;
      break;

    case Ctrl('A'):	/* itoc.020829: 預設選項第一個 */
    case KEY_HOME:
      cc = 0;
      break;

    case KEY_END:
      cc = max;
      break;

    case KEY_PGUP:
      cc = (cc == 0) ? max : 0;
      break;

    case KEY_PGDN:
      cc = (cc == max) ? 0 : max;
      break;

    case '\n':
    case KEY_RIGHT:
      mptr = table[cc];
      cmd = mptr->umode;
#if 1
     /* Thor.990212: dynamic load , with negative umode */
      if (cmd < 0)
      {
	void *p = DL_get(mptr->func);
	if (!p)
	  break;
	mptr->func = p;
	cmd = -cmd;
	mptr->umode = cmd;
      }
#endif
      utmp_mode(cmd);

      if (cmd <= M_XMENU)	/* 子目錄的 mode 要 <= M_XMENU */
      {
	menu->level = PERM_MENU + mptr->desc[0];
	menu = (MENU *) mptr->func;

	mode = MENU_LOAD | MENU_DRAW;
	/* mode = MENU_LOAD | MENU_DRAW | MENU_FILM;	/* itoc.010304: 進入子選單重撥 movie */

	depth++;
	continue;
      }

      {
	int (*func) ();

	func = mptr->func;
	mode = (*func) ();
      }

      utmp_mode(menu->umode);

      if (mode == XEASY)
      {
	outf(feeter);
	mode = 0;
      }
      else
      {
	mode = MENU_DRAW | MENU_FILM;
      }

      cmd = mptr->desc[0];
      continue;

#ifdef EVERY_Z
    case Ctrl('Z'):
      every_Z(0);
      goto every_key;

    case Ctrl('U'):
      every_U(0);
      goto every_key;
#endif

    /* itoc.010911: Select everywhere，不再限制是在 M_0MENU */
    case 's':
    case Ctrl('S'):
      utmp_mode(M_BOARD);
      Select();
      goto every_key;

#ifdef MY_FAVORITE
    /* itoc.010911: Favorite everywhere，不再限制是在 M_0MENU */
    case 'f':
    case Ctrl('F'):
      if (cuser.userlevel)	/* itoc.010407: 要檢查權限 */
      {
	utmp_mode(M_MF);
	MyFavorite();
      }
      goto every_key;
#endif

    /* itoc.020301: Read currboard in M_0MENU */
    case 'r':
      if (bbsmode == M_0MENU)
      {
	if (currbno >= 0)
	{
	  utmp_mode(M_BOARD);
	  XoPost(currbno);
	  xover(XZ_POST);
#ifndef ENHANCED_VISIT
	  time(&brd_visit[currbno]);
#endif
	}
	goto every_key;
      }
      goto default_key;	/* 若不在 M_0MENU 中按 r 的話，要視為一般按鍵 */

every_key:	/* 特殊鍵處理結束 */
      utmp_mode(menu->umode);
      mode = MENU_DRAW | MENU_FILM;
      cmd = table[cc]->desc[0];
      continue;

    case KEY_LEFT:
    case 'e':
      if (depth > 0)
      {
	menu->level = PERM_MENU + table[cc]->desc[0];
	menu = (MENU *) menu->func;
	mode = MENU_LOAD | MENU_DRAW;
	/* mode = MENU_LOAD | MENU_DRAW | MENU_FILM;	/* itoc.010304: 退出子選單重撥 movie */
	depth--;
	continue;
      }
      cmd = 'G';

default_key:
    default:

      if (cmd >= 'a' && cmd <= 'z')
	cmd ^= 0x20;			/* 變大寫 */

      cc = 0;
      for (;;)
      {
	if (table[cc]->desc[0] == cmd)
	  break;
	if (++cc > max)
	{
	  cc = cx;
	  goto menu_key;
	}
      }
    }

    if (cc != cx)	/* 若游標移動位置 */
    {
#ifdef CURSOR_BAR
      if (cx >= 0)
      {
	move(MENU_XPOS + cx, MENU_YPOS);
	if (cx <= max)
	{
	  mptr = table[cx];
	  str = mptr->desc;
	  prints("  (\033[1;36m%c\033[m)%s ", *str, str + 1);
	}
	else
	{
	  outs("  ");
	}
      }
      move(MENU_XPOS + cc, MENU_YPOS);
      mptr = table[cc];
      str = mptr->desc;
      prints(COLOR4 "> (%c)%s \033[m", *str, str + 1);
      cx = cc;
#else		/* 沒有 CURSOR_BAR */
      if (cx >= 0)
      {
	move(MENU_XPOS + cx, MENU_YPOS);
	outc(' ');
      }
      move(MENU_XPOS + cc, MENU_YPOS);
      outc('>');
      cx = cc;
#endif
    }
    else		/* 若游標的位置沒有變 */
    {
#ifdef CURSOR_BAR
      move(MENU_XPOS + cc, MENU_YPOS);
      mptr = table[cc];
      str = mptr->desc;
      prints(COLOR4 "> (%c)%s \033[m", *str, str + 1);
#else
      move(MENU_XPOS + cc, MENU_YPOS + 1);
#endif
    }

menu_key:

    cmd = vkey();
  }
}
