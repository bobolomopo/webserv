#include "ResponseHTTP.hpp"

ResponseHTTP::ResponseHTTP(Config config, RequestHTTP request) : _config(config), _request(request)
{
	_getLocationFirst = true;
	initDirectives();
	initStatusCode();
}

ResponseHTTP::~ResponseHTTP() {};

std::string ResponseHTTP::getStatusCode()	{ return (_statusCode); }
std::string ResponseHTTP::getBodySize()		{ return (make_string(_body.size())); }
std::string ResponseHTTP::getResponseHTTP() { return (_statusLine + _headers + _body); }

void ResponseHTTP::GET(std::string path)
{	
	std::ifstream check_file;
	_directives["Date"] = getDate();

	std::string newPath = isThereRoot(split(path, '/'), 1, _config.get_location());
	if (newPath.empty() == false) {
		path = newPath;
	}

	if (checkConfigRules(path, "GET"))
		return ;


	// Permet de vérifier si le path est un dossier ou un fichier
	struct stat buf;
	std::string tmp = _config.get_root() + '/' + path;
	stat(tmp.c_str(), &buf);
	std::vector<std::string > vecPath = split(path, '/');
	vecPath.erase(vecPath.begin());
	std::string cgi_pass = checkCgiPass(split(path, '/'), _config.get_location(), 1);
	if ((_config.get_autoindex() || (path != "/" && checkAutoIndex(vecPath, _config.get_location(), 0))) && S_ISDIR(buf.st_mode)) {
		generateAutoIndex(path);
		_statusCode = generateStatusCode(200);
	}
	else if (_request.getFile().substr(0, 8) == "/cgi-bin" || cgi_pass.empty() == false) 
	{
		std::string execute_file = "";
		if (_request.getFile().substr(0, 8) == "/cgi-bin")
			execute_file += path;
		else
			execute_file += cgi_pass;
		check_file.open(_config.get_root() + execute_file);
		if (check_file.is_open())
		{
			CGIhandler cgi(_request, _config, execute_file);
			cgi.init_env();
			cgi.execute_CGI_POST();
			this->_body = cgi.get_body();
			if (cgi.get_status_code() == 500)
				createError(500);
			_statusCode = generateStatusCode(cgi.get_status_code());
			_directives["Content-Type"] = "text/html";
			_directives["Content-Length"] = make_string(_body.size());
		}
		else
		{
			_statusCode = generateStatusCode(404);
			if(createError(404))
			{
				_directives["Content-Type"] = "text/html";
				_body = "<!doctype html><html><head><title>404</title></head><body><p><strong>Error : </strong>404 Not Found.</p></body></html>";
				_directives["Content-Length"] = make_string(_body.size());
			}
		}
	}
	else
		generateBody(path);
	createStatusLine();
	createHeaders();
}

void ResponseHTTP::POST(std::string path)
{	
	std::ifstream check_file;
	std::ofstream file;

	_directives["Date"] = getDate();

	if (checkConfigRules(path, "POST"))
		return ;

	_statusCode = generateStatusCode(204);
	std::string cgi_pass = checkCgiPass(split(path, '/'), _config.get_location(), 1);
	if (_request.getFile().substr(0, 8) == "/cgi-bin" || cgi_pass.empty() == false)
	{
		std::string execute_file = "";
		if (_request.getFile().substr(0, 8) == "cgi-bin")
			execute_file += path;
		else
			execute_file += cgi_pass;
		CGIhandler cgi(_request, _config, execute_file);
		cgi.init_env();
		cgi.execute_CGI_POST();
		this->_body = cgi.get_body();
		_statusCode = generateStatusCode(cgi.get_status_code());
		_directives["Content-Type"] = "text/html";
		_directives["Content-Length"] = make_string(_body.size());
	}
	else
	{
		check_file.open(_config.get_root() + path);
		if (check_file && _request.getBody().size() > 0)
			_statusCode = generateStatusCode(200);
		else if (_request.getBody().size() > 0)
			_statusCode = generateStatusCode(201);
		check_file.close();

		//open et rajoute le body au fichier
		file.open(_config.get_root() + path, std::ios_base::app);
		file << _request.getBody();
		file.close();
	}
	createStatusLine();
	createHeaders();
}

void ResponseHTTP::DELETE(std::string path)
{
	_directives["Date"] = getDate();

	if (checkConfigRules(path, "DELETE"))
		return ;

	deleteFile(path);
	createStatusLine();
	createHeaders();
}

void ResponseHTTP::UNKNOWN(std::string path)
{
	_directives["Date"] = getDate();
	_statusCode = generateStatusCode(405);

	createStatusLine();
	createHeaders();
}

// Private
void ResponseHTTP::initDirectives()
{
	_directives["Allow"] = "";
	_directives["Content-Language"] = "";
	_directives["Content-Length"] = "0";
	_directives["Content-Location"] = "";
	_directives["Content-Type"] = "";
	_directives["Date"] = "";
	_directives["Last-Modified"] = "";
	_directives["Connection"] = "";
	_directives["Location"] = "";
	_directives["Retry-After"] = "";
	_directives["Server"] = "Webserv";
	_directives["Transfer-Encoding"] = "identity";
	_directives["WwwAuthenticate"] = "";
	_directives["Set-Cookie"] = "";
}

void ResponseHTTP::initStatusCode()
{
	// 1xx (Informational)
	_statusCodes[100] = "Continue";
	_statusCodes[101] = "Switching Protols";

	// 2xx (Successful)
	_statusCodes[200] = "OK";
	_statusCodes[201] = "Created";
	_statusCodes[202] = "Accepted";
	_statusCodes[203] = "Non-Authorative Information";
	_statusCodes[204] = "No Content";
	_statusCodes[205] = "Reset Content";
	_statusCodes[206] = "Partial Content";

	// 3xx (Redirection)
	_statusCodes[301] = "Moved Permanently";
	_statusCodes[302] = "Found";
	_statusCodes[303] = "See Other";
	_statusCodes[304] = "Not Modified";
	_statusCodes[305] = "Use Proxy";
	_statusCodes[307] = "Temporary Redirect";

	// 4xx (Client Error)
	_statusCodes[400] = "Bad Request";
	_statusCodes[401] = "Unauthorized";
	_statusCodes[402] = "Bad Request";
	_statusCodes[403] = "Forbidden";
	_statusCodes[404] = "Not Found";
	_statusCodes[405] = "Method Not Allowed";
	_statusCodes[406] = "Not Acceptable";
	_statusCodes[407] = "Proxy Authentification Required";
	_statusCodes[408] = "Request Timeout";
	_statusCodes[409] = "Conflict";
	_statusCodes[410] = "Gone";
	_statusCodes[411] = "Length Required";
	_statusCodes[412] = "Precondition Failed";
	_statusCodes[413] = "Payload Too Large";
	_statusCodes[414] = "URI Too Long";
	_statusCodes[415] = "Unsupported Media Type";
	_statusCodes[416] = "Range Not Satisfiable";
	_statusCodes[417] = "Expectation Failed";
	_statusCodes[426] = "Upgrade Required";

	// 5xx (Server Error)
	_statusCodes[500] = "Internal Server Error";
	_statusCodes[501] = "Not Implemented";
	_statusCodes[502] = "Bad Gateway";
	_statusCodes[503] = "Service Unavailable";
	_statusCodes[504] = "Gateway Timeout";
	_statusCodes[505] = "HTTP Version Not Supported";
}

std::string ResponseHTTP::generateStatusCode(int statusCode)
{
	return (make_string(statusCode) + " " + _statusCodes[statusCode]);
}

void ResponseHTTP::createStatusLine()
{
	_statusLine = "HTTP/1.1 ";
	_statusLine += _statusCode;
	_statusLine += "\n";
}

void ResponseHTTP::createHeaders()
{
	_headers += "Date: " 			+ _directives["Date"] + "\n";
	_headers += "Server: "			+ _directives["Server"] + "\n";
	_headers += "Content-Length: "	+ _directives["Content-Length"] + "\n";

	if (_directives["Content-Type"] != "")
		_headers += "Content-Type: "	+ _directives["Content-Type"] + "\n";
	if (_directives["Location"].empty() == false)
		_headers += "Location: "		+ _directives["Location"] + "\n";

	_headers += "\n";
}

int ResponseHTTP::createError(int statusCode)
{
	if (!_config.get_error_page()[statusCode].empty())
	{
		std::stringstream	buffer;
		std::string path = _config.get_error_page()[statusCode];
		std::ifstream errorFile;
		errorFile.open(path);
		if(!errorFile.is_open()) {
			std::cout << RED << "Error: error_page: " << path << " do not exist.\n" << RESET << std::endl;
		}
		buffer << errorFile.rdbuf();
		errorFile.close();
		_body = buffer.str();
		_directives["Content-Length"] = std::to_string(_body.size());
		return(1);
	}
	return(0);
}

bool ResponseHTTP::checkConfigRules(std::string path, std::string method)
{
	if (isThereReturn(_config.get_root() + path))
		return (true);

	std::vector<std::string> splittedPath = split(path, '/');
	std::vector<std::string> lowestLevel = _config.get_methods();

	lowestLevel = isAllowedMethod(splittedPath, 1, _config.get_location(), method, lowestLevel);

	if (std::find(lowestLevel.begin(), lowestLevel.end(), method) == lowestLevel.end()) {
		_statusCode = generateStatusCode(405);
		if (!createError(405)) {
			createStatusLine();
			createHeaders();
		}
		return (true);
	}
	if (_config.get_client_max_body_size() != 0 && _config.get_client_max_body_size() < _request.getBody().size()) {
		_statusCode = generateStatusCode(413);
		if (!createError(413)) {
			createStatusLine();
			createHeaders();
		}
		return (true);
	}

	return (false);
}

void ResponseHTTP::generateAutoIndex(std::string path)
{
	std::string dirPath = _config.get_root() + path;
	DIR * dir 			= opendir(dirPath.c_str());
	std::string	index	= "<!DOCTYPE html><html><head><title>" + dirPath + "</title></head><body><h1>Index of " + path + "</h1><hr>";

	//std::cout << "Current working directory: " << dirPath << std::endl;

	// Comment entrer là dedans ?
	if (dir == NULL) {
		std::cout << RED << "GenerateAutoindex(): Could not open \"" << dirPath << "\"\n"  << RESET;
		_statusCode = generateStatusCode(404);
		_directives["Content-Type"] = "text/html";
		_body = "<!doctype html><html><head><title>404</title></head><body><p><strong>Error : </strong>404 Not Found.</p></body></html>";
		_directives["Content-Length"] = make_string(_body.size());

		return ;
	}

	for (struct dirent *dirEntry = readdir(dir); dirEntry; dirEntry = readdir(dir)) {
		std::stringstream ss;
		if (path != "/")
			ss << "<li><a href=\"" << path << "/" << dirEntry->d_name << "\">" << dirEntry->d_name << "</a></li>\n";
		else
			ss << "<li><a href=\"" << dirEntry->d_name << "\">" << dirEntry->d_name << "</a></li>\n";
		index += ss.str();
		ss.clear();
	}

	index += "<hr></body></html>";
	closedir(dir);

	_body = index;
	_directives["Content-Length"] = make_string(_body.size());
}

void ResponseHTTP::generateBody(std::string path)
{
	if (path.find(_config.get_root()) != std::string::npos){
		path = path.substr(path.find(_config.get_root()) + _config.get_root().size());
		if (path.empty())
			path = "/";
		//std::cout << "path: " << path << "\n";
	}

	struct stat buf;
	std::string tmp = _config.get_root() + '/' + path;
	stat(tmp.c_str(), &buf);

	if (path == "/" || S_ISDIR(buf.st_mode)) {
		std::string index = isThereIndex(split(path, '/'), _config.get_location(), 1);
		if (index.empty())
			path += _config.get_index();
		else
			path += "/" + index;
	}

	std::ifstream		requested_file(_config.get_root() + path);
	std::stringstream	buffer;

	if (requested_file.is_open()) {
		_statusCode = generateStatusCode(200);
		buffer << requested_file.rdbuf();
		requested_file.close();
		_body = buffer.str();
		_directives["Content-Length"] = make_string(_body.size());
	}
	else 
	{
		_statusCode = generateStatusCode(404);
		if (!createError(404)) {
			_directives["Content-Type"] = "text/html";
			_body = "<!doctype html><html><head><title>404</title></head><body><p><strong>Error : </strong>404 Not Found.</p></body></html>";
			_directives["Content-Length"] = make_string(_body.size());
		}
	}
}

void ResponseHTTP::deleteFile(std::string path)
{
	if (path == "/.html")
		path = "index.html";

	std::string	s		= _config.get_root() + "/" + path;
	char *		c_str	= &s[0];

	if (std::remove(c_str) != 0) {
		if (errno == ENOENT) {
			_statusCode = generateStatusCode(204);
			return ;
		}
	}

	_statusCode = generateStatusCode(200);
	_body = "<html><body><h1>File deleted.</h1></body></html>";
	_directives["Content-Length"] = make_string(_body.size());
	_directives["Content-Type"] = "text/html";
}


std::string	ResponseHTTP::getDate(void)
{
	char			buffer[100];
	struct timeval	tv;
	struct tm *		tm;

	gettimeofday(&tv, NULL);
	tm = gmtime(&tv.tv_sec);
	strftime(buffer, 100, "%a, %d %b %Y %H:%M:%S GMT", tm);

	return (std::string(buffer));
}

std::vector<std::string> ResponseHTTP::isAllowedMethod( std::vector<std::string> path, size_t i, std::map<std::string, Config> location, std::string method, std::vector<std::string> lowestLevel )
{
	Config conf;

	if (i >= path.size())
		return (lowestLevel);

	if (path[i].front() != '*')
		path[i] = '/' + path[i];

	for (std::map<std::string, Config>::const_iterator ite = location.begin(); ite != location.end(); ite++) {
		// Si on trouve une location correspondant au path
		if (path[i] == ite->first) {
			conf = ite->second;
			// Si cette location a une règle allow_methods
			if (conf.get_methods().empty() == false) {
				lowestLevel = conf.get_methods();
			}
			// Si une autre location est trouvée
			if (conf.get_location().empty() == false)
	   			return (isAllowedMethod(path, i + 1, conf.get_location(), method, lowestLevel));
			break ;
		}
	}

	return (lowestLevel);
}

// Permet de vérifier s'il existe un location correspondant au @path demandé,
// et le renvoie dans @locationConfig si c'est le cas.
// À réécrire
bool ResponseHTTP::getLocation(std::string path, Config & locationConfig)
{
	std::map<std::string, Config>	location;

	if (_getLocationFirst) {
		location = _config.get_location();
		_getLocationFirst = false;
	}
	else {
		//std::cout << "Current server name1: " << locationConfig.get_server_name() << "\n";
		location = locationConfig.get_location();
		//std::cout << "Location size: " << location.size() << "\n";
	}

	for (std::map<std::string, Config>::iterator it = location.begin() ; it != location.end() ; it++) {
		//std::cout << "Current location: " << it->second.get_server_name() << "\n";

		if (path.find(it->first) != std::string::npos) {
			locationConfig = it->second;
			//std::cout << "Current server name2: " << locationConfig.get_server_name() << "\n";
			return (true);
		}
	}

	return (false);
}

std::string ResponseHTTP::checkCgiPass(std::vector<std::string> path, std::map<std::string, Config> location, size_t i)
{
	Config conf;


	if (i >= path.size())
		return ("");

	for (std::map<std::string, Config>::const_iterator ite = location.begin(); ite != location.end(); ite++) {
		std::string ext;
		if (path[i].find_last_of('.') != std::string::npos)
			ext = "*" + path[i].substr(path[i].find_last_of('.'));
		if (path[i] == ite->first || (!ext.empty() && ext == ite->first)) {
			conf = ite->second;
			if (conf.get_cgi_pass().empty() == false)
				return(conf.get_cgi_pass());
			break ;
		}
	}
	if (conf.get_location().empty() == false)
		return (checkCgiPass(path, conf.get_location(), i + 1));
	return ("");
}


bool ResponseHTTP::checkAutoIndex(std::vector<std::string> path, std::map<std::string, Config> location, size_t i)
{
	Config conf;

	if (i >= path.size())
		return (false);
	if (path[i].front() != '*')
		path[i] = '/' + path[i];
	for (std::map<std::string, Config>::const_iterator ite = location.begin(); ite != location.end(); ite++) {
		if (path[i] == ite->first) {
			conf = ite->second;
			if (conf.get_autoindex() == true)
				return(true);
			break ;
		}
	}
	if (conf.get_location().empty() == false)
		return (checkAutoIndex(path, conf.get_location(), i + 1));
	return (false);
}

// On vérifie que le contentType qu'on veut renvoyer au client est bien dans la liste des accept qu'il nous a envoyé.
// Ne sert actuellement à rien
bool ResponseHTTP::isAllowedContentType(std::string contentType)
{
	std::vector<std::string> requestAccept = _request.getAccept();

	for (std::vector<std::string>::iterator it = requestAccept.begin() ; it != requestAccept.end() ; it++)
	{
		if (*it == contentType)
				return (true);
	}

	return (false);
}

// Vérifie si il y a un éventuel return dans le .conf ou ses locations
bool ResponseHTTP::isThereReturn(std::string path)
{
	Config locationConfig;

	while (getLocation(_config.get_root() + path, locationConfig)) {
		if (locationConfig.get_return().empty() == false) {
			_statusCode = generateStatusCode(locationConfig.get_return().begin()->first);
			_directives["Location"] = locationConfig.get_return().begin()->second;

			createStatusLine();
			createHeaders();
			return (true);
		}
	}

	return (false);
}

std::string ResponseHTTP::isThereIndex(std::vector<std::string> path, std::map<std::string, Config> location, size_t i)
{
	Config conf;

	if (i >= path.size())
		return ("");

	if (path[i].front() != '*')
		path[i] = '/' + path[i];

	for (std::map<std::string, Config>::const_iterator ite = location.begin(); ite != location.end(); ite++) {
		if (path[i] == ite->first) {
			conf = ite->second;
			if (conf.get_index().empty() == false)
				return(conf.get_index());
			break ;
		}
	}
	if (conf.get_location().empty() == false)
		return (isThereIndex(path, conf.get_location(), i + 1));
	return ("");
}

std::string ResponseHTTP::isThereRoot(std::vector<std::string> path, size_t i, std::map<std::string, Config> location)
{
	Config conf;

	if (i >= path.size())
		return ("");

	if (path[i].front() != '*')
		path[i] = '/' + path[i];

	for (std::map<std::string, Config>::const_iterator ite = location.begin(); ite != location.end(); ite++) {
		if (path[i] == ite->first) {
			conf = ite->second;
			if (conf.get_root().empty() == false) {
				std::string restOfPath;
				i++;
				while (i < path.size()) {
					restOfPath += "/" + path[i++];
				}
				return (conf.get_root() + restOfPath);
			}
			break ;
		}
	}
	if (conf.get_location().empty() == false)
		return (isThereRoot(path, i + 1, conf.get_location()));
	return ("");
}

std::string ResponseHTTP::make_string(int n)
{
	std::stringstream 	ss;

	ss << n;
	return (ss.str());
}