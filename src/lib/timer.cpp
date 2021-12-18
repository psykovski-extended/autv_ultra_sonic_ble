//
// Created by Dominik Lovetinsky on 12.11.2020.
//
#include "timer.h"
#include <avr/io.h>
#include <math.h>

#define CLOCK_SPEED 16000000

int getOverflowTrigger(float secs, int &pre, int bits){
    int overflow = (int)(((float)CLOCK_SPEED/pre) * secs);

    while(overflow>pow(2,bits)-1){
        if(pre<=256 && bits==16) {
            if (pre == 0)pre = 1;
            else if(pre >= 64)pre/=2;
            pre *= 8;
        }else if(bits==8 && pre<=256){
            if(pre==8 || pre==256)pre*=4;
            else pre*=2;
        }else {
                secs /= 2;
        }

        overflow = (int)(((float)CLOCK_SPEED/pre) * secs);
    }
    return overflow - 1;
}

int setPreScaler_T0_T1(int pre){
    switch (pre) {
        case 8: return (1<<CS00);
        case 64: return (1<<CS00)|(1<<CS01);
        case 256: return (1<<CS02);
        case 1024: return (1<<CS02)|(1<<CS00);
        default: return 0;
    }
}

int setPreScaler_T2(int pre){
    switch (pre) {
        case 8: return (1<<CS21);
        case 32: return ((1<<CS20)|(1<<CS21));
        case 64: return (1<<CS22);
        case 128: return ((1<<CS20)|(1<CS22));
        case 256: return ((1<<CS22)|(1<<CS21));
        case 1024: return ((1<<CS22)|(1<<CS21)|(1<<CS20));
        default: return 0;
    }
}

int initTimer0(float secs, int pre){
    int overflow = getOverflowTrigger(secs, pre, 8);

    TCCR0A |= (1 << WGM01);
    OCR0A = overflow;

    TCCR0B |= setPreScaler_T0_T1(pre);

    TIMSK0 |= (1<<OCIE0A);

    return overflow;
}

int initTimer1(float secs, int pre){
    int overflow = getOverflowTrigger(secs, pre, 16);

    TCCR1B |= (1<<WGM12);
    TCCR1B |= setPreScaler_T0_T1(pre);

    OCR1A = overflow;
    TIMSK1 |= (1<<OCIE1A);

    return overflow;
}

int initTimer2(float secs, int pre){
    unsigned char overflow = getOverflowTrigger(secs, pre, 8);

    TCCR2A = (1<<WGM21);
    TCCR2B |= setPreScaler_T2(pre);

    OCR2A = overflow;
    TIMSK2 |= (1 << OCIE2A);

    return overflow;
}
