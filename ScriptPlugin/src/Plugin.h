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

#ifndef PLUGIN_PLUGIN_H
#define PLUGIN_PLUGIN_H

#include "ScriptInstance.h"

#include <map>

using std::string;
using std::map;

class Plugin : public ScriptInstance
{
public:
	static Bool DCAPI main(PluginState state, DCCorePtr core, dcptr_t);

	void setTimer(bool bState);

	HubDataPtr createHub(const char* url, const char* nick, const char* password);
	void destroyHub(HubDataPtr hHub);

	// @todo: support clones?
	HubDataPtr addHub(HubDataPtr hHub);
	HubDataPtr findHub(const string& url);
	bool removeHub(HubDataPtr hHub);

	void sendHubCommand(HubDataPtr hHub, const string& cmd);
	void injectHubCommand(HubDataPtr hHub, const string& cmd);

	// Accessors (c<->c connections)
	void sendUdpPacket(const char* ip, uint32_t port, const string& data);
	void sendClientCommand(ConnectionDataPtr hConn, const char* cmd);
	void dropClientConnection(ConnectionDataPtr hConn, bool graceless);

	static Plugin* getInstance() { return instance; }

private:
	Plugin();
	~Plugin();

	bool onLoad(DCCorePtr core, bool install, bool runtime);
	bool onHubEnter(HubDataPtr hHub, CommandDataPtr cmd);
	bool onOwnChatOut(HubDataPtr hHub, char* message);
	bool onHubConnected(HubDataPtr hHub);
	bool onHubDisconnected(HubDataPtr hHub);
	bool onHubDataIn(HubDataPtr hHub, char* message);
	bool onHubDataOut(HubDataPtr hHub, char* message);
	bool onConnectionDataIn(ConnectionDataPtr hConn, char* message);
	bool onConnectionDataOut(ConnectionDataPtr hConn, char* message);
	bool onFormatChat(UserDataPtr hUser, StringDataPtr line, bool& bBreak);
	bool onTimer();

	map<string, string> chatCache;
	map<string, HubDataPtr> hubs;

	static Plugin* instance;
};

#endif
