#include "Webserv.hpp"

volatile bool g_keepRunning = true;

Webserv::Webserv(Parser & parser) : _parser(parser) {};
Webserv::~Webserv() {};

void Webserv::run()
{
	std::cout << YELLOW << getTime() << "Démarrage..." << RESET << std::endl;
	std::cout << YELLOW << getTime() << "Config file : \"" << _parser.get_path_file() << "\"." << RESET << std::endl;

	FD_ZERO(&_connectionSockets);
	launchServers();

	std::map<int, Config> clientToServer;

	while (g_keepRunning) {
		readSockets = _connectionSockets;
		
		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		ssize_t selectRet = select(_maxSocket + 1, &readSockets, NULL, NULL, &timeout);

		if (selectRet < 0) {
			if (g_keepRunning == false)
				continue ;
			std::cerr << RED << "Error: select() failed: " << RESET << std::endl;
			exit(EXIT_FAILURE);
		}

		// On vérifie si un client tente de contacter un serveur
		for (std::map<int, Config>::iterator it = _servers.begin(); it != _servers.end(); it++) {
			if (FD_ISSET(it->first, &readSockets)) {
				std::cout << GREEN << getTime() << "New connection on " << it->second.get_host_name() << ":" << it->second.get_port() << RESET;
				int clientSocket = acceptNewClient(it->first);
				if (clientSocket < 0)
					continue ;
				clientToServer[clientSocket] = it->second;
				RequestHTTP clientRequest;
				_clients[clientSocket] = clientRequest;
				if (clientSocket > _maxSocket)
					_maxSocket = clientSocket;
				FD_SET(clientSocket, &_connectionSockets);
			}
		}

		// On gère les requêtes des clients qui se sont connectés
		for (std::map<int, RequestHTTP>::iterator it = _clients.begin(); it != _clients.end();) {
			if (FD_ISSET(it->first, &readSockets)) {
				int ret = handleRead(it->first, it->second);
				FD_CLR(it->first, &_connectionSockets);
				if (it->first == _maxSocket)
					_maxSocket -= 1;
				if (it->second.isOver()) {
					if (it->second.getHost().empty() == false) {
						sendResponse(it->first, it->second, clientToServer.at(it->first));
						clientToServer.erase(it->first);
					}
					std::cout << GREEN << getTime() << "Closing connection." << RESET << std::endl;
					close(it->first);
					it = _clients.erase(it);
				}
			}
			else {
				++it;
			}
		}
	}

	closeServers();
}

void Webserv::launchServers()
{
	std::vector<Config> serversConfig = _parser.get_vector_config();

	for (std::vector<Config>::iterator it = serversConfig.begin(); it != serversConfig.end(); it++) {
		std::cout << YELLOW << getTime() << "Launching server « " << it->get_server_name() << " »..." << RESET << std::endl;

		int serverSocket = initServerSocket(*it);
		_servers[serverSocket] = *it;

		FD_SET(serverSocket, &_connectionSockets);
		_maxSocket = serverSocket;

		std::cout << GREEN	<< getTime() << "Server successfuly launched. Listening on « " 
							<< it->get_host_name() << ":" << make_string(it->get_port()) << " »." << RESET << std::endl;
	}
}

void Webserv::closeServers()
{
	std::cout << YELLOW << getTime() << "Shutting down server(s)..." << RESET;

	for (std::map<int, Config>::iterator it = _servers.begin(); it != _servers.end(); it++)
		close(it->first);
}

// S'occupe de créer le socket avec l'addresse IP 
// ainsi que le port renseigné dans le fichier config.
// Le socket est réutilisable et non-bloquant.
int Webserv::initServerSocket( Config & serverConfig )
{
	int 				serverSocket;

	fcntl(serverSocket, F_SETFL, O_NONBLOCK);
	if ((serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		closeServers();
		throw (std::logic_error("Error: socket() failed"));
	}

	// Permet de réutiliser directement un socket lorsqu'on relance le programme.
	int optVal = 1;
	int optLen = sizeof(optVal);
	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &optVal, optLen) < 0) {
		closeServers();
		throw (std::logic_error("Error: setsockopt(SO_REUSEADDR) failed"));
	}

	struct sockaddr_in 	serverAddress;

	memset((char *)&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = serverConfig.get_host().s_addr;
	serverAddress.sin_port = htons(serverConfig.get_port());

	if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
		closeServers();
		throw (std::logic_error("Error: bind() failed"));
	}
	
	if (listen(serverSocket, MAX_CLIENTS) < 0) {
		closeServers();
		throw (std::logic_error("Error: listen() failed"));
	}

	return (serverSocket);
}

int Webserv::acceptNewClient( int serverSocket )
{
	int clientSocket;

	
	struct sockaddr_in 	clientAddress;
	socklen_t addressLen = sizeof(clientAddress);

	if ((clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &addressLen)) < 0) {
		if (errno != EWOULDBLOCK) {
			 std::cerr << RED << getTime() << "Error: Could not accept() client." << RESET << std::endl;
			 perror("accept");
			 return (-1);
		}
	}

	std::cout << GREEN << " [Client: " << inet_ntoa(clientAddress.sin_addr) << "]." << RESET << std::endl;


	if (fcntl(clientSocket, F_SETFL, O_NONBLOCK) < 0) {
		std::cerr << RED << "Error: fcntl() failed" << RESET << std::endl;
		return (-1);
	}

	return (clientSocket);
}

int Webserv::handleRead( int clientSocket, RequestHTTP & parsedRequest )
{
	char	request[BUFFER_SIZE + 1] = {0};
	ssize_t	recvRet = recv(clientSocket, request, BUFFER_SIZE, 0);
	
	parsedRequest.setRequest(request);

	//std::cout << "-------DEBUG-------\n" << parsedRequest.getRequest() << "\n-------------------\n";
	
	// Vérification que la requête est bien complète et terminée
	if (parsedRequest.getRequest().find("\r\n\r\n") != std::string::npos) {
		parsedRequest.setIsOver(true);
	}

	if (recvRet == 0 || recvRet == -1) {
		if (!recvRet)
			std::cerr << RED << getTime() << "Connection was closed by client." << RESET << std::endl;
		else
			std::cerr << RED << getTime() << "Read error, closing connection." << RESET << std::endl;
		return (-1);
	}

	if (parsedRequest.isOver()) {
		parsedRequest.parse();

		//std::cout << "-------DEBUG-------\n" << parsedRequest.getRequest() << "\n-------------------\n";

		std::cout	<<	YELLOW << getTime()
					<< "<< [Host: "	<< parsedRequest.getHost()		<< "] "
					<< "[Method: "	<< parsedRequest.getMethod()	<< "] "
					<< "[File : "	<< parsedRequest.getFile()		<< "]" << RESET << std::endl;
	}
	return (0);
}

void Webserv::sendResponse( int clientSocket, RequestHTTP & parsedRequest, Config & serverConfig )
{
		ResponseHTTP	response(serverConfig, parsedRequest);

		if (parsedRequest.getMethod() == "GET")
			response.GET(parsedRequest.getFile());
		else if (parsedRequest.getMethod() == "POST")
			response.POST(parsedRequest.getFile());
		else if (parsedRequest.getMethod() == "DELETE")
			response.DELETE(parsedRequest.getFile());
		else
			response.UNKNOWN(parsedRequest.getFile());


		if (write(clientSocket, response.getResponseHTTP().c_str(), response.getResponseHTTP().size()) > 0) {
			std::cout	<< YELLOW << getTime()
						<< ">> [Return Code: " << response.getStatusCode() << "] "
						<< "[Body Size: " << response.getBodySize() << "]"
						<< RESET << std::endl;
		}
		else
			std::cerr << RED << "Coulnd't respond to the client." << RESET << std::endl;
}

std::string Webserv::getTime() const
{
	char			buffer[32];
	struct timeval	tv;
	struct tm *		tm;

	gettimeofday(&tv, NULL);
	tm = localtime(&tv.tv_sec);
	strftime(buffer, 32, "[%H:%M:%S] ", tm);

	return (std::string(buffer));
}

int Webserv::checkEnd( const std::string & str, const std::string & end ) const
{
	size_t i = str.size();
	size_t j = end.size();

	while (j > 0) {
		i--;
		j--;
		if (i < 0 || str[i] != end[j])
			return (1);
	}

	return (0);
}

std::string Webserv::make_string(int n)
{
	std::stringstream 	ss;

	ss << n;
	return (ss.str());
}