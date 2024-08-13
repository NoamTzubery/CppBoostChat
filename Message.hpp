#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <cstdint>
#include <ctime>
#include <boost/serialization/access.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/split_member.hpp>
#include <chrono>

/**
 * @class Message
 * @brief Represents a message with a timestamp and content.
 *
 * This class provides functionality to store a message along with its timestamp.
 * It supports serialization using Boost.Serialization.
 */
class Message {
public:
    /**
     * @brief Constructs a Message object.
     *
     * @param msg The content of the message.
     */
    Message(const std::string& msg) {
        _timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        _message = msg;
    }

    /**
     * @brief Default constructor for serialization.
     */
    Message() = default;

    /**
     * @brief Saves the Message object using Boost.Serialization.
     */
    template<class Archive>
    void save(Archive& ar, const unsigned int version) const {
        ar& _timestamp;
        ar& _message;
    }

    /**
     * @brief Loads the Message object using Boost.Serialization.
     */
    template<class Archive>
    void load(Archive& ar, const unsigned int version) {
        ar& _timestamp;
        ar& _message;
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()

private:
    std::uint64_t _timestamp;      ///< The timestamp of the message, in Unix time.
    std::string _message;          ///< The message content stored in a std::string.

    friend class boost::serialization::access;
};

#endif 
