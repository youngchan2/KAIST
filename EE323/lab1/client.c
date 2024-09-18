#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

#define MAX_SIZE 9999992

typedef struct
{
    uint16_t op;
    uint16_t shift;
    uint32_t length;
} header;

void send_msg(int sock, char *buffer, uint16_t op, uint16_t shift, uint32_t length)
{
    header h;
    h.op = op;
    h.shift = shift;
    h.length = htonl(length + sizeof(header));
    // h.length = htonl(1001);
    send(sock, &h, sizeof(h), 0);
    send(sock, buffer, length, 0);
}

int recv_msg(int sock)
{
    size_t cur_bytes, cur_length;
    header h;
    recv(sock, &h, sizeof(header), 0);
    h.length = ntohl(h.length);
    size_t buf_size = h.length - sizeof(header);

    char *buffer = (char *)malloc(buf_size + 1);
    cur_length = 0;
    while (cur_length < buf_size)
    {
        cur_bytes = recv(sock, &buffer[cur_length], sizeof(buffer), 0);
        if (cur_bytes == 0)
        {
            free(buffer);
            return -1;
        }
        cur_length += cur_bytes;
    }
    fwrite(buffer, 1, strlen(buffer), stdout);
    fflush(stdout);
    free(buffer);

    return 1;
}

int main(int argc, char *argv[])
{
    if (argc != 9)
        exit(EXIT_FAILURE);

    char *addr;
    int port;
    int cnt_h = 0;
    int cnt_p = 0;
    int cnt_o = 0;
    int cnt_s = 0;
    int op;
    int shift;
    for (int i = 1; i < 9; i += 2)
    {
        if (!strcmp(argv[i], "-h"))
        {
            addr = argv[i + 1];
            cnt_h++;
        }
        else if (!strcmp(argv[i], "-p"))
        {
            port = atoi(argv[i + 1]);
            cnt_p++;
        }
        else if (!strcmp(argv[i], "-o"))
        {
            op = atoi(argv[i + 1]);
            cnt_o++;
        }
        else if (!strcmp(argv[i], "-s"))
        {
            shift = (atoi(argv[i + 1]) % 26);
            cnt_s++;
        }
    }
    if (cnt_h != 1 || cnt_o != 1 || cnt_p != 1 || cnt_s != 1)
        exit(EXIT_FAILURE);

    int sock;
    struct sockaddr_in serv_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(addr);
    serv_addr.sin_port = htons(port);

    int connect_chk = connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    if (connect_chk == -1)
        exit(EXIT_FAILURE);

    char *buffer = (char *)malloc(MAX_SIZE);
    size_t cur_bytes;

    while ((cur_bytes = fread(buffer, 1, MAX_SIZE, stdin)) > 0)
    {
        if (feof(stdin) || cur_bytes >= MAX_SIZE)
        {
            send_msg(sock, buffer, op, shift, cur_bytes);

            if (recv_msg(sock) < 0)
                break;
            free(buffer);
            buffer = (char *)malloc(MAX_SIZE);
        }
    }
    close(sock);
    free(buffer);

    return 0;
}
