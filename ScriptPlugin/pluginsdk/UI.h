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

/* Helpers around the DCUI interface. */

#ifndef PLUGINSDK_UI_H
#define PLUGINSDK_UI_H

#include <functional>
#include <string>
#include <unordered_map>

#include <pluginsdk/PluginDefs.h>

namespace dcapi {

using std::function;
using std::pair;
using std::string;
using std::unordered_map;

class UI
{
public:
	static bool init(string pluginGuid);
	static void init(DCUIPtr coreUI, string pluginGuid);
	static DCUIPtr handle();

	typedef function<void ()> Command;
	static void addCommand(string name, Command command, string icon);
	static void removeCommand(const string& name);
	static void clearCommands();

private:
	static void DCAPI commandCallback(const char* name);

	static DCUIPtr ui;

	static string guid;

	// command name -> pair<callback, icon>
	static unordered_map<string, pair<UI::Command, string>> commands;
};

} // namespace dcapi

#endif
