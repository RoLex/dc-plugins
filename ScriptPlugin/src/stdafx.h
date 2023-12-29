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

#ifndef PLUGIN_STDAFX_H
#define PLUGIN_STDAFX_H

#ifdef _WIN32

#include <windows.h>
#include <tchar.h>

#else

#include <unistd.h>

#endif

// This can make a #define value to string (from boost)
#define STRINGIZE(X) DO_STRINGIZE(X)
#define DO_STRINGIZE(X) #X

#include <cstdio>
#include <cstdint>
#include <string>

#include <pluginsdk/PluginDefs.h>

#include "version.h"

#ifdef _UNICODE
typedef std::wstring tstring;
#else
typedef std::string tstring;
#endif

#endif
