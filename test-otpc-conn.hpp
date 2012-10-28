/**
 * @file   test-otpc-conn.hpp
 * @author Alex Ott <alexott@gmail.com>
 * 
 * @brief  Declaration of sync connection class
 * 
 * 
 */

#ifndef _TEST_OTPC_CONN_H
#define _TEST_OTPC_CONN_H 1

#include "common.h"

/**
 * Class for handling connection in sync mode
 * 
 */
class connection : public boost::enable_shared_from_this<connection> {
public:
	typedef boost::shared_ptr<connection> pointer;

	/** 
	 * Create new connection 
	 * 
	 * @param io_service io_service in which this connection will work
	 * 
	 * @return pointer to newly allocated object
	 */
	static pointer create(ba::io_service& io_service) {
		return pointer(new connection(io_service));
	}

	/** 
	 * Return socket, associated with this connection. This socket used in accept operation
	 * 
	 * 
	 * @return reference to socket
	 */
	ba::ip::tcp::socket& socket() {
		return socket_;
	}
	void run();
	
private:
	connection(ba::io_service& io_service);
	
	ba::io_service& io_service_; /**< reference to io_service, in which work this connection */
	ba::ip::tcp::socket socket_; /**< socket, associated with browser */
	ba::streambuf buf;			 /**< buffer for request data */
	static std::string message_; /**< data, that we'll return to browser */
};


#endif /* _TEST_OTPC_CONN_H */

