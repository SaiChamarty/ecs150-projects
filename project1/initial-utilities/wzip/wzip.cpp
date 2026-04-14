#include <iostream>
#include <string>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

int main (int argc, char* argv[]){

    if (argc == 1){ // no file input
        string errMsg = "wzip: file1 [file2 ...]\n";
        write(STDOUT_FILENO, errMsg.c_str(), errMsg.size());
        return 1;
    }

    int fd;
    char buffer[4096];
    
    // variables defined outside so the runs can continue across multiple files
    int count = 0;
    char currentChar = '\0';
    bool firstChar = true; // flag to catch the very first character of the very first file

    for (int i = 1; i < argc; i++){
        fd = open(argv[i], O_RDONLY); 
        if (fd < 0) {
            string errMsg = "wzip: cannot open file\n";
            write(STDOUT_FILENO, errMsg.c_str(), errMsg.size());
            return 1;
        }

        int ret;
        while ((ret = read(fd, buffer, 4096)) > 0){
            for (int j = 0; j < ret; j++){
                if (firstChar) {
                    // Initialize the tracking on the first byte
                    currentChar = buffer[j];
                    count = 1;
                    firstChar = false;
                } else if (buffer[j] == currentChar) {
                    // Run continues
                    count++;
                } else {
                    // Note: We write the memory address (&) of the integer directly.
                    write(STDOUT_FILENO, &count, sizeof(int)); // 4-byte binary integer
                    write(STDOUT_FILENO, &currentChar, sizeof(char)); // 1-byte ASCII character
                    
                    // Start tracking the new character
                    currentChar = buffer[j];
                    count = 1;
                }
            }
        }
        close(fd);
    }

    // EDGE CASE: After all files are read, write final run
    if (!firstChar) {
        write(STDOUT_FILENO, &count, sizeof(int));
        write(STDOUT_FILENO, &currentChar, sizeof(char));
    }

    return 0;
}