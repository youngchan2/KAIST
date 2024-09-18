#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

#define BUFSIZE 1000000
#define BADREQUEST 0
#define BLACK 2

int parse(char *msg, char **host, int *port, int redirection, char ***blacklist, int black_cnt)
{
    int flag = 1;
    int host_len;
    *port = 80;
    char *host_start;
    char *host_end;
    char *uri_start;
    char *uri_end;

    if (strncmp(msg, "GET", 3))
        flag = BADREQUEST;
    if (!strstr(msg, "HTTP/1.0"))
        flag = BADREQUEST;

    host_start = strstr(msg, "Host:");
    if (host_start == NULL)
    {
        flag = BADREQUEST;
        return flag;
    }
    host_start += 6;

    host_end = strstr(host_start, "\r\n");
    if (host_end == NULL)
        flag = BADREQUEST;
    host_len = host_end - host_start;

    // use only Host header
    memset(host_end, '\0', strlen(host_end));
    strcat(msg, "\r\n\r\n");

    // get host
    *host = (char *)malloc(host_len + 1);
    strncpy(*host, host_start, host_len);
    (*host)[host_len] = '\0';

    char *isport = strstr(host_start, ":");
    if (isport)
    {
        *port = atoi(isport + 1);
    }

    // compare uri & host
    uri_start = strstr(msg, "http://");
    if (uri_start != NULL)
    {
        uri_start += 7;
        uri_end = strchr(uri_start, ' ');
        char *path = strchr(uri_start, '/');
        if (path != NULL && path < uri_end)
            uri_end = path;
        host_len = uri_end - uri_start;
        char *uri = malloc(host_len + 1);
        strncpy(uri, uri_start, host_len);
        uri[host_len] = '\0';
        if (strcmp(uri, *host))
        {
            flag = BADREQUEST;
        }
        free(uri);
    }
    if (flag == BADREQUEST)
    {
        return flag;
    }
    if (redirection == 1)
    {
        uri_start -= 7;
        host_len = uri_end - uri_start;
        char *uri = malloc(host_len + 1);
        strncpy(uri, uri_start, host_len);
        for (int i = 0; i < black_cnt; i++)
        {
            if (!strcmp(uri, (*blacklist)[i]))
                flag = BLACK;
        }
        free(uri);
    }
    return flag;
}

char **make_blacklist(int *cnt)
{
    char **list = malloc(100 * sizeof(char *));
    char *blackuri;
    size_t n = 0;
    ssize_t len;
    int i = 0;
    while ((len = getline(&blackuri, &n, stdin)) != -1)
    {
        if (len > 0 && blackuri[len - 1] == '\n')
            blackuri[len - 1] = '\0';
        list[i] = strdup(blackuri);
        i++;
    }
    *cnt = i;
    return list;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
        exit(EXIT_FAILURE);

    int command_port = atoi(argv[1]);

    if (command_port < 1024 || command_port > 65535)
        exit(EXIT_FAILURE);

    int redirection = 0;
    int black_cnt;
    char **blacklist = NULL;
    if (!isatty(fileno(stdin)))
    {
        redirection = 1;
        blacklist = make_blacklist(&black_cnt);
    }

    int proxy_sock, clnt_sock, host_sock;
    struct sockaddr_in proxy_addr, clnt_addr, host_addr;
    struct hostent *h;
    socklen_t clnt_addr_size = sizeof(clnt_addr);

    proxy_sock = socket(AF_INET, SOCK_STREAM, 0);

    memset(&proxy_addr, 0, sizeof(proxy_addr));
    proxy_addr.sin_family = AF_INET;
    proxy_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    proxy_addr.sin_port = htons(command_port);

    bind(proxy_sock, (struct sockaddr *)&proxy_addr, sizeof(proxy_addr));
    listen(proxy_sock, 50);

    pid_t pid;

    while (1)
    {
        char *msg = malloc(BUFSIZE);
        clnt_sock = accept(proxy_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);

        pid = fork();

        if (pid == -1)
        {
            fprintf(stderr, "fork error:%s\n", strerror(errno));
        }
        else if (pid == 0)
        {
            size_t len_recv = 0;
            size_t cur_length = 0;
            char *msg_400 = "HTTP/1.0 400 Bad Request\r\n\r\n";
            char *crlf = "\r\n\r\n";
            char *p = NULL;
            char *host;
            int port;

            // recv client request msg (buffer)
            while (1)
            {
                len_recv = recv(clnt_sock, &msg[cur_length], BUFSIZE, 0);
                cur_length += len_recv;
                p = strstr(msg, crlf);
                if (p != NULL || cur_length > BUFSIZE)
                    break;
            }
            // request whether 400 or not
            int flag = parse(msg, &host, &port, redirection, &blacklist, black_cnt);
            if (flag == BLACK)
            {
                strcpy(msg, "GET / HTTP/1.0\r\nHost: www.warning.or.kr\r\n\r\n");
                strcpy(host, "www.warning.or.kr");
            }
            h = gethostbyname(host);
            if (h == NULL)
            {
                flag = BADREQUEST;
            }
            if (flag == BADREQUEST)
            {
                send(clnt_sock, msg_400, strlen(msg_400), 0);
                exit(EXIT_SUCCESS);
            }
            else // send request msg to host(server)
            {
                host_sock = socket(AF_INET, SOCK_STREAM, 0);
                host_addr.sin_family = AF_INET;
                host_addr.sin_port = htons(port);
                memcpy(&host_addr.sin_addr, h->h_addr_list[0], h->h_length);
                if (connect(host_sock, (struct sockaddr *)&host_addr, sizeof(host_addr)) < 0)
                {
                    exit(EXIT_FAILURE);
                }
                if ((send(host_sock, msg, strlen(msg), 0)) < 0)
                {
                    exit(EXIT_FAILURE);
                }
            }
            // recv response from host(server);
            len_recv = 0;
            cur_length = 0;
            char *response_msg = malloc(BUFSIZE);
            while (1)
            {
                len_recv = recv(host_sock, &response_msg[cur_length], sizeof(response_msg), 0);
                if (len_recv == 0)
                    break;
                cur_length += len_recv;
            }
            if (send(clnt_sock, response_msg, cur_length, 0) < 0)
            {
                fprintf(stderr, "response message error: %s\n", strerror(errno));
            }
            close(host_sock);
            free(response_msg);
            free(msg);
        }
        close(clnt_sock);
    }
    close(proxy_sock);
    return 0;
}