/**
    \file health_client.h

    \brief This file defines the Mesh Health Model Application Interface
    - includes Data Structures and Methods for Client.
*/

/*
    Copyright (C) 2017. Mindtree Ltd.
    All rights reserved.
*/

#ifndef _H_HEALTH_CLIENT_
#define _H_HEALTH_CLIENT_


/* --------------------------------------------- Header File Inclusion */
#include "MS_access_api.h"
#include "MS_health_client_api.h"


/* --------------------------------------------- Global Definitions */
#ifdef  HEALTH_CLIENT_NO_DEBUG
    #define HEALTH_CLIENT_ERR         EM_debug_null
#else /* HEALTH_CLIENT_NO_DEBUG */
    #ifdef VAR_ARG_IN_MACRO_NOT_SUPPORTED
        #define HEALTH_CLIENT_ERR
    #else
        #define HEALTH_CLIENT_ERR(...)     EM_debug_error(MS_MODULE_ID_MESH_MODEL, __VA_ARGS__)
    #endif /* VAR_ARG_IN_MACRO_NOT_SUPPORTED */
#endif /* HEALTH_CLIENT_NO_DEBUG */
#ifdef HEALTH_CLIENT_DEBUG
    #ifdef VAR_ARG_IN_MACRO_NOT_SUPPORTED
        #define HEALTH_CLIENT_TRC
        #define HEALTH_CLIENT_INF
        #define HEALTH_CLIENT_debug_dump_bytes(data, datalen)
    #else
        #define HEALTH_CLIENT_TRC(...)     EM_debug_trace(MS_MODULE_ID_MESH_MODEL,__VA_ARGS__)
        #define HEALTH_CLIENT_INF(...)     EM_debug_info(MS_MODULE_ID_MESH_MODEL,__VA_ARGS__)
        #define HEALTH_CLIENT_debug_dump_bytes(data, datalen) EM_debug_dump_bytes(MS_MODULE_ID_MESH_MODEL, (data), (datalen))
    #endif /* VAR_ARG_IN_MACRO_NOT_SUPPORTED */
#else /* HEALTH_CLIENT_DEBUG */
    #define HEALTH_CLIENT_TRC          EM_debug_null
    #define HEALTH_CLIENT_INF          EM_debug_null
    #define HEALTH_CLIENT_debug_dump_bytes(data, datalen)
#endif /* HEALTH_CLIENT_DEBUG */



/* --------------------------------------------- Data Types/ Structures */


/* --------------------------------------------- Function */
/**
    \brief Access Layer Application Asynchronous Notification Callback.

    \par Description
    Access Layer calls the registered callback to indicate events occurred to the application.

    \param [in] handle        Model Handle.
    \param [in] saddr         16 bit Source Address.
    \param [in] daddr         16 bit Destination Address.
    \param [in] appkey_handle AppKey Handle.
    \param [in] subnet_handle Subnet Handle.
    \param [in] opcode        Opcode.
    \param [in] data_param    Data associated with the event if any or NULL.
    \param [in] data_len      Size of the event data. 0 if event data is NULL.
*/
API_RESULT health_client_cb
(
    /* IN */ MS_ACCESS_MODEL_HANDLE* handle,
    /* IN */ MS_NET_ADDR              saddr,
    /* IN */ MS_NET_ADDR              daddr,
    /* IN */ MS_SUBNET_HANDLE         subnet_handle,
    /* IN */ MS_APPKEY_HANDLE         appkey_handle,
    /* IN */ UINT32                   opcode,
    /* IN */ UCHAR*                   data_param,
    /* IN */ UINT16                   data_len
);

#endif /*_H_HEALTH_CLIENT_ */
