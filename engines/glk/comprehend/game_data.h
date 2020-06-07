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

#ifndef GLK_COMPREHEND_GAME_DATA_H
#define GLK_COMPREHEND_GAME_DATA_H

#include "glk/comprehend/file_buf.h"
#include "common/serializer.h"
#include "common/str-array.h"

namespace Glk {
namespace Comprehend {

#define MAX_FLAGS 64
#define MAX_VARIABLES 128
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

class ComprehendGame;

enum {
	DIRECTION_NORTH,
	DIRECTION_SOUTH,
	DIRECTION_EAST,
	DIRECTION_WEST,
	DIRECTION_UP,
	DIRECTION_DOWN,
	DIRECTION_IN,
	DIRECTION_OUT,
	NR_DIRECTIONS
};

struct FunctionState {
	bool test_result;
	bool else_result;
	unsigned or_count;
	bool _and;
	bool in_command;
	bool executed;

	FunctionState() {
		clear();
	}

	void clear();
};

struct Room {
	uint8 direction[NR_DIRECTIONS];
	uint8 flags;
	uint8 graphic;
	uint16 string_desc;

	Room() {
		clear();
	}

	void clear();
};

struct Item {
	uint16 string_desc;
	uint16 long_string; /* Only used by version 2 */
	uint8 room;
	uint8 flags;
	uint8 word;
	uint8 graphic;

	Item() {
		clear();
	}

	void clear();

	void synchronize(Common::Serializer &s);
};

struct Word {
	char _word[7];
	uint8 _index;
	uint8 _type;

	Word() {
		clear();
	}

	void clear();

	void load(FileBuffer *fb);
};

struct WordIndex {
	uint8 index;
	uint8 type;

	WordIndex() {
		clear();
	}

	void clear() {
		index = type = 0;
	}
};

struct WordMap {
	/* <word[0]>, <word[1]> == <word[2]> */
	WordIndex word[3];
	uint8 flags;

	WordMap() {
		clear();
	}

	void clear();
};

struct Action {
	int type;
	size_t nr_words;
	// FIXME - use struct word_index here.
	uint8 word[4];
	uint8 word_type[4];
	uint16 function;

	Action() {
		clear();
	}

	void clear();
};

struct Instruction {
	uint8 opcode;
	size_t nr_operands;
	uint8 operand[3];
	bool is_command;

	Instruction() {
		clear();
	}

	void clear();
};

struct Function {
	Instruction instructions[0x100];
	size_t nr_instructions;

	Function() {
		clear();
	}

	void clear();
};

typedef Common::StringArray StringTable;

struct GameHeader {
	uint16 magic;

	uint16 room_desc_table;
	uint16 room_direction_table[NR_DIRECTIONS];
	uint16 room_flags_table;
	uint16 room_graphics_table;

	size_t nr_items;
	uint16 addr_item_locations;
	uint16 addr_item_flags;
	uint16 addr_item_word;
	uint16 addr_item_strings;
	uint16 addr_item_graphics;

	uint16 addr_dictionary;
	uint16 addr_word_map;

	uint16 addr_strings;
	uint16 addr_strings_end;

	uint16 addr_actions_vvnn;
	uint16 addr_actions_unknown;
	uint16 addr_actions_vnjn;
	uint16 addr_actions_vjn;
	uint16 addr_actions_vdn;
	uint16 addr_actions_vnn;
	uint16 addr_actions_vn;
	uint16 addr_actions_v;

	uint16 addr_vm; // FIXME - functions

	GameHeader() {
		clear();
	}

	void clear();
};

struct GameInfo {
	GameHeader _header;

	unsigned _comprehendVersion;

	Common::Array<Room> _rooms;
	uint8 _currentRoom;
	uint8 _startRoom;

	Common::Array<Item> _items;

	Word *_words;
	size_t _nr_words;

	Common::Array<WordMap> _wordMaps;
	Common::Array<Action> _actions;
	Common::Array<Function> _functions;
	Common::StringArray _replaceWords;

	StringTable _strings;
	StringTable _strings2;

	bool _flags[MAX_FLAGS];
	uint16 _variables[MAX_VARIABLES];

	uint8 _currentReplaceWord;
	uint _updateFlags;

	GameInfo() {
		clearInfo();
	}
	~GameInfo() {
		clearInfo();
	}

	void clearInfo();
};

struct StringFile {
	Common::String filename;
	uint32 base_offset;
	uint32 end_offset;

	StringFile() : base_offset(0), end_offset(0) {}
	StringFile(const Common::String &fname, uint32 baseOfs, uint32 endO = 0) : filename(fname), base_offset(baseOfs), end_offset(endO) {
	}
};

enum {
	OPCODE_UNKNOWN,
	OPCODE_TEST_FALSE,
	OPCODE_HAVE_OBJECT,
	OPCODE_OR,
	OPCODE_IN_ROOM,
	OPCODE_VAR_EQ,
	OPCODE_CURRENT_OBJECT_TAKEABLE,
	OPCODE_OBJECT_PRESENT,
	OPCODE_ELSE,
	OPCODE_OBJECT_IN_ROOM,
	OPCODE_OBJECT_NOT_VALID,
	OPCODE_INVENTORY_FULL,
	OPCODE_TEST_FLAG,
	OPCODE_CURRENT_OBJECT_IN_ROOM,
	OPCODE_HAVE_CURRENT_OBJECT,
	OPCODE_OBJECT_IS_NOT_NOWHERE,
	OPCODE_CURRENT_OBJECT_PRESENT,
	OPCODE_TEST_ROOM_FLAG,
	OPCODE_NOT_HAVE_OBJECT,
	OPCODE_NOT_IN_ROOM,
	OPCODE_CURRENT_OBJECT_IS_NOWHERE,
	OPCODE_OBJECT_NOT_PRESENT,
	OPCODE_OBJECT_NOT_IN_ROOM,
	OPCODE_TEST_NOT_FLAG,
	OPCODE_NOT_HAVE_CURRENT_OBJECT,
	OPCODE_OBJECT_IS_NOWHERE,
	OPCODE_CURRENT_OBJECT_NOT_PRESENT,
	OPCODE_CURRENT_OBJECT_NOT_TAKEABLE,
	OPCODE_TEST_NOT_ROOM_FLAG,
	OPCODE_INVENTORY,
	OPCODE_TAKE_OBJECT,
	OPCODE_MOVE_OBJECT_TO_ROOM,
	OPCODE_SAVE_ACTION,
	OPCODE_MOVE_TO_ROOM,
	OPCODE_VAR_ADD,
	OPCODE_SET_ROOM_DESCRIPTION,
	OPCODE_MOVE_OBJECT_TO_CURRENT_ROOM,
	OPCODE_VAR_SUB,
	OPCODE_SET_OBJECT_DESCRIPTION,
	OPCODE_SET_OBJECT_LONG_DESCRIPTION,
	OPCODE_MOVE,
	OPCODE_MOVE_DIRECTION,
	OPCODE_PRINT,
	OPCODE_REMOVE_OBJECT,
	OPCODE_SET_FLAG,
	OPCODE_CALL_FUNC,
	OPCODE_TURN_TICK,
	OPCODE_CLEAR_FLAG,
	OPCODE_INVENTORY_ROOM,
	OPCODE_TAKE_CURRENT_OBJECT,
	OPCODE_SPECIAL,
	OPCODE_DROP_OBJECT,
	OPCODE_DROP_CURRENT_OBJECT,
	OPCODE_SET_ROOM_GRAPHIC,
	OPCODE_SET_OBJECT_GRAPHIC,
	OPCODE_REMOVE_CURRENT_OBJECT,
	OPCODE_DO_VERB,
	OPCODE_VAR_INC,
	OPCODE_VAR_DEC,
	OPCODE_MOVE_CURRENT_OBJECT_TO_ROOM,
	OPCODE_DESCRIBE_CURRENT_OBJECT,
	OPCODE_SET_STRING_REPLACEMENT,
	OPCODE_SET_CURRENT_NOUN_STRING_REPLACEMENT,
	OPCODE_CURRENT_NOT_OBJECT,
	OPCODE_CURRENT_IS_OBJECT,
	OPCODE_DRAW_ROOM,
	OPCODE_DRAW_OBJECT,
	OPCODE_WAIT_KEY
};

/* Game state update flags */
#define UPDATE_GRAPHICS (1 << 0) /* Implies UPDATE_GRAPHICS_ITEMS */
#define UPDATE_GRAPHICS_ITEMS (1 << 1)
#define UPDATE_ROOM_DESC (1 << 2)
#define UPDATE_ITEM_LIST (1 << 3)
#define UPDATE_ALL (~0U)

/* Action types */
enum {
	ACTION_VERB_VERB_NOUN_NOUN,
	ACTION_VERB_NOUN_JOIN_NOUN,
	ACTION_VERB_JOIN_NOUN,
	ACTION_VERB_DIR_NOUN,
	ACTION_VERB_NOUN_NOUN,
	ACTION_VERB_NOUN,
	ACTION_VERB_OPT_NOUN
};

/* Standard strings (main string table) */
#define STRING_CANT_GO 0
#define STRING_DONT_UNDERSTAND 1
#define STRING_YOU_SEE 2
#define STRING_INVENTORY 3
#define STRING_INVENTORY_EMPTY 4
#define STRING_BEFORE_CONTINUE 5
#define STRING_SAVE_GAME 6
#define STRING_RESTORE_GAME 7

/* Special variables */
#define VAR_INVENTORY_WEIGHT 0
#define VAR_INVENTORY_LIMIT 1
#define VAR_TURN_COUNT 2

/* Special rooms */
#define ROOM_INVENTORY 0x00
#define ROOM_NOWHERE 0xff

/* Item flags */
#define ITEMF_WEIGHT_MASK (0x3)
#define ITEMF_CAN_TAKE (1 << 3)

/* Word types */
#define WORD_TYPE_VERB 0x01
#define WORD_TYPE_JOIN 0x02
#define WORD_TYPE_FEMALE 0x10
#define WORD_TYPE_MALE 0x20
#define WORD_TYPE_NOUN 0x40
#define WORD_TYPE_NOUN_PLURAL 0x80
#define WORD_TYPE_NOUN_MASK (WORD_TYPE_FEMALE | WORD_TYPE_MALE | \
                             WORD_TYPE_NOUN | WORD_TYPE_NOUN_PLURAL)

void comprehend_load_game(ComprehendGame *game);
void comprehend_restore_game(ComprehendGame *game,
                             const char *filename);
void comprehend_save_game(ComprehendGame *game, const char *filename);

} // namespace Comprehend
} // namespace Glk

#endif
