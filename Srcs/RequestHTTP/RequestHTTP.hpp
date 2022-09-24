#ifndef REQUESTHTTP_HPP
# define REQUESTHTTP_HPP

# include "../../Includes/headers.hpp"

class RequestHTTP
{
	public:
		RequestHTTP();
		~RequestHTTP();
		RequestHTTP(RequestHTTP const & cpy);
		RequestHTTP & operator=(RequestHTTP const & rhs);
		
		void parse();
		bool isOver();

		void setRequest( std::string request );
		void setIsOver( bool value ) { _isOver = value; };

		std::string getRequest() { return (_request); };


		// Getters
		std::string					getFile();
		std::string					getBody();
		std::string					getMethod();
		std::vector<std::string>	getAccept();
		std::string					getHost();
		std::string					getTransferEncoding();

	private:
		std::string					_request;
		std::string					_file;
		std::string					_body;
		std::string					_method;
		std::vector<std::string>	_accept;
		//std::vector<std::string	_accept-language;
		std::string					_transferEncoding;
		std::string					_contentLength;
		std::string					_host;
		bool						_isOver;

		void parseKeyword( std::string line );
		std::string str_toupper( std::string s );
};

#endif