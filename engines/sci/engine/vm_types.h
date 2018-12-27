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

#ifndef SCI_ENGINE_VM_TYPES_H
#define SCI_ENGINE_VM_TYPES_H

#include "common/scummsys.h"
#include "sci/sciversion.h"

namespace Sci {

// Segment ID type
typedef uint16 SegmentId;
#ifdef ENABLE_SCI32
typedef uint32 Offset;
#else
typedef uint16 Offset;
#endif

enum {
	kUninitializedSegment = 0x1FFF,
	kSegmentMask = 0x1FFF,
	kOffsetMask = 0x7FFFF
};

struct reg_t {
	// Segment and offset. These should never be accessed directly
	SegmentId _segment;
	uint16 _offset;

	SegmentId getSegment() const;

	void setSegment(SegmentId segment);

	Offset getOffset() const;
	void setOffset(Offset offset);

	inline void incOffset(Offset offset) {
		setOffset(getOffset() + offset);
	}

	inline bool isNull() const {
		return (getOffset() | getSegment()) == 0;
	}

	inline uint16 toUint16() const {
		return (uint16)getOffset();
	}

	inline int16 toSint16() const {
		return (int16)getOffset();
	}

	bool isNumber() const {
		return getSegment() == 0;
	}

	bool isPointer() const {
		return getSegment() != 0 && getSegment() != kUninitializedSegment;
	}

	uint16 requireUint16() const;
	int16 requireSint16() const;

	inline bool isInitialized() const {
		return getSegment() != kUninitializedSegment;
	}

	// Comparison operators
	bool operator==(const reg_t &x) const {
		return (getOffset() == x.getOffset()) && (getSegment() == x.getSegment());
	}

	bool operator!=(const reg_t &x) const {
		return (getOffset() != x.getOffset()) || (getSegment() != x.getSegment());
	}

	bool operator>(const reg_t &right) const {
		return cmp(right, false) > 0;
	}

	bool operator>=(const reg_t &right) const {
		return cmp(right, false) >= 0;
	}

	bool operator<(const reg_t &right) const {
		return cmp(right, false) < 0;
	}

	bool operator<=(const reg_t &right) const {
		return cmp(right, false) <= 0;
	}

	// Same as the normal operators, but perform unsigned
	// integer checking
	bool gtU(const reg_t &right) const {
		return cmp(right, true) > 0;
	}

	bool geU(const reg_t &right) const {
		return cmp(right, true) >= 0;
	}

	bool ltU(const reg_t &right) const {
		return cmp(right, true) < 0;
	}

	bool leU(const reg_t &right) const {
		return cmp(right, true) <= 0;
	}

	// Arithmetic operators
	reg_t operator+(const reg_t &right) const;
	reg_t operator-(const reg_t &right) const;
	reg_t operator*(const reg_t &right) const;
	reg_t operator/(const reg_t &right) const;
	reg_t operator%(const reg_t &right) const;
	reg_t operator>>(const reg_t &right) const;
	reg_t operator<<(const reg_t &right) const;

	reg_t operator+(int16 right) const;
	reg_t operator-(int16 right) const;

	void operator+=(const reg_t &right) { *this = *this + right; }
	void operator-=(const reg_t &right) { *this = *this - right; }
	void operator+=(int16 right) { *this = *this + right; }
	void operator-=(int16 right) { *this = *this - right; }

	// Boolean operators
	reg_t operator&(const reg_t &right) const;
	reg_t operator|(const reg_t &right) const;
	reg_t operator^(const reg_t &right) const;

#ifdef ENABLE_SCI32
	reg_t operator&(int16 right) const;
	reg_t operator|(int16 right) const;
	reg_t operator^(int16 right) const;

	void operator&=(const reg_t &right) { *this = *this & right; }
	void operator|=(const reg_t &right) { *this = *this | right; }
	void operator^=(const reg_t &right) { *this = *this ^ right; }
	void operator&=(int16 right) { *this = *this & right; }
	void operator|=(int16 right) { *this = *this | right; }
	void operator^=(int16 right) { *this = *this ^ right; }
#endif

private:
	/**
	 * Compares two reg_t's.
	 * Returns:
	 * - a positive number if *this > right
	 * - 0 if *this == right
	 * - a negative number if *this < right
	 */
	int cmp(const reg_t &right, bool treatAsUnsigned) const;
	reg_t lookForWorkaround(const reg_t &right, const char *operation) const;
	bool pointerComparisonWithInteger(const reg_t &right) const;

#ifdef ENABLE_SCI32
	int sci32Comparison(const reg_t right) const;
#endif
};

inline SegmentId reg_t::getSegment() const {
#ifdef ENABLE_SCI32
	if (getSciVersion() == SCI_VERSION_3) {
		// Return the lower 14 bits of the segment
		return (_segment & 0x3FFF);
	} else
#endif
	{
		return _segment;
	}
}

inline Offset reg_t::getOffset() const {
#ifdef ENABLE_SCI32
	if (getSciVersion() == SCI_VERSION_3) {
		// Return the lower 16 bits from the offset, and the 17th and 18th bits from the segment
		return ((_segment & 0xC000) << 2) | _offset;
	} else
#endif
	{
		return _offset;
	}
}

static inline reg_t make_reg(SegmentId segment, uint16 offset) {
	reg_t r;
	r.setSegment(segment);
	r.setOffset(offset);
	return r;
}

#define PRINT_REG(r) (kSegmentMask) & (unsigned) (r).getSegment(), (unsigned) (r).getOffset()

// A true 32-bit reg_t
struct reg32_t {
	// Segment and offset. These should never be accessed directly
	SegmentId _segment;
	uint32 _offset;

	inline SegmentId getSegment() const {
		return _segment;
	}

	inline void setSegment(SegmentId segment) {
		_segment = segment;
	}

	inline uint32 getOffset() const {
		return _offset;
	}

	inline void setOffset(uint32 offset) {
		_offset = offset;
	}

	inline void incOffset(int32 offset) {
		setOffset(getOffset() + offset);
	}

	// Comparison operators
	bool operator==(const reg32_t &x) const {
		return (getOffset() == x.getOffset()) && (getSegment() == x.getSegment());
	}

	bool operator!=(const reg32_t &x) const {
		return (getOffset() != x.getOffset()) || (getSegment() != x.getSegment());
	}
};

static inline reg32_t make_reg32(SegmentId segment, uint32 offset) {
	reg32_t r;
	r.setSegment(segment);
	r.setOffset(offset);
	return r;
}

// Stack pointer type
typedef reg_t *StackPtr;

enum {
	/**
	 * Special reg_t 'offset' used to indicate an error, or that an operation has
	 * finished (depending on the case).
	 * @see SIGNAL_REG
	 */
	SIGNAL_OFFSET = 0xffff
};

extern const reg_t NULL_REG;
extern const reg_t SIGNAL_REG;
extern const reg_t TRUE_REG;

// Selector ID
typedef int Selector;

enum {
	/** Special 'selector' value, used when calling add_exec_stack_entry. */
	NULL_SELECTOR = -1
};

// Opcode formats
enum opcode_format : byte {
	Script_Invalid = 0xFF,
	Script_None = 0,
	Script_Byte,
	Script_SByte,
	Script_Word,
	Script_SWord,
	Script_Variable,
	Script_SVariable,
	Script_SRelative,
	Script_Property,
	Script_Global,
	Script_Local,
	Script_Temp,
	Script_Param,
	Script_Offset,
	Script_End
};


} // End of namespace Sci

#endif // SCI_ENGINE_VM_TYPES_H
