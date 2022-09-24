#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include "../../Includes/headers.hpp"
# include "../RequestHTTP/RequestHTTP.hpp"
# include "../ResponseHTTP/ResponseHTTP.hpp"

class Webserv
{
	public:
		Webserv(Parser & parser);
		~Webserv();

		void		run();
		std::string	make_string(int n);

	private:
		void		launchServers();
		void 		closeServers();
		int			initServerSocket( Config & serverConfig );
		int			handleRead( int clientSocket, RequestHTTP & parsedRequest );
		void 		sendResponse(int clientSocket, RequestHTTP & parsedRequest, Config & serverConfig );
		int			acceptNewClient( int serverSocket );

		// Utils
		std::string getTime() const;
		int 		checkEnd( const std::string & str, const std::string & end ) const;

	private:
		Parser						_parser;
		std::map<int, Config>		_servers;
		fd_set						_connectionSockets, readSockets;
		int							_maxSocket;
		std::map<int, RequestHTTP>	_clients;

};


#endif