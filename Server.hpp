#pragma once
#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <array>
#include <vector>
#include <unordered_map>

class Server
{
private:
    static constexpr auto PORT = 12345;
    static constexpr auto INIT_BUFFER_SIZE = 1024;
    boost::asio::io_service& _io_service;
    boost::asio::ip::tcp::acceptor _acceptor;
    boost::asio::ip::tcp::socket _socket;
    std::unordered_map<int, std::shared_ptr<boost::asio::ip::tcp::socket>> _clients;

    void startAccepting();
    void handleNewConnection(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const boost::system::error_code& error);
    void readFromClient(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
    void handleRead(const boost::system::error_code& error, std::size_t bytes_transferred, std::shared_ptr<boost::asio::ip::tcp::socket> socket, std::shared_ptr<std::vector<char>> buffer);
    void sendMessageToClients(const std::string& message, std::shared_ptr<boost::asio::ip::tcp::socket> sender_socket);
    void handleWrite(const boost::system::error_code& error, std::size_t bytes_transferred);


public:
    Server(boost::asio::io_service& io_service);
    ~Server();

};
