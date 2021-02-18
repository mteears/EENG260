#include "tm4c123gh6pm.h"

#define SYSCTL_RCGCGPIO_R       (*((volatile unsigned long *)0x400FE608))
#define GPIO_PORTF_DATA_BITS_R  ((volatile unsigned long *)0x40025000)
#define GPIO_PORTF_DATA_R       (*((volatile unsigned long *)0x400253FC))
#define GPIO_PORTF_DIR_R        (*((volatile unsigned long *)0x40025400))
#define GPIO_PORTF_IS_R         (*((volatile unsigned long *)0x40025404))
#define GPIO_PORTF_IBE_R        (*((volatile unsigned long *)0x40025408))
#define GPIO_PORTF_IEV_R        (*((volatile unsigned long *)0x4002540C))
#define GPIO_PORTF_IM_R         (*((volatile unsigned long *)0x40025410))
#define GPIO_PORTF_RIS_R        (*((volatile unsigned long *)0x40025414))
#define GPIO_PORTF_MIS_R        (*((volatile unsigned long *)0x40025418))
#define GPIO_PORTF_ICR_R        (*((volatile unsigned long *)0x4002541C))
#define GPIO_PORTF_AFSEL_R      (*((volatile unsigned long *)0x40025420))
#define GPIO_PORTF_DR2R_R       (*((volatile unsigned long *)0x40025500))
#define GPIO_PORTF_DR4R_R       (*((volatile unsigned long *)0x40025504))
#define GPIO_PORTF_DR8R_R       (*((volatile unsigned long *)0x40025508))
#define GPIO_PORTF_ODR_R        (*((volatile unsigned long *)0x4002550C))
#define GPIO_PORTF_PUR_R        (*((volatile unsigned long *)0x40025510))
#define GPIO_PORTF_PDR_R        (*((volatile unsigned long *)0x40025514))
#define GPIO_PORTF_SLR_R        (*((volatile unsigned long *)0x40025518))
#define GPIO_PORTF_DEN_R        (*((volatile unsigned long *)0x4002551C))
#define GPIO_PORTF_LOCK_R       (*((volatile unsigned long *)0x40025520))
#define GPIO_PORTF_CR_R         (*((volatile unsigned long *)0x40025524))
#define GPIO_PORTF_AMSEL_R      (*((volatile unsigned long *)0x40025528))
#define GPIO_PORTF_PCTL_R       (*((volatile unsigned long *)0x4002552C))
#define GPIO_PORTF_ADCCTL_R     (*((volatile unsigned long *)0x40025530))
#define GPIO_PORTF_DMACTL_R     (*((volatile unsigned long *)0x40025534))
#define NVIC_EN0_R              (*((volatile unsigned long *)0xE000E100))
#define NVIC_PRI7_R             (*((volatile unsigned long *)0xE000E41C))

void Delay(unsigned long counter)
{
    unsigned long i = 0;
    for(i = 0; i<counter; i++);
}

void Config_SysTick(void)
{
    NVIC_ST_CTRL_R = 0x0;
    NVIC_ST_RELOAD_R = 0x00FFFFFF;
    NVIC_ST_CURRENT_R = 0x0;
    NVIC_ST_CTRL_R = 0x00000005;
}

void SysTick_delay(void)
{
    NVIC_ST_RELOAD_R = 0x3E7F;  // Load value for 1 ms delay
    NVIC_ST_CURRENT_R = 0x0;    // Clear current and start timer
    while((NVIC_ST_CTRL_R & 0x00010000) == 0){};   // Wait until COUNT flag is set
}

void ms_delay(unsigned long count)
{
    unsigned long i;
    for(i=0; i<count; i++)
    {
        SysTick_delay();
    }
}

void Config_Port_F(void)
{
    SYSCTL_RCGCGPIO_R |= 0x20;  // Enable clock for Port F
    while((SYSCTL_PRGPIO_R&0x20) == 0){};    // Check if Port F is ready
    GPIO_PORTF_AMSEL_R &= ~0x0E;    // Disable analog function for PF1-PF3
    GPIO_PORTF_PCTL_R &= ~0x0000FFF0;    // Configure PF1-PF3 as I/O ports
    GPIO_PORTF_DIR_R |= 0x0E; // PF1-PF3 set as outputs
    GPIO_PORTF_AFSEL_R &= ~0x0E;  // Disable alternate function for PF1-PF3
    GPIO_PORTF_DEN_R |= 0x0E; // Enable digital I/O for PF1-PF3
    GPIO_PORTF_DATA_R &= ~0x0E; // Clear PF1-PF3
}

int main(void)
{

    Config_SysTick();
    Config_Port_F();

    SYSCTL_RCGCGPIO_R |= (1<<5);   /* Set bit5 of RCGCGPIO to enable clock to PORTF*/

     /* PORTF0 has special function, need to unlock to modify */
    GPIO_PORTF_LOCK_R = 0x4C4F434B;   /* unlock commit register */
    GPIO_PORTF_CR_R = 0x01;           /* make PORTF0 configurable */
    GPIO_PORTF_LOCK_R = 0;            /* lock commit register */


    /*Initialize PF3 as a digital output, PF0 and PF4 as digital input pins */

    GPIO_PORTF_DIR_R &= ~(1<<4)|~(1<<0);  /* Set PF4 and PF0 as a digital input pins */
    GPIO_PORTF_DIR_R  |= (1<<3);           /* Set PF3 as digital output to control green LED */
    GPIO_PORTF_DEN_R  |= (1<<4)|(1<<3)|(1<<0);             /* make PORTF4-0 digital pins */
    GPIO_PORTF_PUR_R  |= (1<<4)|(1<<0);             /* enable pull up for PORTF4, 0 */

    /* configure PORTF4, 0 for falling edge trigger interrupt */
    GPIO_PORTF_IS_R   &= ~(1<<4)|~(1<<0);        /* make bit 4, 0 edge sensitive */
    GPIO_PORTF_IBE_R   &=~(1<<4)|~(1<<0);         /* trigger is controlled by IEV */
    GPIO_PORTF_IEV_R  &= ~(1<<4)|~(1<<0);        /* falling edge trigger */
    GPIO_PORTF_ICR_R   |= (1<<4)|(1<<0);          /* clear any prior interrupt */
    GPIO_PORTF_IM_R   |= (1<<4)|(1<<0);          /* unmask interrupt */

    /* enable interrupt in NVIC and set priority to 3 */
    NVIC_PRI7_R  |= (1<<21)|(0<<22)|(1<<23);     /* set interrupt priority to 3 */
    NVIC_EN0_R   |= 0x10;  /* enable IRQ30 (D30 of ISER[0]) */


    while(1)
    {
        GPIO_PORTF_DATA_R ^= 0x02;
        ms_delay(498);
        GPIO_PORTF_DATA_R ^= 0x02;
        ms_delay(498);
    }
}


void GPIOF_Handler(void)
{
    GPIO_PORTF_ICR_R |= (1<<4);
    GPIO_PORTF_DATA_R ^= 0x0E;
}
