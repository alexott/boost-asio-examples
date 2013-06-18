/**
 * @file   test-ocmt-tp.cpp
 * @author Alex Ott <alex_ott@gmail.com>
 * 
 * @brief Implementation of 'stupid' www server with implementation of thread per
 * connection strategy and usage of threads pool for keeping number of threads limited.
 * Working threads use sync mode for input/output, but server thread work in async mode.
 *
 * Thread pool implemented by threadpool library, that you could obtain from
 * http://threadpool.sf.net 
 * 
 */

#include "test-otpc-conn.hpp"
#include <vector>


/**
 * Server class
 * 
 */
class server : private boost::noncopyable {
public:
	server(ba::io_service& io_service, int thnum, int port=10001);

private:
	void handle_accept(const boost::system::error_code& e);
	
	ba::io_service& io_service_;         /**< reference to io_service */
	boost::asio::io_service::work work_; /**< object to inform the io_service when it has work to do */
	ba::ip::tcp::acceptor acceptor_;     /**< object, that accepts new connections */
	connection::pointer new_connection_; /**< pointer to connection, that will proceed next */
	std::vector<boost::thread> thr_grp;  /**< thread pool object */
};

/** 
 * Initialize all needed data
 * 
 * @param io_service reference to io_service
 * @param thnum number of threads in thread pool
 * @param port port to listen on, by default - 10001
 */
server::server(ba::io_service& io_service, int thnum, int port)
	: io_service_(io_service),
	  work_(io_service_),
	  acceptor_(io_service_, ba::ip::tcp::endpoint(ba::ip::tcp::v4(), port)),
	  new_connection_(connection::create(io_service_))
{
	// create threads in pool
	for(size_t i = 0; i < thnum; ++i)
		thr_grp.emplace_back(boost::bind(&boost::asio::io_service::run, &io_service_));

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
		// schedule new task to thread pool
		io_service_.post(boost::bind(&connection::run, new_connection_));
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
		int thread_num=10, port=10001;
		// read number of threads in thread pool from command line, if provided
		if(argc > 1)
			thread_num=boost::lexical_cast<int>(argv[1]);
		// read port number from command line, if provided
		if(argc > 2)
			port=boost::lexical_cast<int>(argv[2]);
		ba::io_service io_service;
		// construct new server object
		server s(io_service, thread_num, port);
		// run io_service object, that perform all dispatch operations
		io_service.run();
	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

	return 0;
}

     

