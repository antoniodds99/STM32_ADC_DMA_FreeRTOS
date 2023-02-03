/*
 * rgb.c
 *
 *  Created on: Sep 23, 2020
 *      Author: nuno
 */


#include <stdint.h>
#include "stm32f10x.h"
#include "rgb.h"


void sendColor( int8_t red, int8_t green, int8_t blue );
void clk( void );
void sendByte( int8_t b );
void wait_us( void );
void wait_ms( void );


void rgb_init( void )
{

		GPIO_InitTypeDef GPIO_InitStructure;

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
		GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

		/* POWER */
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		/* DIN and CIN */
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4 | GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
}


void rgb_end( void )
{

		GPIO_InitTypeDef GPIO_InitStructure;



		/* POWER */
		//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
		GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		/* DIN and CIN */
		//RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
		GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4 | GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void rgb_power( uint8_t power_state)
{
	//power_state ? :
	GPIO_WriteBit(GPIOA, GPIO_Pin_6, power_state ? Bit_SET : Bit_RESET );

}


void rgb_setcolor( uint8_t red, uint8_t green, uint8_t blue )
{

	// Send data frame prefix (32x "0")
	sendByte(0x00);
	sendByte(0x00);
	sendByte(0x00);
	sendByte(0x00);

	// sendColor(red, green, blue);
    // Start by sending a byte with the format "1 1 /B7 /B6 /G7 /G6 /R7 /R6"
	int8_t prefix = 0xC0;
    if ((blue & 0x80) == 0)     prefix|= 0x20;
    if ((blue & 0x40) == 0)     prefix|= 0x10;
    if ((green & 0x80) == 0)    prefix|= 0x08;
    if ((green & 0x40) == 0)    prefix|= 0x04;
    if ((red & 0x80) == 0)      prefix|= 0x02;
    if ((red & 0x40) == 0)      prefix|= 0x01;
    sendByte(prefix);

    // Send the colors
    sendByte(blue);
    sendByte(green);
    sendByte(red);

	// Terminate data frame (32x "0")
	sendByte(0x00);
	sendByte(0x00);
	sendByte(0x00);
    sendByte(0x00);
}

void sendByte(int8_t b)
{
    // Send one bit at a time, starting with the MSB
    for (uint8_t i=0; i<8; i++)
    {
        // If MSB is 1, write one and clock it, else write 0 and clock
        if ((b & 0x80) != 0)
        		GPIO_WriteBit(GPIOB, GPIO_Pin_4, Bit_SET);
        else
        		GPIO_WriteBit(GPIOB, GPIO_Pin_4, Bit_RESET);
        clk();

        // Advance to the next bit to send
        b <<= 1;
    }
}

void clk(void)
{
	GPIO_WriteBit(GPIOB, GPIO_Pin_10, Bit_RESET);
    wait_us();
    GPIO_WriteBit(GPIOB, GPIO_Pin_10, Bit_SET);
    wait_us();
}

void wait_us( void )
{
volatile uint32_t i;

	for(i=0; i< 1000; i++)
	{
	}
}
