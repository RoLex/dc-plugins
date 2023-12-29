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

#include "UI.h"

#include "Core.h"

namespace dcapi {

DCUIPtr UI::ui;
string UI::guid;
unordered_map<string, pair<UI::Command, string>> UI::commands;

bool UI::init(string pluginGuid) {
	if(!Core::handle()) { return false; }
	init(reinterpret_cast<DCUIPtr>(Core::handle()->query_interface(DCINTF_DCPP_UI, DCINTF_DCPP_UI_VER)), move(pluginGuid));
	return ui;
}
void UI::init(DCUIPtr coreUI, string pluginGuid) { ui = coreUI; guid = move(pluginGuid); }
DCUIPtr UI::handle() { return ui; }

void UI::addCommand(string name, Command command, string icon) {
	const auto& iter = commands.insert(std::make_pair(move(name), std::make_pair(command, move(icon)))).first;
	ui->add_command(guid.c_str(), iter->first.c_str(), commandCallback, iter->second.second.c_str());
}

void UI::removeCommand(const string& name) {
	ui->remove_command(guid.c_str(), name.c_str());
	commands.erase(name);
}

void UI::clearCommands() {
	for(auto i = commands.cbegin(), iend = commands.cend(); i != iend; ++i) {
		ui->remove_command(guid.c_str(), i->first.c_str());
	}
	commands.clear();
}

void DCAPI UI::commandCallback(const char* name) {
	commands[name].first();
}

} // namespace dcapi
