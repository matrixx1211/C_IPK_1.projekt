#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define N 1
#define BUFFER_SIZE 1024
#define RESPONSE_SIZE 1024
#define RESPONSE_TEXT_SIZE 512
#define PATH_SIZE 10
#define LOAD_ITEMS 10
#define TIMES 5
#define SLEEP 1

void param_test(int argc, char const *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Expected count of params -> 1, you entered -> %d!\n", argc - 1);
        exit(EXIT_FAILURE);
    }
}

double load(char *response, char *response_text, int length)
{

    int c, i = 0, load[10] = {0}, j = 0,
           idle, non_idle, total, times = TIMES,
           total_diff, idle_diff, prev_total = 0, prev_idle = 0, prev_non_idle = 0;
    double total_load;
    // 0: user,     1: nice,    2: system,
    // 3: idle,     4: iowait,  5: irq,
    // 6: softirq,  7: steal,   8: guest, 9: guest_nice;
    char temp[20] = {0};

    while (times > 0)
    {
        FILE *load_info = fopen("/proc/stat", "rb");
        while ((c = fgetc(load_info)) != '\n')
        {
            temp[i] = c;
            i++;
            if (c == ' ')
            {
                temp[i] = '\0';
                if (!strstr(temp, "cpu"))
                {
                    load[j] = atoi(temp);
                    j++;
                }
                i = 0;
            }
        }
        fclose(load_info);

        // Idle = idle + iowait
        idle = load[3] + load[4];
        // NonIdle = user + nice + system + irq + softirq + steal
        non_idle = load[0] + load[1] + load[2] + load[5] + load[6] + load[7];
        // Total = Idle + NonIdle
        total = idle + non_idle;

        total_diff = total - prev_total;
        idle_diff = idle - prev_idle;

        prev_idle = idle;
        prev_total = total;

        fprintf(stderr, "----------\n");
        fprintf(stderr, "%d <- idle\n", idle);
        fprintf(stderr, "%d <- non_idle\n", non_idle);
        fprintf(stderr, "%d <- total\n", total);
        fprintf(stderr, "%d <- prev_idle\n", prev_idle);
        fprintf(stderr, "%d <- prev_total\n", prev_total);
        fprintf(stderr, "%d <- idle_diff\n", idle_diff);
        fprintf(stderr, "%d <- total_diff\n", total_diff);

        // total_load = (total_diff - idle_diff) / total_diff;
        //čekání před znovu vypočítáním
        times--;
        sleep(SLEEP);
    }
    total_load = (total_diff - idle_diff) / total_diff;

    // idleFraction = 100 - (idle-lastIdle)*100.0/(sum-lastSum);
    // snprintf(response_text, RESPONSE_TEXT_SIZE, "%d", 100 - (idle_diff)*100 / (total_diff));
}

void get_response_from_path(char *buffer, char *response)
{
    char response_text[RESPONSE_TEXT_SIZE] = {0};

    int length = snprintf(response, RESPONSE_SIZE, "HTTP/1.1 ");
    if (strstr(buffer, " /hostname "))
    {
        memset(response_text, 0, RESPONSE_TEXT_SIZE);
        gethostname(response_text, RESPONSE_TEXT_SIZE);
        snprintf(response + length, RESPONSE_SIZE - length, "200 OK\nContent-Type: text/plain; charset=utf-8\nContent-Length: %ld\n\n%s\r\n", strlen(response_text), response_text);
    }
    else if (strstr(buffer, " /cpu-name "))
    {
        FILE *cpuinfo = popen("lscpu | grep \"Model name:\" | sed -r 's/Model name:\\s{1,}//g'", "r");
        fgets(response_text, RESPONSE_TEXT_SIZE, cpuinfo);
        fclose(cpuinfo);
        snprintf(response + length, RESPONSE_SIZE - length, "200 OK\nContent-Type: text/plain; charset=utf-8\nContent-Length: %ld\n\n%s\r\n", strlen(response_text), response_text);
    }
    else if (strstr(buffer, " /load "))
    {
        
        int c, i = 0, load[10] = {0}, j = 0,
               idle, non_idle, total, times = TIMES,
               total_diff, idle_diff, prev_total = 0, prev_idle = 0, prev_non_idle = 0;
        double total_load;
        // 0: user,     1: nice,    2: system,
        // 3: idle,     4: iowait,  5: irq,
        // 6: softirq,  7: steal,   8: guest, 9: guest_nice;
        char temp[20] = {0};

        while (times > 0)
        {
            FILE *cpuinfo = fopen("/proc/stat", "rb");
            while ((c = fgetc(cpuinfo)) != '\n')
            {
                temp[i] = c;
                i++;
                if (c == ' ')
                {
                    temp[i] = '\0';
                    if (!strstr(temp, "cpu"))
                    {
                        load[j] = atoi(temp);
                        j++;
                    }
                    i = 0;
                }
            }
            fclose(cpuinfo);

            // Idle = idle + iowait
            idle = load[3] + load[4];
            // NonIdle = user + nice + system + irq + softirq + steal
            non_idle = load[0] + load[1] + load[2] + load[5] + load[6] + load[7];
            // Total = Idle + NonIdle
            total = idle + non_idle;



            total_diff = total - prev_total;
            idle_diff = idle - prev_idle;

            prev_idle = idle;
            prev_total = total;

            fprintf(stderr, "%d <- idle\n", load[3]);
            fprintf(stderr, "----------\n");
            fprintf(stderr, "%d <- idle\n", idle);
            fprintf(stderr, "%d <- non_idle\n", non_idle);
            fprintf(stderr, "%d <- total\n", total);
            fprintf(stderr, "%d <- prev_idle\n", prev_idle);
            fprintf(stderr, "%d <- prev_total\n", prev_total);
            fprintf(stderr, "%d <- idle_diff\n", idle_diff);
            fprintf(stderr, "%d <- total_diff\n", total_diff);
            
            //total_load = (total_diff - idle_diff) / total_diff;
            //čekání před znovu vypočítáním
            times--;
            sleep(SLEEP);
        }
        // idleFraction = 100 - (idle-lastIdle)*100.0/(sum-lastSum);
        // snprintf(response_text, RESPONSE_TEXT_SIZE, "%d", 100 - (idle_diff)*100 / (total_diff));
        snprintf(response + length, RESPONSE_SIZE - length,
                 "200 OK\nContent-Type: text/plain; charset=utf-8\nContent-Length: %ld\n\n%s\r\n",
                 strlen(response_text),
                 response_text);
    }
    else
    {
        strncpy(response_text, "error", RESPONSE_TEXT_SIZE - 1);
        snprintf(response + length, RESPONSE_SIZE - length, "404 Not Found\nContent-Type: text/plain\nContent-Length: %ld\n\n%s\r\n", strlen(response_text), response_text);
    }
}

int main(int argc, char const *argv[])
{
    // Otestování parametetrů
    param_test(argc, argv);

    // Proměnné pro socket
    struct sockaddr_in address;
    int server_fd, opt = 1, valread, addrlen = sizeof(address), new_socket;
    char buffer[BUFFER_SIZE] = {0};
    // Pomocné proměnné
    char response[RESPONSE_SIZE] = {0};

    // Vytvoření file descriptoru
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Nastavení socketu
    if ((setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) < 0)
    {
        perror("set socket opt:");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(atoi(argv[argc - 1]));

    // Připojení socketu k portu
    if ((bind(server_fd, (struct sockaddr *)&address, sizeof(address))) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Počet připojení omezen na N
    if (listen(server_fd, N) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Dokud neukončím
    while (1)
    {
        // Přijímání dat
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // Uložím si do bufferu z new_socketu message
        valread = read(new_socket, buffer, BUFFER_SIZE);

        // Podle pathu /... vytvořím odpověď
        get_response_from_path(buffer, response);

        // Odešlu odpověď
        send(new_socket, response, strlen(response), 0);
    }

    return EXIT_SUCCESS;
}