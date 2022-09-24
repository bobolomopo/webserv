#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "../Includes/headers.hpp"

class Config
{
	public:
		Config() :  _root(""), _server_name(""), _index("") {};
		Config(Config const &other) {*this = other;};
		~Config() {};

		Config & operator=(const Config & rhs)
		{
			this->_host = rhs._host;
			this->_port = rhs._port;
			this->_host_name = rhs._host_name;
			this->_root = rhs._root;
			this->_server_name = rhs._server_name;
			this->_index = rhs._index;
			this->_autoindex = rhs._autoindex;
            this->_location = rhs._location;
			this->_client_max_body_size = rhs._client_max_body_size;
            this->_cgi_pass = rhs._cgi_pass;
            this->_alias = rhs._alias;
            this->_methods = rhs._methods;
            this->_return = rhs._return;
			this-> _error_page = rhs._error_page;
			return *this;
		}

		void	set_host(struct in_addr host) {_host = host;};
		void	set_port(int port) {_port = port;};
		void	set_host_name(std::string host_name) { _host_name = host_name;};
		void	set_root(std::string &root) {_root = root;};
		void	set_server_name(std::string server_name) {_server_name = server_name;};
		void	set_index(std::string index) {_index = index;};
		void	set_autoindex(bool autoindex) {_autoindex = autoindex;};
		void	set_client_max_body_size(size_t size) {_client_max_body_size = size;};
		void	set_cgi_pass(std::string cgi_pass) {_cgi_pass = cgi_pass;};
		void	set_alias(std::string alias) {_alias = alias;};
		
		
		struct in_addr					& get_host(void) {return(_host);};
		int								& get_port(void) {return(_port);};
		std::string						& get_host_name(void) {return(_host_name);};
		std::string						& get_root(void) {return(_root);};
		std::string						& get_server_name(void){return(_server_name);};
		std::string						& get_index(void) {return(_index);};
		bool							& get_autoindex(void) {return(_autoindex);};
		size_t							& get_client_max_body_size(void) {return(_client_max_body_size);};
		std::map<std::string, Config>	& get_location(void) {return(_location);};
		std::map<int, std::string>		& get_error_page(void) {return(_error_page);};
		std::string					  	& get_cgi_pass(void) {return(_cgi_pass);};
		std::string     				& get_alias(void) {return(_alias);};
		std::vector<std::string> 		& get_methods(void) {return(_methods);};
		std::map<int, std::string> 		& get_return(void) {return(_return);};

	private: 
		struct in_addr	    			_host;
		int				    			_port;
		std::string		    			_host_name;
		std::string         			_root;
		std::string         			_server_name;
		std::string						_index;
		bool							_autoindex;
		size_t							_client_max_body_size;
		std::map<std::string, Config>	_location;
		std::map<int, std::string>      _error_page;
		std::string					        _cgi_pass;
		std::string						_alias;
		std::vector<std::string>		_methods;
		std::map<int, std::string>     	_return;
};

#endif