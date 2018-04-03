/*
 * gpio_ext.c
 *
 */

#include "gpio_ext.h"
#include "errno.h"


static inline int port_type_to_int(PORT_Type *port)
{
	switch ((int) port) {
	case PORTA_BASE:
		return 0;
	case PORTB_BASE:
		return 1;
	case PORTC_BASE:
		return 2;
	case PORTD_BASE:
		return 3;
	case PORTE_BASE:
		return 4;
	default:
		return -EINVAL;
	}
}

/* returns GPIO index in gpio_list table and -EINVAL
 * if there is no entry for this gpio.
 */
int is_gpio_valid(uint8_t pin)
{
	uint16_t i;
	int temp;
	if (pin == 0xFF)
		return -EINVAL;

	for (i = 0; i < sizeof(gpio_list)/sizeof(struct gpio_id); i++){
		temp = port_type_to_int(gpio_list[i].port) * 32;
		temp += gpio_list[i].pin;
		if ( temp == pin )
			return i;
	}

	return -EINVAL;
}



