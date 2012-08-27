#include "stdafx.h"
#include "BlipEmbededRssExtractor.h"
#include "MysqlConnection.h"
#include "ObjectManager.h"
#include "Log.h"
#include <boost/lexical_cast.hpp>

bool BlipEmbededRssExtractor::start(std::string website)
{
    setWebsite(website);
    char * webstr = getWebsite();

    for(int i = 0; i < getSize(); i++)
    {
        switch(isCorrectStart(i))
        {
            case 1:
                getBlipTitle(i);
                break;

            case 2:
                getVideoData(i);
                break;
        }
    }
    
    return true;
}

//We use is correct start to skip the non important stuff
int BlipEmbededRssExtractor::isCorrectStart(int& position)
{
    const int titleSize = 7;
    char titleStr[titleSize+1] = "<title>";

    if(hasNext(titleStr, titleSize, position))
    {
        position += titleSize;
        return 1; //Title
    }

    const int mediaSize = 7;
    char mediaStr[mediaSize+1] = "<media>";

    if(hasNext(mediaStr, mediaSize, position))
    {
        position += mediaSize;
        return 2; //Media
    }

    return 0;
}

void BlipEmbededRssExtractor::getBlipTitle(int position)
{
    char* website = getWebsite();
    std::string title;

    while(website[position] != 60) //<
    {
        title += website[position];
        position++;
    }

    getInfo()->blipVideoTitle = sObjectMgr.PrepareString(title);
}

void BlipEmbededRssExtractor::getVideoData(int position)
{
    VideoData data = VideoData();
    char* website = getWebsite();
    std::string videoLink;

    const int roleSize = 6;
    char roleStr[roleSize+1] = "<role>";

    data.role = getContentAterItem(roleStr, roleSize, position);

    const int linkSize = 5;
    char linkStr[linkSize+1] = "href=";

    while(!hasNext(linkStr, linkSize, position)) 
        position++;

    position += linkSize+1; //+1 to skip "

    while(website[position] != 34) //"
    {
        videoLink += website[position];
        position++;
    }
    data.link = videoLink;

    const int widthSize = 7;
    char widthStr[widthSize+1] = "<width>";
    data.videoWidth = boost::lexical_cast< int >( getContentAterItem(widthStr, widthSize, position) );

    const int heightSize = 8;
    char heightStr[heightSize+1] = "<height>";
    data.videoLength = boost::lexical_cast< int >( getContentAterItem(heightStr, heightSize, position) );

    if(getInfo()->videos == 0) //First video or aka source
    {
        const int durationSize = 10;
        char durationStr[durationSize+1] = "<duration>";
        getInfo()->videoLength = boost::lexical_cast< int >( getContentAterItem(durationStr, durationSize, position) );
    }

    const int sizeStringSize = 6;
    char sizeStr[sizeStringSize+1] = "<size>";
    data.fileSize = boost::lexical_cast< int >( getContentAterItem(sizeStr, sizeStringSize, position) );

    const int audioSize = 14;
    char audioStr[audioSize+1] = "<audiobitrate>";
    data.audiobitrate = boost::lexical_cast< int >( getContentAterItem(audioStr, audioSize, position) );

    const int videoSize = 14;
    char videoStr[videoSize+1] = "<videobitrate>";
    data.videobitrate = boost::lexical_cast< int >( getContentAterItem(videoStr, videoSize, position) );

    getInfo()->video_Data[getInfo()->videos] = data;
    getInfo()->videos++;
}

std::string BlipEmbededRssExtractor::getContentAterItem(char* item, int itemsize, int position)
{
    char* website = getWebsite();
    std::string new_string;
    while(!hasNext(item, itemsize, position))
        position++;

    position += itemsize;

    while(website[position] != 60) //<
    {
        new_string += website[position];
        position++;
    }

    return new_string;
}