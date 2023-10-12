#include<stdio.h>
#include<stdint.h>
#include<stdbool.h>
#include<string.h>
#include<unistd.h>
#include<netdb.h>
#include<dirent.h>
#include<sys/stat.h>

#define REQUEST_SIZE_MAX 1048576

typedef struct Header {
    char *name;
    char *value;
} Header;

typedef struct Request {
    char ip_address[100];
    char data[REQUEST_SIZE_MAX + 1];
    uint32_t payload_size;
    char *payload;
} Request;

int8_t server_init(uint16_t port)
{
    int8_t socket_server = socket(AF_INET, SOCK_STREAM, 6);
    fprintf(stdout, "Socket Created...\n");

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

#ifdef RELEASE
    if (bind(socket_server, (struct sockaddr *)&addr, sizeof(addr)))
    {
        fprintf(stderr, "Failed to bind() on port: %hu\n", port);
        exit(1);
    }
#else
    while (bind(socket_server, (struct sockaddr *)&addr, sizeof(addr)))
    {
        fprintf(stderr, "Failed to bind() on port: %hu\n", port);
        port++;
        addr.sin_port = htons(port);
    }
#endif

    fprintf(stdout, "Bind socket to adress on pornt: %hu\n", port);

    listen(socket_server, 1);
    fprintf(stdout, "Listen to connections...\n");

    return socket_server;
}

void request_client_information(Request *request, struct sockaddr *address_client)
{
    getnameinfo(address_client, sizeof(*address_client), request->ip_address, sizeof(request->ip_address), 0, 0, NI_NUMERICHOST);
}

bool header_find(Header *header, char *data, uint32_t data_size, char *name)
{
    uint32_t i, j;
    i = 0;
    j = 0;

    do
    {
        if (i >= data_size)
            return false;

        if (data[i] == name[j])
            j++;
        else
            j = 0;

        i++;
    }
    while (j < strlen(name));
    header->name = data;
    header->name[i] = 0;

    i += 2;

    header->value = data + i;

    for (; data[i] != '\r'; i++)
        if (i >= data_size)
            return false;

    data[i] = 0;

    return true;
}

bool request_parse(Request *request)
{
    request->payload = strstr(request->data, "\r\n\r\n");
    request->payload += 4;

    /* Content-Ength Header */
    uint32_t header_data_size = request->payload - request->data;
    Header header;
    if (header_find(&header, request->data, header_data_size, "Content-Length"))
        sscanf(header.value, "%u", &request->payload_size);
    else
        request->payload_size = 0;


    return true;
}

void response_send_header(int8_t client_socket, char *header, char *value)
{
    write(client_socket, header, strlen(header));
    write(client_socket, ": ", 2);
    write(client_socket, value, strlen(value));
    write(client_socket, "\r\n", 2);
}

void number_to_hex_str(char *hex_str, uint32_t number)
{
    sprintf(hex_str, "%x", number);
}

void response_send_chunk(int8_t client_socket, char *data, uint32_t data_size)
{
    char hex_str[9];
    number_to_hex_str(hex_str, data_size);
    write(client_socket, hex_str, strlen(hex_str));
    write(client_socket, "\r\n", 2);
    write(client_socket, data, data_size);
    write(client_socket, "\r\n", 2);
}

void response_send_file_chunked(int8_t client_socket, char *path)
{
    FILE *f = fopen(path, "r");
#define FILE_BUFFER_MAX 2048
    char buffer[FILE_BUFFER_MAX];
    uint32_t bytes;
    while ((bytes = fread(buffer, 1, FILE_BUFFER_MAX, f)))
    {
        buffer[bytes] = 0;
        response_send_chunk(client_socket, buffer, bytes);
    }
    write(client_socket, "0\r\n\r\n", 5);
    fclose(f);
}

void response_send(int8_t socket_client, Request *request)
{
    if (strncmp(request->data, "GET", 3) == 0)
    {
        if (strncmp(request->data + 4, "/ ", 2) == 0)
        {
            write(socket_client, "HTTP/1.1 301 Moved Permanently\r\n", 32);
            response_send_header(socket_client, "Connection", "close");
            response_send_header(socket_client, "Location", "/index.html");
            response_send_header(socket_client, "Content-Length", "0");
            write(socket_client, "\r\n", 2);
            return;
        }

        if (strncmp(request->data + 4, "/index.html", 11) == 0)
        {
            fprintf(stdout, "Response File: %s\n", "index.html");
            write(socket_client, "HTTP/1.1 200 OK\r\n", 17);
            response_send_header(socket_client, "Connection", "close");
            response_send_header(socket_client, "Content-Type", "text/html");
            response_send_header(socket_client, "Transfer-Encoding", "chunked");
            write(socket_client, "\r\n", 2);
            response_send_file_chunked(socket_client, "index.html");
            return;
        }
        if (strncmp(request->data + 4, "/ldek.js", 8) == 0)
        {
            fprintf(stdout, "Response File: %s\n", "ldek.js");
            write(socket_client, "HTTP/1.1 200 OK\r\n", 17);
            response_send_header(socket_client, "Connection", "close");
            response_send_header(socket_client, "Content-Type", "application/javascript");
            response_send_header(socket_client, "Transfer-Encoding", "chunked");
            write(socket_client, "\r\n", 2);
            response_send_file_chunked(socket_client, "ldek.js");
            return;
        }
        if (strncmp(request->data + 4, "/ldek_style.css", 15) == 0)
        {
            fprintf(stdout, "Response File: %s\n", "ldek_style.css");
            write(socket_client, "HTTP/1.1 200 OK\r\n", 17);
            response_send_header(socket_client, "Connection", "close");
            response_send_header(socket_client, "Content-Type", "text/css");
            response_send_header(socket_client, "Transfer-Encoding", "chunked");
            write(socket_client, "\r\n", 2);
            response_send_file_chunked(socket_client, "ldek_style.css");
            return;
        }
        if (strncmp(request->data + 4, "/res/icon.png", 13) == 0)
        {
            fprintf(stdout, "Response File: %s\n", "res/icon.png");
            write(socket_client, "HTTP/1.1 200 OK\r\n", 17);
            response_send_header(socket_client, "Connection", "close");
            response_send_header(socket_client, "Content-Type", "image/png");
            response_send_header(socket_client, "Transfer-Encoding", "chunked");
            write(socket_client, "\r\n", 2);
            response_send_file_chunked(socket_client, "res/icon.png");
            return;
        }
    }

    if (strncmp(request->data, "POST", 4) == 0)
    {
        if (strncmp(request->data + 5, "/save", 5) == 0)
        {
            char *path;
            path = strtok(request->payload, ";");
            char *data = strtok(NULL, ";");
            printf("data length: %ld\n", strlen(data));

            chdir("../data");
            FILE *f = fopen(path, "w+");
            fwrite(data, sizeof(*data), strlen(data), f);
            fclose(f);
            chdir("../frontend");

            write(socket_client, "HTTP/1.1 204 No Content\r\n", 25); 
            write(socket_client, "\r\n\r\n", 4);
            return;
        }

        if (strncmp(request->data + 5, "/open", 5) == 0)
        {
            DIR *dir = opendir("../data");
            struct dirent *ptr;

            write(socket_client, "HTTP/1.1 302 Found\r\n", 20);
            response_send_header(socket_client, "Content-Type", "text/plain");
            response_send_header(socket_client, "Transfer-Encoding", "chunked");
            write(socket_client, "\r\n", 2);

            while ((ptr = readdir(dir)) != NULL)
            {
                if (strncmp(ptr->d_name, "..", 2) == 0 || strncmp(ptr->d_name, ".", 1) == 0)
                    continue;
                else
                {
                    response_send_chunk(socket_client, ptr->d_name, strlen(ptr->d_name));
                    break;
                }
            }

            while ((ptr = readdir(dir)) != NULL)
            {
                if (strncmp(ptr->d_name, "..", 2) == 0 || strncmp(ptr->d_name, ".", 1) == 0)
                    continue;

                printf("%s\n", ptr->d_name);
                response_send_chunk(socket_client, ",", 1);
                response_send_chunk(socket_client, ptr->d_name, strlen(ptr->d_name));
            }
            write(socket_client, "0\r\n\r\n", 5);
        }

        if (strncmp(request->data + 5, "/load", 5) == 0)
        {
            write(socket_client, "HTTP/1.1 200 Ok\r\n", 17);
            response_send_header(socket_client, "Transfer-Encoding", "chunked");
            response_send_header(socket_client, "Content-Type", "text/plain");
            write(socket_client, "\r\n", 2);

            chdir("../data");
            response_send_file_chunked(socket_client, request->payload);
            chdir("../frontend");
        }

        if (strncmp(request->data + 5, "/delete", 7) == 0)
        {
            write(socket_client, "HTTP/1.1 200 OK\r\n", 17);
            chdir("../data");
            fprintf(stdout, "delete test: %s\n", request->payload);
            remove(request->payload);
            chdir("../frontend");
            write(socket_client, "\r\n\r\n", 4);
        }
    }
}

int main(int argc, char **argv)
{
    mkdir("data", 0777);
    chdir("frontend");

    Request request;

    int8_t socket_server = server_init(8080);

    struct sockaddr address_client;
    uint32_t address_len = sizeof(address_client);

    for (;;)
    {
        int8_t socket_client = accept(socket_server, &address_client, &address_len);
        request_client_information(&request, &address_client);
        printf("IP: %s\n", request.ip_address);
        uint16_t bytes_received = read(socket_client, request.data, REQUEST_SIZE_MAX);
        request.data[bytes_received] = 0;

        if (bytes_received < 1)
            fprintf(stdout, "Unexpected disconnection\n");
        else if (request_parse(&request))
        {
            fprintf(stdout, "payload size: %d\n", request.payload_size);
            response_send(socket_client, &request);
        }
        else
        {
            write(socket_client, "HTTP/1.1 404 Not Found\r\n\r\n", 26);
            fprintf(stdout, "Request was not valid\n");
        }
        close(socket_client);
    }

    return 0;
}
