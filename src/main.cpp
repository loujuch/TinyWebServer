#include "web_server.hpp"
#include "log.hpp"

int main(int argc, char *argv[]) {
	logger::log_set_file_size_limit(1024 * 1024);
	web::WebServer web_server(argc, argv, 7000, "~/web_root", logger::LOG_OFF);
	web_server.run();
	return 0;
}