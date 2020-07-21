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

// A pointer to the permanent GFXData struct in memory on the ZZ9000 board,
// used for sharing data between the Amiga side and the RTG board.
#define Z3_GFXDATA_ADDR 0x3200000
#define Z3_SCRATCH_ADDR 0x3210000
#define SURFACE_OFFSET(a) ((unsigned int)a.getPixels() & 0x0FFFFFFF)

#define MNTVA_COLOR_8BIT     0
#define MNTVA_COLOR_16BIT565 1
#define MNTVA_COLOR_32BIT    2
#define MNTVA_COLOR_15BIT    3

#define uint32 unsigned int
#define uint16 unsigned short
#define uint8 unsigned char

unsigned int find_zz9k();
unsigned int zz9k_get_surface_offset(int idx);

void zz9k_clearbuf(unsigned int addr, unsigned int col, unsigned short w, unsigned short h, unsigned char color_format);
void zz9k_flip_surface(unsigned int src, unsigned int dest, unsigned short w, unsigned short h);
void zz9k_set_clut_mouse_cursor(short hot_x, short hot_y, unsigned short w, unsigned short h, const void *bmp, unsigned int key_color);
void zz9k_blit_rect(unsigned int src, unsigned int dest, int x, int y, int src_pitch, int dest_pitch, int w, int h, unsigned char src_bpp = 1, unsigned char dest_bpp = 1, bool reverse = false);
void zz9k_blit_rect_mask(unsigned int src, unsigned int dest, int x, int y, int src_pitch, int dest_pitch, int w, int h, unsigned char mask_color, unsigned char src_bpp = 1, unsigned char dest_bpp = 1);
void zz9k_set_16_to_8_colormap(void *src);

unsigned int zz9k_alloc_surface(unsigned short w, unsigned short h, unsigned char bpp = 0);
void zz9k_free_surface(unsigned int p_);

enum gfx_dma_op {
  OP_NONE,
  OP_DRAWLINE,
  OP_FILLRECT,
  OP_COPYRECT,
  OP_COPYRECT_NOMASK,
  OP_RECT_TEMPLATE,
  OP_RECT_PATTERN,
  OP_P2C,
  OP_P2D,
  OP_INVERTRECT,
  OP_PAN,
  OP_SPRITE_XY,
  OP_SPRITE_COLOR,
  OP_SPRITE_BITMAP,
  OP_SPRITE_CLUT_BITMAP,
  OP_NUM,
};

enum gfx_acc_op {
  ACC_OP_NONE,
  ACC_OP_BUFFER_FLIP,
  ACC_OP_BUFFER_CLEAR,
  ACC_OP_BLIT_RECT,
  ACC_OP_ALLOC_SURFACE,
  ACC_OP_FREE_SURFACE,
  ACC_OP_SET_BPP_CONVERSION_TABLE,
  ACC_OP_NUM,
};

enum gfxdata_offsets {
  GFXDATA_DST,
  GFXDATA_SRC,
};

enum gfxdata_u8_types {
  GFXDATA_U8_COLORMODE,
  GFXDATA_U8_DRAWMODE,
  GFXDATA_U8_LINE_PATTERN_OFFSET,
  GFXDATA_U8_LINE_PADDING,
};

// GFXData struct has a maximum size of 64KB
#pragma pack(4)
struct zz9k_GFXData {
  uint32 offset[2];
  uint32 rgb[2];
  uint16 x[4], y[4];
  uint16 user[4];
  uint16 pitch[4];
  uint8 u8_user[8];
  uint8 op, mask, minterm, u8offset;
  uint8 clut1[768];
  uint8 clut2[768];
  uint8 clut3[768];
  uint8 clut4[768];
};

enum ZZ9K_SURFACE_OFFSETS {
    ZZ9K_OFFSET_GAME_SCREEN,
    ZZ9K_OFFSET_TMP_SCREEN,
    ZZ9K_OFFSET_OVERLAY,
    ZZ9K_OFFSET_OVERLAY16,
    ZZ9K_OFFSET_NUM,
};
