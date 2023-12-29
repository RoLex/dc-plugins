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

#include "stdafx.h"
#include "Plugin.h"

/* Include plugin SDK helpers. There are more interfaces available that can be included in the same
fashion (check the pluginsdk directory). */
#include <pluginsdk/Config.h>
#include <pluginsdk/Connections.h>
#include <pluginsdk/Core.h>
#include <pluginsdk/Hooks.h>
#include <pluginsdk/Hubs.h>
#include <pluginsdk/Logger.h>
#include <pluginsdk/UI.h>
#include <pluginsdk/Util.h>

#include <shellapi.h>

/* Plugin SDK helpers are in the "dcapi" namespace; ease their calling. */
using dcapi::Config;
using dcapi::Connections;
using dcapi::Core;
using dcapi::Hooks;
using dcapi::Hubs;
using dcapi::Logger;
using dcapi::UI;
using dcapi::Util;

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

Plugin* Plugin::instance = nullptr;

Plugin::Plugin() { L = nullptr; }

Plugin::~Plugin() {
	if(L) {
		if(!hubs.empty()) {
			// This is only relevant for runtime unloads
			for(auto i: hubs)
				Hubs::handle()->release(i.second);

			hubs.clear();
		}

		Hooks::clear();
		UI::clearCommands();

		chatCache.clear();

		lua_close(L);
	}
}

Bool DCAPI Plugin::main(PluginState state, DCCorePtr core, dcptr_t) {
	switch(state) {
	case ON_INSTALL:
	case ON_LOAD:
	case ON_LOAD_RUNTIME:
		{
			instance = new Plugin();
			return instance->onLoad(core, state == ON_INSTALL, state == ON_INSTALL || state == ON_LOAD_RUNTIME) ? True : False;
		}

	case ON_UNINSTALL:
	case ON_UNLOAD:
		{
			delete instance;
			instance = 0;
			return True;
		}

	default:
		{
			return False;
		}
	}
}

bool Plugin::onLoad(DCCorePtr core, bool install, bool runtime) {
	/* Initialization phase. Initiate additional interfaces that you may have included from the
	plugin SDK. */
	Core::init(core);
	if(!Config::init(PLUGIN_GUID) || !Connections::init() || !Hooks::init() ||
		!Hubs::init() || !Logger::init() || !UI::init(PLUGIN_GUID) || !Util::init())
	{
		return false;
	}

	// Default settings
	if(Config::getConfig("ScriptPath").empty()) {
		auto path = Config::getInstallPath();
		auto sep = !path.empty() ? *(path.end() - 1) : '/';
		Config::setConfig("ScriptPath", path + string("scripts") + sep);
	}

	if(Config::getConfig("LuaDebug").empty())
		Config::setConfig("LuaDebug", false);

	if(Config::getConfig("FormatChat").empty())
		Config::setConfig("FormatChat", true);

	if(install) {
		// This only executes when the plugin has been installed for the first time.
		Logger::log("Script plugin installed, please restart " + Core::appName + " to begin using the plugin.");
		return true;
	}

	if(runtime) {
		// If the plugin doesn't support being enabled at runtime, cancel its loading here.
		Logger::log("Script plugin enabled, please restart " + Core::appName + " to begin using the plugin.");
		return true;
	}

	// Start the plugin logic here; add hooks with functions from the Hooks interface.
	L = luaL_newstate();
	luaL_openlibs(L);

#ifdef WITH_LUAJIT
	// This just turns the whole JIT compiler on with default behaviour (could probably be improved)
	luaJIT_setmode(L, 0, LUAJIT_MODE_ENGINE|LUAJIT_MODE_ON);
#endif

	Lunar<LuaManager>::Register(L);
	lua_pop(L, lua_gettop(L));

	Hooks::Chat::onOutgoingChat([this](HubDataPtr hub, char* msg, bool&) { return onOwnChatOut(hub, msg); });

	Hooks::Hubs::onOnline([this](HubDataPtr hub, bool&) { return onHubConnected(hub); });
	Hooks::Hubs::onOffline([this](HubDataPtr hub, bool&) { return onHubDisconnected(hub); });

	Hooks::Network::onHubDataIn([this](HubDataPtr hub, char* msg, bool&) { return onHubDataIn(hub, msg); });
	Hooks::Network::onHubDataOut([this](HubDataPtr hub, char* msg, bool&) { return onHubDataOut(hub, msg); });
	Hooks::Network::onClientDataIn([this](ConnectionDataPtr conn, char* msg, bool&) { return onConnectionDataIn(conn, msg); });
	Hooks::Network::onClientDataOut([this](ConnectionDataPtr conn, char* msg, bool&) { return onConnectionDataOut(conn, msg); });

	Hooks::UI::onChatCommand([this](HubDataPtr hub, CommandDataPtr command, bool&) { return onHubEnter(hub, command); });

	ScriptInstance::EvaluateFile("startup.lua");

	// This ensures that FormatChatText is only called when present...
	if(CheckFunction("dcpp", "FormatChatHTML")) {
		Hooks::UI::onChatDisplay([this](UserDataPtr user, StringDataPtr str, bool& bBreak) { return onFormatChat(user, str, bBreak); });
	}

	UI::addCommand("Open the scripts directory", [] {
#ifdef _WIN32
		ShellExecute(nullptr, nullptr, Util::toT(Config::getConfig("ScriptPath")).c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#else
		system(string("xdg-open " + Config::getConfig("ScriptPath")).c_str())
#endif
	}, string());

	return true;
}

void Plugin::setTimer(bool bState) {
	static bool timerHookAdded = false;
	if(bState && !timerHookAdded) {
		Hooks::Timer::onSecond([this](uint64_t, bool&) { return onTimer(); });
		timerHookAdded = true;
	} else if(timerHookAdded) {
		Hooks::remove(HOOK_TIMER_SECOND);
		timerHookAdded = false;
	}
}

HubDataPtr Plugin::createHub(const char* url, const char* nick, const char* password) {
	auto hHub = Hubs::handle()->add_hub(url, nick, password);
	if(!hHub) hubs[url] = hHub;
	return hHub;
}

void Plugin::destroyHub(HubDataPtr hHub) {
	// onHubDisconnected cleans up the copy from createHub
	Hubs::handle()->remove_hub(hHub);
}

HubDataPtr Plugin::addHub(HubDataPtr hHub) {
	auto i = hubs.find(hHub->url);
	if(i != hubs.end())
		return i->second;

	auto copy = Hubs::handle()->copy(hHub);
	hubs[hHub->url] = copy;
	return copy;
}

HubDataPtr Plugin::findHub(const string& url) {
	auto i = hubs.find(url);
	if(i != hubs.end())
		return i->second;
	return nullptr;
}

bool Plugin::removeHub(HubDataPtr hHub) {
	auto i = hubs.find(hHub->url);
	if(i == hubs.end())
		return false;

	// remove cache for chat formatting
	auto j = chatCache.find(i->second->url);
	if(j != chatCache.end())
			chatCache.erase(j);

	Hubs::handle()->release(i->second);
	hubs.erase(i);
	return true;
}

void Plugin::sendHubCommand(HubDataPtr hHub, const string& cmd) {
	if(hHub)
		Hubs::handle()->send_protocol_cmd(hHub, cmd.c_str());
}

void Plugin::injectHubCommand(HubDataPtr hHub, const string& cmd) {
	if(hHub)
		Hubs::handle()->emulate_protocol_cmd(hHub, cmd.c_str());
}

void Plugin::sendUdpPacket(const char* ip, uint32_t port, const string& data) {
	Connections::handle()->send_udp_data(ip, port, reinterpret_cast<dcptr_t>(const_cast<char*>(data.c_str())), data.size());
}

void Plugin::sendClientCommand(ConnectionDataPtr hConn, const char* cmd) {
	if(hConn)
		Connections::handle()->send_protocol_cmd(hConn, cmd);
}

void Plugin::dropClientConnection(ConnectionDataPtr hConn, bool graceless) {
	if(hConn)
		Connections::handle()->terminate_conn(hConn, graceless ? True : False);
}

namespace { string formatBytes(double val) {
	int i = 0;
	while(val >= 1024 && i < 5) { val = val / 1024; i += 1; }
	const char units[5][4] = { "KiB", "MiB", "GiB", "TiB", "PiB" };

	char buf[64];
	snprintf(buf, sizeof(buf), "%.02f %s", val, units[i]);
	return buf;
} }

bool Plugin::onHubEnter(HubDataPtr hHub, CommandDataPtr cmd) {
	if(stricmp(cmd->command, "help") == 0) {
		if(stricmp(cmd->params, "plugins") == 0) {
			const char* help =
				"\t\t\t Help: Script plugin \t\t\t\n"
				"\t /lua <code> \t\t\t Evaluate parameter as Lua code\n"
				"\t /luafile <file> \t\t\t Execute specified script file\n"
				"\t /luamem \t\t\t Return Lua memory usage and version\n"
				"\t /luadebug \t\t\t Toggle non-fatal error messages (default: off)\n"
				"\t /formatchat \t\t\t Toggle Lua chat formatting hook (default: on, if available)\n";

			Hubs::handle()->local_message(hHub, help, MSG_SYSTEM);
			return true;
		}
	} else if(stricmp(cmd->command, "lua") == 0) {
		if(strlen(cmd->params) != 0) {
			ScriptInstance::EvaluateChunk(Util::fromUtf8(cmd->params));
		} else {
			Hubs::handle()->local_message(hHub, "You must supply a parameter!", MSG_SYSTEM);
		}
		return true;
	} else if(stricmp(cmd->command, "luafile") == 0) {
		if(strlen(cmd->params) > 4) {
			ScriptInstance::EvaluateFile(Util::fromUtf8(cmd->params));
		} else {
			Hubs::handle()->local_message(hHub, "You must supply a valid parameter!", MSG_SYSTEM);
		}
		return true;
	} else if(stricmp(cmd->command, "meminfo") == 0 || stricmp(cmd->command, "luamem") == 0) {
		// Run GC and get LUA memory usage
		lua_gc(L, LUA_GCCOLLECT, 0);
		double mem = lua_gc(L, LUA_GCCOUNT, 0);
		auto stats = "Scripts (" LUA_DIST ") are using " + formatBytes(mem) + " of system memory";

		Hubs::handle()->local_message(hHub, stats.c_str(), MSG_SYSTEM);
		return true;
	} else if(stricmp(cmd->command, "luadebug") == 0) {
		bool state = Config::getBoolConfig("LuaDebug");
		const char* status = (state ? "Additional error messages disabled" : "Additional error messages enabled");

		Config::setConfig("LuaDebug", state);
		Hubs::handle()->local_message(hHub, status, MSG_SYSTEM);
		return true;
	} else if(stricmp(cmd->command, "formatchat") == 0) {
		bool state = Config::getBoolConfig("FormatChat");
		const char* status = (state ? "Lua chat formatting disabled" : "Lua chat formatting enabled");

		Config::setConfig("FormatChat", state);
		Hubs::handle()->local_message(hHub, status, MSG_SYSTEM);
		return true;
	}

	// This stupidity is because of the API
	return onOwnChatOut(hHub, const_cast<char*>(string("/" + string(cmd->command) + " " + string(cmd->params)).c_str()));
}

bool Plugin::onOwnChatOut(HubDataPtr hHub, char* message) {
	Lock l(cs);
	// TODO DC++ may trigger this incorrectly for disconnected hubs...
	if ((hHub = findHub(hHub->url)) != nullptr)
		return MakeCall("dcpp", "OnCommandEnter", 1, hHub, string(message)) ? GetLuaBool() : false;
	return false;
}

bool Plugin::onHubConnected(HubDataPtr hHub) {
	Lock l(cs);
	MakeCall(GetHubType(hHub), "OnHubAdded", 0, addHub(hHub));
	return false;
}

bool Plugin::onHubDisconnected(HubDataPtr hHub) {
	Lock l(cs);
	// TODO DC++ may trigger this incorrectly (for hubs where OnHubAdded was never invoked), if socket creation fails...
	if ((hHub = findHub(hHub->url)) != nullptr) {
		MakeCall(GetHubType(hHub), "OnHubRemoved", 0, hHub);
		removeHub(hHub);
	}

	return false;
}

bool Plugin::onHubDataIn(HubDataPtr hHub, char* message) {
	Lock l(cs);
	return MakeCall(GetHubType(hHub), "DataArrival", 1, findHub(hHub->url), string(message)) ? GetLuaBool() : false;
}

bool Plugin::onHubDataOut(HubDataPtr hHub, char* message) {
	//Lock l(cs); // todo: make it useful, dont forget startup.lua
	//return MakeCall("dcpp", "HubDataOut", 1, findHub(hHub->url), string(message)) ? GetLuaBool() : false;
	return false;
}

bool Plugin::onConnectionDataIn(ConnectionDataPtr hConn, char* message) {
	Lock l(cs);
	return MakeCall("dcpp", "UserDataIn", 1, hConn, string(message)) ? GetLuaBool() : false;
}

bool Plugin::onConnectionDataOut(ConnectionDataPtr hConn, char* message) {
	Lock l(cs);
	return MakeCall("dcpp", "UserDataOut", 1, hConn, string(message)) ? GetLuaBool() : false;
}

bool Plugin::onFormatChat(UserDataPtr hUser, StringDataPtr line, bool& bBreak) {
	Lock l(cs);
	if(!hUser || !Config::getBoolConfig("FormatChat") || !MakeCall("dcpp", "FormatChatHTML", 1, findHub(hUser->hubHint), string(line->in)))
		return false;

	if(lua_isstring(L, -1)) {
		string text = lua_tostring(L, -1);
		if(strcmp(line->in, text.c_str()) != 0) {
			string& out = chatCache[hUser->hubHint] = text;
			line->out = &out[0];
		}
	}

	lua_settop(L, 0);

	if(line->out) {
		// We don't send this to other plugins (users can control which formatting applies via plugin order)
		bBreak = true;
		return true;
	}
	return false;
}

bool Plugin::onTimer() {
	MakeCall("dcpp", "OnTimer", 0, 0);
	return false;
}
