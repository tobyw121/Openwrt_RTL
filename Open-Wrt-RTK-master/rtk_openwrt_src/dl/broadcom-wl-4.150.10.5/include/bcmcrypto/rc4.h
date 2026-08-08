/*
 * rc4.h
 * RC4 stream cipher
 *
 * Copyright 2007, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id$
 */

#ifndef _RC4_H_
#define _RC4_H_

#include <typedefs.h>

#define RC4_STATE_NBYTES 256

typedef struct rc4_ks {
	uchar state[RC4_STATE_NBYTES];
	uchar x;
	uchar y;
} rc4_ks_t;

void BCMROMFN(prepare_key)(uchar *key_data_ptr, int key_data_len, rc4_ks_t *key);

void BCMROMFN(rc4)(uchar *buffer_ptr, int buffer_len, rc4_ks_t *key);

#endif /* _RC4_H_ */
