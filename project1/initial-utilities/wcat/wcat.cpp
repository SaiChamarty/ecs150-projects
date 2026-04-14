#include <iostream>
#include <string>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

int main(int argc, char* argv[]){
    int fd; // file descriptor

    if (argc == 1){ // no file has been input
        return 0; // exit
    }

    for (int i = 1; i < argc; i++){
        fd = open(argv[i], O_RDONLY);
        if (fd < 0) {
            string cerr = "wcat: cannot open file\n";
            write(STDOUT_FILENO, cerr.c_str(), cerr.size());
            return 1;
        }

        char buffer[4096];
        int ret;

        while((ret = read(fd, buffer, 4096)) > 0){ // read only 4096 max bytes at a time
            write(STDOUT_FILENO, buffer, ret);
        }
        close(fd);
    }
}