/***************************************************************************//**
 * @file
 * @brief File System - SCSI Media Driver
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc.  Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.
 * The software is governed by the sections of the MSLA applicable to Micrium
 * Software.
 *
 ******************************************************************************/

/****************************************************************************************************//**
 * @addtogroup FS_STORAGE
 * @{
 *******************************************************************************************************/

/********************************************************************************************************
 ********************************************************************************************************
 *                                               MODULE
 ********************************************************************************************************
 *******************************************************************************************************/

#ifndef  FS_SCSI_H_
#define  FS_SCSI_H_

/********************************************************************************************************
 ********************************************************************************************************
 *                                               INCLUDE FILES
 ********************************************************************************************************
 *******************************************************************************************************/

#include  <fs/include/fs_blk_dev.h>
#include  <common/include/rtos_types.h>

/********************************************************************************************************
 ********************************************************************************************************
 *                                               DEFINES
 ********************************************************************************************************
 *******************************************************************************************************/

#define  FS_SCSI_LU_QTY_UNLIMITED                           0u

/********************************************************************************************************
 *                                           SCSI COMMAND INQUIRY
 *******************************************************************************************************/

#define  FS_SCSI_CMD_INQUIRY_VID_FIELD_LEN                  8u
#define  FS_SCSI_CMD_INQUIRY_PID_FIELD_LEN                 16u
#define  FS_SCSI_CMD_INQUIRY_PROD_REV_LEVEL_FIELD_LEN       4u

/********************************************************************************************************
 ********************************************************************************************************
 *                                               DATA TYPES
 ********************************************************************************************************
 *******************************************************************************************************/

typedef struct fs_scsi_handle {
  FS_MEDIA_HANDLE MediaHandle;
} FS_SCSI_HANDLE;

/****************************************************************************************************//**
 *                                       LOGICAL UNIT DATA TYPE
 *
 * @note     (1) +1 accounts for the NULL character added to the string end.
 *******************************************************************************************************/

typedef struct fs_scsi_lu_info {
  CPU_BOOLEAN Removable;                                        ///< Flag indicating if LU is removable.
  FS_LB_SIZE  SecDfltSize;                                      ///< Default size of a sector.
  FS_LB_QTY   SecCnt;                                           ///< Nbr of sectors composing the device.
  CPU_CHAR    VendorID_StrTbl[FS_SCSI_CMD_INQUIRY_VID_FIELD_LEN + 1u]; // See Note #1.
  CPU_CHAR    ProductID_StrTbl[FS_SCSI_CMD_INQUIRY_PID_FIELD_LEN + 1u];
  CPU_CHAR    ProdRevLevelStrTbl[FS_SCSI_CMD_INQUIRY_PROD_REV_LEVEL_FIELD_LEN + 1u];
} FS_SCSI_LU_INFO;

/********************************************************************************************************
 ********************************************************************************************************
 *                                           FUNCTION PROTOTYPES
 ********************************************************************************************************
 *******************************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

FS_SCSI_HANDLE FS_SCSI_Open(FS_MEDIA_HANDLE media_handle,
                            RTOS_ERR        *p_err);

void FS_SCSI_Close(FS_SCSI_HANDLE scsi_handle,
                   RTOS_ERR       *p_err);

void FS_SCSI_LU_InfoGet(FS_SCSI_HANDLE  scsi_handle,
                        FS_SCSI_LU_INFO *p_lu_info,
                        RTOS_ERR        *p_err);

//                                                                 ------------ CMD BLK NOTIFICATION FNCT -------------
void FS_SCSI_LU_Conn(CPU_INT08U lun,
                     CPU_INT16U dev_id,
                     void       *p_transport_api,
                     void       *p_transport_arg);

void FS_SCSI_LU_Disconn(CPU_INT08U lun,
                        CPU_INT16U dev_id,
                        void       *p_transport_arg);

CPU_INT16U FS_SCSI_MaxRespBufLenGet(void);

#ifdef __cplusplus
}
#endif

/****************************************************************************************************//**
 ********************************************************************************************************
 * @}                                          MODULE END
 ********************************************************************************************************
 *******************************************************************************************************/

#endif