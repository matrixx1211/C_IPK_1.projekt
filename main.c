/* AUTOR: Marek Bitomský - xbitom00 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define N 10                   // Počet připojení
#define REQUEST_SIZE 128       // Maximální délka požadavku
#define RESPONSE_SIZE 1024     // Maximální délka odpovědi
#define RESPONSE_TEXT_SIZE 512 // Maximální délka vlastního textu odpovědi
#define TIMES 5                // Počet kolikrát se provede load
#define SLEEP 20000            // 20ms

/* Testování parametru, jestli uživatel zadal port */
void param_test(int argc, char const *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Expected count of params -> 1, you entered -> %d!\n", argc - 1);
        exit(EXIT_FAILURE);
    }
}

/* Podle příchozího požadavku vytvoří odpověď */
void get_response_from_path(char *buffer, char *response)
{
    char response_text[RESPONSE_TEXT_SIZE] = {0};
    int length = snprintf(response, RESPONSE_SIZE, "HTTP/1.1 ");

    /* V případě že příjde požadavek */
    //... na doménové jméno
    if (strstr(buffer, " /hostname "))
    {
        memset(response_text, 0, RESPONSE_TEXT_SIZE);
        gethostname(response_text, RESPONSE_TEXT_SIZE);
        snprintf(response_text + strlen(response_text), RESPONSE_TEXT_SIZE, "\n");
        snprintf(response + length, RESPONSE_SIZE - length,
                 "200 OK\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: %ld\r\n\r\n%s\r\n",
                 strlen(response_text), response_text);
    }
    //... na název procesoru
    else if (strstr(buffer, " /cpu-name "))
    {
        FILE *cpuinfo = popen("lscpu | grep \"Model name:\" | sed -r 's/Model name:\\s{1,}//g'", "r");
        fgets(response_text, RESPONSE_TEXT_SIZE, cpuinfo);
        fclose(cpuinfo);
        if (strlen(response_text) == 0)
            strncpy(response_text, "Please set locale!\nFor example you can use this command:\n\texport LC_ALL=C\n", RESPONSE_TEXT_SIZE);
        snprintf(response + length, RESPONSE_SIZE - length,
                 "200 OK\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: %ld\r\n\r\n%s\r\n",
                 strlen(response_text), response_text);
    }
    //... na zatížení CPU
    else if (strstr(buffer, " /load "))
    {
        int times = TIMES, counter = 0;
        char *token;
        long int idle = 0, non_idle = 0, total, prev_idle = 0, prev_total = 0;
        double idle_diff = 0.0, total_diff = 0.0;
        /* Opakuji výpočty times-krát */
        while (times > 0)
        {
            /* Otevření souboru proc/stat pro hodnoty loadu | oříznuté na jeden řádek | oříznutí o text cpu */
            FILE *stat = popen("cat /proc/stat | head -1 | sed -r 's/cpu\\s{1,}//g'", "r");
            fgets(response_text, RESPONSE_TEXT_SIZE, stat);
            fclose(stat);

            token = strtok(response_text, " ");
            counter = 0;
            idle = 0;
            non_idle = 0;
            /* Načítám tokeny */
            while (token != NULL)
            {
                if (token != NULL)
                {
                    if (counter == 3 || counter == 4)
                        idle += atoi(token);
                    else
                        non_idle += atoi(token);
                    counter++;
                }
                token = strtok(NULL, " ");
            }
            /* Počítám load */
            total = idle + non_idle;

            total_diff = total - prev_total;
            idle_diff = idle - prev_idle;

            prev_idle = idle;
            prev_total = total;

            /* Uspím na dobu SLEEP v mikrosekundách pro lepší výsledky */
            times--;
            usleep(SLEEP);
        }

        snprintf(response_text, RESPONSE_TEXT_SIZE, "%.0f%%\n", ((total_diff - idle_diff) / total_diff) * 100);
        snprintf(response + length, RESPONSE_SIZE - length,
                 "200 OK\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: %ld\r\n\r\n%s\r\n",
                 strlen(response_text), response_text);
    }
    //... na nedefinovanou věc
    else
    {
        strncpy(response_text, "error", RESPONSE_TEXT_SIZE - 1);
        snprintf(response + length, RESPONSE_SIZE - length, "404 Not Found\nContent-Type: text/plain\nContent-Length: %ld\n\n%s\r\n", strlen(response_text), response_text);
    }
}

int main(int argc, char const *argv[])
{
    param_test(argc, argv);

    // Proměnné pro socket
    struct sockaddr_in address;
    int server_fd, opt = 1, valread, addrlen = sizeof(address), new_socket;
    char buffer[REQUEST_SIZE] = {0};
    // Proměnná pro odpověď
    char response[RESPONSE_SIZE] = {0};

    // Vytvoření file descriptoru
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Nastavení socketu
    if ((setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) < 0)
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

    // Dokud neukončím přes CTRL + C nebo kill PID
    while (1)
    {
        // Přijímání dat
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // Uložím si do bufferu požadavek
        valread = read(new_socket, buffer, REQUEST_SIZE);

        get_response_from_path(buffer, response);

        // Odešlu odpověď
        send(new_socket, response, strlen(response), 0);
    }

    return EXIT_SUCCESS;
}