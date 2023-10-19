#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>
#include<stdbool.h>
#include<netdb.h>
#include<unistd.h>
#include<sys/stat.h>
#include<dirent.h>

/* Ansi Escape Codes */
#define ANSI_ESCAPE_COLOR_RED "\033[31m"
#define ANSI_ESCAPE_COLOR_GREEN "\033[32m"
#define ANSI_ESCAPE_COLOR_ORANGE "\033[38;5;208m"
#define ANSI_ESCAPE_COLOR_DEFAULT "\033[m"

/* Respons Codes */
#define RESPONSE_CODE_200 "HTTP/1.1 200 OK"
#define RESPONSE_CODE_201 "HTTP/1.1 201 Created"
#define RESPONSE_CODE_301 "HTTP/1.1 301 Moved Permanetly"
#define RESPONSE_CODE_307 "HTTP/1.1 307 Temporary Redirect"
#define RESPONSE_CODE_400 "HTTP/1.1 400 Bad Request"
#define RESPONSE_CODE_403 "HTTP/1.1 403 Forbidden"
#define RESPONSE_CODE_404 "HTTP/1.1 404 Not Found"

#define REQUEST_SIZE_MAX 2024

typedef struct ContentDynamic {
    char *identify;
    char *value;
    void (*func)(int16_t socket_client);
} ContentDynamic;

typedef struct Header {
    char *name;
    char *value;
} Header;

typedef struct Request {
    char ip_address[100];
    char data[REQUEST_SIZE_MAX + 1];
    Header *header;
    uint32_t payload_size;
    char *payload;
    char cookie_session[37];
} Request;

/* globals */
char *ldek_data_file_path_current;
ContentDynamic content_dynamic_dek_text;
ContentDynamic content_dynamic_dek_open;
ContentDynamic content_dynamic_dek_file_name;

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

    fprintf(stdout, "Bind socket to address on port: %hu\n", port);

    listen(socket_server, 10);

    fprintf(stdout, "Listen to connections...\n");

    return socket_server;
}

void content_dynamic_func_set(ContentDynamic *content_dynamic, void (*func)(int16_t socket_client))
{
    content_dynamic->value = NULL;
    content_dynamic->func = func;
}

void content_dynamic_default_set(ContentDynamic *content_dynamic)
{
    content_dynamic->value = "";
    content_dynamic->func = NULL;
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

void number_to_hex_str(char *hex_str, uint32_t number)
{
    sprintf(hex_str, "%x", number);
}

void response_send_chunk(int8_t socket_client, char *data, uint32_t data_size)
{
    char hex_str[9];
    number_to_hex_str(hex_str, data_size);
    write(socket_client, hex_str, strlen(hex_str));
    write(socket_client, "\r\n", 2);
    write(socket_client, data, data_size);
    write(socket_client, "\r\n", 2);
}

void response_send_file_chunked(int8_t client_socket, char *path)
{
    FILE *f = fopen(path, "r");
    char buffer[2048];
    uint32_t bytes;
    while ((bytes = fread(buffer, 1, 2048, f)))
    {
        buffer[bytes] = 0;
        response_send_chunk(client_socket, buffer, bytes);
    }
    write(client_socket, "0\r\n\r\n", 5);
    fclose(f);
}

int32_t content_dynamic_find_index(ContentDynamic *content_dynamic, uint32_t content_dynamic_count, char *identify, uint32_t identify_size)
{
    uint32_t i;
    for (i = 0; i < content_dynamic_count; i++)
    {
        if (strncmp(content_dynamic[i].identify, identify, identify_size) == 0)
            return i;
    }

    return -1;
}

void response_send_file_chunked_dynamic(int8_t socket_client, char *path, ContentDynamic *content_dynamic, uint32_t content_dynamic_count)
{
    FILE *f = fopen(path, "r");
#define BUFFER_MAX 2048

    char buffer[BUFFER_MAX];

    uint32_t i = 0;
    char c;
    while ((c = fgetc(f)) != EOF)
    {
        if (c == '{')
        {
            if (buffer[0] == '{')
            {
                while ((c = fgetc(f)) != '}')
                {
                    buffer[i] = c;
                    i++;
                }
                fgetc(f);
                c = fgetc(f);

                /* go throuh content_dynamic */
                int32_t index = content_dynamic_find_index(content_dynamic, content_dynamic_count, buffer + 1, i - 1);
                if (index == -1)
                    fprintf(stderr, ANSI_ESCAPE_COLOR_RED "content_dynamic not found: {%.*s}}\n" ANSI_ESCAPE_COLOR_DEFAULT, i, buffer);
                else
                {
                    if (content_dynamic[index].value == NULL)
                        content_dynamic[index].func(socket_client);
                    else if (strlen(content_dynamic[index].value) > 1)
                        response_send_chunk(socket_client, content_dynamic[index].value, strlen(content_dynamic[index].value));
                }
                i = 0;
            }
            else
            {
                response_send_chunk(socket_client, buffer, i);
                i = 0;
            }
        }

        if (i == BUFFER_MAX)
        {
            response_send_chunk(socket_client, buffer, i);
            i = 0;
        }

        buffer[i] = c;
        i++;
    }
    response_send_chunk(socket_client, buffer, i);

    write(socket_client, "0\r\n\r\n", 5);
    fclose(f);
}

void response_send_code(int16_t socket_client, char *response_code)
{
    write(socket_client, response_code, strlen(response_code));
    write(socket_client, "\r\n", 2);
}

void response_send_header(int16_t client_socket, char *header, char *value)
{
    write(client_socket, header, strlen(header));
    write(client_socket, ": ", 2);
    write(client_socket, value, strlen(value));
    write(client_socket, "\r\n", 2);
}

bool request_parse(Request *request)
{
    request->payload = strstr(request->data, "\r\n\r\n");
    request->payload += 4;

    uint32_t header_data_size = request->payload - request->data;
    Header header;
    if (header_find(&header, request->data, header_data_size, "Content-Length"))
        sscanf(header.value, "%u", &request->payload_size);

    if (request->payload - request->data + request->payload_size >= REQUEST_SIZE_MAX)
    {
        fprintf(stderr, "REQUEST_SIZE_MAX overload\n");
        return false;
    }

    return true;
}

void index_html_content_dynamic_func(int16_t socket_client)
{
    response_send_chunk(socket_client, "matthias", 8);
}

void index_html_content_dynamic_open_func(int16_t socket_client)
{
    DIR *dir = opendir("../data");
    struct dirent *ptr;

    while ((ptr = readdir(dir)) != NULL)
    {
        if (strncmp(ptr->d_name, "..", 2) == 0 || strncmp(ptr->d_name, ".", 1) == 0)
            continue;

        response_send_chunk(socket_client, "<div class=\"dek-item\"><div onclick=dek_load(event)>", 53 - 2);
        response_send_chunk(socket_client, ptr->d_name, strlen(ptr->d_name));
        response_send_chunk(socket_client, "</div><button onclick=dek_delete(event)></button></div>", 55);
    }
}

void index_html_content_dynamic_load_func(int16_t socket_client)
{
    chdir("../data");
    FILE *f = fopen(ldek_data_file_path_current, "r");

    int c;
    int i = 0;
    char sword[150];
    char dword[150];
    char *ptr = sword;
    while ((c = fgetc(f)) != EOF)
    {
        if (c == ',')
        {
            dword[i] = 0;
            response_send_chunk(socket_client, "<div class=\"dek-object\"><p class=\"word-object\" contenteditable=\"true\">", 76 - 6);
            if (strlen(sword) > 0)
                response_send_chunk(socket_client, sword, strlen(sword));
            response_send_chunk(socket_client, "</p><p class=\"word-object\" contenteditable=\"true\">", 54 - 4);
            if (strlen(dword) > 0)
                response_send_chunk(socket_client, dword, strlen(dword));
            response_send_chunk(socket_client, "</p></div>", 10);

            i = 0;
            ptr = sword;

            continue;
        }

        if (c == ':')
        {
            ptr = dword;
            sword[i] = 0;
            i = 0;
            continue;
        }

        ptr[i] = c;
        i++;

    }

    chdir("../frontend");
}

void request_send(int16_t socket_client, Request *request)
{
    char *first_line_end = strstr(request->data, "\r\n");
    if (first_line_end)
        fprintf(stdout, "Request <- %.*s\n", (int)(first_line_end - request->data), request->data);
    else
        fprintf(stdout, "Request <- %s\n", request->data);

    if (strncmp(request->data, "GET", 3) == 0)
    {
        if (strncmp(request->data + 4, "/ ", 2) == 0)
        {
            response_send_code(socket_client, RESPONSE_CODE_200);
            response_send_header(socket_client, "Connection", "close");
            response_send_header(socket_client, "Content-Type", "text/html");
            response_send_header(socket_client, "Transfer-Encoding", "chunked");
            write(socket_client, "\r\n", 2);

            ContentDynamic content_dynamics[3] = {
                content_dynamic_dek_text,
                content_dynamic_dek_open,
                content_dynamic_dek_file_name,
            };
            response_send_file_chunked_dynamic(socket_client, "index.html", content_dynamics, 3);

            fprintf(stdout, ANSI_ESCAPE_COLOR_GREEN "%s" ANSI_ESCAPE_COLOR_DEFAULT " -> Chunked File: index.html -> Response\n", RESPONSE_CODE_200);
            return;
        }

        if (strncmp(request->data + 4, "/ldek_style.css", 15) == 0)
        {
            response_send_code(socket_client, RESPONSE_CODE_200);
            response_send_header(socket_client, "Connection", "close");
            response_send_header(socket_client, "Content-Type", "text/css");
            response_send_header(socket_client, "Transfer-Encoding", "chunked");
            write(socket_client, "\r\n", 2);
            response_send_file_chunked(socket_client, "ldek_style.css");

            fprintf(stdout, ANSI_ESCAPE_COLOR_GREEN "%s" ANSI_ESCAPE_COLOR_DEFAULT " -> Chunked File: ldek_style.css -> Response\n", RESPONSE_CODE_200);
            return;
        }

        if (strncmp(request->data + 4, "/ldek.js", 8) == 0)
        {
            response_send_code(socket_client, RESPONSE_CODE_200);
            response_send_header(socket_client, "Connection", "close");
            response_send_header(socket_client, "Content-Type", "application/javascript");
            response_send_header(socket_client, "Transfer-Encoding", "chunked");
            write(socket_client, "\r\n", 2);
            response_send_file_chunked(socket_client, "ldek.js");

            fprintf(stdout, ANSI_ESCAPE_COLOR_GREEN "%s" ANSI_ESCAPE_COLOR_DEFAULT " -> Chunked File: ldek.js -> Response\n", RESPONSE_CODE_200);
            return;
        }

        if (strncmp(request->data + 4, "/res/icon.png", 13) == 0)
        {
            response_send_code(socket_client, RESPONSE_CODE_200);
            response_send_header(socket_client, "Connection", "close");
            response_send_header(socket_client, "Content-Type", "image/png");
            response_send_header(socket_client, "Transfer-Encoding", "chunked");
            write(socket_client, "\r\n", 2);
            response_send_file_chunked(socket_client, "res/icon.png");

            fprintf(stdout, ANSI_ESCAPE_COLOR_GREEN "%s" ANSI_ESCAPE_COLOR_DEFAULT " -> Chunked File: res/icon.png -> Response\n", RESPONSE_CODE_200);
            return;
        }

        response_send_code(socket_client, RESPONSE_CODE_404);
        response_send_header(socket_client, "Content-Length", "0");
        write(socket_client, "\r\n\r\n", 4);
        fprintf(stdout, ANSI_ESCAPE_COLOR_RED "%s" ANSI_ESCAPE_COLOR_DEFAULT " -> Response\n", RESPONSE_CODE_404);
        return;
    }

    if (strncmp(request->data, "POST", 4) == 0)
    {
        if (strncmp(request->data + 5, "/save", 4) == 0)
        {
            char *path;
            path = strtok(request->payload, ";");
            char *data = strtok(NULL, ";");

            chdir("../data");
            FILE *f = fopen(path, "w+");
            fwrite(data, sizeof(*data), strlen(data), f);
            fclose(f);
            chdir("../frontend");

            response_send_code(socket_client, RESPONSE_CODE_201);
            response_send_header(socket_client, "Content-Length", "0");
            write(socket_client, "\r\n\r\n", 4);
            fprintf(stdout, ANSI_ESCAPE_COLOR_GREEN "%s" ANSI_ESCAPE_COLOR_DEFAULT " -> Response\n", RESPONSE_CODE_201);

            return;
        }

        if (strncmp(request->data + 5, "/load", 5) == 0)
        {
            content_dynamic_default_set(&content_dynamic_dek_open);
            content_dynamic_func_set(&content_dynamic_dek_text, index_html_content_dynamic_load_func);

            if (ldek_data_file_path_current != NULL)
                free(ldek_data_file_path_current);
            ldek_data_file_path_current = malloc((strlen(request->payload) + 1) * sizeof(char));
            strcpy(ldek_data_file_path_current, request->payload);
            ldek_data_file_path_current[strlen(request->payload)] = 0;

            content_dynamic_dek_file_name.value = ldek_data_file_path_current;


            response_send_code(socket_client, RESPONSE_CODE_200);
            response_send_header(socket_client, "Connection", "close");
            response_send_header(socket_client, "Content-Type", "text/plain");
            write(socket_client, "\r\n\r\n", 4);
            fprintf(stdout, ANSI_ESCAPE_COLOR_GREEN "%s" ANSI_ESCAPE_COLOR_DEFAULT " -> Response\n", RESPONSE_CODE_200);

            return;
        }

        if (strncmp(request->data + 5, "/open", 5) == 0)
        {
            content_dynamic_default_set(&content_dynamic_dek_file_name);

            content_dynamic_default_set(&content_dynamic_dek_text);
            content_dynamic_func_set(&content_dynamic_dek_open, index_html_content_dynamic_open_func);
#if 0
            response_send_code(socket_client, RESPONSE_CODE_307);
            response_send_header(socket_client, "Connection", "close");
            response_send_header(socket_client, "Location", "/");
            write(socket_client, "\r\n\r\n", 4);
            fprintf(stdout, ANSI_ESCAPE_COLOR_ORANGE "%s" ANSI_ESCAPE_COLOR_DEFAULT " -> Response\n", RESPONSE_CODE_307);
#else
            response_send_code(socket_client, RESPONSE_CODE_200);
            response_send_header(socket_client, "Connection", "close");
            response_send_header(socket_client, "Content-Type", "text/plain");
            write(socket_client, "\r\n\r\n", 4);
            fprintf(stdout, ANSI_ESCAPE_COLOR_GREEN "%s" ANSI_ESCAPE_COLOR_DEFAULT " -> Response\n", RESPONSE_CODE_200);
#endif

            return;
        }

        if (strncmp(request->data + 5, "/delete", 7) == 0)
        {
            content_dynamic_func_set(&content_dynamic_dek_text, index_html_content_dynamic_open_func);

            chdir("../data");
            fprintf(stdout, "delete test: %s\n", request->payload);
            remove(request->payload);
            chdir("../frontend");
            write(socket_client, "\r\n\r\n", 4);

#if 0
            response_send_code(socket_client, RESPONSE_CODE_307);
            response_send_header(socket_client, "Connection", "close");
            response_send_header(socket_client, "Location", "/");
            write(socket_client, "\r\n\r\n", 4);
            fprintf(stdout, ANSI_ESCAPE_COLOR_ORANGE "%s" ANSI_ESCAPE_COLOR_DEFAULT " -> Response\n", RESPONSE_CODE_307);
#else
            response_send_code(socket_client, RESPONSE_CODE_200);
            response_send_header(socket_client, "Connection", "close");
            write(socket_client, "\r\n\r\n", 4);
            fprintf(stdout, ANSI_ESCAPE_COLOR_GREEN "%s" ANSI_ESCAPE_COLOR_DEFAULT " -> Response\n", RESPONSE_CODE_200);
#endif

            return;
        }
    }

    response_send_code(socket_client, RESPONSE_CODE_403);
    response_send_header(socket_client, "Content-Length", "0");
    write(socket_client, "\r\n\r\n", 4);
    fprintf(stdout, ANSI_ESCAPE_COLOR_RED "%s" ANSI_ESCAPE_COLOR_DEFAULT " -> Response\n", RESPONSE_CODE_403);
    
}

int main(int argc, char **argv)
{
    mkdir("data", 0777);
    chdir("frontend");
    Request request;

    content_dynamic_dek_text.identify = "dek_text_content";
    content_dynamic_dek_text.value = "";
    content_dynamic_dek_text.func = NULL;

    content_dynamic_dek_open.identify = "dek_open_content";
    content_dynamic_dek_open.value = " ";
    content_dynamic_dek_open.func = NULL;

    content_dynamic_dek_file_name.identify = "content_file_name";
    content_dynamic_dek_file_name.value = "";
    content_dynamic_dek_file_name.func = NULL;

    ldek_data_file_path_current = NULL;

    int8_t server_socket = server_init(8080);

    struct sockaddr address_client;
    uint32_t address_client_length = sizeof(address_client);

    for (;;)
    {
        int8_t socket_client = accept(server_socket, &address_client, &address_client_length);
        uint16_t bytes_received = read(socket_client, request.data, REQUEST_SIZE_MAX);
        request.data[bytes_received] = 0;
        if (bytes_received < 1)
            fprintf(stdout, "Unexpected disconnection\n");
        else if (request_parse(&request))
        {
            request_send(socket_client, &request);
        }
        else
            fprintf(stdout, "Request was not valid\n");

        close(socket_client);
    }

    return 0;
}
