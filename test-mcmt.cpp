/**
 * @file   test-mcmt.cpp
 * @author Alex Ott <alexott@gmail.com>
 *
 * @brief  Impelements many threads - many connections strategy of connections handling.
 * All input/output is async. Server use several io_service objects to scale to work on
 * multiprocessor/multicore systems
 *
 *
 */

#define BOOST_ASIO_DISABLE_KQUEUE 1
#include "common.h"
#include <deque>

typedef std::deque<io_service_ptr> ios_deque;

/**
 * Connection class, implementing async input/output
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

	/**
	 * Start input/output chain with reading of headers from browser
	 *
	 */
	void start() {
		// start reading of headers from browser
		boost::asio::async_read_until(socket_, buf, boost::regex("\r\n\r\n"),
			boost::bind(&connection::handle_read, shared_from_this(),
						ba::placeholders::error,
						ba::placeholders::bytes_transferred));
	}

private:
	/**
	 * Initialize connection
	 *
	 * @param io_service
	 *
	 * @return
	 */
	connection(ba::io_service& io_service) : socket_(io_service) {
	}

	/**
	 * Called when data written to browser
	 *
	 * @param error object, containing information about errors
	 * @param bytes_transferred number of transferred bytes
	 */
	void handle_write(const boost::system::error_code& error,
					  size_t bytes_transferred) {
	}

	/**
	 * Called when data readed from browser
	 *
	 * @param error object, containing information about errors
	 * @param bytes_transferred number of transferred bytes
	 */
	void handle_read(const boost::system::error_code& error,
					 size_t bytes_transferred) {
		ba::async_write(socket_, ba::buffer(message_),
						boost::bind(&connection::handle_write, shared_from_this(),
									ba::placeholders::error,
									ba::placeholders::bytes_transferred));
	}

	ba::ip::tcp::socket socket_; /**< socket, associated with browser */
	boost::asio::streambuf buf;  /**< buffer for request data */
	static std::string message_; /**< data, that we'll return to browser */
};

std::string connection::message_="HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n"
	"<html><head><title>test</title>"
	"</head><body><h1>Test</h1><p>This is a test!</p></body></html>";


/**
 * Server class
 *
 */
class server {
public:
/**
 * Initialize all needed data
 *
 * @param io_service reference to io_service
 * @param port port to listen on, by default - 10001
 */
	server(const ios_deque& io_services, int port=10001)
		: io_services_(io_services),
		  acceptor_(*io_services.front(),
					ba::ip::tcp::endpoint(ba::ip::tcp::v4(), port)) {
		start_accept();
	}

private:
	/**
	 * start connection accepting in async mode
	 *
	 */
	void start_accept() {
		// select next io_service object
		io_services_.push_back(io_services_.front());
		io_services_.pop_front();
		// create new connection
		connection::pointer new_connection = connection::create(*io_services_.front());
		// start acceptor in async mode
		acceptor_.async_accept(new_connection->socket(),
							   boost::bind(&server::handle_accept, this, new_connection,
										   ba::placeholders::error));
	}

	/**
	 * Run when new connection is accepted
	 *
	 * @param new_connection accepted connection
	 * @param error reference to error object
	 */
	void handle_accept(connection::pointer new_connection,
					   const boost::system::error_code& error) 	{
		if (!error) {
			new_connection->start();
			start_accept();
		}
	}

	ios_deque io_services_;		     /**< deque of pointers to io_services */
	ba::ip::tcp::acceptor acceptor_; /**< object, that accepts new connections */
};

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
		int thread_num=10;
		// read number of threads with io_services from command line, if provided
		if(argc > 1)
			thread_num=boost::lexical_cast<int>(argv[1]);
		ios_deque io_services;

		boost::thread_group thr_grp;
		// create threads for each io_service
		for (int i = 0; i < thread_num; ++i) {
			io_service_ptr ios(new ba::io_service);
			io_services.push_back(ios);
			// run io_service in their own thread
			thr_grp.create_thread(boost::bind(&ba::io_service::run, ios));
		}
		// create server
		server server(io_services);
		// wait until all thread will finished
		thr_grp.join_all();
	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}


	return 0;
}


