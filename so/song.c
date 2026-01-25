/*-------------------------------------------------------*/
/* song.c	( YZU_CSE WindTop BBS )			 */
/*-------------------------------------------------------*/
/* target : song ordering routines			 */
/* create :   /  /  					 */
/* update : 01/12/18					 */
/*-------------------------------------------------------*/


#include "bbs.h"


#ifdef HAVE_SONG

extern BCACHE *bshm;
extern XZ xz[];
extern char xo_pool[];
extern char brd_bits[];


static void XoSong();


#define	SONG_SRC	"<~Src~>"
#define SONG_DES	"<~Des~>"
#define SONG_SAY	"<~Say~>"
#define SONG_END	"<~End~>"


#ifdef LOG_SONG_USIES
static int			/* -1:沒找到  >=0:pos */
song_usies_find(fpath, chrono, songdata)
  char *fpath;
  time_t chrono;
  SONGDATA *songdata;
{
  int fd, pos;
  int rc = -1;

  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    pos = 0;
    while (read(fd, songdata, sizeof(SONGDATA)) == sizeof(SONGDATA))
    {
      if (songdata->chrono == chrono)
      {
	rc = pos;
	break;
      }
      pos++;
    }
    close(fd);
  }
  return rc;
}


static void
song_usies_log(chrono, title)
  time_t chrono;
  char *title;
{
  SONGDATA songdata;
  int pos;
  char *dir;

  dir = FN_RUN_SONGUSIES;

  if ((pos = song_usies_find(dir, chrono, &songdata)) >= 0)
  {
    songdata.count++;
    rec_put(dir, &songdata, sizeof(SONGDATA), pos, NULL);
  }
  else
  {
    songdata.chrono = chrono;
    songdata.count = 1;
    strcpy(songdata.title, title);
    rec_add(dir, &songdata, sizeof(SONGDATA));
  }
}
#endif


static int swaped;		/* 本文是否有 <~Src~> 等 */

static int
song_swap(str, src, des)
  char *str;
  char *src;
  char *des;
{
  char *ptr;
  char buf[ANSILINELEN];

  if (ptr = strstr(str, src))
  {
    *ptr = '\0';
    ptr += strlen(src);
    /* sprintf(buf, "%s%s%s", str, des, ptr); */
    snprintf(buf, ANSILINELEN, "%s%s%s", str, des, ptr);	/* swap 可能會超過 ANSILINELEN */
    strcpy(str, buf);

    /* return 1; */
    return ++swaped;
  }

  return 0;
}


static void
song_quote(fpr, fpw, src, des, say)	/* 從 fpr 讀出內文，把 <~Src~> 等替換掉，寫入 fpw */
  FILE *fpr, *fpw;
  char *src, *des, *say;
{
  char buf[ANSILINELEN];

  swaped = 0;
  while (fgets(buf, sizeof(buf), fpr))
  {
    if (strstr(buf, SONG_END))
      break;

    while (song_swap(buf, SONG_SRC, src));
    while (song_swap(buf, SONG_DES, des));
    while (song_swap(buf, SONG_SAY, say));
    fputs(buf, fpw);
  }

  if (!swaped)	/* itoc.011115: 如果本文沒有 <~Src~>，則在最後加上 */
  {
    /* 在最後一行加上 <~Src~> 想對 <~Des~> 說 <~Say~> */
    /*  想對  */
    /*  說  */
    strcpy(buf, "\033[1;33m" SONG_SRC " \xB7\x51\xB9\xEF " SONG_DES " \xBB\xA1 " SONG_SAY "\033[m\n");
    song_swap(buf, SONG_SRC, src);
    song_swap(buf, SONG_DES, des);
    song_swap(buf, SONG_SAY, say);
    fputs(buf, fpw);
  }

  /* 在檔案最後加上點歌時間 */
  fprintf(fpw, "\n--\n%s\n", Now());
}


#define GEM_FILE	0x01	/* 預期是檔案 */


static HDR *		/* NULL:無權讀取 */
song_check(xo, fpath, op)
  XO *xo;
  char *fpath;
  int op;
{
  HDR *hdr;
  int gtype;

  hdr = (HDR *) xo_pool + (xo->pos - xo->top);
  gtype = hdr->xmode;

  if ((gtype & GEM_RESTRICT) && !(xo->key & GEM_M_BIT))
    return NULL;

  if ((op & GEM_FILE) && (gtype & GEM_FOLDER))
    return NULL;

  if (fpath)
  {
    if (gtype & GEM_BOARD)
      gem_fpath(fpath, hdr->xname, fn_dir);
    else
      hdr_fpath(fpath, xo->dir, hdr);
  }
  return hdr;
}


static void
song_item(num, hdr, level)
  int num;
  HDR *hdr;
  int level;
{
  int xmode, gtype;

  xmode = hdr->xmode;
  gtype = (char) 0xba;

  /* 目錄用實心，不是目錄用空心 */
  if (xmode & GEM_FOLDER)		/* 文章:◇ 卷宗:◆ */
    gtype += 1;

  if (hdr->xname[0] == '@')		/* 資料:☆ 分類:★ */
    gtype -= 2;
  else if (xmode & GEM_BOARD)		/*         看板:■ */
    gtype += 2;

  prints("%6d%c \241%c ", num, xmode & GEM_RESTRICT ? ')' : ' ', gtype);

  if ((xmode & GEM_RESTRICT) && !(level & GEM_M_BIT))
    outs(MSG_DATA_CLOAK);
  else
    prints("%.*s\n", d_cols + 64, hdr->title);
}


static int
song_body(xo)
  XO *xo;
{
  HDR *hdr;
  int num, max, tail;

  max = xo->max;
  if (max <= 0)
  {
    /* \n\n《歌本》尚在吸取天地間的日精月華 :) */
    outs("\n\n\xA1\x6D\xBA\x71\xA5\xBB\xA1\x6E\xA9\x7C\xA6\x62\xA7\x6C\xA8\xFA\xA4\xD1\xA6\x61\xB6\xA1\xAA\xBA\xA4\xE9\xBA\xEB\xA4\xEB\xB5\xD8 :)");
    vmsg(NULL);
    return XO_QUIT;
  }

  hdr = (HDR *) xo_pool;
  num = xo->top;
  tail = num + XO_TALL;
  if (max > tail)
    max = tail;

  move(3, 0);
  tail = xo->key;	/* 借用 tail */
  do
  {
    song_item(++num, hdr++, tail);
  } while (num < max);
  clrtobot();

  /* return XO_NONE; */
  return XO_FOOT;	/* itoc.010403: 把 b_lines 填上 feeter */  
}


static int
song_head(xo)
  XO *xo;
{
  /* 精華文章 */
  vs_head("\xBA\xEB\xB5\xD8\xA4\xE5\xB3\xB9", xo->xyz);
  prints(NECKER_SONG, d_cols, "");
  return song_body(xo);
}


static int
song_init(xo)
  XO *xo;
{
  xo_load(xo, sizeof(HDR));
  return song_head(xo);
}


static int
song_load(xo)
  XO *xo;
{
  xo_load(xo, sizeof(HDR));
  return song_body(xo);
}


/* ----------------------------------------------------- */
/* 資料之新增：append / insert				 */
/* ----------------------------------------------------- */


static int
song_browse(xo)
  XO *xo;
{
  HDR *hdr;
  int op, xmode;
  char fpath[64], title[TTLEN + 1];

  op = 0;

  for (;;)
  {
    if (!(hdr = song_check(xo, fpath, op)))
      break;

    xmode = hdr->xmode;

    /* browse folder */

    if (xmode & GEM_FOLDER)
    {
      if (xmode & GEM_BOARD)
      {
	if ((op = gem_link(hdr->xname)) < 0)
	{
	  /* 對不起，此板精華區只准板友進入，請向板主申請入境許可 */
	  vmsg("\xB9\xEF\xA4\xA3\xB0\x5F\xA1\x41\xA6\xB9\xAA\x4F\xBA\xEB\xB5\xD8\xB0\xCF\xA5\x75\xAD\xE3\xAA\x4F\xA4\xCD\xB6\x69\xA4\x4A\xA1\x41\xBD\xD0\xA6\x56\xAA\x4F\xA5\x44\xA5\xD3\xBD\xD0\xA4\x4A\xB9\xD2\xB3\x5C\xA5\x69");
	  return XO_FOOT;
	}
      }
      else			/* 一般卷宗 */
      {
	op = xo->key;		/* 繼承母卷宗的權限 */
      }

      strcpy(title, hdr->title);
      XoSong(fpath, title, op);
      return song_init(xo);
    }

    /* browse article */

    /* Thor.990204: 為考慮more 傳回值 */
    if ((xmode = more(fpath, FOOTER_GEM)) < 0)
      break;

    op = GEM_FILE;

re_key:
    switch (xo_getch(xo, xmode))
    {
    case XO_BODY:
      continue;

    case '/':
      /* 搜尋： */
      if (vget(b_lines, 0, "\xB7\x6A\xB4\x4D\xA1\x47", hunt, sizeof(hunt), DOECHO))
      {
	more(fpath, FOOTER_GEM);
	goto re_key;
      }
      continue;

    case 'C':
      {
	FILE *fp;
	if (fp = tbf_open())
	{
	  f_suck(fp, fpath);
	  fclose(fp);
	}
      }
      break;

    case 'h':
      xo_help("song");
      break;
    }
    break;
  }

  return song_head(xo);
}


static int
count_ktv()	/* itoc.021102: ktv 板裡面已有幾篇的點歌記錄 */
{
  int count, fd;
  time_t yesterday;
  char folder[64];
  HDR *hdr;

  brd_fpath(folder, BN_KTV, fn_dir);
  if ((fd = open(folder, O_RDONLY)) < 0)
    return 0;

  mgets(-1);
  count = 0;
  yesterday = time(0) - 86400;
  while (hdr = mread(fd, sizeof(HDR)))
  {
    /* 即使是匿名點歌，userid 仍記錄在 hdr.owner + IDLEN + 1 */
    if (!strcmp(hdr->owner + IDLEN + 1, cuser.userid) &&
      hdr->chrono > yesterday)
    {
      if (++count >= 3)		/* 限制點三首 */
	break;
    }
  }
  close(fd);

  return count;
}


static int
song_order(xo)
  XO *xo;
{
#ifdef HAVE_ANONYMOUS
  int annoy;
#endif
  char fpath[64], des[20], say[57], buf[64];
  HDR *hdr, xpost;
  FILE *fpr, *fpw;

  /* itoc.註解: song_order 視同 post */
  if (!HAS_PERM(PERM_POST))
    return XO_NONE;

  /* itoc.010831: 點歌要錢，所以要禁止 multi-login */
  if (HAS_STATUS(STATUS_COINLOCK))
  {
    vmsg(msg_coinlock);
    return XO_FOOT;
  }

  if (count_ktv() >= 3)		/* 限制點三首 */
  {
    /* 過去二十四小時內您已點選過多歌曲 */
    vmsg("\xB9\x4C\xA5\x68\xA4\x47\xA4\x51\xA5\x7C\xA4\x70\xAE\xC9\xA4\xBA\xB1\x7A\xA4\x77\xC2\x49\xBF\xEF\xB9\x4C\xA6\x68\xBA\x71\xA6\xB1");
    return XO_FOOT;
  }

  if (cuser.money < 1000)
  {
    /* 要 1000 銀幣才能點歌到看板喔 */
    vmsg("\xAD\x6E 1000 \xBB\xC8\xB9\xF4\xA4\x7E\xAF\xE0\xC2\x49\xBA\x71\xA8\xEC\xAC\xDD\xAA\x4F\xB3\xE1");
    return XO_FOOT;
  }

  if (!(hdr = song_check(xo, fpath, GEM_FILE)))
    return XO_NONE;

  /* 點歌給誰： */
  if (!vget(b_lines, 0, "\xC2\x49\xBA\x71\xB5\xB9\xBD\xD6\xA1\x47", des, sizeof(des), DOECHO))
    return XO_FOOT;
  /* 想說的話： */
  if (!vget(b_lines, 0, "\xB7\x51\xBB\xA1\xAA\xBA\xB8\xDC\xA1\x47", say, sizeof(say), DOECHO))
    return XO_FOOT;

#ifdef HAVE_ANONYMOUS
  /* 想要匿名嗎(Y/N)？[N]  */
  annoy = vans("\xB7\x51\xAD\x6E\xB0\xCE\xA6\x57\xB6\xDC(Y/N)\xA1\x48[N] ") == 'y';
#endif

  /* 確定點歌嗎(Y/N)？[Y]  */
  if (vans("\xBD\x54\xA9\x77\xC2\x49\xBA\x71\xB6\xDC(Y/N)\xA1\x48[Y] ") == 'n')
    return XO_FOOT;

  if (!(fpr = fopen(fpath, "r")))
    return XO_FOOT;

  cuser.money -= 1000;

#ifdef LOG_SONG_USIES
  song_usies_log(hdr->chrono, hdr->title);
#endif

  /* 加入文章檔案 */

  brd_fpath(fpath, BN_KTV, fn_dir);

  if (fpw = fdopen(hdr_stamp(fpath, 'A', &xpost, buf), "w"))
  {
#ifdef HAVE_ANONYMOUS
    song_quote(fpr, fpw, annoy ? STR_ANONYMOUS : cuser.userid, des, say);
#else
    song_quote(fpr, fpw, cuser.userid, des, say);
#endif
    fclose(fpw);
  }

  fclose(fpr);

  /* 加入 .DIR record */

#ifdef HAVE_ANONYMOUS
  strcpy(xpost.owner, annoy ? STR_ANONYMOUS : cuser.userid);
  /* 即使是匿名點歌，userid 仍記錄在 hdr.owner + IDLEN + 1 */
  strcpy(xpost.owner + IDLEN + 1, cuser.userid);
#else
  strcpy(xpost.owner, cuser.userid);
#endif
  strcpy(xpost.nick, xpost.owner);
  /* %s 點給 %s */
  sprintf(xpost.title, "%s \xC2\x49\xB5\xB9 %s", xpost.owner, des);
  rec_bot(fpath, &xpost, sizeof(HDR));

  btime_update(brd_bno(BN_KTV));

  return XO_FOOT;
}


static int
song_send(xo)
  XO *xo;
{
  char fpath[64], say[57], buf[64];
  HDR *hdr, xpost;
  FILE *fpr, *fpw;
  ACCT acct;

  /* itoc.註解: song_send 視同 mail */
  if (!HAS_PERM(PERM_LOCAL))
    return XO_NONE;

  if (!(hdr = song_check(xo, fpath, GEM_FILE)))
    return XO_NONE;

  /* 點歌給誰： */
  if (acct_get("\xC2\x49\xBA\x71\xB5\xB9\xBD\xD6\xA1\x47", &acct) < 1)	/* acct_get 可能會 clear，所以要重繪 */
    return song_head(xo);

  /* 想說的話： */
  if (!vget(b_lines, 0, "\xB7\x51\xBB\xA1\xAA\xBA\xB8\xDC\xA1\x47", say, sizeof(say), DOECHO))
    strcpy(say, ".........");

  /* 確定點歌嗎(Y/N)？[Y]  */
  if (vans("\xBD\x54\xA9\x77\xC2\x49\xBA\x71\xB6\xDC(Y/N)\xA1\x48[Y] ") == 'n')
    return song_head(xo);			/* acct_get 可能會 clear，所以要重繪 */

#ifdef LOG_SONG_USIES
  song_usies_log(hdr->chrono, hdr->title);
#endif

  /* 加入文章檔案 */

  if (!(fpr = fopen(fpath, "r")))
    return song_head(xo);			/* acct_get 可能會 clear，所以要重繪 */

  usr_fpath(fpath, acct.userid, fn_dir);

  if (fpw = fdopen(hdr_stamp(fpath, 0, &xpost, buf), "w"))
  {
    song_quote(fpr, fpw, cuser.userid, acct.userid, say);
    fclose(fpw);
  }

  fclose(fpr);

  /* 加入 .DIR record */

  strcpy(xpost.owner, cuser.userid);
  strcpy(xpost.nick, cuser.username);
  /* %s 點歌給您 */
  sprintf(xpost.title, "%s \xC2\x49\xBA\x71\xB5\xB9\xB1\x7A", cuser.userid);
  rec_add(fpath, &xpost, sizeof(HDR));

  mail_hold(buf, acct.userid, xpost.title, 0);
  m_biff(acct.userno);		/* 若對方在線上，則要通知有新信 */

  return song_head(xo);				/* acct_get 可能會 clear，所以要重繪 */
}


static int
song_internet(xo)
  XO *xo;
{
  int rc;
  char fpath[64], rcpt[64], des[20], say[57];
  HDR *hdr;
  FILE *fpr, *fpw;

  /* itoc.註解: song_internet 視同 internet_mail */
  if (!HAS_PERM(PERM_INTERNET))
    return XO_NONE;

  if (!(hdr = song_check(xo, fpath, GEM_FILE)))
    return XO_NONE;

  /* 目的地： */
  if (!vget(b_lines, 0, "\xA5\xD8\xAA\xBA\xA6\x61\xA1\x47", rcpt, sizeof(rcpt), DOECHO))
    return XO_FOOT;
  if (!strchr(rcpt, '@'))
    return XO_FOOT;

  /* 點歌給誰： */
  if (!vget(b_lines, 0, "\xC2\x49\xBA\x71\xB5\xB9\xBD\xD6\xA1\x47", des, sizeof(des), DOECHO))
    return XO_FOOT;
  /* 想說的話： */
  if (!vget(b_lines, 0, "\xB7\x51\xBB\xA1\xAA\xBA\xB8\xDC\xA1\x47", say, sizeof(say), DOECHO))
    strcpy(say, ".........");

  /* 確定點歌嗎(Y/N)？[Y]  */
  if (vans("\xBD\x54\xA9\x77\xC2\x49\xBA\x71\xB6\xDC(Y/N)\xA1\x48[Y] ") == 'n')
    return XO_FOOT;

#ifdef LOG_SONG_USIES
  song_usies_log(hdr->chrono, hdr->title);
#endif

  /* 加入文章檔案 */

  if (!(fpr = fopen(fpath, "r")))
    return XO_FOOT;

  sprintf(fpath, "tmp/song_internet.%s", cuser.userid);
  fpw = fopen(fpath, "w");

  song_quote(fpr, fpw, cuser.userid, des, say);

  fclose(fpr);
  fclose(fpw);

  /* 寄給對方 */

  /* 點歌給您 */
  rc = bsmtp(fpath, "\xC2\x49\xBA\x71\xB5\xB9\xB1\x7A", rcpt, 0);
  /* 信件無法寄達，底稿備份在信箱 */
  vmsg(rc >= 0 ? msg_sent_ok : "\xAB\x48\xA5\xF3\xB5\x4C\xAA\x6B\xB1\x48\xB9\x46\xA1\x41\xA9\xB3\xBD\x5A\xB3\xC6\xA5\xF7\xA6\x62\xAB\x48\xBD\x63");

  mail_hold(fpath, rcpt, hdr->title, rc);
  unlink(fpath);

  return XO_FOOT;
}


static int
song_edit(xo)
  XO *xo;
{
  char fpath[64];

  if (!song_check(xo, fpath, GEM_FILE))
    return XO_NONE;

  if (xo->key & GEM_W_BIT)
    vedit(fpath, 0);
  else
    vedit(fpath, -1);		/* itoc.010403: 讓一般使用者也可以 edit 看控制碼 */
  return song_head(xo);
}


static int
song_title(xo)
  XO *xo;
{
  HDR *fhdr, mhdr;
  int pos, cur;

  if (!(xo->key & GEM_W_BIT) || !(fhdr = song_check(xo, NULL, 0)))
    return XO_NONE;

  memcpy(&mhdr, fhdr, sizeof(HDR));

  /* 標題： */
  vget(b_lines, 0, "\xBC\xD0\xC3\x44\xA1\x47", mhdr.title, TTLEN + 1, GCARRY);

  if (xo->key & GEM_X_BIT)
  {
    /* 編者： */
    vget(b_lines, 0, "\xBD\x73\xAA\xCC\xA1\x47", mhdr.owner, IDLEN + 1, GCARRY);
    /* vget(b_lines, 0, "暱稱：", mhdr.nick, sizeof(mhdr.nick), GCARRY); */	/* 精華區此欄位為空 */
    /* 日期： */
    vget(b_lines, 0, "\xA4\xE9\xB4\xC1\xA1\x47", mhdr.date, sizeof(mhdr.date), GCARRY);
  }

  if (memcmp(fhdr, &mhdr, sizeof(HDR)) && vans(msg_sure_ny) == 'y')
  {
    pos = xo->pos;
    cur = pos - xo->top;

    memcpy(fhdr, &mhdr, sizeof(HDR));
    rec_put(xo->dir, fhdr, sizeof(HDR), pos, NULL);

    move(3 + cur, 0);
    song_item(++pos, fhdr, xo->key);

  }
  return XO_FOOT;
}


static int
song_help(xo)
  XO *xo;
{
  xo_help("song");
  return song_head(xo);
}


static KeyFunc song_cb[] =
{
  XO_INIT, song_init,
  XO_LOAD, song_load,
  XO_HEAD, song_head,
  XO_BODY, song_body,

  'r', song_browse,
  'o', song_order,
  'm', song_send,
  'M', song_internet,

  'E', song_edit,
  'T', song_title,

  'h', song_help
};


static void
XoSong(folder, title, level)
  char *folder;
  char *title;
  int level;
{
  XO *xo, *last;

  last = xz[XZ_SONG - XO_ZONE].xo;	/* record */

  xz[XZ_SONG - XO_ZONE].xo = xo = xo_new(folder);
  xz[XZ_SONG - XO_ZONE].cb = song_cb;
  xo->pos = 0;
  xo->key = level;
  xo->xyz = title;
  /* 板主：系統管理者 */
  strcpy(currBM, "\xAA\x4F\xA5\x44\xA1\x47\xA8\x74\xB2\xCE\xBA\xDE\xB2\x7A\xAA\xCC");

  xover(XZ_SONG);

  free(xo);

  xz[XZ_SONG - XO_ZONE].xo = last;	/* restore */
}


int
XoSongMain()
{
  int level;
  char fpath[64];

  gem_fpath(fpath, BN_KTV, fn_dir);

  if (HAS_PERM(PERM_SYSOP))
    level = GEM_W_BIT | GEM_X_BIT | GEM_M_BIT;
  else if (HAS_PERM(PERM_ALLBOARD))
    level = GEM_W_BIT | GEM_M_BIT;
  else
    level = 0;

  /* 點歌系統 */
  XoSong(fpath, "\xC2\x49\xBA\x71\xA8\x74\xB2\xCE", level);
  return 0;
}


int
XoSongSub()
{
  int bno;

  if ((bno = brd_bno(BN_REQUEST)) >= 0)
  {
    XoPost(bno);
    xover(XZ_POST);
    return 0;
  }
  return XEASY;
}


int
XoSongLog()
{
  int bno;

  if ((bno = brd_bno(BN_KTV)) >= 0)
  {
    XoPost(bno);
    xover(XZ_POST);
    return 0;
  }
  return XEASY;
}
#endif	/* HAVE_SONG */
