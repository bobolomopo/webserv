#ifndef HEADERS_HPP
# define HEADERS_HPP

# include <algorithm>
# include <arpa/inet.h>
# include <cstring>
# include <cstdio>
# include <dirent.h>
# include <errno.h>
# include <fstream>
# include <fcntl.h>
# include <iostream>
# include <map>
# include <netinet/in.h>
# include <unistd.h>
# include <sys/socket.h>
# include <string>
# include <stdio.h>
# include <stdlib.h>
# include <sstream>
# include <signal.h>
# include <sys/select.h>
# include <sys/time.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <sys/wait.h>
# include <vector>
# include <locale>


# include "../Srcs/Parser/Parser.hpp"
# include "Config.hpp"

std::vector<std::string> split(const std::string & s, char separator);
std::vector<std::string> split(const std::string & s);

//* COLORS
# define RED "\033[31m"
# define GREEN "\033[32m"
# define YELLOW "\033[33m"
# define CYANBG "\033[46m"
# define BLUE "\033[96m"
# define ORANGEBG "\033[1;44m"
# define PINKBG "\033[1;41m"
# define GREENBG "\033[42m"
# define PURPLEBG "\033[45m"
# define GRAYBG "\033[40m"
# define RESET "\033[0m"

# define MAX_CLIENTS 10
# define MAX_EVENTS 10
# define BUFFER_SIZE 1024

#endif