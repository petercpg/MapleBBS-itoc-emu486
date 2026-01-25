/* ----------------------------------------------------- */
/* pip_pk.c	( NTHU CS MapleBBS Ver 3.10 )      	 */
/* ----------------------------------------------------- */
/* target : PK 對戰選單                                  */
/* create : 02/02/17                                     */
/* update :   /  /		  			 */
/* author : itoc.bbs@bbs.tnfsh.tn.edu.tw		 */
/* ----------------------------------------------------- */


#include "bbs.h"

#ifdef HAVE_GAME

#include "pip.h"


#if 0

  0. 玩法是進入電子雞遊戲以後選 PK，然後一人選 1)蓄勢待發，
     另一人選 2)下挑戰書輸入前者的 ID 即可開始對戰。

  1. PK 的時候用的值是 ptmp-> 裡面的，所以對戰完 d. 並不會改變。
  2. 為增加趣味，對戰的攻擊方式較 pip_fight.c 為多樣。
  3. 對戰時不可吃補品。 (可改)
  4. 對戰時 skillXYZ 沒有特別效用。 (可改)

  5. pip_pk_skill() 也是偷懶做法，想改的人再自己從 pip_fight.c 抄過來。

#endif

/*-------------------------------------------------------*/
/* pip PK cache                                          */
/*-------------------------------------------------------*/


static PCACHE *pshm;
static PTMP *cp;		/* 我的小雞 */
static PTMP *ep;		/* 對手的小雞 */


static void
pip_pkshm_init()
{
  pshm = shm_new(PIPSHM_KEY, sizeof(PCACHE));
}


static int
pip_ptmp_new(pp)
  PTMP *pp;
{
  PTMP *pentp, *ptail;

  /* --------------------------------------------------- */
  /* semaphore : critical section                        */
  /* --------------------------------------------------- */

#ifdef	HAVE_SEM
  sem_lock(BSEM_ENTER);
#endif

  pentp = pshm->pslot;
  ptail = pentp + MAX_PIPPK_USER;

  do
  {
    if (!pentp->inuse)		/* 找到一個空位子，cp-> 指向 */
    {
      memcpy(pentp, pp, sizeof(PTMP));
      cp = pentp;

#ifdef	HAVE_SEM
      sem_lock(BSEM_LEAVE);
#endif

      return 1;
    }
  } while (++pentp < ptail);

  /* Thor:告訴user有人登先一步了 */

#ifdef	HAVE_SEM
  sem_lock(BSEM_LEAVE);
#endif

  return 0;
}


static int
pip_ptmp_setup()
{
  PTMP ptmp;

  memset(&ptmp, 0, sizeof(PTMP));

  /* 基本屬性 */
  strcpy(ptmp.name, d.name);
  strcpy(ptmp.userid, cuser.userid);

  ptmp.sex = d.sex;
  ptmp.level = d.level;

  /* 血補滿 */
  ptmp.hp = d.maxhp;
  ptmp.mp = d.maxmp;
  ptmp.vp = d.maxvp;
  ptmp.sp = d.maxsp;
  ptmp.maxhp = d.maxhp;
  ptmp.maxmp = d.maxmp;
  ptmp.maxvp = d.maxvp;
  ptmp.maxsp = d.maxsp;

  /* 各種攻擊能力 */
  ptmp.combat = d.attack + (d.resist >> 2);	/* 物理身段: 決定「肉搏、防禦」的強度 */
  ptmp.magic = d.immune + (d.mskill >> 2);	/* 魔法造詣: 決定「法術-各系」的強度 */
  ptmp.speed = d.speed + (d.hskill >> 2);	/* 敏捷技巧: 決定「技能-護身、技能-輕功、技能-劍法」的強度 */
  ptmp.spirit = d.brave + (d.etchics >> 2);	/* 內力強度: 決定「技能-心法、技能-拳法、技能-刀法」的強度 */
  ptmp.charm = d.charm + (d.art >> 2);		/* 動感魅力: 決定「魅惑、召喚」的強度 */
  ptmp.oral = d.speech + (d.manners >> 2);	/* 口若懸河: 決定「說服、煽動」的強度 */
  ptmp.cook = d.cook + (d.homework >> 2);	/* 美味烹調: 決定「技能-暗器、法術-治療」 */

  if (!pip_ptmp_new(&ptmp))
  {
    /* 抱歉，PK 場客滿了喔 */
    vmsg("\xA9\xEA\xBA\x70\xA1\x41PK \xB3\xF5\xAB\xC8\xBA\xA1\xA4\x46\xB3\xE1");
    return 0;
  }

  return 1;
}


static void
pip_ptmp_free()
{
  if (!cp || !cp->inuse)
    return;

#ifdef  HAVE_SEM
  sem_lock(BSEM_ENTER);
#endif

  cp->inuse = 0;

#ifdef  HAVE_SEM
  sem_lock(BSEM_LEAVE);
#endif
}


static PTMP *
pip_ptmp_get(userid, inuse)
  char *userid;
  int inuse;		/* 1:找「蓄勢待發」的人來挑戰  2:找「下挑戰書」的挑戰者回應 */
{
  PTMP *pentp, *ptail;

  pentp = pshm->pslot;
  ptail = pentp + MAX_PIPPK_USER;
  do
  {
    if (pentp->inuse == inuse && !str_cmp(pentp->userid, userid))
      return pentp;
  } while (++pentp < ptail);

  return NULL;
}


static void
pip_ptmp_show()
{
  int max;
  PTMP *pentp, *ptail;

  clrfromto(7, 16);
  move(10, 5);

  max = 0;
  pentp = pshm->pslot;
  ptail = pentp + MAX_PIPPK_USER;
  do
  {
    if (pentp->inuse)
    {
      max++;
      prints("\033[1;3%dm%-20s\033[m", 2 + pentp->inuse, pentp->userid);

      if (max % 4 == 0)
        move(10 + max % 4, 5);
    }
  } while (++pentp < ptail);

  move(8, 5);
  /* \033[1;31m戰鬥中  \033[33m蓄勢待發  \033[34m挑戰者等待回應\033[m */
  prints("\033[1;31m\xBE\xD4\xB0\xAB\xA4\xA4  \033[33m\xBB\x57\xB6\xD5\xAB\xDD\xB5\x6F  \033[34m\xAC\x44\xBE\xD4\xAA\xCC\xB5\xA5\xAB\xDD\xA6\x5E\xC0\xB3\033[m"
    /*   目前場子裡有 \033[1;36m%d/%d\033[m 隻雞 */
    "  \xA5\xD8\xAB\x65\xB3\xF5\xA4\x6C\xB8\xCC\xA6\xB3 \033[1;36m%d/%d\033[m \xB0\xA6\xC2\xFB", max, MAX_PIPPK_USER);
}


/*-------------------------------------------------------*/
/* 對戰主函式                                            */
/*-------------------------------------------------------*/


  /*-----------------------------------------------------*/
  /* 輪流控制                                            */
  /*-----------------------------------------------------*/


static void
pip_pk_turn()	/* 換對方 */
{
  cp->done = 1;
  ep->done = 0;
}


  /*-----------------------------------------------------*/
  /* 畫面顯示                                            */
  /*-----------------------------------------------------*/


static void
pip_pk_showfoot()
{
  /*  戰鬥命令  */
  /*  [1]肉搏 [2]技能 [3]魅惑 [4]召喚 [5]說服 [6]煽動 [Q]認輸            \033[m */
  out_cmd("", COLOR1 " \xBE\xD4\xB0\xAB\xA9\x52\xA5\x4F " COLOR2 " [1]\xA6\xD7\xB7\x69 [2]\xA7\xDE\xAF\xE0 [3]\xBE\x79\xB4\x62 [4]\xA5\x6C\xB3\xEA [5]\xBB\xA1\xAA\x41 [6]\xBA\xB4\xB0\xCA [Q]\xBB\x7B\xBF\xE9            \033[m");
}


static void
pip_pk_showing()
{
  int pic;
  char inbuf1[20], inbuf2[20], inbuf3[20], inbuf4[20];
  
  clear();
  move(0, 0);

  /*  ～\033[32m%s\033[37m%-13s                                            \033[m\n */
  prints("\033[1;41m  " BBSNAME PIPNAME " \xA1\xE3\033[32m%s\033[37m%-13s                                            \033[m\n", 
    /* ♂ */
    /* ♀ */
    /* ？ */
    cp->sex == 1 ? "\xA1\xF1" : (cp->sex == 2 ? "\xA1\xF0" : "\xA1\x48"), cp->name);

  /* 螢幕上方秀出我的小雞資料 */

  sprintf(inbuf1, "%d%s/%d%s", cp->hp > 1000 ? cp->hp / 1000 : cp->hp,
    cp->hp > 1000 ? "K" : "", cp->maxhp > 1000 ? cp->maxhp / 1000 : cp->maxhp,
    cp->maxhp > 1000 ? "K" : "");
  sprintf(inbuf2, "%d%s/%d%s", cp->mp > 1000 ? cp->mp / 1000 : cp->mp,
    cp->mp > 1000 ? "K" : "", cp->maxmp > 1000 ? cp->maxmp / 1000 : cp->maxmp,
    cp->maxmp > 1000 ? "K" : "");
  sprintf(inbuf3, "%d%s/%d%s", cp->vp > 1000 ? cp->vp / 1000 : cp->vp,
    cp->vp > 1000 ? "K" : "", cp->maxvp > 1000 ? cp->maxvp / 1000 : cp->maxvp,
    cp->maxvp > 1000 ? "K" : "");
  sprintf(inbuf4, "%d%s/%d%s", cp->sp > 1000 ? cp->sp / 1000 : cp->sp,
    cp->sp > 1000 ? "K" : "", cp->maxsp > 1000 ? cp->maxsp / 1000 : cp->maxsp,
    cp->maxsp > 1000 ? "K" : "");

  /* \033[1;31m┌──────────────────────────────────────┐\033[m\n */
  outs("\033[1;31m\xA2\x7A\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x7B\033[m\n");
  /* \033[1;31m│\033[33m生  命:\033[37m%-12s\033[33m法  力:\033[37m%-12s\033[33m移動力:\033[37m%-12s\033[33m內  力:\033[37m%-12s\033[31m│\033[m\n */
  prints("\033[1;31m\xA2\x78\033[33m\xA5\xCD  \xA9\x52:\033[37m%-12s\033[33m\xAA\x6B  \xA4\x4F:\033[37m%-12s\033[33m\xB2\xBE\xB0\xCA\xA4\x4F:\033[37m%-12s\033[33m\xA4\xBA  \xA4\x4F:\033[37m%-12s\033[31m\xA2\x78\033[m\n", inbuf1, inbuf2, inbuf3, inbuf4);
  /* \033[1;31m│\033[33m攻  擊:\033[37m%-12d\033[33m魔  法:\033[37m%-12d\033[33m敏  捷:\033[37m%-12d\033[33m武  術:\033[37m%-12d\033[31m│\033[m\n */
  prints("\033[1;31m\xA2\x78\033[33m\xA7\xF0  \xC0\xBB:\033[37m%-12d\033[33m\xC5\x5D  \xAA\x6B:\033[37m%-12d\033[33m\xB1\xD3  \xB1\xB6:\033[37m%-12d\033[33m\xAA\x5A  \xB3\x4E:\033[37m%-12d\033[31m\xA2\x78\033[m\n", cp->combat, cp->magic, cp->speed, cp->spirit);
  /* \033[1;31m│\033[33m魅  力:\033[37m%-12d\033[33m口  才:\033[37m%-12d\033[33m烹  調:\033[37m%-12d\033[33m等  級:\033[37m%-12d\033[31m│\033[m\n */
  prints("\033[1;31m\xA2\x78\033[33m\xBE\x79  \xA4\x4F:\033[37m%-12d\033[33m\xA4\x66  \xA4\x7E:\033[37m%-12d\033[33m\xB2\x69  \xBD\xD5:\033[37m%-12d\033[33m\xB5\xA5  \xAF\xC5:\033[37m%-12d\033[31m\xA2\x78\033[m\n", cp->charm, cp->oral, cp->cook, cp->level);
  /* \033[1;31m└──────────────────────────────────────┘\033[m */
  outs("\033[1;31m\xA2\x7C\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x7D\033[m");

  /* 螢幕中間 7~16 列 秀出怪物的圖檔 */
  pic = 101 + 100 * (rand() % 5) + rand() % 3;    /* 101~501 102~502 103~503 十五選一 */
  show_badman_pic(pic);

  /* 螢幕下方秀出對方的小雞資料 */

  sprintf(inbuf1, "%d%s/%d%s", ep->hp > 1000 ? ep->hp / 1000 : ep->hp,
    ep->hp > 1000 ? "K" : "", ep->maxhp > 1000 ? ep->maxhp / 1000 : ep->maxhp,
    ep->maxhp > 1000 ? "K" : "");
  sprintf(inbuf2, "%d%s/%d%s", ep->mp > 1000 ? ep->mp / 1000 : ep->mp,
    ep->mp > 1000 ? "K" : "", ep->maxmp > 1000 ? ep->maxmp / 1000 : ep->maxmp,
    ep->maxmp > 1000 ? "K" : "");
  sprintf(inbuf3, "%d%s/%d%s", ep->vp > 1000 ? ep->vp / 1000 : ep->vp,
    ep->vp > 1000 ? "K" : "", ep->maxvp > 1000 ? ep->maxvp / 1000 : ep->maxvp,
    ep->maxvp > 1000 ? "K" : "");
  sprintf(inbuf4, "%d%s/%d%s", ep->sp > 1000 ? ep->sp / 1000 : ep->sp,
    ep->sp > 1000 ? "K" : "", ep->maxsp > 1000 ? ep->maxsp / 1000 : ep->maxsp,
    ep->maxsp > 1000 ? "K" : "");

  move(18, 0);
  /* \033[1;34m┌──────────────────────────────────────┐\033[m\n */
  outs("\033[1;34m\xA2\x7A\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x7B\033[m\n");
  /* \033[1;34m│\033[32m姓  名:\033[37m%-12s\033[32mＩ  Ｄ:\033[37m%-12s\033[32m性  別:\033[37m%-12s\033[32m等  級:\033[37m%-12d\033[34m│\033[m\n */
  /* ♂ */
  /* ♀ */
  /* ？ */
  prints("\033[1;34m\xA2\x78\033[32m\xA9\x6D  \xA6\x57:\033[37m%-12s\033[32m\xA2\xD7  \xA2\xD2:\033[37m%-12s\033[32m\xA9\xCA  \xA7\x4F:\033[37m%-12s\033[32m\xB5\xA5  \xAF\xC5:\033[37m%-12d\033[34m\xA2\x78\033[m\n", ep->name, ep->userid, ep->sex == 1 ? "\xA1\xF1" : (ep->sex == 2 ? "\xA1\xF0" : "\xA1\x48"), ep->level);
  /* \033[1;34m│\033[32m生  命:\033[37m%-12s\033[32m法  力:\033[37m%-12s\033[32m移動力:\033[37m%-12s\033[32m內  力:\033[37m%-12s\033[34m│\033[m\n */
  prints("\033[1;34m\xA2\x78\033[32m\xA5\xCD  \xA9\x52:\033[37m%-12s\033[32m\xAA\x6B  \xA4\x4F:\033[37m%-12s\033[32m\xB2\xBE\xB0\xCA\xA4\x4F:\033[37m%-12s\033[32m\xA4\xBA  \xA4\x4F:\033[37m%-12s\033[34m\xA2\x78\033[m\n", inbuf1, inbuf2, inbuf3, inbuf4);
  /* \033[1;34m└──────────────────────────────────────┘\033[m\n */
  outs("\033[1;34m\xA2\x7C\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x7D\033[m\n");

  pip_pk_showfoot(); 
}


static void
pip_pk_ending()
{
  clrfromto(7, 16);
  move(8, 0);

  if (cp->hp > 0)
  {
    /*            \033[1;31m┌──────────────────────┐\033[m\n */
    outs("           \033[1;31m\xA2\x7A\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x7B\033[m\n");
    /*            \033[1;31m│ \033[37m武術大會的小雞\033[33m%-13s                \033[31m│\033[m\n */
    prints("           \033[1;31m\xA2\x78 \033[37m\xAA\x5A\xB3\x4E\xA4\x6A\xB7\x7C\xAA\xBA\xA4\x70\xC2\xFB\033[33m%-13s                \033[31m\xA2\x78\033[m\n", cp->name);
    /*            \033[1;31m│ \033[37m打敗了強勁的對手\033[32m%-13s              \033[31m│\033[m\n */
    prints("           \033[1;31m\xA2\x78 \033[37m\xA5\xB4\xB1\xD1\xA4\x46\xB1\x6A\xAB\x6C\xAA\xBA\xB9\xEF\xA4\xE2\033[32m%-13s              \033[31m\xA2\x78\033[m\n", ep->name);
    /*            \033[1;31m│ \033[37m勇敢和經驗都上升了不少                     \033[31m│\033[m\n */
    outs("           \033[1;31m\xA2\x78 \033[37m\xAB\x69\xB4\xB1\xA9\x4D\xB8\x67\xC5\xE7\xB3\xA3\xA4\x57\xA4\xC9\xA4\x46\xA4\xA3\xA4\xD6                     \033[31m\xA2\x78\033[m\n");
    /*            \033[1;31m└──────────────────────┘\033[m */
    outs("           \033[1;31m\xA2\x7C\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x7D\033[m");
    /* 您打敗了一個強硬的傢伙 */
    vmsg("\xB1\x7A\xA5\xB4\xB1\xD1\xA4\x46\xA4\x40\xAD\xD3\xB1\x6A\xB5\x77\xAA\xBA\xB3\xC3\xA5\xEB");
    d.exp += ep->level;
  }
  else
  {
    /*            \033[1;31m┌──────────────────────┐\033[m\n */
    outs("           \033[1;31m\xA2\x7A\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x7B\033[m\n");
    /*            \033[1;31m│ \033[37m武術大會的小雞\033[33m%-13s                \033[31m│\033[m\n */
    prints("           \033[1;31m\xA2\x78 \033[37m\xAA\x5A\xB3\x4E\xA4\x6A\xB7\x7C\xAA\xBA\xA4\x70\xC2\xFB\033[33m%-13s                \033[31m\xA2\x78\033[m\n", cp->name);
    /*            \033[1;31m│ \033[37m被\033[32m%-13s\033[37m對手打得落花流水            \033[31m│\033[m\n */
    prints("           \033[1;31m\xA2\x78 \033[37m\xB3\x51\033[32m%-13s\033[37m\xB9\xEF\xA4\xE2\xA5\xB4\xB1\x6F\xB8\xA8\xAA\xE1\xAC\x79\xA4\xF4            \033[31m\xA2\x78\033[m\n", ep->name);
    /*            \033[1;31m│ \033[37m決定回家好好再鍛練                         \033[31m│\033[m\n */
    outs("           \033[1;31m\xA2\x78 \033[37m\xA8\x4D\xA9\x77\xA6\x5E\xAE\x61\xA6\x6E\xA6\x6E\xA6\x41\xC1\xEB\xBD\x6D                         \033[31m\xA2\x78\033[m\n");
    /*            \033[1;31m└──────────────────────┘\033[m */
    outs("           \033[1;31m\xA2\x7C\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x77\xA2\x7D\033[m");
    /* 被打敗的您心中相當不是味道 */
    vmsg("\xB3\x51\xA5\xB4\xB1\xD1\xAA\xBA\xB1\x7A\xA4\xDF\xA4\xA4\xAC\xDB\xB7\xED\xA4\xA3\xAC\x4F\xA8\xFD\xB9\x44");
    d.exp -= cp->level;
  }
}


  /*-----------------------------------------------------*/
  /* 行動函式                                            */
  /*-----------------------------------------------------*/


static void
pip_pk_combat()		/* 肉搏 */
{
  int injure;

  injure = cp->combat - (ep->combat >> 2);
  if (injure > 0)
  {
    ep->hp -= injure;
    show_fight_pic(1);
    /* 您造成了對方的傷害 */
    vmsg("\xB1\x7A\xB3\x79\xA6\xA8\xA4\x46\xB9\xEF\xA4\xE8\xAA\xBA\xB6\xCB\xAE\x60");
  }
  else
  {
    show_fight_pic(2);
    /* 您的攻擊在對方眼裡簡直是搔癢 */
    vmsg("\xB1\x7A\xAA\xBA\xA7\xF0\xC0\xBB\xA6\x62\xB9\xEF\xA4\xE8\xB2\xB4\xB8\xCC\xC2\xB2\xAA\xBD\xAC\x4F\xB7\x6B\xC4\x6F");
  }
  pip_pk_turn();  
}


static void 
pip_pk_skill()		/* 技能: 武功/魔法 */
{
  /* itoc.020217: 懶得寫像 pip_fight.c 裡面那種的了，有興趣的人自己抄著改 :p */

  /* 技能不扣點，但效果比較差(只有其他的對半) */

  int ch, class;
  int injure[5] = {125, 200, 300, 450, 750};

  /*  武功選單  */
  /*  [1]護身 [2]輕功 [3]心法 [4]拳法 [5]劍法 [6]刀法 [7]暗器 [Q]放棄    \033[m */
  out_cmd(COLOR1 " \xAA\x5A\xA5\x5C\xBF\xEF\xB3\xE6 " COLOR2 " [1]\xC5\x40\xA8\xAD [2]\xBB\xB4\xA5\x5C [3]\xA4\xDF\xAA\x6B [4]\xAE\xB1\xAA\x6B [5]\xBC\x43\xAA\x6B [6]\xA4\x4D\xAA\x6B [7]\xB7\x74\xBE\xB9 [Q]\xA9\xF1\xB1\xF3    \033[m",
    /*  法術選單  */
    /*  [A]治療 [B]雷系 [C]冰系 [D]炎系 [E]土系 [F]風系 [G]特殊 [Q]放棄    \033[m */
    COLOR1 " \xAA\x6B\xB3\x4E\xBF\xEF\xB3\xE6 " COLOR2 " [A]\xAA\x76\xC0\xF8 [B]\xB9\x70\xA8\x74 [C]\xA6\x42\xA8\x74 [D]\xAA\xA2\xA8\x74 [E]\xA4\x67\xA8\x74 [F]\xAD\xB7\xA8\x74 [G]\xAF\x53\xAE\xED [Q]\xA9\xF1\xB1\xF3    \033[m");

  for (;;)
  {
    ch = vkey();

    if (ch == 'q')
    {
      pip_pk_showfoot();
      return;
    }

    else if (ch == '1')		/* 護身 */
    {
      class = rand() % 10;
      if (class == 0)		/* 10% 的機率反彈攻擊，可再度攻擊 */
      {
        ep->hp -= cp->speed >> 3;
        /* 對手的攻擊反彈回他的身上 */
        vmsg("\xB9\xEF\xA4\xE2\xAA\xBA\xA7\xF0\xC0\xBB\xA4\xCF\xBC\x75\xA6\x5E\xA5\x4C\xAA\xBA\xA8\xAD\xA4\x57");
      }
      else if (class <= 2)	/* 20% 的機率使對方物理攻擊永遠降低，可再度攻擊 */
      {
        ep->combat = ep->combat * 4 / 5;
        /* 對方的手扭到了，看來短時間內不會恢復 */
        vmsg("\xB9\xEF\xA4\xE8\xAA\xBA\xA4\xE2\xA7\xE1\xA8\xEC\xA4\x46\xA1\x41\xAC\xDD\xA8\xD3\xB5\x75\xAE\xC9\xB6\xA1\xA4\xBA\xA4\xA3\xB7\x7C\xAB\xEC\xB4\x5F");
      }
      else			/* 70% 的機率什麼都沒做 */
      {
        /* 您採取防禦措施 */
        vmsg("\xB1\x7A\xB1\xC4\xA8\xFA\xA8\xBE\xBF\x6D\xB1\xB9\xAC\x49");
        break;
      }
      pip_pk_showfoot();
      return;
    }

    else if (ch == '2')		/* 輕功 */
    {
      class = cp->speed >> 9;
      if (class > 4)
        class = 4;

      cp->vp += injure[class];	/* 補 vp 也借用 injure[] */      
      if (cp->vp > cp->maxvp)
        cp->vp = cp->maxvp;

      /* 精神飽滿，準備再戰 */
      vmsg("\xBA\xEB\xAF\xAB\xB9\xA1\xBA\xA1\xA1\x41\xB7\xC7\xB3\xC6\xA6\x41\xBE\xD4");
      break;
    }

    else if (ch == '3')		/* 心法 */
    {
      class = cp->spirit >> 9;
      if (class > 4)
        class = 4;

      cp->sp += injure[class];	/* 補 sp 也借用 injure[] */      
      if (cp->sp > cp->maxsp)
        cp->sp = cp->maxsp;

      /* 活力充沛，準備再戰 */
      vmsg("\xAC\xA1\xA4\x4F\xA5\x52\xA8\x4B\xA1\x41\xB7\xC7\xB3\xC6\xA6\x41\xBE\xD4");
      break;
    }

    else if (ch == '4')		/* 拳法 */
    {
      class = cp->spirit >> 9;
      if (class > 4)
        class = 4;

      ep->hp -= injure[class] * (75 + rand() % 50) / 100;
      /* 全身精力集中於掌上，奮力一擊 */
      vmsg("\xA5\xFE\xA8\xAD\xBA\xEB\xA4\x4F\xB6\xB0\xA4\xA4\xA9\xF3\xB4\x78\xA4\x57\xA1\x41\xBE\xC4\xA4\x4F\xA4\x40\xC0\xBB");
      break;
    }

    else if (ch == '5')		/* 劍法 */
    {
      class = cp->speed >> 9;
      if (class > 4)
        class = 4;

      ep->hp -= injure[class] * (50 + rand() % 100) / 100;
      /* 快劍斬亂麻，神劍闖江湖 */
      vmsg("\xA7\xD6\xBC\x43\xB1\xD9\xB6\xC3\xB3\xC2\xA1\x41\xAF\xAB\xBC\x43\xC2\xF4\xA6\xBF\xB4\xF2");
      break;
    }

    else if (ch == '6')		/* 刀法 */
    {
      class = cp->spirit >> 9;
      if (class > 4)
        class = 4;

      ep->hp -= injure[class] * (30 + rand() % 140) / 100;
      /* 全身精力集中於掌上，奮力一擊 */
      vmsg("\xA5\xFE\xA8\xAD\xBA\xEB\xA4\x4F\xB6\xB0\xA4\xA4\xA9\xF3\xB4\x78\xA4\x57\xA1\x41\xBE\xC4\xA4\x4F\xA4\x40\xC0\xBB");
      break;
    }

    else if (ch == '7')		/* 暗器 */
    {
      class = cp->cook >> 9;
      if (class > 4)
        class = 4;

      ep->hp -= injure[class] * (80 + rand() % 40) / 100;
      /* 您的菜餚中下了毒藥，對手不注意就吃了下去 */
      vmsg("\xB1\x7A\xAA\xBA\xB5\xE6\xC0\x61\xA4\xA4\xA4\x55\xA4\x46\xAC\x72\xC3\xC4\xA1\x41\xB9\xEF\xA4\xE2\xA4\xA3\xAA\x60\xB7\x4E\xB4\x4E\xA6\x59\xA4\x46\xA4\x55\xA5\x68");
      break;
    }

    else if (ch == 'a')		/* 治療 */
    {
      class = cp->cook >> 10;	/* 補 hp 的門檻比較高 */
      if (class > 4)
        class = 4;

      cp->hp += injure[class];	/* 補 hp 也借用 injure[] */      
      if (cp->hp > cp->maxhp)
        cp->hp = cp->maxhp;

      /* 充電以後，再度出發 */
      vmsg("\xA5\x52\xB9\x71\xA5\x48\xAB\xE1\xA1\x41\xA6\x41\xAB\xD7\xA5\x58\xB5\x6F");
      break;
    }

    else if (ch >= 'b' && ch <= 'f')		/* 各系法術 */
    {
      char buf[64];
      /* 雷 */
      /* 冰 */
      /* 炎 */
      /* 土 */
      /* 風 */
      char name[5][3] = {"\xB9\x70", "\xA6\x42", "\xAA\xA2", "\xA4\x67", "\xAD\xB7"};

      class = cp->magic >> 9;
      if (class > 4)
        class = 4;

      ep->hp -= injure[class] * (50 + rand() % 100) / 100;
      /* 您施展了%s系法術，威力十足 */
      sprintf(buf, "\xB1\x7A\xAC\x49\xAE\x69\xA4\x46%s\xA8\x74\xAA\x6B\xB3\x4E\xA1\x41\xAB\xC2\xA4\x4F\xA4\x51\xA8\xAC", name[ch - 'b']);
      vmsg(buf);
      break;
    }

    else if (ch == 'g')		/* 特殊 */
    {
      class = cp->magic >> 9;
      if (class > 4)
        class = 4;

      cp->mp += injure[class];	/* 補 mp 也借用 injure[] */      
      if (cp->mp > cp->maxmp)
        cp->mp = cp->maxmp;

      /* 能量充填，魔力再現 */
      vmsg("\xAF\xE0\xB6\x71\xA5\x52\xB6\xF1\xA1\x41\xC5\x5D\xA4\x4F\xA6\x41\xB2\x7B");
      break;
    }
  }

  pip_pk_turn();
}


static void
pip_pk_charm()		/* 魅惑: 耗 hp */
{
  int class;
  char buf[80];
  /* 凡夫俗子 */
  /* 色情狂 */
  /* 小鬼 */
  /* 龍神 */
  /* 站長 */
  char name[5][9] = {"\xA4\x5A\xA4\xD2\xAB\x55\xA4\x6C", "\xA6\xE2\xB1\xA1\xA8\x67", "\xA4\x70\xB0\xAD", "\xC0\x73\xAF\xAB", "\xAF\xB8\xAA\xF8"};
  int injure[5] = {250, 400, 600, 900, 1500};
  int needhp[5] = {350, 600, 900, 1500, 3200};

  class = cp->charm >> 8;	/* 相對於以下三者，其所需門檻比較低就可以施展強大魅惑術，但耗的是 hp */
  if (class > 4)
    class = 4;

  if (cp->hp >= needhp[class])
  {
    cp->hp -= needhp[class];
    ep->hp -= injure[class] * (75 + rand() % 50) / 100;
    /* 一群%s在您的指使之下，拼命攻擊對方 */
    sprintf(buf, "\xA4\x40\xB8\x73%s\xA6\x62\xB1\x7A\xAA\xBA\xAB\xFC\xA8\xCF\xA4\xA7\xA4\x55\xA1\x41\xAB\xF7\xA9\x52\xA7\xF0\xC0\xBB\xB9\xEF\xA4\xE8", name[class]);
    vmsg(buf);
    pip_pk_turn();
  }
  else
  {
    /* 您全身都是傷口，還想魅惑誰 */
    vmsg("\xB1\x7A\xA5\xFE\xA8\xAD\xB3\xA3\xAC\x4F\xB6\xCB\xA4\x66\xA1\x41\xC1\xD9\xB7\x51\xBE\x79\xB4\x62\xBD\xD6");
    pip_pk_showfoot();  
  }
}


static void
pip_pk_summon()		/* 召喚: 耗 mp */
{
  int class;
  char buf[80];
  /* 史萊姆 */
  /* 虎頭蜂 */
  /* 猛虎 */
  /* 遠古巨龍 */
  /* 死神撒旦 */
  char name[5][9] = {"\xA5\x76\xB5\xDC\xA9\x69", "\xAA\xEA\xC0\x59\xB8\xC1", "\xB2\x72\xAA\xEA", "\xBB\xB7\xA5\x6A\xA5\xA8\xC0\x73", "\xA6\xBA\xAF\xAB\xBC\xBB\xA5\xB9"};
  int injure[5] = {250, 400, 600, 900, 1500};
  int needmp[5] = {350, 600, 900, 1500, 3200};

  class = cp->charm >> 9;
  if (class > 4)
    class = 4;

  if (cp->mp >= needmp[class])
  {
    cp->mp -= needmp[class];
    ep->hp -= injure[class] * (75 + rand() % 50) / 100;
    /* 您召喚出%s，重重地給了對方一擊 */
    sprintf(buf, "\xB1\x7A\xA5\x6C\xB3\xEA\xA5\x58%s\xA1\x41\xAD\xAB\xAD\xAB\xA6\x61\xB5\xB9\xA4\x46\xB9\xEF\xA4\xE8\xA4\x40\xC0\xBB", name[class]);
    vmsg(buf);
    pip_pk_turn();
  }
  else
  {
    /* 您感到全身疲憊，什麼也召喚不出來 */
    vmsg("\xB1\x7A\xB7\x50\xA8\xEC\xA5\xFE\xA8\xAD\xAF\x68\xBE\xCE\xA1\x41\xA4\xB0\xBB\xF2\xA4\x5D\xA5\x6C\xB3\xEA\xA4\xA3\xA5\x58\xA8\xD3");
    pip_pk_showfoot();  
  }
}


static void
pip_pk_convince()	/* 說服: 耗 vp */
{
  int class;
  char buf[80];
  /* 面具怪人 */
  /* 骷髏頭怪 */
  /* 炸蛋超人 */
  /* 東海龍王 */
  /* 齊天大聖 */
  char name[5][9] = {"\xAD\xB1\xA8\xE3\xA9\xC7\xA4\x48", "\xBE\x75\xC5\x5C\xC0\x59\xA9\xC7", "\xAC\xB5\xB3\x4A\xB6\x57\xA4\x48", "\xAA\x46\xAE\xFC\xC0\x73\xA4\xFD", "\xBB\xF4\xA4\xD1\xA4\x6A\xB8\x74"};
  int injure[5] = {250, 400, 600, 900, 1500};
  int needvp[5] = {350, 600, 900, 1500, 3200};

  class = cp->oral >> 9;
  if (class > 4)
    class = 4;

  if (cp->vp >= needvp[class])
  {
    cp->vp -= needvp[class];
    ep->hp -= injure[class] * (75 + rand() % 50) / 100;
    /* 您成功地說服%s來助您一臂之力 */
    sprintf(buf, "\xB1\x7A\xA6\xA8\xA5\x5C\xA6\x61\xBB\xA1\xAA\x41%s\xA8\xD3\xA7\x55\xB1\x7A\xA4\x40\xC1\x75\xA4\xA7\xA4\x4F", name[class]);
    vmsg(buf);
    pip_pk_turn();
  }
  else
  {
    /* 遠方傳來一陣聲音：想說服我，再等一百年吧 */
    vmsg("\xBB\xB7\xA4\xE8\xB6\xC7\xA8\xD3\xA4\x40\xB0\x7D\xC1\x6E\xAD\xB5\xA1\x47\xB7\x51\xBB\xA1\xAA\x41\xA7\xDA\xA1\x41\xA6\x41\xB5\xA5\xA4\x40\xA6\xCA\xA6\x7E\xA7\x61");
    pip_pk_showfoot();  
  }
}


static void
pip_pk_incite()		/* 煽動: 耗 sp */
{
  int class;
  char buf[80];
  /* 巨蠅怪 */
  /* 布耶魯 */
  /* 地獄犬 */
  /* 噴火龍 */
  /* 熾天使 */
  char name[5][9] = {"\xA5\xA8\xC3\xC7\xA9\xC7", "\xA5\xAC\xAD\x43\xBE\x7C", "\xA6\x61\xBA\xBB\xA4\xFC", "\xBC\x51\xA4\xF5\xC0\x73", "\xBF\x4B\xA4\xD1\xA8\xCF"};
  int injure[5] = {250, 400, 600, 900, 1500};
  int needsp[5] = {350, 600, 900, 1500, 3200};

  class = cp->oral >> 9;
  if (class > 4)
    class = 4;

  if (cp->sp >= needsp[class])
  {
    cp->sp -= needsp[class];
    ep->hp -= injure[class] * (40 + rand() % 120) / 100;	/* 變異性較前面三者為高 */
    /* 您勇敢地煽動%s整個族群來對付敵人 */
    sprintf(buf, "\xB1\x7A\xAB\x69\xB4\xB1\xA6\x61\xBA\xB4\xB0\xCA%s\xBE\xE3\xAD\xD3\xB1\xDA\xB8\x73\xA8\xD3\xB9\xEF\xA5\x49\xBC\xC4\xA4\x48", name[class]);
    vmsg(buf);
    pip_pk_turn();
  }
  else
  {
    /* 眾人不為所動，您的計畫失敗了 */
    vmsg("\xB2\xB3\xA4\x48\xA4\xA3\xAC\xB0\xA9\xD2\xB0\xCA\xA1\x41\xB1\x7A\xAA\xBA\xAD\x70\xB5\x65\xA5\xA2\xB1\xD1\xA4\x46");
    pip_pk_showfoot();  
  }
}


static void
pip_pk_man()		/* 輪到我下指令 */
{
  /* 秀出戰鬥主畫面 */
  pip_pk_showing();

  while (!cp->done)
  {
    switch (vkey())
    {
    case '1':		/* 肉搏 */
      pip_pk_combat();
      break;

    case '2':		/* 技能: 武功/魔法 */
      pip_pk_skill();
      break;

    case '3':		/* 魅惑 */
      pip_pk_charm();
      break;

    case '4':		/* 召喚 */
      pip_pk_summon();
      break;

    case '5':		/* 說服 */
      pip_pk_convince();
      break;

    case '6':		/* 煽動 */
      pip_pk_incite(); 
      break;

    case 'q':		/* 認輸 */
      cp->hp = 0;
      pip_pk_turn();
      break;
    }
  }
}


static void
pip_pk_wait()		/* 等待對方下指令 */
{
  int fd;
  struct timeval tv = {1, 100};

  /* 秀出戰鬥主畫面 */
  pip_pk_showing();

  /* 等待對方的攻擊 Q)認輸 */
  outz("\xB5\xA5\xAB\xDD\xB9\xEF\xA4\xE8\xAA\xBA\xA7\xF0\xC0\xBB Q)\xBB\x7B\xBF\xE9");
  refresh();

  while (!ep->done)
  {
    fd = 1;
    if (select(1, (fd_set *) &fd, NULL, NULL, &tv) > 0)
    {
      if (vkey() == 'q')
      {
	cp->done = 1;	/* 二人都強迫結束行動 */
	ep->done = 1;      
	cp->hp = 0;	/* 有事離開算輸 */
	break;
      }
    }
  }
}


/*-------------------------------------------------------*/
/* 對戰主選單                                            */
/*-------------------------------------------------------*/


int 
pip_pk_menu()
{
  int ch;
  char userid[IDLEN + 1];
  struct timeval tv = {1, 100};

  /* itoc.020327: 有個蠻大的問題是，如果對戰到一半，其中一人斷線了，
     另外一人會進入迴圈，只能按 Q 離開 */

  if (d.hp <= 0)
    return XEASY;

  pip_pkshm_init();

  pip_ptmp_show();

  /* ◎ 小雞對戰 1)蓄勢待發 2)下挑戰書 [Q]離開  */
  ch = ians(b_lines, 0, "\xA1\xB7 \xA4\x70\xC2\xFB\xB9\xEF\xBE\xD4 1)\xBB\x57\xB6\xD5\xAB\xDD\xB5\x6F 2)\xA4\x55\xAC\x44\xBE\xD4\xAE\xD1 [Q]\xC2\xF7\xB6\x7D ");
  if (ch == '1')
  {
    /* 設定 cp-> */
    if (!pip_ptmp_setup())
      return XEASY;

    cp->inuse = 1;

    /* 等候挑戰中 Q)離開 */
    outz("\xB5\xA5\xAD\xD4\xAC\x44\xBE\xD4\xA4\xA4 Q)\xC2\xF7\xB6\x7D");
    refresh();
    do
    {
      ch = 1;
      if (select(1, (fd_set *) &ch, NULL, NULL, &tv) > 0)
      {
	if (vkey() == 'q')
	{
	  pip_ptmp_free();
	  return XEASY;
	}
      }
    } while (!*cp->mateid);

    if (!(ep = pip_ptmp_get(cp->mateid, 2)))
    {
      pip_ptmp_free();
      return XEASY;
    }
    cp->done = 0;	/* 被挑戰者先行動，挑戰者就會通知他 */
    cp->inuse = -1;	/* 兩方都進入戰鬥 */
    ep->inuse = -1;
  }
  else if (ch == '2')
  {
    /* 決定 PK 對戰對象 ep-> */
    if (!vget(b_lines, 0, msg_uid, userid, IDLEN + 1, DOECHO) || 
      !str_cmp(cuser.userid, userid) ||
      !(ep = pip_ptmp_get(userid, 1)))
    {
      return XEASY;
    }

    /* 設定 cp-> */
    if (!pip_ptmp_setup())
      return XEASY;

    strcpy(cp->mateid, userid);
    strcpy(ep->mateid, cuser.userid);
    cp->done = 1;	/* 挑戰者後行動，就會主動通知被挑戰者 */
    cp->inuse = 2;
  }
  else
  {
    return XEASY;
  }

  ch = d.tired;

  for (;;)		/* 雙方攻防 */
  {  
    pip_pk_man();
    if (ep->hp <= 0 || cp->hp <= 0)
      break;          

    pip_pk_wait();
    if (cp->hp <= 0 || ep->hp <= 0)
      break;
  }

  /* 結果判斷 */
  pip_pk_ending();

  pip_ptmp_free();

  d.tired = ch;		/* 以免 PK 打太久，PK 結束後來因 tired 過高死掉了 */
  return 0;
}
#endif	/* HAVE_GAME */
