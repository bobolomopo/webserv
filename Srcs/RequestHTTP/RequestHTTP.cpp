#include "RequestHTTP.hpp"

// Public
RequestHTTP::RequestHTTP() 
{
	_isOver = false;
}
RequestHTTP::RequestHTTP(RequestHTTP const & cpy) {	*this = cpy; };
RequestHTTP::~RequestHTTP() {};

RequestHTTP & RequestHTTP::operator=( RequestHTTP const & rhs )
{
	_body = rhs._body;
	_file = rhs._file;
	_method = rhs._method;
	_accept = rhs._accept;
	_host = rhs._host;
	_transferEncoding = rhs._transferEncoding;
	_isOver = rhs._isOver;

	return (*this);
};

void RequestHTTP::setRequest( std::string request ) { _request += request; };

void RequestHTTP::parse()
{
	std::vector<std::string> headers;

	headers = split(_request, '\n');

	std::vector<std::string> firstLine = split(*headers.begin(), ' ');
	_method = str_toupper(firstLine[0]);
	_file = firstLine[1];

	for (std::vector<std::string>::iterator it = headers.begin()++ ; it != headers.end() ; it++)
	{
		parseKeyword(*it);
		if ((*it).size() == 1 && isspace((*it).back()))
			_body = *(++it);
	}
}

bool RequestHTTP::isOver() { return (_isOver); };

std::string					RequestHTTP::getFile()				{ return (_file);				};
std::string					RequestHTTP::getBody()				{ return (_body);				};
std::string					RequestHTTP::getMethod()			{ return (_method);				};
std::vector<std::string>	RequestHTTP::getAccept()			{ return (_accept);				};
std::string					RequestHTTP::getHost()				{ return (_host);				};
std::string					RequestHTTP::getTransferEncoding()	{ return (_transferEncoding);	};

// Private
void RequestHTTP::parseKeyword(std::string line)
{
	if (line.find("Accept:") != std::string::npos)
			_accept = split(split(line, ' ')[1], ',');
	else if (line.find("Host:") != std::string::npos)	
		_host = split(line, ' ')[1];
	else if (line.find("Transfer-Encoding:") != std::string::npos)
		_transferEncoding = split(line, ' ')[1];
	else if (line.find("Content-Length") != std::string::npos)
		_contentLength = split(line, ' ')[1];
}

std::string RequestHTTP::str_toupper( std::string s )
{
	std::locale loc;
	std::string result = s;

  	for (std::string::size_type i = 0; i < s.length(); ++i)
    	result[i] = std::toupper(s[i],loc);
  	return (result);
}
