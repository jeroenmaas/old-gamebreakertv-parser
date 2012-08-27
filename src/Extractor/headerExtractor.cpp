#include "stdafx.h"
#include "headerExtractor.h" 
#include "MysqlConnection.h"
#include "ObjectManager.h"
#include <boost/algorithm/string.hpp>

bool HeaderExtractor::start(std::string website)
{
    setWebsite(website);
    char * webstr = getWebsite();
    
    bool foundNewShow = false;

    for(int i = 0; i < getSize(); i++)
    {
        if(webstr[i] == 60) //<
        {
            if(isCorrectStart(i))
            {
                foundNewShow = true;
                getShowInfo(i);
            }
        }
    }
    
    return foundNewShow; 
}

bool HeaderExtractor::isCorrectStart(int position)
{
    const int lastSize = 6;
    char Last[lastSize+1] = "</div>";

    const int nextSize = 12;
    char next[nextSize+1] = "<h2><a href=";

    return (hasPrevious(Last, lastSize, position) && hasNext(next, nextSize, position));
}

void HeaderExtractor::getShowInfo(int position)
{
    //13 is the size from <h2><a href= thats in front of the link
    int currentPosition = position + 13;
    int startPosition = currentPosition;
    char* website = getWebsite();
    std::string pictureurl;
    std::string largepictureurl;
    std::string url;
    std::string titel;
    std::string date;
    std::string category;
    std::string comment;
    int commentcount;

    /* Grab picture url */

    const int srcSize = 4;
    char srcStr[srcSize+1] = "src=";
    while(!hasNext(srcStr, srcSize, currentPosition))
    {
         currentPosition--;
    }

    currentPosition += 5;

    const int picSize = 12;
    char picStr[picSize+1] = "-180x130.jpg";
    while(website[currentPosition] != 34) //"
    {
        if(hasNext(picStr, picSize, currentPosition))
        {
            currentPosition += picSize -1;
            pictureurl += "-180x130.jpg";
            largepictureurl += ".jpg";
            break;
        }

        largepictureurl += website[currentPosition];
        pictureurl += website[currentPosition];
        currentPosition++;
    }

    currentPosition = startPosition;

    /* Grab Url */

    while(website[currentPosition] != 34) //"
    {
        url += website[currentPosition];
        currentPosition++;
    }

    /* Grab Titel */

    //Go further until we're at the title information. And father until next "
    currentPosition += 9; //Skip " title="
    while(website[currentPosition] != 34)
    {
         titel += website[currentPosition];
         currentPosition++;
    }

    titel = sObjectMgr.PrepareString(titel);

    /* Grab Date */

    const int datesize = 10;
    char liClass[datesize+1] = "<li class=";
    while(!hasNext(liClass, datesize, currentPosition))
    {
         currentPosition++;
    }

    currentPosition += datesize + 7; //Skip <li class="date">

    while(website[currentPosition] != 60)
    {
        date += website[currentPosition];
        currentPosition++;
    }

    //Unfortunally because of a site update we can't grab this like we used to :S
    int categoryFlag = 0; 
    commentcount = 0; 

    /* Grab Category */

    /*
    int start = 0;
    int end = 0;
    while(!hasNext(liClass, datesize, currentPosition))
    {
         currentPosition++;
    }

    start = currentPosition;
    currentPosition++;

    while(!hasNext(liClass, datesize, currentPosition))
    {
         currentPosition++;
    }

    end = currentPosition;
    currentPosition = start;

    
    while(currentPosition < end)
    {
        const int tagSize = 12;
        char tagStr[tagSize+1] = "category tag";
        while(!hasNext(tagStr, tagSize, currentPosition) && currentPosition < end )
        {
             currentPosition++;
        }

        if( currentPosition < end )
        {
            currentPosition += tagSize+2;  //category tag">

            while(website[currentPosition] != 60)
            {
                category += website[currentPosition];
                currentPosition++;
            }
            categoryFlag += sObjectMgr.GetFlagFromCategory(category);

            category.clear();
        }
    }*/

    ShowInfo* show = new ShowInfo(pictureurl, largepictureurl, url, titel, date, categoryFlag, commentcount);
    sObjectMgr.AddShow(show);

    return;
}