/**
 * @file   common.h
 * @author Alex Ott <alexott@gmail.com>
 * 
 * @brief  Declarations and includes, common for all boost::asio based programs
 * 
 * 
 */

#ifndef _COMMON_H
#define _COMMON_H 1

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <boost/regex.hpp>

#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>

#include <iostream>
#include <string>

namespace ba=boost::asio;
namespace bs=boost::system;

typedef boost::shared_ptr<ba::ip::tcp::socket> socket_ptr;
typedef boost::shared_ptr<ba::io_service> io_service_ptr;

#endif /* _COMMON_H */

