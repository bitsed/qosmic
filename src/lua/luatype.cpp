/***************************************************************************
 *   Copyright (C) 2007-2024 by David Bitseff                              *
 *   bitsed@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/
#include "luatype.h"
#include "luathreadadapter.h"

namespace Lua
{

LuaType::LuaType()
	: m_adapter(0)
{
}

LuaType::LuaType(LuaThreadAdapter* value)
	: m_adapter(value)
{
}


LuaType::~LuaType()
{
}


LuaThreadAdapter* LuaType::context() const
{
	return m_adapter;
}


void LuaType::setContext(LuaThreadAdapter* value)
{
	m_adapter = value;
}


GenomeVector* LuaType::genomes() const
{
	return m_adapter->genomeVector();
}

}

