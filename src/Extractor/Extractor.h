/*  Header file of extractor.cpp
    What else to write here? :P */

#ifndef _EXTRACTOR_H
#define _EXTRACTOR_H

class ObjectManager;

class Extractor
{
public:
    Extractor() { m_website = 0; }
    ~Extractor() {}
    virtual bool start(std::string website) { return false; }

    bool hasNext(char * text, int textsize, int position);
    bool hasPrevious(char * text, int textsize, int position);

    void setWebsite(std::string website);

    _inline char * getWebsite() { return m_website; }
    _inline int getSize() { return m_size; }

private: 
    char* m_website;
    int m_size;
};

#endif _EXTRACTOR_H