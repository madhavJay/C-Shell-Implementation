#include "headers.h"
void fetchManPage(const char *command_name)
{
    struct hostent *he;
    if ((he = gethostbyname("man.he.net")) == NULL)
    {
        perror("gethostbyname");
        exit(1);
    }

    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(80); // HTTP port
    server_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(server_addr.sin_zero), 0, 8);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
    {
        perror("connect");
        close(sockfd);
        exit(1);
    }

    char request[MAX_BUFFER_SIZE];
    snprintf(request, sizeof(request),
             "GET /?topic=%s&section=all HTTP/1.1\r\nHost: man.he.net\r\nConnection: close\r\n\r\n",
             command_name);

    if (send(sockfd, request, strlen(request), 0) == -1)
    {
        perror("send");
        close(sockfd);
        exit(1);
    }

    // Step 5: Receive and parse the response
    char response[1000000];
    int bytes_received;
    char *part_name_start, *part_center_end;

    // Read response in chunks
    while ((bytes_received = recv(sockfd, response, sizeof(response) - 1, 0)) > 0)
    {
        response[bytes_received] = '\0'; // Null-terminate the response

        // Step 6: Extract "NAME" section
        char *dup_response = strdup(response);
        part_name_start = strstr(dup_response, "NAME");
        part_name_start++;
        part_name_start = strstr(part_name_start, "NAME");
        if (part_name_start)
        {
            part_center_end = strstr(part_name_start, "<center>");
            if (part_center_end)
            {
                *part_center_end = '\0';
            }
            printf("%s", part_name_start); // Print the relevant part
        }
        else
        {
            printf("%s", dup_response);
        }
        free(dup_response); // Free allocated memory
    }

    if (bytes_received == -1)
    {
        perror("recv");
    }

    close(sockfd);
}

void iman_main(char *command_to_fetch)
{
    if (command_to_fetch == NULL || strlen(command_to_fetch) == 0)
    {
        fprintf(stderr, "Error: No command specified.\n");
        return;
    }
    fetchManPage(command_to_fetch);
}