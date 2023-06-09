#include <stdio.h>
#include "ym2610_interf.h"
#include "emu.h"
#include "memory.h"
#include "timer.h"
#include "sound.h"

static timer_struct *Timer[2];

/*------------------------- TM2610 -------------------------------*/
/* IRQ Handler */
/*
static void IRQHandler(int n, int irq)
{
    //printf("IRQ!!!\n");
    neogeo_sound_irq(irq);
}*/

/* Timer overflow callback from timer.c */
void timer_callback_2610(int param)
{
  int c = param;

  Timer[c] = 0;
  YM2610TimerOver(c);
}

/* TimerHandler from fm.c */
static void TimerHandler(int c, int count, double stepTime)
//static void TimerHandler(int c, int count, Uint32 stepTime)
{
  //printf("TimerHandler %d %d %f\n",c,count,stepTime);
  if(count == 0) {		/* Reset FM Timer */
    if(Timer[c]) {
      del_timer(Timer[c]);
      Timer[c] = 0;
    }
  }
  else {			/* Start FM Timer */
    double timeSec = (double) count * stepTime;
    //Uint32 timeSec = count * (Uint32)(stepTime*(1<<TIMER_SH));

    if(Timer[c] == 0) {
      Timer[c] =
        (timer_struct *) insert_timer(timeSec, c,
                                      timer_callback_2610);
    }
  }
}
void FMTimerInit(void)
{
  Timer[0] = Timer[1] = 0;
  free_all_timer();
}
int YM2610_sh_start(void)
{
  int rate = SAMPLE_RATE;
  void *pcmbufa, *pcmbufb;
  int pcmsizea, pcmsizeb;

  /* Timer Handler set */
  FMTimerInit();

  pcmbufa = (void *) memory.rom.adpcma.p;
  pcmsizea = memory.rom.adpcma.size;
  pcmbufb = (void *) memory.rom.adpcmb.p;
  pcmsizeb = memory.rom.adpcmb.size;

  /**** initialize YM2610 ****/
  YM2610Init(8000000, rate, pcmbufa, pcmsizea, pcmbufb, pcmsizeb,
             TimerHandler, neogeo_sound_irq);
  return 0;
}


/************************************************/
/* Sound Hardware Stop				*/
/************************************************/
void YM2610_sh_stop(void)
{
}

/* reset */
void YM2610_sh_reset(void)
{

  YM2610Reset();
}

/************************************************/
/* Status Read for YM2610 - Chip 0		*/
/************************************************/
Uint32 YM2610_status_port_A_r(Uint32 offset)
{
  return YM2610Read(0);
}

Uint32 YM2610_status_port_B_r(Uint32 offset)
{
  return YM2610Read(2);
}

/************************************************/
/* Port Read for YM2610 - Chip 0		*/
/************************************************/
Uint32 YM2610_read_port_r(Uint32 offset)
{
  return YM2610Read(1);
}


/************************************************/
/* Control Write for YM2610 - Chip 0		*/
/* Consists of 2 addresses			*/
/************************************************/
void YM2610_control_port_A_w(Uint32 offset, Uint32 data)
{
  YM2610Write(0, data);
}

void YM2610_control_port_B_w(Uint32 offset, Uint32 data)
{
  YM2610Write(2, data);
}

/************************************************/
/* Data Write for YM2610 - Chip 0		*/
/* Consists of 2 addresses			*/
/************************************************/
void YM2610_data_port_A_w(Uint32 offset, Uint32 data)
{
  YM2610Write(1, data);
}

void YM2610_data_port_B_w(Uint32 offset, Uint32 data)
{
  YM2610Write(3, data);
}

