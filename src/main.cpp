#include "web_server.hpp"
#include "log.hpp"

int main(int argc, char *argv[]) {
	uint16_t port = 80;
	std::string web_root("./web_root");
	uint32_t max_conn = 7000;
	if(argc > 1) {
		port = atoi(argv[1]);
	}
	if(argc > 2) {
		web_root = argv[2];
	}
	if(argc > 3) {
		max_conn = atoi(argv[3]);
	}
	web::WebServer web_server(port, max_conn, web_root, argv[0], logger::LOG_ALL);
	web_server.run();
	return 0;
}