/*
 * iwinfo - Wireless Information Library - QCAWifi Headers
 *
 *   Copyright (c) 2013 The Linux Foundation. All rights reserved.
 *   Copyright (C) 2009 Jo-Philipp Wich <xm@subsignal.org>
 *
 * The iwinfo library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * The iwinfo library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with the iwinfo library. If not, see http://www.gnu.org/licenses/.
 *
 * This file is based on: src/include/iwinfo/madwifi.h
 */

#ifndef __IWINFO_QCAWIFI_H_
#define __IWINFO_QCAWIFI_H_

#include <fcntl.h>

/* The driver is using only one "_" character in front of endianness macros
 * whereas the uClibc is using "__" */
#include <endian.h>
#if __BYTE_ORDER == __BIG_ENDIAN
#define _BYTE_ORDER _BIG_ENDIAN
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define _BYTE_ORDER _LITTLE_ENDIAN
#else
#error "__BYTE_ORDER undefined"
#endif

#include "iwinfo.h"
#include "iwinfo/utils.h"
#include "ieee80211_external.h"

#endif
