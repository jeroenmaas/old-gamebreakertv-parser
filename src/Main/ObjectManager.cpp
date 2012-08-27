
#include "stdafx.h"
#include "objectManager.h"
#include "Master.h"
#include "Config.h"
#include "MysqlConnection.h"
#include "Log.h"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "ShowInfoExtractor.h"
#include "httpRequest.h"
#include "BlipCategoryRssExtractor.h"
#include "BlipEmbededRssExtractor.h"
#include "c2dmMessage.h"

ObjectManager::ObjectManager()
{
}

ObjectManager::~ObjectManager()
{
    ShowList::iterator itr = m_shows.begin();
    for(; itr != m_shows.end(); ++itr)
        delete (*itr);

    m_shows.clear();

    SavedShowList::iterator itr2 = m_savedshows.begin();
    for(; itr2 != m_savedshows.end(); ++itr2)
        delete (*itr2);

    m_savedshows.clear();
}

void ObjectManager::InitCategoryData()
{
    QueryResult* result = sDatabase.Query("SELECT id, Category, rssLink FROM categories");
    if(result)
    {
        Field* field;
        do
        {
            field = result->Fetch();
            Category* cat = new Category();

            cat->flag = pow(2, double(field[0].GetUInt32() - 1));
            cat->category = field[1].GetCppString();
            cat->rssLink = field[2].GetCppString();

            m_Categories.push_back(cat);
        } while (result->NextRow());
    }
}

void ObjectManager::AddShow(ShowInfo * show)
{
    m_shows.push_back(show);
}

void ObjectManager::LoadSavedShows()
{
    uint32 highid = 0;
    QueryResult* result = sDatabase.Query("SELECT Id, title, url, videoUrl, comments FROM shows");
    if(result)
    {
        Field* field;
        do
        {
            field = result->Fetch();
            SavedShowInfo* show = new SavedShowInfo();

            show->id = field[0].GetUInt16();
            show->titel = field[1].GetCppString();
            show->url = field[2].GetCppString();
            show->videourl = field[3].GetCppString();
            show->commentcount = field[4].GetUInt32();
            show->videos = 0;

            if(show->id > highid)
                highid = show->id;

            QueryResult* result2 = sDatabase.PQuery("SELECT COUNT(*) FROM videos WHERE showid ='%i'", show->id);
            if(result2)
            {
                Field* field2;
                field2 = result2->Fetch();
                show->videos = field2[0].GetUInt8();
            }

            m_savedshows.push_back(show);
        } while (result->NextRow());
        delete result;
    }

    m_currentid = m_highid = highid;
}

int ObjectManager::ProccessNewShows()
{
    uint32 highid = m_highid;
	bool done = false;

    ShowList::iterator itr = m_shows.end(); --itr; //End is an invalid pointer to use so we want to skip that one.

    while(!done)
    {
        ShowInfo* show = (*itr);
        bool is_new = true;
        SavedShowList::iterator itr2 = m_savedshows.begin();
        for(; itr2 != m_savedshows.end(); ++itr2)
        {
            if(boost::iequals((*itr)->titel, (*itr2)->titel))
            {
                show->id = (*itr2)->id;
                is_new = false;
            }
        }

        if(is_new)
        {
            highid++;
            show->id = highid;
        }

        if(itr == m_shows.begin())
            done = true;
        else
            --itr;
    }

    return highid-m_highid;
}

void ObjectManager::SaveAll()
{
    ShowList::iterator itr = m_shows.begin();
    for(; itr != m_shows.end(); ++itr)
    {
        if((*itr)->id <= m_highid) //Exists in database
            sDatabase.PExecute("UPDATE show SET comments='%i' WHERE Id='%i'",(*itr)->commentcount, (*itr)->id);
        else
        {
            sDatabase.PExecute("INSERT INTO shows VALUES ('%i', '%s', '%s', '%s', '%s', '%s', '%i', '%s', '%s', '%s', '%i', '%i')",
                (*itr)->id, (*itr)->titel.c_str(), (*itr)->blipVideoTitle.c_str(), (*itr)->url.c_str(), (*itr)->largepictureurl.c_str(), 
                (*itr)->pictureurl.c_str(), (*itr)->categoryFlag, (*itr)->date.c_str(), (*itr)->videourl.c_str(), (*itr)->description.c_str(), 
                (*itr)->videoLength, (*itr)->commentcount);

            SaveVideo((*itr));
        }
    }
}

void ObjectManager::SaveVideo(ShowInfo* show) {
    
    sDatabase.PExecute("DELETE FROM videos WHERE showid='%i'", show->id);
    for(int i=0; i < show->videos; i++)
    {
        VideoData data = show->video_Data[i];

        //Here some checks because of invalid blip.tv data :S
        if(data.fileSize < 0)
            data.fileSize = 0;

        if(data.videoLength < 0)
            data.videoLength = 0;

        if(data.videoWidth < 0)
            data.videoWidth = 0;

        if(data.audiobitrate < 0)
            data.audiobitrate = 0;

        if(data.videobitrate < 0)
            data.videobitrate = 0;

        sDatabase.PExecute("INSERT INTO videos VALUES ('%i', '%i', '%s', '%s', '%i', '%i', '%i', '%i', '%i')",
            show->id, i, data.role.c_str(), data.link.c_str(), data.fileSize, data.videoLength, data.videoWidth, data.audiobitrate, data.videobitrate);
    }
}

void ObjectManager::PushNotifications()
{
    PushAppleNotifications();
    PushAndroidNotifications();
}

void ObjectManager::PushAppleNotifications()
{
    sLog.Debug("OBJMGR", "PushAppleNotifications");
    ShowList::iterator itr = m_shows.begin();
    for(; itr != m_shows.end(); ++itr)
    {
        if((*itr)->id > m_highid) //Exists in database
        {
            std::string command;
            command += sMaster.getConfig()->GetStringDefault("Notifications", "PHPDirectory", "c:/php/");
            command += "php.exe ";
            command += sMaster.getConfig()->GetStringDefault("Notifications", "Executable", "C:/php/scripts/push.php");
            command += " ";
            command += '"';
            command += "New Show: ";
            command += (*itr)->titel;
            command += '"';
            command += " ";

            uint32 catflag = (*itr)->categoryFlag << 2;
            command += boost::lexical_cast<std::string>(catflag);

            system (command.c_str());
        }
    }
}

void ObjectManager::PushAndroidNotifications()
{
    sLog.Debug("OBJMGR", "PushAndroidNotifications");
    QueryResult* result = sDatabase.Query("SELECT deviceid, flag FROM android_devices");
    if(result)
    {
        Field* field;
        do
        {
            field = result->Fetch();
            std::string deviceid = field[0].GetCppString();
            uint32 flag = field[1].GetUInt32();

            ShowList::iterator itr = m_shows.begin();
            for(; itr != m_shows.end(); ++itr)
            {
                if((*itr)->id > m_highid) //Exists in database
                {
                    if(flag & (*itr)->categoryFlag)
                    {
                        std::string title = (*itr)->titel;
                        title = boost::replace_all_copy(title, "&#", "@#");

                        c2dmMessage* message = new c2dmMessage();
                        message->initAndSend(deviceid, (*itr)->id, title);
                    }
                }
            }
        } while (result->NextRow());
        delete result;
    }
}

void ObjectManager::GrabVideoLinkData()
{
    ShowInfoExtractor* InfoExtractor = new ShowInfoExtractor();
    BlipEmbededRssExtractor* blipExtractor = new BlipEmbededRssExtractor();

    ShowList::iterator itr = m_shows.begin();
    for(; itr != m_shows.end(); ++itr)
    {
        if((*itr)->id > m_highid) //Doesn't exists in database
        {
            InfoExtractor->setShow((*itr));
            std::string Url = (*itr)->url;
            boost::replace_last(Url, "http://www.gamebreaker.tv/", "/");
            HandleHttpRequest("www.gamebreaker.tv", Url.c_str(), InfoExtractor);

            if((*itr)->videourl.size() > 5)
            {
                blipExtractor->setShow((*itr));
                std::string Url = (*itr)->videourl;
                boost::replace_last(Url, "http://blip.tv/play/", "/players/episode/");
                boost::replace_last(Url, ".html", "");
                Url += "?skin=api";

                HandleHttpRequest("blip.tv", Url.c_str(), blipExtractor);
            }
        }
    }
}

void ObjectManager::GrabFlagData()
{
    BlipCategoryRssExtractor* Blipextractor = new BlipCategoryRssExtractor();

    CategoryList::iterator itr = m_Categories.begin();
    for(; itr != m_Categories.end(); ++itr)
    {
        Blipextractor->setFlag((*itr)->flag);
        HandleHttpRequest("blip.tv", (*itr)->rssLink.c_str(), Blipextractor);
    }
}

void ObjectManager::CleanUp()
{
    sLog.Debug("ObjMgr", "Cleaning");

    //Find blip.tv duplicates
    SavedShowList::iterator itr = m_savedshows.begin();
    for(; itr != m_savedshows.end(); ++itr)
    {
        //Only for videos with video!
        if((*itr)->videourl.size() < 5)
            continue;

        if(m_highid - sMaster.getConfig()->GetIntDefault("Cleanup", "CheckDuplicatesCount", 100) > (*itr)->id)
            continue;

        SavedShowList::iterator itr2 = m_savedshows.begin();
        for(; itr2 != m_savedshows.end(); ++itr2)
        {
            if(boost::iequals((*itr)->videourl, (*itr2)->videourl) && (*itr)->id != (*itr2)->id)
            {
                std::string url = (*itr)->url;
                boost::replace_last(url, "http://www.gamebreaker.tv/", "/");

                bool exists = HandleHttpRequest("www.gamebreaker.tv", url.c_str(), 0, true);
                if(!exists)
                {
                    sDatabase.PExecute("DELETE FROM shows WHERE id='%i'", (*itr)->id);
                    sDatabase.PExecute("DELETE FROM videos WHERE showid='%i'", (*itr)->id);
                    m_savedshows.erase(itr, itr+1);
                    sLog.Warning("ObjMgr", "Deleted invalid show of Id %i", (*itr)->id);
                    break;
                }
            }
        }
    }

    BlipEmbededRssExtractor* blipExtractor = new BlipEmbededRssExtractor();

    //Process Recent blip.tv links for new/other qualitys
    itr = m_savedshows.begin();
    for(; itr != m_savedshows.end(); ++itr)
    {
        if(m_highid - sMaster.getConfig()->GetIntDefault("Cleanup", "CheckNewVideosCount", 25) > (*itr)->id)
            continue;

        std::string nill = "";
        ShowInfo info = ShowInfo(nill, nill, (*itr)->url, (*itr)->titel, nill, 0, 0);
        info.id = (*itr)->id;
        info.videourl = (*itr)->videourl;
        info.videos = 0;

        if((*itr)->videourl.size() > 5)
        {
            blipExtractor->setShow(&info);
            std::string Url = (*itr)->videourl;
            boost::replace_last(Url, "http://blip.tv/play/", "/players/episode/");
            boost::replace_last(Url, ".html", "");
            Url += "?skin=api";

            HandleHttpRequest("blip.tv", Url.c_str(), blipExtractor);

            if(info.videos > (*itr)->videos)
            {
                sLog.Error("ObjMgr", "Change detected in id %i", (*itr)->id);
                SaveVideo(&info);
            }
        }
    }
}

int ObjectManager::GetFlagFromCategory(std::string category)
{
    CategoryList::iterator itr = m_Categories.begin();
    for(; itr != m_Categories.end(); ++itr)
    {
        if(boost::iequals((*itr)->category, category))
            return (*itr)->flag;
    }

    return 0;
}

ShowInfo* ObjectManager::GetShowByBlipTitle(std::string title)
{
    ShowList::iterator itr = m_shows.begin();
    for(; itr != m_shows.end(); ++itr)
    {
        if(boost::iequals((*itr)->blipVideoTitle, title))
            return (*itr);
    }
    return NULL;
}

std::string ObjectManager::PrepareString(std::string oldstring)
{
    std::string newstring;
    for(int i = 0;i < oldstring.length();i++)
    {
        switch(oldstring[i])
        {
            case 39: // '
                newstring += "&#39;";
                break;
            case 45: // -
                newstring += "&#45;";
                break;
            case 96: // `
                newstring += "&#96;";
                break;
            default:
            {
                if(oldstring[i] > 0) // Only UTF-8 characters :P
                    newstring += oldstring[i];
            }
        }
    }

    boost::trim(newstring);

    return newstring;
}