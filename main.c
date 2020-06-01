#include <msp430.h>

/**
 * main.c
 */

unsigned int RisingEdge;
unsigned int FallingEdge;
unsigned int PulseTimes;
unsigned short First;
unsigned int distance;
void timer_init(void);
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    //-------------------------- motor initiation -------------------------------------------------

    P2DIR |= BIT1; // Initialize direction for motor
    P2DIR |= BIT2; // Initialize pwm output for motor
    P2SEL |= BIT2; // P2sel is set to 1
    P2SEL2 &= ~ BIT2; // P2sel2 is set to 0, this combination is to generation PWM signal

    P2DIR |= BIT5; // Initialize direction for motor
    P2DIR |= BIT4; // Initialize pwm output for motor
    P2SEL |= BIT4; // P2sel is set to 1
    P2SEL2 &= ~ BIT4; // P2sel2 is set to 0, this combination is to generation PWM signal

    TA1CTL = 0;
    TA1CTL |= (MC_1 | ID_0 | TACLR | TASSEL_2);
    TA1CCR0 = 10000; // Maximum speed using frequency
    TA1CCTL1 = OUTMOD_7; // Output mode = set reset
    TA1CCTL2 |= OUTMOD_7;
    TA1CCR1 = 1000;
    TA1CCR2 = 1000;
    P2OUT &= ~(BIT1);
    P2OUT |= (BIT5);

//--------------------------------------ultra sonic sensor------------------------------------

    TA0CTL = (TASSEL_2 | MC_2); // source horloge SMCLK + mode continu
    TA0CCTL1 = CM_3 + CAP + CCIS_0 + SCS + CCIE;
    TA0CCTL1 &= ~CCIFG;

    P1DIR |= BIT1;
    P1OUT &= ~BIT1;

    P1IE |= BIT2;
    P1IFG &= ~BIT2;

    P1SEL |= BIT2;

    init_adc(0x07);
    int irLtest;
    First = 0;
    _BIS_SR(GIE);
    while (1)
    {

        trigger_echo();
        int irLtest = read_adc(0x07);
        if ((irLtest > 600) && (distance < 10))
        {
            reverse();
            Timer_Delay_20ms();
            Timer_Delay_20ms();
        }
        else if (irLtest > 600)
        {
            stop();
            __delay_cycles(3000);
            ir_leftmove();

        }
        else if ((distance < 10) && ((distance = 1)))
        {
            stop();
            us_rightmove();

        }
        else
        {
            straight();
        }
    }
}

void trigger_echo()
{
    P1OUT |= BIT1;
    __delay_cycles(20);
    P1OUT &= ~BIT1;
    __delay_cycles(30000);
}

//------------------------------------------ ultra sonic timer calculation --------------------------------
#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer0_A1(void)
{
    switch (TA0IV)
    {
    case (TA0IV_TACCR1):

    {
        if (First == 0)
        {
            RisingEdge = TACCR1;
            First = 1;
        }
        else
        {
            FallingEdge = TACCR1;
            PulseTimes = FallingEdge - RisingEdge;
            First = 0;
            TACCR1 = 0;
            distance = (PulseTimes) / 58;
        }
    }
    }
    TA0CCTL1 &= ~CCIFG;
}

//------------------------------ movement control ------------------------------------

void wheelRight(int speed2, int dir2)
{
    TA1CCTL2 = OUTMOD_7; // Output mode = set reset
    TA1CCR2 = speed2;

    if (dir2 == 0)
    {
        P2OUT |= BIT5;
    }
    if (dir2 == 1)
    {
        P2OUT &= ~ BIT5;
    }
}

void wheelLeft(int speed1, int dir1)
{
    TA1CCTL1 = OUTMOD_7; // Output mode = set reset
    TA1CCR1 = speed1;

    if (dir1 == 1)
    {
        P2OUT |= BIT1;
    }
    if (dir1 == 0)
    {
        P2OUT &= ~ BIT1;
    }
}

void stop()
{
    wheelLeft(0, 0);
    wheelRight(0, 0);
}

void straight()
{

    wheelRight(1000, 0);
    wheelLeft(1000, 0);
}

void ir_leftmove()
{
    wheelLeft(2000, 0);
    wheelRight(0, 0);
// Timer_Delay_300ms(4);
}

void us_rightmove()
{
    wheelLeft(0, 0);
    wheelRight(2000, 0);
// Timer_Delay_300ms(4);
}

void reverse()
{
    wheelLeft(1000, 1);
    wheelRight(2000, 1);
// Timer_Delay_300ms(4);
}

void Timer_Delay_20ms(void) //Timer for 20 ms delay
{
    __delay_cycles(200000);
}
void Timer_Delay_300ms(int delay_time) // Timer for specific amount of delay
{
    int i;
    for (i = 0; i < delay_time; i++)
    {
        Timer_Delay_20ms();
    }
}
