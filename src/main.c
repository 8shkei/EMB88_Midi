#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdlib.h>

// #include "Megalovania.h"
// #include "Night_of_Nights.h"
// #include "Asgore.h"
// #include "Canon.h"
// #include "Prelude.h"
// #include "Through_the_Fire_and_Flames.h"
// #include "Mario_Underwater_Theme.h"
// #include "Welcome_to_the_JapariPark.h"
// #include "U.N.Owen_was_her.h"
// #include "Nyanyanyanyanyanyanya!.h"
// #include "DangerZone.h"
// #include "Mario.h"
#include "Dial_Up.h"

#define vol 100

#ifndef ncmax
	#define ncmax 255
#endif
#ifndef o2a
	#define o2a 255
#endif
unsigned char notes[tc];
unsigned char note=0;

unsigned int  len=0;
unsigned int  lens[tc];
unsigned int  num[tc];
unsigned char wait[tc];
unsigned char *trackp[tc];
unsigned char mostlong;
void setup(){
#if tc>0
		trackp[0]=(unsigned char *)&track1;
		lens[0]=sizeof(track1)/sizeof(*track1);
#endif
#if tc>1
		trackp[1]=(unsigned char *)&track2;
		lens[1]=sizeof(track2)/sizeof(*track2);
#endif
#if tc>2
		trackp[2]=(unsigned char *)&track3;
		lens[2]=sizeof(track3)/sizeof(*track3);
#endif
#if tc>3
		trackp[3]=(unsigned char *)&track4;
		lens[3]=sizeof(track4)/sizeof(*track4);
#endif
	mostlong=0;
	for(int i = 0;i<tc;i++){
		if(lens[i]>len){
			len=lens[i];
			mostlong=i;
		}
		num[i]=0;
		wait[i]=0;
		vols[i]=vol*(vols[i]/100.0);
	}
	#ifdef includenoise
		srand(10);
	#endif
}

ISR(TIMER1_COMPA_vect){
	for(int i = 0;i<tc;i++){
		if(wait[i]==0){
			if(num[i]>=lens[i]){
				if(num[i]>=len){
					TIMSK1= 0;
					for(int j = 0;j<tc;j++){
						num[j]=0;
						wait[j]=1;
						notes[j]=0;
					}
				}else notes[i]=0;
			}else{
				notes[i]=pgm_read_byte(trackp[i]+ (num[i] * 2));
				wait[i]=pgm_read_byte(trackp[i]+ (num[i] * 2)+1);
				num[i]++;
			}
		}
		wait[i]--;
	}
}

ISR(TIMER2_OVF_vect){
	static unsigned char cnt = 0;
	static unsigned int nc = 0;
	
	#ifdef includenoise
		static unsigned char noise = 0;
		if(cnt++>=notes[note]&&!noise){
			cnt=0;
			OCR2B=0;
		}else if(cnt>=noise&&noise){
			cnt=0;
			OCR2B=0;
		}else{
			if(!noise)
			OCR2B=notes[note]?vols[note]:0;
			else
			OCR2B=noise?vols[note]:0;
		}
	#else
		if(cnt++>=notes[note]){
			cnt=0;
			OCR2B=0;
		}else{
			OCR2B=notes[note]?vols[note]:0;
		}
	#endif
	if(nc++>=ncmax){
		nc=0;
		for(int i = 1;i<tc;i++){
			if(notes[(note+i)%tc]){
				note=(note+i)%tc;
				break;
			}
		}
		#ifdef includenoise
			if(notes[note] < 10 && notes[note])noise = rand()%(notes[note]*10)+(notes[note]*10);
			else noise = 0;
		#endif
	}
}

int main(void){
	setup();
	DDRB = 0xFF;
	DDRC = 0x0F;
	DDRD = 0xFE;
	
	PORTB = 0x80;
	PORTD = 0x00;
	PORTC = 0x30;//ここゼロにしてるとスイッチ反応しない。
	
	TCCR2A= 0x23;
	TCCR2B= 0x09;
	OCR2B = 0;
	TIMSK2= 1;
	
	TCCR1A= 0x00;
	TCCR1B= 0x0D;
	OCR1A = 7811/spd;//一秒にspd回割り込み// 8000000/1024-1/spd
	TIMSK1= 0;
	OCR2A=o2a;

	// ADMUX=0x45;
	// ADCSRB=0;
	// ADCSRA=0xA7;
	// ADCSRA |= 0x40;// 変換開始

	sei();//割り込み有効化
	while(1){
		wdt_reset();
		if((~PINC >> 4)&1){
			TIMSK1= 0x0;
			for(int i = 0;i<tc;i++) notes[i]=0;
		}
		if((~PINC >> 4)>>1&1){
			TIMSK1= 0x2;
		}
		// if(ADCSRA & 0x10){ 
		// 	(ADC>>1);
		// 	ADCSRA |= 0x40;
		// }
		PORTB=0x80>>(int)(7.4/len*num[mostlong]);
	}
	return 0;
}