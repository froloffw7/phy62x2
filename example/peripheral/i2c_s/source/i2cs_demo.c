/**************************************************************************************************
 
  Phyplus Microelectronics Limited confidential and proprietary. 
  All rights reserved.

  IMPORTANT: All rights of this software belong to Phyplus Microelectronics 
  Limited ("Phyplus"). Your use of this Software is limited to those 
  specific rights granted under  the terms of the business contract, the 
  confidential agreement, the non-disclosure agreement and any other forms 
  of agreements as a customer or a partner of Phyplus. You may not use this 
  Software unless you agree to abide by the terms of these agreements. 
  You acknowledge that the Software may not be modified, copied, 
  distributed or disclosed unless embedded on a Phyplus Bluetooth Low Energy 
  (BLE) integrated circuit, either as a product or is integrated into your 
  products.  Other than for the aforementioned purposes, you may not use, 
  reproduce, copy, prepare derivative works of, modify, distribute, perform, 
  display or sell this Software and/or its documentation for any purposes.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED AS IS WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  PHYPLUS OR ITS SUBSIDIARIES BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
  
**************************************************************************************************/

/**************************************************************************************************
  Filename:      
  Revised:       
  Revision:       


**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "rom_sym_def.h"
#include "OSAL.h"
#include "i2c_demo.h"
#include "log.h"
#include "clock.h"
#include "i2c_s.h"
#include "pwrmgr.h"
#include "error.h"

//#define LOG(...)                    {log_printf(__VA_ARGS__);}

uint8 application_TaskID;
void i2cs_Init(uint8 task_id)
{		
	application_TaskID = task_id;
	i2cs_initial();
	LOG("i2c_demo\n");
	osal_start_reload_timer(application_TaskID, OSAL_RELOAY_TIMER_EVT, 1500);
}

extern void i2cs_test(void);

uint16 i2cs_ProcessEvent( uint8 task_id, uint16 events )
{	
	if(task_id != application_TaskID){
		return 0;
	}

	if ( events & OSAL_RELOAY_TIMER_EVT )
	{
		i2cs_test();
		return ( events ^ OSAL_RELOAY_TIMER_EVT );
	}
	
  return 0;
}