
/* EENG 260 Example exercise program listing */

#define GPIO_PORTF_AFSEL_R      (*((volatile unsigned long *)0x40025420))
#define GPIO_PORTF_AMSEL_R      (*((volatile unsigned long *)0x40025528))
#define GPIO_PORTF_DATA_R       (*((volatile unsigned long *)0x400253FC))
#define GPIO_PORTF_DATA_R_PF1   (*((volatile unsigned long *)0x40025008))
#define GPIO_PORTF_DATA_R_PF3   (*((volatile unsigned long *)0x40025020))
#define GPIO_PORTF_DEN_R        (*((volatile unsigned long *)0x4002551C))
#define GPIO_PORTF_DIR_R        (*((volatile unsigned long *)0x40025400))
#define GPIO_PORTF_PCTL_R       (*((volatile unsigned long *)0x4002552C))
#define GPIO_PORTF_PUR_R        (*((volatile unsigned long *)0x40025510))
#define GPIO_PORTF_IS_R         (*((volatile unsigned long *)0x40025404))
#define GPIO_PORTF_IBE_R        (*((volatile unsigned long *)0x40025408))
#define GPIO_PORTF_IEV_R        (*((volatile unsigned long *)0x4002540C))
#define GPIO_PORTF_IM_R         (*((volatile unsigned long *)0x40025410))
#define GPIO_PORTF_ICR_R        (*((volatile unsigned long *)0x4002541C))

#define TIMER0_CFG_R            (*((volatile unsigned long *)0x40030000))
#define TIMER0_TAMR_R           (*((volatile unsigned long *)0x40030004))
#define TIMER0_TBMR_R           (*((volatile unsigned long *)0x40030008))
#define TIMER0_CTL_R            (*((volatile unsigned long *)0x4003000C))
#define TIMER0_IMR_R            (*((volatile unsigned long *)0x40030018))
#define TIMER0_RIS_R            (*((volatile unsigned long *)0x4003001C))
#define TIMER0_MIS_R            (*((volatile unsigned long *)0x40030020))
#define TIMER0_ICR_R            (*((volatile unsigned long *)0x40030024))
#define TIMER0_TAILR_R          (*((volatile unsigned long *)0x40030028))
#define TIMER0_TAPR_R           (*((volatile unsigned long *)0x40030038))

#define SYSCTL_RCGCGPIO_R       (*((volatile unsigned long *)0x400FE608))
#define SYSCTL_PRGPIO_R         (*((volatile unsigned long *)0x400FEA08))
#define SYSCTL_RCGCTIMER_R      (*((volatile unsigned long *)0x400FE604))
#define SYSCTL_PRTIMER_R        (*((volatile unsigned long *)0x400FEA04))

#define NVIC_ST_CTRL_R          (*((volatile unsigned long *)0xE000E010))
#define NVIC_ST_RELOAD_R        (*((volatile unsigned long *)0xE000E014))
#define NVIC_ST_CURRENT_R       (*((volatile unsigned long *)0xE000E018))
#define NVIC_EN0_R              (*((volatile unsigned long *)0xE000E100))
#define NVIC_PRI4_R             (*((volatile unsigned long *)0xE000E410))
#define NVIC_PRI7_R             (*((volatile unsigned long *)0xE000E41C))
#define NVIC_SYS_PRI3_R         (*((volatile unsigned long *)0xE000ED20))

void Disable_Interrupts(void)
{
__asm (" CPSID I\n"
        " BX LR\n");
}
void Enable_Interrupts(void)
{
__asm (" CPSIE I\n"
        " BX LR\n");
}

void Config_SysTick(void)
{
    NVIC_ST_CTRL_R = 0x0;
    NVIC_ST_RELOAD_R = 0x00FFFFFF;
    NVIC_ST_CURRENT_R = 0x0;
    NVIC_ST_CTRL_R = 0x00000005;
}


void Config_Port_F(void)
{
    SYSCTL_RCGCGPIO_R |= 0x20;              // Enable clock for Port F
    while((SYSCTL_PRGPIO_R &0x20) == 0){};   // Check if Port F is ready
    GPIO_PORTF_AMSEL_R &= ~0x1E;            // Disable analog function for PF1-PF4
    GPIO_PORTF_PCTL_R &= ~0x000FFFF0;       // Configure PF1-PF3 as I/O ports
    GPIO_PORTF_DIR_R |= 0x0E;               // PF1-PF3 set as outputs
    GPIO_PORTF_DIR_R &= 0xEF;               // PF4 set as an input
    GPIO_PORTF_AFSEL_R &= ~0x1E;            // Disable alternate function for PF1-PF4
    GPIO_PORTF_DEN_R |= 0x1E;               // Enable digital I/O for PF1-PF4
    GPIO_PORTF_DATA_R &= ~0x0E;             // Clear PF1-PF3
    GPIO_PORTF_PUR_R |= 0x10;                // Set bit 4, weak pull-up on PF4 enabled

    /* Interrupt settings for PF4 */
    GPIO_PORTF_IM_R &= ~0x10;                               // Mask interrupt on PF4
    GPIO_PORTF_IS_R &= ~0x10;                               // Set PF4 to edge-sensitive
    GPIO_PORTF_IBE_R &= ~0x10;                               // Set PF4 for not both edges
    GPIO_PORTF_IEV_R |= 0x10;                              // Set PF4 for falling edge detect
    GPIO_PORTF_ICR_R = 0x10;                                // Clear interrupt flag in GPIORIS and GPIOMIS
    NVIC_PRI7_R = (NVIC_PRI7_R&0xFF00FFFF)|0x00A00000;      // Set to priority level 5
    NVIC_EN0_R |= 0x40000000;                               // Enable interrupt 30 in NVIC
    GPIO_PORTF_IM_R |= 0x10;                                // Unmask interrupt on PF4
}

void Config_Timer0A(void)
{
    SYSCTL_RCGCTIMER_R |= 0x01;                 // Activate TIMER0
    while((SYSCTL_PRTIMER_R & 0x01) == 0){};    // Check if Timer0 is ready
    TIMER0_CTL_R &= ~0x00000001;                // Disable TIMER0A
    TIMER0_CFG_R = 0x00000000;                  // Select 32-bit timer configuration
    TIMER0_TAMR_R = 0x00000001;                 // Select one-shot mode, timer counting down
    TIMER0_TAPR_R = 0;                          // Prescale set to 0
}

void Timer0A_delay(unsigned long delay)
{
    TIMER0_ICR_R = 0x00000001;                  // Clear TIMER0A timeout flag
    TIMER0_TAILR_R = delay;                     // Load count interval
    TIMER0_CTL_R |= 0x00000001;                 // Enable TIMER0A
    while((TIMER0_RIS_R & 0x01) == 0);          // Wait for timeout flat to set
    TIMER0_CTL_R &= ~0x00000001;                // Disable TIMER0A
}

void SysTick_delay(void)
{
    NVIC_ST_RELOAD_R = 0x3E7F;                      // Load value for 1 ms delay
    NVIC_ST_CURRENT_R = 0x0;                        // Clear current and start timer
    while((NVIC_ST_CTRL_R & 0x00010000) == 0){};    // Wait until COUNT flag is set
}


void ms_delay(unsigned long count)
{
    unsigned long i;
    for(i=0; i<count; i++)
    {
        SysTick_delay();
    }
}

void GPIOPortF_Handler(void)
    {
        GPIO_PORTF_IM_R &= ~0x10;           // Mask interrupt on PF4
        ms_delay(498);
        GPIO_PORTF_ICR_R = 0x10;            // Clear interrupt flag
        GPIO_PORTF_DATA_R ^= 0x02;      // Green LED on
        ms_delay(498);
        GPIO_PORTF_DATA_R ^= 0x04;
        ms_delay(498);
        GPIO_PORTF_DATA_R ^= 0x08;
        GPIO_PORTF_IM_R |= 0x10;            // Unmask interrupt on PF4
    }

int main(void)
{
    Disable_Interrupts();
    Config_SysTick();
    Config_Port_F();
    Enable_Interrupts();

    while(1)
    {
        ms_delay(498);
        GPIO_PORTF_DATA_R ^= 0x02;
    }

    return 0;
}
