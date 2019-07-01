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

#ifndef HDB_WINDOW_H
#define HDB_WINDOW_H

namespace HDB {

enum {
	kDialogTextLeft = 64,
	kDialogTextRight = (kDialogTextLeft + kTileWidth * 9),
	kOpenDialogTextLeft = kDialogTextLeft,
	kOpenDialogTextRight = (kDialogTextRight + kTileWidth * 2),
	kMaxMsgQueue = 10,
	kWeaponX = (480 - 34),
	kWeaponY = 2,
	kInvItemSpaceX = 48,
	kInvItemSpaceY = 40,
	kInvItemPerLine = 3,
	kDlvItemSpaceX = 48,
	kDlvItemSpaceY = (kTileHeight * 2 + 16),
	kDlvItemPerLine = 3,
	kDlvItemTextY = (kScreenHeight - 30),
	kNumCrazy = 37,
	kTextOutCenterX = ((kScreenWidth - kTileWidth * 5) / 2),
	kPauseY = (kScreenHeight / 2 - 64)
};

struct DialogInfo {
	char		title[64];				// TITLE string
	int			tileIndex;					// this is for a character picture
	char		string[160];			// actual text in the dialog

	bool		active;					// is it drawing or not?
	int			x, y;					// where to draw dialog
	int			width, height;			// size of the dialog itself
	int			titleWidth;
	Picture		*gfx;					// character tile (picture)
	int			more;					// whether we want to draw the MORE icon or not
	int			el, er, et, eb;			// saves the text edges
	char		luaMore[64];			// the name of the function to call after clicking the MORE button

	DialogInfo() : title(""), tileIndex(0), string(""), active(false), x(0), y(0),
		width(0), height(0), titleWidth(0), gfx(NULL), more(0), el(0), er(0), et(0),
		eb(0), luaMore("") {}
};

struct DialogChoiceInfo {
	char		title[64];				// TITLE string
	char		text[160];				// actual text in the dialog
	char		func[64];				// function to call with result

	bool		active;					// is it drawing or not?
	int			x, y;					// where to draw dialog
	int			width, height;			// size of the dialog itself
	int			textHeight;				// height of everything above choices
	int			titleWidth;
	int			el, er, et, eb;			// saves the text edges
	uint32		timeout;				// timeout value!

	int			selection;				// which choice we've made
	int			numChoices;			// how many choices possible
	char		choices[10][64];		// ptrs to choice text

	DialogChoiceInfo() : title(""), text(""), func(""), active(false), x(0), y(0),
		width(0), height(0), textHeight(0), titleWidth(0), el(0), er(0), et(0),
		eb(0), timeout(0), selection(0), numChoices(0) {
		for (int i = 0; i < 10; i++)
			strcpy(choices[i], "");
	}
};

struct MessageInfo {
	bool		active;
	char		title[128];
	int			timer;
	int			x, y;
	int			width, height;

	MessageInfo() : active(false), title(""), timer(0), x(0), y(0), width(0), height(0) {}
};

struct InvWinInfo {
	int x, y;
	int width, height;
	int selection;
	bool active;

	InvWinInfo() : x(0), y(0), width(0), height(0), selection(0), active(false) {}
};

struct DlvsInfo {
	int x, y;
	int width, height;
	bool active;
	int selected;
	bool animate;
	uint32 delay1, delay2, delay3;
	bool go1, go2, go3;

	DlvsInfo() : x(0), y(0), width(0), height(0), active(false), selected(0), animate(false), delay1(0), delay2(0), delay3(0), go1(false), go2(false), go3(false) {}
};

struct TOut {
	char text[128];
	int x, y;
	uint32 timer;

	TOut() : text(""), x(0), y(0), timer(0) {}
};

class Window {
public:

	bool init();
	void restartSystem();
	void setInfobarDark(int value);

	// Pause Functions
	void drawPause();
	void checkPause(uint x, uint y);

	// Dialog Functions

	void openDialog(const char *title, int tileIndex, const char *string, int more, const char *luaMore);
	void drawDialog();
	void closeDialog();
	bool checkDialogClose(int x, int y);
	void drawBorder(int x, int y, int width, int height, bool guyTalking);
	void setDialogDelay(int delay);
	uint32 getDialogDelay() {
		return _dialogDelay;
	}
	bool dialogActive() {
		return _dialogInfo.active;
	}

	// Dialog Choice Functions

	void openDialogChoice(const char *title, const char *text, const char *func, int numChoices, const char *choices[10]);
	void drawDialogChoice();
	void closeDialogChoice();
	bool checkDialogChoiceClose(int x, int y);
	void dialogChoiceMoveup();
	void dialogChoiceMovedown();
	bool dialogChoiceActive() {
		return _dialogChoiceInfo.active;
	}

	// MessageBar Functions
	void openMessageBar(const char *title, int time);
	void drawMessageBar();
	bool checkMsgClose(int x, int y);
	void nextMsgQueued();
	void closeMsg();
	bool msgBarActive() {
		return _msgInfo.active;
	}

	// Inventory Functions
	void drawInventory();
	void setInvSelect(int status) {
		_invWinInfo.selection = status;
	}
	int getInvSelect() {
		return _invWinInfo.selection;
	}
	void checkInvSelect(int x, int y);

	// Deliveries Functions
	void openDeliveries(bool animate);
	void drawDeliveries();
	void setSelectedDelivery(int which);
	int getSelectedDelivery() {
		return _dlvsInfo.selected;
	}
	bool animatingDelivery() {
		return _dlvsInfo.animate;
	}
	void checkDlvSelect(int x, int y);

	// TextOut functions
	void textOut(const char *text, int x, int y, int timer);
	void centerTextOut(const char *text, int y, int timer);
	void drawTextOut();
	int textOutActive() {
		return (_textOutList.size());
	}
	void closeTextOut();
private:

	DialogInfo _dialogInfo;
	uint32 _dialogDelay;	// Used for Cinematics

	DialogChoiceInfo _dialogChoiceInfo;

	MessageInfo _msgInfo;

	InvWinInfo _invWinInfo;
	Common::Array<TOut *> _textOutList;
	DlvsInfo _dlvsInfo;

	char _msgQueueStr[kMaxMsgQueue][128];
	int _msgQueueWait[kMaxMsgQueue];
	int _numMsgQueue;

	// Windows GFX
	Picture *_gfxTL, *_gfxTM, *_gfxTR;
	Picture *_gfxL, *_gfxM, *_gfxR;
	Picture *_gfxBL, *_gfxBM, *_gfxBR;
	Picture *_gfxTitleL, *_gfxTitleM, *_gfxTitleR;
	Picture *_gGfxTL, *_gGfxTM, *_gGfxTR;
	Picture *_gGfxL, *_gGfxM, *_gGfxR;
	Picture *_gGfxBL, *_gGfxBM, *_gGfxBR;
	Picture *_gGfxTitleL, *_gGfxTitleM, *_gGfxTitleR;
	Picture *_gfxResources, *_gfxDeliveries;
	Picture *_gfxIndent, *_gfxArrowTo, *_gfxHandright;
	Picture *_gfxTry, *_gfxAgain, *_gfxInvSelect;
	Tile *_gfxMonkeystone;
	Picture *_gfxLevelRestart, *_gfxPausePlaque;
	Tile *_gemGfx;
	Picture *_mstoneGfx;

	// Info Bar
	Picture *_gfxInfobar, *_gfxDarken;
	int _infobarDimmed;
};

} // End of Namespace

#endif // !HDB_WINDOW_H
