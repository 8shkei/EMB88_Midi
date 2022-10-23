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
// #include "Dial_Up.h"
// #include "NoiseTest.h"
// #include "Thunderstruck.h"
// #include "CityTrial.h"
// #include "Spelunker.h"
#include "Maizuru.h"

// #define includetaiko
#define includeanalyzer

#define vol 255
#define loop

#ifndef ncmax
	#define ncmax 255
#endif
#ifndef o2a
	#define o2a 255
#endif
unsigned char notes[tc];
unsigned char note=0;

unsigned int  lens[tc];
unsigned int  num[tc];
unsigned char wait[tc];
unsigned char *trackp[tc];
unsigned char mostlong;


#ifdef includetaiko
unsigned char laststate = 0;
#endif

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
		if(lens[i]>lens[mostlong]){
			mostlong=i;
		}
		num[i]=0;
		wait[i]=0;
		vols[i]=vol*(vols[i]/100.0);
	}
	#ifdef includenoise
		srand(10);
	#endif
	#ifdef includetaiko
		UBRR0 = 500000/38400-1;
		UCSR0A= 0;
		UCSR0C= 6;
		UCSR0B= 0x18;
	#endif
}

ISR(TIMER1_COMPA_vect){
	for(int i = 0;i<tc;i++){
		if(wait[i]==0){
			if(num[i]>=lens[i]){
				if(num[i]>=lens[mostlong]){
					#ifndef loop
					TIMSK1= 0;
					#endif
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
			#ifdef includetaiko
				laststate |= notes[0];
			#endif
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
		#ifdef fwaon // 和音と単音の切り替えが多い曲で、切り替わりの違和感を消すためのフラグ
			note=(note+1)%tc;
		#else
			for(int i = 1;i<tc;i++){
				if(notes[(note+i)%tc]){
					note=(note+i)%tc;
					break;
				}
			}
		#endif
		#ifdef includenoise
			if(notes[note] < 10 && notes[note])noise = rand()%(notes[note]*10)+(notes[note]*18);
			else noise = 0;
		#endif
	}
}

#ifdef includetaiko
void sendstring(char str[]){
	int i = 0;
	while(str[i]!='\0'){
		wdt_reset();
		if(UCSR0A&0x20){
			UDR0=str[i];
			i++;
		}
	}
}
void taiko(){
	if(!(laststate & 0x80)){
		if(!(laststate & ~0x80)){
			sendstring("\e[1;37;41mB");
			PORTB=0x00;
		}else if(pgm_read_byte(trackp[0]+ (num[0] * 2)+1)-wait[0]<3 || wait[0]<3){
			PORTB=0xFF;
			laststate &= 0x80;
			sendstring("\e[1;37;42mG");
		}else{
			sendstring("\e[1;37;41mB");
			PORTB=0x00;
		}
		sendstring("\e[0m");
		laststate|=0x80;
		return;
	}else if((PINC >> 4)>>1&1)laststate &= ~0x80;
	
}
#endif

#ifdef includeanalyzer

unsigned char note_sqrt(unsigned char x){
	for(int i = 7;i > 0;i--){
		if((4*i*i) < x) return i;
	}
	if(x)return 0;
	return 8;
}

void analyzer(){
    static unsigned char scan = 0;
	static unsigned int cnt = 0;
	static unsigned char bar[8];
	if(cnt++>49){
		cnt=0;
		if(note_sqrt(notes[note])==scan){
			if(scan<7)bar[(scan+1)%8] = bar[(scan+1)%8]>>1 &0x3F;
			bar[scan] = bar[scan]>>1 &0xF;
			if(scan>0)bar[(scan+7)%8] = bar[(scan+7)%8]>>1 &0x3F;
		}else{
			bar[scan] = (bar[scan]<<1)|0x01;
		}
	}
	PORTB = 0;
	PORTC = (PORTC & 0xF0) | (bar[scan] & 0x0F); 
	PORTD = (PORTD & 0x0F) | (bar[scan] & 0xF0);
	PORTB = 0x01 << scan;
	scan = (scan + 1) &7;
}
#endif

int main(void){
	setup();
	DDRB = 0xFF;
	DDRC = 0x0F;
	DDRD = 0xFE;
	
	PORTB = 0x00;
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

	sei();//割り込み有効化
	while(1){
		wdt_reset();
		if((~PINC >> 4)&1){
			TIMSK1= 0x0;
			for(int i = 0;i<tc;i++) notes[i]=0;
		}else
		if((~PINC >> 4)>>1&1){
			TIMSK1= 0x2;
			for(int i = 0;i<tc;i++)
				notes[i]=num[i]>0?pgm_read_byte(trackp[i]+ ((num[i]-1) * 2)):0;
			#ifdef includetaiko
				taiko();
			#endif
		}
		#ifdef includeanalyzer
			analyzer();
		#else
		#ifndef includetaiko
			PORTB=0x80>>(int)(7.4/lens[mostlong]*num[mostlong]);
		#endif
		#endif
	}
	return 0;
}