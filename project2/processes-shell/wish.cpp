#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>

using namespace std;

vector<string> path = {"/bin"};

void printError() {
    char error_message[30] = "An error has occurred\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}

vector<string> splitWhitespace(const string& s) {
    vector<string> result;
    istringstream ss(s);
    string word;
    while (ss >> word) {
        result.push_back(word);
    }
    return result;
}

// splits by &, and it might not have spaces around it
vector<string> splitAmpersand(const string& line) {
    vector<string> cmds;
    string cur;
    for (char c : line) {
        if (c == '&') {
            cmds.push_back(cur);
            cur = "";
        } else {
            cur += c;
        }
    }
    cmds.push_back(cur);
    return cmds;
}

// Single execution function that handles both standard and parallel commands
void executeCommand(const string& raw, vector<pid_t>& active_pids) {
    string outfile = "";
    string cmdpart = raw;

    int rcount = 0;
    for (char c : raw) if (c == '>') rcount++;

    if (rcount > 1) {
        printError();
        return;
    }

    size_t rpos = raw.find('>');
    if (rpos != string::npos) {
        cmdpart = raw.substr(0, rpos);
        string rhs = raw.substr(rpos + 1);
        vector<string> rtoks = splitWhitespace(rhs);
        if (rtoks.size() != 1) {
            printError();
            return;
        }
        outfile = rtoks[0];
    }

    vector<string> args = splitWhitespace(cmdpart);
    if (args.empty()) {
        if (rpos != string::npos) printError();
        return;
    }

    // Built-ins: texecution in the parent process 
    if (args[0] == "exit") {
        if (args.size() != 1) { printError(); return; }
        exit(0);
    }
    if (args[0] == "cd") {
        if (args.size() != 2) { printError(); return; }
        if (chdir(args[1].c_str()) != 0) printError();
        return;
    }
    if (args[0] == "path") {
        path.clear();
        for (int i = 1; i < (int)args.size(); i++)
            path.push_back(args[i]);
        return;
    }

    // Find the executable
    string execpath = "";
    for (int i = 0; i < (int)path.size(); i++) {
        string attempt = path[i] + "/" + args[0];
        if (access(attempt.c_str(), X_OK) == 0) {
            execpath = attempt;
            break;
        }
    }
    if (execpath.empty()) {
        printError();
        return;
    }

    // Fork for external commands
    pid_t pid = fork();
    if (pid < 0) {
        printError();
        return;
    }

    if (pid == 0) {
        // Child process
        if (!outfile.empty()) {
            int fd = open(outfile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) { 
                printError(); 
                exit(1); 
            }
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);
        }

        // Use a vector instead of 'new' to manage memory cleanly
        vector<char*> argv;
        for (int i = 0; i < (int)args.size(); i++) {
            argv.push_back(strdup(args[i].c_str()));
        }
        argv.push_back(NULL); // execv requires a NULL terminator

        execv(execpath.c_str(), argv.data());
        
        printError();
        exit(1);
    } else {
        // Parent process: track the pid so we can wait for it later
        active_pids.push_back(pid);
    }
}

void processLine(const string& line) {
    vector<string> cmds = splitAmpersand(line);
    vector<pid_t> pids; // Tracks all child processes started from this line

    // Launch all commands (will naturally handle 1 or 10 cmds in parallel)
    for (int i = 0; i < (int)cmds.size(); i++) {
        executeCommand(cmds[i], pids);
    }

    // Wait for all launched processes to finish before returning to prompt
    for (int i = 0; i < (int)pids.size(); i++) {
        waitpid(pids[i], NULL, 0);
    }
}

int main(int argc, char* argv[]) {
    if (argc > 2) {
        printError();
        exit(1);
    }

    istream* input = &cin;
    ifstream batchfile;
    bool interactive = true;

    if (argc == 2) {
        batchfile.open(argv[1]);
        if (!batchfile.is_open()) {
            printError();
            exit(1);
        }
        input = &batchfile;
        interactive = false;
    }

    string line;
    while (true) {
        if (interactive)
            cout << "wish> " << flush;

        if (!getline(*input, line))
            break;

        processLine(line);
    }

    exit(0);
}