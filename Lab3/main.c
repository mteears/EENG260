#include "tm4c123gh6pm.h"

# define GPIO_PORTF_DATA_RD         (*(( volatile unsigned long *)0x40025040))
# define GPIO_PORTF_DATA_WR         (*(( volatile unsigned long *)0x40025020))
# define GPIO_PORTF_PIN3_EN         0x08
# define GPIO_PORTF_PIN3_DI         0x00
# define GPIO_PORTF_PIN4_EN         0x10
# define SYSTEM_CLOC_FREQUENCY      16000000
# define DELAY_DEBOUNCE              SYSTEM_CLOC_FREQUENCY/1000

void Delay(unsigned long counter)
{
    unsigned long i = 0;
    for(i = 0; i<counter; i++);
}


int main ()
{
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
        //Delay(DELAY_DEBOUNCE);
        if(( GPIO_PORTF_DATA_RD  == 0))
        {
            if(flag == 0)
            {
                GPIO_PORTF_DATA_WR = GPIO_PORTF_PIN3_EN;
                flag = 1;
            }
            if(flag == 1)
            {
                GPIO_PORTF_DATA_WR = GPIO_PORTF_PIN3_DI;
                flag = 0;
            }
        }
    }

}
}
