#include <Server.h>

int main(int argc, char* argv[]) {
    // check arguments 
    if(argc-1 != 2) {
        std::cerr << "Usage: " << argv[0] << " <PORT> <WORLD_JSON>" << std::endl;
        return EXIT_FAILURE;
    }
    // process port
    int port = atoi(argv[1]);
    if(std::isnan(port) || port > 65536) {
        std::cerr << "Port : " << port << "is not a real number or not a uint16_t number" << std::endl;
        return EXIT_FAILURE;
    }
    // process JSON file
    std::ifstream config_doc(argv[2]); // TODO : check that's valid file
    Json::CharReaderBuilder rbuilder;
    std::string errs;
    mtx_world.lock();
    bool parsingSuccessful = Json::parseFromStream(rbuilder, config_doc, &world, &errs);
    mtx_world.unlock();
    if(!parsingSuccessful) {
        std::cerr << "Failed to parse configuration\n" << errs;
        return EXIT_FAILURE;
    }
    // check world
    mtx_world.lock();
    std::cout << "World: [" << world["name"].asString()<< "] loaded..." << std::endl;
    mtx_world.unlock();
    // process new thread
    std::thread connection_dealer = std::thread(deal_with_socket, (uint16_t) port);
    mtx_winner_id.lock();
    while(winner_id==-1) {
        mtx_winner_id.unlock();
        mtx_main.lock();    
        mtx_winner_id.lock();
    }
    mtx_winner_id.unlock();
    mtx_winner_id.lock();
    std::cout << "client " << winner_id << " wins !" << std::endl;
    mtx_winner_id.unlock();
    // terminate threads
    connection_dealer.detach();
    mtx_clients.lock();
    for(auto & i:clients) {
        i.first.detach();
        close(i.second);
    }
    mtx_clients.unlock();
    return 0;
}

void deal_with_socket(uint16_t port) {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "socket failed "<<__FILE__<<" "<<__LINE__<<std::endl; 
        exit(EXIT_FAILURE); 
    }
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        std::cerr << "setsockopt" <<__FILE__<<" "<<__LINE__<<std::endl; 
        exit(EXIT_FAILURE); 
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    if(bind(server_fd, (struct sockaddr *) &address, sizeof(address)) <0) {
        std::cerr << "bind failed "<<__FILE__<<" "<<__LINE__<<std::endl; 
        exit(EXIT_FAILURE); 
    }
    if(listen(server_fd, 3) < 0) {
        std::cerr << "listen "<<__FILE__<<" "<<__LINE__<<std::endl; 
        exit(EXIT_FAILURE); 
    }
    while(true) {
        int new_socket;
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) { 
            std::cerr << "accept "<<__FILE__<<" "<<__LINE__<<std::endl; 
            exit(EXIT_FAILURE); 
        }
        std::cout<<"client "<<clients.size() <<" connected"<<std::endl;
        mtx_clients.lock();
        clients.emplace_back(std::thread(deal_with_client, new_socket, clients.size()), new_socket); // emplace_back creates the std::pair...
        mtx_clients.unlock();
        mtx_main.unlock();
    }
}

void deal_with_client(int new_socket, unsigned int id) {
}