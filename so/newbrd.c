/*-------------------------------------------------------*/
/* newbrd.c   ( YZU_CSE WindTop BBS )                    */
/*-------------------------------------------------------*/
/* target : 連署功能    			 	 */
/* create : 00/01/02				 	 */
/* update : 02/04/29				 	 */
/*-------------------------------------------------------*/
/* run/newbrd/_/.DIR - newbrd control header		 */
/* run/newbrd/_/@/@_ - newbrd description file		 */
/* run/newbrd/_/@/G_ - newbrd voted id loG file		 */
/*-------------------------------------------------------*/


#include "bbs.h"


#ifdef HAVE_COSIGN

extern XZ xz[];
extern char xo_pool[];
extern BCACHE *bshm;		/* itoc.010805: 開新板用 */

static int nbrd_add();
static int nbrd_body();
static int nbrd_head();

/* \033[33m──────────────────────────────\033[m\n */
static char *split_line = "\033[33m\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\033[m\n";


typedef struct
{
  char userid[IDLEN + 1];
  char email[60];
}      LOG;


static int
cmpbtime(nbrd)
  NBRD *nbrd;
{
  return nbrd->btime == currchrono;
}


static char
nbrd_attr(nbrd)
  NBRD *nbrd;
{
  int xmode = nbrd->mode;

  /* 筆劃越少的，越傾向結案 */
  if (xmode & NBRD_FINISH)
    return ' ';
  if (xmode & NBRD_END)
    return '-';
#ifdef SYSOP_START_COSIGN
  if (xmode & NBRD_START)
    return '+';
  else
    return 'x';
#else
  return '+';
#endif
}


static int
nbrd_stamp(folder, nbrd, fpath)
  char *folder;
  NBRD *nbrd;
  char *fpath;
{
  char *fname;
  char *family = NULL;
  int rc;
  int token;

  fname = fpath;
  while (rc = *folder++)
  {
    *fname++ = rc;
    if (rc == '/')
      family = fname;
  }

  fname = family;
  *family++ = '@';

  token = time(0);

  archiv32(token, family);

  rc = open(fpath, O_WRONLY | O_CREAT | O_EXCL, 0600);
  nbrd->btime = token;
  str_stamp(nbrd->date, nbrd->btime);
  strcpy(nbrd->xname, fname);

  return rc;
}


static void
nbrd_fpath(fpath, folder, nbrd)
  char *fpath;
  char *folder;
  NBRD *nbrd;
{
  char *str;
  int cc;

  while (cc = *folder++)
  {
    *fpath++ = cc;
    if (cc == '/')
      str = fpath;
  }
  strcpy(str, nbrd->xname);
}


static int
nbrd_init(xo)
  XO *xo;
{
  xo_load(xo, sizeof(NBRD));
  return nbrd_head(xo);
}


static int
nbrd_load(xo)
  XO *xo;
{
  xo_load(xo, sizeof(NBRD));
  return nbrd_body(xo);
}


static void
nbrd_item(num, nbrd)
  int num;
  NBRD *nbrd;
{
  prints("%6d %c %-5s %-13s [%s] %.*s\n", 
    num, nbrd_attr(nbrd), nbrd->date + 3, nbrd->owner, 
    /* \033[1;33m本站公投\033[m */
    (nbrd->mode & NBRD_NEWBOARD) ? nbrd->brdname : "\033[1;33m\xA5\xBB\xAF\xB8\xA4\xBD\xA7\xEB\033[m", d_cols + 20, nbrd->title);
}


static int
nbrd_body(xo)
  XO *xo;
{
  NBRD *nbrd;
  int num, max, tail;

  max = xo->max;
  if (max <= 0)
  {
    /* 要新增連署項目嗎(Y/N)？[N]  */
    if (vans("\xAD\x6E\xB7\x73\xBC\x57\xB3\x73\xB8\x70\xB6\xB5\xA5\xD8\xB6\xDC(Y/N)\xA1\x48[N] ") == 'y')
      return nbrd_add(xo);
    return XO_QUIT;
  }

  nbrd = (NBRD *) xo_pool;
  num = xo->top;
  tail = num + XO_TALL;

  if (max > tail)
    max = tail;

  move(3, 0);  
  do
  {
    nbrd_item(++num, nbrd++);
  } while (num < max);
  clrtobot();

  return XO_FOOT;
}


static int
nbrd_head(xo)
  XO *xo;
{
  /* 連署系統 */
  vs_head("\xB3\x73\xB8\x70\xA8\x74\xB2\xCE", str_site);
  prints(NECKER_COSIGN, d_cols, "");
  return nbrd_body(xo);
}


static int
nbrd_find(fpath, brdname)
  char *fpath, *brdname;
{
  NBRD old;
  int fd;
  int rc = 0;

  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    while (read(fd, &old, sizeof(NBRD)) == sizeof(NBRD))
    {
      if (!str_cmp(old.brdname, brdname) && !(old.mode & NBRD_FINISH))
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
nbrd_add(xo)
  XO *xo;
{
  int fd, ans, days, numbers;
  char *dir, fpath[64], path[64];
  char *brdname, *class, *title;
  FILE *fp;
  NBRD nbrd;

  if (HAS_PERM(PERM_ALLADMIN))
  {
    /* 連署模式 1)開新板 2)記名 3)無記名：[Q]  */
    ans = vans("\xB3\x73\xB8\x70\xBC\xD2\xA6\xA1 1)\xB6\x7D\xB7\x73\xAA\x4F 2)\xB0\x4F\xA6\x57 3)\xB5\x4C\xB0\x4F\xA6\x57\xA1\x47[Q] ");
    if (ans < '1' || ans > '3')
      return xo->max ? XO_FOOT : nbrd_body(xo);	/* itoc.020122: 如果沒有任何連署，要回到 nbrd_body() */
    /* itoc.030613: 其實下面的 return XO_FOOT; 也應該這樣改 */
  }
  else if (HAS_PERM(PERM_POST))
  {
    /* 一段使用者只能開新板連署 */
    ans = '1';
  }
  else
  {
    /* 對不起，本看板是唯讀的 */
    vmsg("\xB9\xEF\xA4\xA3\xB0\x5F\xA1\x41\xA5\xBB\xAC\xDD\xAA\x4F\xAC\x4F\xB0\xDF\xC5\xAA\xAA\xBA");
    return XO_FOOT;
  }

  memset(&nbrd, 0, sizeof(NBRD));

  brdname = nbrd.brdname;
  class = nbrd.class;
  title = nbrd.title;

  if (ans == '1')	/* 新板連署 */
  {
    /* 英文板名： */
    if (!vget(b_lines, 0, "\xAD\x5E\xA4\xE5\xAA\x4F\xA6\x57\xA1\x47", brdname, sizeof(nbrd.brdname), DOECHO))
      return XO_FOOT;

    if (brd_bno(brdname) >= 0 || !valid_brdname(brdname))
    {
      /* 已有此板或板名不合法 */
      vmsg("\xA4\x77\xA6\xB3\xA6\xB9\xAA\x4F\xA9\xCE\xAA\x4F\xA6\x57\xA4\xA3\xA6\x58\xAA\x6B");
      return XO_FOOT;
    }
    if (nbrd_find(xo->dir, brdname))
    {
      /* 正在連署中 */
      vmsg("\xA5\xBF\xA6\x62\xB3\x73\xB8\x70\xA4\xA4");
      return XO_FOOT;
    }

    /* 看板分類： */
    if (!vget(b_lines, 0, "\xAC\xDD\xAA\x4F\xA4\xC0\xC3\xFE\xA1\x47", class, sizeof(nbrd.class), DOECHO) ||
      /* 看板主題： */
      !vget(b_lines, 0, "\xAC\xDD\xAA\x4F\xA5\x44\xC3\x44\xA1\x47", title, sizeof(nbrd.title), DOECHO))
      return XO_FOOT;

    days = NBRD_DAY_BRD;
    numbers = NBRD_NUM_BRD;

#ifdef SYSOP_START_COSIGN
    nbrd.mode = NBRD_NEWBOARD;
#else
    nbrd.mode = NBRD_NEWBOARD | NBRD_START;
#endif
  }
  else			/* 其他連署 */
  {
    char tmp[8];

    /* 連署主題： */
    if (!vget(b_lines, 0, "\xB3\x73\xB8\x70\xA5\x44\xC3\x44\xA1\x47", title, sizeof(nbrd.title), DOECHO))
      return XO_FOOT;

    /* 連署日期最多 30 天，連署人數最多 500 人 */
    /* 連署天數： */
    if (!vget(b_lines, 0, "\xB3\x73\xB8\x70\xA4\xD1\xBC\xC6\xA1\x47", tmp, 5, DOECHO))
      return XO_FOOT;
    days = atoi(tmp);
    if (days > 30 || days < 1)
      return XO_FOOT;
    /* 連署人數： */
    if (!vget(b_lines, 0, "\xB3\x73\xB8\x70\xA4\x48\xBC\xC6\xA1\x47", tmp, 6, DOECHO))
      return XO_FOOT;
    numbers = atoi(tmp);
    if (numbers > 500 || numbers < 1)
      return XO_FOOT;

    nbrd.mode = (ans == '2') ? (NBRD_OTHER | NBRD_START) : (NBRD_OTHER | NBRD_START | NBRD_ANONYMOUS);
  }

  /* 開始編輯 [看板說明與板主抱負或連署原因] */
  vmsg("\xB6\x7D\xA9\x6C\xBD\x73\xBF\xE8 [\xAC\xDD\xAA\x4F\xBB\xA1\xA9\xFA\xBB\x50\xAA\x4F\xA5\x44\xA9\xEA\xAD\x74\xA9\xCE\xB3\x73\xB8\x70\xAD\xEC\xA6\x5D]");
  sprintf(path, "tmp/%s.nbrd", cuser.userid);	/* 連署原因的暫存檔案 */
  if (fd = vedit(path, 0))
  {
    unlink(path);
    vmsg(msg_cancel);
    return nbrd_head(xo);
  }

  dir = xo->dir;
  if ((fd = nbrd_stamp(dir, &nbrd, fpath)) < 0)
    return nbrd_head(xo);
  close(fd);

  nbrd.etime = nbrd.btime + days * 86400;
  nbrd.total = numbers;
  strcpy(nbrd.owner, cuser.userid);

  fp = fopen(fpath, "a");
  /* 作者: %s (%s) 站內: 連署系統\n */
  fprintf(fp, "\xA7\x40\xAA\xCC: %s (%s) \xAF\xB8\xA4\xBA: \xB3\x73\xB8\x70\xA8\x74\xB2\xCE\n", cuser.userid, cuser.username);
  /* 標題: %s\n */
  fprintf(fp, "\xBC\xD0\xC3\x44: %s\n", title);
  /* 時間: %s\n\n */
  fprintf(fp, "\xAE\xC9\xB6\xA1: %s\n\n", Now());

  if (ans == '1')
  {
    /* 英文板名：%s\n */
    fprintf(fp, "\xAD\x5E\xA4\xE5\xAA\x4F\xA6\x57\xA1\x47%s\n", brdname);
    /* 看板分類：%s\n */
    fprintf(fp, "\xAC\xDD\xAA\x4F\xA4\xC0\xC3\xFE\xA1\x47%s\n", class);
    /* 看板主題：%s\n */
    fprintf(fp, "\xAC\xDD\xAA\x4F\xA5\x44\xC3\x44\xA1\x47%s\n", title);
    /* 板主名稱：%s\n */
    fprintf(fp, "\xAA\x4F\xA5\x44\xA6\x57\xBA\xD9\xA1\x47%s\n", cuser.userid);
    /* 電子信箱：%s\n */
    fprintf(fp, "\xB9\x71\xA4\x6C\xAB\x48\xBD\x63\xA1\x47%s\n", cuser.email);
  }
  else
  {
    /* 連署主題：%s\n */
    fprintf(fp, "\xB3\x73\xB8\x70\xA5\x44\xC3\x44\xA1\x47%s\n", title);
  }
  /* 舉辦日期：%s\n */
  fprintf(fp, "\xC1\x7C\xBF\xEC\xA4\xE9\xB4\xC1\xA1\x47%s\n", nbrd.date);
  /* 到期天數：%d\n */
  fprintf(fp, "\xA8\xEC\xB4\xC1\xA4\xD1\xBC\xC6\xA1\x47%d\n", days);
  /* 需連署人：%d\n */
  fprintf(fp, "\xBB\xDD\xB3\x73\xB8\x70\xA4\x48\xA1\x47%d\n", numbers);
  fprintf(fp, split_line);
  /* 連署說明：\n */
  fprintf(fp, "\xB3\x73\xB8\x70\xBB\xA1\xA9\xFA\xA1\x47\n");
  f_suck(fp, path);
  unlink(path);
  fprintf(fp, split_line);
  fclose(fp);

  rec_add(dir, &nbrd, sizeof(NBRD));

#ifdef SYSOP_START_COSIGN
  /* 送交申請了，請等候核准吧 */
  /* 連署開始了！ */
  vmsg(ans == '1' ? "\xB0\x65\xA5\xE6\xA5\xD3\xBD\xD0\xA4\x46\xA1\x41\xBD\xD0\xB5\xA5\xAD\xD4\xAE\xD6\xAD\xE3\xA7\x61" : "\xB3\x73\xB8\x70\xB6\x7D\xA9\x6C\xA4\x46\xA1\x49");
#else
  /* 連署開始了！ */
  vmsg("\xB3\x73\xB8\x70\xB6\x7D\xA9\x6C\xA4\x46\xA1\x49");
#endif
  return nbrd_init(xo);
}


static int
nbrd_seek(fpath)
  char *fpath;
{
  LOG old;
  int fd;
  int rc = 0;

  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    while (read(fd, &old, sizeof(LOG)) == sizeof(LOG))
    {
      if (!strcmp(old.userid, cuser.userid) || !str_cmp(old.email, cuser.email))
      {
	rc = 1;
	break;
      }
    }
    close(fd);
  }
  return rc;
}


static void
addreply(hdd, ram)
  NBRD *hdd, *ram;
{
  if (--hdd->total <= 0)
  {
    if (hdd->mode & NBRD_NEWBOARD)	/* 新板連署掛 END */
      hdd->mode |= NBRD_END;
    else				/* 其他連署掛 FINISH */
      hdd->mode |= NBRD_FINISH;
  }
}


static int
nbrd_reply(xo)
  XO *xo;
{
  NBRD *nbrd;
  char *fname, fpath[64], reason[80];
  LOG mail;

  nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);
  fname = NULL;

  if (nbrd->mode & (NBRD_FINISH | NBRD_END))
    return XO_NONE;

#ifdef SYSOP_START_COSIGN
  if (!(nbrd->mode & NBRD_START))
  {
    /* 尚未開始連署 */
    vmsg("\xA9\x7C\xA5\xBC\xB6\x7D\xA9\x6C\xB3\x73\xB8\x70");
    return XO_FOOT;
  }
#endif

  if (time(0) >= nbrd->etime)
  {
    currchrono = nbrd->btime;
    if (nbrd->mode & NBRD_NEWBOARD)	/* 新板連署掛 END */
    {
      if (!(nbrd->mode & NBRD_END))
      {
	nbrd->mode ^= NBRD_END;
	currchrono = nbrd->btime;
	rec_put(xo->dir, nbrd, sizeof(NBRD), xo->pos, cmpbtime);
      }
    }
    else				/* 其他連署掛 FINISH */
    {
      if (!(nbrd->mode & NBRD_FINISH))
      {
	nbrd->mode ^= NBRD_FINISH;
	currchrono = nbrd->btime;
	rec_put(xo->dir, nbrd, sizeof(NBRD), xo->pos, cmpbtime);
      }
    }
    /* 連署已經截止了 */
    vmsg("\xB3\x73\xB8\x70\xA4\x77\xB8\x67\xBA\x49\xA4\xEE\xA4\x46");
    return XO_FOOT;
  }


  /* --------------------------------------------------- */
  /* 檢查是否已經連署過					 */
  /* --------------------------------------------------- */

  nbrd_fpath(fpath, xo->dir, nbrd);
  fname = strrchr(fpath, '@');
  *fname = 'G';

  if (nbrd_seek(fpath))
  {
    /* 您已經連署過了！ */
    vmsg("\xB1\x7A\xA4\x77\xB8\x67\xB3\x73\xB8\x70\xB9\x4C\xA4\x46\xA1\x49");
    return XO_FOOT;
  }

  /* --------------------------------------------------- */
  /* 開始連署						 */
  /* --------------------------------------------------- */

  *fname = '@';

  /* 要加入連署嗎(Y/N)？[N]  */
  if (vans("\xAD\x6E\xA5\x5B\xA4\x4A\xB3\x73\xB8\x70\xB6\xDC(Y/N)\xA1\x48[N] ") == 'y' && 
    /* 我有話要說： */
    vget(b_lines, 0, "\xA7\xDA\xA6\xB3\xB8\xDC\xAD\x6E\xBB\xA1\xA1\x47", reason, 65, DOECHO))
  {
    FILE *fp;

    if (fp = fopen(fpath, "a"))
    {
      if (nbrd->mode & NBRD_ANONYMOUS)
	fprintf(fp, "%3d -> " STR_ANONYMOUS "\n    %s\n", nbrd->total, reason);
      else
	fprintf(fp, "%3d -> %s (%s)\n    %s\n", nbrd->total, cuser.userid, cuser.email, reason);
      fclose(fp);
    }

    currchrono = nbrd->btime;
    rec_ref(xo->dir, nbrd, sizeof(NBRD), xo->pos, cmpbtime, addreply);

    memset(&mail, 0, sizeof(LOG));
    strcpy(mail.userid, cuser.userid);
    strcpy(mail.email, cuser.email);
    *fname = 'G';
    rec_add(fpath, &mail, sizeof(LOG));

    /* 加入連署完成 */
    vmsg("\xA5\x5B\xA4\x4A\xB3\x73\xB8\x70\xA7\xB9\xA6\xA8");
    return nbrd_init(xo);
  }

  return XO_FOOT;
}


#ifdef SYSOP_START_COSIGN
static int
nbrd_start(xo)
  XO *xo;
{
  NBRD *nbrd;
  char fpath[64], buf[80], tmp[10];
  time_t etime;

  if (!HAS_PERM(PERM_ALLBOARD))
    return XO_NONE;

  nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);

  if (nbrd->mode & (NBRD_FINISH | NBRD_END | NBRD_START))
    return XO_NONE;

  /* 請確定開始連署(Y/N)？[N]  */
  if (vans("\xBD\xD0\xBD\x54\xA9\x77\xB6\x7D\xA9\x6C\xB3\x73\xB8\x70(Y/N)\xA1\x48[N] ") != 'y')
    return XO_FOOT;

  nbrd_fpath(fpath, xo->dir, nbrd);
  etime = time(0) + NBRD_DAY_BRD * 86400;

  str_stamp(tmp, etime);
  /* 開始連署：      到期日期：%s\n */
  sprintf(buf, "\xB6\x7D\xA9\x6C\xB3\x73\xB8\x70\xA1\x47      \xA8\xEC\xB4\xC1\xA4\xE9\xB4\xC1\xA1\x47%s\n", tmp);
  f_cat(fpath, buf);
  f_cat(fpath, split_line);

  nbrd->etime = etime;
  nbrd->mode ^= NBRD_START;
  currchrono = nbrd->btime;
  rec_put(xo->dir, nbrd, sizeof(NBRD), xo->pos, cmpbtime);

  return nbrd_head(xo);
}
#endif


static int
nbrd_finish(xo)
  XO *xo;
{
  NBRD *nbrd;
  char fpath[64], path[64];
  int fd;
  FILE *fp;

  if (!HAS_PERM(PERM_ALLBOARD))
    return XO_NONE;

  nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);

  if (nbrd->mode & NBRD_FINISH)
    return XO_NONE;

  /* 請確定結束連署(Y/N)？[N]  */
  if (vans("\xBD\xD0\xBD\x54\xA9\x77\xB5\xB2\xA7\xF4\xB3\x73\xB8\x70(Y/N)\xA1\x48[N] ") != 'y')
    return XO_FOOT;

  /* 請編輯結束連署原因 */
  vmsg("\xBD\xD0\xBD\x73\xBF\xE8\xB5\xB2\xA7\xF4\xB3\x73\xB8\x70\xAD\xEC\xA6\x5D");
  sprintf(path, "tmp/%s", cuser.userid);	/* 連署原因的暫存檔案 */
  if (fd = vedit(path, 0))
  {
    unlink(path);
    vmsg(msg_cancel);
    return nbrd_head(xo);
  }

  nbrd_fpath(fpath, xo->dir, nbrd);

  /* 結束連署原因：\n\n */
  f_cat(fpath, "\xB5\xB2\xA7\xF4\xB3\x73\xB8\x70\xAD\xEC\xA6\x5D\xA1\x47\n\n");
  fp = fopen(fpath, "a");
  f_suck(fp, path);
  fclose(fp);
  f_cat(fpath, split_line);
  unlink(path);

  nbrd->mode ^= NBRD_FINISH;
  currchrono = nbrd->btime;
  rec_put(xo->dir, nbrd, sizeof(NBRD), xo->pos, cmpbtime);

  return nbrd_head(xo);
}


static int			/* 1:開板成功 */
nbrd_newbrd(nbrd)		/* 開新板 */
  NBRD *nbrd;
{
  BRD newboard;
  ACCT acct;

  /* itoc.030519: 避免重覆開板 */
  if (brd_bno(nbrd->brdname) >= 0)
  {
    /* 已有此板 */
    vmsg("\xA4\x77\xA6\xB3\xA6\xB9\xAA\x4F");
    return 1;
  }

  memset(&newboard, 0, sizeof(BRD));

  /* itoc.010805: 新看板預設 battr = 不轉信; postlevel = PERM_POST; 看板板主為提起連署者 */
  newboard.battr = BRD_NOTRAN | BRD_NOCHANGE;
  newboard.postlevel = PERM_POST;
  strcpy(newboard.brdname, nbrd->brdname);
  strcpy(newboard.class, nbrd->class);
  strcpy(newboard.title, nbrd->title);
  strcpy(newboard.BM, nbrd->owner);

  if (acct_load(&acct, nbrd->owner) >= 0)
    acct_setperm(&acct, PERM_BM, 0);

  if (brd_new(&newboard) < 0)
    return 0;

  /* 新板成立，記著加入分類群組 */
  vmsg("\xB7\x73\xAA\x4F\xA6\xA8\xA5\xDF\xA1\x41\xB0\x4F\xB5\xDB\xA5\x5B\xA4\x4A\xA4\xC0\xC3\xFE\xB8\x73\xB2\xD5");
  return 1;
}


static int
nbrd_open(xo)		/* itoc.010805: 開新板連署，連署完畢開新看板 */
  XO *xo;
{
  NBRD *nbrd;

  if (!HAS_PERM(PERM_ALLBOARD))
    return XO_NONE;

  nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);

  if (nbrd->mode & NBRD_FINISH || !(nbrd->mode & NBRD_NEWBOARD))
    return XO_NONE;

  /* 請確定開啟看板(Y/N)？[N]  */
  if (vans("\xBD\xD0\xBD\x54\xA9\x77\xB6\x7D\xB1\xD2\xAC\xDD\xAA\x4F(Y/N)\xA1\x48[N] ") == 'y')
  {
    if (nbrd_newbrd(nbrd))
    {
      nbrd->mode ^= NBRD_FINISH;
      currchrono = nbrd->btime;
      rec_put(xo->dir, nbrd, sizeof(NBRD), xo->pos, cmpbtime);
    }
    return nbrd_head(xo);
  }

  return XO_FOOT;
}


static int
nbrd_browse(xo)
  XO *xo;
{
  int key;
  NBRD *nbrd;
  char fpath[80];

  /* itoc.010304: 為了讓閱讀到一半也可以加入連署，考慮 more 傳回值 */
  for (;;)
  {
    nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);
    nbrd_fpath(fpath, xo->dir, nbrd);

    if ((key = more(fpath, FOOTER_COSIGN)) < 0)
      break;

    if (!key)
      key = vkey();

    switch (key)
    {
    case KEY_UP:
    case KEY_PGUP:
    case '[':
    case 'k':
      key = xo->pos - 1;

      if (key < 0)
        break;

      xo->pos = key;

      if (key <= xo->top)
      {
	xo->top = (key / XO_TALL) * XO_TALL;
	nbrd_load(xo);
      }
      continue;

    case KEY_DOWN:
    case KEY_PGDN:
    case ']':
    case 'j':
    case ' ':
      key = xo->pos + 1;

      if (key >= xo->max)
        break;

      xo->pos = key;

      if (key >= xo->top + XO_TALL)
      {
	xo->top = (key / XO_TALL) * XO_TALL;
	nbrd_load(xo);
      }
      continue;

    case 'y':
    case 'r':
      nbrd_reply(xo);
      break;

    case 'h':
      xo_help("cosign");
      break;
    }
    break;
  }

  return nbrd_head(xo);
}


static int
nbrd_delete(xo)
  XO *xo;
{
  NBRD *nbrd;
  char *fname, fpath[80];
  char *list = "@G";		/* itoc.註解: 清 newbrd file */

  nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);
  if (strcmp(cuser.userid, nbrd->owner) && !HAS_PERM(PERM_ALLBOARD))
    return XO_NONE;

  if (vans(msg_del_ny) != 'y')
    return XO_FOOT;

  nbrd_fpath(fpath, xo->dir, nbrd);
  fname = strrchr(fpath, '@');
  while (*fname = *list++)
  {
    unlink(fpath);	/* Thor: 確定名字就砍 */
  }

  currchrono = nbrd->btime;
  rec_del(xo->dir, sizeof(NBRD), xo->pos, cmpbtime);
  return nbrd_init(xo);
}


static int
nbrd_edit(xo)
  XO *xo;
{
  if (HAS_PERM(PERM_ALLBOARD))
  {
    char fpath[64];
    NBRD *nbrd;

    nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);
    nbrd_fpath(fpath, xo->dir, nbrd);
    vedit(fpath, 0);
    return nbrd_head(xo);
  }

  return XO_NONE;
}


static int
nbrd_setup(xo)
  XO *xo;
{
  int numbers;
  char ans[6];
  NBRD *nbrd, newnh;

  if (!HAS_PERM(PERM_ALLBOARD))
    return XO_NONE;

  /* 連署設定 */
  vs_bar("\xB3\x73\xB8\x70\xB3\x5D\xA9\x77");
  nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);
  memcpy(&newnh, nbrd, sizeof(NBRD));

  /* 看板名稱：%s\n看板說明：%4.4s %s\n連署發起：%s\n */
  prints("\xAC\xDD\xAA\x4F\xA6\x57\xBA\xD9\xA1\x47%s\n\xAC\xDD\xAA\x4F\xBB\xA1\xA9\xFA\xA1\x47%4.4s %s\n\xB3\x73\xB8\x70\xB5\x6F\xB0\x5F\xA1\x47%s\n",
    newnh.brdname, newnh.class, newnh.title, newnh.owner);
  /* 開始時間：%s\n */
  prints("\xB6\x7D\xA9\x6C\xAE\xC9\xB6\xA1\xA1\x47%s\n", Btime(newnh.btime));
  /* 結束時間：%s\n */
  prints("\xB5\xB2\xA7\xF4\xAE\xC9\xB6\xA1\xA1\x47%s\n", Btime(newnh.etime));
  /* 還需人數：%d\n */
  prints("\xC1\xD9\xBB\xDD\xA4\x48\xBC\xC6\xA1\x47%d\n", newnh.total);

  /* (E)設定 (Q)取消？[Q]  */
  if (vget(8, 0, "(E)\xB3\x5D\xA9\x77 (Q)\xA8\xFA\xAE\xF8\xA1\x48[Q] ", ans, 3, LCECHO) == 'e')
  {
    vget(11, 0, MSG_BID, newnh.brdname, BNLEN + 1, GCARRY);
    /* 看板分類： */
    vget(12, 0, "\xAC\xDD\xAA\x4F\xA4\xC0\xC3\xFE\xA1\x47", newnh.class, sizeof(newnh.class), GCARRY);
    /* 看板主題： */
    vget(13, 0, "\xAC\xDD\xAA\x4F\xA5\x44\xC3\x44\xA1\x47", newnh.title, sizeof(newnh.title), GCARRY);
    sprintf(ans, "%d", newnh.total);
    /* 連署人數： */
    vget(14, 0, "\xB3\x73\xB8\x70\xA4\x48\xBC\xC6\xA1\x47", ans, 6, GCARRY);
    numbers = atoi(ans);
    if (numbers <= 500 && numbers >= 1)
      newnh.total = numbers;

    if (memcmp(&newnh, nbrd, sizeof(newnh)) && vans(msg_sure_ny) == 'y')
    {
      memcpy(nbrd, &newnh, sizeof(NBRD));
      currchrono = nbrd->btime;
      rec_put(xo->dir, nbrd, sizeof(NBRD), xo->pos, cmpbtime);
    }
  }

  return nbrd_head(xo);
}


static int
nbrd_uquery(xo)
  XO *xo;
{
  NBRD *nbrd;

  nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);

  move(1, 0);
  clrtobot();
  my_query(nbrd->owner);
  return nbrd_head(xo);
}


static int
nbrd_usetup(xo)
  XO *xo;
{
  NBRD *nbrd;
  ACCT acct;

  if (!HAS_PERM(PERM_ALLACCT))
    return XO_NONE;

  nbrd = (NBRD *) xo_pool + (xo->pos - xo->top);
  if (acct_load(&acct, nbrd->owner) < 0)
    return XO_NONE;

  move(3, 0);
  acct_setup(&acct, 1);
  return nbrd_head(xo);
}


static int
nbrd_help(xo)
  XO *xo;
{
  xo_help("cosign");
  return nbrd_head(xo);
}


static KeyFunc nbrd_cb[] =
{
  XO_INIT, nbrd_init,
  XO_LOAD, nbrd_load,
  XO_HEAD, nbrd_head,
  XO_BODY, nbrd_body,

  'y', nbrd_reply,
  'r', nbrd_browse,
  'o', nbrd_open,
#ifdef SYSOP_START_COSIGN
  's', nbrd_start,
#endif
  'c', nbrd_finish,
  'd', nbrd_delete,
  'E', nbrd_edit,
  'B', nbrd_setup,

  Ctrl('P'), nbrd_add,
  Ctrl('Q'), nbrd_uquery,
  Ctrl('O'), nbrd_usetup,

  'h', nbrd_help
};


int
XoNewBoard()
{
  XO *xo;
  char fpath[64];

  sprintf(fpath, "run/newbrd/%s", fn_dir);
  xz[XZ_COSIGN - XO_ZONE].xo = xo = xo_new(fpath);
  xz[XZ_COSIGN - XO_ZONE].cb = nbrd_cb;
  xo->key = XZ_COSIGN;
  xover(XZ_COSIGN);
  free(xo);

  return 0;
}
#endif	/* HAVE_COSIGN */
