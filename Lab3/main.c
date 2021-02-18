#include "tm4c123gh6pm.h"

# define GPIO_PORTF_DATA_RD         (*(( volatile unsigned long *)0x40025040))
# define GPIO_PORTF_DATA_WR         (*(( volatile unsigned long *)0x40025020))
#define GPIO_PORTF_AFSEL_R          (*((volatile unsigned long *)0x40025420))
#define GPIO_PORTF_AMSEL_R          (*((volatile unsigned long *)0x40025528))
#define GPIO_PORTF_DATA_R           (*((volatile unsigned long *)0x400253FC))
#define GPIO_PORTF_DATA_R_PF1       (*((volatile unsigned long *)0x40025008))
#define GPIO_PORTF_DEN_R            (*((volatile unsigned long *)0x4002551C))
#define GPIO_PORTF_DIR_R            (*((volatile unsigned long *)0x40025400))
#define GPIO_PORTF_PCTL_R           (*((volatile unsigned long *)0x4002552C))
#define SYSCTL_RCGCGPIO_R           (*((volatile unsigned long *)0x400FE608))
#define SYSCTL_PRGPIO_R             (*((volatile unsigned long *)0x400FEA08))
#define NVIC_ST_CTRL_R              (*((volatile unsigned long *)0xE000E010))
#define NVIC_ST_RELOAD_R            (*((volatile unsigned long *)0xE000E014))
#define NVIC_ST_CURRENT_R           (*((volatile unsigned long *)0xE000E018))
# define GPIO_PORTF_PIN3_EN         0x08
# define GPIO_PORTF_PIN4_EN         0x10
# define SYSTEM_CLOC_FREQUENCY      16000000
# define DELAY_DEBOUNCE             SYSTEM_CLOC_FREQUENCY/1000

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

int main ()
{
    Config_SysTick();
    Config_Port_F();
    static char flag = 0;
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGC2_GPIOF;
    GPIO_PORTF_DEN_R |= GPIO_PORTF_PIN3_EN +GPIO_PORTF_PIN4_EN;
    GPIO_PORTF_DIR_R |= GPIO_PORTF_PIN3_EN;
    GPIO_PORTF_DIR_R &= (~GPIO_PORTF_PIN4_EN);
    GPIO_PORTF_PUR_R |= GPIO_PORTF_PIN4_EN;

while(1)
{
    if(GPIO_PORTF_DATA_RD == 0)
    {
        Delay(DELAY_DEBOUNCE);
        if(( flag == 0) && (GPIO_PORTF_DATA_RD == 0))
        {
            int i = 0;
            for(i = 0; i < 5; i++)
            {
                flag = 1;
                ms_delay(498);
                GPIO_PORTF_DATA_R ^= 0x02;
                ms_delay(498);
                GPIO_PORTF_DATA_R ^= 0x02;
                GPIO_PORTF_DATA_R ^= 0x04;
                ms_delay(498);
                GPIO_PORTF_DATA_R ^= 0x04;
                GPIO_PORTF_DATA_R ^= 0x08;
                ms_delay(498);
                GPIO_PORTF_DATA_R ^= 0x08;
            }
        }
    }
    else
    {
        flag = 0;
    }
}
}
