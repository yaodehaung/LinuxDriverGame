#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <netdb.h>

#define HTTP_PORT 8080
#define MQTT_HOST "test.mosquitto.org"
#define MQTT_PORT 1883
#define MAX_EVENTS 10
#define BUFFER_SIZE 1024

int set_nonblocking(int sock) {
    int flags = fcntl(sock, F_GETFL, 0);
    return fcntl(sock, F_SETFL, flags | O_NONBLOCK);
}

int init_http_server() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr = {0};

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(HTTP_PORT);

    bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_fd, 5);
    set_nonblocking(server_fd);
    return server_fd;
}

void handle_http_request(int client_fd) {
    char buffer[BUFFER_SIZE] = {0};
    read(client_fd, buffer, sizeof(buffer));
    printf("Received HTTP Request:\n%s\n", buffer);

    const char* response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
    send(client_fd, response, strlen(response), 0);
    close(client_fd);
}

int connect_mqtt() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {0};
    struct hostent *host = gethostbyname(MQTT_HOST);

    if (!host) {
        perror("gethostbyname failed");
        return -1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(MQTT_PORT);
    memcpy(&addr.sin_addr, host->h_addr, host->h_length);

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("MQTT connect failed");
        close(sock);
        return -1;
    }

    set_nonblocking(sock);

    // MQTT CONNECT
    char connect_packet[] = {
        0x10, 0x16, 0x00, 0x04, 'M', 'Q', 'T', 'T', 
        0x04, 0x02, 0x00, 0x3C, 0x00, 0x08, 'M', 'Q', 'T', 'T', '_', 'C', 'L', 'I'
    };
    send(sock, connect_packet, sizeof(connect_packet), 0);

    // MQTT SUBSCRIBE
    char subscribe_packet[] = {
        0x82, 0x0C, 0x00, 0x01, 0x00, 0x0A, 't', 'e', 's', 't', '/', 't', 'o', 'p', 'i', 'c', 0x00
    };
    send(sock, subscribe_packet, sizeof(subscribe_packet), 0);

    return sock;
}

void handle_mqtt_message(int sock) {
    char buffer[BUFFER_SIZE] = {0};
    int len = read(sock, buffer, sizeof(buffer));

    if (len > 0) {
        if ((buffer[0] & 0xF0) == 0x30) { // MQTT PUBLISH
            int topic_len = (buffer[2] << 8) | buffer[3];
            char topic[topic_len + 1];
            memcpy(topic, &buffer[4], topic_len);
            topic[topic_len] = '\0';

            int payload_offset = 4 + topic_len;
            int payload_len = len - payload_offset;
            char payload[payload_len + 1];
            memcpy(payload, &buffer[payload_offset], payload_len);
            payload[payload_len] = '\0';

            printf("MQTT Message: Topic=%s, Payload=%s\n", topic, payload);
        }
    }
}

int main() {
    struct epoll_event ev, events[MAX_EVENTS];
    int epoll_fd, http_server_fd, mqtt_fd, nfds;

    epoll_fd = epoll_create1(0);

    http_server_fd = init_http_server();
    ev.events = EPOLLIN;
    ev.data.fd = http_server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, http_server_fd, &ev);

    mqtt_fd = connect_mqtt();
    if (mqtt_fd < 0) {
        fprintf(stderr, "MQTT connection failed\n");
        exit(EXIT_FAILURE);
    }
    ev.data.fd = mqtt_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, mqtt_fd, &ev);

    while (1) {
        nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int i = 0; i < nfds; i++) {
            int fd = events[i].data.fd;

            if (fd == http_server_fd) {
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int client_fd = accept(http_server_fd, (struct sockaddr*)&client_addr, &client_len);
                set_nonblocking(client_fd);
                handle_http_request(client_fd);
            } else if (fd == mqtt_fd) {
                handle_mqtt_message(mqtt_fd);
            }
        }
    }

    close(http_server_fd);
    close(mqtt_fd);
    close(epoll_fd);
    return 0;
}
