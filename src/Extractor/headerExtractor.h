#include "Extractor.h"

class HeaderExtractor : public Extractor
{
public:
    virtual bool start(std::string website);
    void getShowInfo(int position);
    bool isCorrectStart(int position);
};