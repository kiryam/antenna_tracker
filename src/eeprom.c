#include "eeprom.h"
#include "eepromConfig.h"
#include "stm32f10x.h"

//##########################################################################################################
#define _X_BYTE_ROUNDUP(Dividend,Divisor)	((Dividend/Divisor)+(Dividend%Divisor==0?0:1))
//##########################################################################################################
//copy page to transfer page excluding the bad data
static bool 		EE_PageTransfer(uint32_t PageAddress,uint16_t BadDataOffset,uint16_t BadDataNum);
static uint32_t EE_FindPageAddressOfVirtualAddress(uint16_t VirtualAddress);
static uint16_t EE_FindVirtualAddressOffset(uint16_t VirtualAddress);
//##########################################################################################################
bool	EE_FormatPage(uint32_t PageAddress) {
	FLASH_Unlock();
	if(FLASH_ErasePage(PageAddress) == FLASH_COMPLETE) {
		FLASH_Lock();
		return true;
	}
	FLASH_Lock();
	return false;
}
//##########################################################################################################
bool EE_Read(uint16_t VirtualAddress, uint32_t* Data)
{
	if(VirtualAddress >=	_EEPROM_End_VirtualAddr)
		return false;
	*Data =  (*(__IO uint32_t*)((VirtualAddress*4)+_EEPROM_START_FLASH_PAGE_ADDRESS));
	return true;
}
//##########################################################################################################
bool EE_Reads(uint16_t StartVirtualAddress,uint16_t HowMuchToRead,uint32_t* Data)
{
	if((StartVirtualAddress+HowMuchToRead) >	_EEPROM_End_VirtualAddr)
		return false;
	for(uint16_t	i=StartVirtualAddress ; i<HowMuchToRead+StartVirtualAddress ; i++)
	{
		*Data =  (*(__IO uint32_t*)((i*4)+_EEPROM_START_FLASH_PAGE_ADDRESS));
		Data++;
	}
	return true;
}
//##########################################################################################################
bool EE_ReadBytes(uint16_t StartVirtualAddress,uint16_t HowManyBytesToRead,void* Data)
{
	union{uint8_t sbyte[4];uint32_t fbyte;}temp;
	uint8_t *DataP = (uint8_t *)Data;

	//check address validation
	if((StartVirtualAddress + _X_BYTE_ROUNDUP(HowManyBytesToRead,4)) >	_EEPROM_End_VirtualAddr)
		return false;

	for(uint16_t	i=StartVirtualAddress , currentbyte=0 ; i < _X_BYTE_ROUNDUP(HowManyBytesToRead,4)+StartVirtualAddress ; i++)
	{
		temp.fbyte =  (*(__IO uint32_t*)((i*4)+_EEPROM_START_FLASH_PAGE_ADDRESS));
		// Fill bytes
		for(uint8_t j=0;j<4;j++)
		{
			if (currentbyte<HowManyBytesToRead)
			{
				*DataP = temp.sbyte[j];
				++currentbyte;
				if (currentbyte<HowManyBytesToRead)
					DataP++;
			}
			else
				break;
		}
	}
	return true;
}
//##########################################################################################################
bool EE_Write(uint16_t VirtualAddress, uint32_t Data)
{
	if(VirtualAddress >=	_EEPROM_End_VirtualAddr)
		return false;

	//If flash space is not empty make it empty
	if((*(__IO uint32_t*)((VirtualAddress*4)+_EEPROM_START_FLASH_PAGE_ADDRESS)) != 0xFFFFFFFF)
	{
		//create backup of that page excluding bad data
		EE_PageTransfer(EE_FindPageAddressOfVirtualAddress(VirtualAddress),EE_FindVirtualAddressOffset(VirtualAddress),1);
	}
	//Write data
	if(Data!=0xFFFFFFFF)
	{
		FLASH_Unlock();

		if (FLASH_ProgramWord((VirtualAddress*4)+_EEPROM_START_FLASH_PAGE_ADDRESS, Data) == FLASH_COMPLETE)
		{
			FLASH_Lock();
			return true;
		}
		else
		{
			FLASH_Lock();
			return false;
		}
	}
	FLASH_Lock();
	return true;

}
//##########################################################################################################
bool 	EE_Writes(uint16_t StartVirtualAddress,uint16_t HowMuchToWrite,uint32_t* Data)
{
	//check address validation
	if((StartVirtualAddress+HowMuchToWrite) >	_EEPROM_End_VirtualAddr)
		return false;

	//If flash space is not empty make it empty
	for(uint16_t i=StartVirtualAddress ; i<StartVirtualAddress + HowMuchToWrite ; i++)
	{
		if((*(__IO uint32_t*)((i*4)+_EEPROM_START_FLASH_PAGE_ADDRESS)) != 0xFFFFFFFF)
		{
			//create backup of that page excluding bad data
			EE_PageTransfer(EE_FindPageAddressOfVirtualAddress(StartVirtualAddress),EE_FindVirtualAddressOffset(StartVirtualAddress),HowMuchToWrite);
			break;
		}
	}

	//Write new data
	FLASH_Unlock();
	for(uint16_t i=StartVirtualAddress ; i<StartVirtualAddress + HowMuchToWrite ; i++, Data++)
	{
		if (FLASH_ProgramWord((i*4)+_EEPROM_START_FLASH_PAGE_ADDRESS, *Data) != FLASH_COMPLETE)
		{
			FLASH_Lock();
			return false;
		}
	}

	HAL_FLASH_Lock();
	return true;
}
//##########################################################################################################
bool 	EE_WriteBytes(uint16_t StartVirtualAddress,uint16_t HowManyBytesToWrite,void* Data)
{
	union{uint8_t sbyte[4];uint32_t fbyte;}temp;
	uint8_t *DataP = (uint8_t *)Data;

	//check address validation
	if((StartVirtualAddress+_X_BYTE_ROUNDUP(HowManyBytesToWrite,4)) >	_EEPROM_End_VirtualAddr)
		return false;

	//If flash space is not empty make it empty
	for(uint16_t i=StartVirtualAddress ; i<StartVirtualAddress + _X_BYTE_ROUNDUP(HowManyBytesToWrite,4) ; i++)
	{
		if((*(__IO uint32_t*)((i*4)+_EEPROM_START_FLASH_PAGE_ADDRESS)) != 0xFFFFFFFF)
		{
			//create backup of that page excluding bad data
			EE_PageTransfer(EE_FindPageAddressOfVirtualAddress(StartVirtualAddress),EE_FindVirtualAddressOffset(StartVirtualAddress),_X_BYTE_ROUNDUP(HowManyBytesToWrite,4));
			break;
		}
	}

	//Write new data
	HAL_FLASH_Unlock();
	for(uint16_t i=StartVirtualAddress, currentbyte=0 ; i<StartVirtualAddress + _X_BYTE_ROUNDUP(HowManyBytesToWrite,4) ; i++)
	{
		//Convert Bytes to uint32_t and pad the end if more than indicated size
		for(uint8_t j=0;j<4;j++)
		{
			if (currentbyte<HowManyBytesToWrite)
			{
				temp.sbyte[j] = *DataP;
				++currentbyte;
				if (currentbyte<HowManyBytesToWrite)
					DataP++;
			}
			else
				temp.sbyte[j] = 0;
		}
		// program
		if (FLASH_ProgramWord((i*4)+_EEPROM_START_FLASH_PAGE_ADDRESS, temp.fbyte) != FLASH_COMPLETE)
		{
			FLASH_Lock();
			return false;
		}
	}

	HAL_FLASH_Lock();
	return true;
}
//##########################################################################################################
static bool EE_PageTransfer(uint32_t PageAddress,uint16_t BadDataOffset,uint16_t BadDataNum)
{
	uint32_t tempdata;
	//format transfer page
	if(EE_FormatPage(_EEPROM_TRANSFER_PAGE_ADDRESS)==false)
		return false;

	//Write to transfer page but exclude the bad data
	FLASH_Unlock();
	for(uint16_t	i=0 ; i<_EEPROM_FLASH_PAGE_SIZE/4 ; i++)
	{
		if(i>=BadDataOffset && i<BadDataOffset+BadDataNum)
		{
			continue;
		}
		//read original data
		tempdata =  (*(__IO uint32_t*)((i*4)+PageAddress));
		//write to transfer page
		if (FLASH_ProgramWord((i*4)+_EEPROM_TRANSFER_PAGE_ADDRESS, tempdata) != FLASH_COMPLETE)
		{
			FLASH_Lock();
			return false;
		}
	}

	//format original page
	if(EE_FormatPage(PageAddress)==false)
		return false;

	FLASH_Unlock();
	//Write back to Original Page
	for(uint16_t	i=0 ; i<_EEPROM_FLASH_PAGE_SIZE/4 ; i++)
	{
		tempdata =  (*(__IO uint32_t*)((i*4)+_EEPROM_TRANSFER_PAGE_ADDRESS));
		if (FLASH_ProgramWord((i*4)+PageAddress, tempdata) != FLASH_COMPLETE)
		{
			FLASH_Lock();
			return false;
		}
	}

	FLASH_Lock();
	return true;
}
//##########################################################################################################
static uint32_t EE_FindPageAddressOfVirtualAddress(uint16_t VirtualAddress)
{
	uint32_t temp;
	temp = _EEPROM_Num_VirtualAdd_IN_PAGE;
	temp = _EEPROM_START_FLASH_PAGE_ADDRESS + (VirtualAddress/temp)*((uint32_t)FLASH_PAGE_SIZE);
	return temp;
}
//##########################################################################################################
static uint16_t EE_FindVirtualAddressOffset(uint16_t VirtualAddress)
{
	uint16_t temp;
	temp = _EEPROM_Num_VirtualAdd_IN_PAGE;
	temp = (VirtualAddress % temp);
	return temp;
}
//##########################################################################################################
