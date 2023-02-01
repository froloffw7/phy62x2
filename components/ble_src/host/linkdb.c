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

/*************************************************************************************************
    Filename:       linkdb.c
    Revised:
    Revision:

    Description:    This file contains the Link Database.

    These functions are not intended to be used outside of the host base.


**************************************************************************************************/


/*******************************************************************************
    INCLUDES
*/
#include "bcomdef.h"
#include "OSAL.h"
#include "linkdb.h"
#include "log.h"
#include "l2cap_internal.h"
// patch max gatt num conn
#include "sm.h"
#include "l2cap.h"
#include "l2cap_internal.h"
#include "sm_internal.h"
#include "gap_internal.h"
#include "att_internal.h"
#include "gatt_internal.h"

/*********************************************************************
    MACROS
*/

/*********************************************************************
    CONSTANTS
*/

/*********************************************************************
    TYPEDEFS
*/

/*********************************************************************
    GLOBAL VARIABLES
*/

/*********************************************************************
    EXTERNAL VARIABLES
*/
extern smPairingParams_t** pPairingParams;
extern l2capReassemblePkt_t* l2capReassemblePkt;
extern l2capSegmentBuff_t*   l2capSegmentPkt;
extern gattClientInfo_t* clientInfoTbl;
extern uint8 gattClientNum;
extern gattServerInfo_t* serverInfoTbl;
extern uint8 gattServerNum;
/*********************************************************************
    EXTERNAL FUNCTIONS
*/

/*********************************************************************
    LOCAL VARIABLES
*/

// This is the link database, 1 record for each connection
linkDBItem_t* linkDB=NULL;

// Table of callbacks to make when a connection changes state
pfnLinkDBCB_t* linkCBs=NULL;

// linkdb num
uint8 linkDB_Max_num = 0;
uint8 linkDB_MaxCBs_num = 0;
/*********************************************************************
    LOCAL FUNCTIONS
*/

static void reportStatusChange( uint16 connectionHandle, uint8 changeType );

/*********************************************************************
    FUNCTIONS
*/
uint8 linkDB_InitContext(uint8 max_link_num,linkDBItem_t* plinkDB,pfnLinkDBCB_t* plinkCBs)
{
    LOG("%s\n",__func__);

    if( ((max_link_num == 0) || ( max_link_num > MAX_NUM_LL_CONN_ROM_LIMT ) ) || \
            ( plinkDB == NULL ) || ( plinkCBs == NULL) )
        return bleInvalidRange;

    linkDB_Max_num = max_link_num;
    linkDB_MaxCBs_num = max_link_num + LINKDB_STACK_CALLBACK_NUM ;
    linkDB = plinkDB;
    linkCBs = plinkCBs;
    LOG("max_link_num %d,linkDB_MaxCBs_num %d\n",max_link_num,linkDB_MaxCBs_num);
    LOG("linkDB %p\n",linkDB);
    LOG("linkCBs %p\n\n",linkCBs);
    return SUCCESS;
}
uint8 Host_InitContext(         uint8 max_link_num,
                                linkDBItem_t* plinkDB,pfnLinkDBCB_t* plinkCBs,
                                smPairingParams_t** param,
                                uint16* mtu_size_buf,
                                gapAuthStateParams_t** Authen_link,
                                l2capReassemblePkt_t* ressembleBuf,l2capSegmentBuff_t* segmentBuf,
                                gattClientInfo_t* clientInfo,
                                gattServerInfo_t* serverInfo)
{
    LOG("%s\n",__func__);

    if( ((max_link_num == 0) || ( max_link_num > MAX_NUM_LL_CONN_ROM_LIMT ) ) || \
            ( plinkDB == NULL ) || ( plinkCBs == NULL) || (param == NULL) || \
            (Authen_link == NULL) || \
            (ressembleBuf == NULL) || (segmentBuf == NULL) || \
            (clientInfo == NULL ) || (serverInfo == NULL))
        return bleInvalidRange;

    linkDB_Max_num = max_link_num;
    linkDB_MaxCBs_num = max_link_num + LINKDB_STACK_CALLBACK_NUM ;
    linkDB = plinkDB;
    linkCBs = plinkCBs;
    pPairingParams = param;
    gAttMtuSize = mtu_size_buf;
    pAuthLink = Authen_link;
    l2capReassemblePkt = ressembleBuf;
    l2capSegmentPkt = segmentBuf;
    gattClientNum = max_link_num + 1 ;
    clientInfoTbl = clientInfo;
    gattServerNum = max_link_num + 1 ;
    serverInfoTbl = serverInfo;
    return SUCCESS;
}

/*********************************************************************
    @fn          linkDB_Init

    @brief       Initialize the Link Database.

    @param       none

    @return      none
*/
void linkDB_Init( void )
{
    uint8 x; // loop counter
    LOG("\n%s",__func__);

    // Initialize the table
    for ( x = 0; x < linkDB_Max_num; x++ )
    {
        LOG("addr linkDB[%d] = %p\n",x,&linkDB[x]);
        // Mark the record as unused.
        linkDB[x].connectionHandle = INVALID_CONNHANDLE;
        linkDB[x].stateFlags = LINK_NOT_CONNECTED;
        linkDB[x].pEncParams = NULL;
    }

    // Initialize the status callback registration table
    for ( x = 0; x < linkDB_MaxCBs_num; x++ )
    {
        LOG("addr linkCBs[%d] = %p\n",x,&linkCBs[x]);
        // No callbacks
        linkCBs[x] = (pfnLinkDBCB_t)NULL;
    }

    LOG("\n");
}

/*********************************************************************
    @fn          linkDB_Register

    @brief       Register with this function to receive a callback when
                status changes on a connection.  If the stateflag == 0,
                then the connection has been disconnected.

    @param       pFunc - function pointer to callback function

    @return      SUCCESS if successful
                bleMemAllocError if not table space available

*/
uint8 linkDB_Register( pfnLinkDBCB_t pFunc )
{
    LOG("%s\n",__func__);

    // Find an empty slot
    for ( uint8 x = 0; x < linkDB_MaxCBs_num; x++ )
    {
        LOG("linkCBs[%d] %p\n",x,linkCBs[x]);

        if ( linkCBs[x] == NULL )
        {
            linkCBs[x] = pFunc;
            LOG("linkCBs[%d] %p\n",x,linkCBs[x]);
            return ( SUCCESS );
        }
    }

    return ( bleMemAllocError );
}

/*********************************************************************
    @fn          linkDB_Add

    @brief       Adds a record to the link database.

    @param       taskID - Application task ID
    @param       connectionHandle - new record connection handle
    @param       newState - starting connection state
    @param       addrType - new address type
    @param       pAddr - new address
    @param       connInterval - connection's communications interval (n * 1.23 ms)

    @return      SUCCESS if successful
                bleIncorrectMode - hasn't been initialized.
                bleNoResources - table full
                bleAlreadyInRequestedMode - already exist connectionHandle

*/
uint8 linkDB_Add( uint8 taskID, uint16 connectionHandle, uint8  stateFlags,uint8 role,
                  uint8 addrType, uint8* pAddr, uint16 connInterval )
{
    // Check for existing record
    linkDBItem_t* pItem = linkDB_Find( connectionHandle );

    if ( pItem )
    {
        // Item already exists - connectionHandle was found
        return ( bleAlreadyInRequestedMode );
    }
    else
    {
        pItem = linkDB_Find( INVALID_CONNHANDLE );

        if ( pItem )
        {
            // Copy link info
            pItem->addrType = addrType;
            VOID osal_memcpy( pItem->addr, pAddr, B_ADDR_LEN );
            pItem->connectionHandle = connectionHandle;
            pItem->stateFlags = stateFlags;
            pItem->role = role;
            pItem->taskID = taskID;
            pItem->pEncParams = NULL;
            pItem->connInterval = connInterval;
            reportStatusChange( connectionHandle, LINKDB_STATUS_UPDATE_NEW );
            return ( SUCCESS );
        }
        else
        {
            // Table is full
            return ( bleNoResources );
        }
    }
}

/*********************************************************************
    @fn          linkDB_Remove

    @brief       Removes a record from the link database.

    @param       connectionHandle - new record connection handle

    @return      SUCCESS if successful
                INVALIDPARAMETER - connectionHandle not found.

*/
uint8 linkDB_Remove( uint16 connectionHandle )
{
    // Get record
    linkDBItem_t* pItem = linkDB_Find( connectionHandle );

    if ( pItem )
    {
        reportStatusChange( pItem->connectionHandle, LINKDB_STATUS_UPDATE_REMOVED );

        // Free memory for LTK
        if ( pItem->pEncParams )
        {
            osal_mem_free( pItem->pEncParams );
        }

        // Clear the entire entry
        VOID osal_memset( pItem, 0, (int)(sizeof( linkDBItem_t )) );
        // Mark the record as unused.
        pItem->connectionHandle = INVALID_CONNHANDLE;
        pItem->stateFlags = LINK_NOT_CONNECTED;
        return ( SUCCESS );
    }
    else
    {
        // Record not found
        return ( INVALIDPARAMETER );
    }
}


/*********************************************************************
    @fn          linkDB_Update

    @brief       This function is used to update the stateFlags of
                a link record.

    @param       connectionHandle - maximum number of connections.
    @param       newState - new state flag.  This value is OR'd in
                           to this field.

    @return      SUCCESS if successful
                bleNoResources - connectionHandle not found.

*/
uint8 linkDB_Update( uint16 connectionHandle, uint8 newState )
{
    linkDBItem_t* pItem = linkDB_Find( connectionHandle );

    // Find the right connection
    if ( pItem )
    {
        pItem->stateFlags |= newState;
        reportStatusChange( pItem->connectionHandle,
                            LINKDB_STATUS_UPDATE_STATEFLAGS );
        return ( SUCCESS );
    }
    else
    {
        return ( bleNoResources );
    }
}

/*********************************************************************
    @fn          linkDB_Find

    @brief       Find the link.  Uses the connection handle to search
                the link database.

    @param       connectionHandle - controller link connection handle.

    @return      a pointer to the found link item, NULL if not found
*/
linkDBItem_t* linkDB_Find( uint16 connectionHandle )
{
    LOG("%s\n",__func__);

    // Find link record
    for ( uint8 x = 0; x < linkDB_Max_num; x++ )
    {
        LOG("linkDB[%d] %p,linkDB[x].connectionHandle %d\n",x,&linkDB[x],linkDB[x].connectionHandle);

        if ( linkDB[x].connectionHandle == connectionHandle )
        {
            // Found
            return ( &linkDB[x] );
        }
    }

    // Not Found!!
    return ( (linkDBItem_t*)NULL );
}

/*********************************************************************
    @fn          linkDB_FindFirst

    @brief       Find the first link that matches the taskID.

    @param       taskID - taskID of app

    @return      a pointer to the found link item, NULL if not found
*/
linkDBItem_t* linkDB_FindFirst( uint8 taskID )
{
    // Find link record
    for ( uint8 x = 0; x < linkDB_Max_num; x++ )
    {
        if ( (linkDB[x].connectionHandle != INVALID_CONNHANDLE)
                && (linkDB[x].taskID == taskID) )
        {
            // Found
            return ( &linkDB[x] );
        }
    }

    // Not Found!!
    return ( (linkDBItem_t*)NULL );
}

/*********************************************************************
    @fn          linkDB_State

    @brief       Check to see if a physical link is in a specific state.

    @param       connectionHandle - controller link connection handle.
    @param       state - state to look for.

    @return      TRUE if the link is found and state is set in
                state flags. FALSE, otherwise.
*/
uint8 linkDB_State( uint16 connectionHandle, uint8 state )
{
    linkDBItem_t* pLink;

    // Check to see if this is loopback connection
    if ( connectionHandle == LOOPBACK_CONNHANDLE )
    {
        return ( TRUE );
    }

    // Check to see if the physical link is up
    pLink = linkDB_Find( connectionHandle );

    if ( ( pLink != NULL ) && ( pLink->stateFlags & state ) )
    {
        return ( TRUE );
    }

    return ( FALSE );
}

/*********************************************************************
    @fn          linkDB_NumActive

    @brief       Counts the number of active connections (not .

    @param       connectionHandle - controller link connection handle.

    @return      number of active connections
*/
uint8 linkDB_NumActive( void )
{
    uint8 count = 0;

    // Find link record
    for ( uint8 x = 0; x < linkDB_Max_num; x++ )
    {
        if ( linkDB[x].stateFlags )
        {
            count++;
        }
    }

    return ( count );
}

/*********************************************************************
    @fn          linkDB_Authen

    @brief       Check to see if the physical link is encrypted
                and authenticated.

    @param       connectionHandle - item's connection handle
    @param       keySize - minimum key size
    @param       mitmRequired - whether MITM protection is required

    @return      SUCCESS if link is authenticated.
                bleNotConnected - connection handle is invalid
                LINKDB_ERR_INSUFFICIENT_AUTHEN - link is not encrypted
                LINBDB_ERR_INSUFFICIENT_KEYSIZE - key size encrypted is not large enough
                LINKDB_ERR_INSUFFICIENT_ENCRYPTION - link is encrypted, but not authenticated
*/
uint8 linkDB_Authen( uint16 connectionHandle, uint8 keySize, uint8 mitmRequired )
{
    linkDBItem_t* pItem = linkDB_Find( connectionHandle );

    // Check that a connection exists
    if ( pItem == NULL )
    {
        // Check for loopback connection
        if ( connectionHandle == LOOPBACK_CONNHANDLE )
        {
            return ( SUCCESS );
        }
        else
        {
            return ( bleNotConnected );
        }
    }

    // If an LTK is not available, the service request shall be rejected with the
    // error code "Insufficient Authentication".

    // Note: When the link is not encrypted, the error code "Insufficient
    // Authentication" does not indicate that MITM protection is required.
    if ( (pItem->pEncParams == NULL) && ((pItem->stateFlags & LINK_ENCRYPTED) == 0) )
    {
        return ( LINKDB_ERR_INSUFFICIENT_AUTHEN );
    }

    // If an authenticated pairing is required but only an unauthenticated pairing
    // has occurred and the link is currently encrypted, the service request shall
    // be rejected with the error code "Insufficient Authentication."

    // Note: When unauthenticated pairing has occurred and the link is currently
    // encrypted, the error code "Insufficient Authentication" indicates that MITM
    // protection is required.
    if ( (pItem->pEncParams != NULL ) && (pItem->stateFlags & LINK_ENCRYPTED) &&
            ((pItem->stateFlags & LINK_AUTHENTICATED) == 0) && mitmRequired )
    {
        return ( LINKDB_ERR_INSUFFICIENT_AUTHEN );
    }

    // If an LTK is available and encryption is required (LE security mode 1) but
    // encryption is not enabled, the service request shall be rejected with the
    // error code �Insufficient Encryption?.
    if ( (pItem->stateFlags & LINK_ENCRYPTED) == 0 )
    {
        return ( LINKDB_ERR_INSUFFICIENT_ENCRYPTION );
    }

    // If the encryption is enabled with insufficient key size then the service
    // request shall be rejected with the error code �Insufficient Encryption Key
    // Size.?
    if ( pItem->pEncParams->keySize < keySize )
    {
        return ( LINBDB_ERR_INSUFFICIENT_KEYSIZE );
    }

    return ( SUCCESS );
}

/*********************************************************************
    @fn          linkDB_PerformFunc

    @brief       This function will call "cb" for every entry in the link database.

    @param       cb - function pointer to the function to call

    @return      none
*/
void linkDB_PerformFunc( pfnPerformFuncCB_t cb )
{
    if ( cb )
    {
        for ( uint8 x = 0; x < linkDB_Max_num; x++ )
        {
            if ( linkDB[x].connectionHandle != INVALID_CONNHANDLE )
            {
                cb( &linkDB[x] );
            }
        }
    }
}

/*********************************************************************
    @fn          reportStatusChange

    @brief       Call all callback functions with the state update.

    @param       connectionHandle - item's connection handle
    @param       changeType - Change Type:
                  LINKDB_STATUS_UPDATE_NEW - added to the database
                  LINKDB_STATUS_UPDATE_REMOVED - deleted from the database
                  LINKDB_STATUS_UPDATE_STATEFLAGS - stateFlag item has changed

    @return      void
*/
static void reportStatusChange( uint16 connectionHandle, uint8 changeType )
{
    for ( uint8 x = 0; x < linkDB_MaxCBs_num; x++ )
    {
        if ( linkCBs[x] )
        {
            linkCBs[x]( connectionHandle, changeType );
        }
    }
}


/****************************************************************************
****************************************************************************/

