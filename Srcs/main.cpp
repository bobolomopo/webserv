#include "Webserv/Webserv.hpp"
#include "../Includes/Config.hpp"

extern volatile bool g_keepRunning;

void INThandler(int signum)
{
	(void)signum;
	g_keepRunning = false;
}

int main(int argc, char * argv[])
{
	struct sigaction sigIntHandler;

	sigIntHandler.sa_handler = INThandler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;

	sigaction(SIGINT, &sigIntHandler, NULL);

	Parser	parser;

	if (argc >= 2)
		parser.set_path_file(argv[1]);
	
	parser.parse();
	Webserv server(parser);
	try	{
		server.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what();
	}
}