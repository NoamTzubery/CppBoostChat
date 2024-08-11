#include "Server.hpp"

Server::Server(boost::asio::io_service& io_service)
    : _io_service(io_service), _acceptor(io_service), _socket(io_service) {
    boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), PORT);
    _acceptor.open(endpoint.protocol());
    _acceptor.bind(endpoint);
    _acceptor.listen();
    startAccepting();
}

Server::~Server()
{
    _acceptor.close();
    for (auto& client : _clients)
    {
        if (client.second->is_open())
        {
            client.second->close();
        }
    }
    _clients.clear();
}

void Server::startAccepting()
{
    auto new_socket = std::make_shared<boost::asio::ip::tcp::socket>(_acceptor.get_executor());
    _acceptor.async_accept(*new_socket, [this, new_socket](const boost::system::error_code& error) {
        handleNewConnection(new_socket, error);
        });
}

void Server::handleNewConnection(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const boost::system::error_code& error)
{
    if (!error) {
        std::cout << "New connection accepted." << std::endl;
        int socket_fd = socket->native_handle();
        _clients[socket_fd] = socket;
        readFromClient(socket);
        startAccepting();
    }
    else {
        throw std::runtime_error("Error accepting connection: " + error.message());
    }
}

void Server::readFromClient(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
{
    auto buffer = std::make_shared<std::vector<char>>(INIT_BUFFER_SIZE);
    socket->async_receive(boost::asio::buffer(*buffer), [this, socket, buffer](const boost::system::error_code& error, std::size_t bytes) {
        handleRead(error, bytes, socket, buffer);
        });
}

void Server::handleRead(const boost::system::error_code& error, std::size_t bytes, std::shared_ptr<boost::asio::ip::tcp::socket> socket, std::shared_ptr<std::vector<char>> buffer)
{
    int socket_fd = socket->native_handle();
    if (!error)
    {
        if (bytes > 0)
        {
            std::string message(buffer->data(), bytes);
            sendMessageToClients(message, socket);
        }
        readFromClient(socket);
    }
    else
    {
        _clients.erase(socket_fd);
        socket->close();
        std::cout << socket_fd << " disconnected from the chat\n";
    }
}

void Server::sendMessageToClients(const std::string& message, std::shared_ptr<boost::asio::ip::tcp::socket> sender_socket)
{
    auto message_ptr = std::make_shared<std::string>(message);
    for (auto& client : _clients)
    {
        if (client.second != sender_socket)
        {
            boost::asio::async_write(*client.second, boost::asio::buffer(*message_ptr), [this, message_ptr](const boost::system::error_code& error, std::size_t bytes_transferred) {
                handleWrite(error, bytes_transferred);
                });
        }
    }
}

void Server::handleWrite(const boost::system::error_code& error, std::size_t bytes_transferred)
{
    if (error) {
        std::cerr << "Error writing data: " << error.message() << std::endl;
    }
}
