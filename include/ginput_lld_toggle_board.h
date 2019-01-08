#pragma once
#include "stm32f10x.h"

#define GINPUT_TOGGLE_NUM_PORTS			2			// The total number of toggle inputs
#define GINPUT_TOGGLE_CONFIG_ENTRIES	1			// The total number of GToggleConfig entries

#define GINPUT_TOGGLE_DECLARE_STRUCTURE()											\
const GToggleConfig GInputToggleConfigTable[GINPUT_TOGGLE_CONFIG_ENTRIES] = {	\
		{GPIOA,								\
			GPIO_Pin_1 | GPIO_Pin_4,											\
			GPIO_Pin_1 | GPIO_Pin_4,											\
			GPIO_Mode_IPU}													\
	}

