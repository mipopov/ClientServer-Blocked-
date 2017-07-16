#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <cstdio>
#include <vector>

static void writeRead(int fd, const char *txt);

int main(int argc, char *argv[]) {
    const char        *host = "127.0.0.1";
    uint16_t           port = 8000;
    struct sockaddr_in addr;
    int                fd;

    memset(&addr, 0, sizeof addr);    
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    if(inet_pton(AF_INET, host, &addr.sin_addr) < 1) {
        fprintf(stderr, "Wrong host\n");
        return -1;
    }

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1) {
        fprintf(stderr, "Can't create socket\n");
        return -1;
    }

    if(connect(fd, (struct sockaddr *)&addr, sizeof addr) == -1) {
        fprintf(stderr, "Can't connect to the server\n");
        if(close(fd))
            fprintf(stderr, "Can't realese file descriptor\n");  
        return -1;
    }

    writeRead(fd, argc > 1 ? argv[1] : "This is a test message");

    if(shutdown(fd, 2) == -1)
        fprintf(stderr, "Can't shutdown socket\n");

    if(close(fd)) {
        fprintf(stderr, "Can't realese file descriptor\n");
        return -1;
    }

    return 0;
}

static void writeRead(int fd, const char *txt) {
    uint32_t len = strlen(txt) + 1;
 
    try {
        if(write(fd, &len, sizeof len) != sizeof len)
            throw "Can't write a length";

        if(write(fd, txt, len) != (ssize_t)len)
            throw "Can't write a text";
    
        if(read(fd, &len, sizeof len) != sizeof len)
            throw "Can't read a length";

        std::vector<char> buf(len, 0); 

        if(read(fd, &buf[0], len) != (ssize_t)len)
            throw "Can't read a text";

        puts(&buf[0]);
    }
    catch(const char *err) {
        fprintf(stderr, "%s\n", err);
    }
    catch(std::bad_alloc) {
        fprintf(stderr, "Memory allocation error\n");
    }
}

