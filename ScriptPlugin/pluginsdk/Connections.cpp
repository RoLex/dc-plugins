/*
 * Copyright (C) 2012-2024 Jacek Sieka, arnetheduck on gmail point com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

/* Helpers around the DCConnection interface. */

#include "Connections.h"

#include "Core.h"

namespace dcapi {

DCConnectionPtr Connections::connections;

bool Connections::init() {
	if(!Core::handle()) { return false; }
	init(reinterpret_cast<DCConnectionPtr>(Core::handle()->query_interface(DCINTF_DCPP_CONNECTIONS, DCINTF_DCPP_CONNECTIONS_VER)));
	return connections;
}
void Connections::init(DCConnectionPtr coreConnections) { connections = coreConnections; }
DCConnectionPtr Connections::handle() { return connections; }

} // namespace dcapi
