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
	server(ba::io_service& io_service_accept, ba::io_service& io_service_execute, 
		unsigned int thread_num_acceptors, unsigned int thread_num_executors, unsigned int port = 10001, std::string interface_address = "");
	~server();

private:
	void handle_accept(connection::pointer old_connection, const boost::system::error_code& e);
	
	ba::io_service& io_service_acceptors_;  /**< reference to io_service */
	ba::io_service& io_service_executors_;  /**< reference to io_service */
	ba::io_service::work work_acceptors_;   /**< object to inform the io_service_acceptors_ when it has work to do */
	ba::io_service::work work_executors_;   /**< object to inform the io_service_executors_ when it has work to do */
	std::vector<boost::thread> thr_grp_acceptors_;  /**< thread pool object for acceptors */
	std::vector<boost::thread> thr_grp_executors_;  /**< thread pool object for executors */
	const ba::ip::tcp::endpoint endpoint_;   /**< object, that points to the connection endpoint */
	ba::ip::tcp::acceptor acceptor_;         /**< object, that accepts new connections */
};

/** 
 * Initialize all needed data
 * 
 * @param io_service reference to io_service
 * @param thnum number of threads in thread pool
 * @param port port to listen on, by default - 10001
 */
server::server(ba::io_service& io_service_acceptors, ba::io_service& io_service_executors, 
			   unsigned int thread_num_acceptors, unsigned int thread_num_executors, unsigned int port, std::string interface_address)
	: io_service_acceptors_(io_service_acceptors),
	  io_service_executors_(io_service_executors),
	  work_acceptors_(io_service_acceptors_),
	  work_executors_(io_service_executors_),
	  endpoint_(interface_address.empty()?	
				(ba::ip::tcp::endpoint(ba::ip::tcp::v4(), port)): // INADDR_ANY for v4 (in6addr_any if the fix to v6)
				ba::ip::tcp::endpoint(ba::ip::address().from_string(interface_address), port) ),	// specified ip address
	  acceptor_(io_service_acceptors_, endpoint_)	// By default set option to reuse the address (i.e. SO_REUSEADDR)
{
	std::cout << endpoint_.address().to_string() << ":" << endpoint_.port() << std::endl;

	// create threads in pool for executors
	for(size_t i = 0; i < thread_num_executors; ++i)
		thr_grp_executors_.emplace_back(boost::bind(&boost::asio::io_service::run, &io_service_executors_));

	// create threads in pool and start acceptors
	for(size_t i = 0; i < thread_num_acceptors; ++i) {
		// create threads in pool
		if(i != 0)	// one main thread already in pool from: int main() { ... io_service_acceptors.run(); ... }
			thr_grp_acceptors_.emplace_back(boost::bind(&boost::asio::io_service::run, &io_service_acceptors_));

		// create next connection, that will accepted next
		connection::pointer new_connection = connection::create(io_service_executors_);

		// start another acceptor in async mode
		acceptor_.async_accept(new_connection->socket(),
							   boost::bind(&server::handle_accept, this, new_connection,
										   ba::placeholders::error));
	}
}

/** 
 * Wait for all executing threads
 * 
 */
server::~server() {
	io_service_acceptors_.stop();
	io_service_executors_.stop();
	for(auto &i : thr_grp_acceptors_) i.join();
	for(auto &i : thr_grp_executors_) i.join();
}

/** 
 * Run when new connection is accepted
 * 
 * @param e reference to error object
 */
void server::handle_accept(connection::pointer old_connection, const boost::system::error_code& e) {
	if (!e) {
		// schedule new task to thread pool
		io_service_executors_.post(boost::bind(&connection::run, old_connection));
		// create next connection, that will accepted
		connection::pointer new_connection = connection::create(io_service_executors_);
		auto &socket = new_connection->socket();
		// start new accept operation
		acceptor_.async_accept(socket,
							   boost::bind(&server::handle_accept, this, 
										   boost::move(new_connection),	// doesn't copy and doesn't use the atomic counter with memory barrier
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
		int thread_num_acceptors = 2, thread_num_executors = 10, port = 10001;
		std::string interface_address;
		// read number of threads in thread pool from command line, if provided
		if(argc > 1)
			thread_num_acceptors = boost::lexical_cast<int>(argv[1]);
		if(argc > 2)
			thread_num_executors = boost::lexical_cast<int>(argv[2]);
		// read port number from command line, if provided
		if(argc > 3)
			port = boost::lexical_cast<int>(argv[3]);
		// read local interface address from command line, if provided
		if(argc > 4)
			interface_address = argv[4];
		ba::io_service io_service_acceptors, io_service_executors;
		// construct new server object
		server s(io_service_acceptors, io_service_executors, thread_num_acceptors, thread_num_executors, port, interface_address);
		// run io_service object, that perform all dispatch operations
		io_service_acceptors.run();
	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

	return 0;
}

     

