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

#ifndef LOG_H
#define LOG_H

#include "boost/serialization/singleton.hpp"
#include <Windows.h>
#include "common.h"
#include <vector>

class WorldPacket;
class WorldSession;

#ifdef WIN32

#define TRED FOREGROUND_RED | FOREGROUND_INTENSITY
#define TGREEN FOREGROUND_GREEN | FOREGROUND_INTENSITY
#define TYELLOW FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY
#define TNORMAL FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE
#define TWHITE TNORMAL | FOREGROUND_INTENSITY
#define TBLUE FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY

#else

#define TRED 1
#define TGREEN 2
#define TYELLOW 3
#define TNORMAL 4
#define TWHITE 5
#define TBLUE 6

#endif

#define LARGERRORMESSAGE_ERROR 1
#define LARGERRORMESSAGE_WARNING 2

enum LogLevel
{
    LOG_LVL_MINIMAL = 0,                                    // unconditional and errors
    LOG_LVL_BASIC   = 1,
    LOG_LVL_DETAIL  = 2,
    LOG_LVL_DEBUG   = 3
};


class CLog : public boost::serialization::singleton< CLog >
{
public:
#ifdef WIN32
	HANDLE stdout_handle, stderr_handle;
#endif

	CLog();

    void Init(int32 console_level, int32 m_logFileLevel, bool time, FILE* log);

    void Color(unsigned int color);
    void Time();
    void Line();
    void Notice(const char * source, const char * format, ...);
    void Error(const char * source, const char * format, ...);
    void Warning(const char * source, const char * format, ...);
    void Success(const char * source, const char * format, ...);
    void Debug(const char * source, const char * format, ...);
    void String(const char * format, ...);
    void LargeErrorMessage(uint32 Colour, ...);
    void PrintStartupInformationToLog();
    void outTimestamp();

private:
    int32 m_logLevel;
    int32 m_logFileLevel;
    bool m_time;

    FILE* logfile;
};

#define sLog CLog::get_mutable_instance()

#endif

