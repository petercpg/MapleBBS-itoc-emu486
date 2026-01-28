/*-------------------------------------------------------*/
/* include/battr.h      ( NTHU CS MapleBBS Ver 2.36 )    */
/*-------------------------------------------------------*/
/* target : Board Attribution				 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/


#ifndef	_BATTR_H_
#define	_BATTR_H_


/* ----------------------------------------------------- */
/* Board Attribution : flags in BRD.battr		 */
/* ----------------------------------------------------- */


#define BRD_NOZAP	0x01	/* 不可 zap */
#define BRD_NOTRAN	0x02	/* 不轉信 */
#define BRD_NOCOUNT	0x04	/* 不計文章發表篇數 */
#define BRD_NOSTAT	0x08	/* 不納入熱門話題統計 */
#define BRD_NOVOTE	0x10	/* 不公佈投票結果於 [record] 板 */
#define BRD_ANONYMOUS	0x20	/* 匿名看板 */
#define BRD_NOSCORE	0x40	/* 不評分看板 */
#define BRD_NOCHANGE    0x80    /* 板主不能修改看板權限 */


/* ----------------------------------------------------- */
/* 各種旗標的中文意義					 */
/* ----------------------------------------------------- */


#define NUMBATTRS	8

#define STR_BATTR	"zTcsvA%C"			/* itoc: 新增旗標的時候別忘了改這裡啊 */


#ifdef _ADMIN_C_
static char *battr_tbl[NUMBATTRS] =
{
  /* 不可 Zap */
  "\xA4\xA3\xA5\x69 Zap",			/* BRD_NOZAP */
  /* 不轉信出去 */
  "\xA4\xA3\xC2\xE0\xAB\x48\xA5\x58\xA5\x68",			/* BRD_NOTRAN */
  /* 不記錄篇數 */
  "\xA4\xA3\xB0\x4F\xBF\xFD\xBD\x67\xBC\xC6",			/* BRD_NOCOUNT */
  /* 不做熱門話題統計 */
  "\xA4\xA3\xB0\xB5\xBC\xF6\xAA\xF9\xB8\xDC\xC3\x44\xB2\xCE\xAD\x70",		/* BRD_NOSTAT */
  /* 不公開投票結果 */
  "\xA4\xA3\xA4\xBD\xB6\x7D\xA7\xEB\xB2\xBC\xB5\xB2\xAA\x47",		/* BRD_NOVOTE */
  /* 匿名看板 */
  "\xB0\xCE\xA6\x57\xAC\xDD\xAA\x4F",			/* BRD_ANONYMOUS */
  /* 不評分看板 */
  "\xA4\xA3\xB5\xFB\xA4\xC0\xAC\xDD\xAA\x4F",			/* BRD_NOSCORE */
  /* 板主不能修改看板權限 */
  "\xAA\x4F\xA5\x44\xA4\xA3\xAF\xE0\xAD\xD7\xA7\xEF\xAC\xDD\xAA\x4F\xC5\x76\xAD\xAD", 	/* BRD_NOREPLY */
};

#endif

#endif				/* _BATTR_H_ */
