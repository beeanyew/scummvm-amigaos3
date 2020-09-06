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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ULTIMA8_USECODE_USECODE_H
#define ULTIMA8_USECODE_USECODE_H

#include "ultima/shared/std/string.h"

namespace Ultima {
namespace Ultima8 {

class Usecode {
public:
	Usecode() { }
	virtual ~Usecode() { }

	virtual const uint8 *get_class(uint32 classid) = 0;
	virtual uint32 get_class_size(uint32 classid) = 0;
	virtual const char *get_class_name(uint32 classid) = 0;
	virtual uint32 get_class_base_offset(uint32 classid) = 0;
	virtual uint32 get_class_event_count(uint32 classid) = 0;

	virtual uint32 get_class_event(uint32 classid, uint32 eventid);
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif