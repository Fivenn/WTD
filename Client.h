#include <thread>

#define N_CHAR 1024UL
#define PORT 8080
#define SERV_ADDR "127.0.0.1"

std::thread server_dealer;
bool isComplete;

void deal_with_socket();
void deal_with_server(int new_socket);
