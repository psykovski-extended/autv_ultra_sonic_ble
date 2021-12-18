#include "avr/io.h"
#include "avr/interrupt.h"
#include "./lib/timer.h"
#include "Arduino.h"

static volatile int delay_measured = 0;
static volatile uint16_t result = 0;
static volatile uint8_t data_valid = 0;
static volatile uint8_t data_recieved = 0;

ISR(TIMER0_COMPA_vect)
{ /// timer - isr, triggered every us
    if (PIND & (1 << PD3))
    {                     /// check if PD3 is high
        ++delay_measured; /// increment delay_measured
    }
}

ISR(TIMER1_COMPA_vect)
{ /// timer - isr for trigger - 5Hz
    static unsigned char trigger_level = 0;

    if (trigger_level)
    {
        PORTD &= ~(1 << PD2);    /// clock signal low - falling edge needed for trigger
        TIMSK0 |= (1 << OCIE0A); /// activate timer isr
        data_valid = 0;
    }
    else
    {
        TIMSK0 &= ~(1 << OCIE0A); /// deactivate timer isr
        PORTD |= (1 << PD2);      /// clock signal high
        data_valid = 1;
        if (UCSR0A & (1 << RXC0))
        {
            data_recieved = 1;
        }
    }
    trigger_level = ~trigger_level; // invert signal, to generate 5Hz clock
}

void init()
{
    // timer for clock, 5Hz
    uint16_t pre2 = 256;
    initTimer1(0.1F, pre2);
    // timer for counting signal, latency
    uint16_t pre = 8;
    initTimer0(0.000001F, pre);

    // 0100 - PD3 = ECHO -- input; PD2 = Trigger -- output; PD4 = EN for BLE
    DDRD |= (1 << PD2) | (1 << PD4);
    DDRD &= ~(1 << PD3);

    PORTD |= (1 << PD4);
    sei();
}

void UART_init(uint32_t ubrr) {
    /*set BAUD Rate*/
    UBRR0H = (unsigned char)(ubrr>>8);
    UBRR0L = (unsigned char)ubrr;

    UCSR0A = 0;
    UCSR0B = (1<<RXEN0)|(1<<TXEN0)|(1<<RXCIE0)|(1<<TXCIE0);
    UCSR0C = (3 << UCSZ00);
}

void UART_putc(unsigned char data)
{
    // wait for transmit buffer to be empty
    while (!(UCSR0A & (1 << UDRE0)))
        ;

    // load data into transmit register
    UDR0 = data;
}

void UART_puts(char *s)
{
    // transmit character until NULL is reached
    while (*s > 0)
        UART_putc(*s++);
}

void UART_puthex8(uint8_t val)
{
    // extract upper and lower nibbles from input value
    uint8_t upperNibble = (val & 0xF0) >> 4;
    uint8_t lowerNibble = val & 0x0F;

    // convert nibble to its ASCII hex equivalent
    upperNibble += upperNibble > 9 ? 'A' - 10 : '0';
    lowerNibble += lowerNibble > 9 ? 'A' - 10 : '0';

    // print the characters
    UART_putc(upperNibble);
    UART_putc(lowerNibble);
}

void UART_puthex16(uint16_t val)
{
    // transmit upper 8 bits
    UART_puthex8((uint8_t)(val >> 8));

    // transmit lower 8 bits
    UART_puthex8((uint8_t)(val & 0x00FF));
}

void UART_putU16(uint16_t val)
{
    uint8_t dig1 = '0', dig2 = '0', dig3 = '0', dig4 = '0';

    // count value in 10000s place
    while(val >= 10000)
    {
        val -= 10000;
        dig1++;
    }

    // count value in 1000s place
    while(val >= 1000)
    {
        val -= 1000;
        dig2++;
    }

    // count value in 100s place
    while(val >= 100)
    {
        val -= 100;
        dig3++;
    }

    // count value in 10s place
    while(val >= 10)
    {
        val -= 10;
        dig4++;
    }

    // was previous value printed?
    uint8_t prevPrinted = 0;

    // print first digit (or ignore leading zeros)
    if(dig1 != '0') {UART_putc(dig1); prevPrinted = 1;}

    // print second digit (or ignore leading zeros)
    if(prevPrinted || (dig2 != '0')) {UART_putc(dig2); prevPrinted = 1;}

    // print third digit (or ignore leading zeros)
    if(prevPrinted || (dig3 != '0')) {UART_putc(dig3); prevPrinted = 1;}

    // print third digit (or ignore leading zeros)
    if(prevPrinted || (dig4 != '0')) {UART_putc(dig4); prevPrinted = 1;}

    // print final digit
    UART_putc(val + '0');
}

int main()
{
    UART_init(9600);
    Serial.begin(9600);
    init();

    while (true)
    {
        if (data_valid)
        {
            result = (uint16_t)((uint32_t)delay_measured * 17 / 200);
            UART_putU16(result);
            UART_putc('\n');
            Serial.println(result);
            delay_measured = 0;
            data_recieved = 0;
            data_valid = 0;
        }
        else if (data_recieved)
        {
            data_recieved = 0;
        }
    }
}
