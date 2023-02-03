/*
 * rgb.h
 *
 *  Created on: Sep 23, 2020
 *      Author: nuno
 */

#ifndef RGB_H_
#define RGB_H_


extern void rgb_init( void );
extern void rgb_end( void );
extern void rgb_power( uint8_t power_state);
extern void rgb_setcolor( uint8_t red, uint8_t green, uint8_t blue );


#endif /* RGB_H_ */
