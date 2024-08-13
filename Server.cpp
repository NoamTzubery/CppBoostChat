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
    auto size_buffer = std::make_shared<std::vector<char>>(sizeof(uint32_t));
    socket->async_receive(boost::asio::buffer(*size_buffer), [this, socket, size_buffer](const boost::system::error_code& error, std::size_t bytes) {
        if (!error && bytes == sizeof(uint32_t))
        {
            uint32_t message_size = 0;
            std::memcpy(&message_size, size_buffer->data(), sizeof(uint32_t));

            auto message_buffer = std::make_shared<std::vector<char>>(message_size);
            socket->async_receive(boost::asio::buffer(*message_buffer), [this, socket, message_buffer](const boost::system::error_code& error, std::size_t bytes) {
                handleRead(error, bytes, socket, message_buffer);
                });
        }
        else
        {
            handleRead(error, 0, socket, size_buffer);
        }
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
    uint32_t message_size = static_cast<uint32_t>(message.size());

    for (auto& client : _clients)
    {
        if (client.second != sender_socket)
        {
            auto size_buffer = std::make_shared<std::vector<char>>(sizeof(uint32_t));
            std::memcpy(size_buffer->data(), &message_size, sizeof(uint32_t));

            boost::asio::async_write(*client.second, boost::asio::buffer(*size_buffer),
                [this, client, size_buffer, message](const boost::system::error_code& error, std::size_t bytes_transferred) {
                    if (!error)
                    {
                        auto message_ptr = std::make_shared<std::string>(message);
                        boost::asio::async_write(*client.second, boost::asio::buffer(*message_ptr),
                            [this, message_ptr](const boost::system::error_code& error, std::size_t bytes_transferred) {
                                handleWrite(error, bytes_transferred);
                            });
                    }
                    else
                    {
                        std::cerr << "Write error (size): " << error.message() << std::endl;
                    }
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