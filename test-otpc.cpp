/**
 * @file   test-ocmt.cpp
 * @author Alex Ott <alexott@gmail.com>
 * 
 * @brief Implementation of 'stupid' www server with implementation of thread per
 * connection strategy.  Working threads use sync mode for input/output, but server thread
 * work in async mode.
 * 
 * 
 */

#include "test-otpc-conn.hpp"
#include <set>

/**
 * Server class
 * 
 */
class server : private boost::noncopyable {
public:
	server(ba::io_service& io_service, int port=10001);

private:
	void handle_accept(const boost::system::error_code& e);

	ba::io_service& io_service_;         /**< reference to io_service */
	ba::ip::tcp::acceptor acceptor_;     /**< object, that accepts new connections */
	connection::pointer new_connection_; /**< pointer to connection, that will proceed next */
};

/** 
 * Initialize all needed data
 * 
 * @param io_service reference to io_service
 * @param port port to listen on, by default - 10001
 */
server::server(ba::io_service& io_service,int port)
	: io_service_(io_service),
	  acceptor_(io_service_, ba::ip::tcp::endpoint(ba::ip::tcp::v4(), port)),
	  new_connection_(connection::create(io_service_)) {
	// start acceptor in async mode
	acceptor_.async_accept(new_connection_->socket(),
						   boost::bind(&server::handle_accept, this,
									   ba::placeholders::error));
}

/** 
 * Run when new connection is accepted
 * 
 * @param e reference to error object
 */
void server::handle_accept(const boost::system::error_code& e) {
	if (!e) {
		// run connection in new thread
		boost::thread t(boost::bind(&connection::run, new_connection_));
		// create next connection, that will accepted
		new_connection_=connection::create(io_service_);
		// start new accept operation
		acceptor_.async_accept(new_connection_->socket(),
							   boost::bind(&server::handle_accept, this,
										   ba::placeholders::error));
	}
}

/** 
 * Main routine
 * 
 * @param argc number of arguments
 * @param argv pointers to arguments
 * 
 * @return error code
 */
int main(int argc, char** argv) {
	try {
		int port=10001;
		// read port number from command line, if provided
		if(argc > 1)
			port=boost::lexical_cast<int>(argv[1]);
		ba::io_service io_service;
		// construct new server object
		server s(io_service,port);
		// run io_service object, that perform all dispatch operations
		io_service.run();
	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

	return 0;
}

     
