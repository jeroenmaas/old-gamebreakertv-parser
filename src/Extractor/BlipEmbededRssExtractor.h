#include <string>
#include "Extractor.h"

struct ShowInfo;

class BlipEmbededRssExtractor : public Extractor
{
public:
    virtual bool start(std::string website);

    void getBlipTitle(int position);
    void getVideoData(int position);
    int isCorrectStart(int& position);

    std::string getContentAterItem(char* item, int itemsize, int position);

    __inline void setShow(ShowInfo* show) { m_info = show; }
    __inline ShowInfo* getInfo() { return m_info; }

private:
    ShowInfo* m_info;
};