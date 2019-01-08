#include "gfx.h"

#if (GFX_USE_GINPUT && GINPUT_NEED_TOGGLE) /*|| defined(__DOXYGEN__)*/

#include "src/ginput/ginput_driver_toggle.h"

GINPUT_TOGGLE_DECLARE_STRUCTURE();

void ginput_lld_toggle_init(const GToggleConfig *ptc) {
	GPIO_InitTypeDef gpio_port;
	GPIO_StructInit(&gpio_port);
	gpio_port.GPIO_Pin   = ptc->mask;
	gpio_port.GPIO_Mode  = ptc->mode;
	gpio_port.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(ptc->id, &gpio_port);
}

unsigned ginput_lld_toggle_getbits(const GToggleConfig *ptc) {
	return GPIO_ReadInputData(ptc->id) ;
}

#endif /* GFX_USE_GINPUT && GINPUT_NEED_TOGGLE */
