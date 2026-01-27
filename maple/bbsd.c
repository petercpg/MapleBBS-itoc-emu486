/*-------------------------------------------------------*/
/* bbsd.c	( NTHU CS MapleBBS Ver 3.00 )		 */
/*-------------------------------------------------------*/
/* author : opus.bbs@bbs.cs.nthu.edu.tw		 	 */
/* target : BBS daemon/main/login/top-menu routines 	 */
/* create : 95/03/29				 	 */
/* update : 96/10/10				 	 */
/*-------------------------------------------------------*/


#define	_MAIN_C_


#include "bbs.h"
#include "dns.h"


#include <sys/wait.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/telnet.h>
#include <sys/resource.h>


#define	QLEN		3
#define	PID_FILE	"run/bbs.pid"
#define	LOG_FILE	"run/bbs.log"
#undef	SERVER_USAGE


static int myports[MAX_BBSDPORT] = BBSD_PORT;

static pid_t currpid;

extern BCACHE *bshm;
extern UCACHE *ushm;

/* static int mport; */ /* Thor.990325: 不需要了:P */
static u_long tn_addr;


#ifdef CHAT_SECURE
char passbuf[PSWDLEN + 1];
#endif


#ifdef MODE_STAT
extern UMODELOG modelog;
extern time_t mode_lastchange;
#endif


/* ----------------------------------------------------- */
/* 離開 BBS 程式					 */
/* ----------------------------------------------------- */


void
alog(mode, msg)		/* Admin 行為記錄 */
  char *mode, *msg;
{
  char buf[512];

  sprintf(buf, "%s %s %-13s%s\n", Now(), mode, cuser.userid, msg);
  f_cat(FN_RUN_ADMIN, buf);
}


void
blog(mode, msg)		/* BBS 一般記錄 */
  char *mode, *msg;
{
  char buf[512];

  sprintf(buf, "%s %s %-13s%s\n", Now(), mode, cuser.userid, msg);
  f_cat(FN_RUN_USIES, buf);
}


#ifdef MODE_STAT
void
log_modes()
{
  /* time(&modelog.logtime); */
  time_t now;
  time(&now);
  modelog.logtime = (time32_t)now;
  rec_add(FN_RUN_MODE_CUR, &modelog, sizeof(UMODELOG));
}
#endif


void
u_exit(mode)
  char *mode;
{
  int fd, diff;
  char fpath[80];
  ACCT tuser;

  mantime_add(currbno, -1);	/* 退出最後看的那個板 */

  utmp_free(cutmp);		/* 釋放 UTMP shm */

  /* diff = (time(&cuser.lastlogin) - ap_start) / 60; */
  time_t now;
  time(&now);
  cuser.lastlogin = (time32_t)now;
  diff = (now - ap_start) / 60;
  sprintf(fpath, "Stay: %d (%d)", diff, currpid);
  blog(mode, fpath);

  if (cuser.userlevel)
  {
    ve_backup();		/* 編輯器自動備份 */
    brh_save();			/* 儲存閱讀記錄檔 */
  }

#ifndef LOG_BMW	/* 離站刪除水球 */
  usr_fpath(fpath, cuser.userid, fn_amw);
  unlink(fpath);
  usr_fpath(fpath, cuser.userid, fn_bmw);
  unlink(fpath);
#endif

#ifdef MODE_STAT
  log_modes();
#endif


  /* 寫回 .ACCT */

  if (!HAS_STATUS(STATUS_DATALOCK))	/* itoc.010811: 沒有被站長鎖定，才可以回存 .ACCT */
  {
    usr_fpath(fpath, cuser.userid, fn_acct);
    fd = open(fpath, O_RDWR);
    if (fd >= 0)
    {
      if (read(fd, &tuser, sizeof(ACCT)) == sizeof(ACCT))
      {
	if (diff >= 1)
	{
	  cuser.numlogins++;	/* Thor.980727.註解: 在站上未超過一分鐘不予計算次數 */
	  addmoney(diff);	/* itoc.010805: 上站一分鐘加一元 */
	}

	if (HAS_STATUS(STATUS_COINLOCK))	/* itoc.010831: 若是 multi-login 的第二隻以後，不儲存錢幣 */
	{
	  cuser.money = tuser.money;
	  cuser.gold = tuser.gold;
	}

	/* itoc.010811.註解: 如果使用者在線上沒有認證的話，
	  那麼 cuser 及 tuser 的 userlevel/tvalid 是同步的；
	  但若使用者在線上回認證信/填認證碼/被站長審核註冊單..等認證通過的話，
	  那麼 tuser 的 userlevel/tvalid 才是比較新的 */
	cuser.userlevel = tuser.userlevel;
	cuser.tvalid = tuser.tvalid;

	lseek(fd, (off_t) 0, SEEK_SET);
	write(fd, &cuser, sizeof(ACCT));
      }
      close(fd);
    }
  }
}


void
abort_bbs()
{
  if (bbstate)
    u_exit("AXXED");
  exit(0);
}


static void
login_abort(msg)
  char *msg;
{
  outs(msg);
  refresh();
  exit(0);
}


/* Thor.980903: lkchu patch: 不使用上站申請帳號時, 則下列 function均不用 */

#ifdef LOGINASNEW

/* ----------------------------------------------------- */
/* 檢查 user 註冊情況					 */
/* ----------------------------------------------------- */


static int
belong(flist, key)
  char *flist;
  char *key;
{
  int fd, rc;

  rc = 0;
  if ((fd = open(flist, O_RDONLY)) >= 0)
  {
    mgets(-1);

    while (flist = mgets(fd))
    {
      str_lower(flist, flist);
      if (str_str(key, flist))
      {
	rc = 1;
	break;
      }
    }

    close(fd);
  }
  return rc;
}


static int
is_badid(userid)
  char *userid;
{
  int ch;
  char *str;

  if (strlen(userid) < 2)
    return 1;

  if (!is_alpha(*userid))
    return 1;

  if (!str_cmp(userid, STR_NEW))
    return 1;

  str = userid;
  while (ch = *(++str))
  {
    if (!is_alnum(ch))
      return 1;
  }
  return (belong(FN_ETC_BADID, userid));
}


#if 0
static int
uniq_userno(fd)			/* 找 .USR 前面空的 userno */
  int fd;
{
  char buf[4096];
  int userno, size;
  SCHEMA *sp;			/* record length 16 可整除 4096 */

  userno = 1;

  while ((size = read(fd, buf, sizeof(buf))) > 0)
  {
    sp = (SCHEMA *) buf;
    do
    {
      if (sp->userid[0] == '\0')
      {
	lseek(fd, -size, SEEK_CUR);
	return userno;
      }
      userno++;
      size -= sizeof(SCHEMA);
      sp++;
    } while (size);
  }

  return userno;
}
#endif


static int
uniq_userno(fd)
  int fd;
{
  struct stat st;

  fstat(fd, &st);
  lseek(fd, 0, SEEK_END);
  return (st.st_size / sizeof(SCHEMA)) + 1;
}


static void
acct_apply()
{
  SCHEMA slot;
  char buf[80];
  char *userid;
  int try, fd;

  film_out(FILM_APPLY, 0);

  memset(&cuser, 0, sizeof(ACCT));
  userid = cuser.userid;
  try = 0;
  for (;;)
  {
    if (!vget(18, 0, msg_uid, userid, IDLEN + 1, DOECHO))
      /* \n再見 ... */
      login_abort("\n\xA6\x41\xA8\xA3 ...");

    if (is_badid(userid))
    {
      /* 無法接受這個代號，請使用英文字母，並且不要包含空格 */
      vmsg("\xB5\x4C\xAA\x6B\xB1\xB5\xA8\xFC\xB3\x6F\xAD\xD3\xA5\x4E\xB8\xB9\xA1\x41\xBD\xD0\xA8\xCF\xA5\xCE\xAD\x5E\xA4\xE5\xA6\x72\xA5\xC0\xA1\x41\xA8\xC3\xA5\x42\xA4\xA3\xAD\x6E\xA5\x5D\xA7\x74\xAA\xC5\xAE\xE6");
    }
    else
    {
      usr_fpath(buf, userid, NULL);
      if (dashd(buf))
	/* 此代號已經有人使用 */
	vmsg("\xA6\xB9\xA5\x4E\xB8\xB9\xA4\x77\xB8\x67\xA6\xB3\xA4\x48\xA8\xCF\xA5\xCE");
      else
	break;
    }

    if (++try >= 10)
      /* \n您嘗試錯誤的輸入太多，請下次再來吧 */
      login_abort("\n\xB1\x7A\xB9\xC1\xB8\xD5\xBF\xF9\xBB\x7E\xAA\xBA\xBF\xE9\xA4\x4A\xA4\xD3\xA6\x68\xA1\x41\xBD\xD0\xA4\x55\xA6\xB8\xA6\x41\xA8\xD3\xA7\x61");
  }

  for (;;)
  {
    /* 請設定密碼： */
    vget(19, 0, "\xBD\xD0\xB3\x5D\xA9\x77\xB1\x4B\xBD\x58\xA1\x47", buf, PSWDLEN + 1, NOECHO);
    if ((strlen(buf) < 4) || !strcmp(buf, userid))
    {
      /* 密碼太簡單，易遭入侵，至少要 4 個字，請重新輸入 */
      vmsg("\xB1\x4B\xBD\x58\xA4\xD3\xC2\xB2\xB3\xE6\xA1\x41\xA9\xF6\xBE\x44\xA4\x4A\xAB\x49\xA1\x41\xA6\xDC\xA4\xD6\xAD\x6E 4 \xAD\xD3\xA6\x72\xA1\x41\xBD\xD0\xAD\xAB\xB7\x73\xBF\xE9\xA4\x4A");
      continue;
    }

    /* 請檢查密碼： */
    vget(20, 0, "\xBD\xD0\xC0\xCB\xAC\x64\xB1\x4B\xBD\x58\xA1\x47", buf + PSWDLEN + 2, PSWDLEN + 1, NOECHO);
    if (!strcmp(buf, buf + PSWDLEN + 2))
      break;

    /* 密碼輸入錯誤, 請重新輸入密碼 */
    vmsg("\xB1\x4B\xBD\x58\xBF\xE9\xA4\x4A\xBF\xF9\xBB\x7E, \xBD\xD0\xAD\xAB\xB7\x73\xBF\xE9\xA4\x4A\xB1\x4B\xBD\x58");
  }

  str_ncpy(cuser.passwd, genpasswd(buf), sizeof(cuser.passwd));

  do
  {
    /* 暱    稱： */
    vget(20, 0, "\xBC\xCA    \xBA\xD9\xA1\x47", cuser.username, UNLEN + 1, DOECHO);
  } while (strlen(cuser.username) < 2);

  /* itoc.010317: 提示 user 以後將不能改姓名 */
  /* 注意：請輸入真實姓名，本站不提供修改姓名的功能 */
  vmsg("\xAA\x60\xB7\x4E\xA1\x47\xBD\xD0\xBF\xE9\xA4\x4A\xAF\x75\xB9\xEA\xA9\x6D\xA6\x57\xA1\x41\xA5\xBB\xAF\xB8\xA4\xA3\xB4\xA3\xA8\xD1\xAD\xD7\xA7\xEF\xA9\x6D\xA6\x57\xAA\xBA\xA5\x5C\xAF\xE0");

  do
  {
    /* 真實姓名： */
    vget(21, 0, "\xAF\x75\xB9\xEA\xA9\x6D\xA6\x57\xA1\x47", cuser.realname, RNLEN + 1, DOECHO);
  } while (strlen(cuser.realname) < 4);

  cuser.userlevel = PERM_DEFAULT;
  cuser.ufo = UFO_DEFAULT_NEW;
  cuser.numlogins = 1;
  cuser.tvalid = ap_start;		/* itoc.030724: 拿上站時間當第一次認證碼的 seed */
  sprintf(cuser.email, "%s.bbs@%s", cuser.userid, str_host);	/* itoc.010902: 預設 email */

  /* Ragnarok.050528: 可能二人同時申請同一個 ID，在此必須再檢查一次 */
  usr_fpath(buf, userid, NULL);
  if (dashd(buf))
  {
    /* 此代號剛被註冊走，請重新申請 */
    vmsg("\xA6\xB9\xA5\x4E\xB8\xB9\xAD\xE8\xB3\x51\xB5\xF9\xA5\x55\xA8\xAB\xA1\x41\xBD\xD0\xAD\xAB\xB7\x73\xA5\xD3\xBD\xD0");
    abort_bbs();
  }

  /* dispatch unique userno */

  cuser.firstlogin = cuser.lastlogin = cuser.tcheck = slot.uptime = ap_start;
  memcpy(slot.userid, userid, IDLEN);

  fd = open(FN_SCHEMA, O_RDWR | O_CREAT, 0600);
  {
    /* flock(fd, LOCK_EX); */
    /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
    f_exlock(fd);

    cuser.userno = try = uniq_userno(fd);
    write(fd, &slot, sizeof(slot));
    /* flock(fd, LOCK_UN); */
    /* Thor.981205: 用 fcntl 取代flock, POSIX標準用法 */
    f_unlock(fd);
  }
  close(fd);

  /* create directory */

  /* usr_fpath(buf, userid, NULL); */	/* 剛做過 */
  mkdir(buf, 0700);
  strcat(buf, "/@");
  mkdir(buf, 0700);
  usr_fpath(buf, userid, "gem");	/* itoc.010727: 個人精華區 */
  /* mak_dirs(buf); */
  mak_links(buf);			/* itoc.010924: 減少個人精華區目錄 */
#ifdef MY_FAVORITE
  usr_fpath(buf, userid, "MF");
  mkdir(buf, 0700);
#endif

  usr_fpath(buf, userid, fn_acct);
  fd = open(buf, O_WRONLY | O_CREAT, 0600);
  write(fd, &cuser, sizeof(ACCT));
  close(fd);
  /* Thor.990416: 注意: 怎麼會有 .ACCT長度是0的, 而且只有 @目錄, 持續觀察中 */

  sprintf(buf, "%d", try);
  blog("APPLY", buf);
}

#endif /* LOGINASNEW */


/* ----------------------------------------------------- */
/* bad login						 */
/* ----------------------------------------------------- */


#define	FN_BADLOGIN	"logins.bad"


static void
logattempt(type, content)
  int type;			/* '-' login failure   ' ' success */
  char *content;
{
  char buf[128], fpath[64];

  sprintf(buf, "%s %c %s\n", Btime(ap_start), type, content);

  usr_fpath(fpath, cuser.userid, FN_LOG);
  f_cat(fpath, buf);

  if (type != ' ')
  {
    usr_fpath(fpath, cuser.userid, FN_BADLOGIN);
    sprintf(buf, "[%s] %s\n", Btime(ap_start), fromhost);
    f_cat(fpath, buf);
  }
}


/* ----------------------------------------------------- */
/* 登錄 BBS 程式					 */
/* ----------------------------------------------------- */


extern void talk_rqst();
extern void bmw_rqst();


#ifdef HAVE_WHERE
static int		/* 1:在list中 0:不在list中 */
belong_list(filelist, key, desc)
  char *filelist, *key, *desc;
{
  FILE *fp;
  char buf[80], *str;
  int rc;

  rc = 0;
  if (fp = fopen(filelist, "r"))
  {
    while (fgets(buf, sizeof(buf), fp))
    {
      if (buf[0] == '#')
	continue;

      if (str = (char *) strchr(buf, ' '))
      {
	*str = '\0';
	if (strstr(key, buf))
	{
	  /* 跳過空白分隔 */
	  for (str++; *str && isspace(*str); str++)
	    ;

	  strcpy(desc, str);
	  if (str = (char *) strchr(desc, '\n'))	/* 最後的 '\n' 不要 */
	    *str = '\0';
	  rc = 1;
	  break;
	}
      }
    }
    fclose(fp);
  }
  return rc;
}
#endif


static void
utmp_setup(mode)
  int mode;
{
  UTMP utmp;
  uschar *addr;

  memset(&utmp, 0, sizeof(utmp));

  utmp.pid = currpid;
  utmp.userno = cuser.userno;
  utmp.mode = bbsmode = mode;
  /* utmp.in_addr = tn_addr; */ /* itoc.010112: 改變umtp.in_addr以使ulist_cmp_host正常 */
  addr = (uschar *) &tn_addr;
  utmp.in_addr = (addr[0] << 24) + (addr[1] << 16) + (addr[2] << 8) + addr[3];
  utmp.userlevel = cuser.userlevel;	/* itoc.010309: 把 userlevel 也放入 cache */
  utmp.ufo = cuser.ufo;
  utmp.status = 0;

  strcpy(utmp.userid, cuser.userid);
#ifdef DETAIL_IDLETIME
  utmp.idle_time = ap_start;
#endif

#ifdef GUEST_NICK
  if (!cuser.userlevel)		/* guest */
  {
    /* 遊子 */
    /* 水滴 */
    /* 訪客 */
    /* 補帖 */
    /* 豬頭 */
    /* 影子 */
    /* 病毒 */
    /* 童年 */
    /* 石像 */
    char nick[9][5] = {"\xB9\x43\xA4\x6C", "\xA4\xF4\xBA\x77", "\xB3\x58\xAB\xC8", "\xB8\xC9\xA9\xAB", "\xBD\xDE\xC0\x59", "\xBC\x76\xA4\x6C", "\xAF\x66\xAC\x72", "\xB5\xA3\xA6\x7E", "\xA5\xDB\xB9\xB3"};
    /* 太陽下的%s */
    sprintf(cuser.username, "\xA4\xD3\xB6\xA7\xA4\x55\xAA\xBA%s", nick[ap_start % 9]);
  }
#endif	/* GUEST_NICK */

  strcpy(utmp.username, cuser.username);

#ifdef HAVE_WHERE

#  ifdef GUEST_WHERE
  if (!cuser.userlevel)		/* guest */
  {
    /* itoc.010910: GUEST_NICK 和 GUEST_WHERE 的亂數模數避免一樣 */
    /* 新竹站 */
    /* 新貨站 */
    /* 竹中站 */
    /* 上員站 */
    /* 榮華站 */
    /* 竹東站 */
    /* 橫山站 */
    /* 九讚頭站 */
    char from[14][9] = {"\xB7\x73\xA6\xCB\xAF\xB8", "\xB7\x73\xB3\x66\xAF\xB8", "\xA6\xCB\xA4\xA4\xAF\xB8", "\xA4\x57\xAD\xFB\xAF\xB8", "\xBA\x61\xB5\xD8\xAF\xB8", "\xA6\xCB\xAA\x46\xAF\xB8", "\xBE\xEE\xA4\x73\xAF\xB8", "\xA4\x45\xC6\x67\xC0\x59\xAF\xB8",
			/* 合興站 */
			/* 富貴站 */
			/* 內灣站 */
			/* 千甲站 */
			/* 關東站 */
			/* 六家站 */
			"\xA6\x58\xBF\xB3\xAF\xB8", "\xB4\x49\xB6\x51\xAF\xB8", "\xA4\xBA\xC6\x57\xAF\xB8", "\xA4\x64\xA5\xD2\xAF\xB8", "\xC3\xF6\xAA\x46\xAF\xB8", "\xA4\xBB\xAE\x61\xAF\xB8",};
    strcpy(utmp.from, from[ap_start % 14]);
  }
  else
#  endif	/* GUEST_WHERE */
  {

  /* 像 hinet 這種 ip 很多， DN 很少的，就寫入 etc/fqdn    *
   * 像 140.112. 這種就寫在 etc/host                       *
   * 即使 DNS 爛掉，在 etc/host 裡面的還是可以照樣判斷成功 *
   * 如果把 140.112. 寫入 etc/host 中，就不用把 ntu.edu.tw *
   * 重覆寫入 etc/fqdn 裡了                                */

    char name[48];

    /* 先比對 FQDN */
    str_lower(name, fromhost);	/* itoc.011011: 大小寫均可，etc/fqdn 裡面都要寫小寫 */
    if (!belong_list(FN_ETC_FQDN, name, utmp.from))
    {
      /* 再比對 ip */
      sprintf(name, "%d.%d.%d.%d", addr[0], addr[1], addr[2], addr[3]);
      if (!belong_list(FN_ETC_HOST, name, utmp.from))
	str_ncpy(utmp.from, fromhost, sizeof(utmp.from)); /* 如果都沒找到對應故鄉，就是用 fromhost */
    }
  }

#else
  str_ncpy(utmp.from, fromhost, sizeof(utmp.from));
#endif	/* HAVE_WHERE */

  /* Thor: 告訴User已經滿了放不下... */
  if (!utmp_new(&utmp))
    /* \n您剛剛選的位子已經被人捷足先登了，請下次再來吧 */
    login_abort("\n\xB1\x7A\xAD\xE8\xAD\xE8\xBF\xEF\xAA\xBA\xA6\xEC\xA4\x6C\xA4\x77\xB8\x67\xB3\x51\xA4\x48\xB1\xB6\xA8\xAC\xA5\xFD\xB5\x6E\xA4\x46\xA1\x41\xBD\xD0\xA4\x55\xA6\xB8\xA6\x41\xA8\xD3\xA7\x61");

  /* itoc.001223: utmp_new 完再 pal_cache，如果 login_abort 就不做了 */
  pal_cache();
}


/* ----------------------------------------------------- */
/* user login						 */
/* ----------------------------------------------------- */


static int		/* 回傳 multi */
login_user(content)
  char *content;
{
  int attempts;		/* 嘗試幾次錯誤 */
  int multi;
  char fpath[64], uid[IDLEN + 1];
#ifndef CHAT_SECURE
  char passbuf[PSWDLEN + 1];
#endif

  move(b_lines, 0);
  /*    ※ 參觀帳號：\033[1;32m */
  /* \033[m  申請新帳號：\033[1;31m */
  outs("   \xA1\xB0 \xB0\xD1\xC6\x5B\xB1\x62\xB8\xB9\xA1\x47\033[1;32m" STR_GUEST "\033[m  \xA5\xD3\xBD\xD0\xB7\x73\xB1\x62\xB8\xB9\xA1\x47\033[1;31m" STR_NEW "\033[m");

  attempts = 0;
  multi = 0;
  for (;;)
  {
    if (++attempts > LOGINATTEMPTS)
    {
      film_out(FILM_TRYOUT, 0);
      /* \n再見 ... */
      login_abort("\n\xA6\x41\xA8\xA3 ...");
    }

    /*    [您的帳號]  */
    vget(b_lines - 2, 0, "   [\xB1\x7A\xAA\xBA\xB1\x62\xB8\xB9] ", uid, IDLEN + 1, DOECHO);

    if (!str_cmp(uid, STR_NEW))
    {
#ifdef LOGINASNEW
#  ifdef HAVE_GUARANTOR		/* itoc.000319: 保證人制度 */
      /*    [您的保人]  */
      vget(b_lines - 2, 0, "   [\xB1\x7A\xAA\xBA\xAB\x4F\xA4\x48] ", uid, IDLEN + 1, DOECHO);
      if (!*uid || (acct_load(&cuser, uid) < 0))
      {
	/* 抱歉，沒有介紹人不得加入本站 */
	vmsg("\xA9\xEA\xBA\x70\xA1\x41\xA8\x53\xA6\xB3\xA4\xB6\xB2\xD0\xA4\x48\xA4\xA3\xB1\x6F\xA5\x5B\xA4\x4A\xA5\xBB\xAF\xB8");
      }
      else if (!HAS_PERM(PERM_GUARANTOR))
      {
	/* 抱歉，您不夠資格擔任別人的介紹人 */
	vmsg("\xA9\xEA\xBA\x70\xA1\x41\xB1\x7A\xA4\xA3\xB0\xF7\xB8\xEA\xAE\xE6\xBE\xE1\xA5\xF4\xA7\x4F\xA4\x48\xAA\xBA\xA4\xB6\xB2\xD0\xA4\x48");
      }
      /* [保人密碼]  */
      else if (!vget(b_lines - 2, 40, "[\xAB\x4F\xA4\x48\xB1\x4B\xBD\x58] ", passbuf, PSWDLEN + 1, NOECHO))
      {
	continue;
      }
      else
      {
	if (chkpasswd(cuser.passwd, passbuf))
	{
	  logattempt('-', content);
	  vmsg(ERR_PASSWD);
	}
	else
	{
	  FILE *fp;
	  char parentid[IDLEN + 1], buf[80];

	  strcpy(parentid, cuser.userid);

	  acct_apply();
	  logattempt(' ', content);

	  /* itoc.010820: 記錄保人於保證人及被保人 */
	  /* itoc.010820.註解: 把對方 log 在行首，在 reaper 時可以方便砍 tree */
	  /* %s 於 %s 介紹此人(%s)加入本站\n */
	  sprintf(buf, "%s \xA9\xF3 %s \xA4\xB6\xB2\xD0\xA6\xB9\xA4\x48(%s)\xA5\x5B\xA4\x4A\xA5\xBB\xAF\xB8\n", parentid, Btime(ap_start), cuser.userid);
	  usr_fpath(fpath, cuser.userid, "guarantor");
	  if (fp = fopen(fpath, "a"))
	  {
	    fputs(buf, fp);
	    fclose(fp);
	  }
	  /* %s 於 %s 被此人(%s)介紹加入本站\n */
	  sprintf(buf, "%s \xA9\xF3 %s \xB3\x51\xA6\xB9\xA4\x48(%s)\xA4\xB6\xB2\xD0\xA5\x5B\xA4\x4A\xA5\xBB\xAF\xB8\n", cuser.userid, Btime(ap_start), parentid);
	  usr_fpath(fpath, parentid, "guarantor");
	  if (fp = fopen(fpath, "a"))
	  {
	    fputs(buf, fp);
	    fclose(fp);
	  }

	  break;
	}
      }
#  else
      acct_apply(); /* Thor.980917.註解: cuser setup ok */
      logattempt(' ', content);
      break;
#  endif
#else
      /* \n本系統目前暫停線上註冊, 請用  */
      /*  進入 */
      outs("\n\xA5\xBB\xA8\x74\xB2\xCE\xA5\xD8\xAB\x65\xBC\xC8\xB0\xB1\xBD\x75\xA4\x57\xB5\xF9\xA5\x55, \xBD\xD0\xA5\xCE " STR_GUEST " \xB6\x69\xA4\x4A");
      continue;
#endif
    }
    else if (!*uid)
    {
      /* 若沒輸入 ID，那麼 continue */
    }
    else if (str_cmp(uid, STR_GUEST))	/* 一般使用者 */
    {
      /* [您的密碼]  */
      if (!vget(b_lines - 2, 40, "[\xB1\x7A\xAA\xBA\xB1\x4B\xBD\x58] ", passbuf, PSWDLEN + 1, NOECHO))
	continue;	/* 不打密碼則取消登入 */

      /* itoc.040110: 在輸入完 ID 及密碼，才載入 .ACCT */
      if (acct_load(&cuser, uid) < 0)
      {
	vmsg(err_uid);
	continue;
      }

      if (chkpasswd(cuser.passwd, passbuf))
      {
	logattempt('-', content);
	vmsg(ERR_PASSWD);
      }
      else
      {
	if (!str_cmp(cuser.userid, str_sysop))
	{
#ifdef SYSOP_SU
	  /* 簡單的 SU 功能 */
	  /* 變更使用者身分(Y/N)？[N]  */
	  if (vans("\xC5\xDC\xA7\xF3\xA8\xCF\xA5\xCE\xAA\xCC\xA8\xAD\xA4\xC0(Y/N)\xA1\x48[N] ") == 'y')
	  {
	    for (;;)
	    {
	      /*    [變更帳號]  */
	      if (vget(b_lines - 2, 0, "   [\xC5\xDC\xA7\xF3\xB1\x62\xB8\xB9] ", uid, IDLEN + 1, DOECHO) &&
		acct_load(&cuser, uid) >= 0)
		break;
	      vmsg(err_uid);
	    }
	  }
	  else
#endif
	  {
	    /* SYSOP gets all permission bits */
	    /* itoc.010902: DENY perm 排外 */
	    cuser.userlevel = ~0 ^ (PERM_DENYMAIL | PERM_DENYTALK | PERM_DENYCHAT | PERM_DENYPOST | PERM_DENYLOGIN | PERM_PURGE);
	  }
	}

	if (cuser.ufo & UFO_ACL)
	{
	  usr_fpath(fpath, cuser.userid, FN_ACL);
	  str_lower(fromhost, fromhost);	/* lkchu.981201: 換小寫 */
	  if (!acl_has(fpath, "", fromhost))
	  {	/* Thor.980728: 注意 acl 檔中要全部小寫 */
	    logattempt('-', content);
	    /* \n您的上站地點不太對勁，請核對 [上站地點設定檔] */
	    login_abort("\n\xB1\x7A\xAA\xBA\xA4\x57\xAF\xB8\xA6\x61\xC2\x49\xA4\xA3\xA4\xD3\xB9\xEF\xAB\x6C\xA1\x41\xBD\xD0\xAE\xD6\xB9\xEF [\xA4\x57\xAF\xB8\xA6\x61\xC2\x49\xB3\x5D\xA9\x77\xC0\xC9]");
	  }
	}

	logattempt(' ', content);

	/* check for multi-session */

	if (!HAS_PERM(PERM_ALLADMIN))
	{
	  UTMP *ui;
	  pid_t pid;

	  if (HAS_PERM(PERM_DENYLOGIN | PERM_PURGE))
	    /* \n這個帳號暫停服務，詳情請向站長洽詢。 */
	    login_abort("\n\xB3\x6F\xAD\xD3\xB1\x62\xB8\xB9\xBC\xC8\xB0\xB1\xAA\x41\xB0\xC8\xA1\x41\xB8\xD4\xB1\xA1\xBD\xD0\xA6\x56\xAF\xB8\xAA\xF8\xAC\xA2\xB8\xDF\xA1\x43");


	  if (!(ui = (UTMP *) utmp_find(cuser.userno)))
	    break;		/* user isn't logged in */

	  pid = ui->pid;
	  /* 您想踢掉其他重複的 login (Y/N)嗎？[Y]  */
	  if (pid && vans("\xB1\x7A\xB7\x51\xBD\xF0\xB1\xBC\xA8\xE4\xA5\x4C\xAD\xAB\xBD\xC6\xAA\xBA login (Y/N)\xB6\xDC\xA1\x48[Y] ") != 'n' && pid == ui->pid)
	  {
	    if ((kill(pid, SIGTERM) == -1) && (errno == ESRCH))
	      utmp_free(ui);
	    else
	      sleep(3);			/* 被踢的人這時候正在自我了斷 */
	    blog("MULTI", cuser.userid);
	  }

	  if ((multi = utmp_count(cuser.userno, 0)) >= MULTI_MAX || 	/* 線上已有 MULTI_MAX 隻自己，禁止登入 */
	    (!multi && acct_load(&cuser, uid) < 0))			/* yiting.050101: 若剛已踢掉所有 multi-login，那麼重新讀取以套用變更 */
	    /* \n再見 ... */
	    login_abort("\n\xA6\x41\xA8\xA3 ...");
	}
	break;
      }
    }
    else
    {				/* guest */
      if (acct_load(&cuser, uid) < 0)
      {
	vmsg(err_uid);
	continue;
      }
      logattempt(' ', content);
      cuser.userlevel = 0;	/* Thor.981207: 怕人亂玩, 強制寫回cuser.userlevel */
      cuser.ufo = UFO_DEFAULT_GUEST;
      break;	/* Thor.980917.註解: cuser setup ok */
    }
  }

  return multi;
}


static void
login_level()
{
  int fd;
  usint level;
  ACCT tuser;
  char fpath[64];

  /* itoc.010804.註解: 有 PERM_VALID 者自動發給 PERM_POST PERM_PAGE PERM_CHAT */
  level = cuser.userlevel | (PERM_ALLVALID ^ PERM_VALID);

  if (!(level & PERM_ALLADMIN))
  {
#ifdef JUSTIFY_PERIODICAL
    if ((level & PERM_VALID) && (cuser.tvalid + VALID_PERIOD < ap_start))
    {
      level ^= PERM_VALID;
      /* itoc.011116: 主動發信通知使用者，一直送信不知道會不會太耗空間 !? */
      /* 您的認證已經過期，請重新認證 */
      mail_self(FN_ETC_REREG, str_sysop, "\xB1\x7A\xAA\xBA\xBB\x7B\xC3\xD2\xA4\x77\xB8\x67\xB9\x4C\xB4\xC1\xA1\x41\xBD\xD0\xAD\xAB\xB7\x73\xBB\x7B\xC3\xD2", 0);
    }
#endif

#ifdef NEWUSER_LIMIT
    /* 即使已經通過認證，還是要見習三天 */
    if (ap_start - cuser.firstlogin < 3 * 86400)
      level &= ~PERM_POST;
#endif

    /* itoc.000520: 未經身分認證, 禁止 post/chat/talk/write */
    if (!(level & PERM_VALID))
      level &= ~(PERM_POST | PERM_CHAT | PERM_PAGE);

    if (level & PERM_DENYPOST)
      level &= ~PERM_POST;

    if (level & PERM_DENYTALK)
      level &= ~PERM_PAGE;

    if (level & PERM_DENYCHAT)
      level &= ~PERM_CHAT;

    if ((cuser.numemails >> 4) > (cuser.numlogins + cuser.numposts))
      level |= PERM_DENYMAIL;
  }

  cuser.userlevel = level;

  usr_fpath(fpath, cuser.userid, fn_acct);
  if ((fd = open(fpath, O_RDWR)) >= 0)
  {
    if (read(fd, &tuser, sizeof(ACCT)) == sizeof(ACCT))
    {
      /* itoc.010805.註解: 這次的寫回 .ACCT 是為了讓別人 Query 線上使用者時
	 出現的上站時間/來源正確，以及回存正確的 userlvel */
      tuser.userlevel = level;
      tuser.lastlogin = ap_start;
      strcpy(tuser.lasthost, cuser.lasthost);

      lseek(fd, (off_t) 0, SEEK_SET);
      write(fd, &tuser, sizeof(ACCT));
    }
    close(fd);
  }
}


static void
login_status(multi)
  int multi;
{
  usint status;
  char fpath[64];
  struct tm *ptime;

  status = 0;

  /* itoc.010831: multi-login 的第二隻加上不可變動錢幣的旗標 */
  if (multi)
    status |= STATUS_COINLOCK;

  /* itoc.011022: 加入生日旗標 */
  ptime = localtime(&ap_start);
  if (cuser.day == ptime->tm_mday && cuser.month == ptime->tm_mon + 1)
    status |= STATUS_BIRTHDAY;

  /* 朋友名單同步、清理過期信件 */
  if (ap_start > cuser.tcheck + CHECK_PERIOD)
  {
    outz(MSG_CHKDATA);
    refresh();

    cuser.tcheck = ap_start;
    usr_fpath(fpath, cuser.userid, fn_pal);
    pal_sync(fpath);
#ifdef HAVE_ALOHA
    usr_fpath(fpath, cuser.userid, FN_FRIENZ);
    frienz_sync(fpath);
#endif
#ifdef OVERDUE_MAILDEL
    status |= m_quota();		/* Thor.註解: 資料整理稽核有包含 BIFF check */
#endif
  }
#ifdef OVERDUE_MAILDEL
  else
#endif
    status |= m_query(cuser.userid);

  /* itoc.010924: 檢查個人精華區是否過多 */
#ifndef LINUX	/* 在 Linux 下這檢查怪怪的 */
  {
    struct stat st;
    usr_fpath(fpath, cuser.userid, "gem");
    if (!stat(fpath, &st) && (st.st_size >= 512 * 7))
      status |= STATUS_MGEMOVER;
  }
#endif

  cutmp->status |= status;
}


static void
login_other()
{
  usint status;
  char fpath[64];

  /* 刪除錯誤登入記錄 */
  usr_fpath(fpath, cuser.userid, FN_BADLOGIN);
  /* 以上為輸入密碼錯誤時的上站地點記錄，要刪除嗎(Y/N)？[Y]  */
  if (more(fpath, (char *) -1) >= 0 && vans("\xA5\x48\xA4\x57\xAC\xB0\xBF\xE9\xA4\x4A\xB1\x4B\xBD\x58\xBF\xF9\xBB\x7E\xAE\xC9\xAA\xBA\xA4\x57\xAF\xB8\xA6\x61\xC2\x49\xB0\x4F\xBF\xFD\xA1\x41\xAD\x6E\xA7\x52\xB0\xA3\xB6\xDC(Y/N)\xA1\x48[Y] ") != 'n')
    unlink(fpath);

  if (!HAS_PERM(PERM_VALID))
    film_out(FILM_NOTIFY, -1);		/* 尚未認證通知 */
#ifdef JUSTIFY_PERIODICAL
  else if (!HAS_PERM(PERM_ALLADMIN) && (cuser.tvalid + VALID_PERIOD - INVALID_NOTICE_PERIOD < ap_start))
    film_out(FILM_REREG, -1);		/* 有效時間逾期 10 天前提出警告 */
#endif

#ifdef NEWUSER_LIMIT
  if (ap_start - cuser.firstlogin < 3 * 86400)
    film_out(FILM_NEWUSER, -1);		/* 即使已經通過認證，還是要見習三天 */
#endif

  status = cutmp->status;

#ifdef OVERDUE_MAILDEL
  if (status & STATUS_MQUOTA)
    film_out(FILM_MQUOTA, -1);		/* 過期信件即將清除警告 */
#endif

  if (status & STATUS_MAILOVER)
    film_out(FILM_MAILOVER, -1);	/* 信件過多或寄信過多 */

  if (status & STATUS_MGEMOVER)
    film_out(FILM_MGEMOVER, -1);	/* itoc.010924: 個人精華區過多警告 */

  if (status & STATUS_BIRTHDAY)
    film_out(FILM_BIRTHDAY, -1);	/* itoc.010415: 生日當天上站有 special 歡迎畫面 */

  ve_recover();				/* 上次斷線，編輯器回存 */
}


static void
tn_login()
{
  int multi;
  char buf[128];

  bbsmode = M_LOGIN;	/* itoc.020828: 以免過久未輸入時 igetch 會出現 movie */

  /* --------------------------------------------------- */
  /* 登錄系統						 */
  /* --------------------------------------------------- */

  /* Thor.990415: 記錄ip, 怕正查不到 */
  sprintf(buf, "%s ip:%08x (%d)", fromhost, tn_addr, currpid);

  multi = login_user(buf);

  blog("ENTER", buf);

  /* --------------------------------------------------- */
  /* 初始化 utmp、flag、mode、信箱			 */
  /* --------------------------------------------------- */

  bbstate = STAT_STARTED;	/* 進入系統以後才可以回水球 */
  utmp_setup(M_LOGIN);		/* Thor.980917.註解: cutmp, cutmp-> setup ok */
  total_user = ushm->count;	/* itoc.011027: 未進使用者名單前，啟始化 total_user */

  mbox_main();

#ifdef MODE_STAT
  memset(&modelog, 0, sizeof(UMODELOG));
  mode_lastchange = ap_start;
#endif

  if (cuser.userlevel)		/* not guest */
  {
    /* ------------------------------------------------- */
    /* 核對 user level 並將 .ACCT 寫回			 */
    /* ------------------------------------------------- */

    /* itoc.030929: 在 .ACCT 寫回以前，不可以有任何 vmsg(NULL) 或 more(xxxx, NULL)
       等的東西，這樣如果 user 在 vmsg(NULL) 時回認證信，才不會被寫回的 cuser 蓋過 */

    cuser.lastlogin = ap_start;
    str_ncpy(cuser.lasthost, fromhost, sizeof(cuser.lasthost));

    login_level();

    /* ------------------------------------------------- */
    /* 設定 status					 */
    /* ------------------------------------------------- */

    login_status(multi);

    /* ------------------------------------------------- */
    /* 秀些資訊						 */
    /* ------------------------------------------------- */

    login_other();
  }

  srand(ap_start * cuser.userno * currpid);
}


static void
tn_motd()
{
  usint ufo;
  char ans[4];

  ufo = cuser.ufo;

  if (!(ufo & UFO_MOTD))
  {
    more("gem/@/@-day", NULL);	/* 今日熱門話題 */
    pad_view();
  }

#ifdef HAVE_NOALOHA
  if (!(ufo & UFO_NOALOHA))
#endif
  {
#ifdef LOGIN_NOTIFY
    loginNotify();
#endif
#ifdef HAVE_ALOHA
    aloha();
#endif
  }

#ifdef HAVE_FORCE_BOARD
  brd_force();	/* itoc.000319: 強制閱讀公告板 */
#endif

 while (cuser.year < 1 || cuser.year > 99)
 {
   /* 出生年份： */
   vget(22, 0, "\xA5\x58\xA5\xCD\xA6\x7E\xA5\xF7\xA1\x47", ans, 3, DOECHO);
   cuser.year = (ans[0] - '0') * 10 + (ans[1] - '0');
 }

 while (cuser.month < 1 || cuser.month > 12)
 {
   /* 出生月份： */
   vget(22, 0, "\xA5\x58\xA5\xCD\xA4\xEB\xA5\xF7\xA1\x47", ans, 3, DOECHO);
   cuser.month = (ans[0] - '0') * 10 + (ans[1] - '0');
 }

 while (cuser.day < 1 || cuser.day > 31)
 {
   /* 出生日期： */
   vget(22, 0, "\xA5\x58\xA5\xCD\xA4\xE9\xB4\xC1\xA1\x47", ans, 3, DOECHO);
   cuser.day = (ans[0] - '0') * 10 + (ans[1] - '0');
 }

 while (cuser.sex != 0  && cuser.sex != 1 && cuser.sex != 2)
 {
   /* 性別 (0)中性 (1)男性 (2)女性： */
   cuser.sex = vget(22, 0, "\xA9\xCA\xA7\x4F (0)\xA4\xA4\xA9\xCA (1)\xA8\x6B\xA9\xCA (2)\xA4\x6B\xA9\xCA\xA1\x47", ans, 2, DOECHO) - '0';
 }

}


/* ----------------------------------------------------- */
/* trap signals						 */
/* ----------------------------------------------------- */


static void
tn_signals()
{
  struct sigaction act;

  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;

  act.sa_handler = (void *) abort_bbs;
  sigaction(SIGBUS, &act, NULL);
  sigaction(SIGSEGV, &act, NULL);
  sigaction(SIGTERM, &act, NULL);
  sigaction(SIGXCPU, &act, NULL);
#ifdef SIGSYS
  /* Thor.981221: easy for porting */
  sigaction(SIGSYS, &act, NULL);/* bad argument to system call */
#endif

  act.sa_handler = (void *) talk_rqst;
  sigaction(SIGUSR1, &act, NULL);

  act.sa_handler = (void *) bmw_rqst;
  sigaction(SIGUSR2, &act, NULL);

  /* 在此借用 sigset_t act.sa_mask */
  sigaddset(&act.sa_mask, SIGPIPE);
  sigprocmask(SIG_BLOCK, &act.sa_mask, NULL);

}


static inline void
tn_main()
{
  clear();

#ifdef HAVE_LOGIN_DENIED
  if (acl_has(BBS_ACLFILE, "", fromhost))
    /* \n貴機器於不被敝站接受 */
    login_abort("\n\xB6\x51\xBE\xF7\xBE\xB9\xA9\xF3\xA4\xA3\xB3\x51\xB1\xCD\xAF\xB8\xB1\xB5\xA8\xFC");
#endif

  time(&ap_start);

  /* %s ⊙  */
  /*  ⊙  */
  prints("%s \xA1\xF3 " SCHOOLNAME " \xA1\xF3 " MYIPADDR "\n"
    /* 歡迎光臨【\033[1;33;46m %s \033[m】目前線上人數 [%d] 人 */
    "\xC5\x77\xAA\xEF\xA5\xFA\xC1\x7B\xA1\x69\033[1;33;46m %s \033[m\xA1\x6A\xA5\xD8\xAB\x65\xBD\x75\xA4\x57\xA4\x48\xBC\xC6 [%d] \xA4\x48",
    str_host, str_site, ushm->count);

  film_out((ap_start % 3) + FILM_OPENING0, 3);	/* 亂數顯示開頭畫面 */

  currpid = getpid();

  tn_signals();	/* Thor.980806: 放於 tn_login前, 以便 call in不會被踢 */
  tn_login();

  board_main();
  gem_main();
#ifdef MY_FAVORITE
  mf_main();
#endif
  talk_main();

  tn_motd();

  menu();
  abort_bbs();	/* to make sure it will terminate */
}


/* ----------------------------------------------------- */
/* FSA (finite state automata) for telnet protocol	 */
/* ----------------------------------------------------- */


static void
telnet_init()
{
  static char svr[] =
  {
    IAC, DO, TELOPT_TTYPE,
    IAC, SB, TELOPT_TTYPE, TELQUAL_SEND, IAC, SE,
    IAC, WILL, TELOPT_ECHO,
    IAC, WILL, TELOPT_SGA
  };

  int n, len;
  char *cmd;
  int rset;
  struct timeval to;
  char buf[64];

  /* --------------------------------------------------- */
  /* init telnet protocol				 */
  /* --------------------------------------------------- */

  cmd = svr;

  for (n = 0; n < 4; n++)
  {
    len = (n == 1 ? 6 : 3);
    send(0, cmd, len, 0);
    cmd += len;

    rset = 1;
    /* Thor.981221: for future reservation bug */
    to.tv_sec = 1;
    to.tv_usec = 1;
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(0, &readfds);
    if (select(1, &readfds, NULL, NULL, &to) > 0)
      recv(0, buf, sizeof(buf), 0);
  }
}


/* ----------------------------------------------------- */
/* 支援超過 24 列的畫面					 */
/* ----------------------------------------------------- */


static void
term_init()
{
#if 0   /* fuse.030518: 註解 */
  server問：你會改變行列數嗎？(TN_NAWS, Negotiate About Window Size)
  client答：Yes, I do. (TNCH_DO)

  那麼在連線時，當TERM變化行列數時就會發出：
  TNCH_IAC + TNCH_SB + TN_NAWS + 行數列數 + TNCH_IAC + TNCH_SE;
#endif

  /* ask client to report it's term size */
  static char svr[] = 		/* server */
  {
    IAC, DO, TELOPT_NAWS
  };

  int rset;
  char buf[64], *rcv;
  struct timeval to;

  /* 問對方 (telnet client) 有沒有支援不同的螢幕寬高 */
  send(0, svr, 3, 0);

  rset = 1;
  to.tv_sec = 1;
  to.tv_usec = 1;
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(0, &readfds);
  if (select(1, &readfds, NULL, NULL, &to) > 0)
    recv(0, buf, sizeof(buf), 0);

  rcv = NULL;
  if ((uschar) buf[0] == IAC && buf[2] == TELOPT_NAWS)
  {
    /* gslin: Unix 的 telnet 對有無加 port 參數的行為不太一樣 */
    if ((uschar) buf[1] == SB)
    {
      rcv = buf + 3;
    }
    else if ((uschar) buf[1] == WILL)
    {
      if ((uschar) buf[3] != IAC)
      {
	rset = 1;
	to.tv_sec = 1;
	to.tv_usec = 1;
        FD_ZERO(&readfds);
        FD_SET(0, &readfds);
	if (select(1, &readfds, NULL, NULL, &to) > 0)
	  recv(0, buf + 3, sizeof(buf) - 3, 0);
      }
      if ((uschar) buf[3] == IAC && (uschar) buf[4] == SB && buf[5] == TELOPT_NAWS)
	rcv = buf + 6;
    }
  }

  if (rcv)
  {
    b_lines = ntohs(* (short *) (rcv + 2)) - 1;
    b_cols = ntohs(* (short *) rcv) - 1;

    /* b_lines 至少要 23，最多不能超過 T_LINES - 1 */
    if (b_lines >= T_LINES)
      b_lines = T_LINES - 1;
    else if (b_lines < 23)
      b_lines = 23;
    /* b_cols 至少要 79，最多不能超過 T_COLS - 1 */
    if (b_cols >= T_COLS)
      b_cols = T_COLS - 1;
    else if (b_cols < 79)
      b_cols = 79;
  }
  else
  {
    b_lines = 23;
    b_cols = 79;
  }

  d_cols = b_cols - 79;
}


/* ----------------------------------------------------- */
/* stand-alone daemon					 */
/* ----------------------------------------------------- */


static void
start_daemon(port)
  int port; /* Thor.981206: 取 0 代表 *沒有參數* , -1 代表 -i (inetd) */
{
  int n;
  struct linger ld;
  struct sockaddr_in sin;
#ifdef HAVE_RLIMIT
  struct rlimit limit;
#endif
  char buf[80], data[80];
  time_t val;

  /*
   * More idiot speed-hacking --- the first time conversion makes the C
   * library open the files containing the locale definition and time zone.
   * If this hasn't happened in the parent process, it happens in the
   * children, once per connection --- and it does add up.
   */

  time(&val);
  strftime(buf, 80, "%d/%b/%Y %H:%M:%S", localtime(&val));

#ifdef HAVE_RLIMIT
  /* --------------------------------------------------- */
  /* adjust resource : 16 mega is enough		 */
  /* --------------------------------------------------- */

  limit.rlim_cur = limit.rlim_max = 16 * 1024 * 1024;
  /* setrlimit(RLIMIT_FSIZE, &limit); */
  setrlimit(RLIMIT_DATA, &limit);

#ifdef SOLARIS
#define RLIMIT_RSS RLIMIT_AS	/* Thor.981206: port for solaris 2.6 */
#endif

  setrlimit(RLIMIT_RSS, &limit);

  limit.rlim_cur = limit.rlim_max = 0;
  setrlimit(RLIMIT_CORE, &limit);

  limit.rlim_cur = limit.rlim_max = 60 * 20;
  setrlimit(RLIMIT_CPU, &limit);
#endif

  /* --------------------------------------------------- */
  /* speed-hacking DNS resolve				 */
  /* --------------------------------------------------- */

  dns_init();

  /* --------------------------------------------------- */
  /* change directory to bbshome       			 */
  /* --------------------------------------------------- */

  chdir(BBSHOME);
  umask(077);

  /* --------------------------------------------------- */
  /* detach daemon process				 */
  /* --------------------------------------------------- */

  /* The integer file descriptors associated with the streams
     stdin, stdout, and stderr are 0,1, and 2, respectively. */

  close(1);
  close(2);

  if (port == -1) /* Thor.981206: inetd -i */
  {
    /* Give up root privileges: no way back from here	 */
    setgid(BBSGID);
    setuid(BBSUID);
#if 1
    n = sizeof(sin);
    if (getsockname(0, (struct sockaddr *) &sin, &n) >= 0)
      port = ntohs(sin.sin_port);
#endif
    /* mport = port; */ /* Thor.990325: 不需要了:P */

    sprintf(data, "%d\t%s\t%d\tinetd -i\n", getpid(), buf, port);
    f_cat(PID_FILE, data);
    return;
  }

  close(0);

  if (fork())
    exit(0);

  setsid();

  if (fork())
    exit(0);

  /* --------------------------------------------------- */
  /* fork daemon process				 */
  /* --------------------------------------------------- */

  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;

  if (port == 0) /* Thor.981206: port 0 代表沒有參數 */
  {
    n = MAX_BBSDPORT - 1;
    while (n)
    {
      if (fork() == 0)
	break;

      sleep(1);
      n--;
    }
    port = myports[n];
  }

  n = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  val = 1;
  setsockopt(n, SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof(val));

  ld.l_onoff = ld.l_linger = 0;
  setsockopt(n, SOL_SOCKET, SO_LINGER, (char *) &ld, sizeof(ld));

  /* mport = port; */ /* Thor.990325: 不需要了:P */
  sin.sin_port = htons(port);
  if ((bind(n, (struct sockaddr *) &sin, sizeof(sin)) < 0) || (listen(n, QLEN) < 0))
    exit(1);

  /* --------------------------------------------------- */
  /* Give up root privileges: no way back from here	 */
  /* --------------------------------------------------- */

  setgid(BBSGID);
  setuid(BBSUID);

  /* standalone */
  sprintf(data, "%d\t%s\t%d\n", getpid(), buf, port);
  f_cat(PID_FILE, data);
}


/* ----------------------------------------------------- */
/* reaper - clean up zombie children			 */
/* ----------------------------------------------------- */


static inline void
reaper()
{
  while (waitpid(-1, NULL, WNOHANG | WUNTRACED) > 0);
}


#ifdef	SERVER_USAGE
static void
servo_usage()
{
  struct rusage ru;
  FILE *fp;

  fp = fopen("run/bbs.usage", "a");

  if (!getrusage(RUSAGE_CHILDREN, &ru))
  {
    fprintf(fp, "\n[Server Usage] %d: %d\n\n"
      "user time: %.6f\n"
      "system time: %.6f\n"
      "maximum resident set size: %lu P\n"
      "integral resident set size: %lu\n"
      "page faults not requiring physical I/O: %d\n"
      "page faults requiring physical I/O: %d\n"
      "swaps: %d\n"
      "block input operations: %d\n"
      "block output operations: %d\n"
      "messages sent: %d\n"
      "messages received: %d\n"
      "signals received: %d\n"
      "voluntary context switches: %d\n"
      "involuntary context switches: %d\n\n",

      getpid(), ap_start,
      (double) ru.ru_utime.tv_sec + (double) ru.ru_utime.tv_usec / 1000000.0,
      (double) ru.ru_stime.tv_sec + (double) ru.ru_stime.tv_usec / 1000000.0,
      ru.ru_maxrss,
      ru.ru_idrss,
      ru.ru_minflt,
      ru.ru_majflt,
      ru.ru_nswap,
      ru.ru_inblock,
      ru.ru_oublock,
      ru.ru_msgsnd,
      ru.ru_msgrcv,
      ru.ru_nsignals,
      ru.ru_nvcsw,
      ru.ru_nivcsw);
  }

  fclose(fp);
}
#endif


static void
main_term()
{
#ifdef	SERVER_USAGE
  servo_usage();
#endif
  exit(0);
}


static inline void
main_signals()
{
  struct sigaction act;

  /* act.sa_mask = 0; */ /* Thor.981105: 標準用法 */
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;

  act.sa_handler = reaper;
  sigaction(SIGCHLD, &act, NULL);

  act.sa_handler = main_term;
  sigaction(SIGTERM, &act, NULL);

#ifdef	SERVER_USAGE
  act.sa_handler = servo_usage;
  sigaction(SIGPROF, &act, NULL);
#endif

  /* sigblock(sigmask(SIGPIPE)); */
}


int
main(argc, argv)
  int argc;
  char *argv[];
{
  int csock;			/* socket for Master and Child */
  int value;
  int *totaluser;
  struct sockaddr_in sin;

  /* --------------------------------------------------- */
  /* setup standalone daemon				 */
  /* --------------------------------------------------- */

  /* Thor.990325: usage, bbsd, or bbsd -i, or bbsd 1234 */
  /* Thor.981206: 取 0 代表 *沒有參數*, -1 代表 -i */
  start_daemon(argc > 1 ? strcmp("-i", argv[1]) ? atoi(argv[1]) : -1 : 0);

  main_signals();

  /* --------------------------------------------------- */
  /* attach shared memory & semaphore			 */
  /* --------------------------------------------------- */

#ifdef HAVE_SEM
  sem_init();
#endif
  ushm_init();
  bshm_init();
  fshm_init();

  /* --------------------------------------------------- */
  /* main loop						 */
  /* --------------------------------------------------- */

  totaluser = &ushm->count;
  /* avgload = &ushm->avgload; */

  for (;;)
  {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(0, &readfds);
    if (select(1, &readfds, NULL, NULL, NULL) < 0)
      continue;

    value = sizeof(sin);
    csock = accept(0, (struct sockaddr *) &sin, &value);
    if (csock < 0)
    {
      reaper();
      continue;
    }

    ap_start++;
    argc = *totaluser;
    if (argc >= MAXACTIVE - 5 /* || *avgload > THRESHOLD */ )
    {
      /* 借用 currtitle */
      /* 目前線上人數 [%d] 人，系統飽和，請稍後再來\n */
      sprintf(currtitle, "\xA5\xD8\xAB\x65\xBD\x75\xA4\x57\xA4\x48\xBC\xC6 [%d] \xA4\x48\xA1\x41\xA8\x74\xB2\xCE\xB9\xA1\xA9\x4D\xA1\x41\xBD\xD0\xB5\x79\xAB\xE1\xA6\x41\xA8\xD3\n", argc);
      send(csock, currtitle, strlen(currtitle), 0);
      close(csock);
      continue;
    }

    if (fork())
    {
      close(csock);
      continue;
    }

    dup2(csock, 0);
    close(csock);

    /* ------------------------------------------------- */
    /* ident remote host / user name via RFC931		 */
    /* ------------------------------------------------- */

    tn_addr = sin.sin_addr.s_addr;
    dns_name((char *) &sin.sin_addr, fromhost);
    /* str_ncpy(fromhost, (char *)inet_ntoa(sin.sin_addr), sizeof(fromhost)); */

    telnet_init();
    term_init();
    tn_main();
  }
}
