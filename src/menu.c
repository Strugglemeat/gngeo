/*
 *  Copyright (C) 2021 Steward Fu
 *  Copyright (C) 2001 Peponas Mathieu
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <math.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_thread.h>

#include "drv.h"
#include "emu.h"
#include "roms.h"
#include "menu.h"
#include "conf.h"
#include "video.h"
#include "sound.h"
#include "event.h"
#include "state.h"
#include "video.h"
#include "sound.h"
#include "memory.h"
#include "screen.h"
#include "gnutil.h"
#include "messages.h"
#include "frame_skip.h"
#include "hex_arrow_down.h"
#include "hex_arrow_left.h"
#include "hex_arrow_right.h"
#include "hex_arrow_up.h"
#include "hex_back.h"
#include "hex_gngeo.h"
#include "hex_gngeo_mask.h"
#include "hex_font_large.h"
#include "hex_font_small.h"

#define ICOL_BLACK  0
#define ICOL_RED    1
#define ICOL_GREEN  2
#define ICOL_GREY_L 3
#define ICOL_GREY_M 4
#define ICOL_GREY_D 5
#define ICOL_MAX   6

#define COL_BLACK  0x000000
#define COL_RED    0x1010FF
#define COL_GREEN  0x10FF10
#define COL_GREY_L 0xA0A0A0
#define COL_GREY_M 0x808080
#define COL_GREY_D 0x404040

static int pbar_y;

char *abxylr_btn_string[] = {"None", "A", "B", "C", "D", "A+B", "A+C", "A+D", "B+C", "B+D", "C+D", "A+B+C", "A+B+D", "A+C+D", "B+C+D", "A+B+C+D"};
uint32_t font_color[] = { COL_BLACK, COL_RED, COL_GREEN, COL_GREY_L, COL_GREY_M, COL_GREY_D};

typedef struct GNFONT {
  SDL_Surface *bmp;
  SDL_Surface *cbmp[ICOL_MAX];
  uint8_t ysize;
  uint16_t xpos[128 - 32];
  uint8_t xsize[128 - 32];
  Sint8 pad;
} GNFONT;

extern SDL_Surface *buffer;

static SDL_Surface *menu_buf;
static SDL_Surface *menu_back_1;
static SDL_Surface *back;
static GNFONT *sfont;
static GNFONT *mfont;
static SDL_Surface *gngeo_logo, *gngeo_mask, *pbar_logo;

static SDL_Surface *arrow_l, *arrow_r, *arrow_u, *arrow_d;
//static int interp;

#define MENU_BIG   0
#define MENU_SMALL 1

#define MENU_TITLE_X (24 + 30)
#define MENU_TITLE_Y (16 + 20)

#define MENU_TEXT_X (24 + 26)
#define MENU_TEXT_Y (16 + 43)

#define MENU_TEXT_X_END (24 + 277)
#define MENU_TEXT_Y_END (16 + 198)

#define ALIGN_LEFT   (1<<16)
#define ALIGN_RIGHT  (2<<16)
#define ALIGN_CENTER (3<<16)
#define ALIGN_UP     (1<<16)
#define ALIGN_DOWN   (2<<16)

#define MENU_CLOSE        1
#define MENU_STAY         0
#define MENU_EXIT         2
#define MENU_RETURNTOGAME 3

static GN_MENU *main_menu=NULL;
static GN_MENU *option_menu=NULL;
static GN_MENU *btn_menu[6]={0};
static GN_MENU *yesno_menu=NULL;
static GN_MENU *rbrowser_menu=NULL;

static char *romlist[] = {
  "/2020bb.zip",   "/2020bba.zip",  "/2020bbh.zip",  "/3countb.zip",   "/alpham2.zip",   "/androdun.zip",  "/aodk.zip",      "/aof.zip",
  "/aof2.zip",     "/aof2a.zip",    "/aof3.zip",     "/aof3k.zip",     "/bakatono.zip",  "/bangbead.zip",  "/bjourney.zip",  "/blazstar.zip",
  "/breakers.zip", "/breakrev.zip", "/bstars.zip",   "/bstars2.zip",   "/burningf.zip",  "/burningfh.zip", "/crsword.zip",   "/ct2k3sa.zip",
  "/ct2k3sp.zip",  "/cthd2003.zip", "/ctomaday.zip", "/cyberlip.zip",  "/diggerma.zip",  "/doubledr.zip",  "/eightman.zip",  "/fatfursa.zip",
  "/fatfursp.zip", "/fatfury1.zip", "/fatfury2.zip", "/fatfury3.zip",  "/fbfrenzy.zip",  "/fightfev.zip",  "/fightfeva.zip", "/flipshot.zip",
  "/fswords.zip",  "/galaxyfg.zip", "/ganryu.zip",   "/garou.zip",     "/garoubl.zip",   "/garouo.zip",    "/garoup.zip",    "/ghostlop.zip",
  "/goalx3.zip",   "/gowcaizr.zip", "/gpilots.zip",  "/gpilotsh.zip",  "/gururin.zip",   "/irrmaze.zip",   "/janshin.zip",   "/jockeygp.zip",
  "/joyjoy.zip",   "/kabukikl.zip", "/karnovr.zip",  "/kf10thep.zip",  "/kf2k2mp.zip",   "/kf2k2mp2.zip",  "/kf2k2pla.zip",  "/kf2k2pls.zip",
  "/kf2k3bl.zip",  "/kf2k3bla.zip", "/kf2k3pcb.zip", "/kf2k3pl.zip",   "/kf2k3upl.zip",  "/kf2k5uni.zip",  "/kizuna.zip",    "/kof10th.zip",
  "/kof2000.zip",  "/kof2000n.zip", "/kof2001.zip",  "/kof2001h.zip",  "/kof2002.zip",   "/kof2002b.zip",  "/kof2003.zip",   "/kof2003h.zip",
  "/kof2k4se.zip", "/kof94.zip",    "/kof95.zip",    "/kof95h.zip",    "/kof96.zip",     "/kof96h.zip",    "/kof97.zip",     "/kof97a.zip",
  "/kof97pls.zip", "/kof98.zip",    "/kof98k.zip",   "/kof98n.zip",    "/kof99.zip",     "/kof99a.zip",    "/kof99e.zip",    "/kof99n.zip",
  "/kof99p.zip",   "/kog.zip",      "/kotm.zip",     "/kotm2.zip",     "/kotmh.zip",     "/lans2004.zip",  "/lastblad.zip",  "/lastbladh.zip",
  "/lastbld2.zip", "/lastsold.zip", "/lbowling.zip", "/legendos.zip",  "/lresort.zip",   "/magdrop2.zip",  "/magdrop3.zip",  "/maglord.zip",
  "/maglordh.zip", "/mahretsu.zip", "/marukodq.zip", "/matrim.zip",    "/matrimbl.zip",  "/miexchng.zip",  "/minasan.zip",   "/mosyougi.zip",
  "/ms4plus.zip",  "/ms5pcb.zip",   "/ms5plus.zip",  "/mslug.zip",     "/mslug2.zip",    "/mslug3.zip",    "/mslug3b6.zip",  "/mslug3h.zip",
  "/mslug3n.zip",  "/mslug4.zip",   "/mslug5.zip",   "/mslug5h.zip",   "/mslugx.zip",    "/mutnat.zip",    "/nam1975.zip",   "/ncombat.zip",
  "/ncombath.zip", "/ncommand.zip", "/neobombe.zip", "/neocup98.zip",  "/neodrift.zip",  "/neomrdo.zip",   "/ninjamas.zip",  "/nitd.zip",
  "/nitdbl.zip",   "/overtop.zip",  "/panicbom.zip", "/pbobbl2n.zip",  "/pbobblen.zip",  "/pbobblena.zip", "/pgoal.zip",     "/pnyaa.zip",
  "/popbounc.zip", "/preisle2.zip", "/pspikes2.zip", "/pulstar.zip",   "/puzzldpr.zip",  "/puzzledp.zip",  "/quizdai2.zip",  "/quizdais.zip",
  "/quizkof.zip",  "/ragnagrd.zip", "/rbff1.zip",    "/rbff1a.zip",    "/rbff2.zip",     "/rbff2h.zip",    "/rbff2k.zip",    "/rbffspec.zip",
  "/ridhero.zip",  "/ridheroh.zip", "/roboarmy.zip", "/rotd.zip",      "/s1945p.zip",    "/samsh5sp.zip",  "/samsh5sph.zip", "/samsh5spn.zip",
  "/samsho.zip",   "/samsho2.zip",  "/samsho3.zip",  "/samsho3h.zip",  "/samsho4.zip",   "/samsho5.zip",   "/samsho5b.zip",  "/samsho5h.zip",
  "/samshoh.zip",  "/savagere.zip", "/sdodgeb.zip",  "/sengokh.zip",   "/sengoku.zip",   "/sengoku2.zip",  "/sengoku3.zip",  "/shocktr2.zip",
  "/shocktra.zip", "/shocktro.zip", "/socbrawl.zip", "/socbrawla.zip", "/sonicwi2.zip",  "/sonicwi3.zip",  "/spinmast.zip",  "/ssideki.zip",
  "/ssideki2.zip", "/ssideki3.zip", "/ssideki4.zip", "/stakwin.zip",   "/stakwin2.zip",  "/strhoop.zip",   "/superspy.zip",  "/svc.zip",
  "/svcboot.zip",  "/svcpcb.zip",   "/svcpcba.zip",  "/svcplus.zip",   "/svcplusa.zip",  "/svcsplus.zip",  "/tophuntr.zip",  "/tophuntra.zip",
  "/tpgolf.zip",   "/trally.zip",   "/turfmast.zip", "/twinspri.zip",  "/tws96.zip",     "/viewpoin.zip",  "/vliner.zip",    "/vlinero.zip",
  "/wakuwak7.zip", "/wh1.zip",      "/wh1h.zip",     "/wh1ha.zip",     "/wh2.zip",       "/wh2j.zip",      "/wh2jh.zip",     "/whp.zip",
  "/wjammers.zip", "/zedblade.zip", "/zintrckb.zip", "/zupapa.zip",    NULL
};

#define COL32_TO_16(col) ((((col&0xff0000)>>19)<<11)|(((col&0xFF00)>>10)<<5)|((col&0xFF)>>3))

void font_set_color(GNFONT *ft, int col)
{
  if(col >= ICOL_MAX) {
    col = 0;
  }
  ft->bmp = ft->cbmp[col];
}

GNFONT *load_font(int large)
{
  SDL_RWops *rw = NULL;
  GNFONT *ft = malloc(sizeof(GNFONT));
  uint32_t HSV;
  uint32_t RGB;
  int i;
  int x = 0;
  uint32_t *b;
  if(!ft) {
    return NULL;
  }
  memset(ft, 0, sizeof(GNFONT));
  for(i = 0; i < ICOL_MAX; i++) {
    if(large) {
      rw = SDL_RWFromMem(hex_font_large, sizeof(hex_font_large));
    }
    else {
      rw = SDL_RWFromMem(hex_font_small, sizeof(hex_font_small));
    }
    ft->cbmp[i] = IMG_Load_RW(rw, 1);
  }
  ft->bmp = ft->cbmp[0];
  if(!ft->bmp) {
    free(ft);
    return NULL;
  }
  b = ft->bmp->pixels;
  if(ft->bmp->format->BitsPerPixel != 32) {
    printf("unsupported font (bpp=%d)\n", ft->bmp->format->BitsPerPixel);
    SDL_FreeSurface(ft->bmp);
    free(ft);
    return NULL;
  }
  ft->xpos[0] = 0;
  for(i = 0; i < ft->bmp->w; i++) {
    if(b[i] != b[0]) {
      ft->xpos[x + 1] = i + 1;
      if(x > 0) {
        ft->xsize[x] = i - ft->xpos[x];
      }
      else {
        ft->xsize[x] = i;
      }
      x++;
    }
  }

  if(x <= 0 || x > 95) {
    return NULL;
  }
  ft->xsize[94] = ft->bmp->w - ft->xpos[94];
  ft->ysize = ft->bmp->h;

  ft->pad = 0;
  for(x = 1; x < ICOL_MAX; x++) {
    int j;
    b = ft->cbmp[x]->pixels;
    for(i = 0; i < ft->bmp->w; i++) {
      for(j = 0; j < ft->bmp->h; j++) {
        b[i * ft->bmp->h + j] &= 0xff000000;
        b[i * ft->bmp->h + j] |= font_color[x];
      }
    }
  }
  return ft;
}

static uint32_t string_len(GNFONT *f, char *str)
{
  int i;
  int size = 0;
  if(str) {
    for(i = 0; i < strlen(str); i++) {
      switch(str[i]) {
      case ' ':
        size += f->xsize[0];
        break;
      case '\t':
        size += (f->xsize[0] * 8);
        break;
      default:
        size += (f->xsize[(int) str[i] - 32] + f->pad);
        break;
      }
    }
    return size;
  }
  else {
    return 0;
  }
}

void draw_string(SDL_Surface *dst, GNFONT *f, int x, int y, char *str)
{
  SDL_Rect srect, drect;
  int i, s;

  if(!f) {
    if((x & 0xff0000) == ALIGN_LEFT) {
      x += MENU_TEXT_X;
    }
    if((x & 0xff0000) == ALIGN_RIGHT) {
      x += (MENU_TEXT_X_END - strlen(str) * 8);
    }
    if((x & 0xff0000) == ALIGN_CENTER)
      x += (MENU_TEXT_X
            + (MENU_TEXT_X_END - MENU_TEXT_X - strlen(str) * 8) / 2);
    if((y & 0xff0000) == ALIGN_UP) {
      y += MENU_TEXT_Y;
    }
    if((y & 0xff0000) == ALIGN_DOWN) {
      y += (MENU_TEXT_Y_END - 8);
    }
    if((y & 0xff0000) == ALIGN_CENTER) {
      y += (MENU_TEXT_Y + (MENU_TEXT_Y_END - MENU_TEXT_Y - 8) / 2);
    }
    SDL_textout(dst, x & 0xffff, y & 0xffff, str);
    return;
  }

  if((x & 0xff0000) == ALIGN_LEFT) {
    x += MENU_TEXT_X;
  }
  if((x & 0xff0000) == ALIGN_RIGHT) {
    x += (MENU_TEXT_X_END - string_len(f, str));
  }
  if((x & 0xff0000) == ALIGN_CENTER)
    x += (MENU_TEXT_X
          + (MENU_TEXT_X_END - MENU_TEXT_X - string_len(f, str)) / 2);
  if((y & 0xff0000) == ALIGN_UP) {
    y += MENU_TEXT_Y;
  }
  if((y & 0xff0000) == ALIGN_DOWN) {
    y += (MENU_TEXT_Y_END - f->ysize);
  }
  if((y & 0xff0000) == ALIGN_CENTER) {
    y += (MENU_TEXT_Y + (MENU_TEXT_Y_END - MENU_TEXT_Y - f->ysize) / 2);
  }

  x &= 0xffff;
  y &= 0xffff;

  drect.x = x;
  drect.y = y;
  drect.w = 32;
  drect.h = f->bmp->h;
  srect.h = f->bmp->h;
  srect.y = 0;
  for(i = 0; i < strlen(str); i++) {
    switch(str[i]) {
    case ' ':
      drect.x += f->xsize[0];
      break;
    case '\t':
      drect.x += (f->xsize[0] * 8);
      break;
    case '\n':
      drect.x = x;
      drect.y += f->bmp->h;
      break;
    default:
      s = (unsigned char)(str[i]);
      if(s >= 96 + 32) {
        s = (unsigned char) '.';
      }

      srect.x = f->xpos[s - 32];
      srect.w = f->xsize[s - 32];

      SDL_BlitSurface(f->bmp, &srect, dst, &drect);
      drect.x += (f->xsize[s - 32] + f->pad);

      break;
    }
  }
}

static void init_back(void)
{
  SDL_Rect dst_r = { 24, 16, 304, 224 };
  static SDL_Rect screen_rect = { 0, 0, 304, 224 };

  SDL_BlitSurface(state_img, &screen_rect, menu_back_1, &dst_r);
  SDL_BlitSurface(back, NULL, menu_back_1, &dst_r);
}

static void draw_back(void)
{
  SDL_Rect dst_r = { 24, 16, 304, 224 };
  static SDL_Rect screen_rect = { 0, 0, 304, 224 };
  if(back) {
    SDL_BlitSurface(menu_back_1, NULL, menu_buf, NULL);
  }
  else {
    SDL_Rect r1 = { 24 + 16, 16 + 16, 271, 191 };
    SDL_Rect r2 = { 24 + 22, 16 + 35, 259, 166 };
    SDL_Rect r3 = { 24 + 24, 16 + 24, 271, 191 };

    SDL_FillRect(menu_buf, &r3, COL32_TO_16(0x111111));
    SDL_FillRect(menu_buf, &r1, COL32_TO_16(0xE8E8E8));
    SDL_FillRect(menu_buf, &r2, COL32_TO_16(0x1C57A2));
  }

}

#define ALEFT  0
#define ARIGHT 1
#define ADOWN  2
#define AUP    3

static void draw_arrow(int type, int x, int y)
{
  SDL_Rect dst_r = { x, y - 13, 32, 32 };
  switch(type) {
  case ARIGHT:
    SDL_BlitSurface(arrow_r, NULL, menu_buf, &dst_r);
    break;
  case ALEFT:
    SDL_BlitSurface(arrow_l, NULL, menu_buf, &dst_r);
    break;
  case AUP:
    SDL_BlitSurface(arrow_u, NULL, menu_buf, &dst_r);
    break;
  case ADOWN:
    SDL_BlitSurface(arrow_d, NULL, menu_buf, &dst_r);
    break;
  }
}

int gn_init_skin(void)
{
  SDL_RWops *rw = NULL;

  menu_buf = SDL_CreateRGBSurface(SDL_SWSURFACE, 352, 256, 16, 0xf800, 0x7e0, 0x1f, 0);
  menu_back_1 = SDL_CreateRGBSurface(SDL_SWSURFACE, 352, 256, 16, 0xf800, 0x7e0, 0x1f, 0);

  rw = SDL_RWFromMem(hex_back, sizeof(hex_back));
  back = IMG_Load_RW(rw, 1);

  rw = SDL_RWFromMem(hex_arrow_up, sizeof(hex_arrow_up));
  arrow_u = IMG_Load_RW(rw, 1);

  rw = SDL_RWFromMem(hex_arrow_down, sizeof(hex_arrow_down));
  arrow_d = IMG_Load_RW(rw, 1);

  rw = SDL_RWFromMem(hex_arrow_left, sizeof(hex_arrow_left));
  arrow_l = IMG_Load_RW(rw, 1);

  rw = SDL_RWFromMem(hex_arrow_right, sizeof(hex_arrow_right));
  arrow_r = IMG_Load_RW(rw, 1);

  rw = SDL_RWFromMem(hex_gngeo, sizeof(hex_gngeo));
  gngeo_logo = IMG_Load_RW(rw, 1);

  rw = SDL_RWFromMem(hex_gngeo_mask, sizeof(hex_gngeo_mask));
  gngeo_mask = IMG_Load_RW(rw, 1);

  sfont = load_font(0);
  mfont = load_font(1);

  pbar_logo = SDL_CreateRGBSurface(SDL_SWSURFACE, gngeo_logo->w, gngeo_logo->h, 32, 0xff, 0xff00, 0xff0000, 0xff000000);
  SDL_SetAlpha(gngeo_logo, 0, 0);
  init_back();
  if(!back || !sfont || !mfont || !arrow_r || !arrow_l || !arrow_u || !arrow_d || !gngeo_logo || !menu_buf) {
    return GN_FALSE;
  }
  return GN_TRUE;
}

void gn_reset_pbar(void)
{
  pbar_y = 0;
}

static SDL_Thread *pbar_th;

typedef struct pbar_data {
  char *name;
  int pos;
  int size;
  int running;
} pbar_data;

static volatile pbar_data pbar;

int pbar_anim_thread(void *data)
{
  pbar_data *p = (pbar_data *) data;
  SDL_Rect src_r = { 2, 0, gngeo_logo->w, gngeo_logo->h };
  SDL_Rect dst_r = { 219 + 26, 146 + 16, gngeo_logo->w, gngeo_logo->h };
  SDL_Rect dst2_r = { 0, 0, gngeo_logo->w, gngeo_logo->h };
  int x = 0;

  while(p->running) {
    draw_back();
    draw_string(menu_buf, mfont, MENU_TITLE_X, MENU_TITLE_Y + 150, p->name);
    draw_string(menu_buf, sfont, MENU_TITLE_X, MENU_TITLE_Y + 165,
                "Please wait!");

    SDL_BlitSurface(gngeo_logo, NULL, pbar_logo, NULL);

    dst2_r.y = -22 - (p->pos * 64.0) / p->size;
    x += 3;
    if(x > gngeo_logo->w) {
      x -= gngeo_logo->w;
    }

    dst2_r.x = x;
    SDL_BlitSurface(gngeo_mask, NULL, pbar_logo, &dst2_r);

    dst2_r.x = x - gngeo_logo->w;
    dst2_r.y = -22 - (p->pos * 64.0) / p->size;
    SDL_BlitSurface(gngeo_mask, NULL, pbar_logo, &dst2_r);

    SDL_BlitSurface(pbar_logo, &src_r, menu_buf, &dst_r);

    SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
    screen_flip();
    frame_skip();
  }
  SDL_BlitSurface(gngeo_logo, NULL, pbar_logo, NULL);
  SDL_BlitSurface(pbar_logo, &src_r, menu_buf, &dst_r);
  screen_flip();
  frame_skip();
  return 0;
}

void gn_init_pbar(char *name, int size)
{
  pbar.name = name;
  pbar.pos = 0;
  pbar.size = size;
  pbar.running = 1;
  pbar_th = SDL_CreateThread(pbar_anim_thread, (void *) &pbar);
}

void gn_update_pbar(int pos)
{
  pbar.pos = pos;
}

void gn_terminate_pbar(void)
{
  pbar.running = 0;
  SDL_WaitThread(pbar_th, NULL);
}

void gn_popup_error(char *name, char *fmt, ...)
{
  char buf[512];
  va_list pvar;
  va_start(pvar, fmt);
  SDL_Event event = {0};

  draw_back();
  draw_string(menu_buf, mfont, MENU_TITLE_X, MENU_TITLE_Y, name);
  vsnprintf(buf, 511, fmt, pvar);
  draw_string(menu_buf, sfont, MENU_TEXT_X, MENU_TEXT_Y, buf);
  draw_string(menu_buf, sfont, ALIGN_RIGHT, ALIGN_DOWN, "Press any key ...");
  SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
  screen_flip();

  while(1) {
    if(SDL_PollEvent(&event)) {
      break;
    }
    SDL_Delay(50);
  }
}

static int yes_action(GN_MENU_ITEM *self, void *param)
{
  return 1;
}

static int no_action(GN_MENU_ITEM *self, void *param)
{
  return 0;
}

int gn_popup_question(char *name, char *fmt, ...)
{
  char buf[512];
  va_list pvar;
  va_start(pvar, fmt);
  int a;

  while(1) {
    draw_back();
    draw_string(menu_buf, mfont, MENU_TITLE_X, MENU_TITLE_Y, name);
    vsnprintf(buf, 511, fmt, pvar);
    draw_string(menu_buf, sfont, MENU_TEXT_X, MENU_TEXT_Y, buf);
    if(yesno_menu->current == 0) {
      draw_string(menu_buf, sfont, ALIGN_RIGHT, ALIGN_DOWN, " > Yes     No");
    }
    else {
      draw_string(menu_buf, sfont, ALIGN_RIGHT, ALIGN_DOWN, "   Yes  >  No");
    }
    SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
    screen_flip();
    if((a = yesno_menu->event_handling(yesno_menu)) >= 0) {
      printf("return %d\n", a);
      return a;
    }
  }
  return 0;
}

static void draw_menu(GN_MENU *m)
{
  int start, end, i;
  static int cx = 0;
  static int cy = 0;
  static int cx_val[] = { 0, 1, 1, 1, 2, 2, 2, 2, 2, 1, 1, 1, 0, 0, -1, -1, -1, -2, -2, -2, -2, -2, -1, -1, -1, 0};
  int nb_item;
  int ypos, cpos;
  GNFONT *fnt;
  LIST *l = m->item;
  int j;

  if(m->draw_type == MENU_BIG) {
    fnt = mfont;
  }
  else {
    fnt = sfont;
  }

  cx++;
  if(cx > 25) {
    cx = 0;
  }

  nb_item = (MENU_TEXT_Y_END - MENU_TEXT_Y) / fnt->ysize - 1;

  draw_back();

  if(m->title) {
    draw_string(menu_buf, mfont, MENU_TITLE_X, MENU_TITLE_Y, m->title);
  }
  start = m->current - nb_item / 2;
  end = m->current + nb_item / 2;
  if(start < 0) {
    start = 0;
  }
  if(end >= m->nb_elem - 1) {
    end = m->nb_elem - 1;
  }
  else {
    draw_arrow(ADOWN, 24, 200 + cx_val[cx]);
  }

  if(m->current <= nb_item / 2) {
    j = nb_item / 2 - m->current;
  }
  else {
    j = 0;
    draw_arrow(AUP, 24, 76 - cx_val[cx]);
  }

  for(i = 0; i < start; i++, l = l->next) {
    GN_MENU_ITEM *mi = (GN_MENU_ITEM *) l->data;
    if(mi->enabled == 0) {
      i--;
    }
  }
  for(i = start; i <= end; i++, l = l->next) {
    GN_MENU_ITEM *mi = (GN_MENU_ITEM *) l->data;
    if(mi->enabled == 0) {
      i--;
      continue;
    }
    if(m->draw_type == MENU_BIG) {
      font_set_color(fnt, ICOL_GREY_L);
      draw_string(menu_buf, fnt, ALIGN_CENTER + 2, MENU_TEXT_Y + 2 + (j * fnt->ysize + 2), mi->name);
      font_set_color(fnt, ICOL_BLACK);
      draw_string(menu_buf, fnt, ALIGN_CENTER, MENU_TEXT_Y + (j * fnt->ysize + 2), mi->name);
      if(i == m->current) {
        int len = string_len(fnt, mi->name) / 2;
        draw_arrow(ARIGHT, 176 - len - 32 + cx_val[cx], MENU_TEXT_Y + (j * fnt->ysize + 2) + fnt->ysize / 2);
        draw_arrow(ALEFT, 176 + len - cx_val[cx], MENU_TEXT_Y + (j * fnt->ysize + 2) + fnt->ysize / 2);
      }

    }
    else {
      draw_string(menu_buf, fnt, MENU_TEXT_X + 10, MENU_TEXT_Y + (j * fnt->ysize + 2), mi->name);
      if(i == m->current) {
        draw_string(menu_buf, fnt, MENU_TEXT_X, MENU_TEXT_Y + (j * fnt->ysize + 2), ">");
      }
      if(mi->type == MENU_CHECK) {
        if(mi->val) {
          draw_string(menu_buf, fnt, MENU_TEXT_X + 210, MENU_TEXT_Y + (j * fnt->ysize + 2), "ON");
        }
        else {
          draw_string(menu_buf, fnt, MENU_TEXT_X + 210, MENU_TEXT_Y + (j * fnt->ysize + 2), "OFF");
        }
      }
      if(mi->type == MENU_LIST) {
        draw_string(menu_buf, fnt, MENU_TEXT_X + 210, MENU_TEXT_Y + (j * fnt->ysize + 2), mi->str);
      }
    }
    j++;
  }
  SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
  screen_flip();
  frame_skip();
}

GN_MENU_ITEM *gn_menu_create_item(char *name, uint32_t type, int(*action)(GN_MENU_ITEM *self, void *param), void *param)
{
  GN_MENU_ITEM *t = malloc(sizeof(GN_MENU_ITEM));
  t->name = strdup(name);
  t->type = type;
  t->action = action;
  t->arg = param;
  t->enabled = 1;
  return t;
}

GN_MENU_ITEM *gn_menu_get_item_by_index(GN_MENU *gmenu, int index)
{
  GN_MENU_ITEM *gitem;
  LIST *l = gmenu->item;
  int i = 0;
  for(l = gmenu->item; l; l = l->next) {
    gitem = (GN_MENU_ITEM *) l->data;
    if(gitem->enabled) {
      if(i == index) {
        return gitem;
      }
      i++;
    }

  }
  return NULL;
}

int test_action(GN_MENU_ITEM *self, void *param)
{
  printf("Action!!\n");
  return 0;
}

static int load_state_action(GN_MENU_ITEM *self, void *param)
{
  SDL_Event event = {0};
  static uint32_t slot = 0;
  SDL_Rect dstrect = { 24 + 75, 16 + 66, 304 / 2, 224 / 2 };
  SDL_Rect dstrect_binding = { 24 + 73, 16 + 64, 304 / 2 + 4, 224 / 2 + 4 };
  SDL_Surface *tmps, *slot_img;
  char slot_str[32];

  uint32_t nb_slot = how_many_slot(conf.game);

  if(slot > nb_slot - 1) {
    slot = nb_slot - 1;
  }

  if(nb_slot == 0) {
    gn_popup_info("Load State", "There is currently no save state available");
    return 0;
  }

  tmps = load_state_img(conf.game, slot);
  slot_img = SDL_ConvertSurface(tmps, menu_buf->format, SDL_SWSURFACE);

  while(1) {
    draw_back();
    SDL_FillRect(menu_buf, &dstrect_binding, COL32_TO_16(0xFEFEFE));
    SDL_SoftStretch(slot_img, NULL, menu_buf, &dstrect);

    draw_string(menu_buf, mfont, MENU_TITLE_X, MENU_TITLE_Y, "Load State");
    sprintf(slot_str, "Slot Number %03d", slot);
    draw_string(menu_buf, sfont, ALIGN_CENTER, ALIGN_UP, slot_str);

    if(slot > 0) {
      draw_arrow(ALEFT, 44 + 16, 224 / 2 + 16);
    }
    if(slot < nb_slot - 1) {
      draw_arrow(ARIGHT, 304 - 43, 224 / 2 + 16);
    }

    SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
    screen_flip();
    frame_skip();

    if(SDL_PollEvent(&event)) {
      if(event.type == SDL_KEYDOWN) {
        switch(event.key.keysym.sym) {
        case SDLK_LEFT:
          if(slot > 0) {
            slot--;
          }
          slot_img = SDL_ConvertSurface(load_state_img(conf.game, slot), menu_buf->format, SDL_SWSURFACE);
          break;
        case SDLK_RIGHT:
          if(slot < nb_slot - 1) {
            slot++;
          }
          slot_img = SDL_ConvertSurface(load_state_img(conf.game, slot), menu_buf->format, SDL_SWSURFACE);
          break;
        case SDLK_LCTRL:
          return MENU_STAY;
        case SDLK_LALT:
          printf("Loading state %p!!\n", buffer);
          load_state(conf.game, slot);
          printf("Loaded state %p!!\n", buffer);
          return MENU_RETURNTOGAME;
        }
      }
    }
  }
  return 0;
}

static int save_state_action(GN_MENU_ITEM *self, void *param)
{
  static uint32_t slot = 0;
  SDL_Event event = {0};
  SDL_Rect dstrect = { 24 + 75, 16 + 66, 304 / 2, 224 / 2 };
  SDL_Rect dstrect_binding = { 24 + 73, 16 + 64, 304 / 2 + 4, 224 / 2 + 4 };
  SDL_Surface *tmps, *slot_img = NULL;
  char slot_str[32];
  uint32_t nb_slot = how_many_slot(conf.game);

  if(slot > nb_slot) {
    slot = nb_slot;
  }

  if(nb_slot != 0 && slot < nb_slot) {
    tmps = load_state_img(conf.game, slot);
    slot_img = SDL_ConvertSurface(tmps, menu_buf->format, SDL_SWSURFACE);
  }

  while(1) {
    draw_back();
    if(slot != nb_slot) {
      SDL_FillRect(menu_buf, &dstrect_binding, COL32_TO_16(0xFEFEFE));
      SDL_SoftStretch(slot_img, NULL, menu_buf, &dstrect);
    }
    else {
      SDL_FillRect(menu_buf, &dstrect_binding, COL32_TO_16(0xFEFEFE));
      SDL_FillRect(menu_buf, &dstrect, COL32_TO_16(0xA0B0B0));
      draw_string(menu_buf, sfont, ALIGN_CENTER, ALIGN_CENTER,
                  "Create a new Slot");
    }

    draw_string(menu_buf, mfont, MENU_TITLE_X, MENU_TITLE_Y, "Save State");
    sprintf(slot_str, "Slot Number %03d", slot);
    draw_string(menu_buf, sfont, ALIGN_CENTER, ALIGN_UP, slot_str);

    if(slot > 0) {
      draw_arrow(ALEFT, 44 + 16, 224 / 2 + 16);
    }
    if(slot < nb_slot) {
      draw_arrow(ARIGHT, 304 - 43, 224 / 2 + 16);
    }

    SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
    screen_flip();
    frame_skip();

    if(SDL_PollEvent(&event)) {
      if(event.type == SDL_KEYDOWN) {
        switch(event.key.keysym.sym) {
        case SDLK_LEFT:
          if(slot > 0) {
            slot--;
          }
          if(slot != nb_slot)
            slot_img = SDL_ConvertSurface(load_state_img(conf.game, slot),
                                          menu_buf->format, SDL_SWSURFACE);
          break;
        case SDLK_RIGHT:
          if(slot < nb_slot) {
            slot++;
          }
          if(slot != nb_slot)
            slot_img = SDL_ConvertSurface(load_state_img(conf.game, slot),
                                          menu_buf->format, SDL_SWSURFACE);
          break;
        case SDLK_LCTRL:
          return MENU_STAY;
          break;
        case SDLK_LALT:
        case SDLK_LSHIFT:
          save_state(conf.game, slot);
          printf("Save state!!\n");
          return MENU_RETURNTOGAME;
        }
      }
    }
  }
  return 0;
}

static int credit_action(GN_MENU_ITEM *self, void *param)
{

  return MENU_STAY;
}

static int exit_action(GN_MENU_ITEM *self, void *param)
{
  printf("Save all\n");
  return MENU_EXIT;
}

int menu_event_handling(struct GN_MENU *self)
{
  int a;
  LIST *l;
  GN_MENU_ITEM *mi;
  SDL_Event event = {0};

  if(SDL_PollEvent(&event)) {
    if(event.type == SDL_KEYDOWN) {
      switch(event.key.keysym.sym) {
      case SDLK_UP:
        if(self->current > 0) {
          self->current--;
        }
        else {
          self->current = self->nb_elem - 1;
        }
        break;
      case SDLK_DOWN:
        if(self->current < self->nb_elem - 1) {
          self->current++;
        }
        else {
          self->current = 0;
        }
        break;
      case SDLK_LEFT:
        self->current -= 10;
        if(self->current < 0) {
          self->current = 0;
        }
        break;
      case SDLK_RIGHT:
        self->current += 10;
        if(self->current >= self->nb_elem) {
          self->current = self->nb_elem - 1;
        }
        break;
      case SDLK_LCTRL:
        return MENU_CLOSE;
        break;
      case SDLK_LALT:
      case SDLK_SPACE:
        mi = gn_menu_get_item_by_index(self, self->current);
        if(mi && mi->action) {
          if((a = mi->action(mi, NULL)) > 0) {
            return a;
          }
        }
        break;
      }
    }
  }
  return -1;
}

GN_MENU *create_menu(char *name, int type, int(*action)(struct GN_MENU *self), void(*draw)(struct GN_MENU *self))
{
  GN_MENU *gmenu;
  gmenu = malloc(sizeof(GN_MENU));
  gmenu->title = name;
  gmenu->nb_elem = 0;
  gmenu->current = 0;
  gmenu->draw_type = type;
  if(action) {
    gmenu->event_handling = action;
  }
  else {
    gmenu->event_handling = menu_event_handling;
  }
  if(draw) {
    gmenu->draw = draw;
  }
  else {
    gmenu->draw = draw_menu;
  }
  gmenu->item = NULL;
  return gmenu;
}

GN_MENU_ITEM *gn_menu_add_item(GN_MENU *gmenu, char *name, int type, int(*action)(struct GN_MENU_ITEM *self, void *param), void *param)
{
  GN_MENU_ITEM *gitem;
  gitem = gn_menu_create_item(name, type, action, param);
  gmenu->item = list_append(gmenu->item, (void *) gitem);
  gmenu->nb_elem++;
  return gitem;
}

GN_MENU_ITEM *gn_menu_get_item_by_name(GN_MENU *gmenu, char *name)
{
  GN_MENU_ITEM *gitem;
  LIST *l = gmenu->item;

  for(l = gmenu->item; l; l = l->next) {
    gitem = (GN_MENU_ITEM *) l->data;
    if(strncmp(gitem->name, name, 128) == 0 && gitem->enabled != 0) {
      return gitem;
    }
  }
  return NULL;
}

void gn_menu_disable_item(GN_MENU *gmenu, char *name)
{
  GN_MENU_ITEM *gitem;
  LIST *l = gmenu->item;

  for(l = gmenu->item; l; l = l->next) {
    gitem = (GN_MENU_ITEM *) l->data;
    if(strncmp(gitem->name, name, 128) == 0 && gitem->enabled != 0) {
      gitem->enabled = 0;
      gmenu->nb_elem--;
      return;
    }
  }
}

void gn_menu_enable_item(GN_MENU *gmenu, char *name)
{
  GN_MENU_ITEM *gitem;
  LIST *l = gmenu->item;

  for(l = gmenu->item; l; l = l->next) {
    gitem = (GN_MENU_ITEM *) l->data;
    if(strcmp(gitem->name, name) == 0 && gitem->enabled == 0) {
      gitem->enabled = 1;
      gmenu->nb_elem++;
      return;
    }
  }
}

int icasesort(const struct dirent **a, const struct dirent **b)
{
  const char *ca = (*a)->d_name;
  const char *cb = (*b)->d_name;
  return strcasecmp(ca, cb);
}

static int romnamesort(void *a, void *b)
{
  GN_MENU_ITEM *ga = (GN_MENU_ITEM *) a;
  GN_MENU_ITEM *gb = (GN_MENU_ITEM *) b;

  return strcmp(ga->name, gb->name);
}

static int loadrom_action(GN_MENU_ITEM *self, void *param)
{
  char *game = (char *) self->arg;

  printf("Loading %s\n", game);
  close_game();

  if(init_game(game) != GN_TRUE) {
    printf("Can't init %s...\n", game);
    gn_popup_error("Error! :", "Gngeo Couldn't init %s: \n\n%s\nMaybe the romset you're using is too old", game, gnerror);
    return MENU_STAY;
  }

  return MENU_RETURNTOGAME;
}

static volatile int scaning = 0;

int rom_browser_scanning_anim(void *data)
{
  int i = 0;
  while(scaning) {
    draw_back();
    if(i > 20) {
      draw_string(menu_buf, sfont, MENU_TITLE_X, MENU_TITLE_Y, "Scanning ...");
    }
    else {
      draw_string(menu_buf, sfont, MENU_TITLE_X, MENU_TITLE_Y, "Scanning");
    }
    SDL_BlitSurface(menu_buf, NULL, buffer, NULL);
    screen_flip();
    frame_skip();
    i++;
    if(i > 40) {
      i = 0;
    }
  }
  return 0;
}
static void free_rom_browser_menu(void);
void init_rom_browser_menu(void);
static int changedir_action(GN_MENU_ITEM *self, void *param)
{
  char *dir = (char *) self->arg;
  SDL_Thread *anim_th;

  gn_strncat_dir(CF_STR(cf_get_item_by_name("rompath")), dir, CF_MAXSTRLEN);
  cf_save_option(NULL, "rompath", 0);

  scaning = 1;
  anim_th = SDL_CreateThread(rom_browser_scanning_anim, NULL);
  free_rom_browser_menu();
  init_rom_browser_menu();
  scaning = 0;
  SDL_WaitThread(anim_th, NULL);

  return MENU_STAY;
}

static void free_rom_browser_menu(void)
{
  LIST *i, *t;
  if(rbrowser_menu == NULL) {
    return;
  }

  for(i = rbrowser_menu->item; i; i = i->next) {
    GN_MENU_ITEM *gitem;
    gitem = (GN_MENU_ITEM *) i->data;
    if(gitem->name) {
      free(gitem->name);
    }
    if(gitem->arg) {
      free(gitem->arg);
    }
    free(gitem);
  }
  i = rbrowser_menu->item;
  while(i) {
    t = i;
    i = i->next;
    free(t);
  }
}

void init_rom_browser_menu(void)
{
  int i;
  int nbf;
  char filename[strlen(CF_STR(cf_get_item_by_name("rompath"))) + 256];
  struct stat filestat;
  struct dirent **namelist;
  ROM_DEF *drv = NULL;
  int nb_roms = 0;
  DIR *dh;
  struct dirent *file;

  rbrowser_menu = create_menu("Load Game", MENU_SMALL, NULL, NULL);

  if((dh = opendir(CF_STR(cf_get_item_by_name("rompath")))) != NULL) {
    while((file = readdir(dh)) != NULL) {
      sprintf(filename, "%s/%s", CF_STR(cf_get_item_by_name("rompath")),
              file->d_name);
      if(stat(filename, &filestat) == 0) {
        if(S_ISDIR(filestat.st_mode) || (strcmp(file->d_name, ".") == 0 && strcmp(file->d_name, "..") == 0)) {
          rbrowser_menu->item = list_insert_sort(rbrowser_menu->item, gn_menu_create_item(file->d_name, MENU_ACTION, changedir_action, strdup(file->d_name)), romnamesort);
          rbrowser_menu->nb_elem++;
          continue;
        }
        if(S_ISREG(filestat.st_mode)) {
          if(strstr(filename, ".gno") != NULL) {
            printf("GNO %s\n", filename);
            char *gnoname = dr_gno_romname(filename);
            printf("gnoname %s\n", gnoname);
            if(gnoname != NULL && (drv = res_load_drv(gnoname)) != NULL) {
              rbrowser_menu->item = list_insert_sort(rbrowser_menu->item, gn_menu_create_item(drv->longname, MENU_ACTION, loadrom_action, strdup(filename)), romnamesort);
              rbrowser_menu->nb_elem++;
              nb_roms++;
              //free(drv);
            }
            continue;
          }
          if(strstr(filename, ".zip") == NULL) {
            continue;
          }
          i = 0;
          while(romlist[i]) {
            if(strstr(filename, romlist[i]) != NULL) {
              if((drv = dr_check_zip(filename)) != NULL) {
                rbrowser_menu->item = list_insert_sort(rbrowser_menu->item, gn_menu_create_item(drv->longname, MENU_ACTION, loadrom_action, strdup(drv->name)), romnamesort);
                rbrowser_menu->nb_elem++;
                //free(drv);
                nb_roms++;
              }
            }
            i++;
          }

        }
      }
    }
    closedir(dh);
  }
  if(nb_roms == 0) {
    rbrowser_menu->item = list_append(rbrowser_menu->item, gn_menu_create_item("No Games Found...", MENU_ACTION, NULL, NULL));
    rbrowser_menu->nb_elem++;
  }
}

int rom_browser_menu(void)
{
  static uint32_t init = 0;
  int a;
  SDL_Thread *anim_th;

  if(init == 0) {
    char *rpath = strdup(CF_STR(cf_get_item_by_name("rompath")));
    char *apath;
    init = 1;

    apath = realpath(rpath, NULL);
    if(apath == NULL) {
      apath = realpath(".", NULL);
    }
    snprintf(CF_STR(cf_get_item_by_name("rompath")), CF_MAXSTRLEN, "%s", apath);
    free(apath);
    free(rpath);

    scaning = 1;
    anim_th = SDL_CreateThread(rom_browser_scanning_anim, NULL);
    init_rom_browser_menu();
    scaning = 0;
    SDL_WaitThread(anim_th, NULL);
  }

  while(1) {
    rbrowser_menu->draw(rbrowser_menu);
    if((a = rbrowser_menu->event_handling(rbrowser_menu)) > 0) {
      if(a == MENU_CLOSE) {
        return MENU_STAY;
      }
      else {
        return a;
      }
    }
  }
  return MENU_STAY;
}

static int rbrowser_action(GN_MENU_ITEM *self, void *param)
{
  return rom_browser_menu();
}

static int toggle_showfps(GN_MENU_ITEM *self, void *param)
{
  self->val = 1 - self->val;
  conf.show_fps = self->val;
  cf_item_has_been_changed(cf_get_item_by_name("showfps"));
  CF_BOOL(cf_get_item_by_name("showfps")) = self->val;

  return MENU_STAY;
}

static int toggle_sound(GN_MENU_ITEM *self, void *param)
{
  if(conf.sound && conf.game) {
    close_sdl_audio();
  }
  self->val = 1 - self->val;
  conf.sound = self->val;
  cf_item_has_been_changed(cf_get_item_by_name("sound"));
  CF_BOOL(cf_get_item_by_name("sound")) = self->val;
  if(conf.game) {
    init_sdl_audio();
    YM2610ChangeSamplerate(SAMPLE_RATE);
  }
  return MENU_STAY;
}

static int set_a_button_action(GN_MENU_ITEM *self, void *param)
{
  int sel = (int)self->arg;

  conf.a_btn = sel;
  CF_VAL(cf_get_item_by_name("a_btn")) = sel;
  cf_item_has_been_changed(cf_get_item_by_name("a_btn"));
  return MENU_CLOSE;
}

static int set_b_button_action(GN_MENU_ITEM *self, void *param)
{
  int sel = (int)self->arg;

  conf.b_btn = sel;
  CF_VAL(cf_get_item_by_name("b_btn")) = sel;
  cf_item_has_been_changed(cf_get_item_by_name("b_btn"));
  return MENU_CLOSE;
}

static int set_x_button_action(GN_MENU_ITEM *self, void *param)
{
  int sel = (int)self->arg;

  conf.x_btn = sel;
  CF_VAL(cf_get_item_by_name("x_btn")) = sel;
  cf_item_has_been_changed(cf_get_item_by_name("x_btn"));
  return MENU_CLOSE;
}

static int set_y_button_action(GN_MENU_ITEM *self, void *param)
{
  int sel = (int)self->arg;

  conf.y_btn = sel;
  CF_VAL(cf_get_item_by_name("y_btn")) = sel;
  cf_item_has_been_changed(cf_get_item_by_name("y_btn"));
  return MENU_CLOSE;
}

static int set_l_button_action(GN_MENU_ITEM *self, void *param)
{
  int sel = (int)self->arg;

  conf.l_btn = sel;
  CF_VAL(cf_get_item_by_name("l_btn")) = sel;
  cf_item_has_been_changed(cf_get_item_by_name("l_btn"));
  return MENU_CLOSE;
}

static int set_r_button_action(GN_MENU_ITEM *self, void *param)
{
  int sel = (int)self->arg;

  conf.r_btn = sel;
  CF_VAL(cf_get_item_by_name("r_btn")) = sel;
  cf_item_has_been_changed(cf_get_item_by_name("r_btn"));
  return MENU_CLOSE;
}

static int set_abxylr_button(GN_MENU_ITEM *self, void *param)
{
  static int init[6] = {0};
  GN_MENU_ITEM *gitem = NULL;
  int i = 0, sel = (int)self->arg;
  int (*func_ptr[])(GN_MENU_ITEM*, void*) = {
    set_a_button_action,
    set_b_button_action,
    set_x_button_action,
    set_y_button_action,
    set_l_button_action,
    set_r_button_action,
  };

  char* mstr[6]={
    "Choose Button for A",
    "Choose Button for B",
    "Choose Button for X",
    "Choose Button for Y",
    "Choose Button for L",
    "Choose Button for R",
  };

  if(init[sel] == 0) {
    init[sel] = 1;
    btn_menu[sel] = create_menu(mstr[sel], MENU_SMALL, NULL, NULL);
    for(i = 0; i < 16; i++) {
      gitem = gn_menu_create_item(abxylr_btn_string[i], MENU_ACTION, func_ptr[sel], (void *)i);
      btn_menu[sel]->item = list_append(btn_menu[sel]->item, gitem);
      btn_menu[sel]->nb_elem += 1;
    }
  }

  while(1) {
    int ssel=0;
    btn_menu[sel]->draw(btn_menu[sel]);
    if(btn_menu[sel]->event_handling(btn_menu[sel]) > 0) {
      switch(sel){
      case 0: ssel = conf.a_btn; break;
      case 1: ssel = conf.b_btn; break;
      case 2: ssel = conf.x_btn; break;
      case 3: ssel = conf.y_btn; break;
      case 4: ssel = conf.l_btn; break;
      case 5: ssel = conf.r_btn; break;
      }
      sprintf(self->str, "%s", abxylr_btn_string[ssel]);
      return MENU_STAY;
    }
  }
  return 0;
}

static int save_conf_action(GN_MENU_ITEM *self, void *param)
{
  int type = (int) self->arg;
  if(type == 0) {
    cf_save_file(NULL, 0);
  }
  else {
    char *gpath;
    char *drconf;
    char *name = memory.rom.info.name;
    gpath = get_gngeo_dir();
    drconf = alloca(strlen(gpath) + strlen(name) + strlen(".cf") + 1);
    sprintf(drconf, "%s%s.cf", gpath, name);
    cf_save_file(drconf, 0);
  }
  return GN_TRUE;
}

#define RESET_BOOL(name,id) gitem=gn_menu_get_item_by_name(option_menu,name);\
if (gitem) gitem->val = CF_BOOL(cf_get_item_by_name(id));

static void reset_menu_option(void)
{
  GN_MENU_ITEM *gitem;
  gitem = gn_menu_get_item_by_name(option_menu, "Set A Button");
  sprintf(gitem->str, "%s", abxylr_btn_string[conf.a_btn]);
  gitem = gn_menu_get_item_by_name(option_menu, "Set B Button");
  sprintf(gitem->str, "%s", abxylr_btn_string[conf.b_btn]);
  gitem = gn_menu_get_item_by_name(option_menu, "Set X Button");
  sprintf(gitem->str, "%s", abxylr_btn_string[conf.x_btn]);
  gitem = gn_menu_get_item_by_name(option_menu, "Set Y Button");
  sprintf(gitem->str, "%s", abxylr_btn_string[conf.y_btn]);
  gitem = gn_menu_get_item_by_name(option_menu, "Set L Button");
  sprintf(gitem->str, "%s", abxylr_btn_string[conf.l_btn]);
  gitem = gn_menu_get_item_by_name(option_menu, "Set R Button");
  sprintf(gitem->str, "%s", abxylr_btn_string[conf.r_btn]);
}

static int option_action(GN_MENU_ITEM *self, void *param)
{
  int a;
  reset_menu_option();
  while(1) {
    option_menu->draw(option_menu);
    if((a = option_menu->event_handling(option_menu)) > 0) {
      reset_menu_option();
      return MENU_STAY;
    }
  }
  return 0;
}

void gn_init_menu(void)
{
  GN_MENU_ITEM *gitem;
  main_menu = create_menu(NULL, MENU_BIG, NULL, NULL);

  main_menu->item = list_append(main_menu->item, gn_menu_create_item("Load game", MENU_ACTION, rbrowser_action, NULL));
  main_menu->nb_elem++;

  main_menu->item = list_append(main_menu->item, gn_menu_create_item("Load state", MENU_ACTION, load_state_action, NULL));
  main_menu->nb_elem++;
  main_menu->item = list_append(main_menu->item, gn_menu_create_item("Save state", MENU_ACTION, save_state_action, NULL));
  main_menu->nb_elem++;

  main_menu->item = list_append(main_menu->item, gn_menu_create_item("Option", MENU_ACTION, option_action, NULL));
  main_menu->nb_elem++;

  main_menu->item = list_append(main_menu->item, gn_menu_create_item("Exit", MENU_ACTION, exit_action, NULL));
  main_menu->nb_elem++;

  option_menu = create_menu("Options", MENU_SMALL, NULL, NULL);

  gitem = gn_menu_create_item("Show FPS", MENU_CHECK, toggle_showfps, NULL);
  gitem->val = CF_BOOL(cf_get_item_by_name("showfps"));
  option_menu->item = list_append(option_menu->item, gitem);
  option_menu->nb_elem++;

  gitem = gn_menu_create_item("Enable Sound", MENU_CHECK, toggle_sound, NULL);
  gitem->val = CF_BOOL(cf_get_item_by_name("sound"));
  option_menu->item = list_append(option_menu->item, gitem);
  option_menu->nb_elem++;

  gitem = gn_menu_create_item("Set A Button", MENU_LIST, set_abxylr_button, (void *)0);
  gitem->str = malloc(32);
  sprintf(gitem->str, "%s", abxylr_btn_string[conf.a_btn]);
  option_menu->item = list_append(option_menu->item, gitem);
  option_menu->nb_elem++;

  gitem = gn_menu_create_item("Set B Button", MENU_LIST, set_abxylr_button, (void *)1);
  gitem->str = malloc(32);
  sprintf(gitem->str, "%s", abxylr_btn_string[conf.b_btn]);
  option_menu->item = list_append(option_menu->item, gitem);
  option_menu->nb_elem++;

  gitem = gn_menu_create_item("Set X Button", MENU_LIST, set_abxylr_button, (void *)2);
  gitem->str = malloc(32);
  sprintf(gitem->str, "%s", abxylr_btn_string[conf.x_btn]);
  option_menu->item = list_append(option_menu->item, gitem);
  option_menu->nb_elem++;

  gitem = gn_menu_create_item("Set Y Button", MENU_LIST, set_abxylr_button, (void *)3);
  gitem->str = malloc(32);
  sprintf(gitem->str, "%s", abxylr_btn_string[conf.y_btn]);
  option_menu->item = list_append(option_menu->item, gitem);
  option_menu->nb_elem++;

  gitem = gn_menu_create_item("Set L Button", MENU_LIST, set_abxylr_button, (void *)4);
  gitem->str = malloc(32);
  sprintf(gitem->str, "%s", abxylr_btn_string[conf.l_btn]);
  option_menu->item = list_append(option_menu->item, gitem);
  option_menu->nb_elem++;

  gitem = gn_menu_create_item("Set R Button", MENU_LIST, set_abxylr_button, (void *)5);
  gitem->str = malloc(32);
  sprintf(gitem->str, "%s", abxylr_btn_string[conf.r_btn]);
  option_menu->item = list_append(option_menu->item, gitem);
  option_menu->nb_elem++;


  gitem = gn_menu_create_item("Save conf for every game", MENU_ACTION, save_conf_action, (void *)0);
  option_menu->item = list_append(option_menu->item, gitem);
  option_menu->nb_elem++;

  gitem = gn_menu_create_item("Save conf for this game", MENU_ACTION, save_conf_action, (void *)1);
  option_menu->item = list_append(option_menu->item, gitem);
  option_menu->nb_elem++;

  yesno_menu = create_menu(NULL, MENU_SMALL, NULL, NULL);
  gitem = gn_menu_create_item("Yes", MENU_ACTION, yes_action, NULL);
  yesno_menu->item = list_append(yesno_menu->item, (void *) gitem);
  yesno_menu->nb_elem++;

  gitem = gn_menu_create_item("no", MENU_ACTION, no_action, NULL);
  yesno_menu->item = list_append(yesno_menu->item, (void *) gitem);
  yesno_menu->nb_elem++;
}

uint32_t run_menu(void)
{
  int a;
  static uint32_t init = 0;

  if(init == 0) {
    init = 1;
    gn_init_menu();
  }
  init_back();
  reset_frame_skip();

  gn_menu_disable_item(main_menu, "Load state");
  if(conf.game == NULL) {
    gn_menu_disable_item(main_menu, "Save state");
    gn_menu_disable_item(option_menu, "Save conf for this game");
  }
  else {
    uint32_t nb_slot = how_many_slot(conf.game);
    gn_menu_enable_item(main_menu, "Save state");
    gn_menu_enable_item(option_menu, "Save conf for this game");
    if(nb_slot > 0) {
      gn_menu_enable_item(main_menu, "Load state");
    }
  }

  while(1) {
    main_menu->draw(main_menu);
    if((a = main_menu->event_handling(main_menu)) > 0) {
      return a;
    }
  }
  if(conf.game == NULL) {
    return 2;
  }
  return 0;
}

