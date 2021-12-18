//
// Created by Dominik Lovetinsky on 12.11.2020.
//

#ifndef TIMERLIB_TIMER_H
#define TIMERLIB_TIMER_H

/**Specifies the overflow-trigger value for the predefined interval*/
int getOverflowTrigger(float secs, int &pre, int bits);

/**Resolves the Pre-Scaler; Example: setPreScaler(64), so it will return 0b11, works for T0 and T1*/
int setPreScaler_T0_T1(int pre);

/**Resolves the Pre-Scaler; Example: setPreScaler(64), so it will return 0b10, works for T2*/
int setPreScaler_T2(int pre);

/**Initializes Timer0 as 16 Bit CTC Timer; Interrupt: ISR(TIMER0_COMPA_vect)*/
int initTimer0(float secs, int pre);

/**Initializes Timer1 as 16 Bit CTC Timer; Interrupt: ISR(TIMER1_COMPA_vect)*/
int initTimer1(float secs, int pre);

/**Initializes Timer2 as 8 Bit CTC Timer; Interrupt: ISR(TIMER2_COMPA_vect)*/
int initTimer2(float secs, int pre);

#endif //TIMERLIB_TIMER_H
