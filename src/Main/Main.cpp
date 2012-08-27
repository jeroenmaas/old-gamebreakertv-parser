#include "StdAfx.h"
#include <time.h>
#include "headerExtractor.h"
#include "httpRequest.h"
#include "c2dmMessage.h"
#include "MysqlConnection.h"
#include "Log.h"
#include "ObjectManager.h"
#include "Master.h"

int main(int argc, char* argv[])
{
    time_t seconds = time(NULL);

    sLog.String("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM                                          ");
    sLog.String("MM                              MM                                          ");
    sLog.String("MM                              MM                                          ");
    sLog.String("MM               IIIIIIIIIIIIIIIMM                                          ");
    sLog.String("MM               IIIIIIIIIIIIIIIMM                                          ");
    sLog.String("MM               IIIIIIIIIIIIIIIMM                                          ");
    sLog.String("MM               IIIIIIIIIIIIIIIMM            GameBreaker.tv Extractor      ");
    sLog.String("MM      IIIIIIIIIIIIIIIIIIIIIIIIMM                                          ");
    sLog.String("MM      IIIIIIIIIIIIIIIIIIIIIIIIMM            Created by:                   ");
    sLog.String("MM      IIIIIIIIIIIIIIIIIIIIIIIIMM              BlackThunder aka Jeroen     ");
    sLog.String("MM      IIIIIIIIIIIIIIIIIIIIIIIIMM                                          ");
    sLog.String("MM      IIIIIIIIIIIIIIIIIIIIIIIIMM                                          ");
    sLog.String("MM      IIIIIIIIIIIIIIIIIIIIIIIIMM                                          ");
    sLog.String("MM      IIIIIIIIIIIIIIIIIIIIIIIIMM                                          ");
    sLog.String("MM      IIIIIIIIIIIIIIIIIIIIIIIIMM                                          ");
    sLog.String("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM                                          ");

    sLog.String("");

    if(!sMaster.startUp())
        return 1;

    sObjectMgr.LoadSavedShows();

    HeaderExtractor* extractor = new HeaderExtractor();

    sLog.Success("Master", "Initialization compleet");
    sLog.Notice("Extractor", "Start extraction");

    if(HandleHttpRequest("www.gamebreaker.tv", "/", (Extractor*)extractor))
    {
        int i = 2;
        std::string link = "/page/2/";
        char number [4];
        while(HandleHttpRequest("www.gamebreaker.tv", link.c_str(), (Extractor*)extractor))
        {
            i++;
            link = "/page/";
            link += itoa(i, number, 10);
            link += "/";
        }

        sLog.Success("Extractor", "extracted %i indexpages", i);
    }
    delete extractor;


    if(uint32 newShows = sObjectMgr.ProccessNewShows())
    {
        sLog.Notice("Extractor", "Start grabing video data of %i new items.", newShows);
        sObjectMgr.GrabVideoLinkData();
        sObjectMgr.GrabFlagData();

        sObjectMgr.PushNotifications();
    }

    //Cleanup Functions. (Needs a timer?)
    sObjectMgr.CleanUp();

    sObjectMgr.SaveAll();
    seconds = time(NULL) - seconds;

    sLog.Success("MySQL", "saved all new data");
    sLog.Success("Master", "Extracted in %i seconds", seconds);

    return 0;
}