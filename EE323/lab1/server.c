#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>

#define MAX_SIZE 9999992

typedef struct
{
    uint16_t op;
    uint16_t shift;
    uint32_t length;
} header;

void sigchld_handler()
{
    int status;
    waitpid(-1, &status, WNOHANG);
}

void caesar(char *buffer, uint16_t shift, size_t cur_length)
{
    for (size_t i = 0; i < cur_length; i++)
    {
        if (buffer[i] >= 'a' && buffer[i] <= 'z')
            buffer[i] = 'a' + (buffer[i] - 'a' + shift) % 26;
        else if (buffer[i] >= 'A' && buffer[i] <= 'Z')
            buffer[i] = 'a' + (buffer[i] - 'A' + shift) % 26;
    }
}

void send_msg(int clnt_sock, header h, char *buffer, size_t cur_length)
{
    uint16_t op = h.op;
    uint16_t shift = h.shift;
    h.length = htonl(h.length);

    if (op == 0)
        caesar(buffer, shift, cur_length);
    else if (op == 1)
        caesar(buffer, 26 - shift, cur_length);
    else
        return;

    send(clnt_sock, &h, sizeof(h), 0);
    send(clnt_sock, buffer, cur_length, 0);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        exit(EXIT_FAILURE);
    }
    int serv_sock, clnt_sock;
    struct sockaddr_in serv_addr, clnt_addr;
    socklen_t clnt_addr_size = sizeof(clnt_addr);

    pid_t pid;

    serv_sock = socket(AF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[2]));

    if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        exit(EXIT_FAILURE);
    if (listen(serv_sock, 50) < 0)
        exit(EXIT_FAILURE);

    signal(SIGCHLD, sigchld_handler);

    while (1)
    {
        clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);

        size_t cur_bytes, cur_length;
        header h;
        pid = fork();

        if (pid == -1)
        {
            close(clnt_sock);
            continue;
        }
        if (pid == 0)
        {
            close(serv_sock);
            while (recv(clnt_sock, &h, sizeof(header), 0) > 0)
            {
                h.length = ntohl(h.length);
                if (h.op != 0 && h.op != 1)
                {
                    sleep(1);
                    break;
                }
                size_t buf_size = h.length - sizeof(header);
                if (buf_size > MAX_SIZE)
                {
                    sleep(1);
                    break;
                }
                char *buffer = (char *)malloc(buf_size + 1);
                cur_length = 0;

                while (cur_length < buf_size)
                {
                    cur_bytes = recv(clnt_sock, &buffer[cur_length], sizeof(buffer), 0);
                    cur_length += cur_bytes;
                }

                if (cur_length != buf_size)
                {
                    send_msg(clnt_sock, h, buffer, buf_size);
                    sleep(1);
                    break;
                }

                send_msg(clnt_sock, h, buffer, cur_length);
            }
            close(clnt_sock);
            return 0;
        }
        else
        {
            close(clnt_sock);
        }
    }
    close(serv_sock);
    return 0;
}