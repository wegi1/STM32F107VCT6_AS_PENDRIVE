/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_storage_if.c
  * @version        : v2.0_Cube
  * @brief          : Memory management layer.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbd_storage_if.h"

/* USER CODE BEGIN INCLUDE */

/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device.
  * @{
  */

/** @defgroup USBD_STORAGE
  * @brief Usb mass storage device module
  * @{
  */

/** @defgroup USBD_STORAGE_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Private_Defines
  * @brief Private defines.
  * @{
  */

#define STORAGE_LUN_NBR                  1
#define STORAGE_BLK_NBR                  0x10000
#define STORAGE_BLK_SIZ                  0x200

/* USER CODE BEGIN PRIVATE_DEFINES */
#ifdef STORAGE_BLK_SIZ
#undef STORAGE_BLK_SIZ
#endif
#define STORAGE_BLK_SIZ                  2048

#ifdef STORAGE_BLK_NBR
#undef STORAGE_BLK_NBR
#endif
#define STORAGE_CAPPACITY                1024*240
#define STORAGE_BLK_NBR                  STORAGE_CAPPACITY / STORAGE_BLK_SIZ
#define MEDIA_DESCRIPTOR                 0xf8

#define STORAGE_ADDRESS                  0x08004000U


/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */
void STORAGE_FileSystem_Init (void);


const struct __attribute__ ((packed)) BPB_STRUCTURE
		{
	uint8_t   BS_jmpBoot[3] ;
	int8_t    BS_OEMName[8] ;
	uint16_t  BPB_BytsPerSec;
	uint8_t   BPB_SecPerClus;
	uint16_t  BPB_RsvdSecCnt;
	uint8_t   BPB_NumFATs;
	uint16_t  BPB_RootEntCnt;
	uint16_t  BPB_TotSec16;
	uint8_t   BPB_Media;
	uint16_t  BPB_FATSz16;
	uint16_t  BPB_SecPerTrk;
	uint16_t  BPB_NumHeads;
	uint32_t  BPB_HiddSec;
	uint32_t  BPB_TotSec32;
	uint8_t   BS_DrvNum;
	uint8_t   BS_Reserved1;
	uint8_t   BS_BootSig;
	uint32_t  BS_VolID;
	int8_t    BS_VolLab[11];
	int8_t    BS_FilSysType[8];
		} BIOS_Parameter_Block  = { \
				.BS_jmpBoot = {0xeb,0xfe,0x90}, \
				.BS_OEMName = {'M', 'S', 'D', 'O', 'S', '5', '.', '0'}, \
				.BPB_BytsPerSec = STORAGE_BLK_SIZ, \
				.BPB_SecPerClus = 1, \
				.BPB_RsvdSecCnt = 1, \
				.BPB_NumFATs = 1, \
				.BPB_RootEntCnt = STORAGE_BLK_SIZ / 0x20, \
				.BPB_TotSec16 = STORAGE_BLK_NBR, \
				.BPB_Media = MEDIA_DESCRIPTOR, \
				.BPB_FATSz16 = 1, \
				.BPB_SecPerTrk = 0x3f, \
				.BPB_NumHeads = 0xff, \
				.BPB_HiddSec = 0, \
				.BPB_TotSec32 = 0, \
				.BS_DrvNum = 0x80, \
				.BS_Reserved1 = 0, \
				.BS_BootSig = 0x29, \
				.BS_VolID = 0xcafecafe, \
				.BS_VolLab = {'S', 'T', 'M', '_', 'S', 'T', 'O', 'R', 'A', 'G', 'E'}, \
				.BS_FilSysType = {'F', 'A', 'T', '1', '2', ' ', ' ', ' '}, \
		};

const size_t Fat0_len = sizeof(BIOS_Parameter_Block);


/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Private_Variables
  * @brief Private variables.
  * @{
  */

/* USER CODE BEGIN INQUIRY_DATA_FS */
/** USB Mass storage Standard Inquiry Data. */
const int8_t STORAGE_Inquirydata_FS[] = {/* 36 */
  
  /* LUN 0 */
  0x00,
  0x80,
  0x02,
  0x02,
  (STANDARD_INQUIRY_DATA_LEN - 5),
  0x00,
  0x00,	
  0x00,
  'S', 'T', 'M', ' ', ' ', ' ', ' ', ' ', /* Manufacturer : 8 bytes */
  'P', 'r', 'o', 'd', 'u', 'c', 't', ' ', /* Product      : 16 Bytes */
  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  '0', '.', '0' ,'1'                      /* Version      : 4 Bytes */
}; 
/* USER CODE END INQUIRY_DATA_FS */

/* USER CODE BEGIN PRIVATE_VARIABLES */

/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t STORAGE_Init_FS(uint8_t lun);
static int8_t STORAGE_GetCapacity_FS(uint8_t lun, uint32_t *block_num, uint16_t *block_size);
static int8_t STORAGE_IsReady_FS(uint8_t lun);
static int8_t STORAGE_IsWriteProtected_FS(uint8_t lun);
static int8_t STORAGE_Read_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
static int8_t STORAGE_Write_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
static int8_t STORAGE_GetMaxLun_FS(void);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */
static void ERASE_One_Page(uint32_t page_address);
static void WRITE_One_Page(uint64_t *bufptr, uint32_t page_address);
static void STORAGE_FileSystem_Format (void);
/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

USBD_StorageTypeDef USBD_Storage_Interface_fops_FS =
{
  STORAGE_Init_FS,
  STORAGE_GetCapacity_FS,
  STORAGE_IsReady_FS,
  STORAGE_IsWriteProtected_FS,
  STORAGE_Read_FS,
  STORAGE_Write_FS,
  STORAGE_GetMaxLun_FS,
  (int8_t *)STORAGE_Inquirydata_FS
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes over USB FS IP
  * @param  lun:
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_Init_FS(uint8_t lun)
{
  /* USER CODE BEGIN 2 */

  return (USBD_OK);
  /* USER CODE END 2 */
}

/**
  * @brief  .
  * @param  lun: .
  * @param  block_num: .
  * @param  block_size: .
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_GetCapacity_FS(uint8_t lun, uint32_t *block_num, uint16_t *block_size)
{
  /* USER CODE BEGIN 3 */
  *block_num  = STORAGE_BLK_NBR;
  *block_size = STORAGE_BLK_SIZ;
  return (USBD_OK);
  /* USER CODE END 3 */
}

/**
  * @brief  .
  * @param  lun: .
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_IsReady_FS(uint8_t lun)
{
  /* USER CODE BEGIN 4 */
  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  .
  * @param  lun: .
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_IsWriteProtected_FS(uint8_t lun)
{
  /* USER CODE BEGIN 5 */
  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  .
  * @param  lun: .
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_Read_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
	memcpy(buf, (void *)(STORAGE_ADDRESS + STORAGE_BLK_SIZ * blk_addr), (STORAGE_BLK_SIZ * blk_len));

	return (USBD_OK);
}


/**
  * @brief  .
  * @param  lun: .
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_Write_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
	/* USER CODE BEGIN 7 */

    ERASE_One_Page((STORAGE_ADDRESS) + (STORAGE_BLK_SIZ * blk_addr));

    WRITE_One_Page(((uint64_t *)buf), ((STORAGE_ADDRESS)+(STORAGE_BLK_SIZ * blk_addr)));

	return (USBD_OK);
	/* USER CODE END 7 */
}

/**
  * @brief  .
  * @param  None
  * @retval .
  */
int8_t STORAGE_GetMaxLun_FS(void)
{
  /* USER CODE BEGIN 8 */
  return (STORAGE_LUN_NBR - 1);
  /* USER CODE END 8 */
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */
/**
  * @brief  ... Write one page of internal FLASH MEMORY from source buffer
  * @param  *bufptr: unsigned 64bit pointer to buffer with a source data
  * @param  page_address: unsigned 32bit value of address to destination write in FLASH MEMORY
  * @retval - none
  */
void WRITE_One_Page(uint64_t *bufptr, uint32_t page_address)
{
	for(uint32_t i = 0; i < STORAGE_BLK_SIZ ; i += 8)
	{
		HAL_FLASH_Program(FLASH_PROC_PROGRAMDOUBLEWORD , (page_address + i) , *bufptr); //send 8 bytes to write
		bufptr ++; // bufptr is 64bit value - incrementation onward 8 bytes
	}
}

/**
  * @brief  ... Erase one page of internal FLASH MEMORY
  * @param  page_address: unsigned 32bit value of address to destination ERASE in FLASH MEMORY
  * @retval - none
  */
void ERASE_One_Page(uint32_t page_address)
{
	FLASH_EraseInitTypeDef flashErase_handle; // declare structure to erase page
	uint32_t sectorError; // if check any errors after erase here you can...

	//memset(&flashErase_handle, 0, sizeof(flashErase_handle)); // don't need every one field this structure is initialize here
	//HAL_FLASH_Unlock(); //don't need it's on time unlocked in STORAGE_FileSystem_Init function

	flashErase_handle.TypeErase = FLASH_TYPEERASE_PAGES; // page type erase
	flashErase_handle.Banks = FLASH_BANK_1; //bank 1
	flashErase_handle.PageAddress = page_address; // address of page to erase
	flashErase_handle.NbPages = 1 ; // only one page to erase

	HAL_FLASHEx_Erase(&flashErase_handle, &sectorError); // Let's do ERASE ONE PAGE...

	//HAL_FLASH_Lock();
}


void STORAGE_FileSystem_Format (void)
{
	uint8_t RamBlock[STORAGE_BLK_SIZ];


    /*----------- FORMAT BOOT SECTOR -----------------*/
	memset(RamBlock, 0, sizeof RamBlock);
	memcpy((void *)(RamBlock), &BIOS_Parameter_Block , (Fat0_len));
	RamBlock[0x1fe] = 0x55;
	RamBlock[0x1ff] = 0xAA;
//---
    ERASE_One_Page((STORAGE_ADDRESS) + (STORAGE_BLK_SIZ * 0));
    WRITE_One_Page(((uint64_t *)RamBlock), ((STORAGE_ADDRESS)+(STORAGE_BLK_SIZ * 0)));

    /*----------- FORMAT FAT TABLE -----------------*/
	memset(RamBlock, 0, sizeof RamBlock);
	RamBlock[0x00] = MEDIA_DESCRIPTOR;
	RamBlock[0x01] = 0xff;
	RamBlock[0x02] = 0xff;
//---
    ERASE_One_Page((STORAGE_ADDRESS) + (STORAGE_BLK_SIZ * 1));
    WRITE_One_Page(((uint64_t *)RamBlock), ((STORAGE_ADDRESS)+(STORAGE_BLK_SIZ * 1)));

    /*----------- FORMAT DIR ENTRIES AND DISK NAME  -----------------*/
    memcpy((void *)(RamBlock), &BIOS_Parameter_Block.BS_VolLab , (sizeof(BIOS_Parameter_Block.BS_VolLab)));
    RamBlock[11] = 0x08; //disk name attribute
//---
    ERASE_One_Page((STORAGE_ADDRESS) + (STORAGE_BLK_SIZ * 2));
	WRITE_One_Page(((uint64_t *)RamBlock), ((STORAGE_ADDRESS)+(STORAGE_BLK_SIZ * 2)));
    /*----------- FORMAT END -----------------*/
}


void STORAGE_FileSystem_Init (void)
{

	uint32_t i;
	uint32_t format = 0;
	uint8_t *bufptr1 = (uint8_t *) STORAGE_ADDRESS;;
	uint8_t *bufptr2 = (uint8_t *) &BIOS_Parameter_Block;

	HAL_FLASH_Unlock();
	//if((memcmp((void *)STORAGE_ADDRESS, &BIOS_Parameter_Block , Fat0_len)) != 0 ){STORAGE_FileSystem_Format();}


	for(i = 0; i < Fat0_len ; i ++)
	{
		if(*(bufptr1+i) != *(bufptr2+i))
		{
			format = 1;
		}
	}

	if(format == 1) {STORAGE_FileSystem_Format();}
}
/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
