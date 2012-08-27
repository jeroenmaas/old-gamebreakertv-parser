/* asyncHttpRequest.h */

#ifndef C2DM_MESSAGE_H
#define C2DM_MESSAGE_H

#include "stdafx.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>

class c2dmMessage
{
public:
    void initAndSend(std::string registrationId, uint32 showid, std::string title);

private:
    std::ostringstream m_Response; ///Unused currently
};

#endif

