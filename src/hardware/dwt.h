/*
 * Copyright 2014 Patrick Dowling
 *
 * Author: Patrick Dowling for preenfm2 optimization
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DWT_H_
#define DWT_H_

#include "utils/RingBuffer.h"

//#define DEBUG_CPU_USAGE 1

#define REG_DWT_CONTROL 0xE0001000
#define REG_DWT_CYCCNT 0xE0001004
#define REG_SCB_DEMCR 0xE000EDFC

#define DWT_CONTROL ((volatile unsigned int *)REG_DWT_CONTROL)
#define DWT_CYCCNT ((volatile unsigned int *)REG_DWT_CYCCNT)
#define SCB_DEMCT ((volatile unsigned int *)REG_SCB_DEMCR)

#define RESET_DWT_CYCCNT()			\
        do {						\
            *SCB_DEMCT = *SCB_DEMCT | 0x01000000;		\
            *DWT_CYCCNT = 0;				\
            *DWT_CONTROL = *DWT_CONTROL | 1;		\
        } while ( 0 )

#define READ_DWT_CYCCNT()			\
        *(DWT_CYCCNT)


typedef RingBuffer<unsigned int, 32> CYCCNT_buffer;

/**
 * Utitity to automatically track cycles spent within a scope
 * Doesn't support nested measurements!
 */
class scoped_cyccnt
{
public:
    scoped_cyccnt( CYCCNT_buffer &_buffer )
: buffer( _buffer ) {
        RESET_DWT_CYCCNT();
    }

    ~scoped_cyccnt() {
        buffer.insert( READ_DWT_CYCCNT() );
    }

private:
    CYCCNT_buffer &buffer;
};

#define MACRO_CONCAT_(x,y) x##y
#define MACRO_CONCAT(x,y) MACRO_CONCAT_(x,y)

#ifdef DEBUG_CPU_USAGE
#define CYCLE_MEASURE_START( x )			\
        {							\
            scoped_cyccnt MACRO_CONCAT(CYCNT_,__COUNTER__)( x );	\
            do {} while(0)

#define CYCLE_MEASURE_END()			\
        }
#else
#define CYCLE_MEASURE_START( x ) do {} while(0)
#define CYCLE_MEASURE_END() do {} while(0)
#endif


#endif /* DWT_H_ */
