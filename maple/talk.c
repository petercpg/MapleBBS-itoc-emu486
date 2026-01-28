/*-------------------------------------------------------*/
/* maple/talk.c         ( NTHU CS MapleBBS Ver 3.00 )    */
/*-------------------------------------------------------*/
/* target : talk/query routines		 		 */
/* create : 95/03/29				 	 */
/* update : 97/03/29				 	 */
/*-------------------------------------------------------*/


#define	_MODES_C_


#include "bbs.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


extern UCACHE *ushm;


typedef struct
{
  int curcol, curln;
  int sline, eline;
#ifdef HAVE_MULTI_BYTE
  int zhc;
#endif
}      talk_win;


/* ------------------------------------- */
/* 真實動作				 */
/* ------------------------------------- */


char *
bmode(up, simple)
  UTMP *up;
  int simple;
{
  static char modestr[32];
  int mode;
  char *word, *mateid;

  word = ModeTypeTable[mode = up->mode];

  if (simple)
    return word;

#ifdef BMW_COUNT
  if (simple = up->bmw_count)	/* 借用 simple */
  {
    /* %d 個水球 */
    sprintf(modestr, "%d \xAD\xD3\xA4\xF4\xB2\x79", simple);
    return modestr;
  }
#endif

#ifdef HAVE_BRDMATE
  /* itoc.020602: 站長得知使用者在看哪個板 */
  if (mode == M_READA && HAS_PERM(PERM_SYSOP))
  {
    /* 閱:%s */
    sprintf(modestr, "\xBE\x5C:%s", up->reading);
    return modestr;
  }
#endif

  if (mode < M_TALK || mode > M_IDLE)	/* M_TALK(含) 與 M_IDLE(含) 間接 mateid */
    return word;

  mateid = up->mateid;

  if (mode == M_TALK)
  {
    /* itoc.020829: up 在 Talk 時，若 up->mateid 隱形則看不見 */
    if (!utmp_get(0, mateid))
      /* 無名氏 */
      mateid = "\xB5\x4C\xA6\x57\xA4\xF3";
  }

  sprintf(modestr, "%s:%s", word, mateid);
  return modestr;
}


static void
showplans(userid)
  char *userid;
{
  int i;
  FILE *fp;
  char buf[ANSILINELEN];

  usr_fpath(buf, userid, fn_plans);
  if (fp = fopen(buf, "r"))
  {
    i = MAXQUERYLINES;
    while (i-- && fgets(buf, sizeof(buf), fp))
      outx(buf);
    fclose(fp);
  }
}


static void
do_query(acct)
  ACCT *acct;
{
  UTMP *up;
  int userno, rich;
  char *userid;
  /* 赤貧乞丐 */
  /* 一般個體 */
  /* 家境小康 */
  /* 財閥地主 */
  char fortune[4][9] = {"\xA8\xAA\xB3\x68\xA4\x5E\xA4\xA2", "\xA4\x40\xAF\xEB\xAD\xD3\xC5\xE9", "\xAE\x61\xB9\xD2\xA4\x70\xB1\x64", "\xB0\x5D\xBB\xD6\xA6\x61\xA5\x44"};

  utmp_mode(M_QUERY);

  userno = acct->userno;
  userid = acct->userid;
  strcpy(cutmp->mateid, userid);

  up = utmp_find(userno);
  rich = acct->money >= 1000000 ? (acct->gold >= 100 ? 3 : 2) : (acct->money >= 50000 ? 1 : 0);

  /* [帳號] %-12s [暱稱] %-16.16s [上站] %5d 次 [文章] %5d 篇\n */
  prints("[\xB1\x62\xB8\xB9] %-12s [\xBC\xCA\xBA\xD9] %-16.16s [\xA4\x57\xAF\xB8] %5d \xA6\xB8 [\xA4\xE5\xB3\xB9] %5d \xBD\x67\n",
    userid, acct->username, acct->numlogins, acct->numposts);

  /* [認證] %s通過認證 [動態] %-16.16s [財產] %s [信箱] %s\n */
  prints("[\xBB\x7B\xC3\xD2] %s\xB3\x71\xB9\x4C\xBB\x7B\xC3\xD2 [\xB0\xCA\xBA\x41] %-16.16s [\xB0\x5D\xB2\xA3] %s [\xAB\x48\xBD\x63] %s\n",
    /* 已經 */
    /* 尚未 */
    acct->userlevel & PERM_VALID ? "\xA4\x77\xB8\x67" : "\xA9\x7C\xA5\xBC",
    /* 不在站上 */
    (up && can_see(cutmp, up)) ? bmode(up, 1) : "\xA4\xA3\xA6\x62\xAF\xB8\xA4\x57",
    fortune[rich],
    /* 有新信件 */
    /* 都看過了 */
    (m_query(userid) & STATUS_BIFF) ? "\xA6\xB3\xB7\x73\xAB\x48\xA5\xF3" : "\xB3\xA3\xAC\xDD\xB9\x4C\xA4\x46");

  /* [來源] (%s) %s\n */
  prints("[\xA8\xD3\xB7\xBD] (%s) %s\n",
    Btime(acct->lastlogin), acct->lasthost);

  showplans(userid);
  vmsg(NULL);
}


void
my_query(userid)
  char *userid;
{
  ACCT acct;

  if (acct_load(&acct, userid) >= 0)
    do_query(&acct);
  else
    vmsg(err_uid);
}


#ifdef HAVE_ALOHA
static int
chkfrienz(frienz)
  FRIENZ *frienz;
{
  int userno;

  userno = frienz->userno;
  return (userno > 0 && userno == acct_userno(frienz->userid));
}


static int
frienz_cmp(a, b)
  FRIENZ *a, *b;
{
  return a->userno - b->userno;
}


void
frienz_sync(fpath)
  char *fpath;
{
  rec_sync(fpath, sizeof(FRIENZ), frienz_cmp, chkfrienz);
}


void
aloha()
{
  UTMP *up;
  int fd;
  char fpath[64];
  BMW bmw;
  FRIENZ *frienz;
  int userno;

  usr_fpath(fpath, cuser.userid, FN_FRIENZ);

  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    bmw.caller = cutmp;
    /* bmw.sender = cuser.userno; */	/* bmw.sender 依對方可以 call 我，待會再決定 */
    strcpy(bmw.userid, cuser.userid);
    /* ◎ 剛剛踏進 */
    /* 的門 [上站通知] ◎ */
    strcpy(bmw.msg, "\xA1\xB7 \xAD\xE8\xAD\xE8\xBD\xF1\xB6\x69"BBSNAME"\xAA\xBA\xAA\xF9 [\xA4\x57\xAF\xB8\xB3\x71\xAA\xBE] \xA1\xB7");

    mgets(-1);
    while (frienz = mread(fd, sizeof(FRIENZ)))
    {
      userno = frienz->userno;
      up = utmp_find(userno);

      if (up && (up->ufo & UFO_ALOHA) && !(up->status & STATUS_REJECT) && can_see(up, cutmp))	/* 對方看不見我不通知 */
      {
	/* 好友且自己沒有遠離塵囂才可以 reply */
	bmw.sender = (is_mygood(userno) && !(cuser.ufo & UFO_QUIET)) ? cuser.userno : 0;
	bmw.recver = userno;
	bmw_send(up, &bmw);
      }
    }
    close(fd);
  }
}
#endif


#ifdef LOGIN_NOTIFY
extern LinkList *ll_head;


int
t_loginNotify()
{
  LinkList *wp;
  BENZ benz;
  char fpath[64];

  /* 設定 list 的名單 */

  /* 系統協尋網友 */
  vs_bar("\xA8\x74\xB2\xCE\xA8\xF3\xB4\x4D\xBA\xF4\xA4\xCD");

  ll_new();

  if (pal_list(0))
  {
    wp = ll_head;
    benz.userno = cuser.userno;
    strcpy(benz.userid, cuser.userid);

    do
    {
      if (strcmp(cuser.userid, wp->data))	/* 不可協尋自己 */
      {
	usr_fpath(fpath, wp->data, FN_BENZ);
	rec_add(fpath, &benz, sizeof(BENZ));
      }
    } while (wp = wp->next);

    /* 協尋設定完成，對方上站時系統會通知您 */
    vmsg("\xA8\xF3\xB4\x4D\xB3\x5D\xA9\x77\xA7\xB9\xA6\xA8\xA1\x41\xB9\xEF\xA4\xE8\xA4\x57\xAF\xB8\xAE\xC9\xA8\x74\xB2\xCE\xB7\x7C\xB3\x71\xAA\xBE\xB1\x7A");
  }
  return 0;
}


void
loginNotify()
{
  UTMP *up;
  int fd;
  char fpath[64];
  BMW bmw;
  BENZ *benz;
  int userno;
  int row, col;		/* 計算印到哪 */

  usr_fpath(fpath, cuser.userid, FN_BENZ);

  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    /* 系統協尋網友 */
    vs_bar("\xA8\x74\xB2\xCE\xA8\xF3\xB4\x4D\xBA\xF4\xA4\xCD");

    bmw.caller = cutmp;
    /* bmw.sender = cuser.userno; */	/* bmw.sender 依對方可以 call 我，待會再決定 */
    strcpy(bmw.userid, cuser.userid);
    /* ◎ 剛剛踏進 */
    /* 的門 [系統協尋] ◎ */
    strcpy(bmw.msg, "\xA1\xB7 \xAD\xE8\xAD\xE8\xBD\xF1\xB6\x69"BBSNAME"\xAA\xBA\xAA\xF9 [\xA8\x74\xB2\xCE\xA8\xF3\xB4\x4D] \xA1\xB7");

    row = 1;
    col = 0;
    mgets(-1);
    while (benz = mread(fd, sizeof(BENZ)))
    {
      /* 印出哪些人在找我 */
      if (row < b_lines)
      {
	move(row, col);
	outs(benz->userid);
	col += IDLEN + 1;
	if (col > b_cols + 1 - IDLEN - 1)	/* 總共可以放 (b_cols + 1) / (IDLEN + 1) 欄 */
	{
	  row++;
	  col = 0;
	}
      }

      userno = benz->userno;
      up = utmp_find(userno);

      if (up && !(up->status & STATUS_REJECT) && can_see(up, cutmp))	/* 對方看不見我不通知 */
      {
	/* 好友且自己沒有遠離塵囂才可以 reply */
	bmw.sender = (is_mygood(userno) && !(cuser.ufo & UFO_QUIET)) ? cuser.userno : 0;
	bmw.recver = userno;
	bmw_send(up, &bmw);
	outc('*');	/* Thor.980707: 有通知到的有所不同 */
      }
    }
    close(fd);
    unlink(fpath);
    /* 這些使用者設您為上站協尋，打 * 表示目前在站上 */
    vmsg("\xB3\x6F\xA8\xC7\xA8\xCF\xA5\xCE\xAA\xCC\xB3\x5D\xB1\x7A\xAC\xB0\xA4\x57\xAF\xB8\xA8\xF3\xB4\x4D\xA1\x41\xA5\xB4 * \xAA\xED\xA5\xDC\xA5\xD8\xAB\x65\xA6\x62\xAF\xB8\xA4\x57");
  }
}
#endif


#ifdef LOG_TALK
static void
talk_save()
{
  char fpath[64];
  
  /* lkchu.981201: 放進私人信箱內/清除 */
  usr_fpath(fpath, cuser.userid, FN_TALK_LOG);

  /* 本次聊天紀錄處理 (M)備忘錄 (C)清除？[M]  */
  if (!(cuser.ufo & UFO_NTLOG) && vans("\xA5\xBB\xA6\xB8\xB2\xE1\xA4\xD1\xAC\xF6\xBF\xFD\xB3\x42\xB2\x7A (M)\xB3\xC6\xA7\xD1\xBF\xFD (C)\xB2\x4D\xB0\xA3\xA1\x48[M] ") != 'c')
    /* [備 忘 錄] 聊天紀錄 */
    mail_self(fpath, cuser.userid, "[\xB3\xC6 \xA7\xD1 \xBF\xFD] \xB2\xE1\xA4\xD1\xAC\xF6\xBF\xFD", MAIL_READ | MAIL_NOREPLY);

  unlink(fpath);
}
#endif


/* ----------------------------------------------------- */
/* talk sub-routines					 */
/* ----------------------------------------------------- */


static char page_requestor[40];
#ifdef HAVE_MULTI_BYTE
static int page_requestor_zhc;
#endif

/* 每列可以輸入的字數為 SCR_WIDTH */
#ifdef HAVE_MULTI_BYTE
static uschar talk_pic[T_LINES][SCR_WIDTH + 1];	/* 刪除列尾字時會用到後一碼的空白，所以要多一碼 */
#else
static uschar talk_pic[T_LINES][SCR_WIDTH + 2];	/* 刪除列尾中文字時會用到後二碼的空白，所以要多二碼 */
#endif
static int talk_len[T_LINES];			/* 每列目前已輸入多少字 */


static void
talk_clearline(ln, col)
  int ln, col;
{
  int i, len;

  len = talk_len[ln];
  for (i = col; i < len; i++)
    talk_pic[ln][i] = ' ';

  talk_len[ln] = col;
}


static void
talk_outs(str, len)
  uschar *str;
  int len;
{
  int ch;
  uschar *end;

  /* 和一般的 outs() 是相同，只是限制印 len 個字 */
  end = str + len;
  while (ch = *str)
  {
    outc(ch);
    if (++str >= end)
      break;
  }
}


static void
talk_nextline(twin)
  talk_win *twin;
{
  int curln, max, len, i;

  curln = twin->curln;
  if (curln != twin->eline)
  {
    twin->curln = ++curln;
  }
  else	/* 已經是最後一列，要向上捲動 */
  {
    max = twin->eline;
    for (curln = twin->sline; curln < max; curln++)
    {
      len = BMAX(talk_len[curln], talk_len[curln + 1]);
      for (i = 0; i < len; i++)
        talk_pic[curln][i] = talk_pic[curln + 1][i];
      talk_len[curln] = talk_len[curln + 1];
      move(curln, 0);
      talk_outs(talk_pic[curln], talk_len[curln]);
      clrtoeol();
    }
  }

  /* 新的一列 */
  talk_clearline(curln, 0);

  twin->curcol = 0;
  move(curln, 0);
  clrtoeol();
}


static void
talk_char(twin, ch)
  talk_win *twin;
  int ch;
{
  int col, ln, len, i;

  col = twin->curcol;
  ln = twin->curln;
  len = talk_len[ln];

  if (isprint2(ch))
  {
    if (col >= SCR_WIDTH)	/* 若已經打到列尾，先換列 */
    {
      talk_nextline(twin);
      col = twin->curcol;
      ln = twin->curln;
      len = talk_len[ln];
    }

    move(ln, col);
    if (col >= len)
    {
      talk_pic[ln][col] = ch;
      outc(ch);
      twin->curcol = ++col;
      talk_len[ln] = col;
    }
    else		/* 要 insert */
    {
      for (i = SCR_WIDTH - 1; i > col; i--)
	talk_pic[ln][i] = talk_pic[ln][i - 1];
      talk_pic[ln][col] = ch;
      if (len < SCR_WIDTH)
	len++;
      talk_len[ln] = len;
      talk_outs(talk_pic[ln] + col, len - col);
      twin->curcol = ++col;
      move(ln, col);
    }
  }
  else
  {
    switch (ch)
    {
    case '\n':
      talk_nextline(twin);
      break;

    case KEY_BKSP:		/* backspace */
      if (col > 0)
      {
	if (col > len)
	{
	  /* 做和 KEY_LEFT 一樣的事 */
	  twin->curcol = --col;
	  move(ln, col);
	}
	else
	{
	  col--;
#ifdef HAVE_MULTI_BYTE
	  /* hightman.060504: 判斷現在刪除的位置是否為漢字的後半段，若是刪二字元 */
	  if (twin->zhc && col && IS_ZHC_LO(talk_pic[ln], col))
	  {
	    col--;
	    ch = 2;
	  }
	  else
#endif
	    ch = 1;
	  for (i = col; i < SCR_WIDTH; i++)
	    talk_pic[ln][i] = talk_pic[ln][i + ch];
	  move(ln, col);
	  talk_outs(talk_pic[ln] + col, len - col);
	  twin->curcol = col;
	  talk_len[ln] = len - ch;
	  move(ln, col);
	}
      }
      break;

    case Ctrl('D'):		/* KEY_DEL */
      if (col < len)
      {
#ifdef HAVE_MULTI_BYTE
	/* hightman.060504: 判斷現在刪除的位置是否為漢字的前半段，若是刪二字元 */
	if (twin->zhc && col < len - 1 && IS_ZHC_HI(talk_pic[ln][col]))
	  ch = 2;
	else
#endif
	  ch = 1;
	for (i = col; i < SCR_WIDTH; i++)
	  talk_pic[ln][i] = talk_pic[ln][i + ch];
	move(ln, col);
	talk_outs(talk_pic[ln] + col, len - col);
	talk_len[ln] = len - ch;
	move(ln, col);
      }
      break;

    case Ctrl('B'):		/* KEY_LEFT */
      if (col > 0)
      {
	col--;
#ifdef HAVE_MULTI_BYTE
	/* hightman.060504: 左移時碰到漢字移雙格 */
	if (twin->zhc && col && IS_ZHC_LO(talk_pic[ln], col))
	  col--;
#endif
	twin->curcol = col;
	move(ln, col);
      }
      break;

    case Ctrl('F'):		/* KEY_RIGHT */
      if (col < SCR_WIDTH)
      {
	col++;
#ifdef HAVE_MULTI_BYTE
	/* hightman.060504: 右移時碰到漢字移雙格 */
	if (twin->zhc && col < SCR_WIDTH && IS_ZHC_HI(talk_pic[ln][col - 1]))
	  col++;
#endif
	twin->curcol = col;
	move(ln, col);
      }
      break;

    case Ctrl('P'):		/* KEY_UP */ 
      if (ln > twin->sline)
      {
	twin->curln = --ln;
#ifdef HAVE_MULTI_BYTE
	/* hightman.060504: 漢字整字調節 */
	if (twin->zhc && col < SCR_WIDTH && IS_ZHC_LO(talk_pic[ln], col))
	  col++;
#endif
	move(ln, col);
      }
      break;

    case Ctrl('N'):		/* KEY_DOWN */
      if (ln < twin->eline)
      {
	twin->curln = ++ln;
#ifdef HAVE_MULTI_BYTE
	/* hightman.060504: 漢字整字調節 */
	if (twin->zhc && col < SCR_WIDTH && IS_ZHC_LO(talk_pic[ln], col))
	  col++;
#endif
	move(ln, col);
      }
      break;

    case Ctrl('A'):		/* KEY_HOME */
      twin->curcol = 0;
      move(ln, 0);
      break;

    case Ctrl('E'):		/* KEY_END */
      twin->curcol = len;
      move(ln, len);
      break;
    
    case Ctrl('Y'):		/* clear this line */
      talk_clearline(ln, 0);
      twin->curcol = 0;
      move(ln, 0);
      clrtoeol();
      break;

    case Ctrl('K'):		/* clear to end of line */
      talk_clearline(ln, col);
      move(ln, col);
      clrtoeol();
      break;

    case Ctrl('G'):		/* bell */
      bell();
      break;
    }
  }
}


static void
talk_string(twin, str)
  talk_win *twin;
  uschar *str;
{
  int ch;

  while (ch = *str)
  {
    talk_char(twin, ch);
    str++;
  }
}


static void
talk_speak(fd)
  int fd;
{
  talk_win mywin, itswin;
  uschar data[80];
  char buf[80];
  int i, ch;
#ifdef  LOG_TALK
  char mywords[80], itswords[80], itsuserid[40];
  FILE *fp;

#if 0	/* talk log 的 algo */
  對話兩方分別為 mywin & itswin, 只要其一有 recv 的話就會送上對應的 win, 
  所以我們必須要先把自己打的字與對方打的字分開來.

  於是就先建兩個 spool, 分別將 mywin/itswin recv 的 char 往各自的 spool 
  裡丟, 目前設 spool 剛好是一列的大小, 所以只要是 spool 滿了, 或是碰到換
  行字元, 就把 spool 裡的資料寫回 log, 然後清掉 spool, 如此繼續 :)
#endif

  /* lkchu: make sure that's empty */
  mywords[0] = itswords[0] = '\0';
  
  strcpy(itsuserid, page_requestor);
  strtok(itsuserid, " (");
#endif

  utmp_mode(M_TALK);

  ch = 58 - strlen(page_requestor);

  /* %s【%s */
  sprintf(buf, "%s\xA1\x69%s", cuser.userid, cuser.username);

  i = ch - strlen(buf);
  if (i >= 0)
  {
    i = (i >> 1) + 1;
  }
  else
  {
    buf[ch] = '\0';
    i = 1;
  }
  memset(data, ' ', i);
  data[i] = '\0';

  memset(&mywin, 0, sizeof(mywin));
  memset(&itswin, 0, sizeof(itswin));

  i = b_lines >> 1;
  mywin.eline = i - 1;
#ifdef HAVE_MULTI_BYTE
  mywin.zhc = cuser.ufo & UFO_ZHC;
#endif
  itswin.curln = itswin.sline = i + 1;
  itswin.eline = b_lines - 1;
#ifdef HAVE_MULTI_BYTE
  itswin.zhc = page_requestor_zhc;
#endif

  clear();
  move(i, 0);
  /* \033[1;46;37m  談天說地  \033[45m%s%s】 ◆  %s%s\033[m */
  prints("\033[1;46;37m  \xBD\xCD\xA4\xD1\xBB\xA1\xA6\x61  \033[45m%s%s\xA1\x6A \xA1\xBB  %s%s\033[m",
    data, buf, page_requestor, data);
  outf(FOOTER_TALK);
  move(0, 0);

  /* talk_pic 記錄整個畫面的文字，初始值是空白 */
  memset(talk_pic, ' ', sizeof(talk_pic));
  /* talk_len 記錄整個畫面各列已經用了多少字 */
  memset(talk_len, 0, sizeof(talk_len));

#ifdef LOG_TALK				/* lkchu.981201: 聊天記錄 */
  usr_fpath(buf, cuser.userid, FN_TALK_LOG);
  if (fp = fopen(buf, "a+"))
  {
    /* 【 %s 與 %s 之聊天記錄 】\n */
    fprintf(fp, "\xA1\x69 %s \xBB\x50 %s \xA4\xA7\xB2\xE1\xA4\xD1\xB0\x4F\xBF\xFD \xA1\x6A\n", cuser.userid, page_requestor);
    /* 開始聊天時間 [%s]\n */
    fprintf(fp, "\xB6\x7D\xA9\x6C\xB2\xE1\xA4\xD1\xAE\xC9\xB6\xA1 [%s]\n", Now());	/* itoc.010108: 記錄開始聊天時間 */
  }
#endif

  add_io(fd, 60);

  for (;;)
  {
    ch = vkey();

#ifdef EVERY_Z
    /* Thor.980725: talk中, ctrl-z */
    if (ch == Ctrl('Z'))
    {
      char buf[IDLEN + 1];
      screenline slt[T_LINES];

      /* Thor.980731: 暫存 mateid, 因為出去時可能會用掉 mateid */
      strcpy(buf, cutmp->mateid);

      vio_save();	/* Thor.980727: 暫存 vio_fd */
      vs_save(slt);
      every_Z(0);
      vs_restore(slt);
      vio_restore();	/* Thor.980727: 還原 vio_fd */

      /* Thor.980731: 還原 mateid, 因為出去時可能會用掉 mateid */
      strcpy(cutmp->mateid, buf);
      continue;
    }
#endif

    if (ch == Ctrl('D') || ch == Ctrl('C'))
      break;

    if (ch == I_OTHERDATA)
    {
      ch = recv(fd, data, 80, 0);
      if (ch <= 0)
	break;

#ifdef HAVE_GAME
      if (data[0] == Ctrl('O'))
      { /* Thor.990219: 呼叫外掛棋盤 */
	if (DL_func("bin/bwboard.so:vaBWboard", fd, 1) == -2)
	  break;
	continue;
      }
#endif
      for (i = 0; i < ch; i++)
      {
	talk_char(&itswin, data[i]);

#ifdef	LOG_TALK		/* 對方說的話 */
	switch (data[i])
	{
	case '\n':
	  /* lkchu.981201: 有換列就把 itswords 印出清掉 */
	  if (itswords[0] != '\0')
  	  {
  	    /* \033[32m%s：%s\033[m\n */
  	    fprintf(fp, "\033[32m%s\xA1\x47%s\033[m\n", itsuserid, itswords);
	    itswords[0] = '\0';
	  }
	  break;

	case KEY_BKSP:	/* lkchu.981201: backspace */
	  itswords[strlen(itswords) - 1] = '\0';
	  break;

	default:
	  if (isprint2(data[i]))
	  {
	    if (strlen(itswords) < sizeof(itswords))
  	    {
  	      strncat(itswords, (char *)&data[i], 1);
	    }
	    else	/* lkchu.981201: itswords 裝滿了 */
	    {
  	      /* \033[32m%s：%s%c\033[m\n */
  	      fprintf(fp, "\033[32m%s\xA1\x47%s%c\033[m\n", itsuserid, itswords, data[i]);
	      itswords[0] = '\0';
	    }
	  }
	  break;
	}
#endif

      }
    }

#ifdef HAVE_GAME
    else if (ch == Ctrl('O'))
    { /* Thor.990219: 呼叫外掛棋盤 */
      data[0] = ch;
      if (send(fd, data, 1, 0) != 1)
	break;
      if (DL_func("bin/bwboard.so:vaBWboard", fd, 0) == -2)
	break;
    }
#endif

    else if (ch == Ctrl('T'))
    {
      if (cuser.userlevel)	/* guest 有可能被站長邀請 Talk */
      {
	cuser.ufo ^= UFO_PAGER;
	cutmp->ufo = cuser.ufo;
	/* ◆ 關閉呼叫器\n */
	/* ◆ 打開呼叫器\n */
	talk_string(&mywin, (cuser.ufo & UFO_PAGER) ? "\xA1\xBB \xC3\xF6\xB3\xAC\xA9\x49\xA5\x73\xBE\xB9\n" : "\xA1\xBB \xA5\xB4\xB6\x7D\xA9\x49\xA5\x73\xBE\xB9\n");
      }
    }

    else
    {
      switch (ch)
      {
      case KEY_DEL:
	ch = Ctrl('D');
	break;

      case KEY_LEFT:
	ch = Ctrl('B');
	break;

      case KEY_RIGHT:
	ch = Ctrl('F');
	break;

      case KEY_UP:
	ch = Ctrl('P');
	break;

      case KEY_DOWN:
	ch = Ctrl('N');
	break;

      case KEY_HOME:
	ch = Ctrl('A');
	break;

      case KEY_END:
	ch = Ctrl('E');
	break;
      }

      data[0] = ch;
      if (send(fd, data, 1, 0) != 1)
	break;

      talk_char(&mywin, ch);

#ifdef LOG_TALK			/* 自己說的話 */
      switch (ch)
      {
      case '\n':
	if (mywords[0] != '\0')
	{
	  /* %s：%s\n */
	  fprintf(fp, "%s\xA1\x47%s\n", cuser.userid, mywords);
	  mywords[0] = '\0';
	}
	break;
      
      case KEY_BKSP:
	mywords[strlen(mywords) - 1] = '\0';
	break;

      default:
	if (isprint2(ch))
	{
	  if (strlen(mywords) < sizeof(mywords))
	  {
	    strncat(mywords, (char *)&ch, 1);
	  }
	  else
	  {
	    /* %s：%s%c\n */
	    fprintf(fp, "%s\xA1\x47%s%c\n", cuser.userid, mywords, ch);
	    mywords[0] = '\0';
	  }
	}
	break;
      }
#endif

#ifdef EVERY_BIFF 
      /* Thor.980805: 有人在旁邊按enter才需要check biff */ 
      if (ch == '\n')
      {
	static int old_biff; 
	int biff = HAS_STATUS(STATUS_BIFF);
	if (biff && !old_biff) 
	  /* ◆ 噹！郵差來按鈴了！\n */
	  talk_string(&mywin, "\xA1\xBB \xBE\xB4\xA1\x49\xB6\x6C\xAE\x74\xA8\xD3\xAB\xF6\xB9\x61\xA4\x46\xA1\x49\n");
	old_biff = biff; 
      }
#endif
    }
  }

#ifdef LOG_TALK
  /* itoc.021205: 最後一句話若沒有按 ENTER 就不會被記錄進 talk.log，
     在此特別處理，順序定為先 myword 再 itsword，但可能會相反 */
  if (mywords[0] != '\0')
    /* %s：%s\n */
    fprintf(fp, "%s\xA1\x47%s\n", cuser.userid, mywords);
  if (itswords[0] != '\0')
    /* \033[32m%s：%s\033[m\n */
    fprintf(fp, "\033[32m%s\xA1\x47%s\033[m\n", itsuserid, itswords);

  fclose(fp);
#endif

  add_io(0, 60);
}


static void
talk_hangup(sock)
  int sock;
{
  cutmp->sockport = 0;
  add_io(0, 60);
  close(sock);
}


static char *talk_reason[] =
{
  /* 對不起，我有事情不能跟您 talk */
  "\xB9\xEF\xA4\xA3\xB0\x5F\xA1\x41\xA7\xDA\xA6\xB3\xA8\xC6\xB1\xA1\xA4\xA3\xAF\xE0\xB8\xF2\xB1\x7A talk",
  /* 我現在很忙，請等一會兒再 call 我 */
  "\xA7\xDA\xB2\x7B\xA6\x62\xAB\xDC\xA6\xA3\xA1\x41\xBD\xD0\xB5\xA5\xA4\x40\xB7\x7C\xA8\xE0\xA6\x41 call \xA7\xDA",
  /* 現在忙不過來，等一下我會主動 page 您 */
  "\xB2\x7B\xA6\x62\xA6\xA3\xA4\xA3\xB9\x4C\xA8\xD3\xA1\x41\xB5\xA5\xA4\x40\xA4\x55\xA7\xDA\xB7\x7C\xA5\x44\xB0\xCA page \xB1\x7A",
  /* 我現在不想 talk 啦 */
  "\xA7\xDA\xB2\x7B\xA6\x62\xA4\xA3\xB7\x51 talk \xB0\xD5",
  /* 很煩咧，我實在不想 talk */
  "\xAB\xDC\xB7\xD0\xAB\xA8\xA1\x41\xA7\xDA\xB9\xEA\xA6\x62\xA4\xA3\xB7\x51 talk",

#ifdef EVERY_Z
  /* 我的嘴巴正忙著和別人講話呢，沒有空的嘴巴了 */
  "\xA7\xDA\xAA\xBA\xBC\x4C\xA4\xDA\xA5\xBF\xA6\xA3\xB5\xDB\xA9\x4D\xA7\x4F\xA4\x48\xC1\xBF\xB8\xDC\xA9\x4F\xA1\x41\xA8\x53\xA6\xB3\xAA\xC5\xAA\xBA\xBC\x4C\xA4\xDA\xA4\x46"
  /* Thor.980725: for chat&talk 用^z 作準備 */
#endif
};


/* return 0: 沒有 talk, 1: 有 talk, -1: 其他 */


int
talk_page(up)
  UTMP *up;
{
  int sock, msgsock;
  struct sockaddr_in sin;
  pid_t pid;
  int ans, length;
  char buf[60];
#if     defined(__OpenBSD__)
  struct hostent *h;
#endif

#ifdef EVERY_Z
  /* Thor.980725: 為 talk & chat 可用 ^z 作準備 */
  if (vio_holdon())
  {
    /* 您講話講一半還沒講完耶 */
    vmsg("\xB1\x7A\xC1\xBF\xB8\xDC\xC1\xBF\xA4\x40\xA5\x62\xC1\xD9\xA8\x53\xC1\xBF\xA7\xB9\xAD\x43");
    return 0;
  }
#endif

  pid = up->mode;
  if (pid >= M_SYSTEM && pid <= M_CHAT)
  {
    /* 對方無暇聊天 */
    vmsg("\xB9\xEF\xA4\xE8\xB5\x4C\xB7\x76\xB2\xE1\xA4\xD1");
    return 0;
  }

  if (!(pid = up->pid) || kill(pid, 0))
  {
    vmsg(MSG_USR_LEFT);
    return 0;
  }

  /* showplans(up->userid); */

  /* 確定要和他/她談天嗎(Y/N)？[N]  */
  if (vans("\xBD\x54\xA9\x77\xAD\x6E\xA9\x4D\xA5\x4C/\xA6\x6F\xBD\xCD\xA4\xD1\xB6\xDC(Y/N)\xA1\x48[N] ") != 'y')
    return 0;

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
    return 0;

#if     defined(__OpenBSD__)		      /* lkchu */

  if (!(h = gethostbyname(str_host)))
    return -1;  
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = 0;
  memcpy(&sin.sin_addr, h->h_addr, h->h_length);  

#else

  sin.sin_family = AF_INET;
  sin.sin_port = 0;
  sin.sin_addr.s_addr = INADDR_ANY;
  memset(sin.sin_zero, 0, sizeof(sin.sin_zero));

#endif

  length = sizeof(sin);
  if (bind(sock, (struct sockaddr *) &sin, length) < 0 || 
    getsockname(sock, (struct sockaddr *) &sin, &length) < 0)
  {
    close(sock);
    return 0;
  }

  cutmp->sockport = sin.sin_port;
  strcpy(cutmp->mateid, up->userid);
  up->talker = cutmp;
  utmp_mode(M_PAGE);
  kill(pid, SIGUSR1);

  clear();
  /* 首度呼叫 %s ...\n可按 Ctrl-D 中止 */
  prints("\xAD\xBA\xAB\xD7\xA9\x49\xA5\x73 %s ...\n\xA5\x69\xAB\xF6 Ctrl-D \xA4\xA4\xA4\xEE", up->userid);

  listen(sock, 1);
  add_io(sock, 20);
  do
  {
    msgsock = igetch();

    if (msgsock == Ctrl('D'))
    {
      talk_hangup(sock);
      return -1;
    }

    if (msgsock == I_TIMEOUT)
    {
      move(0, 0);
      /* 再 */
      outs("\xA6\x41");
      bell();

      if (kill(pid, SIGUSR1)) 
      /* Thor.990201.註解: 其實這個 kill，也只是看看對方是不是還在線上而已，重發signal其實似乎 talk_rqst 不會再被叫 */
      {
	talk_hangup(sock);
	vmsg(MSG_USR_LEFT);
	return -1;
      }
    }
  } while (msgsock != I_OTHERDATA);

  msgsock = accept(sock, NULL, NULL);
  talk_hangup(sock);
  if (msgsock == -1)
    return -1;

  length = read(msgsock, buf, sizeof(buf));
  ans = buf[0];
  if (ans == 'y')
  {
    sprintf(page_requestor, "%s (%s)", up->userid, up->username);
#ifdef HAVE_MULTI_BYTE
    page_requestor_zhc = up->ufo & UFO_ZHC;
#endif

    /* Thor.980814.注意: 在此有一個雞同鴨講的可能情況, 如果 A 先 page B, 但在 B 回應前卻馬上離開, 換 page C,
	C 尚未回應前, 如果 B 回應了, B 就會被 accept, 而不是 C.
	此時在螢幕中央, 看到的 page_requestor會是 C, 可是事實上, talk的對象是 B, 造成雞同鴨講!
	暫時不予修正, 以作為花心者的懲罰 :P    */

    talk_speak(msgsock);
  }
  else
  {
    char *reply;

    if (ans == ' ')
    {
      reply = buf;
      reply[length] = '\0';
    }
    else
      reply = talk_reason[ans - '1'];

    move(4, 0);
    /* 【回音】 */
    outs("\xA1\x69\xA6\x5E\xAD\xB5\xA1\x6A");
    outs(reply);
  }

  close(msgsock);
  cutmp->talker = NULL;
#ifdef LOG_TALK
  if (ans == 'y')  	/* itoc.000512: 防止Talk被拒絕時，產生聊天記錄的record */
    talk_save();	/* lkchu.981201: talk 記錄處理 */  
#endif
  /* 聊天結束 */
  vmsg("\xB2\xE1\xA4\xD1\xB5\xB2\xA7\xF4");
  return 1;
}


/* ----------------------------------------------------- */
/* talk main-routines					 */
/* ----------------------------------------------------- */


int
t_pager()
{
#if 0	/* itoc.010923: 避免誤按，分清楚一點 */
  ┌────┬─────┬─────┬─────┐
  │        │ UFO_PAGER│ UFO_RCVER│ UFO_QUIET│
  ├────┼─────┼─────┼─────┤
  │完全開放│          │          │          │
  ├────┼─────┼─────┼─────┤   
  │好友專線│    ○    │          │          │
  ├────┼─────┼─────┼─────┤
  │遠離塵囂│    ○    │    ○    │    ○    │
  └────┴─────┴─────┴─────┘
#endif

  /* 切換扣機為 1)完全開放 2)好友專線 3)遠離塵囂 [Q]  */
  switch (vans("\xA4\xC1\xB4\xAB\xA6\xA9\xBE\xF7\xAC\xB0 1)\xA7\xB9\xA5\xFE\xB6\x7D\xA9\xF1 2)\xA6\x6E\xA4\xCD\xB1\x4D\xBD\x75 3)\xBB\xB7\xC2\xF7\xB9\xD0\xC4\xDB [Q] "))
  {
  case '1':
#ifdef HAVE_NOBROAD
    cuser.ufo &= ~(UFO_PAGER | UFO_RCVER | UFO_QUIET);
#else
    cuser.ufo &= ~(UFO_PAGER | UFO_QUIET);
#endif
    cutmp->ufo = cuser.ufo;
    return 0;	/* itoc.010923: 雖然不需要重繪螢幕，但是強迫重繪才能更新 feeter 中的 pager 模式 */

  case '2':
    cuser.ufo |= UFO_PAGER;
#ifdef HAVE_NOBROAD
    cuser.ufo &= ~(UFO_RCVER | UFO_QUIET);
#else
    cuser.ufo &= ~UFO_QUIET;
#endif
    cutmp->ufo = cuser.ufo;
    return 0;

  case '3':
#ifdef HAVE_NOBROAD
    cuser.ufo |= (UFO_PAGER | UFO_RCVER | UFO_QUIET);
#else
    cuser.ufo |= (UFO_PAGER | UFO_QUIET);
#endif
    cutmp->ufo = cuser.ufo;
    return 0;
  }

  return XEASY;  
}


int
t_cloak()
{
#ifdef HAVE_SUPERCLOAK
  if (HAS_PERM(PERM_ALLADMIN))
  {
    /* 隱身模式 1)一般隱形 2)紫色隱形 3)不隱形 [Q]  */
    switch (vans("\xC1\xF4\xA8\xAD\xBC\xD2\xA6\xA1 1)\xA4\x40\xAF\xEB\xC1\xF4\xA7\xCE 2)\xB5\xB5\xA6\xE2\xC1\xF4\xA7\xCE 3)\xA4\xA3\xC1\xF4\xA7\xCE [Q] "))
    {
    case '1':
      cuser.ufo |= UFO_CLOAK;
      cuser.ufo &= ~UFO_SUPERCLOAK;
      /* 嘿嘿，躲起來囉！ */
      vmsg("\xBC\x4B\xBC\x4B\xA1\x41\xB8\xFA\xB0\x5F\xA8\xD3\xC5\x6F\xA1\x49");
      break;

    case '2':
      cuser.ufo |= (UFO_CLOAK | UFO_SUPERCLOAK);
      /* 嘿嘿，藏起來囉，其他站長也看不到我！ */
      vmsg("\xBC\x4B\xBC\x4B\xA1\x41\xC2\xC3\xB0\x5F\xA8\xD3\xC5\x6F\xA1\x41\xA8\xE4\xA5\x4C\xAF\xB8\xAA\xF8\xA4\x5D\xAC\xDD\xA4\xA3\xA8\xEC\xA7\xDA\xA1\x49");
      break;

    case '3':
      cuser.ufo &= ~(UFO_CLOAK | UFO_SUPERCLOAK);
      /* 重現江湖了 */
      vmsg("\xAD\xAB\xB2\x7B\xA6\xBF\xB4\xF2\xA4\x46");
      break;
    }
  }
  else
#endif
  if (HAS_PERM(PERM_CLOAK))
  {
    cuser.ufo ^= UFO_CLOAK;
    /* 嘿嘿，躲起來囉！ */
    /* 重現江湖了 */
    vmsg(cuser.ufo & UFO_CLOAK ? "\xBC\x4B\xBC\x4B\xA1\x41\xB8\xFA\xB0\x5F\xA8\xD3\xC5\x6F\xA1\x49" : "\xAD\xAB\xB2\x7B\xA6\xBF\xB4\xF2\xA4\x46");
  }

  cutmp->ufo = cuser.ufo;
  return XEASY;
}


int
t_query()
{
  ACCT acct;

  /* 查詢網友 */
  vs_bar("\xAC\x64\xB8\xDF\xBA\xF4\xA4\xCD");
  if (acct_get(msg_uid, &acct) > 0)
  {
    move(1, 0);
    clrtobot();
    do_query(&acct);
  }

  return 0;
}


static int
talk_choose()
{
  UTMP *up, *ubase, *uceil;
  int self;
  char userid[IDLEN + 1];

  ll_new();

  self = cuser.userno;
  ubase = up = ushm->uslot;
  /* uceil = ushm->uceil; */
  uceil = ubase + ushm->count;
  do
  {
    if (up->pid && up->userno != self && up->userlevel && can_see(cutmp, up))
      ll_add(up->userid);
  } while (++up <= uceil);

  if (!vget(1, 0, msg_uid, userid, IDLEN + 1, GET_LIST))
    return 0;

  up = ubase;
  do
  {
    if (!str_cmp(userid, up->userid))
      return up->userno;
  } while (++up <= uceil);

  return 0;
}


int
t_talk()
{
  int tuid, unum, ucount;
  UTMP *up;
  char ans[4];

  if (total_user <= 1)
  {
    /* 目前線上只有您一人，快邀請大家來光臨【 */
    /* 】吧！ */
    zmsg("\xA5\xD8\xAB\x65\xBD\x75\xA4\x57\xA5\x75\xA6\xB3\xB1\x7A\xA4\x40\xA4\x48\xA1\x41\xA7\xD6\xC1\xDC\xBD\xD0\xA4\x6A\xAE\x61\xA8\xD3\xA5\xFA\xC1\x7B\xA1\x69" BBSNAME "\xA1\x6A\xA7\x61\xA1\x49");
    return XEASY;
  }

  tuid = talk_choose();
  if (!tuid)
    return 0;

  /* ----------------- */
  /* multi-login check */
  /* ----------------- */

  move(3, 0);
  unum = 1;
  while ((ucount = utmp_count(tuid, 0)) > 1)
  {
    /* (0) 不想 talk 了...\n */
    outs("(0) \xA4\xA3\xB7\x51 talk \xA4\x46...\n");
    utmp_count(tuid, 1);
    /* 請選擇一個聊天對象 [0]： */
    vget(1, 33, "\xBD\xD0\xBF\xEF\xBE\xDC\xA4\x40\xAD\xD3\xB2\xE1\xA4\xD1\xB9\xEF\xB6\x48 [0]\xA1\x47", ans, 3, DOECHO);
    unum = atoi(ans);
    if (unum == 0)
      return 0;
    move(3, 0);
    clrtobot();
    if (unum > 0 && unum <= ucount)
      break;
  }

  if (up = utmp_search(tuid, unum))
  {
    if (can_override(up))
    {
      if (tuid != up->userno)
	vmsg(MSG_USR_LEFT);
      else
	talk_page(up);
    }
    else
    {
      /* 對方關掉呼叫器了 */
      vmsg("\xB9\xEF\xA4\xE8\xC3\xF6\xB1\xBC\xA9\x49\xA5\x73\xBE\xB9\xA4\x46");
    }
  }

  return 0;
}


/* ------------------------------------- */
/* 有人來串門子了，回應呼叫器		 */
/* ------------------------------------- */


void
talk_rqst()
{
  UTMP *up;
  int mode, sock, ans, len, port;
  char buf[80];
  struct sockaddr_in sin;
  screenline sl[T_LINES];
#if     defined(__OpenBSD__)
  struct hostent *h;
#endif

  up = cutmp->talker;
  if (!up)
    return;

  port = up->sockport;
  if (!port)
    return;

  mode = bbsmode;
  utmp_mode(M_TRQST);

  vs_save(sl);

  clear();
  sprintf(page_requestor, "%s (%s)", up->userid, up->username);

#ifdef EVERY_Z
  /* Thor.980725: 為 talk & chat 可用 ^z 作準備 */

  if (vio_holdon())
  {
    /* %s 想和您聊，不過您只有一張嘴 */
    sprintf(buf, "%s \xB7\x51\xA9\x4D\xB1\x7A\xB2\xE1\xA1\x41\xA4\xA3\xB9\x4C\xB1\x7A\xA5\x75\xA6\xB3\xA4\x40\xB1\x69\xBC\x4C", page_requestor);
    vmsg(buf);
    buf[0] = ans = '6';		/* Thor.980725: 只有一張嘴 */
    len = 1;
    goto over_for;
  }
#endif

  bell();
  /* 您想跟 %s 聊天嗎？(來自 %s) */
  prints("\xB1\x7A\xB7\x51\xB8\xF2 %s \xB2\xE1\xA4\xD1\xB6\xDC\xA1\x48(\xA8\xD3\xA6\xDB %s)", page_requestor, up->from);
  for (;;)	/* 讓使用者可以先查詢要求聊天的對方 */
  {
    /* ==> Q)查詢 Y)聊天 N)取消？[Y]  */
    ans = vget(1, 0, "==> Q)\xAC\x64\xB8\xDF Y)\xB2\xE1\xA4\xD1 N)\xA8\xFA\xAE\xF8\xA1\x48[Y] ", buf, 3, LCECHO);
    if (ans == 'q')
      my_query(up->userid);
    else
      break;
  }

  len = 1;

  if (ans == 'n')
  {
    move(2, 0);
    clrtobot();
    for (ans = 0; ans < 5; ans++)
      prints("\n (%d) %s", ans + 1, talk_reason[ans]);
    /* 請輸入選項或其他情由 [1]：\n==>  */
    ans = vget(10, 0, "\xBD\xD0\xBF\xE9\xA4\x4A\xBF\xEF\xB6\xB5\xA9\xCE\xA8\xE4\xA5\x4C\xB1\xA1\xA5\xD1 [1]\xA1\x47\n==> ", buf + 1, 60, DOECHO);

    if (ans == 0)
      ans = '1';

    if (ans >= '1' && ans <= '5')
    {
      buf[0] = ans;
    }
    else
    {
      buf[0] = ans = ' ';
      len = strlen(buf);
    }
  }
  else
  {
    buf[0] = ans = 'y';
  }

#ifdef EVERY_Z
over_for:
#endif

  sock = socket(AF_INET, SOCK_STREAM, 0);

#if     defined(__OpenBSD__)		      /* lkchu */

  if (!(h = gethostbyname(str_host)))
    return;
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = port;
  memcpy(&sin.sin_addr, h->h_addr, h->h_length);
  
#else

  sin.sin_family = AF_INET;
  sin.sin_port = port;
  /* sin.sin_addr.s_addr = INADDR_LOOPBACK; */
  /* sin.sin_addr.s_addr = INADDR_ANY; */
  /* For FreeBSD 4.x */
  sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  memset(sin.sin_zero, 0, sizeof(sin.sin_zero));

#endif

  if (!connect(sock, (struct sockaddr *) & sin, sizeof(sin)))
  {
    send(sock, buf, len, 0);

    if (ans == 'y')
    {
      strcpy(cutmp->mateid, up->userid);
#ifdef HAVE_MULTI_BYTE
      page_requestor_zhc = up->ufo & UFO_ZHC;
#endif

      talk_speak(sock);
    }
  }

  close(sock);
#ifdef  LOG_TALK
  if (ans == 'y')	/* mat.991011: 防止Talk被拒絕時，產生聊天記錄的record */
    talk_save();	/* lkchu.981201: talk 記錄處理 */
#endif
  vs_restore(sl);
  utmp_mode(mode);
}
