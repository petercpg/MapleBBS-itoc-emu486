/*-------------------------------------------------------*/
/* so/vote.c            ( NTHU CS MapleBBS Ver 2.36 )    */
/*-------------------------------------------------------*/
/* target : boards' vote routines		 	 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/
/* brd/_/.VCH  : Vote Control Header    目前所有投票索引 */
/* brd/_/@vote : vote history           過去的投票歷史	 */
/* brd/_/@/@_  : vote description       投票說明	 */
/* brd/_/@/I_  : vote selection Items   投票選項	 */
/* brd/_/@/O_  : users' Opinions        使用者有話要說	 */
/* brd/_/@/L_  : can vote List          可投票名單	 */
/* brd/_/@/G_  : voted id loG file      已領票名單	 */
/* brd/_/@/Z_  : final/temporary result	投票結果	 */
/*-------------------------------------------------------*/


#include "bbs.h"


extern BCACHE *bshm;
extern XZ xz[];
extern char xo_pool[];


static char *
vch_fpath(fpath, folder, vch)
  char *fpath, *folder;
  VCH *vch;
{
  /* VCH 和 HDR 的 xname 欄位匹配，所以直接借用 hdr_fpath() */
  hdr_fpath(fpath, folder, (HDR *) vch);
  return strrchr(fpath, '@');
}


static int vote_add();


int
vote_result(xo)
  XO *xo;
{
  char fpath[64];

  setdirpath(fpath, xo->dir, "@/@vote");
  /* Thor.990204: 為考慮more 傳回值 */   
  if (more(fpath, NULL) >= 0)
    return XO_HEAD;	/* XZ_POST 和 XZ_VOTE 共用 vote_result() */

  /* 目前沒有任何開票的結果 */
  vmsg("\xA5\xD8\xAB\x65\xA8\x53\xA6\xB3\xA5\xF4\xA6\xF3\xB6\x7D\xB2\xBC\xAA\xBA\xB5\xB2\xAA\x47");
  return XO_FOOT;
}


static void
vote_item(num, vch)
  int num;
  VCH *vch;
{
  prints("%6d%c%c%c%c%c %-9.8s%-12s %.44s\n",
    num, tag_char(vch->chrono), vch->vgamble, vch->vsort, vch->vpercent, vch->vprivate, 
    vch->cdate, vch->owner, vch->title);
}


static int
vote_body(xo)
  XO *xo;
{
  VCH *vch;
  int num, max, tail;

  max = xo->max;
  if (max <= 0)
  {
    if (bbstate & STAT_BOARD)
    {
      /* 要舉辦投票嗎(Y/N)？[N]  */
      if (vans("\xAD\x6E\xC1\x7C\xBF\xEC\xA7\xEB\xB2\xBC\xB6\xDC(Y/N)\xA1\x48[N] ") == 'y')
	return vote_add(xo);
    }
    else
    {
      /* 目前並無投票舉行 */
      vmsg("\xA5\xD8\xAB\x65\xA8\xC3\xB5\x4C\xA7\xEB\xB2\xBC\xC1\x7C\xA6\xE6");
    }
    return XO_QUIT;
  }

  vch = (VCH *) xo_pool;
  num = xo->top;
  tail = num + XO_TALL;
  if (max > tail)
    max = tail;

  move(3, 0);
  do
  {
    vote_item(++num, vch++);
  } while (num < max);
  clrtobot();

  /* return XO_NONE; */
  return XO_FOOT;	/* itoc.010403: 把 b_lines 填上 feeter */
}


static int
vote_head(xo)
  XO *xo;
{
  /* 投票所 */
  vs_head(currBM, "\xA7\xEB\xB2\xBC\xA9\xD2");
  prints(NECKER_VOTE, d_cols, "");
  return vote_body(xo);
}


static int
vote_init(xo)
  XO *xo;
{
  xo_load(xo, sizeof(VCH));
  return vote_head(xo);
}


static int
vote_load(xo)
  XO *xo;
{
  xo_load(xo, sizeof(VCH));
  return vote_body(xo);
}


static void
vch_edit(vch, item, echo)
  VCH *vch;
  int item;		/* vlist 有幾項 */
  int echo;
{
  int num, row;
  char ans[8], buf[80];

  clear();

  row = 3;

  if (echo == DOECHO)	/* 只有新增時才能決定是否為賭盤 */
    /* 是否為賭盤(Y/N)？[N]  */
    vch->vgamble = (vget(++row, 0, "\xAC\x4F\xA7\x5F\xAC\xB0\xBD\xE4\xBD\x4C(Y/N)\xA1\x48[N] ", ans, 3, LCECHO) == 'y') ? '$' : ' ';

  if (vch->vgamble == ' ')
  {
    /* 請問每人最多可投幾票？([1]～%d)： */
    sprintf(buf, "\xBD\xD0\xB0\xDD\xA8\x43\xA4\x48\xB3\xCC\xA6\x68\xA5\x69\xA7\xEB\xB4\x58\xB2\xBC\xA1\x48([1]\xA1\xE3%d)\xA1\x47", item);
    vget(++row, 0, buf, ans, 3, DOECHO);
    num = atoi(ans);
    if (num < 1)
      num = 1;
    else if (num > item)
      num = item;
    vch->maxblt = num;
  }
  else if (echo == DOECHO)	/* 只有新增時才能改變賭盤的票價 */
  {
    /* 賭盤就只能選一項 */
    vch->maxblt = 1;

    /* 請問每票售價多少銀幣？(100～100000)： */
    vget(++row, 0, "\xBD\xD0\xB0\xDD\xA8\x43\xB2\xBC\xB0\xE2\xBB\xF9\xA6\x68\xA4\xD6\xBB\xC8\xB9\xF4\xA1\x48(100\xA1\xE3""100000)\xA1\x47", ans, 7, DOECHO);
    num = atoi(ans);
    if (num < 100)
      num = 100;
    else if (num > 100000)
      num = 100000;
    vch->price = num;
  }

  /* 本項投票進行幾小時 (至少一小時)？[1]  */
  vget(++row, 0, "\xA5\xBB\xB6\xB5\xA7\xEB\xB2\xBC\xB6\x69\xA6\xE6\xB4\x58\xA4\x70\xAE\xC9 (\xA6\xDC\xA4\xD6\xA4\x40\xA4\x70\xAE\xC9)\xA1\x48[1] ", ans, 5, DOECHO);
  num = atoi(ans);
  if (num < 1)
    num = 1;
  vch->vclose = vch->chrono + num * 3600;
  str_stamp(vch->cdate, vch->vclose);

  if (vch->vgamble == ' ')	/* 賭盤一定排序、及顯示百分比 */
  {
    /* 開票結果是否排序(Y/N)？[N]  */
    vch->vsort = (vget(++row, 0, "\xB6\x7D\xB2\xBC\xB5\xB2\xAA\x47\xAC\x4F\xA7\x5F\xB1\xC6\xA7\xC7(Y/N)\xA1\x48[N] ", ans, 3, LCECHO) == 'y') ? 's' : ' ';
    /* 開票結果是否顯示百分比例(Y/N)？[N]  */
    vch->vpercent = (vget(++row, 0, "\xB6\x7D\xB2\xBC\xB5\xB2\xAA\x47\xAC\x4F\xA7\x5F\xC5\xE3\xA5\xDC\xA6\xCA\xA4\xC0\xA4\xF1\xA8\xD2(Y/N)\xA1\x48[N] ", ans, 3, LCECHO) == 'y') ? '%' : ' ';
  }
  else
  {
    vch->vsort = 's';
    vch->vpercent = '%';
  }

  /* 是否限制投票名單(Y/N)？[N]  */
  vch->vprivate = (vget(++row, 0, "\xAC\x4F\xA7\x5F\xAD\xAD\xA8\xEE\xA7\xEB\xB2\xBC\xA6\x57\xB3\xE6(Y/N)\xA1\x48[N] ", ans, 3, LCECHO) == 'y') ? ')' : ' ';

  /* 是否限制投票資格(Y/N)？[N]  */
  if (vch->vprivate == ' ' && vget(++row, 0, "\xAC\x4F\xA7\x5F\xAD\xAD\xA8\xEE\xA7\xEB\xB2\xBC\xB8\xEA\xAE\xE6(Y/N)\xA1\x48[N] ", ans, 3, LCECHO) == 'y')
  {
    /* 請問要登入幾次以上才可以參加本次投票？([0]～9999)： */
    vget(++row, 0, "\xBD\xD0\xB0\xDD\xAD\x6E\xB5\x6E\xA4\x4A\xB4\x58\xA6\xB8\xA5\x48\xA4\x57\xA4\x7E\xA5\x69\xA5\x48\xB0\xD1\xA5\x5B\xA5\xBB\xA6\xB8\xA7\xEB\xB2\xBC\xA1\x48([0]\xA1\xE3""9999)\xA1\x47", ans, 5, DOECHO);
    num = atoi(ans);
    if (num < 0)
      num = 0;
    vch->limitlogins = num;

    /* 請問要發文幾次以上才可以參加本次投票？([0]～9999)： */
    vget(++row, 0, "\xBD\xD0\xB0\xDD\xAD\x6E\xB5\x6F\xA4\xE5\xB4\x58\xA6\xB8\xA5\x48\xA4\x57\xA4\x7E\xA5\x69\xA5\x48\xB0\xD1\xA5\x5B\xA5\xBB\xA6\xB8\xA7\xEB\xB2\xBC\xA1\x48([0]\xA1\xE3""9999)\xA1\x47", ans, 5, DOECHO);
    num = atoi(ans);
    if (num < 0)
      num = 0;
    vch->limitposts = num;
  }
}


static int
vlist_edit(vlist)
  vitem_t vlist[];
{
  int item;
  char buf[80];

  clear();

  /* 請依序輸入選項 (最多 32 項)，按 ENTER 結束： */
  outs("\xBD\xD0\xA8\xCC\xA7\xC7\xBF\xE9\xA4\x4A\xBF\xEF\xB6\xB5 (\xB3\xCC\xA6\x68 32 \xB6\xB5)\xA1\x41\xAB\xF6 ENTER \xB5\xB2\xA7\xF4\xA1\x47");

  strcpy(buf, " ) ");
  for (;;)
  {
    item = 0;
    for (;;)
    {
      buf[0] = radix32[item];
      if (!vget((item & 15) + 3, (item / 16) * 40, buf, vlist[item], sizeof(vitem_t), GCARRY) || 
        (++item >= MAX_CHOICES))
	break;
    }
    /* 是否重新輸入選項(Y/N)？[N]  */
    if (item && vans("\xAC\x4F\xA7\x5F\xAD\xAB\xB7\x73\xBF\xE9\xA4\x4A\xBF\xEF\xB6\xB5(Y/N)\xA1\x48[N] ") != 'y')
      break;
  }
  return item;
}


static int
vlog_seek(fpath)
  char *fpath;
{
  VLOG old;
  int fd;
  int rc = 0;

  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    while (read(fd, &old, sizeof(VLOG)) == sizeof(VLOG))
    {
      if (!strcmp(old.userid, cuser.userid))
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
vote_add(xo)
  XO *xo;
{
  VCH vch;
  int fd, item;
  char *dir, *str, fpath[64], title[TTLEN + 1];
  vitem_t vlist[MAX_CHOICES];
  BRD *brd;

  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  /* 標題： */
  if (!vget(b_lines, 0, "\xBC\xD0\xC3\x44\xA1\x47", title, TTLEN + 1, DOECHO))
    return xo->max ? XO_FOOT : vote_body(xo);	/* itoc.011125: 如果沒有任何投票，要回到 vote_body() */
    /* return XO_FOOT; */

  dir = xo->dir;
  if ((fd = hdr_stamp(dir, 0, (HDR *) &vch, fpath)) < 0)
  {
    /* 無法建立投票說明檔 */
    vmsg("\xB5\x4C\xAA\x6B\xAB\xD8\xA5\xDF\xA7\xEB\xB2\xBC\xBB\xA1\xA9\xFA\xC0\xC9");
    return XO_FOOT;
  }

  close(fd);
  /* 開始編輯 [投票說明] */
  vmsg("\xB6\x7D\xA9\x6C\xBD\x73\xBF\xE8 [\xA7\xEB\xB2\xBC\xBB\xA1\xA9\xFA]");
  fd = vedit(fpath, 0); /* Thor.981020: 注意被talk的問題 */
  if (fd)
  {
    unlink(fpath);
    /* 取消投票 */
    vmsg("\xA8\xFA\xAE\xF8\xA7\xEB\xB2\xBC");
    return vote_head(xo);
  }

  strcpy(vch.title, title);
  str = strrchr(fpath, '@');

  /* --------------------------------------------------- */
  /* 投票選項檔 : Item					 */
  /* --------------------------------------------------- */

  memset(vlist, 0, sizeof(vlist));
  item = vlist_edit(vlist);

  *str = 'I';
  if ((fd = open(fpath, O_WRONLY | O_CREAT | O_TRUNC, 0600)) < 0)
  {
    /* 無法建立投票選項檔 */
    vmsg("\xB5\x4C\xAA\x6B\xAB\xD8\xA5\xDF\xA7\xEB\xB2\xBC\xBF\xEF\xB6\xB5\xC0\xC9");
    return vote_head(xo);
  }
  write(fd, vlist, item * sizeof(vitem_t));
  close(fd);

  vch_edit(&vch, item, DOECHO);

  strcpy(vch.owner, cuser.userid);

  brd = bshm->bcache + currbno;

  brd->bvote++;
  if (brd->bvote >= 0)
    brd->bvote = (vch.vgamble == '$') ? -1 : 1;
  vch.bstamp = brd->bstamp;

  rec_add(dir, &vch, sizeof(VCH));

  /* 開始投票了！ */
  vmsg("\xB6\x7D\xA9\x6C\xA7\xEB\xB2\xBC\xA4\x46\xA1\x49");
  return vote_init(xo);
}


static int
vote_edit(xo)
  XO *xo;
{
  int pos;
  VCH *vch, vxx;
  char *dir, fpath[64];

  /* Thor: for 修改投票選項 */
  int fd, item;
  vitem_t vlist[MAX_CHOICES];
  char *fname;

  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  pos = xo->pos;
  dir = xo->dir;
  vch = (VCH *) xo_pool + (pos - xo->top);

  /* Thor: 修改投票主題 */

  vxx = *vch;

  /* 標題： */
  if (!vget(b_lines, 0, "\xBC\xD0\xC3\x44\xA1\x47", vxx.title, TTLEN + 1, GCARRY))
    return XO_FOOT;

  fname = vch_fpath(fpath, dir, vch);
  vedit(fpath, 0);	/* Thor.981020: 注意被talk的問題  */

  /* Thor: 修改投票選項 */

  memset(vlist, 0, sizeof(vlist));
  *fname = 'I';
  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    read(fd, vlist, sizeof(vlist));
    close(fd);
  }

  item = vlist_edit(vlist);

  if ((fd = open(fpath, O_WRONLY | O_CREAT | O_TRUNC, 0600)) < 0)
  {
    /* 無法建立投票選項檔 */
    vmsg("\xB5\x4C\xAA\x6B\xAB\xD8\xA5\xDF\xA7\xEB\xB2\xBC\xBF\xEF\xB6\xB5\xC0\xC9");
    return vote_head(xo);
  }
  write(fd, vlist, item * sizeof(vitem_t));
  close(fd);

  vch_edit(&vxx, item, GCARRY);

  if (memcmp(&vxx, vch, sizeof(VCH)))
  {
    /* 確定要修改這項投票嗎(Y/N)？[N]  */
    if (vans("\xBD\x54\xA9\x77\xAD\x6E\xAD\xD7\xA7\xEF\xB3\x6F\xB6\xB5\xA7\xEB\xB2\xBC\xB6\xDC(Y/N)\xA1\x48[N] ") == 'y')
    {
      *vch = vxx;
      currchrono = vch->chrono;
      rec_put(dir, vch, sizeof(VCH), pos, cmpchrono);
    }
  }

  return vote_head(xo);
}


static int
vote_query(xo)
  XO *xo;
{
  char *dir, *fname, fpath[64], buf[80];
  VCH *vch;
  int cc, pos;

  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  pos = xo->pos;
  dir = xo->dir;
  vch = (VCH *) xo_pool + (pos - xo->top);

  fname = vch_fpath(fpath, dir, vch);
  more(fpath, (char *) -1);

  *fname = 'G';
  /* 共有 %d 人參加投票，確定要將開票時間改期(Y/N)？[N]  */
  sprintf(buf, "\xA6\x40\xA6\xB3 %d \xA4\x48\xB0\xD1\xA5\x5B\xA7\xEB\xB2\xBC\xA1\x41\xBD\x54\xA9\x77\xAD\x6E\xB1\x4E\xB6\x7D\xB2\xBC\xAE\xC9\xB6\xA1\xA7\xEF\xB4\xC1(Y/N)\xA1\x48[N] ", rec_num(fpath, sizeof(VLOG)));
  if (vans(buf) == 'y')
  {
    /* 請更改開票時間(-n提前n小時/+m延後m小時/0不改)： */
    vget(b_lines, 0, "\xBD\xD0\xA7\xF3\xA7\xEF\xB6\x7D\xB2\xBC\xAE\xC9\xB6\xA1(-n\xB4\xA3\xAB\x65n\xA4\x70\xAE\xC9/+m\xA9\xB5\xAB\xE1m\xA4\x70\xAE\xC9/0\xA4\xA3\xA7\xEF)\xA1\x47", buf, 5, DOECHO);
    if (cc = atoi(buf))
    {
      vch->vclose = vch->vclose + cc * 3600;
      str_stamp(vch->cdate, vch->vclose);
      currchrono = vch->chrono;
      rec_put(dir, vch, sizeof(VCH), pos, cmpchrono);
    }
  }

  return vote_head(xo); 
}


static int
vfyvch(vch, pos)
  VCH *vch;
  int pos;
{
  return Tagger(vch->chrono, pos, TAG_NIN);
}


static void
delvch(xo, vch)
  XO *xo;
  VCH *vch;
{
  int fd;
  char fpath[64], buf[64], *fname;
  char *list = "@IOLGZ";	/* itoc.註解: 清 vote file */
  VLOG vlog;
  PAYCHECK paycheck;

  fname = vch_fpath(fpath, xo->dir, vch);

  if (vch->vgamble == '$')	/* itoc.050313: 如果是賭盤被刪除，那麼要退賭金 */
  {
    *fname = 'G';

    if ((fd = open(fpath, O_RDONLY)) >= 0)
    {
      memset(&paycheck, 0, sizeof(PAYCHECK));
      time(&paycheck.tissue);
      /* [退款] %s */
      sprintf(paycheck.reason, "[\xB0\x68\xB4\xDA] %s", currboard);

      while (read(fd, &vlog, sizeof(VLOG)) == sizeof(VLOG))
      {
	paycheck.money = vlog.numvotes * vch->price;
	usr_fpath(buf, vlog.userid, FN_PAYCHECK);
	rec_add(buf, &paycheck, sizeof(PAYCHECK));
      }
    }
    close(fd);
  }

  while (*fname = *list++)
    unlink(fpath); /* Thor: 確定名字就砍 */
}



static int
vote_delete(xo)
  XO *xo;
{
  int pos;
  VCH *vch;

  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  pos = xo->pos;
  vch = (VCH *) xo_pool + (pos - xo->top);

  if (vans(msg_del_ny) == 'y')
  {
    delvch(xo, vch);

    currchrono = vch->chrono;
    rec_del(xo->dir, sizeof(VCH), pos, cmpchrono);    
    return vote_load(xo);
  }

  return XO_FOOT;
}


static int
vote_rangedel(xo)
  XO *xo;
{
  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  return xo_rangedel(xo, sizeof(VCH), NULL, delvch);
}


static int
vote_prune(xo)
  XO *xo;
{
  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  return xo_prune(xo, sizeof(VCH), vfyvch, delvch);
}


static int
vote_pal(xo)		/* itoc.020117: 編輯限制投票名單 */
  XO *xo;
{
  char *fname, fpath[64];
  VCH *vch;
  XO *xt;

  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  vch = (VCH *) xo_pool + (xo->pos - xo->top);

  if (vch->vprivate != ')')
    return XO_NONE;

  fname = vch_fpath(fpath, xo->dir, vch);
  *fname = 'L';

  xz[XZ_PAL - XO_ZONE].xo = xt = xo_new(fpath);
  xt->key = PALTYPE_VOTE;
  xover(XZ_PAL);		/* Thor: 進xover前, pal_xo 一定要 ready */

  free(xt);
  return vote_init(xo);
}


static int
vote_join(xo)
  XO *xo;
{
  VCH *vch, vbuf;
  VLOG vlog;
  int count, fd;
  usint choice;
  char *dir, *fname, fpath[64], buf[80], ans[4], *slist[MAX_CHOICES];
  vitem_t vlist[MAX_CHOICES];

  if (HAS_STATUS(STATUS_COINLOCK))
  {
    vmsg(msg_coinlock);
    return XO_FOOT;
  }

  vch = (VCH *) xo_pool + (xo->pos - xo->top);

  /* --------------------------------------------------- */
  /* 檢查是否已經結束投票				 */
  /* --------------------------------------------------- */

  if (time(0) > vch->vclose)
  {
    /* 投票已經截止了，請靜候開票 */
    vmsg("\xA7\xEB\xB2\xBC\xA4\x77\xB8\x67\xBA\x49\xA4\xEE\xA4\x46\xA1\x41\xBD\xD0\xC0\x52\xAD\xD4\xB6\x7D\xB2\xBC");
    return XO_FOOT;
  }

  /* --------------------------------------------------- */
  /* 檢查是否有足夠錢					 */
  /* --------------------------------------------------- */

  if (vch->vgamble == '$')
  {
    if (cuser.money < vch->price)
    {
      /* 您的錢不夠參加賭盤 */
      vmsg("\xB1\x7A\xAA\xBA\xBF\xFA\xA4\xA3\xB0\xF7\xB0\xD1\xA5\x5B\xBD\xE4\xBD\x4C");
      return XO_FOOT;
    }
  }

  /* --------------------------------------------------- */
  /* 投票檔案						 */
  /* --------------------------------------------------- */

  dir = xo->dir;
  fname = vch_fpath(fpath, dir, vch);

  /* --------------------------------------------------- */
  /* 檢查是否已經投過票					 */
  /* --------------------------------------------------- */

  if (vch->vgamble == ' ')	/* itoc.031101: 賭盤可以一直下注 */
  {
    *fname = 'G';
    if (vlog_seek(fpath))
    {
      /* 您已經投過票了！ */
      vmsg("\xB1\x7A\xA4\x77\xB8\x67\xA7\xEB\xB9\x4C\xB2\xBC\xA4\x46\xA1\x49");
      return XO_FOOT;
    }
  }

  /* --------------------------------------------------- */
  /* 檢查投票限制					 */
  /* --------------------------------------------------- */

  if (vch->vprivate == ' ')
  {
    if (cuser.numlogins < vch->limitlogins || cuser.numposts < vch->limitposts)
    {
      /* 您不夠資深喔！ */
      vmsg("\xB1\x7A\xA4\xA3\xB0\xF7\xB8\xEA\xB2\x60\xB3\xE1\xA1\x49");
      return XO_FOOT;
    }
  }
  else		/* itoc.020117: 私人投票檢查是否在投票名單中 */
  {
    *fname = 'L';

    if (!pal_find(fpath, cuser.userno) &&
      !(bbstate & STAT_BOARD))		/* 由於並不能把自己加入朋友名單，所以要多檢查是否為板主 */
    {
      /* 您沒有受邀本次私人投票！ */
      vmsg("\xB1\x7A\xA8\x53\xA6\xB3\xA8\xFC\xC1\xDC\xA5\xBB\xA6\xB8\xA8\x70\xA4\x48\xA7\xEB\xB2\xBC\xA1\x49");
      return XO_FOOT;
    }
  }

  /* --------------------------------------------------- */
  /* 確認進入投票					 */
  /* --------------------------------------------------- */

  /* 是否參加投票(Y/N)？[N]  */
  /* 是否參加賭盤(Y/N)？[N]  */
  if (vans(vch->vgamble == ' ' ? "\xAC\x4F\xA7\x5F\xB0\xD1\xA5\x5B\xA7\xEB\xB2\xBC(Y/N)\xA1\x48[N] " : "\xAC\x4F\xA7\x5F\xB0\xD1\xA5\x5B\xBD\xE4\xBD\x4C(Y/N)\xA1\x48[N] ") != 'y')
    return XO_FOOT;

  /* --------------------------------------------------- */
  /* 開始投票，顯示投票說明				 */
  /* --------------------------------------------------- */

  *fname = '@';
  more(fpath, NULL);

  /* --------------------------------------------------- */
  /* 載入投票選項檔					 */
  /* --------------------------------------------------- */

  *fname = 'I';
  if ((fd = open(fpath, O_RDONLY)) < 0)
  {
    /* 無法讀取投票選項檔 */
    vmsg("\xB5\x4C\xAA\x6B\xC5\xAA\xA8\xFA\xA7\xEB\xB2\xBC\xBF\xEF\xB6\xB5\xC0\xC9");
    return vote_head(xo);
  }
  count = read(fd, vlist, sizeof(vlist)) / sizeof(vitem_t);
  close(fd);

  for (fd = 0; fd < count; fd++)
    slist[fd] = (char *) &vlist[fd];

  /* --------------------------------------------------- */
  /* 進行投票						 */
  /* --------------------------------------------------- */

  choice = 0;
  /* 投下神聖的 %d 票 */
  sprintf(buf, "\xA7\xEB\xA4\x55\xAF\xAB\xB8\x74\xAA\xBA %d \xB2\xBC", vch->maxblt); /* Thor: 顯示最多幾票 */
  vs_bar(buf);
  /* 投票主題： */
  outs("\xA7\xEB\xB2\xBC\xA5\x44\xC3\x44\xA1\x47");
  for (;;)
  {
    choice = bitset(choice, count, vch->maxblt, vch->title, slist);

    if (vch->vgamble == ' ')		/* 一般投票才能寫意見 */
      /* 我有話要說： */
      vget(b_lines - 1, 0, "\xA7\xDA\xA6\xB3\xB8\xDC\xAD\x6E\xBB\xA1\xA1\x47", buf, 60, DOECHO);

    /* 投票 (Y)確定 (N)重來 (Q)取消？[N]  */
    fd = vans("\xA7\xEB\xB2\xBC (Y)\xBD\x54\xA9\x77 (N)\xAD\xAB\xA8\xD3 (Q)\xA8\xFA\xAE\xF8\xA1\x48[N] ");

    if (fd == 'q')
      return vote_head(xo);

    if ((fd == 'y') && (vch->vgamble == ' ' || choice))	/* 若是賭盤則一定要選 */
      break;
  }

  /* --------------------------------------------------- */
  /* 記錄結果：一票也未投的情況 ==> 相當於投廢票	 */
  /* --------------------------------------------------- */

  if (vch->vgamble == '$')
  {
    /* 賭盤可以買入多張 */
    for (;;)
    {
      /* 每張賭票 %d 銀幣，請問要買幾張？[1]  */
      sprintf(buf, "\xA8\x43\xB1\x69\xBD\xE4\xB2\xBC %d \xBB\xC8\xB9\xF4\xA1\x41\xBD\xD0\xB0\xDD\xAD\x6E\xB6\x52\xB4\x58\xB1\x69\xA1\x48[1] ", vch->price);
      vget(b_lines, 0, buf, ans, 3, DOECHO);	/* 最多買 99 張，避免溢位 */

      if (time(0) > vch->vclose)	/* 因為有個 vget，所以還要再檢查一次 */
      {
	/* 投票已經截止了，請靜候開票 */
	vmsg("\xA7\xEB\xB2\xBC\xA4\x77\xB8\x67\xBA\x49\xA4\xEE\xA4\x46\xA1\x41\xBD\xD0\xC0\x52\xAD\xD4\xB6\x7D\xB2\xBC");
	return vote_head(xo);
      }

      if ((count = atoi(ans)) < 1)
	count = 1;
      fd = count * vch->price;
      if (cuser.money >= fd)
	break;
    }
  }
  else
  {
    /* 一般投票就是一張票 */
    count = 1;
  }

  /* 確定投票尚未截止 */
  /* itoc.050514: 因為板主可以改變開票時間，為了避免使用者會龜在 vget() 或是
     利用 xo_pool[] 未同步來規避 time(0) > vclose 的檢查，所以就得重新載入 VCH */
  if (rec_get(dir, &vbuf, sizeof(VCH), xo->pos) || vch->chrono != vch->chrono || time(0) > vbuf.vclose)
  {
    /* 投票已經截止了，請靜候開票 */
    vmsg("\xA7\xEB\xB2\xBC\xA4\x77\xB8\x67\xBA\x49\xA4\xEE\xA4\x46\xA1\x41\xBD\xD0\xC0\x52\xAD\xD4\xB6\x7D\xB2\xBC");
    return vote_init(xo);
  }

  if (vch->vgamble == '$')
  {
    cuser.money -= fd;	/* fd 是要付的賭金 */
  }
  else if (*buf)	/* 一般投票才能寫入使用者意見 */
  {
    FILE *fp;

    *fname = 'O';
    if (fp = fopen(fpath, "a"))
    {
      /* ‧%-12s：%s\n */
      fprintf(fp, "\xA1\x45%-12s\xA1\x47%s\n", cuser.userid, buf);
      fclose(fp);
    }
  }

  /* 加入記錄檔 */
  memset(&vlog, 0, sizeof(VLOG));
  strcpy(vlog.userid, cuser.userid);
  vlog.numvotes = count;
  vlog.choice = choice;
  *fname = 'G';
  rec_add(fpath, &vlog, sizeof(VLOG));

  /* 投票完成！ */
  vmsg("\xA7\xEB\xB2\xBC\xA7\xB9\xA6\xA8\xA1\x49");
  return vote_head(xo);
}


struct Tchoice
{
  int count;
  vitem_t vitem;
};


static int
TchoiceCompare(i, j)
  struct Tchoice *i, *j;
{
  return j->count - i->count;
}


static char *			/* NULL:失敗(還沒有人投票) */
draw_vote(fpath, folder, vch, preview)	/* itoc.030906: 投票結果 (與 account.c:draw_vote() 格式相同) */
  char *fpath;
  char *folder;
  VCH *vch;
  int preview;		/* 1:預覽 0:開票 */
{
  struct Tchoice choice[MAX_CHOICES];
  FILE *fp;
  char *fname;
  int total, items, num, fd, ticket, bollt;
  VLOG vlog;

  fname = vch_fpath(fpath, folder, vch);

  /* vote item */

  *fname = 'I';

  items = 0;
  if (fp = fopen(fpath, "r"))
  {
    while (fread(&choice[items].vitem, sizeof(vitem_t), 1, fp) == 1)
    {
      choice[items].count = 0;
      items++;
    }
    fclose(fp);
  }

  if (items == 0)
    return NULL;

  /* 累計投票結果 */

  *fname = 'G';
  bollt = 0;		/* Thor: 總票數歸零 */
  total = 0;

  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    while (read(fd, &vlog, sizeof(VLOG)) == sizeof(VLOG))
    {
      for (ticket = vlog.choice, num = 0; ticket && num < items; ticket >>= 1, num++)
      {
	if (ticket & 1)
	{
	  choice[num].count += vlog.numvotes;
	  bollt += vlog.numvotes;
	}
      }
      total++;
    }
    close(fd);
  }

  /* 產生開票結果 */

  *fname = 'Z';
  if (!(fp = fopen(fpath, "w")))
    return NULL;

  fprintf(fp, "\n\033[1;34m%s\033[m\n\n"
    /* \033[1;32m◆ [%s] 看板投票：%s\033[m\n\n舉辦板主：%s\n\n舉辦日期：%s\n\n */
    "\033[1;32m\xA1\xBB [%s] \xAC\xDD\xAA\x4F\xA7\xEB\xB2\xBC\xA1\x47%s\033[m\n\n\xC1\x7C\xBF\xEC\xAA\x4F\xA5\x44\xA1\x47%s\n\n\xC1\x7C\xBF\xEC\xA4\xE9\xB4\xC1\xA1\x47%s\n\n",
    msg_seperator, currboard, vch->title, vch->owner, Btime(vch->chrono));
  /* 開票日期：%s\n\n\033[1;32m◆ 投票主題：\033[m\n\n */
  fprintf(fp, "\xB6\x7D\xB2\xBC\xA4\xE9\xB4\xC1\xA1\x47%s\n\n\033[1;32m\xA1\xBB \xA7\xEB\xB2\xBC\xA5\x44\xC3\x44\xA1\x47\033[m\n\n", Btime(vch->vclose));

  *fname = '@';
  f_suck(fp, fpath);

  /* \n\033[1;32m◆ 投票結果：每人可投 %d 票，共 %d 人參加，投出 %d 票\033[m\n\n */
  fprintf(fp, "\n\033[1;32m\xA1\xBB \xA7\xEB\xB2\xBC\xB5\xB2\xAA\x47\xA1\x47\xA8\x43\xA4\x48\xA5\x69\xA7\xEB %d \xB2\xBC\xA1\x41\xA6\x40 %d \xA4\x48\xB0\xD1\xA5\x5B\xA1\x41\xA7\xEB\xA5\x58 %d \xB2\xBC\033[m\n\n",
    vch->maxblt, total, bollt);

  if (vch->vsort == 's')
    qsort(choice, items, sizeof(struct Tchoice), TchoiceCompare);

  if (vch->vpercent == '%')
    fd = BMAX(1, bollt);
  else
    fd = 0;

  if (preview && vch->vgamble == ' ')	/* 只有預覽賭盤才需要顯示賠率 */
    preview = 0;

  for (num = 0; num < items; num++)
  {
    ticket = choice[num].count;
    if (preview)	/* 顯示加買一張時的賠率 */
      /*     %-36s%5d 票 (%4.1f%%) 賠率 1:%.3f\n */
      fprintf(fp, "    %-36s%5d \xB2\xBC (%4.1f%%) \xBD\xDF\xB2\x76 1:%.3f\n", &choice[num].vitem, ticket, 100.0 * ticket / fd, 0.9 * (bollt + 1) / (ticket + 1));
    else if (fd)
      /*     %-36s%5d 票 (%4.1f%%)\n */
      fprintf(fp, "    %-36s%5d \xB2\xBC (%4.1f%%)\n", &choice[num].vitem, ticket, 100.0 * ticket / fd);
    else
      /*     %-36s%5d 票\n */
      fprintf(fp, "    %-36s%5d \xB2\xBC\n", &choice[num].vitem, ticket);
  }

  /* other opinions */

  *fname = 'O';
  /* \n\033[1;32m◆ 我有話要說：\033[m\n\n */
  fputs("\n\033[1;32m\xA1\xBB \xA7\xDA\xA6\xB3\xB8\xDC\xAD\x6E\xBB\xA1\xA1\x47\033[m\n\n", fp);
  f_suck(fp, fpath);
  fputs("\n", fp);
  fclose(fp);

  /* 最後傳回的 fpath 即為投票結果檔 */
  *fname = 'Z';
  return fname;
}


static int
vote_view(xo)
  XO *xo;
{
  char fpath[64];
  VCH *vch;

  vch = (VCH *) xo_pool + (xo->pos - xo->top);

  if (bbstate & STAT_BOARD || vch->vgamble == '$')
  {
    if (draw_vote(fpath, xo->dir, vch, 1))
    {
      more(fpath, NULL);
      unlink(fpath);
      return vote_head(xo);
    }

    /* 目前尚未有人投票 */
    vmsg("\xA5\xD8\xAB\x65\xA9\x7C\xA5\xBC\xA6\xB3\xA4\x48\xA7\xEB\xB2\xBC");
    return XO_FOOT;
  }

  return XO_NONE;
}


static void
keeplog(fnlog, board, title)
  char *fnlog;
  char *board;
  char *title;
{
  HDR hdr;
  char folder[64], fpath[64];
  FILE *fp;

  if (!dashf(fnlog))	/* Kudo.010804: 檔案是空的就不 keeplog */
    return;

  brd_fpath(folder, board, fn_dir);

  if (fp = fdopen(hdr_stamp(folder, 'A', &hdr, fpath), "w"))
  {
    /* 作者: %s (%s)\n標題: %s\n時間: %s\n\n */
    fprintf(fp, "\xA7\x40\xAA\xCC: %s (%s)\n\xBC\xD0\xC3\x44: %s\n\xAE\xC9\xB6\xA1: %s\n\n",
      str_sysop, SYSOPNICK, title, Btime(hdr.chrono));
    f_suck(fp, fnlog);
    fclose(fp);

    strcpy(hdr.title, title);
    strcpy(hdr.owner, str_sysop);
    rec_bot(folder, &hdr, sizeof(HDR));

    btime_update(brd_bno(board));
  }
}


static void
vlog_pay(fpath, choice, fp, vch)/* 賠錢給押對的使用者 */
  char *fpath;			/* 記錄檔路徑 */
  usint choice;			/* 正確的答案 */
  FILE *fp;			/* 寫入的檔案 */
  VCH *vch;
{
  int fd;
  int correct, bollt;		/* 押對/全部 的票數 */
  int single, money;
  char buf[64];
  VLOG vlog;
  PAYCHECK paycheck;

  if ((fd = open(fpath, O_RDONLY)) >= 0)
  {
    /* 第一圈算出賠率 */
    correct = bollt = 0;
    while (read(fd, &vlog, sizeof(VLOG)) == sizeof(VLOG))
    {
      bollt += vlog.numvotes;
      if (vlog.choice == choice)
	correct += vlog.numvotes;
    }

    /* 給板主抽頭 1% */
    money = (INT_MAX / vch->price) * 100;	/* BioStar.050626: 避免溢位 */
    money = (bollt > money) ? INT_MAX : vch->price / 100 * bollt;
    /* 板主 %s 抽頭，可獲得 %d 銀幣\n */
    fprintf(fp, "\xAA\x4F\xA5\x44 %s \xA9\xE2\xC0\x59\xA1\x41\xA5\x69\xC0\xF2\xB1\x6F %d \xBB\xC8\xB9\xF4\n", vch->owner, money);

    memset(&paycheck, 0, sizeof(PAYCHECK));
    time(&paycheck.tissue);
    paycheck.money = money;
    /* [抽頭] %s */
    sprintf(paycheck.reason, "[\xA9\xE2\xC0\x59] %s", currboard);
    usr_fpath(buf, vch->owner, FN_PAYCHECK);
    rec_add(buf, &paycheck, sizeof(PAYCHECK));

    if (correct)	/* 如果沒人押中，就不需要發錢 */
    {
      /* 發獎金，系統抽 10% 的稅 */
      single = (float) vch->price * 0.9 * bollt / correct;
      /* 每張可獲 %d 銀幣，押對的使用者有：\n */
      fprintf(fp, "\xA8\x43\xB1\x69\xA5\x69\xC0\xF2 %d \xBB\xC8\xB9\xF4\xA1\x41\xA9\xE3\xB9\xEF\xAA\xBA\xA8\xCF\xA5\xCE\xAA\xCC\xA6\xB3\xA1\x47\n", single);

      /* 第二圈開始發錢 */
      lseek(fd, (off_t) 0, SEEK_SET);
      while (read(fd, &vlog, sizeof(VLOG)) == sizeof(VLOG))
      {
	if (vlog.choice == choice)
	{
	  money = INT_MAX / single;		/* BioStar.050626: 避免溢位 */
	  money = (vlog.numvotes > money) ? INT_MAX : single * vlog.numvotes;
	  /* %s 買了 %d 張，共可獲得 %d 銀幣\n */
	  fprintf(fp, "%s \xB6\x52\xA4\x46 %d \xB1\x69\xA1\x41\xA6\x40\xA5\x69\xC0\xF2\xB1\x6F %d \xBB\xC8\xB9\xF4\n", vlog.userid, vlog.numvotes, money);

	  paycheck.money = money;
	  /* [賭盤] %s */
	  sprintf(paycheck.reason, "[\xBD\xE4\xBD\x4C] %s", currboard);
	  usr_fpath(buf, vlog.userid, FN_PAYCHECK);
	  rec_add(buf, &paycheck, sizeof(PAYCHECK));
	}
      }
    }

    close(fd);
  }
}


static int
vote_open(xo)
  XO *xo;
{
  int pos, fd, count;
  char *dir, *fname, fpath[64], buf[80];
  usint choice;
  char *slist[MAX_CHOICES];
  vitem_t vlist[MAX_CHOICES];
  VCH *vch;
  FILE *fp;

  if (!(bbstate & STAT_BOARD))
    return XO_NONE;

  pos = xo->pos;
  vch = (VCH *) xo_pool + (pos - xo->top);

  if (time(NULL) < vch->vclose)
  {
    /* 尚未到原定開票時間，確定要提早開票(Y/N)？[N]  */
    if (vans("\xA9\x7C\xA5\xBC\xA8\xEC\xAD\xEC\xA9\x77\xB6\x7D\xB2\xBC\xAE\xC9\xB6\xA1\xA1\x41\xBD\x54\xA9\x77\xAD\x6E\xB4\xA3\xA6\xAD\xB6\x7D\xB2\xBC(Y/N)\xA1\x48[N] ") != 'y')
      return XO_FOOT;
  }

  dir = xo->dir;

  /* 投票結果 */

  if (!(fname = draw_vote(fpath, dir, vch, 0)))
  {
    /* 目前尚未有人投票 */
    vmsg("\xA5\xD8\xAB\x65\xA9\x7C\xA5\xBC\xA6\xB3\xA4\x48\xA7\xEB\xB2\xBC");
    return XO_FOOT;
  }

  if (vch->vgamble == '$')	/* 賭盤 */
  {
    /* 板主輸入結果，並寫入投票結果 */
    /* 請輸入正確答案： */
    while (!vget(b_lines, 0, "\xBD\xD0\xBF\xE9\xA4\x4A\xA5\xBF\xBD\x54\xB5\xAA\xAE\xD7\xA1\x47", buf, 60, DOECHO))
      ;

    /* 載入投票選項檔 */
    *fname = 'I';
    if ((fd = open(fpath, O_RDONLY)) >= 0)
    {
      count = read(fd, vlist, sizeof(vlist)) / sizeof(vitem_t);
      close(fd);

      for (fd = 0; fd < count; fd++)
	slist[fd] = (char *) &vlist[fd];

      /* 板主選出正確答案 */
      choice = 0;
      /* 選擇正確答案 */
      vs_bar("\xBF\xEF\xBE\xDC\xA5\xBF\xBD\x54\xB5\xAA\xAE\xD7");
      /* 投票主題： */
      outs("\xA7\xEB\xB2\xBC\xA5\x44\xC3\x44\xA1\x47");
      for (;;)
      {
	choice = bitset(choice, count, vch->maxblt, vch->title, slist);

	/* 開票 (Y)確定 (N)重來 (Q)取消？[N]  */
	fd = vans("\xB6\x7D\xB2\xBC (Y)\xBD\x54\xA9\x77 (N)\xAD\xAB\xA8\xD3 (Q)\xA8\xFA\xAE\xF8\xA1\x48[N] ");

	if (fd == 'q')
	{
	  *fname = 'Z';
	  unlink(fpath);
	  return vote_head(xo);
	}

	if (fd == 'y' && choice)	/* 若是賭盤則一定要選 */
	  break;
      }

      /* 開始發錢 */
      *fname = 'Z';
      if (fp = fopen(fpath, "a"))
      {
	/* 板主公佈答案：%s\n\n */
	fprintf(fp, "\xAA\x4F\xA5\x44\xA4\xBD\xA7\x47\xB5\xAA\xAE\xD7\xA1\x47%s\n\n", buf);

	*fname = 'G';
	vlog_pay(fpath, choice, fp, vch);

	fputs("\n", fp);
	fclose(fp);
      }

      /* 開票結果 */
      *fname = 'Z';
    }
  }

  /* 將開票結果 post 到 [BN_RECORD] 與 本看板 */

  if (!(currbattr & BRD_NOVOTE))
  {
    /* [記錄] %s <<看板選情報導>> */
    sprintf(buf, "[\xB0\x4F\xBF\xFD] %s <<\xAC\xDD\xAA\x4F\xBF\xEF\xB1\xA1\xB3\xF8\xBE\xC9>>", currboard);
    keeplog(fpath, BN_RECORD, buf);
  }

  /* [記錄] 選情報導 */
  keeplog(fpath, currboard, "[\xB0\x4F\xBF\xFD] \xBF\xEF\xB1\xA1\xB3\xF8\xBE\xC9");

  /* 投票結果附加到 @vote */

  setdirpath(buf, dir, "@/@vote");
  if (fp = fopen(fpath, "a"))
  {
    f_suck(fp, buf);
    fclose(fp);
    rename(fpath, buf);
  }

  /* 開完票就刪除 */
  vch->vgamble = ' ';	/* 令為非賭盤，如此在 delvch 裡面就不會退賭金 */
  delvch(xo, vch);

  currchrono = vch->chrono;
  rec_del(dir, sizeof(VCH), pos, cmpchrono);    

  /* 開票完畢 */
  vmsg("\xB6\x7D\xB2\xBC\xA7\xB9\xB2\xA6");
  return vote_init(xo);
}


static int
vote_tag(xo)
  XO *xo;
{
  VCH *vch;
  int tag, pos, cur;

  pos = xo->pos;
  cur = pos - xo->top;
  vch = (VCH *) xo_pool + cur;

  if (tag = Tagger(vch->chrono, pos, TAG_TOGGLE))
  {
    move(3 + cur, 6);
    outc(tag > 0 ? '*' : ' ');
  }

  /* return XO_NONE; */
  return xo->pos + 1 + XO_MOVE;	/* lkchu.981201: 跳至下一項 */
}


static int
vote_help(xo)
  XO *xo;
{
  xo_help("vote");
  return vote_head(xo);
}


static KeyFunc vote_cb[] =
{
  XO_INIT, vote_init,
  XO_LOAD, vote_load,
  XO_HEAD, vote_head,
  XO_BODY, vote_body,

  'r', vote_join,	/* itoc.010901: 按右鍵比較方便 */
  'v', vote_join,
  'R', vote_result,

  'V', vote_view,
  'E', vote_edit,
  'o', vote_pal,
  'd', vote_delete,
  'D', vote_rangedel,
  't', vote_tag,
  'b', vote_open,

  Ctrl('D'), vote_prune,
  Ctrl('G'), vote_pal,
  Ctrl('P'), vote_add,
  Ctrl('Q'), vote_query,

  'h', vote_help
};


int
XoVote(xo)
  XO *xo;
{
  char fpath[64];

  /* 有 post 權利的才能參加投票 */
  /* 而且要避免 guest 在 sysop 板投票 */

  if (!(bbstate & STAT_POST) || !cuser.userlevel)
    return XO_NONE;

  setdirpath(fpath, xo->dir, FN_VCH);
  if (!(bbstate & STAT_BOARD) && !rec_num(fpath, sizeof(VCH)))
  {
    /* 目前沒有投票舉行 */
    vmsg("\xA5\xD8\xAB\x65\xA8\x53\xA6\xB3\xA7\xEB\xB2\xBC\xC1\x7C\xA6\xE6");
    return XO_FOOT;
  }

  xz[XZ_VOTE - XO_ZONE].xo = xo = xo_new(fpath);
  xz[XZ_VOTE - XO_ZONE].cb = vote_cb;
  xover(XZ_VOTE);
  free(xo);

  return XO_INIT;
}


int
vote_all()		/* itoc.010414: 投票中心 */
{
  typedef struct
  {
    char brdname[BNLEN + 1];
    char class[BCLEN + 1];
    char title[BTLEN + 1];
    char BM[BMLEN + 1];
    char bvote;
  } vbrd_t;

  extern char brd_bits[];
  char *str;
  char fpath[64];
  int num, pageno, pagemax, redraw;
  int ch, cur;
  BRD *bhead, *btail;
  XO *xo;
  vbrd_t vbrd[MAXBOARD], *vb;

  bhead = bshm->bcache;
  btail = bhead + bshm->number;
  cur = 0;
  num = 0;

  do
  {
    str = &brd_bits[cur];
    ch = *str;
    if (bhead->bvote && (ch & BRD_W_BIT))
    {
      vb = vbrd + num;
      strcpy(vb->brdname, bhead->brdname);
      strcpy(vb->class, bhead->class);
      strcpy(vb->title, bhead->title);
      strcpy(vb->BM, bhead->BM);
      vb->bvote = bhead->bvote;
      num++;
    }
    cur++;
  } while (++bhead < btail);

  if (!num)
  {
    /* 目前站內並沒有任何投票 */
    vmsg("\xA5\xD8\xAB\x65\xAF\xB8\xA4\xBA\xA8\xC3\xA8\x53\xA6\xB3\xA5\xF4\xA6\xF3\xA7\xEB\xB2\xBC");
    return XEASY;
  }

  num--;
  pagemax = num / XO_TALL;
  pageno = 0;
  cur = 0;
  redraw = 1;

  do
  {
    if (redraw)
    {
      /* itoc.註解: 盡量做得像 xover 格式 */
      /* 投票中心 */
      vs_head("\xA7\xEB\xB2\xBC\xA4\xA4\xA4\xDF", str_site);
      prints(NECKER_VOTEALL, d_cols >> 1, "", d_cols - (d_cols >> 1), "");

      redraw = pageno * XO_TALL;	/* 借用 redraw */
      ch = BMIN(num, redraw + XO_TALL - 1);
      move(3, 0);
      do
      {
	vb = vbrd + redraw;
	/* itoc.010909: 板名太長的刪掉、加分類顏色。假設 BCLEN = 4 */
	prints("%6d   %-13s\033[1;3%dm%-5s\033[m%s %-*.*s %.*s\n",
	  redraw + 1, vb->brdname,
	  vb->class[3] & 7, vb->class,
	  vb->bvote > 0 ? ICON_VOTED_BRD : ICON_GAMBLED_BRD,
	  (d_cols >> 1) + 34, (d_cols >> 1) + 33, vb->title, d_cols - (d_cols >> 1) + 13, vb->BM);

	redraw++;
      } while (redraw <= ch);

      outf(FEETER_VOTEALL);
      move(3 + cur, 0);
      outc('>');
      redraw = 0;
    }

    switch (ch = vkey())
    {
    case KEY_RIGHT:
    case '\n':
    case ' ':
    case 'r':
      vb = vbrd + (cur + pageno * XO_TALL);

      /* itoc.060324: 等同進入新的看板，XoPost() 有做的事，這裡幾乎都要做 */
      if (!vb->brdname[0])	/* 已刪除的看板 */
	break;

      redraw = brd_bno(vb->brdname);	/* 借用 redraw */
      if (currbno != redraw)
      {
	ch = brd_bits[redraw];

	/* 處理權限 */
	if (ch & BRD_M_BIT)
	  bbstate |= (STAT_BM | STAT_BOARD | STAT_POST);
	else if (ch & BRD_X_BIT)
	  bbstate |= (STAT_BOARD | STAT_POST);
	else if (ch & BRD_W_BIT)
	  bbstate |= STAT_POST;

	mantime_add(currbno, redraw);

	currbno = redraw;
	bhead = bshm->bcache + currbno;
	currbattr = bhead->battr;
	strcpy(currboard, bhead->brdname);
	str = bhead->BM;
	/* 板主：%s */
	/* 徵求中 */
	sprintf(currBM, "\xAA\x4F\xA5\x44\xA1\x47%s", *str <= ' ' ? "\xBC\x78\xA8\x44\xA4\xA4" : str);
#ifdef HAVE_BRDMATE
	strcpy(cutmp->reading, currboard);
#endif

	brd_fpath(fpath, currboard, fn_dir);
#ifdef AUTO_JUMPPOST
	xz[XZ_POST - XO_ZONE].xo = xo = xo_get_post(fpath, bhead);	/* itoc.010910: 為 XoPost 量身打造一支 xo_get() */
#else
	xz[XZ_POST - XO_ZONE].xo = xo = xo_get(fpath);
#endif
	xo->key = XZ_POST;
	xo->xyz = bhead->title;
      }

      sprintf(fpath, "brd/%s/%s", currboard, FN_VCH);
      xz[XZ_VOTE - XO_ZONE].xo = xo = xo_new(fpath);
      xz[XZ_VOTE - XO_ZONE].cb = vote_cb;
      xover(XZ_VOTE);
      free(xo);
      redraw = 1;
      break;

    default:
      ch = xo_cursor(ch, pagemax, num, &pageno, &cur, &redraw);
      break;
    }
  } while (ch != 'q');

  return 0;
}
