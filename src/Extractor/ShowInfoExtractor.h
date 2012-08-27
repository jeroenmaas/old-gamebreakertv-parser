#include "Extractor.h"

struct ShowInfo;

enum Results
{
    RESULT_VIDEOLINK = 0,
    RESULT_DESCRIPTION = 1,
    RESULT_NONE = 2,
};

class ShowInfoExtractor : public Extractor
{
public:
    virtual bool start(std::string website);

    void getShowInfo(int position);
    void getDescription(int position);
    Results isCorrectStart(int& position);

    __inline void setShow(ShowInfo* show) { m_info = show; }
    __inline ShowInfo* getInfo() { return m_info; }

private:
    ShowInfo* m_info;
};