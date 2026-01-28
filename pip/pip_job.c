/*-------------------------------------------------------*/
/* pip/pip_job.c        ( NTHU CS MapleBBS Ver 3.10 )    */
/*-------------------------------------------------------*/
/* target : 打工                                         */
/* create :   /  /                                       */
/* update : 01/08/15                                     */
/* author : dsyan.bbs@forever.twbbs.org		 	 */
/* modify : itoc.bbs@bbs.tnfsh.tn.edu.tw                 */
/*-------------------------------------------------------*/


#include "bbs.h"

#ifdef HAVE_GAME

#include "pip.h"


/*-------------------------------------------------------*/
/* 函式庫                                                */
/*-------------------------------------------------------*/


/* itoc.010815: 寫一隻公用的 function */

static int		/* 傳回: -1:放棄  0~100:成功狀況 */
pip_job_function(classgrade, tired_prob, tired_base, pic)
  int classgrade;	/* 身心狀況: 0% ~ 100 % */
  int tired_prob;	/* 計算疲勞的機率 */
  int tired_base;	/* 計算疲勞的底數 */
  int pic;		/* 要秀的圖 */
{
  int grade;

  /* 因為還沒 update，learn_skill 可能 < 0 */
  if (LEARN_LEVEL < 0)
  {
    /* 您已經累到爆了 */
    vmsg("\xB1\x7A\xA4\x77\xB8\x67\xB2\xD6\xA8\xEC\xC3\x7A\xA4\x46");
    return -1;
  }

  grade = classgrade * LEARN_LEVEL;

  /* grade 應該只從 0~100% */
  if (grade < 0)
    grade = 0;
  else if (grade > 100)
    grade = 100;

  /* 所有工作都會改變的屬性 */
  count_tired(tired_prob, tired_base, 1, 100, 1);	/* 增加疲勞與年齡有關 */
  d.shit += rand() % 3 + 5;
  d.hp -= rand() % 2 + 4;
  d.happy -= rand() % 3 + 4;
  d.satisfy -= rand() % 3 + 4;

  show_job_pic(pic);
  return grade;		/* 回傳工作結果: 0:失敗透了  ~   100:完全成功 */
}


/*-------------------------------------------------------*/
/* 打工選單:家事 苦工 家教 地攤				 */
/*-------------------------------------------------------*/


int
pip_job_workA()
{
  /* ├────┼──────────────────────┤ */
  /* │家庭管理│+ 待人接物 掃地洗衣 烹飪 親子關係 家事評價  │ */
  /* │        │- 感受                                      │ */
  /* ├────┼──────────────────────┤ */

  int class;

  class = d.hp * 100 / d.maxhp - d.tired;
  if ((class = pip_job_function(class, 2, 5, 11)) < 0)
    return 0;

  if (class >= 75)
  {
    class = 4;
    d.money += 80 + (d.homework + d.cook) / 50;
    /* 家事很成功喔..多一點錢給您.. */
    vmsg("\xAE\x61\xA8\xC6\xAB\xDC\xA6\xA8\xA5\x5C\xB3\xE1..\xA6\x68\xA4\x40\xC2\x49\xBF\xFA\xB5\xB9\xB1\x7A..");
  }
  else if (class >= 50)
  {
    class = 3;
    d.money += 60 + (d.homework + d.cook) / 55;
    /* 家事還蠻順利的唷..嗯嗯.. */
    vmsg("\xAE\x61\xA8\xC6\xC1\xD9\xC6\x5A\xB6\xB6\xA7\x51\xAA\xBA\xAD\xF2..\xB6\xE2\xB6\xE2..");
  }
  else if (class >= 25)
  {
    class = 2;
    d.money += 40 + (d.homework + d.cook) / 60;
    /* 家事普普通通啦..可以更好的..加油.. */
    vmsg("\xAE\x61\xA8\xC6\xB4\xB6\xB4\xB6\xB3\x71\xB3\x71\xB0\xD5..\xA5\x69\xA5\x48\xA7\xF3\xA6\x6E\xAA\xBA..\xA5\x5B\xAA\x6F..");
  }
  else
  {
    class = 1;
    d.money += 20 + (d.homework + d.cook) / 65;
    /* 家事很糟糕喔..這樣不行啦.. */
    vmsg("\xAE\x61\xA8\xC6\xAB\xDC\xC1\x56\xBF\x7C\xB3\xE1..\xB3\x6F\xBC\xCB\xA4\xA3\xA6\xE6\xB0\xD5..");
  }

  d.toman += rand() % 2;
  d.homework += rand() % 2 + class;
  d.cook += rand() % 2 + class;
  d.relation += rand() % 2;
  d.family += class;

  d.affect -= rand() % 5;
  if (d.affect < 0)
    d.affect = 0;

  d.workA++;
  return 0;
}


int
pip_job_workB()
{
  /* ├────┼──────────────────────┤ */
  /* │育幼院  │+ 待人接物 愛心 感受                        │ */
  /* │        │- 魅力                                      │ */
  /* ├────┼──────────────────────┤ */

  int class;

  class = d.hp * 100 / d.maxhp - d.tired;
  if ((class = pip_job_function(class, 3, 7, 21)) < 0)
    return 0;

  if (class >= 75)
  {
    class = 4;
    d.money += 150 + (d.toman + d.love) / 50;
    /* 當保姆很成功喔..下次再來喔.. */
    vmsg("\xB7\xED\xAB\x4F\xA9\x69\xAB\xDC\xA6\xA8\xA5\x5C\xB3\xE1..\xA4\x55\xA6\xB8\xA6\x41\xA8\xD3\xB3\xE1..");
  }
  else if (class >= 50)
  {
    class = 3;
    d.money += 120 + (d.toman + d.love) / 55;
    /* 保姆還當的不錯唷..嗯嗯.. */
    vmsg("\xAB\x4F\xA9\x69\xC1\xD9\xB7\xED\xAA\xBA\xA4\xA3\xBF\xF9\xAD\xF2..\xB6\xE2\xB6\xE2..");
  }
  else if (class >= 25)
  {
    class = 2;
    d.money += 100 + (d.toman + d.love) / 60;
    /* 小朋友很皮喔..加油.. */
    vmsg("\xA4\x70\xAA\x42\xA4\xCD\xAB\xDC\xA5\xD6\xB3\xE1..\xA5\x5B\xAA\x6F..");
  }
  else
  {
    class = 1;
    d.money += 80 + (d.toman + d.love) / 65;
    /* 很糟糕喔..連小朋友都罩不住.. */
    vmsg("\xAB\xDC\xC1\x56\xBF\x7C\xB3\xE1..\xB3\x73\xA4\x70\xAA\x42\xA4\xCD\xB3\xA3\xB8\x6E\xA4\xA3\xA6\xED..");
  }

  d.toman += rand() % 3;
  d.love += rand() % 3 + class;
  d.affect += class;

  d.charm -= rand() % 5;
  if (d.charm < 0)
    d.charm = 0;

  /* itoc.010824: 亂數學會暗器 */
  if (rand() % 30 == 0)
    pip_learn_skill(7);

  d.workB++;
  return 0;
}


int
pip_job_workC()
{
  /* ├────┼──────────────────────┤ */
  /* │旅館    │+ 掃地洗衣 烹飪 家事評價                    │ */
  /* │        │- 無                                        │ */
  /* ├────┼──────────────────────┤ */

  int class;

  class = d.hp * 100 / d.maxhp - d.tired;
  if ((class = pip_job_function(class, 4, 8, 31)) < 0)
    return 0;

  if (class >= 75)
  {
    class = 4;
    d.money += 250 + (d.homework + d.cook) / 50;
    /* 旅館事業蒸蒸日上..希望您再過來幫忙.. */
    vmsg("\xAE\xC8\xC0\x5D\xA8\xC6\xB7\x7E\xBB\x5D\xBB\x5D\xA4\xE9\xA4\x57..\xA7\xC6\xB1\xE6\xB1\x7A\xA6\x41\xB9\x4C\xA8\xD3\xC0\xB0\xA6\xA3..");
  }
  else if (class >= 50)
  {
    class = 3;
    d.money += 200 + (d.homework + d.cook) / 55;
    /* 旅館還蠻順利的唷..嗯嗯.. */
    vmsg("\xAE\xC8\xC0\x5D\xC1\xD9\xC6\x5A\xB6\xB6\xA7\x51\xAA\xBA\xAD\xF2..\xB6\xE2\xB6\xE2..");
  }
  else if (class >= 25)
  {
    class = 2;
    d.money += 150 + (d.homework + d.cook) / 60;
    /* 普普通通啦..可以更好的..加油.. */
    vmsg("\xB4\xB6\xB4\xB6\xB3\x71\xB3\x71\xB0\xD5..\xA5\x69\xA5\x48\xA7\xF3\xA6\x6E\xAA\xBA..\xA5\x5B\xAA\x6F..");
  }
  else
  {
    class = 1;
    d.money += 100 + (d.homework + d.cook) / 65;
    /* 這個很糟糕喔..這樣不行啦.. */
    vmsg("\xB3\x6F\xAD\xD3\xAB\xDC\xC1\x56\xBF\x7C\xB3\xE1..\xB3\x6F\xBC\xCB\xA4\xA3\xA6\xE6\xB0\xD5..");
  }

  d.homework += rand() % 2 + class;
  d.cook += rand() % 2 + class;
  d.family += class;

  d.workC++;
  return 0;
}


int
pip_job_workD()
{
  /* ├────┼──────────────────────┤ */
  /* │農場    │+ 無                                        │ */
  /* │        │- 氣質                                      │ */
  /* ├────┼──────────────────────┤ */

  int class;

  class = d.hp * 100 / d.maxhp - d.tired;
  if ((class = pip_job_function(class, 6, 12, 41)) < 0)
    return 0;

  if (class >= 75)
  {
    d.money += 250 + (d.attack + d.resist) / 50;
    /* 牛羊長的好好喔..希望您再來幫忙.. */
    vmsg("\xA4\xFB\xA6\xCF\xAA\xF8\xAA\xBA\xA6\x6E\xA6\x6E\xB3\xE1..\xA7\xC6\xB1\xE6\xB1\x7A\xA6\x41\xA8\xD3\xC0\xB0\xA6\xA3..");
  }
  else if (class >= 50)
  {
    d.money += 210 + (d.attack + d.resist) / 55;
    /* 呵呵..還不錯喔.. */
    vmsg("\xA8\xFE\xA8\xFE..\xC1\xD9\xA4\xA3\xBF\xF9\xB3\xE1..");
  }
  else if (class >= 25)
  {
    d.money += 160 + (d.attack + d.resist) / 60;
    /* 普普通通啦..可以更好的.. */
    vmsg("\xB4\xB6\xB4\xB6\xB3\x71\xB3\x71\xB0\xD5..\xA5\x69\xA5\x48\xA7\xF3\xA6\x6E\xAA\xBA..");
  }
  else
  {
    d.money += 120 + (d.attack + d.resist) / 65;
    /* 您不太適合農場的工作.. */
    vmsg("\xB1\x7A\xA4\xA3\xA4\xD3\xBE\x41\xA6\x58\xB9\x41\xB3\xF5\xAA\xBA\xA4\x75\xA7\x40..");
  }

  d.character -= rand() % 5;
  if (d.character < 0)
    d.character = 0;

  d.workD++;
  return 0;
}


int
pip_job_workE()
{
  /* ├────┼──────────────────────┤ */
  /* │餐廳    │+ 掃地洗衣 烹飪                             │ */
  /* │        │- 無                                        │ */
  /* ├────┼──────────────────────┤ */

  int class;

  class = d.cook / 6 - d.tired;
  if ((class = pip_job_function(class, 4, 9, 51)) < 0)
    return 0;

  if (class >= 75)
  {
    class = 4;
    d.money += 250 + (d.homework + d.cook) / 50;
    /* 客人都說太好吃了..再來一盤吧.. */
    vmsg("\xAB\xC8\xA4\x48\xB3\xA3\xBB\xA1\xA4\xD3\xA6\x6E\xA6\x59\xA4\x46..\xA6\x41\xA8\xD3\xA4\x40\xBD\x4C\xA7\x61..");
  }
  else if (class >= 50)
  {
    class = 3;
    d.money += 200 + (d.homework + d.cook) / 55;
    /* 煮的還不錯吃唷.. */
    vmsg("\xB5\x4E\xAA\xBA\xC1\xD9\xA4\xA3\xBF\xF9\xA6\x59\xAD\xF2..");
  }
  else if (class >= 25)
  {
    class = 2;
    d.money += 150 + (d.homework + d.cook) / 60;
    /* 普普通通啦..可以更好的.. */
    vmsg("\xB4\xB6\xB4\xB6\xB3\x71\xB3\x71\xB0\xD5..\xA5\x69\xA5\x48\xA7\xF3\xA6\x6E\xAA\xBA..");
  }
  else
  {
    class = 1;
    d.money += 100 + (d.homework + d.cook) / 65;
    /* 廚藝待加強喔.. */
    vmsg("\xBC\x70\xC3\xC0\xAB\xDD\xA5\x5B\xB1\x6A\xB3\xE1..");
  }

  d.homework += rand() % 2 + class;
  d.cook += rand() % 5 + class;

  d.workE++;
  return 0;
}


int
pip_job_workF()
{
  /* ├────┼──────────────────────┤ */
  /* │教堂    │+ 愛心 道德 信仰                            │ */
  /* │        │- 罪孽                                      │ */
  /* ├────┼──────────────────────┤ */

  int class;

  class = d.hp * 100 / d.maxhp - d.tired;
  if ((class = pip_job_function(class, 3, 6, 61)) < 0)
    return 0;

  if (class >= 75)
  {
    class = 4;
    d.money += 100 + (d.etchics + d.belief) / 50;
    /* 非常謝謝您喔..真是得力的助手 */
    vmsg("\xAB\x44\xB1\x60\xC1\xC2\xC1\xC2\xB1\x7A\xB3\xE1..\xAF\x75\xAC\x4F\xB1\x6F\xA4\x4F\xAA\xBA\xA7\x55\xA4\xE2");
  }
  else if (class >= 50)
  {
    class = 3;
    d.money += 75 + (d.etchics + d.belief) / 55;
    /* 謝謝您的熱心幫忙.. */
    vmsg("\xC1\xC2\xC1\xC2\xB1\x7A\xAA\xBA\xBC\xF6\xA4\xDF\xC0\xB0\xA6\xA3..");
  }
  else if (class >= 25)
  {
    class = 2;
    d.money += 50 + (d.etchics + d.belief) / 60;
    /* 真的很有愛心啦..不過有點小累的樣子.. */
    vmsg("\xAF\x75\xAA\xBA\xAB\xDC\xA6\xB3\xB7\x52\xA4\xDF\xB0\xD5..\xA4\xA3\xB9\x4C\xA6\xB3\xC2\x49\xA4\x70\xB2\xD6\xAA\xBA\xBC\xCB\xA4\x6C..");
  }
  else
  {
    class = 1;
    d.money += 25 + (d.etchics + d.belief) / 65;
    /* 來奉獻不錯..但也不能打混ㄚ.. */
    vmsg("\xA8\xD3\xA9\x5E\xC4\x6D\xA4\xA3\xBF\xF9..\xA6\xFD\xA4\x5D\xA4\xA3\xAF\xE0\xA5\xB4\xB2\x56\xA3\xAB..");
  }

  d.love += rand() % 2 + class;
  d.etchics += rand() % 4 + class;
  d.belief += rand() % 4 + class;

  d.sin -= rand() % 9;
  if (d.sin < 0)
    d.sin = 0;

  d.workF++;
  return 0;
}


int
pip_job_workG()
{
  /* ├────┼──────────────────────┤ */
  /* │地攤    │+ 待人接物 魅力 談吐 速度                   │ */
  /* │        │- 無                                        │ */
  /* ├────┼──────────────────────┤ */

  int class;

  class = d.hp * 100 / d.maxhp - d.tired;
  if ((class = pip_job_function(class, 5, 10, 71)) < 0)
    return 0;

  d.money += 200 + (d.charm + d.speech) * class / 5000;
  /* 擺地攤要躲警察啦..:p */
  vmsg("\xC2\x5C\xA6\x61\xC5\x75\xAD\x6E\xB8\xFA\xC4\xB5\xB9\xEE\xB0\xD5..:p");

  d.toman += rand() % 2;
  d.charm += rand() % 2;
  d.speed += rand() % 2 + 1;
  d.speech += rand() % 2 + 1;

  d.workG++;
  return 0;
}


int
pip_job_workH()
{
  /* ├────┼──────────────────────┤ */
  /* │伐木場  │+ 攻擊力                                    │ */
  /* │        │- 氣質                                      │ */
  /* ├────┼──────────────────────┤ */

  int class;

  if (d.bbtime < 1800 * 1)
  {
    /* 小雞太小了，一歲以後再來吧.. */
    vmsg("\xA4\x70\xC2\xFB\xA4\xD3\xA4\x70\xA4\x46\xA1\x41\xA4\x40\xB7\xB3\xA5\x48\xAB\xE1\xA6\x41\xA8\xD3\xA7\x61..");
    return 0;
  }

  class = d.hp * 100 / d.maxhp - d.tired;
  if ((class = pip_job_function(class, 7, 14, 81)) < 0)
    return 0;

  if (class >= 75)
  {
    class = 4;
    d.money += 350 + (d.maxhp + d.attack) / 50;
    /* 您腕力很好唷.. */
    vmsg("\xB1\x7A\xB5\xC3\xA4\x4F\xAB\xDC\xA6\x6E\xAD\xF2..");
  }
  else if (class >= 50)
  {
    class = 3;
    d.money += 300 + (d.maxhp + d.attack) / 55;
    /* 砍了不少樹喔.. */
    vmsg("\xAC\xE5\xA4\x46\xA4\xA3\xA4\xD6\xBE\xF0\xB3\xE1..");
  }
  else if (class >= 25)
  {
    class = 2;
    d.money += 250 + (d.maxhp + d.attack) / 60;
    /* 普普通通啦..可以更好的.. */
    vmsg("\xB4\xB6\xB4\xB6\xB3\x71\xB3\x71\xB0\xD5..\xA5\x69\xA5\x48\xA7\xF3\xA6\x6E\xAA\xBA..");
  }
  else
  {
    class = 1;
    d.money += 200 + (d.maxhp + d.attack) / 65;
    /* 待加強喔..鍛鍊再來吧.. */
    vmsg("\xAB\xDD\xA5\x5B\xB1\x6A\xB3\xE1..\xC1\xEB\xC1\xE5\xA6\x41\xA8\xD3\xA7\x61..");
  }

  d.attack += rand() % 2 + class;

  d.character -= rand() % 5;
  if (d.character < 0)
    d.character = 0;

  d.workH++;
  return 0;
}


int
pip_job_workI()
{
  /* ├────┼──────────────────────┤ */
  /* │美容院  │+ 藝術 感受                                 │ */
  /* │        │- 無                                        │ */
  /* ├────┼──────────────────────┤ */

  int class;

  if (d.bbtime < 1800 * 1)
  {
    /* 小雞太小了，一歲以後再來吧.. */
    vmsg("\xA4\x70\xC2\xFB\xA4\xD3\xA4\x70\xA4\x46\xA1\x41\xA4\x40\xB7\xB3\xA5\x48\xAB\xE1\xA6\x41\xA8\xD3\xA7\x61..");
    return 0;
  }

  class = d.art / 6 - d.tired;
  if ((class = pip_job_function(class, 5, 10, 91)) < 0)
    return 0;

  if (class >= 75)
  {
    class = 4;
    d.money += 400 + (d.art + d.affect) / 50;
    /* 客人都很喜歡讓您做造型唷.. */
    vmsg("\xAB\xC8\xA4\x48\xB3\xA3\xAB\xDC\xB3\xDF\xC5\x77\xC5\xFD\xB1\x7A\xB0\xB5\xB3\x79\xAB\xAC\xAD\xF2..");
  }
  else if (class >= 50)
  {
    class = 3;
    d.money += 360 + (d.art + d.affect) / 55;
    /* 做的不錯喔..頗有天份.. */
    vmsg("\xB0\xB5\xAA\xBA\xA4\xA3\xBF\xF9\xB3\xE1..\xBB\xE1\xA6\xB3\xA4\xD1\xA5\xF7..");
  }
  else if (class >= 25)
  {
    class = 2;
    d.money += 320 + (d.art + d.affect) / 60;
    /* 馬馬虎虎啦..再加油一點.. */
    vmsg("\xB0\xA8\xB0\xA8\xAA\xEA\xAA\xEA\xB0\xD5..\xA6\x41\xA5\x5B\xAA\x6F\xA4\x40\xC2\x49..");
  }
  else
  {
    class = 1;
    d.money += 250 + (d.art + d.affect) / 65;
    /* 待加強喔..以後再來吧.. */
    vmsg("\xAB\xDD\xA5\x5B\xB1\x6A\xB3\xE1..\xA5\x48\xAB\xE1\xA6\x41\xA8\xD3\xA7\x61..");
  }

  d.art += rand() % 3 + class;
  d.affect += rand() % 2 + class;

  d.workI++;
  return 0;
}


int
pip_job_workJ()
{
  /* ├────┼──────────────────────┤ */
  /* │狩獵區  │+ 攻擊力 速度                               │ */
  /* │        │- 氣質 愛心                                 │ */
  /* ├────┼──────────────────────┤ */

  int class;

  if (d.bbtime < 1800 * 2)
  {
    /* 小雞太小了，二歲以後再來吧.. */
    vmsg("\xA4\x70\xC2\xFB\xA4\xD3\xA4\x70\xA4\x46\xA1\x41\xA4\x47\xB7\xB3\xA5\x48\xAB\xE1\xA6\x41\xA8\xD3\xA7\x61..");
    return 0;
  }

  class = d.hp * 100 / d.maxhp - d.tired;
  if ((class = pip_job_function(class, 6, 13, 101)) < 0)
    return 0;

  if (class >= 75)
  {
    class = 4;
    d.money += 300 + (d.attack + d.speed) / 50;
    /* 您是完美的獵人.. */
    vmsg("\xB1\x7A\xAC\x4F\xA7\xB9\xAC\xFC\xAA\xBA\xC2\x79\xA4\x48..");
  }
  else if (class >= 50)
  {
    class = 3;
    d.money += 270 + (d.attack + d.speed) / 55;
    /* 收獲還不錯喔..可以飽餐一頓了.. */
    vmsg("\xA6\xAC\xC0\xF2\xC1\xD9\xA4\xA3\xBF\xF9\xB3\xE1..\xA5\x69\xA5\x48\xB9\xA1\xC0\x5C\xA4\x40\xB9\x79\xA4\x46..");
  }
  else if (class >= 25)
  {
    class = 2;
    d.money += 240 + (d.attack + d.speed) / 60;
    /* 狩獵是體力與智力的結合.. */
    vmsg("\xAC\xBC\xC2\x79\xAC\x4F\xC5\xE9\xA4\x4F\xBB\x50\xB4\xBC\xA4\x4F\xAA\xBA\xB5\xB2\xA6\x58..");
  }
  else
  {
    class = 1;
    d.money += 210 + (d.attack + d.speed) / 65;
    /* 技術差強人意..再加油喔.. */
    vmsg("\xA7\xDE\xB3\x4E\xAE\x74\xB1\x6A\xA4\x48\xB7\x4E..\xA6\x41\xA5\x5B\xAA\x6F\xB3\xE1..");
  }

  d.attack += rand() % 2 + class;
  d.speed += rand() % 2 + class;

  d.character -= rand() % 5;
  if (d.character < 0)
    d.character = 0;
  d.love -= rand() % 5;
  if (d.love < 0)
    d.love = 0;

  d.workJ++;
  return 0;
}


int
pip_job_workK()
{
  /* ├────┼──────────────────────┤ */
  /* │工地    │+ 防禦力                                    │ */
  /* │        │- 魅力                                      │ */
  /* ├────┼──────────────────────┤ */

  int class;

  if (d.bbtime < 1800 * 2)
  {
    /* 小雞太小了，二歲以後再來吧.. */
    vmsg("\xA4\x70\xC2\xFB\xA4\xD3\xA4\x70\xA4\x46\xA1\x41\xA4\x47\xB7\xB3\xA5\x48\xAB\xE1\xA6\x41\xA8\xD3\xA7\x61..");
    return 0;
  }

  class = d.hp * 100 / d.maxhp - d.tired;
  if ((class = pip_job_function(class, 7, 15, 111)) < 0)
    return 0;

  if (class >= 75)
  {
    class = 4;
    d.money += 250 + (d.maxhp + d.resist) / 50;
    /* 工程很完美..謝謝了.. */
    vmsg("\xA4\x75\xB5\x7B\xAB\xDC\xA7\xB9\xAC\xFC..\xC1\xC2\xC1\xC2\xA4\x46..");
  }
  else if (class >= 50)
  {
    class = 3;
    d.money += 220 + (d.maxhp + d.resist) / 55;
    /* 工程尚稱順利..辛苦了.. */
    vmsg("\xA4\x75\xB5\x7B\xA9\x7C\xBA\xD9\xB6\xB6\xA7\x51..\xA8\xAF\xAD\x57\xA4\x46..");
  }
  else if (class >= 25)
  {
    class = 2;
    d.money += 200 + (d.maxhp + d.resist) / 60;
    /* 工程差強人意..再加油喔.. */
    vmsg("\xA4\x75\xB5\x7B\xAE\x74\xB1\x6A\xA4\x48\xB7\x4E..\xA6\x41\xA5\x5B\xAA\x6F\xB3\xE1..");
  }
  else
  {
    class = 1;
    d.money += 160 + (d.maxhp + d.resist) / 65;
    /* ㄜ..待加強待加強.. */
    vmsg("\xA3\xAD..\xAB\xDD\xA5\x5B\xB1\x6A\xAB\xDD\xA5\x5B\xB1\x6A..");
  }

  d.resist += rand() % 2 + class;

  d.charm -= rand() % 5;
  if (d.charm < 0)
    d.charm = 0;

  d.workK++;
  return 0;
}


int
pip_job_workL()
{
  /* ├────┼──────────────────────┤ */
  /* │墓園    │+ 勇敢 抗魔能力 感受                        │ */
  /* │        │- 魅力                                      │ */
  /* ├────┼──────────────────────┤ */

  int class;

  if (d.bbtime < 1800 * 3)
  {
    /* 小雞太小了，三歲以後再來吧.. */
    vmsg("\xA4\x70\xC2\xFB\xA4\xD3\xA4\x70\xA4\x46\xA1\x41\xA4\x54\xB7\xB3\xA5\x48\xAB\xE1\xA6\x41\xA8\xD3\xA7\x61..");
    return 0;
  }

  class = d.hp * 100 / d.maxhp - d.tired;
  if ((class = pip_job_function(class, 4, 8, 121)) < 0)
    return 0;

  if (class >= 75)
  {
    class = 4;
    d.money += 200 + (d.brave + d.affect) / 50;
    /* 守墓成功喔..多謝了 */
    vmsg("\xA6\x75\xB9\xD3\xA6\xA8\xA5\x5C\xB3\xE1..\xA6\x68\xC1\xC2\xA4\x46");
  }
  else if (class >= 50)
  {
    class = 3;
    d.money += 150 + (d.brave + d.affect) / 55;
    /* 守墓還算成功喔..謝啦.. */
    vmsg("\xA6\x75\xB9\xD3\xC1\xD9\xBA\xE2\xA6\xA8\xA5\x5C\xB3\xE1..\xC1\xC2\xB0\xD5..");
  }
  else if (class >= 25)
  {
    class = 2;
    d.money += 120 + (d.brave + d.affect) / 60;
    /* 守墓還算差強人意喔..加油.. */
    vmsg("\xA6\x75\xB9\xD3\xC1\xD9\xBA\xE2\xAE\x74\xB1\x6A\xA4\x48\xB7\x4E\xB3\xE1..\xA5\x5B\xAA\x6F..");
  }
  else
  {
    class = 1;
    d.money += 80 + (d.brave + d.affect) / 65;
    /* 我也不方便說啥了..請再加油.. */
    vmsg("\xA7\xDA\xA4\x5D\xA4\xA3\xA4\xE8\xAB\x4B\xBB\xA1\xD4\xA3\xA4\x46..\xBD\xD0\xA6\x41\xA5\x5B\xAA\x6F..");
  }

  d.brave += rand() % 4 + class;
  d.immune += rand() % 3 + class;
  d.affect += class;

  d.charm -= rand() % 5;
  if (d.charm < 0)
    d.charm = 0;

  d.workL++;
  return 0;
}


int
pip_job_workM()
{
  /* ├────┼──────────────────────┤ */
  /* │家庭教師│+ 智力 談吐                                 │ */
  /* │        │- 無                                        │ */
  /* ├────┼──────────────────────┤ */

  int class;

  if (d.bbtime < 1800 * 4)
  {
    /* 小雞太小了，四歲以後再來吧.. */
    vmsg("\xA4\x70\xC2\xFB\xA4\xD3\xA4\x70\xA4\x46\xA1\x41\xA5\x7C\xB7\xB3\xA5\x48\xAB\xE1\xA6\x41\xA8\xD3\xA7\x61..");
    return 0;
  }

  class = d.hp * 100 / d.maxhp - d.tired;
  if ((class = pip_job_function(class, 3, 7, 131)) < 0)
    return 0;

  d.money += 50 + (d.wisdom + d.character) * class / 5000;
  /* 家教輕鬆..當然錢就少一點囉 */
  vmsg("\xAE\x61\xB1\xD0\xBB\xB4\xC3\x50..\xB7\xED\xB5\x4D\xBF\xFA\xB4\x4E\xA4\xD6\xA4\x40\xC2\x49\xC5\x6F");

  d.wisdom += rand() % 2 + 3;
  d.speech += rand() % 2 + 1;

  d.workM++;
  return 0;
}


int
pip_job_workN()
{
  /* ├────┼──────────────────────┤ */
  /* │酒店    │+ 魅力 談吐 烹飪                            │ */
  /* │        │- 智力 社交評價                             │ */
  /* ├────┼──────────────────────┤ */

  int class;

  if (d.bbtime < 1800 * 5)
  {
    /* 小雞太小了，五歲以後再來吧.. */
    vmsg("\xA4\x70\xC2\xFB\xA4\xD3\xA4\x70\xA4\x46\xA1\x41\xA4\xAD\xB7\xB3\xA5\x48\xAB\xE1\xA6\x41\xA8\xD3\xA7\x61..");
    return 0;
  }

  class = d.charm / 6 - d.tired;
  if ((class = pip_job_function(class, 5, 11, 141)) < 0)
    return 0;

  if (class >= 75)
  {
    class = 4;
    d.money += 500 + (d.charm + d.speech) / 50;
    /* 很紅唷.. */
    vmsg("\xAB\xDC\xAC\xF5\xAD\xF2..");
  }
  else if (class >= 50)
  {
    class = 3;
    d.money += 400 + (d.charm + d.speech) / 55;
    /* 蠻受歡迎的耶.. */
    vmsg("\xC6\x5A\xA8\xFC\xC5\x77\xAA\xEF\xAA\xBA\xAD\x43..");
  }
  else if (class >= 25)
  {
    class = 2;
    d.money += 300 + (d.charm + d.speech) / 60;
    /* 很平凡啦..但馬馬虎虎.. */
    vmsg("\xAB\xDC\xA5\xAD\xA4\x5A\xB0\xD5..\xA6\xFD\xB0\xA8\xB0\xA8\xAA\xEA\xAA\xEA..");
  }
  else
  {
    class = 1;
    d.money += 200 + (d.charm + d.speech) / 65;
    /* 媚力不夠啦..請加油.. */
    vmsg("\xB4\x41\xA4\x4F\xA4\xA3\xB0\xF7\xB0\xD5..\xBD\xD0\xA5\x5B\xAA\x6F..");
  }

  d.charm += rand() % 3 + class;
  d.speech += rand() % 2 + class;
  d.cook += class;

  d.wisdom -= rand() % 5;
  if (d.wisdom < 0)
    d.wisdom = 0;
  d.social -= rand() % 5;
  if (d.social < 0)
    d.social = 0;

  d.workN++;
  return 0;
}


int
pip_job_workO()
{
  /* ├────┼──────────────────────┤ */
  /* │酒家    │+ 魅力 罪孽                                 │ */
  /* │        │- 待人接物 道德 親子關係 信仰 社交評價      │ */
  /* ├────┼──────────────────────┤ */

  int class;

  if (d.bbtime < 1800 * 5)
  {
    /* 小雞太小了，五歲以後再來吧.. */
    vmsg("\xA4\x70\xC2\xFB\xA4\xD3\xA4\x70\xA4\x46\xA1\x41\xA4\xAD\xB7\xB3\xA5\x48\xAB\xE1\xA6\x41\xA8\xD3\xA7\x61..");
    return 0;
  }

  class = d.charm / 6 - d.tired;
  if ((class = pip_job_function(class, 6, 12, 151)) < 0)
    return 0;

  if (class >= 75)
  {
    class = 4;
    d.money += 600 + (d.charm + d.speech) / 50;
    /* 您是本店的紅牌唷.. */
    vmsg("\xB1\x7A\xAC\x4F\xA5\xBB\xA9\xB1\xAA\xBA\xAC\xF5\xB5\x50\xAD\xF2..");
  }
  else if (class >= 50)
  {
    class = 3;
    d.money += 500 + (d.charm + d.speech) / 55;
    /* 您蠻受歡迎的耶.. */
    vmsg("\xB1\x7A\xC6\x5A\xA8\xFC\xC5\x77\xAA\xEF\xAA\xBA\xAD\x43..");
  }
  else if (class >= 25)
  {
    class = 2;
    d.money += 400 + (d.charm + d.speech) / 60;
    /* 很平凡..但馬馬虎虎啦.. */
    vmsg("\xAB\xDC\xA5\xAD\xA4\x5A..\xA6\xFD\xB0\xA8\xB0\xA8\xAA\xEA\xAA\xEA\xB0\xD5..");
  }
  else
  {
    class = 1;
    d.money += 300 + (d.charm + d.speech) / 65;
    /* 唉..媚力不夠啦.. */
    vmsg("\xAD\xFC..\xB4\x41\xA4\x4F\xA4\xA3\xB0\xF7\xB0\xD5..");
  }

  d.charm += rand() % 4 + class;
  d.sin += rand() % 4 + class;

  d.toman -= rand() % 5;
  if (d.toman < 0)
    d.toman = 0;
  d.etchics -= rand() % 5;
  if (d.etchics < 0)
    d.etchics = 0;
  d.relation -= rand() % 5;
  if (d.relation < 0)
    d.relation = 0;
  d.belief -= rand() % 5;
  if (d.belief < 0)
    d.belief = 0;
  d.social -= rand() % 5;
  if (d.social < 0)
    d.social = 0;

  d.workO++;
  return 0;
}


int
pip_job_workP()
{
  /* ├────┼──────────────────────┤ */
  /* │夜總會  │+ 魅力 談吐 罪孽                            │ */
  /* │        │- 待人接物 氣質 道德 親子關係 信仰 社交評價 │ */
  /* ├────┼──────────────────────┤ */

  int class;

  if (d.bbtime < 1800 * 6)
  {
    /* 小雞太小了，六歲以後再來吧.. */
    vmsg("\xA4\x70\xC2\xFB\xA4\xD3\xA4\x70\xA4\x46\xA1\x41\xA4\xBB\xB7\xB3\xA5\x48\xAB\xE1\xA6\x41\xA8\xD3\xA7\x61..");
    return 0;
  }

  class = (d.charm + d.art - d.belief) / 6 - d.tired;
  if ((class = pip_job_function(class, 6, 12, 161)) < 0)
    return 0;

  if (class >= 75)
  {
    class = 4;
    d.money += 1000 + (d.charm + d.speech) / 50;
    /* 您是本夜總會最閃亮的星星唷.. */
    vmsg("\xB1\x7A\xAC\x4F\xA5\xBB\xA9\x5D\xC1\x60\xB7\x7C\xB3\xCC\xB0\x7B\xAB\x47\xAA\xBA\xAC\x50\xAC\x50\xAD\xF2..");
  }
  else if (class >= 50)
  {
    class = 3;
    d.money += 800 + (d.charm + d.speech) / 55;
    /* 嗯嗯..您蠻受歡迎的耶.. */
    vmsg("\xB6\xE2\xB6\xE2..\xB1\x7A\xC6\x5A\xA8\xFC\xC5\x77\xAA\xEF\xAA\xBA\xAD\x43..");
  }
  else if (class >= 25)
  {
    class = 2;
    d.money += 600 + (d.charm + d.speech) / 60;
    /* 要加油了啦..但普普啦.. */
    vmsg("\xAD\x6E\xA5\x5B\xAA\x6F\xA4\x46\xB0\xD5..\xA6\xFD\xB4\xB6\xB4\xB6\xB0\xD5..");
  }
  else
  {
    class = 1;
    d.money += 400 + (d.charm + d.speech) / 65;
    /* 唉..不行啦.. */
    vmsg("\xAD\xFC..\xA4\xA3\xA6\xE6\xB0\xD5..");
  }

  d.charm += rand() % 5 + class;
  d.speech += rand() % 2 + class;
  d.sin += rand() % 6 + class;

  d.toman -= rand() % 5;
  if (d.toman < 0)
    d.toman = 0;
  d.character -= rand() % 5;
  if (d.character < 0)
    d.character = 0;
  d.etchics -= rand() % 5;
  if (d.etchics < 0)
    d.etchics = 0;
  d.relation -= rand() % 5;
  if (d.relation < 0)
    d.relation = 0;
  d.belief -= rand() % 5;
  if (d.belief < 0)
    d.belief = 0;
  d.social -= rand() % 5;
  if (d.social < 0)
    d.social = 0;

  d.workP++;
  return 0;
}
#endif		/* HAVE_GAME */
