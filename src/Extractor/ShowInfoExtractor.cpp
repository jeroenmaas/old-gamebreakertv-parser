#include "stdafx.h"
#include "ShowInfoExtractor.h"
#include "MysqlConnection.h"
#include "ObjectManager.h"
#include "Log.h"

//ShowInfo Extractor only extracts the link to the video currently
bool ShowInfoExtractor::start(std::string website)
{
    setWebsite(website);
    char * webstr = getWebsite();
    
    bool hasVideoLink = false;
    bool hasDescription = false;
    

    for(int i = 0; i < getSize(); i++)
    {
        switch (isCorrectStart(i))
        {
            case RESULT_VIDEOLINK:
            {
                getShowInfo(i);
                hasVideoLink = true;
                break;
            }
            case RESULT_DESCRIPTION:
            {
                getDescription(i);
                hasDescription = true;
                break;
            }
        }

        if(hasVideoLink && hasDescription)
                return true;
    }
    
    return false;
}

Results ShowInfoExtractor::isCorrectStart(int& position)
{
    const int bitTvSize = 20;
    char bitTvStr[bitTvSize+1] = "http://blip.tv/play/";

    const int descriptionSize = 11;
    char descriptionstr[descriptionSize+1] = "<div class=";
    const int description2Size = 12;
    char description2str[description2Size+1] = "post-content";

    
    //This method works but doesn't provide a valid UTF-8 link to work with. (in combination with rss)
    //const int YoutubeSize = 19;
    //char YoutubeStr[YoutubeSize+1] = "</param><embed src=";


    if(hasNext(bitTvStr, bitTvSize, position))
    {
        return RESULT_VIDEOLINK;
    }

    if(hasNext(descriptionstr, descriptionSize, position) && hasNext(description2str, description2Size, position + 12))
    {
        position+=descriptionSize+description2Size+10;
        return RESULT_DESCRIPTION;
    }

    /*
    if(hasNext(YoutubeStr, YoutubeSize, position))
    {
        position += YoutubeSize+1;
        return true;
    }*/

    return RESULT_NONE;
}

void ShowInfoExtractor::getShowInfo(int position)
{
    char* website = getWebsite();
    std::string VideoUrl;

    while(website[position] != 34) //"
    {
        VideoUrl += website[position];
        position++;
    }

    getInfo()->videourl = VideoUrl;
}

void ShowInfoExtractor::getDescription(int position)
{
    char* website = getWebsite();
    std::string Description;

    const int endSize = 25;
    char endstr[endSize+1] = "</div> <!-- post-item -->";

    const int fbBarSize = 39;
    char fbBar[fbBarSize+1] = "</p><p><span class='st_facebook_hcount'";

    while(!hasNext(endstr, endSize, position-1) && !hasNext(fbBar, fbBarSize, position-1))
    {
        Description += website[position-1];
        position++;
    }

    getInfo()->description = sObjectMgr.PrepareString(Description);
}