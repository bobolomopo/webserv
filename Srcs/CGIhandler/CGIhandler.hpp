/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIhandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jandre <jandre@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/06/14 09:19:04 by jandre            #+#    #+#             */
/*   Updated: 2022/06/28 15:38:43 by jandre           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGIHANDLER_HPP
# define CGIHANDLER_HPP

# include "../RequestHTTP/RequestHTTP.hpp"

class CGIhandler {
	private:
		std::string							_path;
		RequestHTTP							_request;
		std::string							_body;
		std::map<std::string, std::string>	_env;
		Config								_config;

	public:
		//CONSTRUCTOR DESTRUCTOR COPY
		CGIhandler(RequestHTTP request, Config config, std::string path);
		CGIhandler(CGIhandler const & cpy);
		~CGIhandler();
		CGIhandler & operator=(CGIhandler const & rhs);
		
		//MEMBER FUNCTION
		void			init_env();
		std::string		& get_body();
		void			execute_CGI_GET();
		void			execute_CGI_POST();
		char			**get_env_as_char_array();
		char			**get_arg_as_char_array();
		int				get_status_code();
		std::string 	& get_content_type();
};

#endif
