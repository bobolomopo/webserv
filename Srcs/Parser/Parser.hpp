#ifndef PARSER_HPP
# define PARSER_HPP

# include "../../Includes/Config.hpp"

class Parser
{
    public:
        Parser(void);
		Parser(Parser const & src);
		~Parser(void);

        Parser & operator=(const Parser &rhs);
        
        void    init_vector_string(void);
        void	init_config_file(void);
        void    parse(void);
        void    get_info(std::vector<std::string> vector_string, Config &config);

        void    is_listen(std::string info, Config &config);
        void    is_server_name(std::string info, Config &config);
        void    is_root(std::string info, Config &config);
        void    is_index(std::string info, Config &config);
        void    is_autoindex(std::string info, Config &config);
        void    is_client_max_body_size(std::string info, Config &config);
        void    is_error_page(std::string info, Config &config);
        void    is_fastcgi_param(std::string info, Config &config);
        void    is_alias(std::string info, Config &config);
        void    is_allow_methods(std::string info, Config &config);
        void    is_location(std::vector<std::string> info, Config &config);
        void    is_return(std::string info, Config &config);
        
        Config              & get_config_file(void){return(_config_file);};
        std::vector<Config> & get_vector_config(void) {return(_vector_Config);};
        std::string			& get_path_file(void) {return(_path_file);};

        void	set_path_file(std::string path_file) { _path_file = path_file;};
        
    private:
        Config  _config_file;
        std::string _path_file;
        std::vector<std::vector<std::string> > _allblock;;
        std::vector<Config> _vector_Config;
};

#endif