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

#include "common/file.h"
#include "common/substream.h"

#include "audio/decoders/wave.h"
#include "audio/decoders/raw.h"
#include "audio/mixer.h"
#include "audio/softsynth/pcspk.h"
#include "audio/decoders/aiff.h"

#include "director/director.h"
#include "director/cast.h"
#include "director/sound.h"

namespace Director {

DirectorSound::DirectorSound(DirectorEngine *vm) : _vm(vm) {
	uint numChannels = 2;
	if (g_director->getVersion() >= 4) {
		numChannels = 4;
	}

	for (uint i = 0; i < numChannels; i++) {
		_channels.push_back(SoundChannel());
	}

	_scriptSound = new Audio::SoundHandle();
	_mixer = g_system->getMixer();

	_speaker = new Audio::PCSpeaker();
	_pcSpeakerHandle = new Audio::SoundHandle();
	_mixer->playStream(Audio::Mixer::kSFXSoundType,
		_pcSpeakerHandle, _speaker, -1, 50, 0, DisposeAfterUse::NO, true);
}

DirectorSound::~DirectorSound() {
	delete _scriptSound;
}

SoundChannel *DirectorSound::getChannel(uint8 soundChannel) {
	if (soundChannel == 0 || soundChannel > _channels.size()) {
		warning("Invalid sound channel %d", soundChannel);
		return nullptr;
	}
	return &_channels[soundChannel - 1];
}

void DirectorSound::playFile(Common::String filename, uint8 soundChannel) {
	if (debugChannelSet(-1, kDebugFast))
		return;

	Common::File *file = new Common::File();

	if (!file->open(filename)) {
		warning("Failed to open %s", filename.c_str());

		delete file;

		return;
	}

	uint32 magic1 = file->readUint32BE();
	file->readUint32BE();
	uint32 magic2 = file->readUint32BE();
	delete file;

	if (magic1 == MKTAG('R', 'I', 'F', 'F') &&
		magic2 == MKTAG('W', 'A', 'V', 'E')) {
		playWAV(filename, soundChannel);
	} else if (magic1 == MKTAG('F', 'O', 'R', 'M') &&
				magic2 == MKTAG('A', 'I', 'F', 'F')) {
		playAIFF(filename, soundChannel);
	} else {
		warning("Unknown file type for %s", filename.c_str());
	}
}

void DirectorSound::playWAV(Common::String filename, uint8 soundChannel) {
	if (soundChannel == 0 || soundChannel > _channels.size()) {
		warning("Invalid sound channel %d", soundChannel);

		return;
	}
	_channels[soundChannel - 1].lastPlayingCast = 0;

	Common::File *file = new Common::File();

	if (!file->open(filename)) {
		warning("Failed to open %s", filename.c_str());

		delete file;

		return;
	}

	Audio::RewindableAudioStream *sound = Audio::makeWAVStream(file, DisposeAfterUse::YES);

	_mixer->stopHandle(_channels[soundChannel - 1].handle);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_channels[soundChannel - 1].handle, sound, -1, _channels[soundChannel - 1].volume);
}

void DirectorSound::playAIFF(Common::String filename, uint8 soundChannel) {
	if (soundChannel == 0 || soundChannel > _channels.size()) {
		warning("Invalid sound channel %d", soundChannel);
		return;
	}
	_channels[soundChannel - 1].lastPlayingCast = 0;

	Common::File *file = new Common::File();

	if (!file->open(filename)) {
		warning("Failed to open %s", filename.c_str());
		delete file;
		return;
	}

	Audio::RewindableAudioStream *sound = Audio::makeAIFFStream(file, DisposeAfterUse::YES);

	_mixer->stopHandle(_channels[soundChannel - 1].handle);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_channels[soundChannel - 1].handle, sound, -1, _channels[soundChannel - 1].volume);
}

void DirectorSound::playMCI(Audio::AudioStream &stream, uint32 from, uint32 to) {
	Audio::SeekableAudioStream *seekStream = static_cast<Audio::SeekableAudioStream *>(&stream);
	Audio::SubSeekableAudioStream *subSeekStream = new Audio::SubSeekableAudioStream(seekStream, Audio::Timestamp(from, seekStream->getRate()), Audio::Timestamp(to, seekStream->getRate()));

	_mixer->stopHandle(*_scriptSound);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, _scriptSound, subSeekStream);
}

void DirectorSound::playStream(Audio::AudioStream &stream, uint8 soundChannel) {
	if (soundChannel == 0 || soundChannel > _channels.size()) {
		warning("Invalid sound channel %d", soundChannel);
		return;
	}

	_mixer->stopHandle(_channels[soundChannel - 1].handle);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_channels[soundChannel - 1].handle, &stream, -1, _channels[soundChannel - 1].volume);
}

void DirectorSound::playCastMember(int castId, uint8 soundChannel, bool allowRepeat) {
	if (castId == 0) {
		stopSound(soundChannel);
	} else {
		Cast *soundCast = _vm->getCastMember(castId);
		if (soundCast) {
			if (soundCast->_type != kCastSound) {
				warning("DirectorSound::playCastMember: attempted to play a non-SoundCast cast member %d", castId);
			} else {
				if (!allowRepeat && lastPlayingCast(soundChannel) == castId)
					return;
				bool looping = ((SoundCast *)soundCast)->_looping;
				SNDDecoder *sd = ((SoundCast *)soundCast)->_audio;
				if (!sd) {
					warning("DirectorSound::playCastMember: no audio data attached to cast member %d", castId);
					return;
				}
				Audio::AudioStream *as;
				if (looping)
					as = sd->getLoopingAudioStream();
				else
					as = sd->getAudioStream();
				if (!as) {
					warning("DirectorSound::playCastMember: audio data failed to load from cast");
					return;
				}
				playStream(*as, soundChannel);
				_channels[soundChannel - 1].lastPlayingCast = castId;
			}
		} else {
			warning("DirectorSound::playCastMember: couldn't find cast member %d", castId);
		}
	}
}

bool DirectorSound::isChannelActive(uint8 soundChannel) {
	if (soundChannel == 0 || soundChannel > _channels.size()) {
		warning("Invalid sound channel %d", soundChannel);
	return false;
}

	return _mixer->isSoundHandleActive(_channels[soundChannel - 1].handle);
}

int DirectorSound::lastPlayingCast(uint8 soundChannel) {
	if (soundChannel == 0 || soundChannel > _channels.size()) {
		warning("Invalid sound channel %d", soundChannel);
		return false;
	}

	return _channels[soundChannel - 1].lastPlayingCast;
}

void DirectorSound::stopSound(uint8 soundChannel) {
	if (soundChannel == 0 || soundChannel > _channels.size()) {
		warning("Invalid sound channel %d", soundChannel);
		return;
	}

	_mixer->stopHandle(_channels[soundChannel - 1].handle);
	_channels[soundChannel - 1].lastPlayingCast = 0;
	return;
}

void DirectorSound::stopSound() {
	for (uint i = 0; i < _channels.size(); i++) {
		_mixer->stopHandle(_channels[i].handle);
		_channels[i].lastPlayingCast = 0;
	}
	_mixer->stopHandle(*_scriptSound);
	_mixer->stopHandle(*_pcSpeakerHandle);
}

void DirectorSound::systemBeep() {
	_speaker->play(Audio::PCSpeaker::kWaveFormSquare, 500, 150);
}

SNDDecoder::SNDDecoder() {
	_data = nullptr;
	_channels = 0;
	_size = 0;
	_rate = 0;
	_flags = 0;
}

SNDDecoder::~SNDDecoder() {
	if (_data) {
		free(_data);
	}
}

bool SNDDecoder::loadStream(Common::SeekableSubReadStreamEndian &stream) {
	if (_data) {
		free(_data);
		_data = nullptr;
	}

	if (debugChannelSet(5, kDebugLoading)) {
		debugC(5, kDebugLoading, "snd header:");
		stream.hexdump(0x4e);
	}

	// unk1
	for (uint32 i = 0; i < 0x14; i++) {
		stream.readByte();
	}
	_channels = stream.readUint16();
	if (!(_channels == 1 || _channels == 2)) {
		warning("STUB: SNDDecoder::loadStream: no support for old sound format");
		return false;
	}
	_rate = stream.readUint16();

	// unk2
	for (uint32 i = 0; i < 0x06; i++) {
		stream.readByte();
	}
	uint32 length = stream.readUint32();
	/*uint16 unk3 =*/stream.readUint16();
	/*uint32 length_copy =*/stream.readUint32();
	/*uint8 unk4 =*/stream.readByte();
	/*uint8 unk5 =*/stream.readByte();
	/*uint16 unk6 =*/stream.readUint16();
	// unk7
	for (uint32 i = 0; i < 0x12; i++) {
		stream.readByte();
	}
	uint16 bits = stream.readUint16();
	// unk8
	for (uint32 i = 0; i < 0x0e; i++) {
		stream.readByte();
	}

	_flags = 0;
	_flags |= _channels == 2 ? Audio::FLAG_STEREO : 0;
	_flags |= bits == 16 ? Audio::FLAG_16BITS : 0;
	_flags |= bits == 8 ? Audio::FLAG_UNSIGNED : 0;
	_size = length * _channels * (bits == 16 ? 2 : 1);

	_data = (byte *)malloc(_size);
	assert(_data);
	stream.read(_data, _size);

	return true;
}

Audio::SeekableAudioStream *SNDDecoder::getAudioStream() {
	if (!_data)
		return nullptr;
	return Audio::makeRawStream(_data, _size, _rate, _flags, DisposeAfterUse::NO);
}

Audio::AudioStream *SNDDecoder::getLoopingAudioStream() {
	if (!_data)
		return nullptr;
	return new Audio::LoopingAudioStream(Audio::makeRawStream(_data, _size, _rate, _flags, DisposeAfterUse::NO), 0);
}



} // End of namespace Director
