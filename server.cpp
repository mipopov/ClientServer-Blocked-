#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cinttypes>
#include <csignal>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <vector>

static int signalIgnoring();
static void loop(int ld);

int main() {
    const char *host = "127.0.0.1";
    uint16_t    port = 8000;
    sockaddr_in addr;
    int         ld   = -1;
    int         on;

    if(signalIgnoring() != 0)
        return -1;

    memset(&addr, 0, sizeof addr);    
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    if(inet_pton(AF_INET, host, &addr.sin_addr) < 1) {
        fprintf(stderr, "Wrong host\n");
        return -1;
    }

    ld = socket(AF_INET, SOCK_STREAM, 0);
    if(ld == -1) {
        fprintf(stderr, "Can't create socket\n");
        return -1;
    }

    on = 1;
    if(setsockopt(ld, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on) == -1) {
        fprintf(stderr, "Can't setsockopt\n");
        if(close(ld))
            fprintf(stderr, "Can't realese file descriptor\n");
        return -1;
    }

    if(bind(ld, (sockaddr *)&addr, sizeof addr) == -1) {
        fprintf(stderr, "Can't bind\n"); 
        if(close(ld))
            fprintf(stderr, "Can't realese file descriptor\n");
        return -1;
    }

    if(listen(ld, 5) == -1) {
        fprintf(stderr, "Can't listen\n");
        if(close(ld))
            fprintf(stderr, "Can't realese file descriptor\n");
        return -1;
    }

    loop(ld);

    if(close(ld)) {
        fprintf(stderr, "Can't realese file descriptor\n");
        return -1;
    }

    puts("Done.");

    return 0;
}

static void handler(int signo);

static int signalIgnoring() {
    struct sigaction act;
    act.sa_handler = handler;
    act.sa_flags   = 0;
    sigemptyset(&act.sa_mask);

    if(sigaction(SIGINT, &act, 0) == -1) {
        fprintf(stderr, "Can't setup SIGINT ignoring\n");
        return -1;
    }

    if(sigaction(SIGPIPE, &act, 0) == -1) {
        fprintf(stderr, "Can't setup SIGPIPE ignoring\n");
        return -1;
    }

    return 0;
}

static void handler(int signo) {
    (void)signo;
}

static void loop(int ld) {
    for(int fd = -1;; fd = -1) {
        try {
            sockaddr_in addr;
            socklen_t   addrlen;

            memset(&addr, 0, sizeof addr);
            addrlen = sizeof addr;

            int fd = accept(ld, (struct sockaddr *)&addr, &addrlen);
            if(fd == -1) {
                if(errno == EINTR)
                    return;
                else 
                    throw"Can't accept";
            }
    
            uint32_t len;
            if(read(fd, &len, sizeof len) != sizeof len)
                throw "Can't read a length";

            printf("Conn: %d Header: %" PRIu32 "\n", fd, len);

            std::vector<char> buf(6 + len, 0);
            strcpy(&buf[0], "Echo: ");

            if(read(fd, &buf[6], len) != (ssize_t)len)
                throw "Can't read a text";

            printf("Conn: %d Body: %s\n", fd, &buf[6]);

            len += 6;

            if(write(fd, &len, sizeof len) != sizeof len)
                throw "Can't write a length";

            if(write(fd, &buf[0], len) != (ssize_t)len)
                throw "Can't write a text";
        }
        catch(const char *err) {
            fprintf(stderr, "%s\n", err);
        }
        catch(std::bad_alloc) {
            fprintf(stderr, "Memory allocation error\n");
        }

        if(fd != -1) {
            if(shutdown(fd, 2) == -1)
                fprintf(stderr, "Can't shutdown socket\n");

            if(close(fd))
                fprintf(stderr, "Can't realese file descriptor\n");
        }
    }
}

