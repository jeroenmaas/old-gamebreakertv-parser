/* HttpRequest.cpp */

#include "stdafx.h"
#include "httpRequest.h"
#include "Extractor.h"
#include <boost/asio.hpp>
#include "Log.h"

using boost::asio::ip::tcp;

bool HandleHttpRequest(char* host, const char* location, Extractor* extractor, bool return_result_only)
{
    try
    {
        boost::asio::io_service io_service;

        // Get a list of endpoints corresponding to the server name.
        tcp::resolver resolver(io_service);
        tcp::resolver::query query(host, "http");
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
        tcp::resolver::iterator end;

        // Try each endpoint until we successfully establish a connection.
        tcp::socket socket(io_service);
        boost::system::error_code error = boost::asio::error::host_not_found;
        while (error && endpoint_iterator != end)
        {
            socket.close();
            socket.connect(*endpoint_iterator++, error);
        }
        if (error)
            throw boost::system::system_error(error);

        // Form the request. We specify the "Connection: close" header so that the
        // server will close the socket after transmitting the response. This will
        // allow us to treat all data up until the EOF as the content.
        boost::asio::streambuf request;
        std::ostream request_stream(&request);
        request_stream << "GET " << location << " HTTP/1.0\r\n";
        request_stream << "Host: " << host << "\r\n";
        request_stream << "Accept: */*\r\n";
        request_stream << "Connection: close\r\n\r\n";

        // Send the request.
        boost::asio::write(socket, request);

        // Read the response status line.
        boost::asio::streambuf response;
        boost::asio::read_until(socket, response, "\r\n");

        // Check that response is OK.
        std::istream response_stream(&response);
        std::string http_version;
        response_stream >> http_version;
        unsigned int status_code;
        response_stream >> status_code;
        std::string status_message;
        std::getline(response_stream, status_message);
        if (!response_stream || http_version.substr(0, 5) != "HTTP/")
        {
            sLog.Warning("HttpHandler", "Invalid response on location %s", location);
            return false;
        }

        if (status_code != 200)
        {
            sLog.Warning("HttpHandler", "Response on %s returned with status code %u", location, status_code);
            return false;
        }else if (return_result_only)
            return true;

        // Read the response headers, which are terminated by a blank line.
        boost::asio::read_until(socket, response, "\r\n\r\n");

        std::ostringstream ss;

        // Read until EOF, writing data to output as we go.
        while (boost::asio::read(socket, response,
                boost::asio::transfer_at_least(1), error))
        {
            ss << &response;
        }

        std::string s = ss.str();
        bool results = extractor->start(s);
        sLog.Debug("Extractor", "Extracted info from page %s", location);
        return results;

        if (error != boost::asio::error::eof)
            throw boost::system::system_error(error);
    }
    catch (std::exception& e)
    {
        sLog.Error("HttpHandler", "Exception: %s",e.what());
    }

    return true;
}