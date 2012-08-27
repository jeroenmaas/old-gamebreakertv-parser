#include "stdafx.h"
#include "BlipCategoryRssExtractor.h"
#include "MysqlConnection.h"
#include "ObjectManager.h"

//ShowInfo Extractor only extracts the link to the video currently
bool BlipCategoryRssExtractor::start(std::string website)
{
    setWebsite(website);
    char * webstr = getWebsite();

    for(int i = 0; i < getSize(); i++)
    {
        if(webstr[i] == 60) //<
        {
            if(isCorrectStart(i))
            {
                getShowInfo(i);
            }
        }
    }
    
    return true;
}

bool BlipCategoryRssExtractor::isCorrectStart(int& position)
{
    const int titleSize = 7;
    char titleStr[titleSize+1] = "<title>";

    
    //This method works but doesn't provide a valid UTF-8 link to work with. (in combination with rss)
    //const int YoutubeSize = 19;
    //char YoutubeStr[YoutubeSize+1] = "</param><embed src=";


    if(hasNext(titleStr, titleSize, position))
    {
        position += titleSize;
        return true;
    }

    return false;
}

void BlipCategoryRssExtractor::getShowInfo(int position)
{
    char* website = getWebsite();
    std::string title;

    while(website[position] != 60) //<
    {
        title += website[position];
        position++;
    }

    std::string newTitle = sObjectMgr.PrepareString(title);

    ShowInfo* sInfo = sObjectMgr.GetShowByBlipTitle(newTitle);

    if(sInfo)
        sInfo->categoryFlag += getFlag();
}