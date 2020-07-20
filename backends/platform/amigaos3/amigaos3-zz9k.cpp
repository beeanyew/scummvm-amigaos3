/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#include "amigaos3-zz9k.h"
#include <stdio.h>
#include <proto/expansion.h>
#include <proto/dos.h>
#include <proto/exec.h>

#define ZZWRITE16(a, b) *(volatile short*)((unsigned int)zz9k_addr+a) = (volatile short)b;

// These are offsets in the "scratch area" in the memory on the ZZ9000 board.
// They are located after the main framebuffer/video RAM and are guaranteed to not be in use.
unsigned int zz9k_offsets[ZZ9K_OFFSET_NUM] = {
    0x3300000,
    0x3400000,
    0x3500000,
    0x3600000,
};

static unsigned int zz9k_addr;
static unsigned int zz9k_gfxdata = Z3_GFXDATA_ADDR;
unsigned char zz9k_palette[768];

unsigned int find_zz9k() {
	struct Library *expansionBase = NULL;

	if ((expansionBase = OpenLibrary("expansion.library",0L)) == NULL) {
	    printf("Failed to open expansion.library, could not get ZZ9000 board address.\n");
  	}
	else {
		struct ConfigDev* cd = NULL;
		cd = (struct ConfigDev*)FindConfigDev(cd,0x6d6e,0x4);
		if (cd != NULL) {
			zz9k_addr = (unsigned int)cd->cd_BoardAddr;
            zz9k_gfxdata += zz9k_addr;
			printf("Found ZZ9000 board at address $%X\n", (unsigned int)cd->cd_BoardAddr);
		}
		else {
			printf("No ZZ9000 board found.\n");
		}
        CloseLibrary(expansionBase);
	}

    return zz9k_addr;
}

// ZZ9000 register offsets, will always* be backward compatible.
#define REG_ZZ_DMA_OP 0x5A
#define REG_ZZ_ACC_OP 0x5C

#define ZZWRITE16(a, b) *(volatile short*)((unsigned int)zz9k_addr+a) = (volatile short)b;

void zz9k_clearbuf(unsigned int addr, unsigned int col, unsigned short w, unsigned short h, unsigned char color_format) {
    struct zz9k_GFXData* gfxdata = (struct zz9k_GFXData*)((uint32)zz9k_gfxdata);

    gfxdata->offset[0] = addr;
    gfxdata->x[0] = w; gfxdata->y[0] = h;
    gfxdata->pitch[0] = w;
    gfxdata->rgb[0] = col;
    gfxdata->u8_user[GFXDATA_U8_COLORMODE] = color_format;

    //printf("zz9k clearbuf to %p: @%.8X %dx%d - Size: %d\n", gfxdata, addr, w, h, sizeof(struct zz9k_GFXData));
    ZZWRITE16(REG_ZZ_ACC_OP, ACC_OP_BUFFER_CLEAR);
}

void zz9k_flip_surface(unsigned int src, unsigned int dest, unsigned short w, unsigned short h) {
    struct zz9k_GFXData* gfxdata = (struct zz9k_GFXData*)((uint32)zz9k_gfxdata);

    gfxdata->offset[0] = src;
    gfxdata->offset[1] = dest;
    gfxdata->x[0] = w; gfxdata->y[0] = h;
    gfxdata->pitch[0] = w;
    gfxdata->u8_user[GFXDATA_U8_COLORMODE] = MNTVA_COLOR_8BIT;

    ZZWRITE16(REG_ZZ_ACC_OP, ACC_OP_BUFFER_FLIP);
}

void zz9k_set_clut_mouse_cursor(short hot_x, short hot_y, unsigned short w, unsigned short h, const void *bmp, unsigned int key_color) {
    struct zz9k_GFXData* gfxdata = (struct zz9k_GFXData*)((uint32)zz9k_gfxdata);

    gfxdata->offset[1] = Z3_GFXDATA_ADDR + 0x10000;
    memcpy((uint8_t *)(uint32_t)(zz9k_gfxdata + 0x10000), bmp, w * h);
    memcpy(gfxdata->clut1, zz9k_palette, 768);
    gfxdata->x[0] = -(hot_x+1); gfxdata->y[0] = -hot_y;
    gfxdata->x[1] = w; gfxdata->y[1] = h;
    gfxdata->u8offset = (uint8_t)key_color;

    ZZWRITE16(REG_ZZ_DMA_OP, OP_SPRITE_CLUT_BITMAP);
}

void zz9k_blit_rect(unsigned int src, unsigned int dest, int x, int y, int src_pitch, int dest_pitch, int w, int h) {
    struct zz9k_GFXData* gfxdata = (struct zz9k_GFXData*)((uint32)zz9k_gfxdata);
    
    gfxdata->offset[0] = src;
    gfxdata->offset[1] = dest;
    gfxdata->pitch[0] = (unsigned short)src_pitch;
    gfxdata->pitch[1] = (unsigned short)dest_pitch;
    gfxdata->x[0] = (unsigned short)x; gfxdata->y[0] = (unsigned short)y;
    gfxdata->x[1] = (unsigned short)w; gfxdata->y[1] = (unsigned short)h;

    ZZWRITE16(REG_ZZ_ACC_OP, ACC_OP_BLIT_RECT);
}

unsigned int zz9k_get_surface_offset(int idx) {
    return zz9k_offsets[idx];
}
