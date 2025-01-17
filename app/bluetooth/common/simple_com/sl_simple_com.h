/***************************************************************************//**
 * @file
 * @brief Simple Communication Interface (UART)
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#ifndef SL_SIMPLE_COM_H
#define SL_SIMPLE_COM_H

#include <stdint.h>
#include "sl_status.h"

/**************************************************************************//**
 * Simple Comm Init.
 *****************************************************************************/
void sl_simple_com_init(void);

/**************************************************************************//**
 * Uart transmit function
 *****************************************************************************/
void sl_simple_com_transmit(uint32_t len, uint8_t *data);

/**************************************************************************//**
 * Uart transmit completed callback
 *****************************************************************************/
void sl_simple_com_transmit_cb(sl_status_t status);

/**************************************************************************//**
 * Uart receive function
 *****************************************************************************/
void sl_simple_com_receive(void);

/**************************************************************************//**
 * Uart receive completed callback
 *****************************************************************************/
void sl_simple_com_receive_cb(sl_status_t status, uint32_t len, uint8_t *data);

#endif // SL_SIMPLE_COM_H
