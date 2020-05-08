/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2014 Liviu Ionescu.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

// ----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"
#include "STD_Types.h"
#include "util.h"
#include "RCC_int.h"
#include "DIO_int.h"
#include "NVIC_int.h"
#include "CAN.h"
#include "CANHANDLER_int.h"
#include "CANHANDLER_cfg.h"
#include "FLASH_int.h"
#include "SCB_int.h"
#include "HexDataProcessor_int.h"

// ----------------------------------------------------------------------------
//
// Standalone STM32F1 empty sample (trace via DEBUG).
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the DEBUG output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"


#define BANK0BASEADRESS	0x08002800
#define BANK1BASEADRESS	0x08014000

int
main(int argc, char* argv[])
{
	u8 HexArrayLine[261] = {0};
	u8 u8Counter  = 0;
	u16 u16Counter = 0;
	Error_Status err = OK;
	u32 address = 0;
	u8 HexData[8] = {0};
	volatile u8 test[48] = {0};
	u8 countertest = 0;
//	u8 u8Counter  = 0;
	u32 i = 0;
	u8 rxcount = 0;
	void (*pfunc)(void) = 0;
	u8 u8UsedBank = 0;
	u8 u8ProgrammingBank = 0;
	u32* pu32BaseAddress = 0;
	u32* pu32ProgrammingBaseAddresss = 0;
	volatile u8 u8ResetReason = 0;
//	pu32BaseAddress = 0x08010000;

	filter_type filters[] = {{CANHANDLER_u8HEXFILEID,DATA_FRAME, STANDARD_FORMAT}};
	RCC_vidInit();
	RCC_vidEnablePeripheral(RCC_u8GPIOCCLK);
	//	RCC_vidEnablePeripheral(RCC_u8GPIOBCLK);
	u8UsedBank = FLASH_u8GetOptionByteData(FLASH_u8OPTDATA0);
	switch (u8UsedBank)
	{
	case 0:
		u8ProgrammingBank = 1;
		pu32ProgrammingBaseAddresss = (u32*)BANK1BASEADRESS;
		pu32BaseAddress = (u32*)BANK0BASEADRESS;
		break;
	case 1:
		u8ProgrammingBank = 0;
		pu32ProgrammingBaseAddresss = (u32*)BANK0BASEADRESS;
		pu32BaseAddress = (u32*)BANK1BASEADRESS;
		break;
	default:
		u8ProgrammingBank = 0;
		pu32ProgrammingBaseAddresss = (u32*)BANK0BASEADRESS;
		pu32BaseAddress = (u32*)BANK1BASEADRESS;
		break;
	}

	u8ResetReason = RCC_u8GetResetFlag(RCC_u8SOFTRESET);

	if (u8ResetReason != 1)
	{
		if (FLASH_u32ReadWord(pu32BaseAddress) != 0xFFFFFFFF)
		{
			pfunc = *(u32*)(pu32BaseAddress + 1);
			pfunc();
		}
	}
	RCC_vidResetResetFlags();

	NVIC_vidInit();
	CAN_setup ();                                   // setup CAN interface
	CAN_vid_filter_list(filters,CANHANDLER_u8MAXFILTERNUMBERS);
	CAN_testmode(0);      // Normal, By Salma
	CAN_start ();                                   // leave init mode
	CAN_waitReady ();                               // wait til mbx is empty
	//	CANHANDLER_vidSend(35, DATA_FRAME, HexData,1);
	for ( u8Counter = 0; u8Counter < 59; u8Counter++)
	{
		FLASH_vidErasePage(u8Counter + 10 + 59*u8ProgrammingBank);
	}

	while (1)
	{
		// Add your code here.
		do
		{
			do
			{
				while (CAN_RxRdy == 0);
				if (CAN_RxRdy)
				{
					CAN_RxRdy = 0;
					if (CAN_RxMsg[rxcount].u8ActiveFlag == 1)
					{
						switch (CAN_RxMsg[rxcount].id)
						{
						case CANHANDLER_u8HEXFILEID:
							for (u8Counter = 0; u8Counter < CAN_RxMsg[rxcount].len ; u8Counter++ )
							{
								HexArrayLine[countertest] = CAN_RxMsg[rxcount].data[u8Counter];
								countertest++;
								CANHANDLER_vidSend(CANHANDLER_u8NEXTMSGREQUEST,CAN_u8REMOTEFRAME,(void*)0,0);
							}
							break;
						}
						CAN_RxMsg[rxcount].u8ActiveFlag = 0;
						rxcount++;
						if (rxcount == 3)
						{
							rxcount = 0;
						}
					}
				}
			}while ( HexArrayLine[countertest-1] != '\r');
			//				}while (u8Received != '\n');
			//			USART_voidSendChar(USART_CHANNEL_1,u8CharCount);
			//			u8CharCount  = 0;
			//				HexDataProcessor_vidGetHexData(HexArrayLine, &(strHexData[u8Counter]));
			//				SET_BIT(PORTA_BASEADDRESS->GPIO_ODR, 0);
			countertest = 0;
			err = HexDataProcessor_u32StoreHexInFlash(HexArrayLine);
			//				CLR_BIT(PORTA_BASEADDRESS->GPIO_ODR, 0);
//			u8Counter++;
			//				if (u8Counter != 55)
			//				{
			//					USART_voidSendChar(USART_CHANNEL_1,u8CharCount);
			//				}
			//			}while ( (u8Counter != 55) && (strHexData[u8Counter - 1].enuDataRecord != EndOfLine) );
			//			}while ( (u8Counter != 55) && (address == 0) );

			//			address = HexDataProcessor_u32StoreHexInFlash(strHexData,55);
			//			USART_voidSendChar(USART_CHANNEL_1,'a');
		} while (err != limitReached);
		pfunc = *(u32*)(pu32ProgrammingBaseAddresss + 1);

		FLASH_vidWriteOptionByteData(FLASH_u8OPTDATA0 , u8ProgrammingBank);
//		SCB_vidSetInterruptVectorTable(0x08010000);
//		__set_MSP(*(u32*)(0x08010000));
//		__asm__ volatile ("MSR msp, %0\n" : : "r" (*(u32*)(0x08010000)) : "sp");
//		SCB_vidPerformSoftReset();
		pfunc();

	}
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
