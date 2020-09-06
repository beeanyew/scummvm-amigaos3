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

#include "common/system.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-builtins.h"
#include "director/lingo/lingo-code.h"
#include "director/frame.h"
#include "director/score.h"
#include "director/sound.h"
#include "director/sprite.h"
#include "director/stxt.h"
#include "director/util.h"

#include "graphics/macgui/macwindowmanager.h"
#include "graphics/macgui/macmenu.h"

namespace Director {

#define ARGNUMCHECK(n) \
	if (nargs != (n)) { \
		warning("%s: expected %d argument%s, got %d", __FUNCTION__, (n), ((n) == 1 ? "" : "s"), nargs); \
		g_lingo->dropStack(nargs); \
		return; \
	}

#define TYPECHECK(datum,t) \
	if ((datum).type != (t)) { \
		warning("%s: %s arg should be of type %s, not %s", __FUNCTION__, #datum, #t, (datum).type2str()); \
		return; \
	}

#define TYPECHECK2(datum, t1, t2)	\
	if ((datum).type != (t1) && (datum).type != (t2)) { \
		warning("%s: %s arg should be of type %s or %s, not %s", __FUNCTION__, #datum, #t1, #t2, (datum).type2str()); \
		return; \
	}

#define ARRBOUNDSCHECK(idx,array) \
	if ((idx)-1 < 0 || (idx) > (int)(array).u.farr->size()) { \
		warning("%s: index out of bounds (%d of %d)", __FUNCTION__, (idx), (array).u.farr->size()); \
		return; \
	}

static struct BuiltinProto {
	const char *name;
	void (*func)(int);
	int minArgs;	// -1 -- arglist
	int maxArgs;
	bool parens;
	int version;
	int type;
} builtins[] = {
	// Math
	{ "abs",			LB::b_abs,			1, 1, true, 2, FBLTIN },	// D2 function
	{ "atan",			LB::b_atan,			1, 1, true, 4, FBLTIN },	//			D4 f
	{ "cos",			LB::b_cos,			1, 1, true, 4, FBLTIN },	//			D4 f
	{ "exp",			LB::b_exp,			1, 1, true, 4, FBLTIN },	//			D4 f
	{ "float",			LB::b_float,		1, 1, true, 4, FBLTIN },	//			D4 f
	{ "integer",		LB::b_integer,		1, 1, true, 3, FBLTIN },	//		D3 f
	{ "log",			LB::b_log,			1, 1, true, 4, FBLTIN },	//			D4 f
	{ "pi",				LB::b_pi,			0, 0, true, 4, FBLTIN },	//			D4 f
	{ "power",			LB::b_power,		2, 2, true, 4, FBLTIN },	//			D4 f
	{ "random",			LB::b_random,		1, 1, true, 2, FBLTIN },	// D2 f
	{ "sin",			LB::b_sin,			1, 1, true, 4, FBLTIN },	//			D4 f
	{ "sqrt",			LB::b_sqrt,			1, 1, true, 2, FBLTIN },	// D2 f
	{ "tan",			LB::b_tan,			1, 1, true, 4, FBLTIN },	//			D4 f
	// String
	{ "chars",			LB::b_chars,		3, 3, true, 2, FBLTIN },	// D2 f
	{ "charToNum",		LB::b_charToNum,	1, 1, true, 2, FBLTIN },	// D2 f
	{ "delete",			LB::b_delete,		1, 1, true, 3, BLTIN },		//		D3 command
	{ "hilite",			LB::b_hilite,		1, 1, true, 3, BLTIN },		//		D3 c
	{ "length",			LB::b_length,		1, 1, true, 2, FBLTIN },	// D2 f
	{ "numToChar",		LB::b_numToChar,	1, 1, true, 2, FBLTIN },	// D2 f
	{ "offset",			LB::b_offset,		2, 3, true, 2, FBLTIN },	// D2 f
	{ "string",			LB::b_string,		1, 1, true, 2, FBLTIN },	// D2 f
	{ "value",		 	LB::b_value,		1, 1, true, 2, FBLTIN },	// D2 f
	// Lists
	{ "add",			LB::b_add,			2, 2, false, 4, BLTIN },	//			D4 c
	{ "addAt",			LB::b_addAt,		3, 3, false, 4, BLTIN },	//			D4 c
	{ "addProp",		LB::b_addProp,		3, 3, false, 4, BLTIN },	//			D4 c
	{ "append",			LB::b_append,		2, 2, false, 4, BLTIN },	//			D4 c
	{ "count",			LB::b_count,		1, 1, true,  4, FBLTIN },	//			D4 f
	{ "deleteAt",		LB::b_deleteAt,		2, 2, false, 4, BLTIN },	//			D4 c
	{ "deleteProp",		LB::b_deleteProp,	2, 2, false, 4, BLTIN },	//			D4 c
	{ "findPos",		LB::b_findPos,		2, 2, true,  4, FBLTIN },	//			D4 f
	{ "findPosNear",	LB::b_findPosNear,	2, 2, true,  4, FBLTIN },	//			D4 f
	{ "getaProp",		LB::b_getaProp,		2, 2, true,  4, FBLTIN },	//			D4 f
	{ "getAt",			LB::b_getAt,		2, 2, true,  4, FBLTIN },	//			D4 f
	{ "getLast",		LB::b_getLast,		1, 1, true,  4, FBLTIN },	//			D4 f
	{ "getOne",			LB::b_getOne,		2, 2, true,  4, FBLTIN },	//			D4 f
	{ "getPos",			LB::b_getPos,		2, 2, true,  4, FBLTIN },	//			D4 f
	{ "getProp",		LB::b_getProp,		2, 2, true,  4, FBLTIN },	//			D4 f
	{ "getPropAt",		LB::b_getPropAt,	2, 2, true,  4, FBLTIN },	//			D4 f
	{ "list",			LB::b_list,			-1, 0, true, 4, FBLTIN },	//			D4 f
	{ "listP",			LB::b_listP,		1, 1, true,  4, FBLTIN },	//			D4 f
	{ "max",			LB::b_max,			-1,0, true,  4, FBLTIN },	//			D4 f
	{ "min",			LB::b_min,			-1,0, true,  4, FBLTIN },	//			D4 f
	{ "setaProp",		LB::b_setaProp,		3, 3, false, 4, BLTIN },	//			D4 c
	{ "setAt",			LB::b_setAt,		3, 3, false, 4, BLTIN },	//			D4 c
	{ "setProp",		LB::b_setProp,		3, 3, false, 4, BLTIN },	//			D4 c
	{ "sort",			LB::b_sort,			1, 1, false, 4, BLTIN },	//			D4 c
	// Files
	{ "closeDA",	 	LB::b_closeDA, 		0, 0, false, 2, BLTIN },	// D2 c
	{ "closeResFile",	LB::b_closeResFile,	0, 1, false, 2, BLTIN },	// D2 c
	{ "closeXlib",		LB::b_closeXlib,	0, 1, false, 2, BLTIN },	// D2 c
	{ "getNthFileNameInFolder",LB::b_getNthFileNameInFolder,2,2,true,4,FBLTIN },//	D4 f
		// open															// D2 c
	{ "openDA",	 		LB::b_openDA, 		1, 1, false, 2, BLTIN },	// D2 c
	{ "openResFile",	LB::b_openResFile,	1, 1, false, 2, BLTIN },	// D2 c
	{ "openXlib",		LB::b_openXlib,		1, 1, false, 2, BLTIN },	// D2 c
	{ "saveMovie",		LB::b_saveMovie,	1, 1, false, 4, BLTIN },	//			D4 c
	{ "setCallBack",	LB::b_setCallBack,	2, 2, false, 3, BLTIN },	//		D3 c
	{ "showResFile",	LB::b_showResFile,	0, 1, false, 2, BLTIN },	// D2 c
	{ "showXlib",		LB::b_showXlib,		0, 1, false, 2, BLTIN },	// D2 c
	{ "xFactoryList",	LB::b_xFactoryList,	1, 1, true,  3, FBLTIN },	//		D3 f
	// Control
	{ "abort",			LB::b_abort,		0, 0, false, 4, BLTIN },	//			D4 c
	{ "continue",		LB::b_continue,		0, 0, false, 2, BLTIN },	// D2 c
	{ "dontPassEvent",	LB::b_dontPassEvent,0, 0, false, 2, BLTIN },	// D2 c
	{ "delay",	 		LB::b_delay,		1, 1, false, 2, BLTIN },	// D2 c
	{ "do",		 		LB::b_do,			1, 1, false, 2, BLTIN },	// D2 c
	{ "go",		 		LB::b_go,			1, 2, false, 4, BLTIN },	// 			D4 c
	{ "halt",	 		LB::b_halt,			0, 0, false, 4, BLTIN },	//			D4 c
	{ "nothing",		LB::b_nothing,		0, 0, false, 2, BLTIN },	// D2 c
	{ "pass",			LB::b_pass,			0, 0, false, 4, BLTIN },	//			D4 c
	{ "pause",			LB::b_pause,		0, 0, false, 2, BLTIN },	// D2 c
	{ "play",			LB::b_play,			1, 2, false, 2, BLTIN },	// D2 c
	{ "playAccel",		LB::b_playAccel,	-1,0, false, 2, BLTIN },	// D2
		// play done													// D2
	{ "preLoad",		LB::b_preLoad,		-1,0, false, 3, BLTIN },	//		D3.1 c
	{ "preLoadCast",	LB::b_preLoadCast,	-1,0, false, 3, BLTIN },	//		D3.1 c
	{ "quit",			LB::b_quit,			0, 0, false, 2, BLTIN },	// D2 c
	{ "restart",		LB::b_restart,		0, 0, false, 2, BLTIN },	// D2 c
	{ "return",			LB::b_return,		0, 1, false, 2, BLTIN },	// D2 f
	{ "shutDown",		LB::b_shutDown,		0, 0, false, 2, BLTIN },	// D2 c
	{ "startTimer",		LB::b_startTimer,	0, 0, false, 2, BLTIN },	// D2 c
		// when keyDown													// D2
		// when mouseDown												// D2
		// when mouseUp													// D2
		// when timeOut													// D2
	// Types
	{ "factory",		LB::b_factory,		1, 1, true,  3, FBLTIN },	//		D3
	{ "floatP",			LB::b_floatP,		1, 1, true,  3, FBLTIN },	//		D3
	{ "ilk",	 		LB::b_ilk,			1, 2, false, 4, FBLTIN },	//			D4 f
	{ "integerp",		LB::b_integerp,		1, 1, true,  2, FBLTIN },	// D2 f
	{ "objectp",		LB::b_objectp,		1, 1, true,  2, FBLTIN },	// D2 f
	{ "pictureP",		LB::b_pictureP,		1, 1, true,  4, FBLTIN },	//			D4 f
	{ "stringp",		LB::b_stringp,		1, 1, true,  2, FBLTIN },	// D2 f
	{ "symbolp",		LB::b_symbolp,		1, 1, true,  2, FBLTIN },	// D2 f
	{ "voidP",			LB::b_voidP,		1, 1, true,  4, FBLTIN },	//			D4 f
	// Misc
	{ "alert",	 		LB::b_alert,		1, 1, false, 2, BLTIN },	// D2 c
	{ "birth",	 		LB::b_birth,		-1,0, false, 4, FBLTIN },	//			D4 f
	{ "clearGlobals",	LB::b_clearGlobals,	0, 0, false, 3, BLTIN },	//		D3.1 c
	{ "cursor",	 		LB::b_cursor,		1, 1, false, 2, BLTIN },	// D2 c
	{ "framesToHMS",	LB::b_framesToHMS,	4, 4, false, 3, FBLTIN },	//		D3 f
	{ "HMStoFrames",	LB::b_HMStoFrames,	4, 4, false, 3, FBLTIN },	//		D3 f
	{ "param",	 		LB::b_param,		1, 1, true,  4, FBLTIN },	//			D4 f
	{ "printFrom",	 	LB::b_printFrom,	-1,0, false, 2, BLTIN },	// D2 c
	{ "put",			LB::b_put,			-1,0, false, 2, BLTIN },	// D2
		// set															// D2
	{ "showGlobals",	LB::b_showGlobals,	0, 0, false, 2, BLTIN },	// D2 c
	{ "showLocals",		LB::b_showLocals,	0, 0, false, 2, BLTIN },	// D2 c
	// Score
	{ "constrainH",		LB::b_constrainH,	2, 2, true,  2, FBLTIN },	// D2 f
	{ "constrainV",		LB::b_constrainV,	2, 2, true,  2, FBLTIN },	// D2 f
	{ "copyToClipBoard",LB::b_copyToClipBoard,1,1, false, 4, BLTIN },	//			D4 c
	{ "duplicate",		LB::b_duplicate,	1, 2, false, 4, BLTIN },	//			D4 c
	{ "editableText",	LB::b_editableText,	0, 0, false, 2, BLTIN },	// D2, FIXME: the field in D4+
	{ "erase",			LB::b_erase,		1, 1, false, 4, BLTIN },	//			D4 c
	{ "findEmpty",		LB::b_findEmpty,	1, 1, true,  4, FBLTIN },	//			D4 f
		// go															// D2
	{ "importFileInto",	LB::b_importFileInto,2, 2, false, 4, BLTIN },	//			D4 c
	{ "installMenu",	LB::b_installMenu,	1, 1, false, 2, BLTIN },	// D2 c
	{ "label",			LB::b_label,		1, 1, true,  2, FBLTIN },	// D2 f
	{ "marker",			LB::b_marker,		1, 1, true,  2, FBLTIN },	// D2 f
	{ "move",			LB::b_move,			1, 2, false, 4, BLTIN },	//			D4 c
	{ "moveableSprite",	LB::b_moveableSprite,0, 0, false, 2, BLTIN },	// D2, FIXME: the field in D4+
	{ "pasteClipBoardInto",LB::b_pasteClipBoardInto,1,1,false,4,BLTIN },//			D4 c
	{ "puppetPalette",	LB::b_puppetPalette, -1,0, false, 2, BLTIN },	// D2 c
	{ "puppetSound",	LB::b_puppetSound,	-1,0, false, 2, BLTIN },	// D2 c
	{ "puppetSprite",	LB::b_puppetSprite,	-1,0, false, 2, BLTIN },	// D2 c
	{ "puppetTempo",	LB::b_puppetTempo,	1, 1, false, 2, BLTIN },	// D2 c
	{ "puppetTransition",LB::b_puppetTransition,-1,0,false,2, BLTIN },	// D2 c
	{ "ramNeeded",		LB::b_ramNeeded,	2, 2, true,  3, FBLTIN },	//		D3.1 f
	{ "rollOver",		LB::b_rollOver,		1, 1, true,  2, FBLTIN },	// D2 f
	{ "spriteBox",		LB::b_spriteBox,	-1,0, false, 2, BLTIN },	// D2 c
	{ "unLoad",			LB::b_unLoad,		0, 2, false, 3, BLTIN },	//		D3.1 c
	{ "unLoadCast",		LB::b_unLoadCast,	0, 2, false, 3, BLTIN },	//		D3.1 c
	{ "updateStage",	LB::b_updateStage,	0, 0, false, 2, BLTIN },	// D2 c
	{ "zoomBox",		LB::b_zoomBox,		-1,0, false, 2, BLTIN },	// D2 c
	// Point
	{ "point",			LB::b_point,		2, 2, true,  4, FBLTIN },	//			D4 f
	{ "inside",			LB::b_inside,		2, 2, true,  4, FBLTIN },	//			D4 f
	{ "intersect",		LB::b_intersect,	2, 2, false, 4, FBLTIN },	//			D4 f
	{ "map",			LB::b_map,			3, 3, true,  4, FBLTIN },	//			D4 f
	{ "rect",			LB::b_rect,			4, 4, true,  4, FBLTIN },	//			D4 f
	{ "union",			LB::b_union,		2, 2, true,  4, FBLTIN },	//			D4 f
	// Sound
	{ "beep",	 		LB::b_beep,			0, 1, false, 2, BLTIN },	// D2
	{ "mci",	 		LB::b_mci,			1, 1, false, 3, BLTIN },	//		D3.1 c
	{ "mciwait",		LB::b_mciwait,		1, 1, false, 4, BLTIN },	//			D4 c
	{ "sound",			LB::b_sound,		2, 3, false, 3, BLTIN },	//		D3 c
	{ "soundBusy",		LB::b_soundBusy,	1, 1, true,  3, FBLTIN },	//		D3 f
	// Window
	{ "close",			LB::b_close,		1, 1, false, 4, BLTIN },	//			D4 c
	{ "forget",			LB::b_forget,		1, 1, false, 4, BLTIN },	//			D4 c
	{ "inflate",		LB::b_inflate,		3, 3, true,  4, FBLTIN },	//			D4 f
	{ "moveToBack",		LB::b_moveToBack,	1, 1, false, 4, BLTIN },	//			D4 c
	{ "moveToFront",	LB::b_moveToFront,	1, 1, false, 4, BLTIN },	//			D4 c
	// Constants
	{ "backspace",		LB::b_backspace,	0, 0, false, 2, FBLTIN },	// D2
	{ "empty",			LB::b_empty,		0, 0, false, 2, FBLTIN },	// D2
	{ "enter",			LB::b_enter,		0, 0, false, 2, FBLTIN },	// D2
	{ "false",			LB::b_false,		0, 0, false, 2, FBLTIN },	// D2
	{ "quote",			LB::b_quote,		0, 0, false, 2, FBLTIN },	// D2
	{ "scummvm_return",	LB::b_returnconst,	0, 0, false, 2, FBLTIN },	// D2
	{ "tab",			LB::b_tab,			0, 0, false, 2, FBLTIN },	// D2
	{ "true",			LB::b_true,			0, 0, false, 2, FBLTIN },	// D2
	{ "version",		LB::b_version,		0, 0, false, 3, FBLTIN },	//		D3
	// References
	{ "cast",			LB::b_cast,			1, 1, false, 4, RBLTIN },	//			D4 f
	{ "field",			LB::b_field,		1, 1, false, 3, RBLTIN },	//		D3 f
	{ "script",			LB::b_script,		1, 1, false, 4, RBLTIN },	//			D4 f
	{ "window",			LB::b_window,		1, 1, false, 4, RBLTIN },	//			D4 f
	// Chunk operations
	{ "numberOfChars",	LB::b_numberofchars,1, 1, false, 3, FBLTIN },	//			D3 f
	{ "numberOfItems",	LB::b_numberofitems,1, 1, false, 3, FBLTIN },	//			D3 f
	{ "numberOfLines",	LB::b_numberoflines,1, 1, false, 3, FBLTIN },	//			D3 f
	{ "numberOfWords",	LB::b_numberofwords,1, 1, false, 3, FBLTIN },	//			D3 f
	{ "lastCharOf",		LB::b_lastcharof,	1, 1, false, 4, FBLTIN },	//			D4 f
	{ "lastItemOf",		LB::b_lastitemof,	1, 1, false, 4, FBLTIN },	//			D4 f
	{ "lastLineOf",		LB::b_lastlineof,	1, 1, false, 4, FBLTIN },	//			D4 f
	{ "lastWordOf",		LB::b_lastwordof,	1, 1, false, 4, FBLTIN },	//			D4 f

	{ 0, 0, 0, 0, false, 0, 0 }
};

void Lingo::initBuiltIns() {
	for (BuiltinProto *blt = builtins; blt->name; blt++) {
		if (blt->version > _vm->getVersion())
			continue;

		Symbol sym;

		sym.name = new Common::String(blt->name);
		sym.type = blt->type;
		sym.nargs = blt->minArgs;
		sym.maxArgs = blt->maxArgs;
		sym.parens = blt->parens;
		sym.u.bltin = blt->func;

		_builtins[blt->name] = sym;

		_functions[(void *)sym.u.s] = new FuncDesc(blt->name, "");
	}
}

void Lingo::cleanupBuiltins() {
	for (FuncHash::iterator it = _functions.begin(); it != _functions.end(); ++it)
		delete it->_value;
}

void Lingo::printSTUBWithArglist(const char *funcname, int nargs, const char *prefix) {
	Common::String s(funcname);

	s += '(';

	for (int i = 0; i < nargs; i++) {
		Datum d = _stack[_stack.size() - nargs + i];

		s += d.asString(true);

		if (i != nargs - 1)
			s += ", ";
	}

	s += ")";

	debug(5, "%s %s", prefix, s.c_str());
}

void Lingo::convertVOIDtoString(int arg, int nargs) {
	if (_stack[_stack.size() - nargs + arg].type == VOID) {
		if (_stack[_stack.size() - nargs + arg].u.s != NULL)
			g_lingo->_stack[_stack.size() - nargs + arg].type = STRING;
		else
			warning("Incorrect convertVOIDtoString for arg %d of %d", arg, nargs);
	}
}

void Lingo::dropStack(int nargs) {
	for (int i = 0; i < nargs; i++)
		pop();
}

void Lingo::drop(uint num) {
	if (num > _stack.size() - 1) {
		warning("Incorrect number of elements to drop from stack: %d > %d", num, _stack.size() - 1);
		return;
	}
	_stack.remove_at(_stack.size() - 1 - num);
}


///////////////////
// Math
///////////////////
void LB::b_abs(int nargs) {
	Datum d = g_lingo->pop();

	if (d.type == INT)
		d.u.i = ABS(d.u.i);
	else if (d.type == FLOAT)
		d.u.f = ABS(d.u.f);

	g_lingo->push(d);
}

void LB::b_atan(int nargs) {
	Datum d = g_lingo->pop();
	Datum res(atan(d.asFloat()));
	g_lingo->push(res);
}

void LB::b_cos(int nargs) {
	Datum d = g_lingo->pop();
	Datum res(cos(d.asFloat()));
	g_lingo->push(res);
}

void LB::b_exp(int nargs) {
	Datum d = g_lingo->pop();
	// Lingo uses int, so we're enforcing it
	Datum res((double)exp((double)d.asInt()));
	g_lingo->push(res);
}

void LB::b_float(int nargs) {
	Datum d = g_lingo->pop();
	Datum res(d.asFloat());
	g_lingo->push(res);
}

void LB::b_integer(int nargs) {
	Datum d = g_lingo->pop();
	Datum res(d.asInt());
	g_lingo->push(res);
}

void LB::b_log(int nargs) {
	Datum d = g_lingo->pop();
	Datum res(log(d.asFloat()));
	g_lingo->push(res);
}

void LB::b_pi(int nargs) {
	Datum res((double)M_PI);
	g_lingo->push(res);
}

void LB::b_power(int nargs) {
	Datum d1 = g_lingo->pop();
	Datum d2 = g_lingo->pop();
	Datum res(pow(d2.asFloat(), d1.asFloat()));
	g_lingo->push(d1);
}

void LB::b_random(int nargs) {
	Datum max = g_lingo->pop();
	Datum res((int)(g_lingo->_vm->_rnd.getRandomNumber(max.asInt() - 1) + 1));
	g_lingo->push(res);
}

void LB::b_sin(int nargs) {
	Datum d = g_lingo->pop();
	Datum res(sin(d.asFloat()));
	g_lingo->push(res);
}

void LB::b_sqrt(int nargs) {
	Datum d = g_lingo->pop();
	Datum res(sqrt(d.asFloat()));
	g_lingo->push(res);
}

void LB::b_tan(int nargs) {
	Datum d = g_lingo->pop();
	Datum res(tan(d.asFloat()));
	g_lingo->push(res);
}

///////////////////
// String
///////////////////
void LB::b_chars(int nargs) {
	int to = g_lingo->pop().asInt();
	int from = g_lingo->pop().asInt();
	Datum s = g_lingo->pop();
	TYPECHECK2(s, STRING, REFERENCE);

	Common::String src = s.asString();

	int len = strlen(src.c_str());
	int f = MAX(0, MIN(len, from - 1));
	int t = MAX(0, MIN(len, to));

	Common::String result;
	if (f > t) {
		result = Common::String("");
	} else {
		result = Common::String(&(src.c_str()[f]), &(src.c_str()[t]));
	}

	Datum res(result);
	g_lingo->push(res);
}

void LB::b_charToNum(int nargs) {
	Datum d = g_lingo->pop();

	TYPECHECK(d, STRING);

	int chr = (uint8)d.u.s->c_str()[0];

	Datum res(chr);
	g_lingo->push(res);
}

void LB::b_delete(int nargs) {
	Datum d = g_lingo->pop();

	Datum res(d.asInt());

	warning("STUB: b_delete");

	g_lingo->push(res);
}

void LB::b_hilite(int nargs) {
	Datum d = g_lingo->pop();

	Datum res(d.asInt());

	warning("STUB: b_hilite");

	g_lingo->push(res);
}

void LB::b_length(int nargs) {
	Datum d = g_lingo->pop();
	TYPECHECK2(d, STRING, REFERENCE);

	int len = strlen(d.asString().c_str());

	Datum res(len);
	g_lingo->push(res);
}

void LB::b_numToChar(int nargs) {
	Datum d = g_lingo->pop();

	char result[2];
	result[0] = (char)d.asInt();
	result[1] = 0;

	g_lingo->push(Datum(Common::String(result)));
}

void LB::b_offset(int nargs) {
	if (nargs == 3) {
		b_offsetRect(nargs);
		return;
	}
	Common::String target = g_lingo->pop().asString();
	Common::String source = g_lingo->pop().asString();

	warning("STUB: b_offset()");

	g_lingo->push(Datum(0));
}

void LB::b_string(int nargs) {
	Datum d = g_lingo->pop();
	Datum res(d.asString());
	g_lingo->push(res);
}

void LB::b_value(int nargs) {
	Datum d = g_lingo->pop();
	warning("STUB: b_value()");
	g_lingo->push(Datum(0));
}

///////////////////
// Lists
///////////////////
void LB::b_add(int nargs) {
	// FIXME: when a list is "sorted", add should insert based on
	// the current ordering. otherwise, append to the end.
	LB::b_append(nargs);
}

void LB::b_addAt(int nargs) {
	ARGNUMCHECK(3);

	Datum value = g_lingo->pop();
	Datum indexD = g_lingo->pop();
	Datum list = g_lingo->pop();

	TYPECHECK2(indexD, INT, FLOAT);
	int index = indexD.asInt();
	TYPECHECK(list, ARRAY);

	int size = list.u.farr->size();
	if (index > size) {
		for (int i = 0; i < index - size - 1; i++)
			list.u.farr->push_back(Datum(0));
	}
	list.u.farr->insert_at(index - 1, value);
}

void LB::b_addProp(int nargs) {
	ARGNUMCHECK(3);

	Datum value = g_lingo->pop();
	Datum prop = g_lingo->pop();
	Datum list = g_lingo->pop();

	TYPECHECK(list, PARRAY);
	if (prop.type == REFERENCE)
		prop = g_lingo->varFetch(prop);

	PCell cell = PCell(prop, value);
	list.u.parr->push_back(cell);
}

void LB::b_append(int nargs) {
	ARGNUMCHECK(2);

	Datum value = g_lingo->pop();
	Datum list = g_lingo->pop();

	TYPECHECK(list, ARRAY);

	list.u.farr->push_back(value);
}

void LB::b_count(int nargs) {
	ARGNUMCHECK(1);

	Datum list = g_lingo->pop();
	Datum result;
	result.type = INT;

	switch (list.type) {
	case ARRAY:
		result.u.i = list.u.farr->size();
		break;
	case PARRAY:
		result.u.i = list.u.parr->size();
		break;
	default:
		TYPECHECK2(list, ARRAY, PARRAY);
	}

	g_lingo->push(result);
}

void LB::b_deleteAt(int nargs) {
	ARGNUMCHECK(2);

	Datum indexD = g_lingo->pop();
	Datum list = g_lingo->pop();
	TYPECHECK2(indexD, INT, FLOAT);
	TYPECHECK2(list, ARRAY, PARRAY);
	int index = indexD.asInt();

	switch (list.type) {
	case ARRAY:
		list.u.farr->remove_at(index - 1);
		break;
	case PARRAY:
		list.u.parr->remove_at(index - 1);
		break;
	default:
		break;
	}
}

void LB::b_deleteProp(int nargs) {
	ARGNUMCHECK(2);

	Datum prop = g_lingo->pop();
	Datum list = g_lingo->pop();
	TYPECHECK2(list, ARRAY, PARRAY);

	switch (list.type) {
	case ARRAY:
		g_lingo->push(list);
		g_lingo->push(prop);
		b_deleteAt(nargs);
		break;
	case PARRAY: {
		int index = LC::compareArrays(LC::eqData, list, prop, true).u.i;
		if (index > 0) {
			list.u.parr->remove_at(index - 1);
		}
		break;
	}
	default:
		break;
	}
}

void LB::b_findPos(int nargs) {
	ARGNUMCHECK(2);

	Datum prop = g_lingo->pop();
	Datum list = g_lingo->pop();
	Datum d(0);
	TYPECHECK(list, PARRAY);

	int index = LC::compareArrays(LC::eqData, list, prop, true).u.i;
	if (index > 0) {
		d.type = INT;
		d.u.i = index;
	}

	g_lingo->push(d);
}

void LB::b_findPosNear(int nargs) {
	ARGNUMCHECK(2);

	Common::String prop = g_lingo->pop().asString();
	Datum list = g_lingo->pop();
	Datum res(0);
	TYPECHECK(list, PARRAY);

	// FIXME: Integrate with compareTo framework
	prop.toLowercase();

	for (uint i = 0; i < list.u.parr->size(); i++) {
		Datum p = list.u.parr->operator[](i).p;
		Common::String tgt = p.asString();
		tgt.toLowercase();
		if (tgt.find(prop.c_str()) == 0) {
			res.u.i = i + 1;
			break;
		}
	}

	g_lingo->push(res);
}

void LB::b_getaProp(int nargs) {
	ARGNUMCHECK(2);
	Datum prop = g_lingo->pop();
	Datum list = g_lingo->pop();

	switch (list.type) {
	case ARRAY:
		g_lingo->push(list);
		g_lingo->push(prop);
		b_getAt(nargs);
		break;
	case PARRAY: {
		Datum d;
		int index = LC::compareArrays(LC::eqData, list, prop, true).u.i;
		if (index > 0) {
			d = list.u.parr->operator[](index - 1).v;
		}
		g_lingo->push(d);
		break;
	}
	default:
		TYPECHECK2(list, ARRAY, PARRAY);
	}
}

void LB::b_getAt(int nargs) {
	ARGNUMCHECK(2);

	Datum indexD = g_lingo->pop();
	TYPECHECK2(indexD, INT, FLOAT);
	Datum list = g_lingo->pop();
	int index = indexD.asInt();

	switch (list.type) {
	case ARRAY:
		ARRBOUNDSCHECK(index, list);
		g_lingo->push(list.u.farr->operator[](index - 1));
		break;
	case PARRAY:
		ARRBOUNDSCHECK(index, list);
		g_lingo->push(list.u.parr->operator[](index - 1).v);
		break;
	default:
		TYPECHECK2(list, ARRAY, PARRAY);
	}
}

void LB::b_getLast(int nargs) {
	ARGNUMCHECK(1);

	Datum list = g_lingo->pop();
	switch (list.type) {
	case ARRAY:
		g_lingo->push(list.u.farr->back());
		break;
	case PARRAY:
		g_lingo->push(list.u.parr->back().v);
		break;
	default:
		TYPECHECK(list, ARRAY);
	}
}

void LB::b_getOne(int nargs) {
	ARGNUMCHECK(2);
	Datum val = g_lingo->pop();
	Datum list = g_lingo->pop();

	switch (list.type) {
	case ARRAY:
		g_lingo->push(list);
		g_lingo->push(val);
		b_getPos(nargs);
		break;
	case PARRAY: {
		Datum d;
		int index = LC::compareArrays(LC::eqData, list, val, true, true).u.i;
		if (index > 0) {
			d = list.u.parr->operator[](index - 1).p;
		}
		g_lingo->push(d);
		break;
	}
	default:
		TYPECHECK2(list, ARRAY, PARRAY);
	}
}

void LB::b_getPos(int nargs) {
	ARGNUMCHECK(2);
	Datum val = g_lingo->pop();
	Datum list = g_lingo->pop();
	TYPECHECK2(val, INT, FLOAT);
	TYPECHECK2(list, ARRAY, PARRAY);

	switch (list.type) {
	case ARRAY: {
		Datum d(0);
		int index = LC::compareArrays(LC::eqData, list, val, true).u.i;
		if (index > 0) {
			d.u.i = index;
		}
		g_lingo->push(d);
		break;
	}
	case PARRAY: {
		Datum d(0);
		int index = LC::compareArrays(LC::eqData, list, val, true, true).u.i;
		if (index > 0) {
			d.u.i = index;
		}
		g_lingo->push(d);
		break;
	}
	default:
		break;
	}
}

void LB::b_getProp(int nargs) {
	ARGNUMCHECK(2);
	Datum prop = g_lingo->pop();
	Datum list = g_lingo->pop();
	TYPECHECK2(list, ARRAY, PARRAY);

	switch (list.type) {
	case ARRAY:
		g_lingo->push(list);
		g_lingo->push(prop);
		b_getPos(nargs);
		break;
	case PARRAY: {
		int index = LC::compareArrays(LC::eqData, list, prop, true).u.i;
		if (index > 0) {
			g_lingo->push(list.u.parr->operator[](index - 1).v);
		} else {
			error("b_getProp: Property %s not found", prop.asString().c_str());
		}
		break;
	}
	default:
		break;
	}
}

void LB::b_getPropAt(int nargs) {
	ARGNUMCHECK(2);

	Datum indexD = g_lingo->pop();
	Datum list = g_lingo->pop();
	TYPECHECK2(indexD, INT, FLOAT);
	TYPECHECK(list, PARRAY);
	int index = indexD.asInt();

	g_lingo->push(list.u.parr->operator[](index - 1).p);
}

void LB::b_list(int nargs) {
	Datum result;
	result.type = ARRAY;
	result.u.farr = new DatumArray;

	for (int i = 0; i < nargs; i++)
		result.u.farr->insert_at(0, g_lingo->pop());

	g_lingo->push(result);
}

void LB::b_listP(int nargs) {
	ARGNUMCHECK(1);
	Datum list = g_lingo->pop();
	Datum d(0);
	if (list.type == ARRAY || list.type == PARRAY) {
		d.u.i = 1;
	}
	g_lingo->push(d);
}

void LB::b_max(int nargs) {
	Datum max;
	max.type = INT;
	max.u.i = 0;

	if (nargs == 1) {
		Datum d = g_lingo->pop();
		if (d.type == ARRAY) {
			uint arrsize = d.u.farr->size();
			for (uint i = 0; i < arrsize; i++) {
				Datum item = d.u.farr->operator[](i);
				if (i == 0 || item.compareTo(max) > 0) {
					max = item;
				}
			}
		} else {
			max = d;
		}
	} else if (nargs > 0) {
		for (int i = 0; i < nargs; i++) {
			Datum d = g_lingo->_stack[g_lingo->_stack.size() - nargs + i];
			if (d.type == ARRAY) {
				warning("b_max: undefined behavior: array mixed with other args");
			}
			if (i == 0 || d.compareTo(max) > 0) {
				max = d;
			}
		}
		g_lingo->dropStack(nargs);
	}
	g_lingo->push(max);
}

void LB::b_min(int nargs) {
	Datum min;
	min.type = INT;
	min.u.i = 0;

	if (nargs == 1) {
		Datum d = g_lingo->pop();
		if (d.type == ARRAY) {
			uint arrsize = d.u.farr->size();
			for (uint i = 0; i < arrsize; i++) {
				Datum item = d.u.farr->operator[](i);
				if (i == 0 || item.compareTo(min) < 0) {
					min = item;
				}
			}
		} else {
			min = d;
		}
	} else if (nargs > 0) {
		for (int i = 0; i < nargs; i++) {
			Datum d = g_lingo->_stack[g_lingo->_stack.size() - nargs + i];
			if (d.type == ARRAY) {
				warning("b_min: undefined behavior: array mixed with other args");
			}
			if (i == 0 || d.compareTo(min) < 0) {
				min = d;
			}
		}
		g_lingo->dropStack(nargs);
	}
	g_lingo->push(min);
}

void LB::b_setaProp(int nargs) {
	ARGNUMCHECK(3);
	Datum value = g_lingo->pop();
	Datum prop = g_lingo->pop();
	Datum list = g_lingo->pop();

	switch (list.type) {
	case ARRAY:
		g_lingo->push(list);
		g_lingo->push(prop);
		g_lingo->push(value);
		b_setAt(nargs);
		break;
	case PARRAY: {
		int index = LC::compareArrays(LC::eqData, list, prop, true).u.i;
		if (index > 0) {
			list.u.parr->operator[](index - 1).v = value;
		} else {
			PCell cell = PCell(prop, value);
			list.u.parr->push_back(cell);
		}
		break;
	}
	default:
		TYPECHECK2(list, ARRAY, PARRAY);
	}
}

void LB::b_setAt(int nargs) {
	ARGNUMCHECK(3);
	Datum value = g_lingo->pop();
	Datum indexD = g_lingo->pop();
	Datum list = g_lingo->pop();

	TYPECHECK2(indexD, INT, FLOAT);
	TYPECHECK2(list, ARRAY, PARRAY);
	int index = indexD.asInt();

	switch (list.type) {
	case ARRAY:
		if ((uint)index < list.u.farr->size()) {
			list.u.farr->operator[](index - 1) = value;
		} else {
			// TODO: Extend the list if we request an index beyond it
			ARRBOUNDSCHECK(index, list);
		}
		break;
	case PARRAY:
		ARRBOUNDSCHECK(index, list);
		list.u.parr->operator[](index - 1).v = value;
		break;
	default:
		break;
	}
}

void LB::b_setProp(int nargs) {
	ARGNUMCHECK(3);
	Datum value = g_lingo->pop();
	Datum prop = g_lingo->pop();
	Datum list = g_lingo->pop();
	TYPECHECK(list, PARRAY);
	if (prop.type == REFERENCE)
		prop = g_lingo->varFetch(prop);

	int index = LC::compareArrays(LC::eqData, list, prop, true).u.i;
	if (index > 0) {
		list.u.parr->operator[](index - 1).v = value;
	} else {
		warning("b_setProp: Property not found");
	}
}

void LB::b_sort(int nargs) {
	g_lingo->printSTUBWithArglist("b_sort", nargs);
	g_lingo->dropStack(nargs);
}


///////////////////
// Files
///////////////////
void LB::b_closeDA(int nargs) {
	warning("STUB: b_closeDA");
}

void LB::b_closeResFile(int nargs) {
	Datum d = g_lingo->pop();

	warning("STUB: b_closeResFile(%s)", d.asString().c_str());
}

void LB::b_closeXlib(int nargs) {
	Datum d = g_lingo->pop();

	warning("STUB: b_closeXlib(%s)", d.asString().c_str());
}

void LB::b_getNthFileNameInFolder(int nargs) {
	g_lingo->printSTUBWithArglist("b_getNthFileNameInFolder", nargs);

	g_lingo->dropStack(nargs);

	g_lingo->push(Datum(0));
}

void LB::b_openDA(int nargs) {
	Datum d = g_lingo->pop();

	warning("STUB: b_openDA(%s)", d.asString().c_str());
}

void LB::b_openResFile(int nargs) {
	Datum d = g_lingo->pop();

	warning("STUB: b_openResFile(%s)", d.asString().c_str());
}

void LB::b_openXlib(int nargs) {
	Datum d = g_lingo->pop();

	warning("STUB: b_openXlib(%s)", d.asString().c_str());
}

void LB::b_saveMovie(int nargs) {
	g_lingo->printSTUBWithArglist("b_saveMovie", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_setCallBack(int nargs) {
	warning("STUB: b_setCallBack");
}

void LB::b_showResFile(int nargs) {
	Datum d = g_lingo->pop();

	warning("STUB: b_showResFile(%s)", d.asString().c_str());
}

void LB::b_showXlib(int nargs) {
	Datum d = g_lingo->pop();

	warning("STUB: b_showXlib(%s)", d.asString().c_str());
}

void LB::b_xFactoryList(int nargs) {
	Datum d = g_lingo->pop();

	warning("STUB: b_xFactoryList(%s)", d.asString().c_str());
}

///////////////////
// Control
///////////////////
void LB::b_abort(int nargs) {
	warning("STUB: b_abort");
}

void LB::b_continue(int nargs) {
	g_director->_playbackPaused = false;
}

void LB::b_dontPassEvent(int nargs) {
	g_lingo->_dontPassEvent = true;
	warning("dontPassEvent raised");
}

void LB::b_nothing(int nargs) {
	// Noop
}

void LB::b_delay(int nargs) {
	Datum d = g_lingo->pop();

	g_director->getCurrentScore()->_nextFrameTime = g_system->getMillis() + (float)d.asInt() / 60 * 1000;
}

void LB::b_do(int nargs) {
	Datum d = g_lingo->pop();
	warning("STUB: b_do(%s)", d.asString().c_str());
}

void LB::b_go(int nargs) {
	// Builtin function for go as used by the Director bytecode engine.
	//
	// Accepted arguments:
	// "loop"
	// "next"
	// "previous"
	// (STRING|INT) frame
	// STRING movie, (STRING|INT) frame

	if (nargs >= 1 && nargs <= 2) {
		Datum firstArg = g_lingo->pop();
		nargs -= 1;
		bool callSpecial = false;

		if (firstArg.type == STRING) {
			if (*firstArg.u.s == "loop") {
				g_lingo->func_gotoloop();
				callSpecial = true;
			} else if (*firstArg.u.s == "next") {
				g_lingo->func_gotonext();
				callSpecial = true;
			} else if (*firstArg.u.s == "previous") {
				g_lingo->func_gotoprevious();
				callSpecial = true;
			}
		}

		if (!callSpecial) {
			Datum movie;
			Datum frame;

			if (nargs > 0) {
				movie = firstArg;
				TYPECHECK(movie, STRING);

				frame = g_lingo->pop();
				nargs -= 1;
			} else {
				frame = firstArg;
			}

			if (frame.type != STRING && frame.type != INT) {
				warning("b_go: frame arg should be of type STRING or INT, not %s", frame.type2str());
			}

			g_lingo->func_goto(frame, movie);
		}

		if (nargs > 0) {
			warning("b_go: ignoring %d extra args", nargs);
			g_lingo->dropStack(nargs);
		}

	} else {
		warning("b_go: expected 1 or 2 args, not %d", nargs);
		g_lingo->dropStack(nargs);
	}
}

void LB::b_halt(int nargs) {
	b_quit(nargs);

	warning("Movie halted");
}

void LB::b_pass(int nargs) {
	g_lingo->printSTUBWithArglist("b_pass", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_pause(int nargs) {
	g_director->_playbackPaused = true;
}

void LB::b_play(int nargs) {
	// Builtin function for play as used by the Director bytecode engine.
	//
	// Accepted arguments:
	// 0  									# "play done"
	// (STRING|INT) frame
	// STRING movie, (STRING|INT) frame

	if (nargs >= 1 && nargs <= 2) {
		Datum movie;
		Datum frame;

		Datum firstArg = g_lingo->pop();
		if (nargs == 2) {
			movie = firstArg;
			frame = g_lingo->pop();
		} else {
			if (firstArg.asInt() == 0) {
				frame.type = SYMBOL;
				frame.u.s = new Common::String("done");
			} else {
				frame = firstArg;
			}
		}

		g_lingo->func_play(frame, movie);
	} else {
		warning("b_play: expected 1 or 2 args, not %d", nargs);
		g_lingo->dropStack(nargs);
	}
}

void LB::b_playAccel(int nargs) {
	g_lingo->printSTUBWithArglist("b_playAccel", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_preLoad(int nargs) {
	g_lingo->printSTUBWithArglist("b_preLoad", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_preLoadCast(int nargs) {
	g_lingo->printSTUBWithArglist("b_preLoadCast", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_framesToHMS(int nargs) {
	g_lingo->printSTUBWithArglist("b_framesToHMS", nargs);

	g_lingo->dropStack(nargs);

	g_lingo->push(Datum(0));
}

void LB::b_HMStoFrames(int nargs) {
	g_lingo->printSTUBWithArglist("b_HMStoFrames", nargs);

	g_lingo->dropStack(nargs);

	g_lingo->push(Datum(0));
}

void LB::b_param(int nargs) {
	g_lingo->printSTUBWithArglist("b_param", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_printFrom(int nargs) {
	g_lingo->printSTUBWithArglist("b_printFrom", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_quit(int nargs) {
	if (g_director->getCurrentScore())
		g_director->getCurrentScore()->_stopPlay = true;

	g_lingo->pushVoid();
}

void LB::b_return(int nargs) {
	CFrame *fp = g_lingo->_callstack.back();
	// Do not allow a factory's mNew method to return a value
	// Otherwise do not touch the top of the stack, it will be returned
	if (g_lingo->_currentMeObj && g_lingo->_currentMeObj->type == kFactoryObj && fp->sp.name->equalsIgnoreCase("mNew")) {
		g_lingo->pop();
	}
	LC::c_procret();
}

void LB::b_restart(int nargs) {
	b_quit(nargs);

	warning("Computer restarts");
}

void LB::b_shutDown(int nargs) {
	b_quit(nargs);

	warning("Computer shuts down");
}

void LB::b_startTimer(int nargs) {
	g_director->getCurrentScore()->_lastTimerReset = g_director->getMacTicks();
}

///////////////////
// Types
///////////////////
void LB::b_factory(int nargs) {
	Datum factoryName = g_lingo->pop();
	factoryName.type = VAR;
	Datum o = g_lingo->varFetch(factoryName, true);
	if (o.type == OBJECT && o.u.obj->type == kFactoryObj
			&& o.u.obj->name->equalsIgnoreCase(*factoryName.u.s) && o.u.obj->inheritanceLevel == 1) {
		g_lingo->push(o);
	} else {
		g_lingo->push(Datum(0));
	}
}

void LB::b_floatP(int nargs) {
	Datum d = g_lingo->pop();
	Datum res((d.type == FLOAT) ? 1 : 0);
	g_lingo->push(res);
}

void LB::b_ilk(int nargs) {
	Datum d = g_lingo->pop();
	Datum res(Common::String(d.type2str(true)));
	g_lingo->push(res);
}

void LB::b_integerp(int nargs) {
	Datum d = g_lingo->pop();
	Datum res((d.type == INT) ? 1 : 0);
	g_lingo->push(res);
}

void LB::b_objectp(int nargs) {
	Datum d = g_lingo->pop();
	Datum res;
	if (d.type == OBJECT) {
		res = !d.u.obj->disposed;
	} else {
		res = 0;
	}
	g_lingo->push(res);
}

void LB::b_pictureP(int nargs) {
	g_lingo->pop();
	warning("STUB: b_pictureP");
	g_lingo->push(Datum(0));
}

void LB::b_stringp(int nargs) {
	Datum d = g_lingo->pop();
	Datum res((d.type == STRING) ? 1 : 0);
	g_lingo->push(res);
}

void LB::b_symbolp(int nargs) {
	Datum d = g_lingo->pop();
	Datum res((d.type == SYMBOL) ? 1 : 0);
	g_lingo->push(res);
}

void LB::b_voidP(int nargs) {
	Datum d = g_lingo->pop();
	Datum res((d.type == VOID) ? 1 : 0);
	g_lingo->push(res);
}


///////////////////
// Misc
///////////////////
void LB::b_alert(int nargs) {
	Datum d = g_lingo->pop();

	warning("STUB: b_alert(%s)", d.asString().c_str());
}

void LB::b_birth(int nargs) {
	g_lingo->printSTUBWithArglist("b_birth", nargs);

	g_lingo->dropStack(nargs);

	g_lingo->push(Datum(0));
}

void LB::b_clearGlobals(int nargs) {
	g_lingo->printSTUBWithArglist("b_clearGlobals", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_cursor(int nargs) {
	Datum d = g_lingo->pop();

	if (d.type == ARRAY) {
		Datum sprite = d.u.farr->operator[](0);
		Datum mask = d.u.farr->operator[](1);

		g_lingo->func_cursor(sprite.asInt(), mask.asInt());
	} else {
		g_lingo->func_cursor(d.asInt(), -1);
	}
}

void LB::b_put(int nargs) {
	// Prints a statement to the Message window
	Common::String output;
	for (int i = nargs - 1; i >= 0; i--) {
		output += g_lingo->peek(i).asString();
		if (i > 0)
			output += " ";
	}
	debug("-- %s", output.c_str());
	g_lingo->dropStack(nargs);
}

void LB::b_showGlobals(int nargs) {
	warning("STUB: b_showGlobals");
}

void LB::b_showLocals(int nargs) {
	warning("STUB: b_showLocals");
}

///////////////////
// Score
///////////////////
void LB::b_constrainH(int nargs) {
	Datum num = g_lingo->pop();
	Datum sprite = g_lingo->pop();

	warning("STUB: b_constrainH(%d, %d)", sprite.asInt(), num.asInt());

	g_lingo->push(Datum(0));
}

void LB::b_constrainV(int nargs) {
	Datum num = g_lingo->pop();
	Datum sprite = g_lingo->pop();

	warning("STUB: b_constrainV(%d, %d)", sprite.asInt(), num.asInt());

	g_lingo->push(Datum(0));
}

void LB::b_copyToClipBoard(int nargs) {
	g_lingo->printSTUBWithArglist("b_copyToClipBoard", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_duplicate(int nargs) {
	g_lingo->printSTUBWithArglist("b_duplicate", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_editableText(int nargs) {
	Frame *frame = g_director->getCurrentScore()->_frames[g_director->getCurrentScore()->getCurrentFrame()];

	if (g_lingo->_currentChannelId == -1) {
		warning("b_editableText: channel Id is missing");
		return;
	}
	frame->_sprites[g_lingo->_currentChannelId]->_editable = true;
}

void LB::b_erase(int nargs) {
	g_lingo->printSTUBWithArglist("b_erase", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_findEmpty(int nargs) {
	g_lingo->printSTUBWithArglist("b_findEmpty", nargs);

	g_lingo->dropStack(nargs);

	g_lingo->push(Datum(0));
}

void LB::b_importFileInto(int nargs) {
	g_lingo->printSTUBWithArglist("b_importFileInto", nargs);

	g_lingo->dropStack(nargs);
}

void menuCommandsCallback(int action, Common::String &text, void *data) {
	Common::String name = Common::String::format("scummvmMenu%d", action);

	LC::call(name, 0);
}

void LB::b_installMenu(int nargs) {
	// installMenu castNum
	Datum d = g_lingo->pop();

	int castId = d.asInt();

	if (g_director->getVersion() < 4)
		castId += g_director->getCurrentScore()->_castIDoffset;

	const Stxt *stxt = g_director->getCurrentScore()->_loadedStxts->getVal(castId, nullptr);

	if (!stxt) {
		warning("installMenu: Unknown cast number #%d", castId);
		return;
	}

	Common::String menuStxt = g_lingo->codePreprocessor(stxt->_ptext.c_str(), kNoneScript, castId, true);
	Common::String line;
	int linenum = -1; // We increment it before processing

	Graphics::MacMenu *menu = g_director->_wm->addMenu();
	int submenu = -1;
	Common::String submenuText;
	Common::String command;
	int commandId = 100;

	Common::String handlers;

	menu->setCommandsCallback(menuCommandsCallback, g_director);

	debugC(3, kDebugLingoExec, "installMenu: '%s'", Common::toPrintable(menuStxt).c_str());

	for (const byte *s = (const byte *)menuStxt.c_str(); *s; s++) {
		// Get next line
		line.clear();
		while (*s && *s != '\n') { // If we see a whitespace
			if (*s == (byte)'\xc2') {
				s++;
				if (*s == '\n') {
					line += ' ';

					s++;
				}
			} else {
				line += *s++;
			}
		}

		linenum++;

		if (line.empty())
			continue;

		if (line.hasPrefixIgnoreCase("menu:")) {
			const char *p = &line.c_str()[5];

			while (*p && (*p == ' ' || *p == '\t'))
				p++;

			if (!submenuText.empty()) { // Adding submenu for previous menu
				if (!command.empty()) {
					handlers += g_lingo->genMenuHandler(&commandId, command);
					submenuText += Common::String::format("[%d]", commandId);
				}

				menu->createSubMenuFromString(submenu, submenuText.c_str(), 0);
			}

			if (!strcmp(p, "@"))
				p = "\xf0";	// Apple symbol

			submenu = menu->addMenuItem(nullptr, Common::String(p));

			submenuText.clear();

			continue;
		}

		// We have either '=' or \xc5 as a separator
		const char *p = strchr(line.c_str(), '=');

		if (!p)
			p = strchr(line.c_str(), '\xc5');

		Common::String text;

		if (p) {
			text = Common::String(line.c_str(), p);
			command = Common::String(p + 1);
		} else {
			text = line;
			command = "";
		}

		text.trim();
		command.trim();

		if (!submenuText.empty()) {
			if (!command.empty()) {
				handlers += g_lingo->genMenuHandler(&commandId, command);
				submenuText += Common::String::format("[%d];", commandId);
			} else {
				submenuText += ';';
			}
		}

		submenuText += text;

		if (!*s) // if we reached end of string, do not increment it but break
			break;
	}

	if (!submenuText.empty()) {
		if (!command.empty()) {
			handlers += g_lingo->genMenuHandler(&commandId, command);
			submenuText += Common::String::format("[%d]", commandId);
		}
		menu->createSubMenuFromString(submenu, submenuText.c_str(), 0);
	}

	g_lingo->addCode(handlers.c_str(), kMovieScript, 1337);
}

Common::String Lingo::genMenuHandler(int *commandId, Common::String &command) {
	Common::String name;

	do {
		(*commandId)++;

		name = Common::String::format("scummvmMenu%d", *commandId);
	} while (getHandler(name).type != VOID);

	return Common::String::format("on %s\n  %s\nend %s\n\n", name.c_str(), command.c_str(), name.c_str());
}

void LB::b_label(int nargs) {
	Datum d = g_lingo->pop();
	warning("STUB: b_label(%d)", d.asInt());

	g_lingo->push(Datum(0));
}

void LB::b_marker(int nargs) {
	Datum d = g_lingo->pop();
	int marker = g_lingo->func_marker(d.asInt());
	g_lingo->push(marker);
}

void LB::b_move(int nargs) {
	g_lingo->printSTUBWithArglist("b_move", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_moveableSprite(int nargs) {
	Frame *frame = g_director->getCurrentScore()->_frames[g_director->getCurrentScore()->getCurrentFrame()];

	if (g_lingo->_currentChannelId == -1) {
		warning("b_moveableSprite: channel Id is missing");
		assert(0);
		return;
	}

	frame->_sprites[g_lingo->_currentChannelId]->_moveable = true;
}

void LB::b_pasteClipBoardInto(int nargs) {
	g_lingo->printSTUBWithArglist("b_pasteClipBoardInto", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_puppetPalette(int nargs) {
	g_lingo->convertVOIDtoString(0, nargs);

	g_lingo->printSTUBWithArglist("b_puppetPalette", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_puppetSound(int nargs) {
	ARGNUMCHECK(1);

	DirectorSound *sound = g_director->getSoundManager();
	Datum castMember = g_lingo->pop();
	Score *score = g_director->getCurrentScore();

	if (!score) {
		warning("b_puppetSound(): no score");
		return;
	}

	int castId = g_lingo->castIdFetch(castMember);
	sound->playCastMember(castId, 1);
}

void LB::b_puppetSprite(int nargs) {
	if (!g_director->getCurrentScore()) {
		warning("b_puppetSprite: no score");

		return;
	}

	Frame *frame = g_director->getCurrentScore()->_frames[g_director->getCurrentScore()->getCurrentFrame()];

	if (g_lingo->_currentChannelId == -1) {
		warning("b_puppetSprite: channel Id is missing");
		return;
	}
	frame->_sprites[g_lingo->_currentChannelId]->_puppet = true;
}

void LB::b_puppetTempo(int nargs) {
	Datum d = g_lingo->pop();
	warning("STUB: b_puppetTempo(%d)", d.asInt());
}

void LB::b_puppetTransition(int nargs) {
	g_lingo->printSTUBWithArglist("b_puppetTransition", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_ramNeeded(int nargs) {
	Datum d = g_lingo->pop();
	warning("STUB: b_ramNeeded(%d)", d.u.i);

	g_lingo->push(Datum(0));
}

void LB::b_rollOver(int nargs) {
	Datum d = g_lingo->pop();
	Datum res(0);
	int arg = d.asInt();

	Score *score = g_director->getCurrentScore();

	if (!score) {
		warning("b_rollOver: Reference to an empty score");
		return;
	}

	if (arg >= (int32) score->_sprites.size()) {
		g_lingo->push(res);
		return;
	}

	Common::Point pos = g_system->getEventManager()->getMousePos();

	if (score->checkSpriteIntersection(arg, pos))
		res.u.i = 1; // TRUE

	g_lingo->push(res);
}

void LB::b_spriteBox(int nargs) {
	g_lingo->printSTUBWithArglist("b_spriteBox", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_unLoad(int nargs) {
	g_lingo->printSTUBWithArglist("b_unLoad", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_unLoadCast(int nargs) {
	g_lingo->printSTUBWithArglist("b_unLoadCast", nargs);

	g_lingo->dropStack(nargs);
}

void LB::b_zoomBox(int nargs) {
	// zoomBox startSprite, endSprite [, delatTicks]
	//   ticks are in 1/60th, default 1
	if (nargs < 2 || nargs > 3) {
		warning("b_zoomBox: expected 2 or 3 arguments, got %d", nargs);

		g_lingo->dropStack(nargs);

		return;
	}

	int delayTicks = 1;
	if (nargs > 2) {
		Datum d = g_lingo->pop();
		delayTicks = d.asInt();
	}

	int endSprite = g_lingo->pop().asInt();
	int startSprite = g_lingo->pop().asInt();

	Score *score = g_director->getCurrentScore();
	uint16 curFrame = score->getCurrentFrame();

	Common::Rect *startRect = score->getSpriteRect(startSprite);
	if (!startRect) {
		warning("b_zoomBox: unknown start sprite #%d", startSprite);
		return;
	}

	// Looks for endSprite in the current frame, otherwise
	// Looks for endSprite in the next frame
	Common::Rect *endRect = score->getSpriteRect(endSprite);
	if (!endRect) {
		if ((uint)curFrame + 1 < score->_frames.size())
			endRect = &score->_frames[curFrame + 1]->_sprites[endSprite]->_currentBbox;
	}

	if (!endRect) {
		warning("b_zoomBox: unknown end sprite #%d", endSprite);
		return;
	}

	ZoomBox *box = new ZoomBox;
	box->start = *startRect;
	box->end = *endRect;
	box->delay = delayTicks;
	box->step = 0;
	box->startTime = g_system->getMillis();
	box->nextTime  = g_system->getMillis() + 1000 * box->step / 60;

	score->addZoomBox(box);
}

void LB::b_updateStage(int nargs) {
	ARGNUMCHECK(0);

	if (g_director->getGameGID() == GID_TEST) {
		warning("b_updateStage: Skipping due to tests");

		return;
	}

	Score *score = g_director->getCurrentScore();

	if (!score) {
		warning("b_updateStage: no score");

		return;
	}

	score->renderFrame(score->getCurrentFrame(), false, true);
	g_director->processEvents(true);

	if (debugChannelSet(-1, kDebugFewFramesOnly)) {
		score->_framesRan++;

		if (score->_framesRan > 9) {
			warning("b_updateStage(): exiting due to debug few frames only");
			score->_stopPlay = true;
		}
	}
}


///////////////////
// Window
///////////////////

void LB::b_close(int nargs) {
	g_lingo->printSTUBWithArglist("b_close", nargs);
	g_lingo->dropStack(nargs);
}

void LB::b_forget(int nargs) {
	g_lingo->printSTUBWithArglist("b_forget", nargs);
	g_lingo->dropStack(nargs);
}

void LB::b_inflate(int nargs) {
	g_lingo->printSTUBWithArglist("b_inflate", nargs);
	g_lingo->dropStack(nargs);
}

void LB::b_moveToBack(int nargs) {
	g_lingo->printSTUBWithArglist("b_moveToBack", nargs);
	g_lingo->dropStack(nargs);
}

void LB::b_moveToFront(int nargs) {
	g_lingo->printSTUBWithArglist("b_moveToFront", nargs);
	g_lingo->dropStack(nargs);
}


///////////////////
// Point
///////////////////
void LB::b_point(int nargs) {
	Datum y(g_lingo->pop().asFloat());
	Datum x(g_lingo->pop().asFloat());
	Datum d;

	d.u.farr = new DatumArray;

	d.u.farr->push_back(x);
	d.u.farr->push_back(y);
	d.type = POINT;

	g_lingo->push(d);
}

void LB::b_rect(int nargs) {
	g_lingo->printSTUBWithArglist("b_rect", nargs);

	g_lingo->dropStack(nargs);

	g_lingo->push(Datum(0));
}


void LB::b_intersect(int nargs) {
	g_lingo->printSTUBWithArglist("b_intersect", nargs);

	g_lingo->dropStack(nargs);

	g_lingo->push(Datum(0));
}

void LB::b_inside(int nargs) {
	g_lingo->printSTUBWithArglist("b_inside", nargs);

	g_lingo->dropStack(nargs);

	g_lingo->push(Datum(0));
}

void LB::b_map(int nargs) {
	g_lingo->printSTUBWithArglist("b_map", nargs);

	g_lingo->dropStack(nargs);

	g_lingo->push(Datum(0));
}

void LB::b_offsetRect(int nargs) {
	g_lingo->printSTUBWithArglist("b_offsetRect", nargs);

	g_lingo->dropStack(nargs);

	g_lingo->push(Datum(0));
}

void LB::b_union(int nargs) {
	g_lingo->printSTUBWithArglist("b_union", nargs);

	g_lingo->dropStack(nargs);

	g_lingo->push(Datum(0));
}


///////////////////
// Sound
///////////////////
void LB::b_beep(int nargs) {
	int repeat = 1;
	if (nargs == 1) {
		Datum d = g_lingo->pop();
		repeat = d.u.i;
	}
	g_lingo->func_beep(repeat);
}

void LB::b_mci(int nargs) {
	Datum d = g_lingo->pop();

	g_lingo->func_mci(d.asString());
}

void LB::b_mciwait(int nargs) {
	Datum d = g_lingo->pop();

	g_lingo->func_mciwait(d.asString());
}

void LB::b_sound(int nargs) {
	// Builtin function for sound as used by the Director bytecode engine.
	//
	// Accepted arguments:
	// "close", INT soundChannel
	// "fadeIn", INT soundChannel(, INT ticks)
	// "fadeOut", INT soundChannel(, INT ticks)
	// "playFile", INT soundChannel, STRING fileName
	// "stop", INT soundChannel

	if (nargs < 2 || nargs > 3) {
		warning("b_sound: expected 2 or 3 args, not %d", nargs);
		g_lingo->dropStack(nargs);

		return;
	}

	Datum secondArg = g_lingo->pop();
	Datum firstArg = g_lingo->pop();
	Datum verb;
	if (nargs > 2) {
		verb = g_lingo->pop();
	} else {
		verb = firstArg;
		firstArg = secondArg;
	}

	if (verb.type != STRING && verb.type != SYMBOL) {
		warning("b_sound: verb arg should be of type STRING, not %s", verb.type2str());
		return;
	}

	if (verb.u.s->equalsIgnoreCase("close") || verb.u.s->equalsIgnoreCase("stop")) {
		if (nargs != 2) {
			warning("sound %s: expected 1 argument, got %d", verb.u.s->c_str(), nargs - 1);
			return;
		}

		TYPECHECK(firstArg, INT);

		g_director->getSoundManager()->stopSound(firstArg.u.i);
	} else if (verb.u.s->equalsIgnoreCase("fadeIn")) {
		warning("STUB: sound fadeIn");
		return;
	} else if (verb.u.s->equalsIgnoreCase("fadeOut")) {
		warning("STUB: sound fadeOut");
		return;
	} else if (verb.u.s->equalsIgnoreCase("playFile")) {
		ARGNUMCHECK(3)

		TYPECHECK(firstArg, INT);
		TYPECHECK(secondArg, STRING);

		g_director->getSoundManager()->playFile(pathMakeRelative(*secondArg.u.s), firstArg.u.i);
	} else {
		warning("b_sound: unknown verb %s", verb.u.s->c_str());
	}
}

void LB::b_soundBusy(int nargs) {
	ARGNUMCHECK(1);

	DirectorSound *sound = g_director->getSoundManager();
	Datum whichChannel = g_lingo->pop();

	TYPECHECK(whichChannel, INT);

	bool isBusy = sound->isChannelActive(whichChannel.u.i);
	Datum result;
	result.type = INT;
	result.u.i = isBusy ? 1 : 0;
	g_lingo->push(result);
}

///////////////////
// Constants
///////////////////
void LB::b_backspace(int nargs) {
	g_lingo->push(Datum(Common::String("\b")));
}

void LB::b_empty(int nargs) {
	g_lingo->push(Datum(Common::String("")));
}

void LB::b_enter(int nargs) {
	g_lingo->push(Datum(Common::String("\n")));
}

void LB::b_false(int nargs) {
	g_lingo->push(Datum(0));
}

void LB::b_quote(int nargs) {
	g_lingo->push(Datum(Common::String("\"")));
}

void LB::b_returnconst(int nargs) {
	g_lingo->push(Datum(Common::String("\n")));
}

void LB::b_tab(int nargs) {
	g_lingo->push(Datum(Common::String("\t")));
}

void LB::b_true(int nargs) {
	g_lingo->push(Datum(1));
}

void LB::b_version(int nargs) {
	switch (g_director->getVersion()) {
	case 3:
		g_lingo->push(Datum(Common::String("3.1.1"))); // Mac
		break;
	case 4:
		g_lingo->push(Datum(Common::String("4.0"))); // Mac
		break;
	default:
		error("Unsupported Director for 'version'");
		break;
	}
}

///////////////////
// References
///////////////////
void LB::b_cast(int nargs) {
	Datum d = g_lingo->pop();

	warning("STUB: b_cast");

	Datum res(0);
	res.type = REFERENCE;
	g_lingo->push(res);
}

void LB::b_field(int nargs) {
	Datum d = g_lingo->pop();

	Datum res(g_lingo->castIdFetch(d));
	res.type = REFERENCE;
	g_lingo->push(res);
}

void LB::b_script(int nargs) {
	Datum d = g_lingo->pop();

	warning("STUB: b_script");

	Datum res(0);
	d.type = REFERENCE;
	g_lingo->push(res);
}

void LB::b_window(int nargs) {
	Datum d = g_lingo->pop();

	warning("STUB: b_window");

	Datum res(0);
	res.type = REFERENCE;
	g_lingo->push(res);
}

void LB::b_numberofchars(int nargs) {
	Datum d = g_lingo->pop();

	int len = strlen(d.asString().c_str());

	Datum res(len);
	g_lingo->push(res);
}

void LB::b_numberofitems(int nargs) {
	Datum d = g_lingo->pop();

	int numberofitems = 1;
	Common::String contents = d.asString();
	for (uint32 i = 0;  i < contents.size(); i++) {
		if (contents[i] == ',')
			numberofitems++;
	}

	Datum res(numberofitems);
	g_lingo->push(res);
}

void LB::b_numberoflines(int nargs) {
	Datum d = g_lingo->pop();

	int numberoflines = 1;
	Common::String contents = d.asString();
	for (uint32 i = 0; i < contents.size(); i++) {
		if (contents[i] == '\n')
			numberoflines++;
	}

	Datum res(numberoflines);
	g_lingo->push(res);
}

void LB::b_numberofwords(int nargs) {
	Datum d = g_lingo->pop();

	int numberofwords = 0;
	Common::String contents = d.asString();
	if (contents.empty()) {
		g_lingo->push(Datum(0));
		return;
	}
	for (uint32 i = 1; i < contents.size(); i++) {
		if (Common::isSpace(contents[i]) && !Common::isSpace(contents[i - 1]))
			numberofwords++;
	}
	// Count the last word
	if (!Common::isSpace(contents[contents.size() - 1]))
		numberofwords++;

	Datum res(numberofwords);
	g_lingo->push(res);
}

void LB::b_lastcharof(int nargs) {
	Datum d = g_lingo->pop();

	warning("STUB: b_lastcharof");

	g_lingo->push(Datum(0));
}

void LB::b_lastitemof(int nargs) {
	Datum d = g_lingo->pop();

	warning("STUB: b_lastitemof");

	g_lingo->push(Datum(0));
}

void LB::b_lastlineof(int nargs) {
	Datum d = g_lingo->pop();

	warning("STUB: b_lastlineof");

	g_lingo->push(Datum(0));
}

void LB::b_lastwordof(int nargs) {
	Datum d = g_lingo->pop();

	warning("STUB: b_lastwordof");

	g_lingo->push(Datum(0));
}

} // End of namespace Director