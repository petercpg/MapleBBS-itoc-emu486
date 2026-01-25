/*-------------------------------------------------------*/
/* innbbs.c	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : 轉信設定					 */
/* create : 04/04/25					 */
/* update :   /  /  					 */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/*-------------------------------------------------------*/


#include "bbs.h"


extern BCACHE *bshm;


/* ----------------------------------------------------- */
/* nodelist.bbs 子函式					 */
/* ----------------------------------------------------- */


static void
nl_item(num, nl)
  int num;
  nodelist_t *nl;
{
  prints("%6d %-13s%-*.*s %s(%d)\n", num, 
    nl->name, d_cols + 45, d_cols + 45, nl->host, nl->xmode & INN_USEIHAVE ? "IHAVE" : "POST", nl->port);
}


static void
nl_query(nl)
  nodelist_t *nl;
{
  move(3, 0);
  clrtobot();
  /* \n\n轉信站台：%s\n站台位址：%s\n站台協定：%s(%d)\n被 餵 信：%s */
  prints("\n\n\xC2\xE0\xAB\x48\xAF\xB8\xA5\x78\xA1\x47%s\n\xAF\xB8\xA5\x78\xA6\xEC\xA7\x7D\xA1\x47%s\n\xAF\xB8\xA5\x78\xA8\xF3\xA9\x77\xA1\x47%s(%d)\n\xB3\x51 \xC1\xFD \xAB\x48\xA1\x47%s", 
    /* 是 */
    /* 否 */
    nl->name, nl->host, nl->xmode & INN_USEIHAVE ? "IHAVE" : "POST", nl->port, nl->xmode & INN_FEEDED ? "\xAC\x4F" : "\xA7\x5F");
  vmsg(NULL);
}


static int	/* 1:成功 0:失敗 */
nl_add(fpath, old, pos)
  char *fpath;
  nodelist_t *old;
  int pos;
{
  nodelist_t nl;
  int ch, port;
  char ans[8];
  /* 協定：(1)IHAVE (2)POST [1]  */
  char msg1[] = "\xA8\xF3\xA9\x77\xA1\x47(1)IHAVE (2)POST [1] ";
  /* 此站台會主動餵信給本站嗎(Y/N)？[N]  */
  char msg2[] = "\xA6\xB9\xAF\xB8\xA5\x78\xB7\x7C\xA5\x44\xB0\xCA\xC1\xFD\xAB\x48\xB5\xB9\xA5\xBB\xAF\xB8\xB6\xDC(Y/N)\xA1\x48[N] ";

  if (old)
    memcpy(&nl, old, sizeof(nodelist_t));
  else
    memset(&nl, 0, sizeof(nodelist_t));

  /* 英文站名： */
  if (vget(b_lines, 0, "\xAD\x5E\xA4\xE5\xAF\xB8\xA6\x57\xA1\x47", nl.name, sizeof(nl.name), GCARRY) &&
    /* 站址： */
    vget(b_lines, 0, "\xAF\xB8\xA7\x7D\xA1\x47", nl.host, /* sizeof(nl.host) */ 70, GCARRY))
  {
    msg1[24] = (nl.xmode & INN_USEPOST) ? '2' : '1';	/* 新增資料預設 INN_HAVE */
    ch = vans(msg1);
    if (ch != '1' && ch != '2')
      ch = msg1[24];

    if (ch == '1')
    {
      nl.xmode = INN_USEIHAVE | INN_FEEDED;	/* IHAVE 一定是被餵信 */
      /* Port：[7777]  */
      vget(b_lines, 0, "Port\xA1\x47[7777] ", ans, 6, DOECHO);
      if ((port = atoi(ans)) <= 0)
	port = 7777;
    }
    else /* if (ch == '2') */
    {
      nl.xmode = INN_USEPOST;
      /* Port：[119]  */
      vget(b_lines, 0, "Port\xA1\x47[119] ", ans, 6, DOECHO);
      if ((port = atoi(ans)) <= 0)
	port = 119;

      msg2[32] = (old && old->xmode & INN_FEEDED) ? 'Y' : 'N';	/* 新增資料預設不餵信 */
      ch = vans(msg2);
      if (ch != 'y' && ch != 'n')
	ch = msg2[32] | 0x20;

      if (ch == 'y')
	nl.xmode |= INN_FEEDED;
    }
    nl.port = port;

    if (old)
      rec_put(fpath, &nl, sizeof(nodelist_t), pos, NULL);
    else
      rec_add(fpath, &nl, sizeof(nodelist_t));
    return 1;
  }
  return 0;
}


static int
nl_cmp(a, b)
  nodelist_t *a, *b;
{
  /* 依 name 排序 */
  return str_cmp(a->name, b->name);
}


static int
nl_search(nl, key)
  nodelist_t *nl;
  char *key;
{
  return (int) (str_str(nl->name, key) || str_str(nl->host, key));
}


/* ----------------------------------------------------- */
/* newsfeeds.bbs 子函式					 */
/* ----------------------------------------------------- */


static void
nf_item(num, nf)
  int num;
  newsfeeds_t *nf;
{
  int bno;
  BRD *brd;
  char outgo, income;

  if ((bno = brd_bno(nf->board)) >= 0)
  {
    if (nf->xmode & INN_ERROR)
    {
      outgo = income = '?';
    }
    else
    {
      brd = bshm->bcache + bno;
      outgo = brd->battr & BRD_NOTRAN ? ' ' : '<';
      income = nf->xmode & INN_NOINCOME ? ' ': '>';
    }
  }
  else
  {
    outgo = income = 'X';
  }

  prints("%6d %-13s%-*.*s %c-%c %-13s %.7s\n", num, 
    nf->path, d_cols + 32, d_cols + 32, nf->newsgroup, outgo, income, nf->board, nf->charset);
}


static void
nf_query(nf)
  newsfeeds_t *nf;
{
  nodelist_t nl;
  int fd;
  int rc = 0;
  BRD *brd;
  char *outgo, *income;

  /* 找出該站台在 nodelist.bbs 中的資訊 */
  if ((fd = open("innd/nodelist.bbs", O_RDONLY)) >= 0)
  {
    while (read(fd, &nl, sizeof(nodelist_t)) == sizeof(nodelist_t))
    {
      if (!strcmp(nl.name, nf->path))
      {
	rc = 1;
	break;
      }
    }
    close(fd);
  }
  if (!rc)
  {
    memset(&nl, 0, sizeof(nodelist_t));
    /* \033[1;33m此站台不在 nodelist.bbs 中\033[m */
    strcpy(nl.host, "\033[1;33m\xA6\xB9\xAF\xB8\xA5\x78\xA4\xA3\xA6\x62 nodelist.bbs \xA4\xA4\033[m");
  }

  /* 看板狀態 */
  if ((rc = brd_bno(nf->board)) >= 0)
  {
    brd = bshm->bcache + rc;
    /* \033[1;33m不轉出\033[m */
    /* 轉出 */
    outgo = brd->battr & BRD_NOTRAN ? "\033[1;33m\xA4\xA3\xC2\xE0\xA5\x58\033[m"  : "\xC2\xE0\xA5\x58";
    /* 且\033[1;33m不轉進\033[m */
    /* 且轉進 */
    income = nf->xmode & INN_NOINCOME ? "\xA5\x42\033[1;33m\xA4\xA3\xC2\xE0\xB6\x69\033[m" : "\xA5\x42\xC2\xE0\xB6\x69";
  }
  else
  {
    /* \033[1;33m此看板不存在\033[m */
    outgo = "\033[1;33m\xA6\xB9\xAC\xDD\xAA\x4F\xA4\xA3\xA6\x73\xA6\x62\033[m";
    income = "";
  }

  move(3, 0);
  clrtobot();
  /* \n\n轉信站台：%s\n站台位址：%s\n站台協定：%s(%d)\n */
  prints("\n\n\xC2\xE0\xAB\x48\xAF\xB8\xA5\x78\xA1\x47%s\n\xAF\xB8\xA5\x78\xA6\xEC\xA7\x7D\xA1\x47%s\n\xAF\xB8\xA5\x78\xA8\xF3\xA9\x77\xA1\x47%s(%d)\n"
    /* 轉信群組：%s%s\n本站看板：%s (%s%s)\n使用字集：%s */
    "\xC2\xE0\xAB\x48\xB8\x73\xB2\xD5\xA1\x47%s%s\n\xA5\xBB\xAF\xB8\xAC\xDD\xAA\x4F\xA1\x47%s (%s%s)\n\xA8\xCF\xA5\xCE\xA6\x72\xB6\xB0\xA1\x47%s", 
    nf->path, nl.host, nl.xmode & INN_USEIHAVE ? "IHAVE" : "POST", nl.port, 
    /*  (\033[1;33m此群組不存在\033[m) */
    nf->newsgroup, nf->xmode & INN_ERROR ? " (\033[1;33m\xA6\xB9\xB8\x73\xB2\xD5\xA4\xA3\xA6\x73\xA6\x62\033[m)" : "", 
    nf->board, outgo, income, nf->charset);
  if (rc && !(nl.xmode & INN_FEEDED))
    /* \n目前篇數：%d */
    prints("\n\xA5\xD8\xAB\x65\xBD\x67\xBC\xC6\xA1\x47%d", nf->high);
  vmsg(NULL);
}


static int	/* 1:成功 0:失敗 */
nf_add(fpath, old, pos)
  char *fpath;
  newsfeeds_t *old;
  int pos;
{
  newsfeeds_t nf;
  int high;
  char ans[12];
  BRD *brd;

  if (old)
    memcpy(&nf, old, sizeof(newsfeeds_t));
  else
  {
    memset(&nf, 0, sizeof(newsfeeds_t));
    nf.high = INT_MAX;		/* 第一次取信強迫 reload */
  }

  if ((brd = ask_board(nf.board, BRD_L_BIT, NULL)) &&
    /* 英文站名： */
    vget(b_lines, 0, "\xAD\x5E\xA4\xE5\xAF\xB8\xA6\x57\xA1\x47", nf.path, sizeof(nf.path), GCARRY) &&
    /* 群組： */
    vget(b_lines, 0, "\xB8\x73\xB2\xD5\xA1\x47", nf.newsgroup, /* sizeof(nf.newsgroup) */ 70, GCARRY))
  {
    /* 字集 [ */
    /* ]： */
    if (!vget(b_lines, 0, "\xA6\x72\xB6\xB0 [" MYCHARSET "]\xA1\x47", nf.charset, sizeof(nf.charset), GCARRY))
      str_ncpy(nf.charset, MYCHARSET, sizeof(nf.charset));
    /* 是否轉進(Y/N)？[Y]  */
    nf.xmode = (vans("\xAC\x4F\xA7\x5F\xC2\xE0\xB6\x69(Y/N)\xA1\x48[Y] ") == 'n') ? INN_NOINCOME : 0;

    /* 是否更改轉信的 high-number 設定，這設定對被餵信的群組無效(Y/N)？[N]  */
    if (vans("\xAC\x4F\xA7\x5F\xA7\xF3\xA7\xEF\xC2\xE0\xAB\x48\xAA\xBA high-number \xB3\x5D\xA9\x77\xA1\x41\xB3\x6F\xB3\x5D\xA9\x77\xB9\xEF\xB3\x51\xC1\xFD\xAB\x48\xAA\xBA\xB8\x73\xB2\xD5\xB5\x4C\xAE\xC4(Y/N)\xA1\x48[N] ") == 'y')
    {
      sprintf(ans, "%d", nf.high);
      /* 目前篇數： */
      vget(b_lines, 0, "\xA5\xD8\xAB\x65\xBD\x67\xBC\xC6\xA1\x47", ans, 11, GCARRY);
      if ((high = atoi(ans)) >= 0)
	nf.high = high;
    }

    if (old)
      rec_put(fpath, &nf, sizeof(newsfeeds_t), pos, NULL);
    else
      rec_add(fpath, &nf, sizeof(newsfeeds_t));

    /* 本板屬性目前為不轉出，是否改為轉出(Y/N)？[Y]  */
    if ((brd->battr & BRD_NOTRAN) && vans("\xA5\xBB\xAA\x4F\xC4\xDD\xA9\xCA\xA5\xD8\xAB\x65\xAC\xB0\xA4\xA3\xC2\xE0\xA5\x58\xA1\x41\xAC\x4F\xA7\x5F\xA7\xEF\xAC\xB0\xC2\xE0\xA5\x58(Y/N)\xA1\x48[Y] ") != 'n')
    {
      high = brd - bshm->bcache;
      brd->battr &= ~BRD_NOTRAN;
      rec_put(FN_BRD, brd, sizeof(BRD), high, NULL);
    }

    return 1;
  }
  return 0;
}


static int
nf_cmp(a, b)
  newsfeeds_t *a, *b;
{
  /* path/newsgroup 交叉比對 */
  int k = str_cmp(a->path, b->path);
  return k ? k : str_cmp(a->newsgroup, b->newsgroup);
}


static int
nf_search(nf, key)
  newsfeeds_t *nf;
  char *key;
{
  return (int) (str_str(nf->newsgroup, key) || str_str(nf->board, key));
}


/* ----------------------------------------------------- */
/* ncmperm.bbs 子函式					 */
/* ----------------------------------------------------- */


static void
ncm_item(num, ncm)
  int num;
  ncmperm_t *ncm;
{
  prints("%6d %-*.*s%-23.23s %s\n", num, 
    /* ○ */
    /* ╳ */
    d_cols + 44, d_cols + 44, ncm->issuer, ncm->type, ncm->perm ? "\xA1\xB3" : "\xA2\xAE");
}


static void
ncm_query(ncm)
  ncmperm_t *ncm;
{
  move(3, 0);
  clrtobot();
  /* \n\n發行站台：%s\n砍信種類：%s\n允許砍信：%s */
  prints("\n\n\xB5\x6F\xA6\xE6\xAF\xB8\xA5\x78\xA1\x47%s\n\xAC\xE5\xAB\x48\xBA\xD8\xC3\xFE\xA1\x47%s\n\xA4\xB9\xB3\x5C\xAC\xE5\xAB\x48\xA1\x47%s", 
    /* ○ */
    /* ╳ */
    ncm->issuer, ncm->type, ncm->perm ? "\xA1\xB3" : "\xA2\xAE");
  vmsg(NULL);
}


static int	/* 1:成功 0:失敗 */
ncm_add(fpath, old, pos)
  char *fpath;
  ncmperm_t *old;
  int pos; 
{
  ncmperm_t ncm;

  if (old)
    memcpy(&ncm, old, sizeof(ncmperm_t));
  else
    memset(&ncm, 0, sizeof(ncmperm_t));

  /* 發行： */
  if (vget(b_lines, 0, "\xB5\x6F\xA6\xE6\xA1\x47", ncm.issuer, /* sizeof(ncm.issuer) */ 70, GCARRY) &&
    /* 種類： */
    vget(b_lines, 0, "\xBA\xD8\xC3\xFE\xA1\x47", ncm.type, sizeof(ncm.type), GCARRY))
  {
    /* 允許此 NCM message 砍信(Y/N)？[N]  */
    ncm.perm = (vans("\xA4\xB9\xB3\x5C\xA6\xB9 NCM message \xAC\xE5\xAB\x48(Y/N)\xA1\x48[N] ") == 'y');

    if (old)
      rec_put(fpath, &ncm, sizeof(ncmperm_t), pos, NULL);
    else
      rec_add(fpath, &ncm, sizeof(ncmperm_t));
    return 1;
  }
  return 0;
}


static int
ncm_cmp(a, b)
  ncmperm_t *a, *b;
{
  /* issuer/type 交叉比對 */
  int k = str_cmp(a->issuer, b->issuer);
  return k ? k : str_cmp(a->type, b->type);
}


static int
ncm_search(ncm, key)
  ncmperm_t *ncm;
  char *key;
{
  return (int) (str_str(ncm->issuer, key) || str_str(ncm->type, key));
}


/* ----------------------------------------------------- */
/* spamrule.bbs 子函式					 */
/* ----------------------------------------------------- */


static char *
spam_compare(xmode)
  int xmode;
{
  if (xmode & INN_SPAMADDR)
    /* 作者 */
    return "\xA7\x40\xAA\xCC";
  if (xmode & INN_SPAMNICK)
    /* 暱稱 */
    return "\xBC\xCA\xBA\xD9";
  if (xmode & INN_SPAMSUBJECT)
    /* 標題 */
    return "\xBC\xD0\xC3\x44";
  if (xmode & INN_SPAMPATH)
    /* 路徑 */
    return "\xB8\xF4\xAE\x7C";
  if (xmode & INN_SPAMMSGID)
    return "MSID";
  if (xmode & INN_SPAMBODY)
    /* 本文 */
    return "\xA5\xBB\xA4\xE5";
  if (xmode & INN_SPAMSITE)
    /* 組織 */
    return "\xB2\xD5\xC2\xB4";
  if (xmode & INN_SPAMPOSTHOST)
    /* 來源 */
    return "\xA8\xD3\xB7\xBD";
  /* ？？ */
  return "\xA1\x48\xA1\x48";
}


static void
spam_item(num, spam)
  int num;
  spamrule_t *spam;
{
  char *path, *board;

  path = spam->path;
  board = spam->board;
  /* %6d %-13s%-13s[%s] 包含 %.*s\n */
  prints("%6d %-13s%-13s[%s] \xA5\x5D\xA7\x74 %.*s\n", 
    /* 所有站台 */
    /* 所有看板 */
    num, *path ? path : "\xA9\xD2\xA6\xB3\xAF\xB8\xA5\x78", *board ? board : "\xA9\xD2\xA6\xB3\xAC\xDD\xAA\x4F", 
    spam_compare(spam->xmode), d_cols + 30, spam->detail);
}


static void
spam_query(spam)
  spamrule_t *spam;
{
  char *path, *board;

  path = spam->path;
  board = spam->board;

  move(3, 0);
  clrtobot();
  /* \n\n適用站台：%s\n適用看板：%s\n比較項目：%s\n比較內容：%s */
  prints("\n\n\xBE\x41\xA5\xCE\xAF\xB8\xA5\x78\xA1\x47%s\n\xBE\x41\xA5\xCE\xAC\xDD\xAA\x4F\xA1\x47%s\n\xA4\xF1\xB8\xFB\xB6\xB5\xA5\xD8\xA1\x47%s\n\xA4\xF1\xB8\xFB\xA4\xBA\xAE\x65\xA1\x47%s", 
    /* 所有站台 */
    /* 所有看板 */
    *path ? path : "\xA9\xD2\xA6\xB3\xAF\xB8\xA5\x78", *board ? board : "\xA9\xD2\xA6\xB3\xAC\xDD\xAA\x4F", spam_compare(spam->xmode), spam->detail);
  /* 若滿足此規則，會被視為廣告而無法轉信進來 */
  vmsg("\xAD\x59\xBA\xA1\xA8\xAC\xA6\xB9\xB3\x57\xAB\x68\xA1\x41\xB7\x7C\xB3\x51\xB5\xF8\xAC\xB0\xBC\x73\xA7\x69\xA6\xD3\xB5\x4C\xAA\x6B\xC2\xE0\xAB\x48\xB6\x69\xA8\xD3");
}


static int	/* 1:成功 0:失敗 */
spam_add(fpath, old, pos)
  char *fpath;
  spamrule_t *old;
  int pos; 
{
  spamrule_t spam;

  if (old)
    memcpy(&spam, old, sizeof(spamrule_t));
  else
    memset(&spam, 0, sizeof(spamrule_t));

  /* 英文站名： */
  vget(b_lines, 0, "\xAD\x5E\xA4\xE5\xAF\xB8\xA6\x57\xA1\x47", spam.path, sizeof(spam.path), GCARRY);
  ask_board(spam.board, BRD_L_BIT, NULL);

  /* 擋信規則 1)作者 2)暱稱 3)標題 4)路徑 5)MSGID 6)本文 7)組織 8)來源 [Q]  */
  switch (vans("\xBE\xD7\xAB\x48\xB3\x57\xAB\x68 1)\xA7\x40\xAA\xCC 2)\xBC\xCA\xBA\xD9 3)\xBC\xD0\xC3\x44 4)\xB8\xF4\xAE\x7C 5)MSGID 6)\xA5\xBB\xA4\xE5 7)\xB2\xD5\xC2\xB4 8)\xA8\xD3\xB7\xBD [Q] "))
  {
  case '1':
    spam.xmode = INN_SPAMADDR;
    break;
  case '2':
    spam.xmode = INN_SPAMNICK;
    break;
  case '3':
    spam.xmode = INN_SPAMSUBJECT;
    break;
  case '4':
    spam.xmode = INN_SPAMPATH;
    break;
  case '5':
    spam.xmode = INN_SPAMMSGID;
    break;
  case '6':
    spam.xmode = INN_SPAMBODY;
    break;
  case '7':
    spam.xmode = INN_SPAMSITE;
    break;
  case '8':
    spam.xmode = INN_SPAMPOSTHOST;
    break;
  default:
    return 0;
  }

  /* 包含： */
  if (vget(b_lines, 0, "\xA5\x5D\xA7\x74\xA1\x47", spam.detail, /* sizeof(spam.detail) */ 70, GCARRY))
  {
    if (old)
      rec_put(fpath, &spam, sizeof(spamrule_t), pos, NULL);
    else
      rec_add(fpath, &spam, sizeof(spamrule_t));
    return 1;
  }
  return 0;
}


static int
spam_cmp(a, b)
  spamrule_t *a, *b;
{
  /* path/board/xmode/detail 交叉比對 */
  int i = strcmp(a->path, b->path);
  int j = strcmp(a->board, b->board);
  int k = a->xmode - b->xmode;
  return i ? i : j ? j : k ? k : str_cmp(a->detail, b->detail);
}


static int
spam_search(spam, key)
  spamrule_t *spam;
  char *key;
{
  return (int) (str_str(spam->detail, key));
}


/* ----------------------------------------------------- */
/* 轉信設定主函式					 */
/* ----------------------------------------------------- */


int
a_innbbs()
{
  int num, pageno, pagemax, redraw, reload;
  int ch, cur, i, dirty;
  struct stat st;
  char *data;
  int recsiz;
  char *fpath;
  char buf[40];
  void (*item_func)(), (*query_func)();
  int (*add_func)(), (*sync_func)(), (*search_func)();

  /* 轉信設定 */
  vs_bar("\xC2\xE0\xAB\x48\xB3\x5D\xA9\x77");
  more("etc/innbbs.hlp", (char *) -1);

  /* 請選擇 1)轉文站台列表 2)轉文看板列表 3)NoCeM擋文規則 4)廣告文名單：[Q]  */
  switch (vans("\xBD\xD0\xBF\xEF\xBE\xDC 1)\xC2\xE0\xA4\xE5\xAF\xB8\xA5\x78\xA6\x43\xAA\xED 2)\xC2\xE0\xA4\xE5\xAC\xDD\xAA\x4F\xA6\x43\xAA\xED 3)NoCeM\xBE\xD7\xA4\xE5\xB3\x57\xAB\x68 4)\xBC\x73\xA7\x69\xA4\xE5\xA6\x57\xB3\xE6\xA1\x47[Q] "))
  {
  case '1':
    fpath = "innd/nodelist.bbs";
    recsiz = sizeof(nodelist_t);
    item_func = nl_item;
    query_func = nl_query;
    add_func = nl_add;
    sync_func = nl_cmp;
    search_func = nl_search;
    break;

  case '2':
    fpath = "innd/newsfeeds.bbs";
    recsiz = sizeof(newsfeeds_t);
    item_func = nf_item;
    query_func = nf_query;
    add_func = nf_add;
    sync_func = nf_cmp;
    search_func = nf_search;
    break;

  case '3':
    fpath = "innd/ncmperm.bbs";
    recsiz = sizeof(ncmperm_t);
    item_func = ncm_item;
    query_func = ncm_query;
    add_func = ncm_add;
    sync_func = ncm_cmp;
    search_func = ncm_search;
    break;

  case '4':
    fpath = "innd/spamrule.bbs";
    recsiz = sizeof(spamrule_t);
    item_func = spam_item;
    query_func = spam_query;
    add_func = spam_add;
    sync_func = spam_cmp;
    search_func = spam_search;
    break;

  default:
    return 0;
  }

  dirty = 0;	/* 1:有新增/刪除資料 */
  reload = 1;
  pageno = 0;
  cur = 0;
  data = NULL;

  do
  {
    if (reload)
    {
      if (stat(fpath, &st) == -1)
      {
	if (!add_func(fpath, NULL, -1))
	  return 0;
	dirty = 1;
	continue;
      }

      i = st.st_size;
      num = (i / recsiz) - 1;
      if (num < 0)
      {
	if (!add_func(fpath, NULL, -1))
	  return 0;
	dirty = 1;
	continue;
      }

      if ((ch = open(fpath, O_RDONLY)) >= 0)
      {
	data = data ? (char *) realloc(data, i) : (char *) malloc(i);
	read(ch, data, i);
	close(ch);
      }

      pagemax = num / XO_TALL;
      reload = 0;
      redraw = 1;
    }

    if (redraw)
    {
      /* itoc.註解: 盡量做得像 xover 格式 */
      /* 轉信設定 */
      vs_head("\xC2\xE0\xAB\x48\xB3\x5D\xA9\x77", str_site);
      prints(NECKER_INNBBS, d_cols, "");

      i = pageno * XO_TALL;
      ch = BMIN(num, i + XO_TALL - 1);
      move(3, 0);
      do
      {
	item_func(i + 1, data + i * recsiz);
	i++;
      } while (i <= ch);

      outf(FEETER_INNBBS);
      move(3 + cur, 0);
      outc('>');
      redraw = 0;
    }

    ch = vkey();
    switch (ch)
    {
    case KEY_RIGHT:
    case '\n':
    case ' ':
    case 'r':
      i = cur + pageno * XO_TALL;
      query_func(data + i * recsiz);
      redraw = 1;
      break;

    case Ctrl('P'):
      if (add_func(fpath, NULL, -1))
      {
	dirty = 1;
	num++;
	cur = num % XO_TALL;		/* 游標放在新加入的這篇 */
	pageno = num / XO_TALL;
	reload = 1;
      }
      redraw = 1;
      break;

    case 'd':
      if (vans(msg_del_ny) == 'y')
      {
	dirty = 1;
	i = cur + pageno * XO_TALL;
	rec_del(fpath, recsiz, i, NULL);
	cur = i ? ((i - 1) % XO_TALL) : 0;	/* 游標放在砍掉的前一篇 */
	reload = 1;
      }
      redraw = 1;
      break;

    case 'E':
      i = cur + pageno * XO_TALL;
      if (add_func(fpath, data + i * recsiz, i))
      {
	dirty = 1;
	reload = 1;
      }
      redraw = 1;
      break;

    case '/':
      /* 關鍵字： */
      if (vget(b_lines, 0, "\xC3\xF6\xC1\xE4\xA6\x72\xA1\x47", buf, sizeof(buf), DOECHO))
      {
	str_lower(buf, buf);
	for (i = pageno * XO_TALL + cur + 1; i <= num; i++)	/* 從游標下一個開始找 */
	{
	  if (search_func(data + i * recsiz, buf))
	  {
	    pageno = i / XO_TALL;
	    cur = i % XO_TALL;
	    break;
	  }
	}
      }
      redraw = 1;
      break;

    default:
      ch = xo_cursor(ch, pagemax, num, &pageno, &cur, &redraw);
      break;
    }
  } while (ch != 'q');

  free(data);

  if (dirty)
    rec_sync(fpath, recsiz, sync_func, NULL);
  return 0;
}
