/*
 * This header provides constants for most GPIO bindings.
 *
 * Most GPIO bindings include a flags cell as part of the GPIO specifier.
 * In most cases, the format of the flags cell uses the standard values
 * defined in this header.
 */

#ifndef _DT_BINDINGS_GPIO_GPIO_H
#define _DT_BINDINGS_GPIO_GPIO_H

#define GPIO_ACTIVE_HIGH 0
#define GPIO_ACTIVE_LOW 1

#define  PA  0
#define  PB  1
#define  PC  2
#define  PD  3
#define  PE  4
#define  PF  5
#define  PG  6
#define  PH  7
#define  PI  8
#define  PJ  9
#define  PK  10
#define  PL  11
#define  PM  12
#define  PN  13
#define  PO  14
#define  PP  15
#define  default 0xffffffff
#endif
