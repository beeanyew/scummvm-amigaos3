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
 */

#include "common/scummsys.h"

#include "common/file.h"

#include "audio/decoders/wave.h"

#include "zvision/actions.h"
#include "zvision/zvision.h"
#include "zvision/script_manager.h"
#include "zvision/render_manager.h"
#include "zvision/action_node.h"
#include "zvision/zork_raw.h"

namespace ZVision {

//////////////////////////////////////////////////////////////////////////////
// ActionAdd
//////////////////////////////////////////////////////////////////////////////

ActionAdd::ActionAdd(const Common::String &line) {
	sscanf(line.c_str(), "%*[^(](%u,%u)", &_key, &_value);
}

bool ActionAdd::execute(ZVision *engine) {
	engine->getScriptManager()->addToStateValue(_key, _value);
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionAssign
//////////////////////////////////////////////////////////////////////////////

ActionAssign::ActionAssign(const Common::String &line) {
	sscanf(line.c_str(), "%*[^(](%u, %u)", &_key, &_value);
}

bool ActionAssign::execute(ZVision *engine) {
	engine->getScriptManager()->setStateValue(_key, _value);
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionAttenuate
//////////////////////////////////////////////////////////////////////////////

ActionAttenuate::ActionAttenuate(const Common::String &line) {
	sscanf(line.c_str(), "%*[^(](%u, %d)", &_key, &_attenuation);
}

bool ActionAttenuate::execute(ZVision *engine) {
	// TODO: Implement
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionChangeLocation
//////////////////////////////////////////////////////////////////////////////

ActionChangeLocation::ActionChangeLocation(const Common::String &line) {
	sscanf(line.c_str(), "%*[^(](%c,%c,%c%c,%u)", &_world, &_room, &_node, &_view, &_x);
}

bool ActionChangeLocation::execute(ZVision *engine) {
	// TODO: Implement
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionCrossfade
//////////////////////////////////////////////////////////////////////////////

ActionCrossfade::ActionCrossfade(const Common::String &line) {
	sscanf(line.c_str(),
           "%*[^(](%u %u %u %u %u %u %u)",
           &_keyOne, &_keyTwo, &_oneStartVolume, &_twoStartVolume, &_oneEndVolume, &_twoEndVolume, &_timeInMillis);
}

bool ActionCrossfade::execute(ZVision *engine) {
	// TODO: Implement
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionMusic
//////////////////////////////////////////////////////////////////////////////

ActionMusic::ActionMusic(const Common::String &line) : _volume(255) {
	uint type;
	char fileNameBuffer[25];
	uint loop;
	uint volume = 255;

	sscanf(line.c_str(), "%*[^:]:%*[^:]:%u(%u %25s %u %u)", &_key, &type, fileNameBuffer, &loop, &volume);

	// type 4 are midi sound effect files
	if (type == 4) {
		_soundType = Audio::Mixer::kSFXSoundType;
		_fileName = Common::String::format("midi/%s/%u.wav", fileNameBuffer, loop);
		_loop = false;
	} else {
		// TODO: See what the other types are so we can specify the correct Mixer::SoundType. In the meantime use kPlainSoundType
		_soundType = Audio::Mixer::kPlainSoundType;
		_fileName = Common::String(fileNameBuffer);
		_loop = loop == 1 ? true : false;
	}

	// Volume is optional. If it doesn't appear, assume full volume
	if (volume != 255) {
		// Volume in the script files is mapped to [0, 100], but the ScummVM mixer uses [0, 255]
		_volume = volume * 255 / 100;
	}
}

bool ActionMusic::execute(ZVision *engine) {
	Audio::RewindableAudioStream *audioStream;

	if (_fileName.contains(".wav")) {
		Common::File file;
		if (file.open(_fileName)) {
			audioStream = Audio::makeWAVStream(&file, DisposeAfterUse::NO);
		}
	} else {
		audioStream = makeRawZorkStream(_fileName, engine);
	}
	
	if (_loop) {
		Audio::LoopingAudioStream *loopingAudioStream = new Audio::LoopingAudioStream(audioStream, 0, DisposeAfterUse::YES);
		engine->_mixer->playStream(_soundType, 0, loopingAudioStream, -1, _volume);
	} else {
		engine->_mixer->playStream(_soundType, 0, audioStream, -1, _volume);
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionPreloadAnimation
//////////////////////////////////////////////////////////////////////////////

ActionPreloadAnimation::ActionPreloadAnimation(const Common::String &line) {
	char fileName[25];

	// The two %*u are always 0 and dont seem to have a use
	sscanf(line.c_str(), "%*[^:]:%*[^:]:%u(%25s %*u %*u %u %u)", &_key, fileName, &_mask, &_framerate);

	_fileName = Common::String(fileName);
}

bool ActionPreloadAnimation::execute(ZVision *engine) {
	// TODO: Implement
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionPlayAnimation
//////////////////////////////////////////////////////////////////////////////

ActionPlayAnimation::ActionPlayAnimation(const Common::String &line) {
	char fileName[25];
	uint loop;

	// The two %*u are always 0 and dont seem to have a use
	sscanf(line.c_str(),
           "%*[^:]:%*[^:]:%u(%25s %u %u %u %u %u %u %u %*u %*u %u %u)",
           &_key, fileName, &_x, &_y, &_width, &_height, &_start, &_end, &loop, &_mask, &_framerate);

	_fileName = Common::String(fileName);
	_loop = loop == 1 ? true : false;
}

bool ActionPlayAnimation::execute(ZVision *engine) {
	// TODO: Implement
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionQuit
//////////////////////////////////////////////////////////////////////////////

bool ActionQuit::execute(ZVision *engine) {
	engine->quitGame();

	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionRandom
//////////////////////////////////////////////////////////////////////////////

ActionRandom::ActionRandom(const Common::String &line) {
	sscanf(line.c_str(), "%*[^:]:%*[^:]:%u, %u)", &_key, &_max);
}

bool ActionRandom::execute(ZVision *engine) {
	uint randNumber = engine->getRandomSource()->getRandomNumber(_max);
	engine->getScriptManager()->setStateValue(_key, randNumber);
	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionSetScreen
//////////////////////////////////////////////////////////////////////////////

ActionSetScreen::ActionSetScreen(const Common::String &line) {
	char fileName[25];
	sscanf(line.c_str(), "%*[^(](%25[^)])", fileName);

	_fileName = Common::String(fileName);
}

bool ActionSetScreen::execute(ZVision *engine) {
	RenderManager *renderManager = engine->getRenderManager();
	renderManager->setBackgroundImage(_fileName);

	return true;
}


//////////////////////////////////////////////////////////////////////////////
// ActionTimer
//////////////////////////////////////////////////////////////////////////////

ActionTimer::ActionTimer(const Common::String &line) {
	sscanf(line.c_str(), "%*[^:]:%*[^:]:%u(%u)", &_key, &_time);
}

bool ActionTimer::execute(ZVision *engine) {
	engine->getScriptManager()->addActionNode(Common::SharedPtr<ActionNode>(new NodeTimer(_key, _time)));
	return true;
}

} // End of namespace ZVision
