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

#include "glk/comprehend/debugger_dumper.h"
#include "glk/comprehend/dictionary.h"
#include "glk/comprehend/game.h"

namespace Glk {
namespace Comprehend {

DebuggerDumper::DebuggerDumper() : _game(nullptr) {
	_opcodes[OPCODE_UNKNOWN] = "unknown";

	_opcodes[OPCODE_HAVE_OBJECT] = "have_object";
	_opcodes[OPCODE_NOT_HAVE_OBJECT] = "not_have_object";
	_opcodes[OPCODE_HAVE_CURRENT_OBJECT] = "have_current_object";
	_opcodes[OPCODE_NOT_HAVE_CURRENT_OBJECT] = "not_have_current_object";

	_opcodes[OPCODE_OBJECT_IS_NOT_NOWHERE] = "object_is_not_nowhere";

	_opcodes[OPCODE_CURRENT_OBJECT_TAKEABLE] = "current_object_takeable";
	_opcodes[OPCODE_CURRENT_OBJECT_NOT_TAKEABLE] = "current_object_not_takeable";

	_opcodes[OPCODE_CURRENT_OBJECT_IS_NOWHERE] = "current_object_is_nowhere";

	_opcodes[OPCODE_CURRENT_OBJECT_NOT_PRESENT] = "current_object_not_present";

	_opcodes[OPCODE_TAKE_OBJECT] = "take_object";
	_opcodes[OPCODE_TAKE_CURRENT_OBJECT] = "take_current_object";
	_opcodes[OPCODE_DROP_OBJECT] = "drop_object";
	_opcodes[OPCODE_DROP_CURRENT_OBJECT] = "drop_current_object";

	_opcodes[OPCODE_OR] = "or";
	_opcodes[OPCODE_IN_ROOM] = "in_room";
	_opcodes[OPCODE_VAR_EQ] = "var_eq";
	_opcodes[OPCODE_OBJECT_NOT_VALID] = "object_not_valid";
	_opcodes[OPCODE_INVENTORY_FULL] = "inventory_full";
	_opcodes[OPCODE_OBJECT_PRESENT] = "object_present";
	_opcodes[OPCODE_ELSE] = "else";
	_opcodes[OPCODE_OBJECT_IN_ROOM] = "object_in_room";
	_opcodes[OPCODE_TEST_FLAG] = "test_flag";
	_opcodes[OPCODE_CURRENT_OBJECT_IN_ROOM] = "current_object_in_room";
	_opcodes[OPCODE_CURRENT_OBJECT_PRESENT] = "current_object_present";
	_opcodes[OPCODE_TEST_ROOM_FLAG] = "test_room_flag";
	_opcodes[OPCODE_NOT_IN_ROOM] = "not_in_room";
	_opcodes[OPCODE_OBJECT_NOT_PRESENT] = "object_not_present";
	_opcodes[OPCODE_OBJECT_NOT_IN_ROOM] = "object_not_in_room";
	_opcodes[OPCODE_TEST_NOT_FLAG] = "test_not_flag";
	_opcodes[OPCODE_OBJECT_IS_NOWHERE] = "object_is_nowhere";
	_opcodes[OPCODE_TEST_NOT_ROOM_FLAG] = "test_not_room_flag";
	_opcodes[OPCODE_INVENTORY] = "inventory";
	_opcodes[OPCODE_MOVE_OBJECT_TO_ROOM] = "move_object_to_room";
	_opcodes[OPCODE_SAVE_ACTION] = "save_action";
	_opcodes[OPCODE_MOVE_TO_ROOM] = "move_to_room";
	_opcodes[OPCODE_VAR_ADD] = "var_add";
	_opcodes[OPCODE_SET_ROOM_DESCRIPTION] = "set_room_description";
	_opcodes[OPCODE_MOVE_OBJECT_TO_CURRENT_ROOM] = "move_object_to_current_room";
	_opcodes[OPCODE_VAR_SUB] = "var_sub";
	_opcodes[OPCODE_SET_OBJECT_DESCRIPTION] = "set_object_description";
	_opcodes[OPCODE_SET_OBJECT_LONG_DESCRIPTION] = "set_object_long_description";
	_opcodes[OPCODE_MOVE] = "move";
	_opcodes[OPCODE_PRINT] = "print";
	_opcodes[OPCODE_REMOVE_OBJECT] = "remove_object";
	_opcodes[OPCODE_SET_FLAG] = "set_flag";
	_opcodes[OPCODE_CALL_FUNC] = "call_func";
	_opcodes[OPCODE_TURN_TICK] = "turn_tick";
	_opcodes[OPCODE_CLEAR_FLAG] = "clear_flag";
	_opcodes[OPCODE_INVENTORY_ROOM] = "inventory_room";
	_opcodes[OPCODE_SPECIAL] = "special";
	_opcodes[OPCODE_SET_ROOM_GRAPHIC] = "set_room_graphic";
	_opcodes[OPCODE_SET_OBJECT_GRAPHIC] = "set_object_graphic";
	_opcodes[OPCODE_REMOVE_CURRENT_OBJECT] = "remove_current_object";
	_opcodes[OPCODE_DO_VERB] = "do_verb";
	_opcodes[OPCODE_VAR_INC] = "var_inc";
	_opcodes[OPCODE_VAR_DEC] = "var_dec";
	_opcodes[OPCODE_MOVE_CURRENT_OBJECT_TO_ROOM] = "move_current_object_to_room";
	_opcodes[OPCODE_DESCRIBE_CURRENT_OBJECT] = "describe_current_object";
	_opcodes[OPCODE_SET_STRING_REPLACEMENT] = "set_string_replacement";
	_opcodes[OPCODE_SET_CURRENT_NOUN_STRING_REPLACEMENT] = "set_current_noun_string_replacement";
	_opcodes[OPCODE_CURRENT_NOT_OBJECT] = "current_not_object";
	_opcodes[OPCODE_CURRENT_IS_OBJECT] = "current_is_object";
	_opcodes[OPCODE_DRAW_ROOM] = "draw_room";
	_opcodes[OPCODE_DRAW_OBJECT] = "draw_object";
	_opcodes[OPCODE_WAIT_KEY] = "wait_key";
}

Common::String DebuggerDumper::dumpInstruction(ComprehendGame *game,
        FunctionState *func_state, Instruction *instr) {
	uint i;
	int str_index, str_table;
	uint8 *opcode_map, opcode;
	Common::String line;

	if (func_state)
		line = Common::String::format("[or=%d,and=%d,test=%d,else=%d]",
		                              func_state->or_count, func_state->_and,
		                              func_state->test_result, func_state->else_result);

	opcode_map = game->_opcodeMap;
	opcode = opcode_map[instr->opcode];

	line += Common::String::format("  [%.2x] ", instr->opcode);
	if (_opcodes.contains(opcode))
		line += _opcodes[opcode];
	else
		line += "unknown";

	if (instr->nr_operands) {
		line += "(";
		for (i = 0; i < instr->nr_operands; i++)
			line += Common::String::format("%.2x%s",
			                               instr->operand[i],
			                               i == (instr->nr_operands - 1) ? ")" : ", ");
	}

	switch (opcode) {
	case OPCODE_PRINT:
	case OPCODE_SET_ROOM_DESCRIPTION:
	case OPCODE_SET_OBJECT_DESCRIPTION:
	case OPCODE_SET_OBJECT_LONG_DESCRIPTION:

		if (opcode == OPCODE_PRINT) {
			str_index = instr->operand[0];
			str_table = instr->operand[1];
		} else {
			str_index = instr->operand[1];
			str_table = instr->operand[2];
		}

		line += Common::String::format(" %s", game->instrStringLookup(str_index, str_table).c_str());
		break;

	case OPCODE_SET_STRING_REPLACEMENT:
		line += Common::String::format(" %s", game->_replaceWords[instr->operand[0] - 1].c_str());
		break;
	}

	line += "\n";
	return line;
}

void DebuggerDumper::dumpFunctions() {
	Function *func;
	uint i, j;

	print("Functions (%u entries)\n", _game->_functions.size());
	for (i = 0; i < _game->_functions.size(); i++) {
		func = &_game->_functions[i];

		print("[%.4x] (%u instructions)\n", i, (uint)func->nr_instructions);
		for (j = 0; j < func->nr_instructions; j++) {
			Common::String line = dumpInstruction(_game, NULL, &func->instructions[j]);
			print("%s", line.c_str());
		}
		print("\n");
	}
}

void DebuggerDumper::dumpActionTable() {
	Action *action;
	Word *word;
	uint i, j;

	print("Action table (%u entries)\n", _game->_actions.size());
	for (i = 0; i < _game->_actions.size(); i++) {
		action = &_game->_actions[i];

		print("(");
		for (j = 0; j < 4; j++) {
			if (j < action->nr_words) {
				switch (action->word_type[j]) {
				case WORD_TYPE_VERB:
					print("v");
					break;
				case WORD_TYPE_JOIN:
					print("j");
					break;
				case WORD_TYPE_NOUN_MASK:
					print("n");
					break;
				default:
					print("?");
					break;
				}
			} else {
				print(" ");
			}
		}

		print(") [%.4x] ", i);

		for (j = 0; j < action->nr_words; j++)
			print("%.2x:%.2x ",
			      action->word[j], action->word_type[j]);

		print("| ");

		for (j = 0; j < action->nr_words; j++) {
			word = find_dict_word_by_index(_game, action->word[j],
			                               action->word_type[j]);
			if (word)
				print("%-6s ", word->_word);
			else
				print("%.2x:%.2x  ", action->word[j],
				      action->word_type[j]);
		}

		print("-> %.4x\n", action->function);
	}
}

int DebuggerDumper::wordIndexCompare(const void *a, const void *b) {
	const Word *word_a = (const Word *)a, *word_b = (const Word *)b;

	if (word_a->_index > word_b->_index)
		return 1;
	if (word_a->_index < word_b->_index)
		return -1;
	return 0;
}

void DebuggerDumper::dumpDictionary() {
	Word *dictionary;
	Word *words;
	uint i;

	/* Sort the dictionary by index */
	dictionary = (Word *)malloc(sizeof(*words) * _game->_nr_words);
	memcpy(dictionary, _game->_words,
	       sizeof(*words) * _game->_nr_words);
	qsort(dictionary, _game->_nr_words, sizeof(*words),
	      wordIndexCompare);

	print("Dictionary (%u words)\n", (uint)_game->_nr_words);
	for (i = 0; i < _game->_nr_words; i++) {
		words = &dictionary[i];
		print("  [%.2x] %.2x %s\n", words->_index, words->_type,
		      words->_word);
	}

	free(dictionary);
}

void DebuggerDumper::dumpWordMap() {
	Word *word[3];
	char str[3][6];
	WordMap *map;
	uint i, j;

	print("Word pairs (%u entries)\n", _game->_wordMaps.size());
	for (i = 0; i < _game->_wordMaps.size(); i++) {
		map = &_game->_wordMaps[i];

		for (j = 0; j < 3; j++) {
			word[j] = dict_find_word_by_index_type(
			              _game, map->word[j].index, map->word[j].type);
			if (word[j])
				snprintf(str[j], sizeof(str[j]),
				         "%s", word[j]->_word);
			else
				snprintf(str[j], sizeof(str[j]), "%.2x:%.2x ",
				         map->word[j].index, map->word[j].type);
		}

		print("  [%.2x] %-6s %-6s -> %-6s\n",
		      i, str[0], str[1], str[2]);
	}
}

void DebuggerDumper::dumpRooms() {
	Room *room;
	uint i;

	// Room zero acts as the players inventory
	print("Rooms (%u entries)\n", (uint)_game->_rooms.size() - 1);
	for (i = 1; i < _game->_rooms.size(); i++) {
		room = &_game->_rooms[i];

		print("  [%.2x] flags=%.2x, graphic=%.2x\n",
		      i, room->flags, room->graphic);
		print("    %s\n", _game->stringLookup(room->string_desc).c_str());
		print("    n: %.2x  s: %.2x  e: %.2x  w: %.2x\n",
		      room->direction[DIRECTION_NORTH],
		      room->direction[DIRECTION_SOUTH],
		      room->direction[DIRECTION_EAST],
		      room->direction[DIRECTION_WEST]);
		print("    u: %.2x  d: %.2x  i: %.2x  o: %.2x\n",
		      room->direction[DIRECTION_UP],
		      room->direction[DIRECTION_DOWN],
		      room->direction[DIRECTION_IN],
		      room->direction[DIRECTION_OUT]);
		print("\n");
	}
}

void DebuggerDumper::dumpItems() {
	Item *item;
	uint i, j;

	print("Items (%u entries)\n", _game->_items.size());
	for (i = 0; i < _game->_items.size(); i++) {
		item = &_game->_items[i];

		print("  [%.2x] %s\n", i + 1,
		      item->string_desc ? _game->stringLookup(item->string_desc).c_str() : "");
		if (_game->_comprehendVersion == 2)
			print("    long desc: %s\n",
			      _game->stringLookup(item->long_string).c_str());

		print("    words: ");
		for (j = 0; j < _game->_nr_words; j++)
			if (_game->_words[j]._index == item->word &&
			        (_game->_words[j]._type & WORD_TYPE_NOUN_MASK))
				print("%s ", _game->_words[j]._word);
		print("\n");
		print("    flags=%.2x (takeable=%d, weight=%d)\n",
		      item->flags, !!(item->flags & ITEMF_CAN_TAKE),
		      (item->flags & ITEMF_WEIGHT_MASK));
		print("    room=%.2x, graphic=%.2x\n",
		      item->room, item->graphic);
		print("\n");
	}
}

void DebuggerDumper::dumpStringTable(Common::StringArray &table) {
	uint i;

	for (i = 0; i < table.size(); i++)
		print("[%.4x] %s\n", i, table[i].c_str());
}

void DebuggerDumper::dumpGameDataStrings() {
	print("Main string table (%u entries)\n",
	      _game->_strings.size());
	dumpStringTable(_game->_strings);
}

void DebuggerDumper::dumpExtraStrings() {
	print("Extra strings (%u entries)\n",
	      _game->_strings2.size());
	dumpStringTable(_game->_strings2);
}

void DebuggerDumper::dumpReplaceWords() {
	uint i;

	print("Replacement words (%u entries)\n",
	      _game->_replaceWords.size());
	for (i = 0; i < _game->_replaceWords.size(); i++)
		print("  [%.2x] %s\n", i + 1, _game->_replaceWords[i].c_str());
}

void DebuggerDumper::dumpHeader() {
	GameHeader *header = &_game->_header;
	uint16 *dir_table = header->room_direction_table;

	print("Game header:\n");
	print("  magic:                %.4x\n", header->magic);
	print("  action(vvnn):         %.4x\n", header->addr_actions_vvnn);
	print("  actions(?):\n");
	print("  actions(vnjn):        %.4x\n", header->addr_actions_vnjn);
	print("  actions(vjn):         %.4x\n", header->addr_actions_vjn);
	print("  actions(vdn):         %.4x\n", header->addr_actions_vdn);
	print("  actions(vnn):         %.4x\n", header->addr_actions_vnn);
	print("  actions(vn):          %.4x\n", header->addr_actions_vn);
	print("  actions(v):           %.4x\n", header->addr_actions_v);
	print("  functions:            %.4x\n", header->addr_vm);
	print("  dictionary:           %.4x\n", header->addr_dictionary);
	print("  word map pairs:       %.4x\n", header->addr_word_map);
	print("  room desc strings:    %.4x\n", header->room_desc_table);
	print("  room north:           %.4x\n", dir_table[DIRECTION_NORTH]);
	print("  room south:           %.4x\n", dir_table[DIRECTION_SOUTH]);
	print("  room east:            %.4x\n", dir_table[DIRECTION_EAST]);
	print("  room west:            %.4x\n", dir_table[DIRECTION_WEST]);
	print("  room up:              %.4x\n", dir_table[DIRECTION_UP]);
	print("  room down:            %.4x\n", dir_table[DIRECTION_DOWN]);
	print("  room in:              %.4x\n", dir_table[DIRECTION_IN]);
	print("  room out:             %.4x\n", dir_table[DIRECTION_OUT]);
	print("  room flags:           %.4x\n", header->room_flags_table);
	print("  room images:          %.4x\n", header->room_graphics_table);
	print("  item locations:       %.4x\n", header->addr_item_locations);
	print("  item flags:           %.4x\n", header->addr_item_flags);
	print("  item words:           %.4x\n", header->addr_item_word);
	print("  item desc strings:    %.4x\n", header->addr_item_strings);
	print("  item images:          %.4x\n", header->addr_item_graphics);
	print("  string table:         %.4x\n", header->addr_strings);
	print("  string table end:     %.4x\n", header->addr_strings_end);
}

void DebuggerDumper::dumpState() {
	print("Current room: %.2x\n", _game->_currentRoom);
	print("Carry weight %d/%d\n\n",
	      _game->_variables[VAR_INVENTORY_WEIGHT],
	      _game->_variables[VAR_INVENTORY_LIMIT]);

	print("Flags:\n");
	for (uint i = 0; i < ARRAY_SIZE(_game->_flags); i++)
		print("  [%.2x]: %d\n", i, _game->_flags[i]);
	print("\n");

	print("Variables:\n");
	for (uint i = 0; i < ARRAY_SIZE(_game->_variables); i++)
		print("  [%.2x]: %5d (0x%.4x)\n",
		      i, _game->_variables[i],
		      _game->_variables[i]);
	print("\n");
}

bool DebuggerDumper::dumpGameData(ComprehendGame *game, const Common::String &type) {
	_game = game;

	if (type == "header")
		dumpHeader();
	else if (type == "strings")
		dumpGameDataStrings();
	else if (type == "extra_strings")
		dumpExtraStrings();
	else if (type == "rooms")
		dumpRooms();
	else if (type == "items")
		dumpItems();
	else if (type == "dictionary")
		dumpDictionary();
	else if (type == "word_map")
		dumpWordMap();
	else if (type == "actions")
		dumpActionTable();
	else if (type == "functions")
		dumpFunctions();
	else if (type == "replace_words")
		dumpReplaceWords();
	else if (type == "state")
		dumpState();
	else
		return false;

	return true;
}

} // namespace Comprehend
} // namespace Glk
