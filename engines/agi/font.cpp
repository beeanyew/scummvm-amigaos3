/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2001 Sarien Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"

#include "agi/agi.h"

namespace Agi {

/* 8x8 font patterns */
uint8 cur_font[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x7E, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x7E,	/* cursor hollow */
	0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E,	/* cursor solid */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	/* cursor empty */
	0x10, 0x38, 0x7C, 0xFE, 0x7C, 0x38, 0x10, 0x00,
	0x3C, 0x3C, 0x18, 0xFF, 0xE7, 0x18, 0x3C, 0x00,
	0x10, 0x38, 0x7C, 0xFE, 0xEE, 0x10, 0x38, 0x00,
	0x00, 0x00, 0x18, 0x3C, 0x3C, 0x18, 0x00, 0x00,
	0xFF, 0xFF, 0xE7, 0xC3, 0xC3, 0xE7, 0xFF, 0xFF,
	0x00, 0x3C, 0x66, 0x42, 0x42, 0x66, 0x3C, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	/* \n */
	0x0F, 0x07, 0x0F, 0x7D, 0xCC, 0xCC, 0xCC, 0x78,
	0x3C, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x7E, 0x18,
	0x08, 0x0C, 0x0A, 0x0A, 0x08, 0x78, 0xF0, 0x00,
	0x18, 0x14, 0x1A, 0x16, 0x72, 0xE2, 0x0E, 0x1C,
	0x10, 0x54, 0x38, 0xEE, 0x38, 0x54, 0x10, 0x00,
	0x80, 0xE0, 0xF8, 0xFE, 0xF8, 0xE0, 0x80, 0x00,
	0x02, 0x0E, 0x3E, 0xFE, 0x3E, 0x0E, 0x02, 0x00,
	0x18, 0x3C, 0x5A, 0x18, 0x5A, 0x3C, 0x18, 0x00,
	0x66, 0x66, 0x66, 0x66, 0x66, 0x00, 0x66, 0x00,
	0x7F, 0xDB, 0xDB, 0xDB, 0x7B, 0x1B, 0x1B, 0x00,
	0x1C, 0x22, 0x38, 0x44, 0x44, 0x38, 0x88, 0x70,
	0x00, 0x00, 0x00, 0x00, 0x7E, 0x7E, 0x7E, 0x00,
	0x18, 0x3C, 0x5A, 0x18, 0x5A, 0x3C, 0x18, 0x7E,
	0x18, 0x3C, 0x5A, 0x18, 0x18, 0x18, 0x18, 0x00,
	0x18, 0x18, 0x18, 0x18, 0x5A, 0x3C, 0x18, 0x00,
	0x00, 0x18, 0x0C, 0xFE, 0x0C, 0x18, 0x00, 0x00,
	0x00, 0x30, 0x60, 0xFE, 0x60, 0x30, 0x00, 0x00,
	0x00, 0x00, 0xC0, 0xC0, 0xC0, 0xFE, 0x00, 0x00,
	0x00, 0x24, 0x42, 0xFF, 0x42, 0x24, 0x00, 0x00,
	0x00, 0x10, 0x38, 0x7C, 0xFE, 0xFE, 0x00, 0x00,
	0x00, 0xFE, 0xFE, 0x7C, 0x38, 0x10, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00,
	0x6C, 0x24, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x6C, 0x6C, 0xFE, 0x6C, 0xFE, 0x6C, 0x6C, 0x00,
	0x10, 0x7C, 0xD0, 0x7C, 0x16, 0xFC, 0x10, 0x00,
	0x00, 0x66, 0xAC, 0xD8, 0x36, 0x6A, 0xCC, 0x00,
	0x38, 0x4C, 0x38, 0x78, 0xCE, 0xCC, 0x7A, 0x00,
	0x30, 0x10, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x18, 0x30, 0x60, 0x60, 0x60, 0x30, 0x18, 0x00,
	0x60, 0x30, 0x18, 0x18, 0x18, 0x30, 0x60, 0x00,
	0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00,
	0x00, 0x30, 0x30, 0xFC, 0x30, 0x30, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x10, 0x20,
	0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00,
	0x02, 0x06, 0x0C, 0x18, 0x30, 0x60, 0xC0, 0x00,
	0x7C, 0xCE, 0xDE, 0xF6, 0xE6, 0xE6, 0x7C, 0x00,
	0x18, 0x38, 0x78, 0x18, 0x18, 0x18, 0x7E, 0x00,
	0x7C, 0xC6, 0x06, 0x1C, 0x70, 0xC6, 0xFE, 0x00,
	0x7C, 0xC6, 0x06, 0x3C, 0x06, 0xC6, 0x7C, 0x00,
	0x1C, 0x3C, 0x6C, 0xCC, 0xFE, 0x0C, 0x1E, 0x00,
	0xFE, 0xC0, 0xFC, 0x06, 0x06, 0xC6, 0x7C, 0x00,
	0x7C, 0xC6, 0xC0, 0xFC, 0xC6, 0xC6, 0x7C, 0x00,
	0xFE, 0xC6, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x00,
	0x7C, 0xC6, 0xC6, 0x7C, 0xC6, 0xC6, 0x7C, 0x00,
	0x7C, 0xC6, 0xC6, 0x7E, 0x06, 0xC6, 0x7C, 0x00,
	0x00, 0x30, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00,
	0x00, 0x30, 0x00, 0x00, 0x00, 0x30, 0x10, 0x20,
	0x0C, 0x18, 0x30, 0x60, 0x30, 0x18, 0x0C, 0x00,
	0x00, 0x00, 0x7E, 0x00, 0x00, 0x7E, 0x00, 0x00,
	0x60, 0x30, 0x18, 0x0C, 0x18, 0x30, 0x60, 0x00,
	0x78, 0xCC, 0x0C, 0x18, 0x30, 0x00, 0x30, 0x00,
	0x7C, 0x82, 0x9E, 0xA6, 0x9E, 0x80, 0x7C, 0x00,
	0x7C, 0xC6, 0xC6, 0xFE, 0xC6, 0xC6, 0xC6, 0x00,
	0xFC, 0x66, 0x66, 0x7C, 0x66, 0x66, 0xFC, 0x00,
	0x7C, 0xC6, 0xC0, 0xC0, 0xC0, 0xC6, 0x7C, 0x00,
	0xFC, 0x66, 0x66, 0x66, 0x66, 0x66, 0xFC, 0x00,
	0xFE, 0x62, 0x68, 0x78, 0x68, 0x62, 0xFE, 0x00,
	0xFE, 0x62, 0x68, 0x78, 0x68, 0x60, 0xF0, 0x00,
	0x7C, 0xC6, 0xC6, 0xC0, 0xCE, 0xC6, 0x7E, 0x00,
	0xC6, 0xC6, 0xC6, 0xFE, 0xC6, 0xC6, 0xC6, 0x00,
	0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00,
	0x1E, 0x0C, 0x0C, 0x0C, 0xCC, 0xCC, 0x78, 0x00,
	0xE6, 0x66, 0x6C, 0x78, 0x6C, 0x66, 0xE6, 0x00,
	0xF0, 0x60, 0x60, 0x60, 0x62, 0x66, 0xFE, 0x00,
	0x82, 0xC6, 0xEE, 0xFE, 0xD6, 0xC6, 0xC6, 0x00,
	0xC6, 0xE6, 0xF6, 0xDE, 0xCE, 0xC6, 0xC6, 0x00,
	0x7C, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0x7C, 0x00,
	0xFC, 0x66, 0x66, 0x7C, 0x60, 0x60, 0xF0, 0x00,
	0x7C, 0xC6, 0xC6, 0xC6, 0xD6, 0xDE, 0x7C, 0x06,
	0xFC, 0x66, 0x66, 0x7C, 0x66, 0x66, 0xE6, 0x00,
	0x7C, 0xC6, 0xC0, 0x7C, 0x06, 0xC6, 0x7C, 0x00,
	0x7E, 0x5A, 0x5A, 0x18, 0x18, 0x18, 0x3C, 0x00,
	0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0xC6, 0x7C, 0x00,
	0xC6, 0xC6, 0xC6, 0xC6, 0x6C, 0x38, 0x10, 0x00,
	0xC6, 0xC6, 0xD6, 0xFE, 0xEE, 0xC6, 0x82, 0x00,
	0xC6, 0x6C, 0x38, 0x38, 0x38, 0x6C, 0xC6, 0x00,
	0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x3C, 0x00,
	0xFE, 0xC6, 0x8C, 0x18, 0x32, 0x66, 0xFE, 0x00,
	0x78, 0x60, 0x60, 0x60, 0x60, 0x60, 0x78, 0x00,
	0xC0, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x02, 0x00,
	0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0x78, 0x00,
	0x10, 0x38, 0x6C, 0xC6, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
	0x30, 0x20, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x78, 0x0C, 0x7C, 0xCC, 0x76, 0x00,
	0xE0, 0x60, 0x60, 0x7C, 0x66, 0x66, 0x7C, 0x00,
	0x00, 0x00, 0x7C, 0xC6, 0xC0, 0xC6, 0x7C, 0x00,
	0x1C, 0x0C, 0x0C, 0x7C, 0xCC, 0xCC, 0x76, 0x00,
	0x00, 0x00, 0x7C, 0xC6, 0xFE, 0xC0, 0x7C, 0x00,
	0x1C, 0x36, 0x30, 0x78, 0x30, 0x30, 0x78, 0x00,
	0x00, 0x00, 0x76, 0xCC, 0xCC, 0x7C, 0x0C, 0x78,
	0xE0, 0x60, 0x6C, 0x76, 0x66, 0x66, 0xE6, 0x00,
	0x18, 0x00, 0x38, 0x18, 0x18, 0x18, 0x3C, 0x00,
	0x00, 0x0C, 0x00, 0x1C, 0x0C, 0x0C, 0xCC, 0x78,
	0xE0, 0x60, 0x66, 0x6C, 0x78, 0x6C, 0xE6, 0x00,
	0x38, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00,
	0x00, 0x00, 0xCC, 0xFE, 0xD6, 0xD6, 0xD6, 0x00,
	0x00, 0x00, 0xDC, 0x66, 0x66, 0x66, 0x66, 0x00,
	0x00, 0x00, 0x7C, 0xC6, 0xC6, 0xC6, 0x7C, 0x00,
	0x00, 0x00, 0xDC, 0x66, 0x66, 0x7C, 0x60, 0xF0,
	0x00, 0x00, 0x7C, 0xCC, 0xCC, 0x7C, 0x0C, 0x1E,
	0x00, 0x00, 0xDE, 0x76, 0x60, 0x60, 0xF0, 0x00,
	0x00, 0x00, 0x7C, 0xC0, 0x7C, 0x06, 0x7C, 0x00,
	0x10, 0x30, 0xFC, 0x30, 0x30, 0x34, 0x18, 0x00,
	0x00, 0x00, 0xCC, 0xCC, 0xCC, 0xCC, 0x76, 0x00,
	0x00, 0x00, 0xC6, 0xC6, 0x6C, 0x38, 0x10, 0x00,
	0x00, 0x00, 0xC6, 0xD6, 0xD6, 0xFE, 0x6C, 0x00,
	0x00, 0x00, 0xC6, 0x6C, 0x38, 0x6C, 0xC6, 0x00,
	0x00, 0x00, 0xCC, 0xCC, 0xCC, 0x7C, 0x0C, 0xF8,
	0x00, 0x00, 0xFC, 0x98, 0x30, 0x64, 0xFC, 0x00,
	0x0E, 0x18, 0x18, 0x30, 0x18, 0x18, 0x0E, 0x00,
	0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00,
	0xE0, 0x30, 0x30, 0x18, 0x30, 0x30, 0xE0, 0x00,
	0x76, 0xDC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/*0x00, 0x10, 0x38, 0x6C, 0xC6, 0xC6, 0xFE, 0x00, */
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,	/*replacement 0x7F */

	0x1E, 0x36, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x00,
	0x7C, 0x60, 0x60, 0x7C, 0x66, 0x66, 0x7C, 0x00,
	0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00,
	0x7E, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x00,
	0x38, 0x6C, 0x6C, 0x6C, 0x6C, 0x6C, 0xFE, 0xC6,
	0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x7E, 0x00,
	0xDB, 0xDB, 0x7E, 0x3C, 0x7E, 0xDB, 0xDB, 0x00,
	0x3C, 0x66, 0x06, 0x1C, 0x06, 0x66, 0x3C, 0x00,
	0x66, 0x66, 0x6E, 0x7E, 0x76, 0x66, 0x66, 0x00,
	0x3C, 0x66, 0x6E, 0x7E, 0x76, 0x66, 0x66, 0x00,
	0x66, 0x6C, 0x78, 0x70, 0x78, 0x6C, 0x66, 0x00,
	0x1E, 0x36, 0x66, 0x66, 0x66, 0x66, 0x66, 0x00,
	0xC6, 0xEE, 0xFE, 0xFE, 0xD6, 0xC6, 0xC6, 0x00,
	0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00,
	0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00,
	0x7E, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x00,
	0x7C, 0x66, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x00,
	0x3C, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3C, 0x00,
	0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00,
	0x66, 0x66, 0x66, 0x3E, 0x06, 0x66, 0x3C, 0x00,
	0x7E, 0xDB, 0xDB, 0xDB, 0x7E, 0x18, 0x18, 0x00,
	0x66, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x66, 0x00,
	0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x7F, 0x03,
	0x66, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x06, 0x00,
	0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xFF, 0x00,
	0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xDB, 0xFF, 0x03,
	0xE0, 0x60, 0x60, 0x7C, 0x66, 0x66, 0x7C, 0x00,
	0xC6, 0xC6, 0xC6, 0xF6, 0xDE, 0xDE, 0xF6, 0x00,
	0x60, 0x60, 0x60, 0x7C, 0x66, 0x66, 0x7C, 0x00,
	0x78, 0x8C, 0x06, 0x3E, 0x06, 0x8C, 0x78, 0x00,
	0xCE, 0xDB, 0xDB, 0xFB, 0xDB, 0xDB, 0xCE, 0x00,
	0x3E, 0x66, 0x66, 0x66, 0x3E, 0x36, 0x66, 0x00,
	0x00, 0x00, 0x3C, 0x06, 0x3E, 0x66, 0x3A, 0x00,
	0x00, 0x3C, 0x60, 0x3C, 0x66, 0x66, 0x3C, 0x00,
	0x00, 0x00, 0x7C, 0x66, 0x7C, 0x66, 0x7C, 0x00,
	0x00, 0x00, 0x7E, 0x60, 0x60, 0x60, 0x60, 0x00,
	0x00, 0x00, 0x3C, 0x6C, 0x6C, 0x6C, 0xFE, 0xC6,
	0x00, 0x00, 0x3C, 0x66, 0x7E, 0x60, 0x3C, 0x00,
	0x00, 0x00, 0xDB, 0x7E, 0x3C, 0x7E, 0xDB, 0x00,
	0x00, 0x00, 0x3C, 0x66, 0x0C, 0x66, 0x3C, 0x00,
	0x00, 0x00, 0x66, 0x6E, 0x7E, 0x76, 0x66, 0x00,
	0x00, 0x18, 0x66, 0x6E, 0x7E, 0x76, 0x66, 0x00,
	0x00, 0x00, 0x66, 0x6C, 0x78, 0x6C, 0x66, 0x00,
	0x00, 0x00, 0x1E, 0x36, 0x66, 0x66, 0x66, 0x00,
	0x00, 0x00, 0xC6, 0xFE, 0xFE, 0xD6, 0xC6, 0x00,
	0x00, 0x00, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x00,
	0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x3C, 0x00,
	0x00, 0x00, 0x7E, 0x66, 0x66, 0x66, 0x66, 0x00,
	0x11, 0x44, 0x11, 0x44, 0x11, 0x44, 0x11, 0x44,
	0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA,
	0xDD, 0x77, 0xDD, 0x77, 0xDD, 0x77, 0xDD, 0x77,
	0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
	0x18, 0x18, 0x18, 0xF8, 0x18, 0x18, 0x18, 0x18,
	0x18, 0xF8, 0x18, 0xF8, 0x18, 0x18, 0x18, 0x18,
	0x36, 0x36, 0x36, 0xF6, 0x36, 0x36, 0x36, 0x36,
	0x00, 0x00, 0x00, 0xFE, 0x36, 0x36, 0x36, 0x36,
	0x00, 0xF8, 0x18, 0xF8, 0x18, 0x18, 0x18, 0x18,
	0x36, 0xF6, 0x06, 0xF6, 0x36, 0x36, 0x36, 0x36,
	0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36,
	0x00, 0xFE, 0x06, 0xF6, 0x36, 0x36, 0x36, 0x36,
	0x36, 0xF6, 0x06, 0xFE, 0x00, 0x00, 0x00, 0x00,
	0x36, 0x36, 0x36, 0xFE, 0x00, 0x00, 0x00, 0x00,
	0x18, 0xF8, 0x18, 0xF8, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xF8, 0x18, 0x18, 0x18, 0x18,
	0x18, 0x18, 0x18, 0x1F, 0x00, 0x00, 0x00, 0x00,
	0x18, 0x18, 0x18, 0xFF, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xFF, 0x18, 0x18, 0x18, 0x18,
	0x18, 0x18, 0x18, 0x1F, 0x18, 0x18, 0x18, 0x18,
	0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00,
	0x18, 0x18, 0x18, 0xFF, 0x18, 0x18, 0x18, 0x18,
	0x18, 0x1F, 0x18, 0x1F, 0x18, 0x18, 0x18, 0x18,
	0x36, 0x36, 0x36, 0x37, 0x36, 0x36, 0x36, 0x36,
	0x36, 0x37, 0x30, 0x3F, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x3F, 0x30, 0x37, 0x36, 0x36, 0x36, 0x36,
	0x36, 0xF7, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xFF, 0x00, 0xF7, 0x36, 0x36, 0x36, 0x36,
	0x36, 0x37, 0x30, 0x37, 0x36, 0x36, 0x36, 0x36,
	0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00,
	0x36, 0xF7, 0x00, 0xF7, 0x36, 0x36, 0x36, 0x36,
	0x18, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00,
	0x36, 0x36, 0x36, 0xFF, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xFF, 0x00, 0xFF, 0x18, 0x18, 0x18, 0x18,
	0x00, 0x00, 0x00, 0xFF, 0x36, 0x36, 0x36, 0x36,
	0x36, 0x36, 0x36, 0x3F, 0x00, 0x00, 0x00, 0x00,
	0x18, 0x1F, 0x18, 0x1F, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x1F, 0x18, 0x1F, 0x18, 0x18, 0x18, 0x18,
	0x00, 0x00, 0x00, 0x3F, 0x36, 0x36, 0x36, 0x36,
	0x36, 0x36, 0x36, 0xFF, 0x36, 0x36, 0x36, 0x36,
	0x18, 0xFF, 0x18, 0xFF, 0x18, 0x18, 0x18, 0x18,
	0x18, 0x18, 0x18, 0xF8, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x1F, 0x18, 0x18, 0x18, 0x18,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0,
	0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
	0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x7C, 0x66, 0x66, 0x7C, 0x60, 0x00,
	0x00, 0x00, 0x3C, 0x66, 0x60, 0x66, 0x3C, 0x00,
	0x00, 0x00, 0x7E, 0x18, 0x18, 0x18, 0x18, 0x00,
	0x00, 0x00, 0x66, 0x66, 0x3E, 0x06, 0x7C, 0x00,
	0x00, 0x00, 0x7E, 0xDB, 0xDB, 0x7E, 0x18, 0x00,
	0x00, 0x00, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x00,
	0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x7F, 0x03,
	0x00, 0x00, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x00,
	0x00, 0x00, 0xDB, 0xDB, 0xDB, 0xDB, 0xFF, 0x00,
	0x00, 0x00, 0xDB, 0xDB, 0xDB, 0xDB, 0xFF, 0x03,
	0x00, 0x00, 0xE0, 0x60, 0x7C, 0x66, 0x7C, 0x00,
	0x00, 0x00, 0xC6, 0xC6, 0xF6, 0xDE, 0xF6, 0x00,
	0x00, 0x00, 0x60, 0x60, 0x7C, 0x66, 0x7C, 0x00,
	0x00, 0x00, 0x7C, 0x06, 0x3E, 0x06, 0x7C, 0x00,
	0x00, 0x00, 0xCE, 0xDB, 0xFB, 0xDB, 0xCE, 0x00,
	0x00, 0x00, 0x3E, 0x66, 0x3E, 0x36, 0x66, 0x00,
	0x00, 0x00, 0xFE, 0x00, 0xFE, 0x00, 0xFE, 0x00,
	0x10, 0x10, 0x7C, 0x10, 0x10, 0x00, 0x7C, 0x00,
	0x00, 0x30, 0x18, 0x0C, 0x06, 0x0C, 0x18, 0x30,
	0x00, 0x0C, 0x18, 0x30, 0x60, 0x30, 0x18, 0x0C,
	0x0E, 0x1B, 0x1B, 0x18, 0x18, 0x18, 0x18, 0x18,
	0x18, 0x18, 0x18, 0x18, 0x18, 0xD8, 0xD8, 0x70,
	0x00, 0x18, 0x18, 0x00, 0x7E, 0x00, 0x18, 0x18,
	0x00, 0x76, 0xDC, 0x00, 0x76, 0xDC, 0x00, 0x00,
	0x00, 0x38, 0x6C, 0x6C, 0x38, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x38, 0x38, 0x00, 0x00, 0x00,
	0x03, 0x02, 0x06, 0x04, 0xCC, 0x68, 0x38, 0x10,
	0x3C, 0x42, 0x99, 0xA1, 0xA1, 0x99, 0x42, 0x3C,
	0x30, 0x48, 0x10, 0x20, 0x78, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x7C, 0x7C, 0x7C, 0x7C, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x7E, 0x00
};

}                             // End of namespace Agi
