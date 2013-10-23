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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef ARRAYSIZE // winnt.h defines ARRAYSIZE, but we want our own one...
#endif

#include "backends/platform/sdl/sdl.h"
#include "common/config-manager.h"
#include "gui/EventRecorder.h"
#include "common/taskbar.h"
#include "common/textconsole.h"

#include "backends/saves/default/default-saves.h"

// Audio CD support was removed with SDL 1.3
#if SDL_VERSION_ATLEAST(1, 3, 0)
#include "backends/audiocd/default/default-audiocd.h"
#else
#include "backends/audiocd/sdl/sdl-audiocd.h"
#endif

#include "backends/events/sdl/sdl-events.h"
#include "backends/mutex/sdl/sdl-mutex.h"
#include "backends/timer/sdl/sdl-timer.h"
#include "backends/graphics/surfacesdl/surfacesdl-graphics.h"
#ifdef USE_OPENGL
#include "backends/graphics/openglsdl/openglsdl-graphics.h"
#include "graphics/cursorman.h"
#endif

#include "icons/scummvm.xpm"

#include <time.h>	// for getTimeAndDate()

#ifdef USE_DETECTLANG
#ifndef WIN32
#include <locale.h>
#endif // !WIN32
#endif

OSystem_SDL::OSystem_SDL()
	:
#ifdef USE_OPENGL
	_desktopWidth(0),
	_desktopHeight(0),
	_graphicsModes(),
	_graphicsMode(0),
	_firstGLMode(0),
	_defaultSDLMode(0),
	_defaultGLMode(0),
#endif
	_inited(false),
	_initedSDL(false),
	_logger(0),
	_mixerManager(0),
	_eventSource(0) {

}

OSystem_SDL::~OSystem_SDL() {
	SDL_ShowCursor(SDL_ENABLE);

	// Delete the various managers here. Note that the ModularBackend
	// destructor would also take care of this for us. However, various
	// of our managers must be deleted *before* we call SDL_Quit().
	// Hence, we perform the destruction on our own.
	delete _savefileManager;
	_savefileManager = 0;
	if (_graphicsManager) {
		dynamic_cast<SdlGraphicsManager *>(_graphicsManager)->deactivateManager();
	}
	delete _graphicsManager;
	_graphicsManager = 0;
	delete _eventManager;
	_eventManager = 0;
	delete _eventSource;
	_eventSource = 0;
	delete _audiocdManager;
	_audiocdManager = 0;
	delete _mixerManager;
	_mixerManager = 0;

#ifdef ENABLE_EVENTRECORDER
	// HACK HACK HACK
	// This is nasty.
	delete g_eventRec.getTimerManager();
#else
	delete _timerManager;
#endif

	_timerManager = 0;
	delete _mutexManager;
	_mutexManager = 0;

	delete _logger;
	_logger = 0;

	SDL_Quit();
}

void OSystem_SDL::init() {
	// Initialize SDL
	initSDL();

	// Enable unicode support if possible
	SDL_EnableUNICODE(1);

	// Disable OS cursor
	SDL_ShowCursor(SDL_DISABLE);

	if (!_logger)
		_logger = new Backends::Log::Log(this);

	if (_logger) {
		Common::WriteStream *logFile = createLogFile();
		if (logFile)
			_logger->open(logFile);
	}


	// Creates the early needed managers, if they don't exist yet
	// (we check for this to allow subclasses to provide their own).
	if (_mutexManager == 0)
		_mutexManager = new SdlMutexManager();

#if defined(USE_TASKBAR)
	if (_taskbarManager == 0)
		_taskbarManager = new Common::TaskbarManager();
#endif

}

void OSystem_SDL::initBackend() {
	// Check if backend has not been initialized
	assert(!_inited);

	// Create the default event source, in case a custom backend
	// manager didn't provide one yet.
	if (_eventSource == 0)
		_eventSource = new SdlEventSource();

#ifdef USE_OPENGL
	// Query the desktop resolution. We simply hope nothing tried to change
	// the resolution so far.
	const SDL_VideoInfo *videoInfo = SDL_GetVideoInfo();
	if (videoInfo && videoInfo->current_w > 0 && videoInfo->current_h > 0) {
		_desktopWidth  = videoInfo->current_w;
		_desktopHeight = videoInfo->current_h;
	}
#endif

	if (_graphicsManager == 0) {
#ifdef USE_OPENGL
		// Setup a list with both SDL and OpenGL graphics modes. We only do
		// this whenever the subclass did not already set up an graphics
		// manager yet. This is because we don't know the type of the graphics
		// manager of the subclass, thus we cannot easily switch between the
		// OpenGL one and the set up one. It also is to be expected that the
		// subclass does not want any switching of graphics managers anyway.
		setupGraphicsModes();

		if (ConfMan.hasKey("gfx_mode")) {
			// If the gfx_mode is from OpenGL, create the OpenGL graphics manager
			Common::String gfxMode(ConfMan.get("gfx_mode"));
			for (uint i = _firstGLMode; i < _graphicsModeIds.size(); ++i) {
				if (!scumm_stricmp(_graphicsModes[i].name, gfxMode.c_str())) {
					_graphicsManager = new OpenGLSdlGraphicsManager(_desktopWidth, _desktopHeight, _eventSource);
					_graphicsMode = i;
					break;
				}
			}
		}
#endif

		if (_graphicsManager == 0) {
			_graphicsManager = new SurfaceSdlGraphicsManager(_eventSource);
		}
	}

	if (_savefileManager == 0)
		_savefileManager = new DefaultSaveFileManager();

	if (_mixerManager == 0) {
		_mixerManager = new SdlMixerManager();
		// Setup and start mixer
		_mixerManager->init();
	}

#ifdef ENABLE_EVENTRECORDER
	g_eventRec.registerMixerManager(_mixerManager);

	g_eventRec.registerTimerManager(new SdlTimerManager());
#else
	if (_timerManager == 0)
		_timerManager = new SdlTimerManager();
#endif

	if (_audiocdManager == 0) {
		// Audio CD support was removed with SDL 1.3
#if SDL_VERSION_ATLEAST(1, 3, 0)
		_audiocdManager = new DefaultAudioCDManager();
#else
		_audiocdManager = new SdlAudioCDManager();
#endif

	}

	// Setup a custom program icon.
	setupIcon();

	_inited = true;

	ModularBackend::initBackend();

	// We have to initialize the graphics manager before the event manager
	// so the virtual keyboard can be initialized, but we have to add the
	// graphics manager as an event observer after initializing the event
	// manager.
	dynamic_cast<SdlGraphicsManager *>(_graphicsManager)->activateManager();
}

#if defined(USE_TASKBAR)
void OSystem_SDL::engineInit() {
	// Add the started engine to the list of recent tasks
	_taskbarManager->addRecent(ConfMan.getActiveDomainName(), ConfMan.get("description"));

	// Set the overlay icon the current running engine
	_taskbarManager->setOverlayIcon(ConfMan.getActiveDomainName(), ConfMan.get("description"));
}

void OSystem_SDL::engineDone() {
	// Remove overlay icon
	_taskbarManager->setOverlayIcon("", "");
}
#endif

void OSystem_SDL::initSDL() {
	// Check if SDL has not been initialized
	if (!_initedSDL) {
		// We always initialize the video subsystem because we will need it to
		// be initialized before the graphics managers to retrieve the desktop
		// resolution, for example. WebOS also requires this initialization
		// or otherwise the application won't start.
		uint32 sdlFlags = SDL_INIT_VIDEO;

		if (ConfMan.hasKey("disable_sdl_parachute"))
			sdlFlags |= SDL_INIT_NOPARACHUTE;

		// Initialize SDL (SDL Subsystems are initiliazed in the corresponding sdl managers)
		if (SDL_Init(sdlFlags) == -1)
			error("Could not initialize SDL: %s", SDL_GetError());

		_initedSDL = true;
	}
}

void OSystem_SDL::addSysArchivesToSearchSet(Common::SearchSet &s, int priority) {

#ifdef DATA_PATH
	// Add the global DATA_PATH to the directory search list
	// FIXME: We use depth = 4 for now, to match the old code. May want to change that
	Common::FSNode dataNode(DATA_PATH);
	if (dataNode.exists() && dataNode.isDirectory()) {
		s.add(DATA_PATH, new Common::FSDirectory(dataNode, 4), priority);
	}
#endif

}

void OSystem_SDL::setWindowCaption(const char *caption) {
	Common::String cap;
	byte c;

	// The string caption is supposed to be in LATIN-1 encoding.
	// SDL expects UTF-8. So we perform the conversion here.
	while ((c = *(const byte *)caption++)) {
		if (c < 0x80)
			cap += c;
		else {
			cap += 0xC0 | (c >> 6);
			cap += 0x80 | (c & 0x3F);
		}
	}

	SDL_WM_SetCaption(cap.c_str(), cap.c_str());
}

void OSystem_SDL::quit() {
	delete this;
	exit(0);
}

void OSystem_SDL::fatalError() {
	delete this;
	exit(1);
}


void OSystem_SDL::logMessage(LogMessageType::Type type, const char *message) {
	// First log to stdout/stderr
	FILE *output = 0;

	if (type == LogMessageType::kInfo || type == LogMessageType::kDebug)
		output = stdout;
	else
		output = stderr;

	fputs(message, output);
	fflush(output);

	// Then log into file (via the logger)
	if (_logger)
		_logger->print(message);

	// Finally, some Windows / WinCE specific logging code.
#if defined( USE_WINDBG )
#if defined( _WIN32_WCE )
	TCHAR buf_unicode[1024];
	MultiByteToWideChar(CP_ACP, 0, message, strlen(message) + 1, buf_unicode, sizeof(buf_unicode));
	OutputDebugString(buf_unicode);

	if (type == LogMessageType::kError) {
#ifndef DEBUG
		drawError(message);
#else
		int cmon_break_into_the_debugger_if_you_please = *(int *)(message + 1);	// bus error
		printf("%d", cmon_break_into_the_debugger_if_you_please);			// don't optimize the int out
#endif
	}

#else
	OutputDebugString(message);
#endif
#endif
}

Common::String OSystem_SDL::getSystemLanguage() const {
#if defined(USE_DETECTLANG) && !defined(_WIN32_WCE)
#ifdef WIN32
	// We can not use "setlocale" (at least not for MSVC builds), since it
	// will return locales like: "English_USA.1252", thus we need a special
	// way to determine the locale string for Win32.
	char langName[9];
	char ctryName[9];

	const LCID languageIdentifier = GetThreadLocale();

	// GetLocalInfo is only supported starting from Windows 2000, according to this:
	// http://msdn.microsoft.com/en-us/library/dd318101%28VS.85%29.aspx
	// On the other hand the locale constants used, seem to exist on Windows 98 too,
	// check this for that: http://msdn.microsoft.com/en-us/library/dd464799%28v=VS.85%29.aspx
	//
	// I am not exactly sure what is the truth now, it might be very well that this breaks
	// support for systems older than Windows 2000....
	//
	// TODO: Check whether this (or ScummVM at all ;-) works on a system with Windows 98 for
	// example and if it does not and we still want Windows 9x support, we should definitly
	// think of another solution.
	if (GetLocaleInfo(languageIdentifier, LOCALE_SISO639LANGNAME, langName, sizeof(langName)) != 0 &&
		GetLocaleInfo(languageIdentifier, LOCALE_SISO3166CTRYNAME, ctryName, sizeof(ctryName)) != 0) {
		Common::String localeName = langName;
		localeName += "_";
		localeName += ctryName;

		return localeName;
	} else {
		return ModularBackend::getSystemLanguage();
	}
#else // WIN32
	// Activating current locale settings
	const char *locale = setlocale(LC_ALL, "");

	// Detect the language from the locale
	if (!locale) {
		return ModularBackend::getSystemLanguage();
	} else {
		int length = 0;

		// Strip out additional information, like
		// ".UTF-8" or the like. We do this, since
		// our translation languages are usually
		// specified without any charset information.
		for (int i = 0; locale[i]; ++i, ++length) {
			// TODO: Check whether "@" should really be checked
			// here.
			if (locale[i] == '.' || locale[i] == ' ' || locale[i] == '@')
				break;
		}

		return Common::String(locale, length);
	}
#endif // WIN32
#else // USE_DETECTLANG
	return ModularBackend::getSystemLanguage();
#endif // USE_DETECTLANG
}

void OSystem_SDL::setupIcon() {
	int x, y, w, h, ncols, nbytes, i;
	unsigned int rgba[256];
	unsigned int *icon;

	if (sscanf(scummvm_icon[0], "%d %d %d %d", &w, &h, &ncols, &nbytes) != 4) {
		warning("Wrong format of scummvm_icon[0] (%s)", scummvm_icon[0]);

		return;
	}
	if ((w > 512) || (h > 512) || (ncols > 255) || (nbytes > 1)) {
		warning("Could not load the built-in icon (%d %d %d %d)", w, h, ncols, nbytes);
		return;
	}
	icon = (unsigned int*)malloc(w*h*sizeof(unsigned int));
	if (!icon) {
		warning("Could not allocate temp storage for the built-in icon");
		return;
	}

	for (i = 0; i < ncols; i++) {
		unsigned char code;
		char color[32];
		memset(color, 0, sizeof(color));
		unsigned int col;
		if (sscanf(scummvm_icon[1 + i], "%c c %s", &code, color) != 2) {
			warning("Wrong format of scummvm_icon[%d] (%s)", 1 + i, scummvm_icon[1 + i]);
		}
		if (!strcmp(color, "None"))
			col = 0x00000000;
		else if (!strcmp(color, "black"))
			col = 0xFF000000;
		else if (color[0] == '#') {
			if (sscanf(color + 1, "%06x", &col) != 1) {
				warning("Wrong format of color (%s)", color + 1);
			}
			col |= 0xFF000000;
		} else {
			warning("Could not load the built-in icon (%d %s - %s) ", code, color, scummvm_icon[1 + i]);
			free(icon);
			return;
		}

		rgba[code] = col;
	}
	for (y = 0; y < h; y++) {
		const char *line = scummvm_icon[1 + ncols + y];
		for (x = 0; x < w; x++) {
			icon[x + w * y] = rgba[(int)line[x]];
		}
	}

	SDL_Surface *sdl_surf = SDL_CreateRGBSurfaceFrom(icon, w, h, 32, w * 4, 0xFF0000, 0x00FF00, 0x0000FF, 0xFF000000);
	if (!sdl_surf) {
		warning("SDL_CreateRGBSurfaceFrom(icon) failed");
	}
	SDL_WM_SetIcon(sdl_surf, NULL);
	SDL_FreeSurface(sdl_surf);
	free(icon);
}


uint32 OSystem_SDL::getMillis(bool skipRecord) {
	uint32 millis = SDL_GetTicks();

#ifdef ENABLE_EVENTRECORDER
	g_eventRec.processMillis(millis, skipRecord);
#endif

	return millis;
}

void OSystem_SDL::delayMillis(uint msecs) {
#ifdef ENABLE_EVENTRECORDER
	if (!g_eventRec.processDelayMillis())
#endif
		SDL_Delay(msecs);
}

void OSystem_SDL::getTimeAndDate(TimeDate &td) const {
	time_t curTime = time(0);
	struct tm t = *localtime(&curTime);
	td.tm_sec = t.tm_sec;
	td.tm_min = t.tm_min;
	td.tm_hour = t.tm_hour;
	td.tm_mday = t.tm_mday;
	td.tm_mon = t.tm_mon;
	td.tm_year = t.tm_year;
	td.tm_wday = t.tm_wday;
}

Audio::Mixer *OSystem_SDL::getMixer() {
	assert(_mixerManager);
	return getMixerManager()->getMixer();
}

SdlMixerManager *OSystem_SDL::getMixerManager() {
	assert(_mixerManager);

#ifdef ENABLE_EVENTRECORDER
	return g_eventRec.getMixerManager();
#else
	return _mixerManager;
#endif
}

Common::TimerManager *OSystem_SDL::getTimerManager() {
#ifdef ENABLE_EVENTRECORDER
	return g_eventRec.getTimerManager();
#else
	return _timerManager;
#endif
}

#ifdef USE_OPENGL

const OSystem::GraphicsMode *OSystem_SDL::getSupportedGraphicsModes() const {
	if (_graphicsModes.empty()) {
		return _graphicsManager->getSupportedGraphicsModes();
	} else {
		return _graphicsModes.begin();
	}
}

int OSystem_SDL::getDefaultGraphicsMode() const {
	if (_graphicsModes.empty()) {
		return _graphicsManager->getDefaultGraphicsMode();
	} else {
		// Return the default graphics mode from the current graphics manager
		if (_graphicsMode < _firstGLMode)
			return _defaultSDLMode;
		else
			return _defaultGLMode;
	}
}

bool OSystem_SDL::setGraphicsMode(int mode) {
	if (_graphicsModes.empty()) {
		return _graphicsManager->setGraphicsMode(mode);
	}

	// Check whether a invalid mode is requested.
	if (mode < 0 || (uint)mode >= _graphicsModeIds.size()) {
		return false;
	}

	// Very hacky way to set up the old graphics manager state, in case we
	// switch from SDL->OpenGL or OpenGL->SDL.
	//
	// This is a probably temporary workaround to fix bugs like #3368143
	// "SDL/OpenGL: Crash when switching renderer backend".
	const int screenWidth = _graphicsManager->getWidth();
	const int screenHeight = _graphicsManager->getHeight();
	const bool arState = _graphicsManager->getFeatureState(kFeatureAspectRatioCorrection);
	const bool fullscreen = _graphicsManager->getFeatureState(kFeatureFullscreenMode);
	const bool cursorPalette = _graphicsManager->getFeatureState(kFeatureCursorPalette);
#ifdef USE_RGB_COLOR
	const Graphics::PixelFormat pixelFormat = _graphicsManager->getScreenFormat();
#endif

	bool switchedManager = false;

	// If the new mode and the current mode are not from the same graphics
	// manager, delete and create the new mode graphics manager
	if (_graphicsMode >= _firstGLMode && mode < _firstGLMode) {
		debug(1, "switching to plain SDL graphics");
		dynamic_cast<SdlGraphicsManager *>(_graphicsManager)->deactivateManager();
		delete _graphicsManager;
		_graphicsManager = new SurfaceSdlGraphicsManager(_eventSource);

		switchedManager = true;
	} else if (_graphicsMode < _firstGLMode && mode >= _firstGLMode) {
		debug(1, "switching to OpenGL graphics");
		dynamic_cast<SdlGraphicsManager *>(_graphicsManager)->deactivateManager();
		delete _graphicsManager;
		_graphicsManager = new OpenGLSdlGraphicsManager(_desktopWidth, _desktopHeight, _eventSource);

		switchedManager = true;
	}

	_graphicsMode = mode;

	if (switchedManager) {
		dynamic_cast<SdlGraphicsManager *>(_graphicsManager)->activateManager();

		_graphicsManager->beginGFXTransaction();
#ifdef USE_RGB_COLOR
		_graphicsManager->initSize(screenWidth, screenHeight, &pixelFormat);
#else
		_graphicsManager->initSize(screenWidth, screenHeight, 0);
#endif
		_graphicsManager->setFeatureState(kFeatureAspectRatioCorrection, arState);
		_graphicsManager->setFeatureState(kFeatureFullscreenMode, fullscreen);
		_graphicsManager->setFeatureState(kFeatureCursorPalette, cursorPalette);

		// Worst part about this right now, tell the cursor manager to
		// resetup the cursor + cursor palette if necessarily

		// First we need to try to setup the old state on the new manager...
		if (_graphicsManager->endGFXTransaction() != kTransactionSuccess) {
			// Oh my god if this failed the client code might just explode.
			return false;
		}

		// Next setup the cursor again
		CursorMan.pushCursor(0, 0, 0, 0, 0, 0);
		CursorMan.popCursor();

		// Next setup cursor palette if needed
		if (cursorPalette) {
			CursorMan.pushCursorPalette(0, 0, 0);
			CursorMan.popCursorPalette();
		}

		_graphicsManager->beginGFXTransaction();
		// Oh my god if this failed the client code might just explode.
		return _graphicsManager->setGraphicsMode(_graphicsModeIds[mode]);
	} else {
		return _graphicsManager->setGraphicsMode(_graphicsModeIds[mode]);
	}
}

int OSystem_SDL::getGraphicsMode() const {
	if (_graphicsModes.empty()) {
		return _graphicsManager->getGraphicsMode();
	} else {
		return _graphicsMode;
	}
}

void OSystem_SDL::setupGraphicsModes() {
	_graphicsModes.clear();
	_graphicsModeIds.clear();
	_defaultSDLMode = _defaultGLMode = -1;

	// Count the number of graphics modes
	const OSystem::GraphicsMode *srcMode;
	int defaultMode;

	GraphicsManager *manager = new SurfaceSdlGraphicsManager(_eventSource);
	srcMode = manager->getSupportedGraphicsModes();
	defaultMode = manager->getDefaultGraphicsMode();
	while (srcMode->name) {
		if (defaultMode == srcMode->id) {
			_defaultSDLMode = _graphicsModes.size();
		}
		_graphicsModes.push_back(*srcMode);
		srcMode++;
	}
	delete manager;
	assert(_defaultSDLMode != -1);

	_firstGLMode = _graphicsModes.size();
	manager = new OpenGLSdlGraphicsManager(_desktopWidth, _desktopHeight, _eventSource);
	srcMode = manager->getSupportedGraphicsModes();
	defaultMode = manager->getDefaultGraphicsMode();
	while (srcMode->name) {
		if (defaultMode == srcMode->id) {
			_defaultGLMode = _graphicsModes.size();
		}
		_graphicsModes.push_back(*srcMode);
		srcMode++;
	}
	delete manager;
	manager = nullptr;
	assert(_defaultGLMode != -1);

	// Set a null mode at the end
	GraphicsMode nullMode;
	memset(&nullMode, 0, sizeof(nullMode));
	_graphicsModes.push_back(nullMode);

	// Set new internal ids for all modes
	int i = 0;
	OSystem::GraphicsMode *mode = _graphicsModes.begin();
	while (mode->name) {
		_graphicsModeIds.push_back(mode->id);
		mode->id = i++;
		mode++;
	}
}

#endif
