#include "stdafx.h"
#include "Master.h"
#include "Config.h"
#include "MysqlConnection.h"
#include "Log.h"
#include "ObjectManager.h"

Master::Master()
{
    m_config = new ConfigFile();
}

Master::~Master()
{
}

bool Master::startUp()
{
    if(!m_config->SetSource("Extractor.conf",true))
    {
        sLog.LargeErrorMessage(LARGERRORMESSAGE_ERROR, "Couldn't locate config file", "Please check if Extractor.conf is in the same directory as Extractor.exe", NULL);
        return false;
    }

    sLog.Success( "Config", "Loaded Extractor.conf" );

    int32 ConsoleLevel = m_config->GetIntDefault("Log", "Screen", 1);
    int32 FileLevel = m_config->GetIntDefault("Log", "File", 1);
    bool m_time = m_config->GetBoolDefault("Log", "Time", false);
    std::string logfile = m_config->GetStringDefault("Log", "LogFile", "Extractor.log");
    FILE* file = fopen(logfile.c_str(), "a");
    sLog.Init(ConsoleLevel, FileLevel, m_time, file);

    sLog.Success("Log","Initialised");

    std::string Hostname = m_config->GetStringDefault("Database", "Hostname", "");
    std::string Username = m_config->GetStringDefault("Database", "Username", "");
    std::string Password = m_config->GetStringDefault("Database", "Password", "");
    std::string Name = m_config->GetStringDefault("Database", "Name", "gbtv");
    uint32 Port = m_config->GetIntDefault("Database", "Port", 3306);

    if(!sDatabase.Initialize(Hostname, Username, Password, Name, Port))
    {
        return false;
    }

    sLog.Success("MySQL", "Initialised");

    sObjectMgr.InitCategoryData();

    sLog.Success("CategoryData", "Initialised");

    return true;
}