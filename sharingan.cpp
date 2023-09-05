#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <thread>
#include <vector>
#include <chrono>

using namespace std;

string red = "\033[1;31m";
string yellow = "\033[1;33m";
string green = "\033[1;32m";
string reset = "\033[0m";

void logo() {
    cout << red << R"(
                                       &&&&&&&     &&&&
                                &&&&&&&&&&&&     .&&&&&&&&.
                            &&&&&&&&&&&&&      .&&&&&&&&&&&&.
                         &&&&&&&&&&&&&&       .&&&&&&&&&&&&&&&&.
                       &&&&&&&&&&&&&&&        &&&&&&&&&&&&&&&&&&&.
                     &&&&&&&&&&&&&&&&         &&&&&&&&&&&&&&&&&&&&&.
                    &&&&&&&&&&&&&&&&          &&&&&&&&&&&&&&&&&&&&&&&.
                  &&&&&&&&&&&&&&&&&           .&&&&&&&&&&&&&&&&&&&&&&&.
                 &&&&&&&&&&&&&&&&&              .'''&&&&&&&&&&&&&&&&&&&.
                &&&&&&&&&&&&&&&&&                 ..''&&&&&&&&&&&&&&&&&&.
               &&&&&&&&&&&&&&&&&&                    ..'&&&&&&&&&&&&&&&&&.
               &&&&&&&&&&&&&&&&'                        .&&&&&&&&&&&&&&&&.
              &&&&&&&&&&&&&&&&'.                          .&&&&&&&&&&&&&&&.
              &&&&&&&&&&&&&&&&'.                            .&&&&&&&&&&&&&.
              &&&&&&&&&&&&&&&''.         &&&&&&&              .&&&&&&&&&&&.
              &&&&&&&&&&&&&&&''.         &&&&&&&                .&&&&&&&&&.
              &&&&&&&&&&&&&&&&'          .'&&&'.                 .&&&&&&&&.
              &&&&&&&&&&&&&&&&&                                   .&&&&&&&.
              &&&&&&&&&&&&&&&&                                     .&&&&&&.
               &&&&&&&&&&&&&                                        .&&&&.
                                                     &&&&&&&        .&&&&.
                &&                               &&'''&&&&&&&&&     .&&&.
                 &&&&                        &&&''&&&&&&&&&&&&&&     &&&
                  &&&&&&&&            .'&&&&&&&&&&&&&&&&&&&&&&&&&   .&.
                    .&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&  &
                      .&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.
                        &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.
                          .&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.
                             .&&&&&&&&&&&&&&&&&&&&&&&&&&&&&.
                                 .&&&&&&&&&&&&&&&&&&&&&.
                                         &&&&&&&
    )" << endl;
}

const char* host; // The target hostname or IP address
int startPort;    // The starting port
int endPort;      // The ending port
int connectionTimeoutMs; // Timeout for socket connections in milliseconds

// Function to perform port scanning for a range of ports
void scanPorts(int begin, int range) {
    for (int port = begin; port < begin + range; port++) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1) {
	  cerr << red << "Error creating socket" << endl;
            return;
        }

        struct hostent* server = gethostbyname(host);
        if (server == NULL) {
	  cerr << red << "Error resolving hostname" << endl;
            return;
        }

        struct sockaddr_in serverAddress;
        memset(&serverAddress, 0, sizeof(serverAddress));
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(port);
        bcopy((char*)server->h_addr, (char*)&serverAddress.sin_addr.s_addr, server->h_length);

        struct timeval timeout;
        timeout.tv_sec = connectionTimeoutMs / 1000; // seconds
        timeout.tv_usec = (connectionTimeoutMs % 1000) * 1000; // microseconds
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

        if (connect(sock, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == 0) {
	  cout << reset << "Port " << port << " is" << green << " open." << endl;
            close(sock);
        } else {
            // Port is closed or unreachable
	  // cout << "Port " << port << " is" << yellow << " closed or unreachable." << endl;
            close(sock);
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
      cerr <<red << "Usage: " << argv[0] << " <hostname> <startPort> <endPort> <connectionTimeoutMs>" << endl;
        return 1;
    }

    host = argv[1];
    startPort = atoi(argv[2]);
    endPort = atoi(argv[3]);
    connectionTimeoutMs = atoi(argv[4]);
    int numThreads = endPort;

    logo();

    cout << yellow << "Scanning ports for host: " << host << " from port " << startPort << " to " << endPort << endl;
    cout << "" << endl;
    cout << "Using " << connectionTimeoutMs << "ms for each port." << endl;
    cout << "" << endl;
    cout << "" << endl;

    vector<thread> threads;

    int portsPerThread = (endPort - startPort + 1) / numThreads;

    for (int i = 0; i < numThreads; i++) {
        int beginPort = startPort + i * portsPerThread;
        int range = (i == numThreads - 1) ? (endPort - beginPort + 1) : portsPerThread;
        threads.emplace_back(scanPorts, beginPort, range);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return 0;
}
