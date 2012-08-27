/*
 * Mesmeric Emu
 * Copyright (C) 2010-2011 Mesmeric WoW <http://www.MesmericWoW.com/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MASTER_H
#define MASTER_H

#include "boost/serialization/singleton.hpp"
#include "common.h"

class ConfigFile;

class Master : public boost::serialization::singleton< Master >
{
public:
	Master();
    ~Master();

    bool startUp();

    _inline ConfigFile* getConfig() { return m_config; }

private:
    ConfigFile* m_config;
};

#define sMaster Master::get_mutable_instance()

#endif

