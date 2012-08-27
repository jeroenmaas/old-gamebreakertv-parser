#include "stdafx.h"
#include "boost/serialization/singleton.hpp"
#include <vector>

class MySQLConnection;

#define MAX_VIDEO_DATA_PER_SHOW 6 //I believe its actually 5 but we'll just do 6 to be save :D

struct SavedShowInfo
{
    uint16 id;
    std::string titel;
    std::string url;
    std::string videourl;
    uint32 commentcount;
    uint8 videos;
};

struct VideoData
{
    std::string role;
    std::string link;
    uint32 fileSize; //Saved in kilobytes

    //These we just save and not use in the program. Just nice as statistics :D
    int videoLength;
    int videoWidth;
    int audiobitrate;
    int videobitrate;
};

struct ShowInfo
{
    ShowInfo(std::string _pictureurl, std::string _largepictureurl, std::string _url, std::string _titel, std::string _date, uint32 _CategoryFlag, uint32 _commentcount)
        : pictureurl(_pictureurl), largepictureurl(_largepictureurl), url(_url), titel(_titel), date(_date), categoryFlag(_CategoryFlag), commentcount(_commentcount), videos(0)
    {}

    //grabed from main page (http://www.gamebreaker.tv/page/1/) as example
    int id;
    std::string pictureurl;
    std::string largepictureurl;
    std::string url;
    std::string titel;
    std::string date;
    uint32 commentcount;

    //grabed from show page (http://www.gamebreaker.tv/this-week-in-mmo-63-world-of-darkness/)
    std::string videourl;
    std::string description;
    std::string ShowComment;

    //grabed from blip Embeded Rss (http://blip.tv/players/episode/hq8%2BgtiufAA?skin=api)
    std::string blipVideoTitle;
    int videoLength; //in seconds

    //Grabed from blip Category Rss (http://blip.tv/legendary-the-world-of-warcraft-show-video-edition/rss)
    uint32 categoryFlag; //Doesn't grab Blog Currently!
    int videos;
    VideoData video_Data[MAX_VIDEO_DATA_PER_SHOW];
};

struct Category
{
    uint32 flag;
    std::string category;
    std::string rssLink;
};

typedef std::vector<ShowInfo*> ShowList;
typedef std::vector<SavedShowInfo*> SavedShowList;
typedef std::vector<Category*> CategoryList;

class ObjectManager : public boost::serialization::singleton< ObjectManager >
{
public:
    ObjectManager();
    ~ObjectManager();

    void InitCategoryData();

    void LoadSavedShows();
    void AddShow(ShowInfo* show);
    int ProccessNewShows(); 
    void SaveAll();
    void SaveVideo(ShowInfo* show);

    void PushNotifications();
    void PushAppleNotifications();
    void PushAndroidNotifications();

    void GrabVideoLinkData();
    void GrabFlagData();
    void CleanUp();

    std::string PrepareString(std::string);

    int GetFlagFromCategory(std::string category);
    ShowInfo* GetShowByBlipTitle(std::string title);

private:
    SavedShowList m_savedshows;
    ShowList m_shows;
    CategoryList m_Categories;

    uint32 m_highid;

    uint32 m_currentid;
};

#define sObjectMgr ObjectManager::get_mutable_instance()
