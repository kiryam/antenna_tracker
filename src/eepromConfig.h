#ifndef __EEPROMCONFIG_H
#define __EEPROMCONFIG_H


#define     FLASH_PAGE_SIZE                         0x400 // 1024byte
#define     _EEPROM_PAGES_TO_USE					2			//The number of pages needed to be used (excluding the transfer page)
#define		_EEPROM_START_FLASH_PAGE				125
#define		_EEPROM_TRANSFER_PAGE					127		    //Should be last page of memory - this is the temp page for updating other pages without using RAM

/*################################################################################################################################*/
/*######################DO NOT EDIT ANYTHING BELOW THIS LINE UNLESS YOU KNOW WHAT YOU ARE DOING!!!!!!#############################*/
/*################################################################################################################################*/
#if(_EEPROM_PAGES_TO_USE+_EEPROM_START_FLASH_PAGE>_EEPROM_TRANSFER_PAGE)
	#error  "Transfer Page is overlapping Data Pages"
#endif
#if(_EEPROM_PAGES_TO_USE+_EEPROM_START_FLASH_PAGE!=_EEPROM_TRANSFER_PAGE)
	#warning  "Page Gap between User Data and Transfer Page"
#endif

/* Emulated data and virtual address bits */
#define EE_DATA_16BIT         16
#define EE_DATA_32BIT         32
#define EE_DATA_WIDTH 		 (uint32_t)EE_DATA_32BIT

//  Valid Values: 0x400 , 0x800
#define		_EEPROM_FLASH_PAGE_SIZE  				(uint32_t)FLASH_PAGE_SIZE  /* Page size --> From HAL Library */

#define		_EEPROM_Num_VirtualAdd_IN_PAGE	((uint32_t)FLASH_PAGE_SIZE)/(EE_DATA_WIDTH/8)

//	Virtual Address points to uint32_t types of data so we divide it by 32bits=4byes
#define		_EEPROM_End_VirtualAddr	 (_EEPROM_FLASH_PAGE_SIZE/4)*_EEPROM_PAGES_TO_USE

/* Base address of the Flash sectors */
#define ADDR_FLASH_PAGE_0     ((uint32_t)0x08000000) /* Base @ of Page 0 */
#define _EEPROM_START_FLASH_PAGE_ADDRESS (uint32_t)(ADDR_FLASH_PAGE_0|(_EEPROM_FLASH_PAGE_SIZE*_EEPROM_START_FLASH_PAGE))
#define _EEPROM_TRANSFER_PAGE_ADDRESS    (uint32_t)(ADDR_FLASH_PAGE_0|(_EEPROM_FLASH_PAGE_SIZE*_EEPROM_TRANSFER_PAGE))

#endif
