/**
 * @file   test-otpc-conn.cpp
 * @author Alex Ott <alexott@gmail.com>
 * 
 * @brief  Implementation of sync connection class
 * 
 * 
 */

#include "test-otpc-conn.hpp"

/**
 * What we'll return to browser/test utility
 * 
 */
const std::string connection::message_="HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n"
	"<html><head><title>test</title>"
	"</head><body><h1>Test</h1><p>This is a test!</p></body></html>";


/** 
 * Constructor for class, initilize socket for this connection
 * 
 * @param io_service reference to io_service
 * 
 * @return nothing
 */
connection::connection(ba::io_service& io_service, hide_me) :
	io_service_(io_service), socket_(io_service) {
}
	

/** 
 * Perform all input/output operations in sync mode
 * 
 */
void connection::run() {
	try {
		// read data from socket until empty line
		ba::read_until(socket_, buf, boost::regex("\r\n\r\n"));
		// write answer to socket
		ba::write(socket_,ba::buffer(message_),ba::transfer_all());
		// close socket
		socket_.close();
	} catch(std::exception& x) {
//		std::cerr << "Exception: " << x.what() << std::endl;
	}
}
	
