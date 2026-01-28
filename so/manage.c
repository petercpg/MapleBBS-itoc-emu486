/*-------------------------------------------------------*/
/* so/manage.c          ( NTHU CS MapleBBS Ver 3.10 )    */
/*-------------------------------------------------------*/
/* target : 看板管理				 	 */
/* create : 95/03/29				 	 */
/* update : 96/04/05				 	 */
/*-------------------------------------------------------*/


#include "bbs.h"


extern BCACHE *bshm;


#ifdef HAVE_TERMINATOR
/* ----------------------------------------------------- */
/* 站長功能 : 拂楓落葉斬				 */
/* ----------------------------------------------------- */


extern char xo_pool[];


/* 《拂楓落葉斬》 */
#define MSG_TERMINATOR	"\xA1\x6D\xA9\xD8\xB7\xAC\xB8\xA8\xB8\xAD\xB1\xD9\xA1\x6E"

int
post_terminator(xo)		/* Thor.980521: 終極文章刪除大法 */
  XO *xo;
{
  int mode, type;
  HDR *hdr;
  char keyOwner[80], keyTitle[TTLEN + 1], buf[80];

  if (!HAS_PERM(PERM_ALLBOARD))
    return XO_FOOT;

  /* 刪除 (1)本文作者 (2)本文標題 (3)自定？[Q]  */
  mode = vans(MSG_TERMINATOR "\xA7\x52\xB0\xA3 (1)\xA5\xBB\xA4\xE5\xA7\x40\xAA\xCC (2)\xA5\xBB\xA4\xE5\xBC\xD0\xC3\x44 (3)\xA6\xDB\xA9\x77\xA1\x48[Q] ") - '0';

  if (mode == 1)
  {
    hdr = (HDR *) xo_pool + (xo->pos - xo->top);
    strcpy(keyOwner, hdr->owner);
  }
  else if (mode == 2)
  {
    hdr = (HDR *) xo_pool + (xo->pos - xo->top);
    strcpy(keyTitle, str_ttl(hdr->title));		/* 拿掉 Re: */
  }
  else if (mode == 3)
  {
    /* 作者： */
    if (!vget(b_lines, 0, "\xA7\x40\xAA\xCC\xA1\x47", keyOwner, 73, DOECHO))
      mode ^= 1;
    /* 標題： */
    if (!vget(b_lines, 0, "\xBC\xD0\xC3\x44\xA1\x47", keyTitle, TTLEN + 1, DOECHO))
      mode ^= 2;
  }
  else
  {
    return XO_FOOT;
  }

  /* 刪除 (1)轉信板 (2)非轉信板 (3)所有看板？[Q]  */
  type = vans(MSG_TERMINATOR "\xA7\x52\xB0\xA3 (1)\xC2\xE0\xAB\x48\xAA\x4F (2)\xAB\x44\xC2\xE0\xAB\x48\xAA\x4F (3)\xA9\xD2\xA6\xB3\xAC\xDD\xAA\x4F\xA1\x48[Q] ");
  if (type < '1' || type > '3')
    return XO_FOOT;

  /* 刪除%s：%.35s 於%s板，確定嗎(Y/N)？[N]  */
  sprintf(buf, "\xA7\x52\xB0\xA3%s\xA1\x47%.35s \xA9\xF3%s\xAA\x4F\xA1\x41\xBD\x54\xA9\x77\xB6\xDC(Y/N)\xA1\x48[N] ", 
    /* 作者 */
    /* 標題 */
    /* 條件 */
    mode == 1 ? "\xA7\x40\xAA\xCC" : mode == 2 ? "\xBC\xD0\xC3\x44" : "\xB1\xF8\xA5\xF3", 
    /* 自定 */
    mode == 1 ? keyOwner : mode == 2 ? keyTitle : "\xA6\xDB\xA9\x77", 
    /* 轉信 */
    /* 非轉信 */
    /* 所有看 */
    type == '1' ? "\xC2\xE0\xAB\x48" : type == '2' ? "\xAB\x44\xC2\xE0\xAB\x48" : "\xA9\xD2\xA6\xB3\xAC\xDD");

  if (vans(buf) == 'y')
  {
    BRD *bhdr, *head, *tail;
    char tmpboard[BNLEN + 1];

    /* Thor.980616: 記下 currboard，以便復原 */
    strcpy(tmpboard, currboard);

    head = bhdr = bshm->bcache;
    tail = bhdr + bshm->number;
    do				/* 至少有 note 一板 */
    {
      int fdr, fsize, xmode;
      FILE *fpw;
      char fpath[64], fnew[64], fold[64];
      HDR *hdr;

      xmode = head->battr;
      if ((type == '1' && (xmode & BRD_NOTRAN)) || (type == '2' && !(xmode & BRD_NOTRAN)))
	continue;

      /* Thor.980616: 更改 currboard，以 cancel post */
      strcpy(currboard, head->brdname);

      /* 看板：%s \033[5m...\033[m */
      sprintf(buf, MSG_TERMINATOR "\xAC\xDD\xAA\x4F\xA1\x47%s \033[5m...\033[m", currboard);
      outz(buf);
      refresh();

      brd_fpath(fpath, currboard, fn_dir);

      if ((fdr = open(fpath, O_RDONLY)) < 0)
	continue;

      if (!(fpw = f_new(fpath, fnew)))
      {
	close(fdr);
	continue;
      }

      fsize = 0;
      mgets(-1);
      while (hdr = mread(fdr, sizeof(HDR)))
      {
	xmode = hdr->xmode;

	if ((xmode & POST_MARKED) || 
	  ((mode & 1) && strcmp(keyOwner, hdr->owner)) ||
	  ((mode & 2) && strcmp(keyTitle, str_ttl(hdr->title))))
	{
	  if ((fwrite(hdr, sizeof(HDR), 1, fpw) != 1))
	  {
	    fclose(fpw);
	    close(fdr);
	    goto contWhileOuter;
	  }
	  fsize++;
	}
	else
	{
	  /* 砍文並連線砍信 */

	  cancel_post(hdr);
	  hdr_fpath(fold, fpath, hdr);
	  unlink(fold);
	}
      }
      close(fdr);
      fclose(fpw);

      sprintf(fold, "%s.o", fpath);
      rename(fpath, fold);
      if (fsize)
	rename(fnew, fpath);
      else
  contWhileOuter:
	unlink(fnew);

      btime_update(brd_bno(currboard));
    } while (++head < tail);

    /* 還原 currboard */
    strcpy(currboard, tmpboard);
    return XO_LOAD;
  }

  return XO_FOOT;
}
#endif	/* HAVE_TERMINATOR */


/* ----------------------------------------------------- */
/* 板主功能 : 修改板名					 */
/* ----------------------------------------------------- */


static int
post_brdtitle(xo)
  XO *xo;
{
  BRD *oldbrd, newbrd;

  oldbrd = bshm->bcache + currbno;
  memcpy(&newbrd, oldbrd, sizeof(BRD));

  /* itoc.註解: 其實呼叫 brd_title(bno) 就可以了，沒差，蠻幹一下好了 :p */
  /* 是否修改中文板名敘述(Y/N)？[N]  */
  if (vans("\xAC\x4F\xA7\x5F\xAD\xD7\xA7\xEF\xA4\xA4\xA4\xE5\xAA\x4F\xA6\x57\xB1\xD4\xAD\x7A(Y/N)\xA1\x48[N] ") == 'y')
  {
    /* 看板主題： */
    vget(b_lines, 0, "\xAC\xDD\xAA\x4F\xA5\x44\xC3\x44\xA1\x47", newbrd.title, BTLEN + 1, GCARRY);

    if (memcmp(&newbrd, oldbrd, sizeof(BRD)) && vans(msg_sure_ny) == 'y')
    {
      memcpy(oldbrd, &newbrd, sizeof(BRD));
      rec_put(FN_BRD, &newbrd, sizeof(BRD), currbno, NULL);
    }
  }

  return XO_HEAD;
}


/* ----------------------------------------------------- */
/* 板主功能 : 修改進板畫面				 */
/* ----------------------------------------------------- */


static int
post_memo_edit(xo)
  XO *xo;
{
  int mode;
  char fpath[64];

  /* 進板畫面 (D)刪除 (E)修改 (Q)取消？[E]  */
  mode = vans("\xB6\x69\xAA\x4F\xB5\x65\xAD\xB1 (D)\xA7\x52\xB0\xA3 (E)\xAD\xD7\xA7\xEF (Q)\xA8\xFA\xAE\xF8\xA1\x48[E] ");

  if (mode != 'q')
  {
    brd_fpath(fpath, currboard, fn_note);

    if (mode == 'd')
    {
      unlink(fpath);
    }
    else
    {
      if (vedit(fpath, 0))	/* Thor.981020: 注意被talk的問題 */
	vmsg(msg_cancel);
    }
  }
  return XO_HEAD;
}


/* ----------------------------------------------------- */
/* 板主功能 : 看板屬性					 */
/* ----------------------------------------------------- */


#ifdef HAVE_SCORE
static int
post_battr_noscore(xo)
  XO *xo;
{
  BRD *oldbrd, newbrd;

  oldbrd = bshm->bcache + currbno;
  memcpy(&newbrd, oldbrd, sizeof(BRD));

  /* 開放評分 (1)允許 (2)不許 (Q)取消？[Q]  */
  switch (vans("\xB6\x7D\xA9\xF1\xB5\xFB\xA4\xC0 (1)\xA4\xB9\xB3\x5C (2)\xA4\xA3\xB3\x5C (Q)\xA8\xFA\xAE\xF8\xA1\x48[Q] "))
  {
  case '1':
    newbrd.battr &= ~BRD_NOSCORE;
    break;
  case '2':
    newbrd.battr |= BRD_NOSCORE;
    break;
  default:
    return XO_HEAD;
  }

  if (memcmp(&newbrd, oldbrd, sizeof(BRD)) && vans(msg_sure_ny) == 'y')
  {
    memcpy(oldbrd, &newbrd, sizeof(BRD));
    rec_put(FN_BRD, &newbrd, sizeof(BRD), currbno, NULL);
  }

  return XO_HEAD;
}
#endif	/* HAVE_SCORE */


/* ----------------------------------------------------- */
/* 板主功能 : 修改板主名單				 */
/* ----------------------------------------------------- */


static int
post_changeBM(xo)
  XO *xo;
{
  char buf[80], userid[IDLEN + 2], *blist;
  BRD *oldbrd, newbrd;
  ACCT acct;
  int BMlen, len;

  oldbrd = bshm->bcache + currbno;

  blist = oldbrd->BM;
  if (is_bm(blist, cuser.userid) != 1)	/* 只有正板主可以設定板主名單 */
    return XO_HEAD;

  memcpy(&newbrd, oldbrd, sizeof(BRD));

  move(3, 0);
  clrtobot();

  move(8, 0);
  /* 目前板主為 %s\n請輸入新的板主名單，或按 [Return] 不改 */
  prints("\xA5\xD8\xAB\x65\xAA\x4F\xA5\x44\xAC\xB0 %s\n\xBD\xD0\xBF\xE9\xA4\x4A\xB7\x73\xAA\xBA\xAA\x4F\xA5\x44\xA6\x57\xB3\xE6\xA1\x41\xA9\xCE\xAB\xF6 [Return] \xA4\xA3\xA7\xEF", oldbrd->BM);

  strcpy(buf, oldbrd->BM);
  BMlen = strlen(buf);

  /* 請輸入副板主，結束請按 Enter，清掉所有副板主請打「無」： */
  while (vget(10, 0, "\xBD\xD0\xBF\xE9\xA4\x4A\xB0\xC6\xAA\x4F\xA5\x44\xA1\x41\xB5\xB2\xA7\xF4\xBD\xD0\xAB\xF6 Enter\xA1\x41\xB2\x4D\xB1\xBC\xA9\xD2\xA6\xB3\xB0\xC6\xAA\x4F\xA5\x44\xBD\xD0\xA5\xB4\xA1\x75\xB5\x4C\xA1\x76\xA1\x47", userid, IDLEN + 1, DOECHO))
  {
    /* 無 */
    if (!strcmp(userid, "\xB5\x4C"))
    {
      strcpy(buf, cuser.userid);
      BMlen = strlen(buf);
    }
    else if (is_bm(buf, userid))	/* 刪除舊有的板主 */
    {
      len = strlen(userid);
      if (!str_cmp(cuser.userid, userid))
      {
	/* 不可以將自己移出板主名單 */
	vmsg("\xA4\xA3\xA5\x69\xA5\x48\xB1\x4E\xA6\xDB\xA4\x76\xB2\xBE\xA5\x58\xAA\x4F\xA5\x44\xA6\x57\xB3\xE6");
	continue;
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
	blist = str_str(buf, userid);
	strcpy(blist, blist + len);
      }
      BMlen -= len;
    }
    else if (acct_load(&acct, userid) >= 0 && !is_bm(buf, userid))	/* 輸入新板主 */
    {
      len = strlen(userid) + 1;	/* '/' + userid */
      if (BMlen + len > BMLEN)
      {
	/* 板主名單過長，無法將這 ID 設為板主 */
	vmsg("\xAA\x4F\xA5\x44\xA6\x57\xB3\xE6\xB9\x4C\xAA\xF8\xA1\x41\xB5\x4C\xAA\x6B\xB1\x4E\xB3\x6F ID \xB3\x5D\xAC\xB0\xAA\x4F\xA5\x44");
	continue;
      }
      sprintf(buf + BMlen, "/%s", acct.userid);
      BMlen += len;

      acct_setperm(&acct, PERM_BM, 0);
    }
    else
      continue;

    move(8, 0);
    /* 目前板主為 %s */
    prints("\xA5\xD8\xAB\x65\xAA\x4F\xA5\x44\xAC\xB0 %s", buf);
    clrtoeol();
  }
  strcpy(newbrd.BM, buf);

  if (memcmp(&newbrd, oldbrd, sizeof(BRD)) && vans(msg_sure_ny) == 'y')
  {
    memcpy(oldbrd, &newbrd, sizeof(BRD));
    rec_put(FN_BRD, &newbrd, sizeof(BRD), currbno, NULL);

    /* 板主：%s */
    sprintf(currBM, "\xAA\x4F\xA5\x44\xA1\x47%s", newbrd.BM);	/* 要重繪檔頭的板主 */
  }

  return XO_HEAD;
}


#ifdef HAVE_MODERATED_BOARD
/* ----------------------------------------------------- */
/* 板主功能 : 看板權限					 */
/* ----------------------------------------------------- */


static int
post_brdlevel(xo)
  XO *xo;
{
  BRD *oldbrd, newbrd;

  oldbrd = bshm->bcache + currbno;
  memcpy(&newbrd, oldbrd, sizeof(BRD));

  /* 1)公開看板 2)秘密看板 3)好友看板？[Q]  */
  switch (vans("1)\xA4\xBD\xB6\x7D\xAC\xDD\xAA\x4F 2)\xAF\xB5\xB1\x4B\xAC\xDD\xAA\x4F 3)\xA6\x6E\xA4\xCD\xAC\xDD\xAA\x4F\xA1\x48[Q] "))
  {
  case '1':				/* 公開看板 */
    newbrd.readlevel = 0;
    newbrd.postlevel = PERM_POST;
    newbrd.battr &= ~(BRD_NOSTAT | BRD_NOVOTE);
    break;

  case '2':				/* 秘密看板 */
    newbrd.readlevel = PERM_SYSOP;
    newbrd.postlevel = 0;
    newbrd.battr |= (BRD_NOSTAT | BRD_NOVOTE);
    break;

  case '3':				/* 好友看板 */
    newbrd.readlevel = PERM_BOARD;
    newbrd.postlevel = 0;
    newbrd.battr |= (BRD_NOSTAT | BRD_NOVOTE);
    break;

  default:
    return XO_HEAD;
  }

  if (memcmp(&newbrd, oldbrd, sizeof(BRD)) && vans(msg_sure_ny) == 'y')
  {
    memcpy(oldbrd, &newbrd, sizeof(BRD));
    rec_put(FN_BRD, &newbrd, sizeof(BRD), currbno, NULL);
  }

  return XO_HEAD;
}
#endif	/* HAVE_MODERATED_BOARD */


#ifdef HAVE_MODERATED_BOARD
/* ----------------------------------------------------- */
/* 板友名單：moderated board				 */
/* ----------------------------------------------------- */


static void
bpal_cache(fpath)
  char *fpath;
{
  BPAL *bpal;

  bpal = bshm->pcache + currbno;
  bpal->pal_max = image_pal(fpath, bpal->pal_spool);
}


extern XZ xz[];


static int
XoBM(xo)
  XO *xo;
{
  XO *xt;
  char fpath[64];

  brd_fpath(fpath, currboard, fn_pal);
  xz[XZ_PAL - XO_ZONE].xo = xt = xo_new(fpath);
  xt->key = PALTYPE_BPAL;
  xover(XZ_PAL);		/* Thor: 進xover前, pal_xo 一定要 ready */

  /* build userno image to speed up, maybe upgreade to shm */

  bpal_cache(fpath);

  free(xt);

  return XO_INIT;
}
#endif	/* HAVE_MODERATED_BOARD */


/* ----------------------------------------------------- */
/* 板主選單						 */
/* ----------------------------------------------------- */


int
post_manage(xo)
  XO *xo;
{
  BRD *brd;

#ifdef POPUP_ANSWER
  char *menu[] = 
  {
    "BQ",
    /* BTitle  修改看板主題 */
    "BTitle  \xAD\xD7\xA7\xEF\xAC\xDD\xAA\x4F\xA5\x44\xC3\x44",
    /* WMemo   編輯進板畫面 */
    "WMemo   \xBD\x73\xBF\xE8\xB6\x69\xAA\x4F\xB5\x65\xAD\xB1",
    /* Manager 增減副板主 */
    "Manager \xBC\x57\xB4\xEE\xB0\xC6\xAA\x4F\xA5\x44",
#  ifdef HAVE_SCORE
    /* Score   設定可否評分 */
    "Score   \xB3\x5D\xA9\x77\xA5\x69\xA7\x5F\xB5\xFB\xA4\xC0",
#  endif
#  ifdef HAVE_MODERATED_BOARD
    /* Level   公開/好友/秘密 */
    "Level   \xA4\xBD\xB6\x7D/\xA6\x6E\xA4\xCD/\xAF\xB5\xB1\x4B",
    /* OPal    板友名單 */
    "OPal    \xAA\x4F\xA4\xCD\xA6\x57\xB3\xE6",
#  endif
    NULL
  };
#else
  /* ◎ 板主選單 (B)主題 (W)進板 (M)副板 */
  char *menu = "\xA1\xB7 \xAA\x4F\xA5\x44\xBF\xEF\xB3\xE6 (B)\xA5\x44\xC3\x44 (W)\xB6\x69\xAA\x4F (M)\xB0\xC6\xAA\x4F"
#  ifdef HAVE_SCORE
    /*  (S)評分 */
    " (S)\xB5\xFB\xA4\xC0"
#  endif
#  ifdef HAVE_MODERATED_BOARD
    /*  (L)權限 (O)板友 */
    " (L)\xC5\x76\xAD\xAD (O)\xAA\x4F\xA4\xCD"
#  endif
    /* ？[Q]  */
    "\xA1\x48[Q] ";
#endif

  /* 板主管理 */
  vs_bar("\xAA\x4F\xA5\x44\xBA\xDE\xB2\x7A");
  brd = bshm->bcache + currbno;
  /* 看板名稱：%s\n看板說明：[%s] %s\n板主名單：%s\n */
  prints("\xAC\xDD\xAA\x4F\xA6\x57\xBA\xD9\xA1\x47%s\n\xAC\xDD\xAA\x4F\xBB\xA1\xA9\xFA\xA1\x47[%s] %s\n\xAA\x4F\xA5\x44\xA6\x57\xB3\xE6\xA1\x47%s\n",
    brd->brdname, brd->class, brd->title, brd->BM);
  /* 中文敘述：%s\n */
  prints("\xA4\xA4\xA4\xE5\xB1\xD4\xAD\x7A\xA1\x47%s\n", brd->title);
#ifdef HAVE_MODERATED_BOARD
  /* 看板權限：%s看板\n */
  /* 秘密 */
  /* 好友 */
  /* 公開 */
  prints("\xAC\xDD\xAA\x4F\xC5\x76\xAD\xAD\xA1\x47%s\xAC\xDD\xAA\x4F\n", brd->readlevel == PERM_SYSOP ? "\xAF\xB5\xB1\x4B" : brd->readlevel == PERM_BOARD ? "\xA6\x6E\xA4\xCD" : "\xA4\xBD\xB6\x7D");
#endif

  if (!(bbstate & STAT_BOARD))
  {
    vmsg(NULL);
    return XO_HEAD;
  }

#ifdef POPUP_ANSWER
  /* 板主選單 */
  switch (pans(3, 20, "\xAA\x4F\xA5\x44\xBF\xEF\xB3\xE6", menu))
#else
  switch (vans(menu))
#endif
  {
  case 'b':
    return post_brdtitle(xo);

  case 'w':
    return post_memo_edit(xo);

  case 'm':
    return post_changeBM(xo);

#ifdef HAVE_SCORE
  case 's':
    return post_battr_noscore(xo);
#endif

#ifdef HAVE_MODERATED_BOARD
  case 'l':
    return post_brdlevel(xo);

  case 'o':
    return XoBM(xo);
#endif
  }

  return XO_HEAD;
}
