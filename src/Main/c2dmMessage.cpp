//TODO: Move client to a other file
//TODO: Find a way to get the whole responce message and check it.

#include "stdafx.h"
#include "c2dmMessage.h"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include "MysqlConnection.h"
#include "Log.h"

using boost::asio::ip::tcp;


/* Here is the http client we use for the c2dm Message. */
/* It works by using asio from boost framework and works async */

class client
{
public:
  client(boost::asio::io_service& io_service,
      const std::string& server, const std::string& path, const std::string& deviceId, const std::string& args)
    : resolver_(io_service),
      socket_(io_service),
      m_DeviceId(deviceId)
  {
    // Form the request. We specify the "Connection: close" header so that the
    // server will close the socket after transmitting the response. This will
    // allow us to treat all data up until the EOF as the content.
    std::ostream request_stream(&request_);
    request_stream << "POST " << path << " HTTP/1.0\r\n";
    request_stream << "Host: " << server << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Authorization: GoogleLogin auth=DQAAAMcAAAC7GAzk3rTWuCITSLbeQFQyecWmpHym2vWsLjtwoPMfPn95B8d_987U_SQdMWFysg6wdik2woRPgJ3MhXkHap17ROiZCJJxnvfbVtN63WFHhJIbfzmV9KWOW14hoXv30OMvN0HzgTgSPe9xwI-7nOXZtUK5mLpGO-WKK3vKwHiDJe78QKb40rdlk9x7-oRZdcAPP8WK720BHsVQH5UmhqpCqOmtjNTlOrzBVKZC9bmIJXDEIL3yBSefe1F5f5qQtTgod7czoVhA8h-ykXN8NP-0 \r\n";
    request_stream << "Content-Length: " << args.size() << "\r\n";
    request_stream << "Content-Type: application/x-www-form-urlencoded \r\n";
    request_stream << "\r\n";
    request_stream << args << "\r\n";
    request_stream << "Connection: close\r\n\r\n";

    // Start an asynchronous resolve to translate the server and service names
    // into a list of endpoints.
    tcp::resolver::query query(server, "http");
    resolver_.async_resolve(query,
        boost::bind(&client::handle_resolve, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::iterator));
  }

private:
  void handle_resolve(const boost::system::error_code& err,
      tcp::resolver::iterator endpoint_iterator)
  {
    if (!err)
    {
      // Attempt a connection to the first endpoint in the list. Each endpoint
      // will be tried until we successfully establish a connection.
      tcp::endpoint endpoint = *endpoint_iterator;
      socket_.async_connect(endpoint,
          boost::bind(&client::handle_connect, this,
            boost::asio::placeholders::error, ++endpoint_iterator));
    }
    else
    {
      std::cout << "Error: " << err.message() << "\n";
    }
  }

  void handle_connect(const boost::system::error_code& err,
      tcp::resolver::iterator endpoint_iterator)
  {
    if (!err)
    {
      // The connection was successful. Send the request.
      boost::asio::async_write(socket_, request_,
          boost::bind(&client::handle_write_request, this,
            boost::asio::placeholders::error));
    }
    else if (endpoint_iterator != tcp::resolver::iterator())
    {
      // The connection failed. Try the next endpoint in the list.
      socket_.close();
      tcp::endpoint endpoint = *endpoint_iterator;
      socket_.async_connect(endpoint,
          boost::bind(&client::handle_connect, this,
            boost::asio::placeholders::error, ++endpoint_iterator));
    }
    else
    {
      std::cout << "Error: " << err.message() << "\n";
    }
  }

  void handle_write_request(const boost::system::error_code& err)
  {
    if (!err)
    {
      // Read the response status line.
      boost::asio::async_read_until(socket_, response_, "\r\n",
          boost::bind(&client::handle_read_status_line, this,
            boost::asio::placeholders::error));
    }
    else
    {
      std::cout << "Error: " << err.message() << "\n";
    }
  }

  void handle_read_status_line(const boost::system::error_code& err)
  {
    if (!err)
    {
      // Check that response is OK.
      std::istream response_stream(&response_);
      std::string http_version;
      response_stream >> http_version;
      unsigned int status_code;
      response_stream >> status_code;
      std::string status_message;
      std::getline(response_stream, status_message);
      if (!response_stream || http_version.substr(0, 5) != "HTTP/")
      {
        std::cout << "Invalid response\n";
        return;
      }
      if (status_code != 200)
      {
        std::cout << "Response returned with status code ";
        std::cout << status_code << "\n";
        return;
      }

      // Read the response headers, which are terminated by a blank line.
      boost::asio::async_read_until(socket_, response_, "\r\n\r\n",
          boost::bind(&client::handle_read_headers, this,
            boost::asio::placeholders::error));
    }
    else
    {
      std::cout << "Error: " << err << "\n";
    }
  }

  void handle_read_headers(const boost::system::error_code& err)
  {
    if (!err)
    {
      // Process the response headers.
      std::istream response_stream(&response_);
      std::string header;
      while (std::getline(response_stream, header) && header != "\r")
        continue;

      // Write whatever content we already have to output.
      if (response_.size() > 0)
      {
          m_Response << &response_;
          processData();
      }

      // Start reading remaining data until EOF.
      boost::asio::async_read(socket_, response_,
          boost::asio::transfer_at_least(1),
          boost::bind(&client::handle_read_content, this,
            boost::asio::placeholders::error));
    }
    else
    {
      std::cout << "Error: " << err << "\n";
    }
  }

  void handle_read_content(const boost::system::error_code& err)
  {
    if (!err)
    {
      // Write all of the data that has been read so far.
      m_Response << &response_;
      processData();

      // Continue reading remaining data until EOF.
      boost::asio::async_read(socket_, response_,
          boost::asio::transfer_at_least(1),
          boost::bind(&client::handle_read_content, this,
            boost::asio::placeholders::error));
    }
    else if (err != boost::asio::error::eof)
    {
      std::cout << "Error: " << err << "\n";
    }
  }

  void processData()
  {
        std::string responceStr = m_Response.str();
        if(boost::starts_with(responceStr.c_str(), "Error="))
        {
            sLog.String("c2dm", responceStr.c_str());
            std::string error = boost::replace_first_copy(responceStr, "Error=", "");

            if (!strcmp(error.c_str(), "InvalidRegistration"))
            {
                sLog.Error("c2dm", "invalid reg");
                sDatabase.PExecute("DELETE FROM android_devices WHERE deviceid = '%s'", m_DeviceId.c_str());
            }
        }else
            sLog.String("c2dm", "message send successfully");
  }

  tcp::resolver resolver_;
  tcp::socket socket_;
  boost::asio::streambuf request_;
  boost::asio::streambuf response_;
  std::ostringstream m_Response; //m_ prefix to show we handle this :D
  std::string m_DeviceId;
};

void c2dmMessage::initAndSend(std::string registrationId, uint32 showid, std::string title)
{
    std::string postContent;
    postContent += "registration_id=";
    postContent += registrationId;
    postContent += "&collapse_key=";
    postContent += boost::lexical_cast<std::string>(showid);
    postContent += "&data.id=";
    postContent += boost::lexical_cast<std::string>(showid);
    postContent += "&data.title=";
    postContent += title;
    
    boost::asio::io_service iohandler;
    client c(iohandler, "android.apis.google.com", "/c2dm/send",registrationId, postContent.c_str() );
    iohandler.run();
}