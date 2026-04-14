#include <iostream>
#include <string>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

/// @brief this fucntion checks if the word exists in each file and writes to terminal
/// @param fd file descriptor
/// @param word word to find
void helper (int fd, const string& word){
    char buffer[4096];
    string line = ""; 
    int ret;

    while ((ret = read(fd, buffer, 4096)) > 0){
        // read a buffer then look through it for a new line

        for (int j = 0; j < ret; j++){
            if(buffer[j] != '\n'){
                line = line + buffer[j];
            } else {
                // we have a complete line = line
                // check for the word

                if(line.find(word) != string::npos){ // found, (documentation of C++)
                    // write the line
                    line = line + '\n'; // so that the lines are not crammed together
                    write(STDOUT_FILENO, line.c_str(), line.size());
                    line = ""; // empty the line
                } else {
                    // word not found
                    line = ""; // empty the current line
                }
            }
        }
    } 
    if (!line.empty()) { // if last line does not have a /n
        if (line.find(word) != string::npos) {
            line += '\n';
            write(STDOUT_FILENO, line.c_str(), line.size());
        }
    }
} 

int main (int argc, char* argv[]){

    int fd; // file descriptor

    if (argc == 1){ 
        string cerr = "wgrep: searchterm [file ...]\n";
        write(STDOUT_FILENO, cerr.c_str(), cerr.size());
        return 1;
    }

    string word = argv[1]; // first argument is the search term


    if (argc == 2) {
        fd = STDIN_FILENO; 
        helper(fd, word);
    }
    
    // parse through each file to find the lines with the search term
    for (int i = 2; i < argc; i++){
        fd = open(argv[i], O_RDONLY); // open the file
        if (fd < 0) {
            string cerr = "wgrep: cannot open file\n";
            write(STDOUT_FILENO, cerr.c_str(), cerr.size());
            return 1;
        }

        helper(fd, word); 

        close(fd);
    }
    return 0;
}