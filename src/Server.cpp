#include <Server.hpp>

Server::Server()
{
}

Server::Server(Server const &src)
{
	*this = src;
}

Server	&Server::operator=(Server const &src)
{
	if(this != &src)
		return (*this);
	return (*this);
}

Server::Server(std::string const port, std::string const psw): password(psw)
{
	long	p_num;
	int	is_it;

	is_it = 1;
	if (port.find_first_not_of("0123456789") != std::string::npos)
		throw	std::invalid_argument("port is invalid");
	if (port.size() > 5)
		throw	std::invalid_argument("port is too big");
	p_num = std::strtol(port.c_str(), NULL, 10);
	if (p_num > 65535)
		throw	std::invalid_argument("port is too big");
	if (p_num < IPPORT_RESERVED)
		throw	std::invalid_argument("port is privileged");
	server_sk.sin_family = AF_INET;
	server_sk.sin_addr.s_addr = htonl(INADDR_ANY);
	server_sk.sin_port = htons(p_num);
	for (std::string::const_iterator it = psw.begin(); it != psw.end();
			it++)
	{
		if (std::isgraph(static_cast<unsigned char>(*it)) == 0)
			throw	std::invalid_argument("password is invalid");
	}
	server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server_fd == -1)
		throw	std::runtime_error("socket allocation");
	if (fcntl(server_fd, F_SETFL, O_NONBLOCK) == -1)
		throw	std::runtime_error("socket blocking");
	if (bind(server_fd, (struct sockaddr const *)&server_sk, sizeof(server_sk)) == -1)
		throw	std::runtime_error("socket binding");
	if (listen(server_fd, 128) == -1)
		throw	std::runtime_error("socket listening");
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &is_it, sizeof(is_it));
	addToList(server_fd);
	std::cout << "server listening on port " << p_num << std::endl;
}

Server::~Server()
{
}

bool	Server::acceptReq()
{
	int	client_fd;

	client_fd = accept(server_fd, NULL, NULL);
	if (client_fd == -1)
		return (false);
	if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1)
		return (false);
	addToList(client_fd);
	return (true);
}

void	Server::startServer()
{
	int	count;

	while (1)
	{
		count = poll(fd_list.data(), static_cast<nfds_t>(fd_list.size()), -1);
		if (count == -1)
		{
			std::cout << "poll() error" << std::endl;
			continue ;
		}
		else if (count > 0)
		{
			checkList();
		}
	}
}

bool	Server::checkList()
{
	for (std::vector<struct pollfd>::iterator it = fd_list.begin();
		it != fd_list.end(); it++)
	{
		if ((it->fd == server_fd) && (it->revents & POLLIN))
		{
			if (acceptReq() == 0)
				continue ;
			it = fd_list.begin();
			continue ;
		}
		else if (it->revents & POLLIN)
		{
			parseIt(it->fd);
			engine();
		}
	}
}

void	Server::addToList(int fd)
{
	struct pollfd	tmp;

	tmp.fd = fd;
	tmp.events = POLLIN | POLLOUT;
	tmp.revents = 0;
	fd_list.push_back(tmp);
}

void	Server::rmvFromList(int fd)
{
	for (std::vector<struct pollfd>::iterator it = fd_list.begin();
		it != fd_list.end(); it++)
	{
		if (it->fd == fd)
		{
			close(fd);
			fd_list.erase(it);
		}
	}
}

void	Server::parseIt(int fd)
{

}
