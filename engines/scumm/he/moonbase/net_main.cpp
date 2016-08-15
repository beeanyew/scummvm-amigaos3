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

#include "scumm/he/intern_he.h"
#include "scumm/he/moonbase/moonbase.h"
#include "scumm/he/moonbase/net_main.h"

namespace Scumm {

Net::Net(ScummEngine_v100he *vm) : _vm(vm) {
	//some defaults for fields
}

int Net::hostGame(char *sessionName, char *userName) {
	warning("STUB: op_net_host_tcpip_game()"); // PN_HostTCPIPGame
	return 0;
}

int Net::joinGame(char *IP, char *userName) {
	warning("STUB: Net::joinGame()"); // PN_JoinTCPIPGame
	return 0;
}

} // End of namespace Scumm
