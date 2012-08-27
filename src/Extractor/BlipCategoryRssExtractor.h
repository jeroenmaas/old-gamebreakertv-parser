#include <string>
#include "Extractor.h"

class BlipCategoryRssExtractor : public Extractor
{
public:
    virtual bool start(std::string website);

    void getShowInfo(int position);
    bool isCorrectStart(int& position);
    
    _inline void setFlag(int flag) { m_Flag = flag; }
    _inline int getFlag() { return m_Flag; }

private:
    int m_Flag;
};