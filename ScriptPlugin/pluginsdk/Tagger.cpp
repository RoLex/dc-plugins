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

/* Helpers around the DCTagger interface. */

#include "Tagger.h"

#include "Core.h"

namespace dcapi {

DCTaggerPtr Tagger::tagger;

bool Tagger::init() {
	if(!Core::handle()) { return false; }
	init(reinterpret_cast<DCTaggerPtr>(Core::handle()->query_interface(DCINTF_DCPP_TAGGER, DCINTF_DCPP_TAGGER_VER)));
	return tagger;
}
void Tagger::init(DCTaggerPtr coreTagger) { tagger = coreTagger; }
DCTaggerPtr Tagger::handle() { return tagger; }

} // namespace dcapi
