/*-------------------------------------------------------*/
/* theme.h	( NTHU CS MapleBBS Ver 3.10 )		 */
/*-------------------------------------------------------*/
/* target : custom theme				 */
/* create : 02/08/17				 	 */
/* update :   /  /  				 	 */
/*-------------------------------------------------------*/


#ifndef	_THEME_H_
#define	_THEME_H_


/* ----------------------------------------------------- */
/* 基本顏色定義，以利介面修改				 */
/* ----------------------------------------------------- */

#define COLOR1		"\033[1;33;44m"	/* footer/feeter 的前段顏色 */
#define COLOR2		"\033[37;46m"	/* footer/feeter 的後段顏色 */
#define COLOR3		"\033[1;46m"	/* neck 的顏色 */
#define COLOR4		"\033[1;46m"	/* 光棒 的顏色 */
#define COLOR5		"\033[1;33;46m"	/* more 檔頭的標題顏色 */
#define COLOR6		"\033[m"	/* more 檔頭的內容顏色 */
#define COLOR7		"\033[36m"	/* 作者在線上的顏色 */


/* ----------------------------------------------------- */
/* 使用者名單顏色					 */
/* ----------------------------------------------------- */

#define COLOR_NORMAL	""		/* 一般使用者 */
#define COLOR_MYBAD	"\033[1;31m"	/* 壞人 */
#define COLOR_MYGOOD	"\033[1;32m"	/* 我的好友 */
#define COLOR_OGOOD	"\033[1;33m"	/* 與我為友 */
#define COLOR_CLOAK	"\033[1;35m"	/* 隱形 */	/* itoc.註解: 沒用到，要的人請自行加入 ulist_body() */
#define COLOR_SELF	"\033[1;36m"	/* 自己 */
#define COLOR_BOTHGOOD	"\033[1;37m"	/* 互設好友 */
#define COLOR_BRDMATE	"\033[36m"	/* 板伴 */


/* ----------------------------------------------------- */
/* 選單位置						 */
/* ----------------------------------------------------- */

/* itoc.註解: 注意 MENU_XPOS 要 >= MENU_XNOTE + MOVIE_LINES */

#define MENU_XNOTE	12		/* 動態看板由 (12, 0) 開始 */
#define MOVIE_LINES	11		/* 動畫最多有 11 列 */

#define MENU_XPOS	1		/* 選單開始的 (x, y) 座標 */
#define MENU_YPOS	((d_cols >> 1) + 4)


/* ----------------------------------------------------- */
/* 訊息字串：*_neck() 時的 necker 都抓出來定義在這	 */
/* ----------------------------------------------------- */

/* necker 的行數都是二行，從 (1, 0) 到 (2, 80) */

/* 所有的 XZ_* 都有 necker，只是有些在 *_neck()，有些藏在 *_head() */

/* ulist_neck() 及 xpost_head() 的第一行比較特別，不在此定義 */

/* [←]主選單 [→]閱讀 [↑↓]選擇 [c]篇數 [y]載入 [/?]搜尋 [s]看板 [h]說明\n */
#define NECKER_CLASS	"[\xA1\xF6]\xA5\x44\xBF\xEF\xB3\xE6 [\xA1\xF7]\xBE\x5C\xC5\xAA [\xA1\xF4\xA1\xF5]\xBF\xEF\xBE\xDC [c]\xBD\x67\xBC\xC6 [y]\xB8\xFC\xA4\x4A [/?]\xB7\x6A\xB4\x4D [s]\xAC\xDD\xAA\x4F [h]\xBB\xA1\xA9\xFA\n" \
			/*   %s   看  板       類別轉信中   文   敘   述%*s              人氣 板    主%*s    \033[m */ \
			COLOR3 "  %s   \xAC\xDD  \xAA\x4F       \xC3\xFE\xA7\x4F\xC2\xE0\xAB\x48\xA4\xA4   \xA4\xE5   \xB1\xD4   \xAD\x7A%*s              \xA4\x48\xAE\xF0 \xAA\x4F    \xA5\x44%*s    \033[m"

#define NECKER_ULIST	"\n" \
			/*   編號  代號         暱稱%*s                 %-*s               動態        閒置 \033[m */ \
			COLOR3 "  \xBD\x73\xB8\xB9  \xA5\x4E\xB8\xB9         \xBC\xCA\xBA\xD9%*s                 %-*s               \xB0\xCA\xBA\x41        \xB6\xA2\xB8\x6D \033[m"

/* [←]離開 [a]新增 [c]修改 [d]刪除 [m]寄信 [w]水球 [s]整理 [→]查詢 [h]說明\n */
#define NECKER_PAL	"[\xA1\xF6]\xC2\xF7\xB6\x7D [a]\xB7\x73\xBC\x57 [c]\xAD\xD7\xA7\xEF [d]\xA7\x52\xB0\xA3 [m]\xB1\x48\xAB\x48 [w]\xA4\xF4\xB2\x79 [s]\xBE\xE3\xB2\x7A [\xA1\xF7]\xAC\x64\xB8\xDF [h]\xBB\xA1\xA9\xFA\n" \
			/*   編號    代 號         友       誼%*s                                           \033[m */ \
			COLOR3 "  \xBD\x73\xB8\xB9    \xA5\x4E \xB8\xB9         \xA4\xCD       \xBD\xCB%*s                                           \033[m"

/* [←]離開 [a]新增 [d]刪除 [D]區段刪除 [m]寄信 [w]水球 [s]重整 [f]引入 [h]說明\n */
#define NECKER_ALOHA	"[\xA1\xF6]\xC2\xF7\xB6\x7D [a]\xB7\x73\xBC\x57 [d]\xA7\x52\xB0\xA3 [D]\xB0\xCF\xAC\x71\xA7\x52\xB0\xA3 [m]\xB1\x48\xAB\x48 [w]\xA4\xF4\xB2\x79 [s]\xAD\xAB\xBE\xE3 [f]\xA4\xDE\xA4\x4A [h]\xBB\xA1\xA9\xFA\n" \
			/*   編號   上 站 通 知 名 單%*s                                                    \033[m */ \
			COLOR3 "  \xBD\x73\xB8\xB9   \xA4\x57 \xAF\xB8 \xB3\x71 \xAA\xBE \xA6\x57 \xB3\xE6%*s                                                    \033[m"

/* [←]離開 [R]結果 [^P]舉行 [E]修改 [V]預覽 [^Q]改期 [o]名單 [h]說明\n */
#define NECKER_VOTE	"[\xA1\xF6]\xC2\xF7\xB6\x7D [R]\xB5\xB2\xAA\x47 [^P]\xC1\x7C\xA6\xE6 [E]\xAD\xD7\xA7\xEF [V]\xB9\x77\xC4\xFD [^Q]\xA7\xEF\xB4\xC1 [o]\xA6\x57\xB3\xE6 [h]\xBB\xA1\xA9\xFA\n" \
			/*   編號      開票日   主辦人       投  票  宗  旨%*s                              \033[m */ \
			COLOR3 "  \xBD\x73\xB8\xB9      \xB6\x7D\xB2\xBC\xA4\xE9   \xA5\x44\xBF\xEC\xA4\x48       \xA7\xEB  \xB2\xBC  \xA9\x76  \xA6\xAE%*s                              \033[m"

/* [←]離開 [d]刪除 [D]區段刪除 [m]寄信 [M]儲存 [w]水球 [s]更新 [→]查詢 [h]說明\n */
#define NECKER_BMW	"[\xA1\xF6]\xC2\xF7\xB6\x7D [d]\xA7\x52\xB0\xA3 [D]\xB0\xCF\xAC\x71\xA7\x52\xB0\xA3 [m]\xB1\x48\xAB\x48 [M]\xC0\x78\xA6\x73 [w]\xA4\xF4\xB2\x79 [s]\xA7\xF3\xB7\x73 [\xA1\xF7]\xAC\x64\xB8\xDF [h]\xBB\xA1\xA9\xFA\n" \
			/*   編號 代  號       內       容%*s                                          時間 \033[m */ \
			COLOR3 "  \xBD\x73\xB8\xB9 \xA5\x4E  \xB8\xB9       \xA4\xBA       \xAE\x65%*s                                          \xAE\xC9\xB6\xA1 \033[m"

/* [←]離開 [→]進入 [^P]新增 [d]刪除 [c]切換 [C]複製 [^V]貼上 [m]移動 [h]說明\n */
#define NECKER_MF	"[\xA1\xF6]\xC2\xF7\xB6\x7D [\xA1\xF7]\xB6\x69\xA4\x4A [^P]\xB7\x73\xBC\x57 [d]\xA7\x52\xB0\xA3 [c]\xA4\xC1\xB4\xAB [C]\xBD\xC6\xBB\x73 [^V]\xB6\x4B\xA4\x57 [m]\xB2\xBE\xB0\xCA [h]\xBB\xA1\xA9\xFA\n" \
			/*   %s   看  板       類別轉信中   文   敘   述%*s              人氣 板    主%*s    \033[m */ \
			COLOR3 "  %s   \xAC\xDD  \xAA\x4F       \xC3\xFE\xA7\x4F\xC2\xE0\xAB\x48\xA4\xA4   \xA4\xE5   \xB1\xD4   \xAD\x7A%*s              \xA4\x48\xAE\xF0 \xAA\x4F    \xA5\x44%*s    \033[m"

/* [←]離開 [→]閱讀 [^P]申請 [d]刪除 [o]開板 [h]說明\n */
#define NECKER_COSIGN	"[\xA1\xF6]\xC2\xF7\xB6\x7D [\xA1\xF7]\xBE\x5C\xC5\xAA [^P]\xA5\xD3\xBD\xD0 [d]\xA7\x52\xB0\xA3 [o]\xB6\x7D\xAA\x4F [h]\xBB\xA1\xA9\xFA\n" \
			/*   編號   日 期  舉辦人       看  板  標  題%*s                                   \033[m */ \
			COLOR3 "  \xBD\x73\xB8\xB9   \xA4\xE9 \xB4\xC1  \xC1\x7C\xBF\xEC\xA4\x48       \xAC\xDD  \xAA\x4F  \xBC\xD0  \xC3\x44%*s                                   \033[m"

/* [←]離開 [→]瀏覽 [o]點歌到看板 [m]點歌到信箱 [Enter]瀏覽 [h]說明\n */
#define NECKER_SONG	"[\xA1\xF6]\xC2\xF7\xB6\x7D [\xA1\xF7]\xC2\x73\xC4\xFD [o]\xC2\x49\xBA\x71\xA8\xEC\xAC\xDD\xAA\x4F [m]\xC2\x49\xBA\x71\xA8\xEC\xAB\x48\xBD\x63 [Enter]\xC2\x73\xC4\xFD [h]\xBB\xA1\xA9\xFA\n" \
			/*   編號     主              題%*s                            [編      選] [日  期]\033[m */ \
			COLOR3 "  \xBD\x73\xB8\xB9     \xA5\x44              \xC3\x44%*s                            [\xBD\x73      \xBF\xEF] [\xA4\xE9  \xB4\xC1]\033[m"

/* [←]離開 [→]閱讀 [h]說明\n */
#define NECKER_NEWS	"[\xA1\xF6]\xC2\xF7\xB6\x7D [\xA1\xF7]\xBE\x5C\xC5\xAA [h]\xBB\xA1\xA9\xFA\n" \
			/*   編號    日 期 作  者       新  聞  標  題%*s                                   \033[m */ \
			COLOR3 "  \xBD\x73\xB8\xB9    \xA4\xE9 \xB4\xC1 \xA7\x40  \xAA\xCC       \xB7\x73  \xBB\x44  \xBC\xD0  \xC3\x44%*s                                   \033[m"

#define NECKER_XPOST	"\n" \
			/*   編號    日 期 作  者       文  章  標  題%*s                            評:%s  \033[m */ \
			COLOR3 "  \xBD\x73\xB8\xB9    \xA4\xE9 \xB4\xC1 \xA7\x40  \xAA\xCC       \xA4\xE5  \xB3\xB9  \xBC\xD0  \xC3\x44%*s                            \xB5\xFB:%s  \033[m"

/* [←]離開 [→,r]讀信 [d]刪除 [R,y](群組)回信 [s]寄信 [x]轉錄 [X]轉達 [h]說明\n */
#define NECKER_MBOX	"[\xA1\xF6]\xC2\xF7\xB6\x7D [\xA1\xF7,r]\xC5\xAA\xAB\x48 [d]\xA7\x52\xB0\xA3 [R,y](\xB8\x73\xB2\xD5)\xA6\x5E\xAB\x48 [s]\xB1\x48\xAB\x48 [x]\xC2\xE0\xBF\xFD [X]\xC2\xE0\xB9\x46 [h]\xBB\xA1\xA9\xFA\n" \
			/*   編號   日 期 作  者       信  件  標  題%*s                                    \033[m */ \
			COLOR3 "  \xBD\x73\xB8\xB9   \xA4\xE9 \xB4\xC1 \xA7\x40  \xAA\xCC       \xAB\x48  \xA5\xF3  \xBC\xD0  \xC3\x44%*s                                    \033[m"

/* [←]離開 [→]閱讀 [^P]發表 [b]進板畫面 [d]刪除 [V]投票 [TAB]精華區 [h]說明\n */
#define NECKER_POST	"[\xA1\xF6]\xC2\xF7\xB6\x7D [\xA1\xF7]\xBE\x5C\xC5\xAA [^P]\xB5\x6F\xAA\xED [b]\xB6\x69\xAA\x4F\xB5\x65\xAD\xB1 [d]\xA7\x52\xB0\xA3 [V]\xA7\xEB\xB2\xBC [TAB]\xBA\xEB\xB5\xD8\xB0\xCF [h]\xBB\xA1\xA9\xFA\n" \
			/*   編號    日 期 作  者       文  章  標  題%*s                 評:%s  人氣:%-4d  \033[m */ \
			COLOR3 "  \xBD\x73\xB8\xB9    \xA4\xE9 \xB4\xC1 \xA7\x40  \xAA\xCC       \xA4\xE5  \xB3\xB9  \xBC\xD0  \xC3\x44%*s                 \xB5\xFB:%s  \xA4\x48\xAE\xF0:%-4d  \033[m"

/* [←]離開 [→]瀏覽 [B]模式 [C]暫存 [F]轉寄 [d]刪除 [h]說明  %s\n */
#define NECKER_GEM	"[\xA1\xF6]\xC2\xF7\xB6\x7D [\xA1\xF7]\xC2\x73\xC4\xFD [B]\xBC\xD2\xA6\xA1 [C]\xBC\xC8\xA6\x73 [F]\xC2\xE0\xB1\x48 [d]\xA7\x52\xB0\xA3 [h]\xBB\xA1\xA9\xFA  %s\n" \
			/*   編號     主              題%*s                            [編      選] [日  期]\033[m */ \
			COLOR3 "  \xBD\x73\xB8\xB9     \xA5\x44              \xC3\x44%*s                            [\xBD\x73      \xBF\xEF] [\xA4\xE9  \xB4\xC1]\033[m"

/* 以下這些則是一些類 XZ_* 結構的 necker */

/* [↑/↓]上下 [PgUp/PgDn]上下頁 [Home/End]首尾 [→]投票 [←][q]離開\n */
#define NECKER_VOTEALL	"[\xA1\xF4/\xA1\xF5]\xA4\x57\xA4\x55 [PgUp/PgDn]\xA4\x57\xA4\x55\xAD\xB6 [Home/End]\xAD\xBA\xA7\xC0 [\xA1\xF7]\xA7\xEB\xB2\xBC [\xA1\xF6][q]\xC2\xF7\xB6\x7D\n" \
			/*   編號   看  板       類別轉信中   文   敘   述%*s                  板    主%*s     \033[m */ \
			COLOR3 "  \xBD\x73\xB8\xB9   \xAC\xDD  \xAA\x4F       \xC3\xFE\xA7\x4F\xC2\xE0\xAB\x48\xA4\xA4   \xA4\xE5   \xB1\xD4   \xAD\x7A%*s                  \xAA\x4F    \xA5\x44%*s     \033[m"

/* [←]離開 [C]換頁 [1]新增 [2]刪除 [3]全刪 [4]總計\n */
#define NECKER_CREDIT	"[\xA1\xF6]\xC2\xF7\xB6\x7D [C]\xB4\xAB\xAD\xB6 [1]\xB7\x73\xBC\x57 [2]\xA7\x52\xB0\xA3 [3]\xA5\xFE\xA7\x52 [4]\xC1\x60\xAD\x70\n" \
			/*   編號   日  期   收支  金  額  分類     說  明%*s                               \033[m */ \
			COLOR3 "  \xBD\x73\xB8\xB9   \xA4\xE9  \xB4\xC1   \xA6\xAC\xA4\xE4  \xAA\xF7  \xC3\x42  \xA4\xC0\xC3\xFE     \xBB\xA1  \xA9\xFA%*s                               \033[m"

/* [←]離開 [→]閱讀 [^P]新增 [d]刪除 [T]標題 [E]編輯 [m]移動\n */
#define NECKER_HELP	"[\xA1\xF6]\xC2\xF7\xB6\x7D [\xA1\xF7]\xBE\x5C\xC5\xAA [^P]\xB7\x73\xBC\x57 [d]\xA7\x52\xB0\xA3 [T]\xBC\xD0\xC3\x44 [E]\xBD\x73\xBF\xE8 [m]\xB2\xBE\xB0\xCA\n" \
			/*   編號    檔 案         標       題%*s                                           \033[m */ \
			COLOR3 "  \xBD\x73\xB8\xB9    \xC0\xC9 \xAE\xD7         \xBC\xD0       \xC3\x44%*s                                           \033[m"

/* [←]離開 [^P]新增 [d]刪除 [E]編輯 [/]搜尋 [Enter]詳細\n */
#define NECKER_INNBBS	"[\xA1\xF6]\xC2\xF7\xB6\x7D [^P]\xB7\x73\xBC\x57 [d]\xA7\x52\xB0\xA3 [E]\xBD\x73\xBF\xE8 [/]\xB7\x6A\xB4\x4D [Enter]\xB8\xD4\xB2\xD3\n" \
			/*   編號            內         容%*s                                               \033[m */ \
			COLOR3 "  \xBD\x73\xB8\xB9            \xA4\xBA         \xAE\x65%*s                                               \033[m"


/* ----------------------------------------------------- */
/* 訊息字串：more() 時的 footer 都抓出來定義在這	 */
/* ----------------------------------------------------- */

/* itoc.010914.註解: 單一篇，所以叫 FOOTER，都是 78 char */

/* itoc.010821: 注意 \\ 是 \，最後別漏了一個空白鍵 :p */

#define FOOTER_POST	\
/*  文章選讀  */ \
/*  (ry)回應 (=\\[]<>-+;'`)主題 (|?QA)搜尋標題作者 (kj)上下篇 (C)暫存    */ \
COLOR1 " \xA4\xE5\xB3\xB9\xBF\xEF\xC5\xAA " COLOR2 " (ry)\xA6\x5E\xC0\xB3 (=\\[]<>-+;'`)\xA5\x44\xC3\x44 (|?QA)\xB7\x6A\xB4\x4D\xBC\xD0\xC3\x44\xA7\x40\xAA\xCC (kj)\xA4\x57\xA4\x55\xBD\x67 (C)\xBC\xC8\xA6\x73   "

#define FOOTER_MAILER	\
/*  魚雁往返  */ \
/*  (ry)回信/群組 (X)轉達 (d)刪除 (m)標記 (C)暫存 (=\\[]<>-+;'`|?QAkj)   */ \
COLOR1 " \xB3\xBD\xB6\xAD\xA9\xB9\xAA\xF0 " COLOR2 " (ry)\xA6\x5E\xAB\x48/\xB8\x73\xB2\xD5 (X)\xC2\xE0\xB9\x46 (d)\xA7\x52\xB0\xA3 (m)\xBC\xD0\xB0\x4F (C)\xBC\xC8\xA6\x73 (=\\[]<>-+;'`|?QAkj)  "

#define FOOTER_GEM	\
/*  精華選讀  */ \
/*  (=\\[]<>-+;'`)主題 (|?QA)搜尋標題作者 (kj)上下篇 (↑↓←)上下離開    */ \
COLOR1 " \xBA\xEB\xB5\xD8\xBF\xEF\xC5\xAA " COLOR2 " (=\\[]<>-+;'`)\xA5\x44\xC3\x44 (|?QA)\xB7\x6A\xB4\x4D\xBC\xD0\xC3\x44\xA7\x40\xAA\xCC (kj)\xA4\x57\xA4\x55\xBD\x67 (\xA1\xF4\xA1\xF5\xA1\xF6)\xA4\x57\xA4\x55\xC2\xF7\xB6\x7D   "

#ifdef HAVE_GAME
#define FOOTER_TALK	\
/*  交談模式  */ \
/*  (^O)對奕模式 (^C,^D)結束交談 (^T)切換呼叫器 (^Z)快捷列表 (^G)嗶嗶   */ \
COLOR1 " \xA5\xE6\xBD\xCD\xBC\xD2\xA6\xA1 " COLOR2 " (^O)\xB9\xEF\xAB\xB3\xBC\xD2\xA6\xA1 (^C,^D)\xB5\xB2\xA7\xF4\xA5\xE6\xBD\xCD (^T)\xA4\xC1\xB4\xAB\xA9\x49\xA5\x73\xBE\xB9 (^Z)\xA7\xD6\xB1\xB6\xA6\x43\xAA\xED (^G)\xB9\xCD\xB9\xCD  "
#else
#define FOOTER_TALK	\
/*  交談模式  */ \
/*  (^C,^D)結束交談 (^T)切換呼叫器 (^Z)快捷列表 (^G)嗶嗶 (^Y)清除       */ \
COLOR1 " \xA5\xE6\xBD\xCD\xBC\xD2\xA6\xA1 " COLOR2 " (^C,^D)\xB5\xB2\xA7\xF4\xA5\xE6\xBD\xCD (^T)\xA4\xC1\xB4\xAB\xA9\x49\xA5\x73\xBE\xB9 (^Z)\xA7\xD6\xB1\xB6\xA6\x43\xAA\xED (^G)\xB9\xCD\xB9\xCD (^Y)\xB2\x4D\xB0\xA3      "
#endif

#define FOOTER_COSIGN	\
/*  連署機制  */ \
/*  (ry)加入連署 (kj)上下篇 (↑↓←)上下離開 (h)說明                    */ \
COLOR1 " \xB3\x73\xB8\x70\xBE\xF7\xA8\xEE " COLOR2 " (ry)\xA5\x5B\xA4\x4A\xB3\x73\xB8\x70 (kj)\xA4\x57\xA4\x55\xBD\x67 (\xA1\xF4\xA1\xF5\xA1\xF6)\xA4\x57\xA4\x55\xC2\xF7\xB6\x7D (h)\xBB\xA1\xA9\xFA                   " 

#define FOOTER_MORE	\
/*  瀏覽 P.%d (%d%%)  */ \
/*  (h)說明 [PgUp][PgDn][0][$]移動 (/n)搜尋 (C)暫存 (←q)結束  */ \
COLOR1 " \xC2\x73\xC4\xFD P.%d (%d%%) " COLOR2 " (h)\xBB\xA1\xA9\xFA [PgUp][PgDn][0][$]\xB2\xBE\xB0\xCA (/n)\xB7\x6A\xB4\x4D (C)\xBC\xC8\xA6\x73 (\xA1\xF6q)\xB5\xB2\xA7\xF4 "

#define FOOTER_VEDIT	\
/*  (^Z)說明 (^W)符號 (^L)重繪 (^X)檔案處理 ║%s│%s║%5d:%3d    \033[m */ \
COLOR1 " %s " COLOR2 " (^Z)\xBB\xA1\xA9\xFA (^W)\xB2\xC5\xB8\xB9 (^L)\xAD\xAB\xC3\xB8 (^X)\xC0\xC9\xAE\xD7\xB3\x42\xB2\x7A \xF9\xF8%s\xA2\x78%s\xF9\xF8%5d:%3d    \033[m"


/* ----------------------------------------------------- */
/* 訊息字串：xo_foot() 時的 feeter 都抓出來定義在這      */
/* ----------------------------------------------------- */


/* itoc.010914.註解: 列表多篇，所以叫 FEETER，都是 78 char */

#define FEETER_CLASS	\
/*  看板選擇  */ \
/*  (c)新文章 (vV)標記已讀未讀 (y)全部列出 (z)選訂 (A)全域搜尋 (S)排序  */ \
COLOR1 " \xAC\xDD\xAA\x4F\xBF\xEF\xBE\xDC " COLOR2 " (c)\xB7\x73\xA4\xE5\xB3\xB9 (vV)\xBC\xD0\xB0\x4F\xA4\x77\xC5\xAA\xA5\xBC\xC5\xAA (y)\xA5\xFE\xB3\xA1\xA6\x43\xA5\x58 (z)\xBF\xEF\xAD\x71 (A)\xA5\xFE\xB0\xEC\xB7\x6A\xB4\x4D (S)\xB1\xC6\xA7\xC7 "

#define FEETER_ULIST	\
/*  網友列表  */ \
/*  (f)好友 (t)聊天 (q)查詢 (ad)交友 (m)寄信 (w)水球 (s)更新 (TAB)切換  */ \
COLOR1 " \xBA\xF4\xA4\xCD\xA6\x43\xAA\xED " COLOR2 " (f)\xA6\x6E\xA4\xCD (t)\xB2\xE1\xA4\xD1 (q)\xAC\x64\xB8\xDF (ad)\xA5\xE6\xA4\xCD (m)\xB1\x48\xAB\x48 (w)\xA4\xF4\xB2\x79 (s)\xA7\xF3\xB7\x73 (TAB)\xA4\xC1\xB4\xAB "

#define FEETER_PAL	\
/*  呼朋引伴  */ \
/*  (a)新增 (d)刪除 (c)友誼 (m)寄信 (f)引入好友 (r^Q)查詢 (s)更新       */ \
COLOR1 " \xA9\x49\xAA\x42\xA4\xDE\xA6\xF1 " COLOR2 " (a)\xB7\x73\xBC\x57 (d)\xA7\x52\xB0\xA3 (c)\xA4\xCD\xBD\xCB (m)\xB1\x48\xAB\x48 (f)\xA4\xDE\xA4\x4A\xA6\x6E\xA4\xCD (r^Q)\xAC\x64\xB8\xDF (s)\xA7\xF3\xB7\x73      "

#define FEETER_ALOHA	\
/*  上站通知  */ \
/*  (a)新增 (d)刪除 (D)區段刪除 (f)引入好友 (r^Q)查詢 (s)更新           */ \
COLOR1 " \xA4\x57\xAF\xB8\xB3\x71\xAA\xBE " COLOR2 " (a)\xB7\x73\xBC\x57 (d)\xA7\x52\xB0\xA3 (D)\xB0\xCF\xAC\x71\xA7\x52\xB0\xA3 (f)\xA4\xDE\xA4\x4A\xA6\x6E\xA4\xCD (r^Q)\xAC\x64\xB8\xDF (s)\xA7\xF3\xB7\x73          "

#define FEETER_VOTE	\
/*  看板投票  */ \
/*  (→/r/v)投票 (R)結果 (^P)新增投票 (E)修改 (V)預覽 (b)開票 (o)名單   */ \
COLOR1 " \xAC\xDD\xAA\x4F\xA7\xEB\xB2\xBC " COLOR2 " (\xA1\xF7/r/v)\xA7\xEB\xB2\xBC (R)\xB5\xB2\xAA\x47 (^P)\xB7\x73\xBC\x57\xA7\xEB\xB2\xBC (E)\xAD\xD7\xA7\xEF (V)\xB9\x77\xC4\xFD (b)\xB6\x7D\xB2\xBC (o)\xA6\x57\xB3\xE6  "

#define FEETER_BMW	\
/*  水球回顧  */ \
/*  (d)刪除 (D)區段刪除 (m)寄信 (w)水球 (^R)回訊 (^Q)查詢 (s)更新       */ \
COLOR1 " \xA4\xF4\xB2\x79\xA6\x5E\xC5\x55 " COLOR2 " (d)\xA7\x52\xB0\xA3 (D)\xB0\xCF\xAC\x71\xA7\x52\xB0\xA3 (m)\xB1\x48\xAB\x48 (w)\xA4\xF4\xB2\x79 (^R)\xA6\x5E\xB0\x54 (^Q)\xAC\x64\xB8\xDF (s)\xA7\xF3\xB7\x73      "

#define FEETER_MF	\
/*  最愛看板  */ \
/*  (^P)新增 (Cg)複製 (p^V)貼上 (d)刪除 (c)新文章 (vV)標記已讀/未讀     */ \
COLOR1 " \xB3\xCC\xB7\x52\xAC\xDD\xAA\x4F " COLOR2 " (^P)\xB7\x73\xBC\x57 (Cg)\xBD\xC6\xBB\x73 (p^V)\xB6\x4B\xA4\x57 (d)\xA7\x52\xB0\xA3 (c)\xB7\x73\xA4\xE5\xB3\xB9 (vV)\xBC\xD0\xB0\x4F\xA4\x77\xC5\xAA/\xA5\xBC\xC5\xAA    "

#define FEETER_COSIGN	\
/*  連署小站  */ \
/*  (r)讀取 (y)回應 (^P)發表 (d)刪除 (o)開板 (c)關閉 (E)編輯 (B)設定    */ \
COLOR1 " \xB3\x73\xB8\x70\xA4\x70\xAF\xB8 " COLOR2 " (r)\xC5\xAA\xA8\xFA (y)\xA6\x5E\xC0\xB3 (^P)\xB5\x6F\xAA\xED (d)\xA7\x52\xB0\xA3 (o)\xB6\x7D\xAA\x4F (c)\xC3\xF6\xB3\xAC (E)\xBD\x73\xBF\xE8 (B)\xB3\x5D\xA9\x77   "

#define FEETER_SONG	\
/*  點歌系統  */ \
/*  (r)讀取 (o)點歌到看板 (m)點歌到信箱 (E)編輯檔案 (T)編輯標題         */ \
COLOR1 " \xC2\x49\xBA\x71\xA8\x74\xB2\xCE " COLOR2 " (r)\xC5\xAA\xA8\xFA (o)\xC2\x49\xBA\x71\xA8\xEC\xAC\xDD\xAA\x4F (m)\xC2\x49\xBA\x71\xA8\xEC\xAB\x48\xBD\x63 (E)\xBD\x73\xBF\xE8\xC0\xC9\xAE\xD7 (T)\xBD\x73\xBF\xE8\xBC\xD0\xC3\x44        "

#define FEETER_NEWS	\
/*  新聞點選  */ \
/*  (↑/↓)上下 (PgUp/PgDn)上下頁 (Home/End)首尾 (→r)選取 (←)(q)離開  */ \
COLOR1 " \xB7\x73\xBB\x44\xC2\x49\xBF\xEF " COLOR2 " (\xA1\xF4/\xA1\xF5)\xA4\x57\xA4\x55 (PgUp/PgDn)\xA4\x57\xA4\x55\xAD\xB6 (Home/End)\xAD\xBA\xA7\xC0 (\xA1\xF7r)\xBF\xEF\xA8\xFA (\xA1\xF6)(q)\xC2\xF7\xB6\x7D "

#define FEETER_XPOST	\
/*  串列搜尋  */ \
/*  (y)回應 (x)轉錄 (m)標記 (d)刪除 (^P)發表 (^Q)查詢作者 (t)標籤       */ \
COLOR1 " \xA6\xEA\xA6\x43\xB7\x6A\xB4\x4D " COLOR2 " (y)\xA6\x5E\xC0\xB3 (x)\xC2\xE0\xBF\xFD (m)\xBC\xD0\xB0\x4F (d)\xA7\x52\xB0\xA3 (^P)\xB5\x6F\xAA\xED (^Q)\xAC\x64\xB8\xDF\xA7\x40\xAA\xCC (t)\xBC\xD0\xC5\xD2      "

#define FEETER_MBOX	\
/*  信信相惜  */ \
/*  (y)回信 (F/X/x)轉寄/轉達/轉錄 (d)刪除 (D)區段刪除 (m)標記 (E)編輯   */ \
COLOR1 " \xAB\x48\xAB\x48\xAC\xDB\xB1\xA4 " COLOR2 " (y)\xA6\x5E\xAB\x48 (F/X/x)\xC2\xE0\xB1\x48/\xC2\xE0\xB9\x46/\xC2\xE0\xBF\xFD (d)\xA7\x52\xB0\xA3 (D)\xB0\xCF\xAC\x71\xA7\x52\xB0\xA3 (m)\xBC\xD0\xB0\x4F (E)\xBD\x73\xBF\xE8  "

#define FEETER_POST	\
/*  文章列表  */ \
/*  (ry)回信 (S/a)搜尋/標題/作者 (~G)串列搜尋 (x)轉錄 (V)投票 (u)新聞   */ \
COLOR1 " \xA4\xE5\xB3\xB9\xA6\x43\xAA\xED " COLOR2 " (ry)\xA6\x5E\xAB\x48 (S/a)\xB7\x6A\xB4\x4D/\xBC\xD0\xC3\x44/\xA7\x40\xAA\xCC (~G)\xA6\xEA\xA6\x43\xB7\x6A\xB4\x4D (x)\xC2\xE0\xBF\xFD (V)\xA7\xEB\xB2\xBC (u)\xB7\x73\xBB\x44  "

#define FEETER_GEM	\
/*  看板精華  */ \
/*  (^P/a/f)新增/文章/目錄 (E)編輯 (T)標題 (m)移動 (c)複製 (p^V)貼上    */ \
COLOR1 " \xAC\xDD\xAA\x4F\xBA\xEB\xB5\xD8 " COLOR2 " (^P/a/f)\xB7\x73\xBC\x57/\xA4\xE5\xB3\xB9/\xA5\xD8\xBF\xFD (E)\xBD\x73\xBF\xE8 (T)\xBC\xD0\xC3\x44 (m)\xB2\xBE\xB0\xCA (c)\xBD\xC6\xBB\x73 (p^V)\xB6\x4B\xA4\x57   "

#define FEETER_VOTEALL	\
/*  投票中心  */ \
/*  (↑/↓)上下 (PgUp/PgDn)上下頁 (Home/End)首尾 (→)投票 (←)(q)離開   */ \
COLOR1 " \xA7\xEB\xB2\xBC\xA4\xA4\xA4\xDF " COLOR2 " (\xA1\xF4/\xA1\xF5)\xA4\x57\xA4\x55 (PgUp/PgDn)\xA4\x57\xA4\x55\xAD\xB6 (Home/End)\xAD\xBA\xA7\xC0 (\xA1\xF7)\xA7\xEB\xB2\xBC (\xA1\xF6)(q)\xC2\xF7\xB6\x7D  "

#define FEETER_HELP	\
/*  說明文件  */ \
/*  (↑/↓)上下 (PgUp/PgDn)上下頁 (Home/End)首尾 (→r)瀏覽 (←)(q)離開  */ \
COLOR1 " \xBB\xA1\xA9\xFA\xA4\xE5\xA5\xF3 " COLOR2 " (\xA1\xF4/\xA1\xF5)\xA4\x57\xA4\x55 (PgUp/PgDn)\xA4\x57\xA4\x55\xAD\xB6 (Home/End)\xAD\xBA\xA7\xC0 (\xA1\xF7r)\xC2\x73\xC4\xFD (\xA1\xF6)(q)\xC2\xF7\xB6\x7D "

#define FEETER_INNBBS	\
/*  轉信設定  */ \
/*  (↑/↓)上下 (PgUp/PgDn)上下頁 (Home/End)首尾 (←)(q)離開            */ \
COLOR1 " \xC2\xE0\xAB\x48\xB3\x5D\xA9\x77 " COLOR2 " (\xA1\xF4/\xA1\xF5)\xA4\x57\xA4\x55 (PgUp/PgDn)\xA4\x57\xA4\x55\xAD\xB6 (Home/End)\xAD\xBA\xA7\xC0 (\xA1\xF6)(q)\xC2\xF7\xB6\x7D           "


/* ----------------------------------------------------- */
/* 站台來源簽名						 */
/* ----------------------------------------------------- */

/* itoc: 建議 banner 不要超過三行，過長的站簽可能會造成某些使用者的反感 */

#define EDIT_BANNER	"\n--\n" \
			/* \033[36m※ 來自： */ \
			"\033[36m\xA1\xB0 \xA8\xD3\xA6\xDB\xA1\x47"BBSNAME" ("MYHOSTNAME")\033[m\n" \
			/* \033[1;31m㊣ \033[36mPost by \033[37m%-12s \033[36mfrom \033[33m%s\033[m\n */ \
			"\033[1;31m\xA1\xC0 \033[36mPost by \033[37m%-12s \033[36mfrom \033[33m%s\033[m\n"

/* \033[1;31m㊣ \033[36mEdit by \033[37m%-12s \033[36mat \033[33m%s\033[m\n */
#define MODIFY_BANNER	"\033[1;31m\xA1\xC0 \033[36mEdit by \033[37m%-12s \033[36mat \033[33m%s\033[m\n"


/* ----------------------------------------------------- */
/* 其他訊息字串						 */
/* ----------------------------------------------------- */

/* \033[1;46m                        ★ 請按 (Space/Return) 繼續 ★                        \033[m */
#define VMSG_NULL	"\033[1;46m                        \xA1\xB9 \xBD\xD0\xAB\xF6 (Space/Return) \xC4\x7E\xC4\xF2 \xA1\xB9                        \033[m"

/* \033[36m● */
#define ICON_UNREAD_BRD		"\033[36m\xA1\xB4"		/* 未讀看板 */
#define ICON_READ_BRD		"  "			/* 已讀看板 */

/* \033[1;31m賭\033[m */
#define ICON_GAMBLED_BRD	"\033[1;31m\xBD\xE4\033[m"	/* 舉行賭盤中的看板 */
/* \033[1;33m投\033[m */
#define ICON_VOTED_BRD		"\033[1;33m\xA7\xEB\033[m"	/* 舉行投票中的看板 */
/* ☆ */
#define ICON_NOTRAN_BRD		"\xA1\xB8"			/* 不轉信板 */
/* ★ */
#define ICON_TRAN_BRD		"\xA1\xB9"			/* 轉信板 */

#define TOKEN_ZAP_BRD		'-'			/* zap 板 */
#define TOKEN_FRIEND_BRD	'-'			/* 好友板 */
#define TOKEN_SECRET_BRD	')'			/* 秘密板 */

#endif				/* _THEME_H_ */
