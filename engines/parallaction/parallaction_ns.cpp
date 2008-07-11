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

#include "common/system.h"

#include "common/config-manager.h"

#include "parallaction/parallaction.h"
#include "parallaction/input.h"
#include "parallaction/sound.h"


namespace Parallaction {

#define MOUSEARROW_WIDTH		16
#define MOUSEARROW_HEIGHT		16

#define MOUSECOMBO_WIDTH		32	// sizes for cursor + selected inventory item
#define MOUSECOMBO_HEIGHT		32

LocationName::LocationName() {
	_buf = 0;
	_hasSlide = false;
	_hasCharacter = false;
}

LocationName::~LocationName() {
	free(_buf);
}


/*
	bind accept the following input formats:

    1 - [S].slide.[L]{.[C]}
	2 - [L]{.[C]}

    where:

	[S] is the slide to be shown
    [L] is the location to switch to (immediately in case 2, or right after slide [S] in case 1)
    [C] is the character to be selected, and is optional

    The routine tells one form from the other by searching for the '.slide.'

    NOTE: there exists one script in which [L] is not used in the case 1, but its use
		  is commented out, and would definitely crash the current implementation.
*/
void LocationName::bind(const char *s) {

	free(_buf);

	_buf = strdup(s);
	_hasSlide = false;
	_hasCharacter = false;

	Common::StringList list;
	char *tok = strtok(_buf, ".");
	while (tok) {
		list.push_back(tok);
		tok = strtok(NULL, ".");
	}

	if (list.size() < 1 || list.size() > 4)
		error("changeLocation: ill-formed location name '%s'", s);

	if (list.size() > 1) {
		if (list[1] == "slide") {
			_hasSlide = true;
			_slide = list[0];

			list.remove_at(0);		// removes slide name
			list.remove_at(0);		// removes 'slide'
		}

		if (list.size() == 2) {
			_hasCharacter = true;
			_character = list[1];
		}
	}

	_location = list[0];

	strcpy(_buf, s);		// kept as reference
}



int Parallaction_ns::init() {

	_screenWidth = 320;
	_screenHeight = 200;

	if (getPlatform() == Common::kPlatformPC) {
		_disk = new DosDisk_ns(this);
	} else {
		if (getFeatures() & GF_DEMO) {
			strcpy(_location._name, "fognedemo");
		}
		_disk = new AmigaDisk_ns(this);
		_disk->selectArchive((getFeatures() & GF_DEMO) ? "disk0" : "disk1");
	}

	if (getPlatform() == Common::kPlatformPC) {
		int midiDriver = MidiDriver::detectMusicDriver(MDT_MIDI | MDT_ADLIB | MDT_PREFER_MIDI);
		MidiDriver *driver = MidiDriver::createMidi(midiDriver);
		_soundMan = new DosSoundMan(this, driver);
		_soundMan->setMusicVolume(ConfMan.getInt("music_volume"));
	} else {
		_soundMan = new AmigaSoundMan(this);
	}

	initResources();
	initFonts();
	initCursors();
	_locationParser = new LocationParser_ns(this);
	_locationParser->init();
	_programParser = new ProgramParser_ns(this);
	_programParser->init();

	_cmdExec = new CommandExec_ns(this);
	_cmdExec->init();
	_programExec = new ProgramExec_ns(this);
	_programExec->init();

	_introSarcData1 = 0;
	_introSarcData2 = 1;
	_introSarcData3 = 200;

	num_foglie = 0;

	_inTestResult = false;

	_location._animations.push_front(_char._ani);

	Parallaction::init();

	return 0;
}

Parallaction_ns::~Parallaction_ns() {
	freeFonts();

	delete _locationParser;
	delete _programParser;
	delete _mouseComposedArrow;

	_location._animations.remove(_char._ani);

}


void Parallaction_ns::freeFonts() {

	delete _dialogueFont;
	delete _labelFont;
	delete _menuFont;
	delete _introFont;

}

void Parallaction_ns::initCursors() {
	_mouseComposedArrow = _disk->loadPointer("pointer");
	_mouseArrow = _resMouseArrow;
}

void Parallaction_ns::setArrowCursor() {

	debugC(1, kDebugInput, "setting mouse cursor to arrow");

	// this stuff is needed to avoid artifacts with labels and selected items when switching cursors
	_gfx->hideFloatingLabel();
	_input->_activeItem._id = 0;

	_system->setMouseCursor(_mouseArrow, MOUSEARROW_WIDTH, MOUSEARROW_HEIGHT, 0, 0, 0);
	_system->showMouse(true);

}

void Parallaction_ns::setInventoryCursor(int pos) {

	if (pos == -1)
		return;

	const InventoryItem *item = getInventoryItem(pos);
	if (item->_index == 0)
		return;

	_input->_activeItem._id = item->_id;

	byte *v8 = _mouseComposedArrow->getData(0);

	// FIXME: destination offseting is not clear
	byte* s = _char._objs->getData(item->_index);
	byte* d = v8 + 7 + MOUSECOMBO_WIDTH * 7;

	for (uint i = 0; i < INVENTORYITEM_HEIGHT; i++) {
		memcpy(d, s, INVENTORYITEM_WIDTH);

		s += INVENTORYITEM_PITCH;
		d += MOUSECOMBO_WIDTH;
	}

	_system->setMouseCursor(v8, MOUSECOMBO_WIDTH, MOUSECOMBO_HEIGHT, 0, 0, 0);

}


void Parallaction_ns::callFunction(uint index, void* parm) {
	assert(index < 25);	// magic value 25 is maximum # of callables for Nippon Safes

	(this->*_callables[index])(parm);
}


int Parallaction_ns::go() {
	renameOldSavefiles();

	_globalTable = _disk->loadTable("global");

	guiStart();

	if (_engineFlags & kEngineQuit)
		return 0;

	changeLocation(_location._name);

	if (_engineFlags & kEngineQuit)
		return 0;

	_input->_inputMode = Input::kInputModeGame;
	while ((_engineFlags & kEngineQuit) == 0) {
		runGame();
	}

	return 0;
}

void Parallaction_ns::switchBackground(const char* background, const char* mask) {
//	printf("switchBackground(%s)", name);

	Palette pal;

	uint16 v2 = 0;
	if (!scumm_stricmp(background, "final")) {
		_gfx->clearScreen();
		for (uint16 _si = 0; _si < 32; _si++) {
			pal.setEntry(_si, v2, v2, v2);
			v2 += 4;
		}

		g_system->delayMillis(20);
		_gfx->setPalette(pal);
		_gfx->updateScreen();
	}

	setBackground(background, mask, mask);

	return;
}


void Parallaction_ns::showSlide(const char *name) {
	_gfx->setBackground(kBackgroundSlide, name, 0, 0);
}

void Parallaction_ns::runPendingZones() {
	if (_activeZone) {
		ZonePtr z = _activeZone;	// speak Zone or sound
		_activeZone = nullZonePtr;
		runZone(z);
	}
}

//	changeLocation handles transitions between locations, and is able to display slides
//	between one and the other.
//
void Parallaction_ns::changeLocation(char *location) {
	debugC(1, kDebugExec, "changeLocation(%s)", location);

	_soundMan->playLocationMusic(location);

	_gfx->hideFloatingLabel();
	_gfx->freeLabels();

	_input->stopHovering();
	if (_engineFlags & kEngineBlockInput) {
		setArrowCursor();
	}

	_gfx->showGfxObj(_char._ani->gfxobj, false);
	_location._animations.remove(_char._ani);

	freeLocation();

	LocationName locname;
	locname.bind(location);

	if (locname.hasSlide()) {
		showSlide(locname.slide());
		uint id = _gfx->createLabel(_menuFont, _location._slideText[0], 1);
		_gfx->showLabel(id, CENTER_LABEL_HORIZONTAL, 14);
		_input->waitUntilLeftClick();
		_gfx->freeLabels();
		freeBackground();
	}

	if (locname.hasCharacter()) {
		changeCharacter(locname.character());
	}

	_location._animations.push_front(_char._ani);
	_gfx->showGfxObj(_char._ani->gfxobj, true);

	strcpy(_saveData1, locname.location());
	parseLocation(_saveData1);

	_char._ani->_oldPos.x = -1000;
	_char._ani->_oldPos.y = -1000;

	_char._ani->field_50 = 0;
	if (_location._startPosition.x != -1000) {
		_char._ani->_left = _location._startPosition.x;
		_char._ani->_top = _location._startPosition.y;
		_char._ani->_frame = _location._startFrame;
		_location._startPosition.y = -1000;
		_location._startPosition.x = -1000;
	}


	_gfx->setBlackPalette();
	_gfx->updateScreen();

	// BUG #1837503: kEngineChangeLocation flag must be cleared *before* commands
	// and acommands are executed, so that it can be set again if needed.
	_engineFlags &= ~kEngineChangeLocation;

	_cmdExec->run(_location._commands);

	doLocationEnterTransition();

	_cmdExec->run(_location._aCommands);

	if (_location._hasSound)
		_soundMan->playSfx(_location._soundFile, 0, true);

	debugC(1, kDebugExec, "changeLocation() done");

	return;

}


void Parallaction_ns::parseLocation(const char *filename) {
	debugC(1, kDebugParser, "parseLocation('%s')", filename);

	allocateLocationSlot(filename);
	Script *script = _disk->loadLocation(filename);

	// TODO: the following two lines are specific to Nippon Safes
	// and should be moved into something like 'initializeParsing()'
	_vm->_location._hasSound = false;

	_locationParser->parse(script);

	delete script;

	// this loads animation scripts
	AnimationList::iterator it = _vm->_location._animations.begin();
	for ( ; it != _vm->_location._animations.end(); it++) {
		if ((*it)->_scriptName) {
			loadProgram(*it, (*it)->_scriptName);
		}
	}

	debugC(1, kDebugParser, "parseLocation('%s') done", filename);
	return;
}



void Parallaction_ns::changeCharacter(const char *name) {
	debugC(1, kDebugExec, "changeCharacter(%s)", name);

	_char.setName(name);

	if (!scumm_stricmp(_char.getFullName(), _characterName1)) {
		debugC(3, kDebugExec, "changeCharacter: nothing done");
		return;
	}

	// freeCharacter takes responsibility for checking
	// character for sanity before memory is freed
	freeCharacter();

	Common::String oldArchive = _disk->selectArchive((getFeatures() & GF_DEMO) ? "disk0" : "disk1");
	_char._ani->gfxobj = _gfx->loadAnim(_char.getFullName());
	_char._ani->gfxobj->setFlags(kGfxObjCharacter);

	if (!_char.dummy()) {
		if (getPlatform() == Common::kPlatformAmiga) {
			_disk->selectArchive("disk0");
		}

		_char._head = _disk->loadHead(_char.getBaseName());
		_char._talk = _disk->loadTalk(_char.getBaseName());
		_char._objs = _disk->loadObjects(_char.getBaseName());
		_objectsNames = _disk->loadTable(_char.getBaseName());

		_soundMan->playCharacterMusic(_char.getBaseName());

		// The original engine used to reload 'common' only on loadgames. We are reloading here since 'common'
		// contains character specific stuff. This causes crashes like bug #1816899, because parseLocation tries
		// to reload scripts but the data archive selected is occasionally wrong. This has been solved by having
		// parseLocation only load scripts when they aren't already loaded - which it should have done since the
		// beginning nevertheless.
		if (!(getFeatures() & GF_DEMO))
			parseLocation("common");
	}

	if (!oldArchive.empty())
		_disk->selectArchive(oldArchive);

	strcpy(_characterName1, _char.getFullName());

	debugC(3, kDebugExec, "changeCharacter: switch completed");

	return;
}

void Parallaction_ns::cleanupGame() {

	_engineFlags &= ~kEngineTransformedDonna;

	// this code saves main character animation from being removed from the following code
	_location._animations.remove(_char._ani);
	_numLocations = 0;
	_commandFlags = 0;

	memset(_localFlags, 0, sizeof(_localFlags));
	memset(_locationNames, 0, sizeof(_locationNames));

	// this flag tells freeZones to unconditionally remove *all* Zones
	_engineFlags |= kEngineQuit;

	freeZones();
	freeAnimations();

	// this dangerous flag can now be cleared
	_engineFlags &= ~kEngineQuit;

	// main character animation is restored
	_location._animations.push_front(_char._ani);
	_score = 0;

	return;
}

} // namespace Parallaction
