#include "tm4c123gh6pm.h"
#include <stdio.h>

void I2C3_Init                  ( void );
char I2C3_Write_Multiple        (int slave_address, char slave_memory_address, int bytes_count, char* data);
char I2C3_read_Multiple         (int slave_address, char slave_memory_address, int bytes_count, char* data);

#define SYSCTL_RCGCGPIO_R       (*((volatile unsigned long *)0x400FE608))
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
#define I2C1_MCS_R              (*((volatile unsigned long *)0x40021004))
#define I2C1_MDR_R              (*((volatile unsigned long *)0x40021008))

int main(void)
{
     I2C3_Init();
     char data[2];
     while(1)
     {
         I2C3_read_Multiple(8, 0, 1, &data[0]);
     }
}


void I2C3_Init ( void )
{
    SYSCTL_RCGCGPIO_R    |= 0x00000008 ; // Enable the clock for port D
    SYSCTL_RCGCI2C_R     |= 0x00000008 ; // Enable the clock for I2C 3
    GPIO_PORTD_DEN_R     |= 0x03;        // Assert DEN for port D
                                        // Configure Port D pins 0 and 1 as I2C 3
    GPIO_PORTD_AFSEL_R   |= 0x00000003 ;
    GPIO_PORTD_PCTL_R    |= 0x00000033 ;
    GPIO_PORTD_ODR_R     |= 0x00000002 ; // SDA (PD1 ) pin as open drain
    I2C3_MCR_R           = 0x0010 ;      // Enable I2C 3 master function
    I2C3_MTPR_R          = 0x07 ;
}

static int I2C_wait_till_done(void)
{
    while(I2C3_MCS_R  & 1);               /* wait until I2C master is not busy */
    return I2C3_MCS_R  & 0xE;             /* return I2C error code, 0 if no error*/
}

char I2C3_read_Multiple(int slave_address, char slave_memory_address, int bytes_count, char* data)
{
    char error;

    if (bytes_count <= 0)
        return -1;         /* no read was performed */

    /* send slave address and starting address */
    I2C3_MSA_R = slave_address << 1;
    I2C3_MDR_R = slave_memory_address;
    I2C3_MCS_R = 3;       /* S-(saddr+w)-ACK-maddr-ACK */
    error = I2C_wait_till_done();
    if (error)
        return error;

    /* to change bus from write to read, send restart with slave addr */
    I2C3_MSA_R = (slave_address << 1) + 1;   /* restart: -R-(saddr+r)-ACK */

    if (bytes_count == 1)             /* if last byte, don't ack */
        I2C3_MCS_R = 7;              /* -data-NACK-P */
    else                            /* else ack */
        I2C3_MCS_R = 0xB;            /* -data-ACK- */
    error = I2C_wait_till_done();
    if (error) return error;

    *data++ = I2C3_MDR_R;            /* store the data received */

    if (--bytes_count == 0)           /* if single byte read, done */
    {
        while(I2C3_MCS_R & 0x40);    /* wait until bus is not busy */
        return 0;       /* no error */
    }

    /* read the rest of the bytes */
    while (bytes_count > 1)
    {
        I2C3_MCS_R = 9;              /* -data-ACK- */
        error = I2C_wait_till_done();
        if (error) return error;
        bytes_count--;
        *data++ = I2C3_MDR_R;        /* store data received */
    }

    I2C3_MCS_R = 5;                  /* -data-NACK-P */
    error = I2C_wait_till_done();
    *data = I2C3_MDR_R;              /* store data received */
    while(I2C3_MCS_R & 0x40);        /* wait until bus is not busy */

    return 0;       /* no error */
}
