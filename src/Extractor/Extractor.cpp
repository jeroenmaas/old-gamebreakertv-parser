/*  Extractor.cpp
    Extracts all information from the string it gets */

#include "stdafx.h"
#include "Extractor.h"
#include "ObjectManager.h"

void Extractor::setWebsite(std::string website)
{
    if(m_website)
        delete m_website;

    char * cstr; 
    cstr = new char [website.size()+1];
    strcpy (cstr, website.c_str());

    m_website = cstr;
    m_size = website.size();
}

bool Extractor::hasPrevious(char * text, int textsize, int position)
{
    char* website = getWebsite();
    for(int i = position-1; position >= 0; i--)
    {
        if(website[i] > 32) //A real character not spacebar and other crap
        {
            for(int y = 0; y < textsize; y++)
            {
                if(text[textsize-1-y] != website[i - y])
                    return false;
            }

            return true;
        }
    }
    return false;
}
	
bool Extractor::hasNext(char * text, int textsize, int position)
{
    char* website = getWebsite();
    for(int i = position; i - position < textsize; i++)
    {
        if(website[i] != text[i-position])
            return false;
    }

    return true;
}
	