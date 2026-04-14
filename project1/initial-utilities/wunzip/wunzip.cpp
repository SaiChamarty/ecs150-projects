#include <iostream>
#include <string>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

int main (int argc, char* argv[]){
    
    if (argc == 1){ 
        string errMsg = "wunzip: file1 [file2 ...]\n";
        write(STDOUT_FILENO, errMsg.c_str(), errMsg.size());
        return 1;
    }

    int fd;
    
    for (int i = 1; i < argc; i++){
        fd = open(argv[i], O_RDONLY);
        if (fd < 0) {
            string errMsg = "wunzip: cannot open file\n";
            write(STDOUT_FILENO, errMsg.c_str(), errMsg.size());
            return 1;
        }

        int count;
        char currentChar;

        // Read exactly 4 bytes (the int). If successful, read 1 byte (the char).
        while (read(fd, &count, sizeof(int)) == sizeof(int)) {
            if (read(fd, &currentChar, sizeof(char)) == sizeof(char)) {
                
                // Write the character 'count' times
                for (int j = 0; j < count; j++) {
                    write(STDOUT_FILENO, &currentChar, sizeof(char));
                }
            }
        }
        close(fd);
    }

    return 0;
}