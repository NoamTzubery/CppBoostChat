#include "Server.hpp"

int main() {
    try {
        boost::asio::io_service io_service;
        Server server(io_service);
        io_service.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}
