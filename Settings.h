#ifndef SETTINGS_H
#define SETTINGS_H
#include <map>

//asio settings
#define _GNU_SOURCE

#define ASIO_STANDALONE 
#define ASIO_HAS_STD_ADDRESSOF
#define ASIO_HAS_STD_ARRAY
#define ASIO_HAS_CSTDINT
#define ASIO_HAS_STD_SHARED_PTR
#define ASIO_HAS_STD_TYPE_TRAITS

//server settings
enum class RequestType {GET, POST, INVALID};
typedef std::map<std::string, std::string> ParamMap;

#define MAXBUF 4096
#define HOMEDIR "home/"

#endif