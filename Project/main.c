#include "TM4C123GH6PM.h"

// Function prototypes initialize, transmit and receive functions
void I2C3_Init ( void );
char I2C3_Write_Multiple(int slave_address, char slave_memory_address, int bytes_count, char* data);
char I2C3_read_Multiple(int slave_address, char slave_memory_address, int bytes_count, char* data);

#define SYSCTL_RCGCGPIO_R       (*((volatile unsigned long *)0x400FE608))
#define SYSCTL_RCGCI2C_R        (*((volatile unsigned long *)0x400FE620))
#define GPIO_PORTF_LOCK_R       (*((volatile unsigned long *)0x40025520)
#define GPIO_PORTF_PUR_R        (*((volatile unsigned long *)0x40025510))
#define GPIO_PORTF_DIR_R        (*((volatile unsigned long *)0x40025400))
#define GPIO_PORTF_DEN_R        (*((volatile unsigned long *)0x4002551C))
#define GPIO_PORTF_DATA_R       (*((volatile unsigned long *)0x400253FC))
#define SYSCTL_RCGCI2C_R        (*((volatile unsigned long *)0x400FE620))
#define GPIO_PORTD_DEN_R        (*((volatile unsigned long *)0x4000751C))
#define GPIO_PORTD_AFSEL_R      (*((volatile unsigned long *)0x40007420))
#define GPIO_PORTD_PCTL_R       (*((volatile unsigned long *)0x4000752C))
#define GPIO_PORTD_ODR_R        (*((volatile unsigned long *)0x4000750C))
#define I2C3_MCR_R              (*((volatile unsigned long *)0x40023020))
#define I2C3_MTPR_R             (*((volatile unsigned long *)0x4002300C))
#define I2C3_MCS_R              (*((volatile unsigned long *)0x40023004))
#define I2C1_MSA_R              (*((volatile unsigned long *)0x40021000))
#define I2C3_MSA_R              (*((volatile unsigned long *)0x40023000))
#define I2C3_MCS_R              (*((volatile unsigned long *)0x40023004))
#define I2C3_MDR_R              (*((volatile unsigned long *)0x40023008))
#define I2C3_MCR_R              (*((volatile unsigned long *)0x40023020))
#define I2C1_MCS_R              (*((volatile unsigned long *)0x40021004))
#define I2C1_MDR_R              (*((volatile unsigned long *)0x40021008))

int main(void)
{
     char data[2] = {0x01,0x02};
     I2C3_Init();
     SYSCTL_RCGCGPIO_R |= 0x20;   /* enable clock to GPIOF */
     GPIO_PORTF_LOCK_R  = 0x4C4F434B;   // unlockGPIOCR register
     GPIO_PORTF_PUR_R |= 0x10;        // Enable Pull Up resistor PF4
     GPIO_PORTF_DIR_R |= 0x08;          //set PF1 as an output and PF4 as an input pin
     GPIO_PORTF_DEN_R |= 0x18;         // Enable PF1 and PF4 as a digital GPIO pins

    while (1)
    {
    unsigned int    value = GPIO_PORTF_DATA_R;
        value = value >> 1;
        GPIO_PORTF_DATA_R = value;
        if(value & (5<<1))
            I2C3_Write_Multiple(4, 0, 1, &data[1]);
        else
            I2C3_Write_Multiple(4, 0, 1, &data[0]);

    }

}
// I2C intialization and GPIO alternate function configuration
void I2C3_Init ( void )
{
    SYSCTL_RCGCGPIO_R  |= 0x00000008 ; // Enable the clock for port D
    SYSCTL_RCGCI2C_R   |= 0x00000008 ; // Enable the clock for I2C 3
    GPIO_PORTD_DEN_R   |= 0x03; // Assert DEN for port D

    GPIO_PORTD_AFSEL_R |= 0x00000003 ;
    GPIO_PORTD_PCTL_R  |= 0x00000033 ;
    GPIO_PORTD_ODR_R   |= 0x00000002 ; // SDA (PD1 ) pin as open darin
    I2C3_MCR_R          = 0x0010 ; // Enable I2C 3 master function

    I2C3_MTPR_R         = 0x07 ;
}


static int I2C_wait_till_done(void)
{
    while(I2C3_MCS_R & 1);   /* wait until I2C master is not busy */
    return I2C3_MCS_R & 0xE; /* return I2C error code, 0 if no error*/
}
// Receive one byte of data from I2C slave device
char I2C3_Write_Multiple(int slave_address, char slave_memory_address, int bytes_count, char* data)
{
    char error;
    if (bytes_count <= 0)
        return -1;                  /* no write was performed */
    /* send slave address and starting address */
    I2C3_MSA_R = slave_address << 1;
    I2C3_MDR_R   = slave_memory_address;
    I2C3_MCS_R = 3;                  /* S-(saddr+w)-ACK-maddr-ACK */

    error = I2C_wait_till_done();   /* wait until write is complete */
    if (error) return error;

    /* send data one byte at a time */
    while (bytes_count > 1)
    {
        I2C3_MDR_R   = *data++;             /* write the next byte */
        I2C3_MCS_R = 1;                   /* -data-ACK- */
        error = I2C_wait_till_done();
        if (error) return error;
        bytes_count--;
    }

    /* send last byte and a STOP */
    I2C3_MDR_R   = *data++;                 /* write the last byte */
    I2C3_MCS_R = 5;                       /* -data-ACK-P */
    error = I2C_wait_till_done();
    while(I2C3_MCS_R & 0x40);             /* wait until bus is not busy */
    if (error) return error;
    return 0;       /* no error */
}
