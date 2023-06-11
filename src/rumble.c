#include "memory.h"
#include "rumble.h"

void rumble_checkMemory()
{
//watch for moves - NOTE: need to restrict this on a per-game basis! right now it's running for every single NEO*GEO game!
    uint8_t onLeftSide = mem68k_fetch_ram_byte(0x108131);
    uint16_t p1move = mem68k_fetch_ram_word(0x108102); //actually a long, might cause false positive by only watching from 108102 instead of 108100
    uint16_t p2move = mem68k_fetch_ram_word(0x108302); //actually a long, might cause false positive by only watching from 108302 instead of 108300
    uint8_t triggerByte = mem68k_fetch_ram_byte(0x108175);

    uint8_t p2_damage_accumulator = mem68k_fetch_ram_byte(0x10843b);

//these 2 are global and unrelated to taking damage:
	uint8_t superFlash = mem68k_fetch_ram_byte(0x10A787);
	uint16_t player1ABCexplosion = mem68k_fetch_ram_word(0x1081EA);

//this is to restrict the checking to only P1's current character
	uint8_t p1_currentCharIDoffset = mem68k_fetch_ram_byte(0x10A84A);
	uint8_t p1currentCharID = mem68k_fetch_ram_byte(0x10A84B + p1_currentCharIDoffset);
	//uint8_t p1currentCharID = mem68k_fetch_ram_byte(0x10A84B);

//note that we need to BREAK after every rumble is found, stop checking afterwards!

//PER-CHARACTER - hitting
	if(p2_damage_accumulator!=0)//p2 damage accumulator
	{
		if(p1currentCharID == 0x02)//DAIMON - note that daimon is a special case because several of his moves cause him to change sides, thus requiring the 'onLeftSide' check byte
		{
			//近敌时→↘↓↙←→↘↓↙←＋ＡorＣ
			if(p1move==0x4F6A && (triggerByte==0x7E || triggerByte==0x8A || triggerByte==0x12 || triggerByte==0x18 || triggerByte==0x72))rumble_do(onLeftSide,100);
			if(p1move==0x4F6A && (triggerByte==0x48 || triggerByte==0x54 || triggerByte==0x3C))rumble_do(onLeftSide,100);
			if(p1move==0x4F6A && triggerByte==0x96)rumble_do(onLeftSide,100);
			if(p1move==0x41CC && p2move==0x51C4)rumble_do(onLeftSide,100);
			if(p1move==0x4206 && p2move==0x84DA)rumble_do(onLeftSide,100);

			//近敌时←↙↓↘→←↙↓↘→＋ＢorＤ
			if(p1move==0x5372)rumble_do(onLeftSide,100);

			//近敌时←↙↓↘→＋ＡorＣ
			if(p1move==0x4802)rumble_do(onLeftSide,50);
			if(p2move==0x4460 && (triggerByte==0x42 || triggerByte==0x48))rumble_do(onLeftSide,50);

			//近敌时→↘↓↙←→＋ＢorＤ
			if(p1move==0x4C42)rumble_do(onLeftSide,60);

			//近敌时→↘↓↙←→＋ＡorＣ
			if(p1move==0x4D2C)rumble_do(onLeftSide,40);
			if(p2move==0x4E1C)rumble_do(onLeftSide,40);
		}
		else if(p1currentCharID == 0x00)//KYO
		{
			switch(p1move)
			{
				case 0x17D2://QCF x 2 + P
					rumble_do(onLeftSide,100);
					break;
			}			
		}
		else if(p1currentCharID == 0x1B)//IORI)
		{
			switch(p1move)
			{
				case 0xfab2://QCB + P ( x3 ) 1 and 2 of 3
					if(p2move==0x8a3a)rumble_do(onLeftSide,40);
					break;
				case 0xfa5a://QCB + P ( x3 ) 3 of 3
					if(triggerByte==0x24)rumble_do(onLeftSide,50);
					break;
				case 0xf344://DP + P
					rumble_do(onLeftSide,60);
					break;
				case 0xF5BE://HCB + K
					rumble_do(onLeftSide,70);
					break;
				case 0xF62E://HCB + K
					rumble_do(onLeftSide,70);
					break;
				case 0x0018://QCF , HCB + P
					rumble_do(onLeftSide,100);
					break;
				case 0x0102://final hit
					rumble_do(onLeftSide,100);
					break;
			}
		}
	}

//GLOBAL
	//A+B+C explosion (ADVANCED mode)
	if(player1ABCexplosion>=0x3f00 && player1ABCexplosion<0x5000)
	{
		rumble_do(0,100);
		rumble_do(1,100);
		//mem68k_store_ram_byte(0x10A83A, 0x99);//testing
	}

	if(superFlash>0x00)
	{
		rumble_do(0,100);
		rumble_do(1,100);
		//mem68k_store_ram_byte(0x10A83A, 0x99);//testing		
	}


}

void rumble_do(uint8_t bool_onLeftSide, uint8_t intensity)
{
	/*
	//using HP bars
	uint8_t sendAmount = 0x67*intensity/100;

	if(bool_onLeftSide==1)mem68k_store_ram_byte(0x108239, sendAmount);//P1 on left side	
	else if(bool_onLeftSide==0)mem68k_store_ram_byte(0x108439, sendAmount);//P1 on right side
	*/

	//int currentTime=mem68k_fetch_ram_byte(0x110A83A);
	if(bool_onLeftSide==1)mem68k_store_ram_byte(0x10A83A, 0x88);
	else if(bool_onLeftSide==0)mem68k_store_ram_byte(0x10A83A, 0x11);
}

void rumble_kof97_training()
{
    if(mem68k_fetch_ram_byte(0x10A84A)==0x03)
	{
    	//mem68k_store_ram_byte(0x100000, 0x20);//both players take no damage (debug code)
    	mem68k_store_ram_byte(0x1085d3, 0x00);//chrsel timer goes to zero once player 1 has picked 3 characters
    }

    if(mem68k_fetch_ram_byte(0x10A7F2)==0x01)mem68k_store_ram_byte(0x10A7F2, 0x03);//p2 joins when p1 presses start

    if(mem68k_fetch_ram_byte(0x108100)==0x00)
    {
    	mem68k_store_ram_byte(0x1082E3, 0x03);//p1 infinite meter 1 of 3
    	//mem68k_store_ram_byte(0x1081EA, 0x40);//p1 infinite meter 2 of 3
    	mem68k_store_ram_byte(0x10825F, 0x23);//p1 infinite meter 3 of 3

    	mem68k_store_ram_byte(0x10EC34, 0x1F);//hidden characters

    	mem68k_store_ram_byte(0x108239, 0x67);//p1 infinite HP
      	mem68k_store_ram_byte(0x108439, 0x67);//p2 infinite HP

      	if(mem68k_fetch_ram_byte(0x110A83A)<=0x01)mem68k_store_ram_byte(0x10A83A, 0x99);//infinite time
    }
}

void rumble_temporary_visualization()//this is now unused, because of the conflict between using the HP bars and the no damage debug code.
{
#define p1addressHP 0x108239
#define p2addressHP 0x108439

#define reductionSpeed 0x4

	if(mem68k_fetch_ram_byte(0x100000)==0x20)
    {
		uint8_t p1HP=mem68k_fetch_ram_byte(p1addressHP);
		uint8_t p2HP=mem68k_fetch_ram_byte(p2addressHP);

		if(p1HP>reductionSpeed+1)p1HP-=reductionSpeed;

		if(p2HP>reductionSpeed+1)p2HP-=reductionSpeed;
			
		if(p1HP>reductionSpeed)mem68k_store_ram_byte(p1addressHP, p1HP);

		else mem68k_store_ram_byte(p1addressHP, 0x01);
	
		if(p2HP>reductionSpeed)mem68k_store_ram_byte(p2addressHP, p2HP);
		
		else mem68k_store_ram_byte(p2addressHP, 0x01);
		
	}
}