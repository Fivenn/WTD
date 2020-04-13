#include <iostream>
#include <cstdlib> 
#include <string> 
#include <fstream>
#include <unistd.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <jsoncpp/json/json.h>
#include <thread>
#include <mutex>    
#include <cmath>          

std::mutex mtx_main; // this will allow a passive lock for main

std::vector<std::pair<std::thread,int>> clients;
std::mutex mtx_clients;

int winner_id=-1;
std::mutex mtx_winner_id;

Json::Value world;
std::mutex mtx_world;

void deal_with_socket(uint16_t port);
void deal_with_client(int new_socket, unsigned int id);