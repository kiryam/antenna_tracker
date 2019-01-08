#include "stm32f10x.h"

#ifndef _GDISP_LLD_TOGGLE_BOARD_H
#define _GDISP_LLD_TOGGLE_BOARD_H

#define GINPUT_TOGGLE_NUM_PORTS			7			// The total number of toggle inputs
#define GINPUT_TOGGLE_CONFIG_ENTRIES	1			// The total number of GToggleConfig entries

#define GINPUT_TOGGLE_DECLARE_STRUCTURE()											\
const GToggleConfig GInputToggleConfigTable[GINPUT_TOGGLE_CONFIG_ENTRIES] = {	\
		{GPIOC,								\
			GPIO_Pin_7,											\
			GPIO_Pin_7 ,											\
			GPIO_Mode_IPD}													\
	}


#endif /* _GDISP_LLD_TOGGLE_BOARD_H */
