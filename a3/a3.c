#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define REQ_PIPE_NAME "REQ_PIPE_75814"
#define RESP_PIPE_NAME "RESP_PIPE_75814"
#define BUFFER_SIZE 256

int main(int argc, char **argv)
{

    if (mkfifo(RESP_PIPE_NAME, 0666) < 0)
    {
        printf("ERROR\ncannot create the response pipe");
    }

    int req_pipe, resp_pipe;

    req_pipe = open(REQ_PIPE_NAME, O_RDONLY);
    if (req_pipe == -1)
    {
        printf("ERROR\ncannot open the request pipe");
    }

    resp_pipe = open(RESP_PIPE_NAME, O_WRONLY);
    if (resp_pipe == -1)
    {
        printf("ERROR\ncannot open the response pipe");
    }

    printf("SUCCESS\n");

    write(resp_pipe, "START$", 6);

    while (1)
    {
        char buffer[BUFFER_SIZE];
        unsigned char x;
        read(req_pipe, &x, sizeof(unsigned char));
        int i = 0;

        while (x != '$')
        {
            buffer[i] = x;
            i++;
            read(req_pipe, &x, sizeof(unsigned char));
        }

        buffer[i] = '\0';

        if (strcmp(buffer, "PING") == 0)
        {
            write(resp_pipe, "PING$", 5);
            write(resp_pipe, "PONG$", 5);
            unsigned int y = 75814;
            write(resp_pipe, &y, sizeof(unsigned int));
        }
        else if (strcmp(buffer, "EXIT") == 0)
        {
            close(req_pipe);
            close(resp_pipe);
            unlink(RESP_PIPE_NAME);
        }
        else
            break;
    }

}
