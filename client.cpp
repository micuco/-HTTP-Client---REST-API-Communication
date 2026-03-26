#include <iostream>
#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include <string>
#include "helpers.hpp"
#include "requests.hpp"
#include "json.hpp"
#define PORT 8081
using json = nlohmann::json;

int get_http_status_code(const char *response) {
    const char *status_line = strstr(response, "HTTP/");
    if (status_line == NULL) {
        return -1; // nu s-a gasit linia de status
    }

    const char *space = strchr(status_line, ' ');
    // caut primul spatiu dupa "HTTP/1.1"
    if (space == NULL) {
        return -1;
    }

    int code = atoi(space + 1);
    // extrag codul ca string
    return code;
}

// Functia cookie-func extrage cookie-urile din raspunsul http
char **cookie_func(const char *aux) {
    char *start = (char *)strstr(aux, "Set-Cookie:");  // Gasesc inceputul header-ului
    char **cookie_array = (char **)malloc(sizeof(char *));
    start = start + strlen("Set-Cookie: ");  // Avansez pointerul dupa Set-Cookie
    char *end = (char *)strchr(start, ';');  // Gaseste sfarsitul cookie-ului
    size_t length = end - start;
    // Calculez lungimea cookie-ului, apoi aloc memorie pentru cookie
    cookie_array[0] = (char *)malloc(length + 1);
    strncpy(cookie_array[0], start, length);  // Copiez cookie-ul in array
    cookie_array[0][length] = '\0';
    return cookie_array;
}

// Functia token_func extrage token-ul din raspunsul http
char *token_func(const char *aux) {
    json data = json::parse(aux); 
    // Parsez raspunsul json, apoi extrag token-ul
    std::string token = data["token"];
    char *token_cstr = (char *)malloc(token.length() + 1);  
    // Aloc memorie pentru token ca string si il copiez
    strcpy(token_cstr, token.c_str());
    return token_cstr;
}

int main(int argc, char *argv[])
{
    const char *ADDR_IP  = "63.32.125.183", *APP_JSON = "application/json";
    // Am retinut adresa ip a serverului si tipul de continut
    int sockfd;
    char **cookie = NULL, *token = NULL, command[30], *pack;
    bool admin = false;
    
    while (true) {
        fgets(command, 30, stdin);  // Citesc comanda de la utilizator
        command[strcspn(command, "\n")] = 0;
        if(!strcmp(command, "login_admin")) {  // Verific daca e comanda de autentificare a adminului
            char username[30], password[30], *packet[1], *aux;
            json data;
            if (cookie) {
                printf("ERROR: Reautentificare\n");
            } else {
                sockfd = open_connection((char *)ADDR_IP, PORT, AF_INET, SOCK_STREAM, 0);  // Deschid o conexiune la server
                printf("username="); fgets(username, 30, stdin);  // Citesc username
                username[strcspn(username, "\n")] = 0;
                printf("password="); fgets(password, 30, stdin);  // Citesc parola
                password[strcspn(password, "\n")] = 0;
                data["username"] = username;  // Adaug username-ul si parola in json
                data["password"] = password;
                std::string data_str = data.dump();  // Convertesc json-ul in string
                packet[0] = (char *)malloc(data_str.size());  // Aloc memorie pentru pachet
                strcpy(packet[0], data_str.c_str()); 
                pack = compute_post_request((char *)ADDR_IP, (char *)"/api/v1/tema/admin/login", (char *)APP_JSON, packet, 1, NULL, 0, NULL);
                // Creez cererea
                send_to_server(sockfd, pack);  // Trimit cererea la server
                aux = receive_from_server(sockfd);
                int status = get_http_status_code(aux);
                if (status >= 200 && status < 300) {
                    // Daca am un raspuns pozitiv afisez mesajul
                    printf("SUCCESS: Admin autentificat cu succes\n");
                    // Setez adminul pe true pentru a stii ca sunt autentificat ca admin
                    admin = true;
                    cookie = cookie_func(aux);
                } else {
                    printf("ERROR: incorect\n");
                }
                close(sockfd);
            }
        }
        if(!strcmp(command, "add_user")) {  // Verific daca e comanda adaugare utilizatori
            char username[30], password[30], *packet[1];
            json data;
            if (admin == true) {
                // Verific daca am drept de admin
                sockfd = open_connection((char *)ADDR_IP, PORT, AF_INET, SOCK_STREAM, 0);  
                // Deschid o conexiune la server si citesc username-ul
                printf("username="); fgets(username, 30, stdin);
                username[strcspn(username, "\n")] = 0;
                printf("password="); fgets(password, 30, stdin); // Citesc parola
                password[strcspn(password, "\n")] = 0;
                if (username[0] == '\0' || password[0] == '\0') {
                    // Daca nu se da un username sau o parola afisez eroare
                    printf("ERROR: info incomplet\n");
                } else {
                    data["username"] = username;  // Adaug username-ul si parola in json
                    data["password"] = password;
                    std::string data_str = data.dump();  // Convertesc json-ul in string
                    packet[0] = (char *)malloc(data_str.size());  // Aloc memorie pentru pachet
                    strcpy(packet[0], data_str.c_str()); 
                    pack = compute_post_request((char *)ADDR_IP, (char *)"/api/v1/tema/admin/users", (char *)APP_JSON, packet, 1, cookie, 1, NULL);
                    // Creez cererea
                    send_to_server(sockfd, pack);  // Trimit cererea la server
                    char *aux = receive_from_server(sockfd);
                    int status = get_http_status_code(aux);
                    if (status >= 200 && status < 300) {
                        // Daca am un raspuns pozitiv afisez mesajul
                        printf("SUCCESS: Utilizator adaugat cu succes\n");
                    } else {
                        printf("ERROR\n");
                    }
                    close(sockfd);
                }
            } else {
                printf("ERROR: lipsa admin\n");
            }
        }
        if (!strcmp(command, "get_users")) {  // Verific daca e comanda pentru obtinerea listei de utilizatori
            if (admin) {
                // Verific daca am drept de admin
                char *aux;
                sockfd = open_connection((char *)ADDR_IP, PORT, AF_INET, SOCK_STREAM, 0);  // deschid o conexiune la server
                pack = compute_get_request((char *)ADDR_IP, (char *)"/api/v1/tema/admin/users", NULL, cookie, 1, NULL);  // Creez cererea
                send_to_server(sockfd, pack);  // Trimit cererea la server
                aux = basic_extract_json_response(receive_from_server(sockfd));  // Primesc raspunsul de la server
                if (aux) {
                    // In caz pozitiv afisez mesajul
                    printf("SUCCESS: Lista utilizatorilor\n");
                    json j = json::parse(aux);
                    std::stringstream formatted_output;
                    // Afisez sub formatul cerut indexul, usernameul si parola pentru fiecare utilizator
                    int index = 1;
                    for (const auto &user : j["users"])
                    {
                        std::string username = user["username"].get<std::string>();
                        std::string password = user["password"].get<std::string>();
                        formatted_output << "#" << index << " " << username << ":" << password << "\n";
                        ++index;
                    }

                    std::string aux = formatted_output.str();
                    printf("%s\n", aux.c_str());
                }
                close(sockfd);
            } else {
                printf("ERROR\n");
            }
        }
        if (!strcmp(command, "delete_user")) {  // Verific daca e comanda pentru stergerea unui utilizator
            if (admin) {
                // Verific daca am drept de admin
                sockfd = open_connection((char *)ADDR_IP, PORT, AF_INET, SOCK_STREAM, 0);  // Deschid o conexiune la server
                char username[30];
                printf("username="); 
                if (scanf("%s", username) == 0) {  // Citesc usernameul si daca e corect
                    printf("ERROR: Username invalid\n");
                } else {
                    std::string ruta_acces = (char *)"/api/v1/tema/admin/users/";
                    ruta_acces += username;
                    // Creez ruta de acces pentru cerere
                    pack = compute_delete_request((char *)ADDR_IP, (char *)ruta_acces.c_str(), NULL, cookie, 1, NULL);
                    // Creez cererea
                    send_to_server(sockfd, pack);  // Trimit cererea la server
                    char *aux = receive_from_server(sockfd);
                    int status = get_http_status_code(aux);
                    if (status >= 200 && status < 300) {
                        // In caz afirmativ afisez mesajul
                        printf("SUCCESS: Utilizator sters\n");
                    } else {
                        printf("ERROR\n");
                    }
                    close(sockfd);
                }
            } else {
                printf("ERROR\n");
            }
        }
        if(!strcmp(command, "login")) {  // Verific daca e comanda de autentificare
            char admin_username[30], username[30], password[30], *packet[1], *aux;
            json data;
            if (cookie) {
                printf("ERROR: Reautentificare\n");
            } else {
                sockfd = open_connection((char *)ADDR_IP, PORT, AF_INET, SOCK_STREAM, 0);  // Deschid o conexiune la server
                printf("admin_username="); fgets(admin_username, 30, stdin);  // Citesc usernameul adminului
                admin_username[strcspn(admin_username, "\n")] = 0;
                printf("username="); fgets(username, 30, stdin);  // Citesc username
                username[strcspn(username, "\n")] = 0;
                printf("password="); fgets(password, 30, stdin);  // Citesc parola
                password[strcspn(password, "\n")] = 0;
                data["admin_username"] = admin_username;  // Adaug username-ul adminului, username-ul si parola in json
                data["username"] = username;
                data["password"] = password;
                std::string data_str = data.dump();  // Convertesc json-ul in string
                packet[0] = (char *)malloc(data_str.size());  // Aloc memorie pentru pachet
                strcpy(packet[0], data_str.c_str()); 
                pack = compute_post_request((char *)ADDR_IP, (char *)"/api/v1/tema/user/login", (char *)APP_JSON, packet, 1, NULL, 0, NULL);
                // Creez cererea
                send_to_server(sockfd, pack);  // Trimit cererea la server
                aux = receive_from_server(sockfd);
                int status = get_http_status_code(aux);
                if (status >= 200 && status < 300) {
                    // In caz afirmativ afisez mesajul
                    printf("SUCCESS: Autentificare reusita\n");
                    cookie = cookie_func(aux);
                } else {
                    printf("ERROR\n");
                }
                close(sockfd);
            }
        }
        if (!strcmp(command, "get_access")) {  // Verific daca e comanda pentru acces
            if (cookie && !admin) {
                sockfd = open_connection((char *)ADDR_IP, PORT, AF_INET, SOCK_STREAM, 0);  // Deschid o conexiune la server
                pack = compute_get_request((char *)ADDR_IP, (char *)"/api/v1/tema/library/access", NULL, cookie, 1, token);  // Creez cererea
                send_to_server(sockfd, pack);  // Trimit cererea la server
                token = token_func(basic_extract_json_response(receive_from_server(sockfd)));  // Primesc token-ul
                printf("SUCCESS: Token JWT primit\n");  // Afisez mesaj ca am primit tokenul
                close(sockfd);
            } else {
                printf("ERROR\n");
            }
        }
        if (!strcmp(command, "get_movies")) {  // Verific daca e comanda pentru obtinerea listei de filme
            if (token) {
                char *aux;
                sockfd = open_connection((char *)ADDR_IP, PORT, AF_INET, SOCK_STREAM, 0);  // deschid o conexiune la server
                pack = compute_get_request((char *)ADDR_IP, (char *)"/api/v1/tema/library/movies", NULL, cookie, 1, token);  // Creez cererea
                send_to_server(sockfd, pack);  // Trimit cererea la server
                aux = basic_extract_json_response(receive_from_server(sockfd));  // Primesc raspunsul de la server
                if (aux) {
                    // In caz afirmativ afisez mesajul
                    printf("SUCCESS: Lista filmelor\n");
                    json j = json::parse(aux);
                    std::stringstream formatted_output;
                    // Pentru fiecare film afisez id ul si titlul sub formatul dorit
                    for (const auto &movie : j["movies"])
                    {
                        int id = movie["id"].get<int>();
                        std::string title = movie["title"].get<std::string>();
                        formatted_output << "#" << id << " " << title << "\n";
                    }

                    std::string aux = formatted_output.str();
                    printf("%s\n", aux.c_str());
                }
                close(sockfd);
            } else {
                printf("ERROR\n");
            }
        }
        if (!strcmp(command, "get_movie")) {  // Verific daca e comanda pentru obtinerea unui film specific
            if (token) {
                char *aux;
                int id;
                printf("id=");
                if(scanf("%d", &id) == 0) {  // Citesc id-ul filmului
                    printf("ERROR: Input gresit\n");  // Afisez eroare daca inputul este gresit
                } else {
                    sockfd = open_connection((char *)ADDR_IP, PORT, AF_INET, SOCK_STREAM, 0);  // Deschid o conexiune la server
                    std::string ruta_acces = (char *)"/api/v1/tema/library/movies/" + std::to_string(id);  // Creez ruta de acces pentru cerere
                    pack = compute_get_request((char *)ADDR_IP, (char *)ruta_acces.c_str(), NULL, cookie, 1, token);  // Creez cererea
                    send_to_server(sockfd, pack);  // Trimit cererea la server
                    aux = receive_from_server(sockfd);  // Primesc raspunsul de la server
                    char *response = basic_extract_json_response(aux);
                    int status = get_http_status_code(aux);
                    if (status >= 200 && status < 300) {
                        // In caz afirmativ afisez mesajul
                        printf("SUCCESS: Detalii film\n");
                        json j = json::parse(response);
                        std::stringstream formatted_output;
                        // Afisez titlul, anul, descrierea si ratingul sub formatul cerut
                        std::string title = j["title"].get<std::string>();
                        formatted_output << "title: " << title << "\n";

                        int year = j["year"].get<int>();
                        formatted_output << "year: " << year << "\n";

                        std::string description = j["description"].get<std::string>();
                        formatted_output << "description: " << description << "\n";

                        std::string rating_str = j["rating"].get<std::string>();
                        float rating = std::stof(rating_str);
                        formatted_output << "rating: " << rating << "\n";

                        std::string response = formatted_output.str();
                        printf("%s\n", response.c_str());
                    } else {
                        printf("ERROR: ceva\n");
                    }
                    close(sockfd);
                }
            } else {
                printf("ERROR\n");
            }
        }
        if(!strcmp(command, "add_movie")) {  // Verific daca e comanda pentru adaugarea unui film
            if(token) {
                char info[100], *packet[1];
                int info_int;
                float info_float;
                json data;
                sockfd = open_connection((char *)ADDR_IP, PORT, AF_INET, SOCK_STREAM, 0);  // Deschid o conexiune la server
                printf("title="); fgets(info, 100, stdin);  // Citesc titlul
                info[strcspn(info, "\n")] = 0;
                data["title"] = info;  // Adaug titlul in json
                printf("year=");
                if(scanf("%d", &info_int) == 0) {  // Citesc anul si verific daca e ok
                    printf("ERROR: Input gresit\n");
                } else {
                    getchar();  // Curata buffer-ul de newline
                    data["year"] = info_int;  // Adaug anul in json
                    info_int = 0;
                    printf("description="); fgets(info, 100, stdin);  // Citesc descrierea
                    info[strcspn(info, "\n")] = 0;
                    data["description"] = info;  // Adaug descrierea in json
                    printf("rating=");
                    if(scanf("%f", &info_float) == 0) {  // Citesc ratingul
                        printf("ERROR: Input gresit\n");
                    } else {
                        getchar();  // Curata buffer-ul de newline
                        data["rating"] = info_float;  // Adaug ratingul in json
                        std::string data_str = data.dump();  // Convertesc json-ul in string
                        packet[0] = (char *)malloc(data_str.size());
                        strcpy(packet[0], data_str.c_str()); 
                        pack = compute_post_request((char *)ADDR_IP, (char *)"/api/v1/tema/library/movies", (char *)APP_JSON, packet, 1, cookie, 1, token);
                        // Creez cererea
                        send_to_server(sockfd, pack);  // Trimit cererea la server
                        char *aux = receive_from_server(sockfd);
                        int status = get_http_status_code(aux);
                        if (status >= 200 && status < 300) {
                            // In caz afirmativ afisez mesajul
                            printf("SUCCESS: Film adaugat\n");
                            cookie = cookie_func(aux);
                        } else {
                            printf("ERROR\n");
                        }
                        close(sockfd);
                    }
                }
            } else {
                printf("ERROR\n");
            }
        }
        if(!strcmp(command, "delete_movie")) {  // Verific daca e comanda pentru stergerea unui film
            if(token) {
                sockfd = open_connection((char *)ADDR_IP, PORT, AF_INET, SOCK_STREAM, 0);  // Deschid o conexiune la server
                int id;
                printf("id="); 
                if(scanf("%d", &id) == 0) {  // Citesc id-ul filmului si daca e corect
                    printf("ERROR: Input gresit\n");
                } else {
                    std::string ruta_acces = (char *)"/api/v1/tema/library/movies/" + std::to_string(id);
                    // Creez ruta de acces pentru cerere
                    pack = compute_delete_request((char *)ADDR_IP, (char *)ruta_acces.c_str(), NULL, cookie, 1, token);
                    // Creez cererea
                    send_to_server(sockfd, pack);  // Trimit cererea la server
                    char *aux = receive_from_server(sockfd);
                    int status = get_http_status_code(aux);
                    if (status >= 200 && status < 300) {
                        // In caz afirmativ afisez mesajul
                        printf("SUCCESS: Film sters cu succes\n");
                    } else {
                        printf("ERROR\n");
                    }
                    close(sockfd);
                }
            } else {
                printf("ERROR\n");
            }
        }
        if(!strcmp(command, "update_movie")) {  // Verific daca e comanda pentru updatarea unui film
            if(token) {
                char info[100], *packet[1];
                int info_int, id;
                float info_float;
                json data;
                sockfd = open_connection((char *)ADDR_IP, PORT, AF_INET, SOCK_STREAM, 0);  // Deschid o conexiune la server
                printf("title="); fgets(info, 100, stdin);  // Citesc titlul
                info[strcspn(info, "\n")] = 0;
                data["title"] = info;  // Adaug titlul in json
                printf("year=");
                if(scanf("%d", &info_int) == 0) {  // Citesc anul
                    printf("ERROR: Input gresit\n");
                } else {
                    getchar();  // Curata buffer-ul de newline
                    data["year"] = info_int;  // Adaug anul in json
                    info_int = 0;
                    printf("description="); fgets(info, 100, stdin);  // Citesc descrierea
                    info[strcspn(info, "\n")] = 0;
                    data["description"] = info;  // Adaug descrierea in json
                    printf("rating=");
                    if(scanf("%f", &info_float) == 0) {  // Citesc ratingul
                        printf("ERROR: Input gresit\n");
                    } else {
                        getchar();  // Curata buffer-ul de newline
                        data["rating"] = info_float;  // Adaug ratingul in json
                        printf("id=");
                        if(scanf("%d", &id) == 0) {  // Citesc id ul
                            printf("ERROR: Input gresit\n");
                        } else {
                            std::string data_str = data.dump();  // Convertesc json-ul in string
                            packet[0] = (char *)malloc(data_str.size());
                            strcpy(packet[0], data_str.c_str()); 
                            std::string ruta_acces = (char *)"/api/v1/tema/library/movies/" + std::to_string(id);  // Creez ruta de acces pentru cerere
                            pack = compute_put_request((char *)ADDR_IP, (char *)ruta_acces.c_str(), (char *)APP_JSON, packet, 1, cookie, 1, token);
                            // Creez cererea
                            send_to_server(sockfd, pack);  // Trimit cererea la server
                            char *aux = receive_from_server(sockfd);
                            int status = get_http_status_code(aux);
                            if (status >= 200 && status < 300) {
                                // In caz afirmativ afisez mesajul
                                printf("SUCCESS: Film actualizat\n");
                            } else {
                                printf("ERROR\n");
                            }
                            close(sockfd);
                        }
                    }
                }
            } else {
                printf("ERROR\n");
            }
        }
        if (!strcmp(command, "get_collections")) {  // Verific daca e comanda pentru obtinerea colectiilor
            if (token) {
                char *aux;
                sockfd = open_connection((char *)ADDR_IP, PORT, AF_INET, SOCK_STREAM, 0);  // deschid o conexiune la server
                pack = compute_get_request((char *)ADDR_IP, (char *)"/api/v1/tema/library/collections", NULL, cookie, 1, token);  // Creez cererea
                send_to_server(sockfd, pack);  // Trimit cererea la server
                aux = basic_extract_json_response(receive_from_server(sockfd));  // Primesc raspunsul de la server
                if (aux) {
                    // In caz afirmativ afisez mesajul
                    printf("SUCCESS: Lista colectiilor\n");
                    json j = json::parse(aux);
                    std::stringstream formatted_output;
                    // Afisez id ul si titlul sub formatul cerut
                    for (const auto &collection : j["collections"])
                    {
                        int id = collection["id"].get<int>();
                        std::string title = collection["title"].get<std::string>();
                        formatted_output << "#" << id << ": " << title << "\n";
                    }

                    std::string aux = formatted_output.str();
                    printf("%s\n", aux.c_str());
                }
                close(sockfd);
            } else {
                printf("ERROR\n");
            }
        }
        if (!strcmp(command, "get_collection")) {  // Verific daca e comanda pentru obtinerea unei colectii specifice
            if (token) {
                char *aux;
                int id;
                printf("id=");
                if(scanf("%d", &id) == 0) {  // Citesc id-ul colectiei
                    printf("ERROR: Input gresit\n");  // Afisez eroare daca inputul este gresit
                } else {
                    sockfd = open_connection((char *)ADDR_IP, PORT, AF_INET, SOCK_STREAM, 0);  // Deschid o conexiune la server
                    std::string ruta_acces = (char *)"/api/v1/tema/library/collections/" + std::to_string(id);  // Creez ruta de acces pentru cerere
                    pack = compute_get_request((char *)ADDR_IP, (char *)ruta_acces.c_str(), NULL, cookie, 1, token);  // Creez cererea
                    send_to_server(sockfd, pack);  // Trimit cererea la server
                    aux = basic_extract_json_response(receive_from_server(sockfd));  // Primesc raspunsul de la server
                    if (strstr(aux, "error")) {
                        printf("ERROR\n");
                    } else {
                        // In caz afirmativ afisez mesajul
                        printf("SUCCESS: Detalii colectie\n");
                        json j = json::parse(aux);
                        std::stringstream formatted_output;
                        // Afisez titlul, ownerul si filmele in formatul cerut
                        std::string title = j["title"].get<std::string>();
                        formatted_output << "title: " << title << "\n";

                        std::string owner = j["owner"].get<std::string>();
                        formatted_output << "owner: " << owner << "\n";

                        for (const auto &movie : j["movies"])
                        {
                            int id = movie["id"].get<int>();
                            std::string title = movie["title"].get<std::string>();
                            formatted_output << "#" << id << ": " << title << "\n";
                        }

                        std::string aux = formatted_output.str();
                        printf("%s\n", aux.c_str());
                    }
                    close(sockfd);
                }
            } else {
                printf("ERROR\n");
            }
        }
        if (!strcmp(command, "add_collection")) {  // Verific daca e comanda pentru adaugarea unei colectii
            if (token) {
                char info[100], *packet[1];
                int info_int, num_movies;
                float info_float;
                int movie_idx[100];
                json data, data_aux;
                sockfd = open_connection((char *)ADDR_IP, PORT, AF_INET, SOCK_STREAM, 0);  // Deschid o conexiune la server
                printf("title="); fgets(info, 100, stdin);  // Citesc titlul
                info[strcspn(info, "\n")] = 0;
                data["title"] = info;
                printf("num_movies=");
                if (scanf("%d", &num_movies) == 0) {  // Citesc numarul de filme
                    printf("ERROR: Input gresit\n");
                } else {
                    getchar();
                    int x;
                    for (int i = 0; i < num_movies; i++) {
                        printf("movie_id[%d]=", i);
                        if (scanf("%d", &x) == 0) {  // Citesc id ul filmelor
                            printf("ERROR: Input gresit\n");
                        } else {
                            getchar();
                            movie_idx[i] = x;
                        }
                    }
                    std::string data_str = data.dump();  // Convertesc json-ul in string
                    packet[0] = (char *)malloc(data_str.size());
                    strcpy(packet[0], data_str.c_str()); 
                    pack = compute_post_request((char *)ADDR_IP, (char *)"/api/v1/tema/library/collections", (char *)APP_JSON, packet, 1, cookie, 1, token);
                    // Creez cererea
                    send_to_server(sockfd, pack);  // Trimit cererea la server
                    char *aux = receive_from_server(sockfd);
                    char *response = basic_extract_json_response(aux);
                    json j = json::parse(response);
                    int collection_id = j["id"];

                    bool ok = true;

                    for (int i = 0; i < num_movies; i++) {
                        data_aux["id"] = movie_idx[i];
                        std::string data_str = data_aux.dump();  // Convertesc json-ul in string
                        packet[0] = (char *)malloc(data_str.size());
                        strcpy(packet[0], data_str.c_str()); 
                        std::string ruta_acces = (char *)"/api/v1/tema/library/collections/" + std::to_string(collection_id) + (char *)"/movies";
                        pack = compute_post_request((char *)ADDR_IP, (char *)ruta_acces.c_str(), (char *)APP_JSON, packet, 1, cookie, 1, token);
                        // Creez cererea
                        send_to_server(sockfd, pack);  // Trimit cererea la server
                        char *aux = receive_from_server(sockfd);
                        int status = get_http_status_code(aux);
                        if (status <= 200 || status >= 300) {
                           ok = false;
                           break;
                        }
                    }
                    int status = get_http_status_code(aux);
                    if (status >= 200 && status < 300 && ok) {
                        // In caz afirmativ afisez mesajul
                        printf("SUCCESS: Colectie adaugata\n");
                    } else {
                        printf("ERROR\n");
                    }
                    close(sockfd);
                }
            } else {
                printf("ERROR\n");
            }
        }
        if(!strcmp(command, "delete_collection")) {  // Verific daca e comanda pentru stergerea unei colectii
            if(token) {
                sockfd = open_connection((char *)ADDR_IP, PORT, AF_INET, SOCK_STREAM, 0);  // Deschid o conexiune la server
                int id;
                printf("id="); 
                if(scanf("%d", &id) == 0) {  // Citesc id-ul si daca e corect
                    printf("ERROR: Input gresit\n");
                } else {
                    std::string ruta_acces = (char *)"/api/v1/tema/library/collections/" + std::to_string(id);
                    // Creez ruta de acces pentru cerere
                    pack = compute_delete_request((char *)ADDR_IP, (char *)ruta_acces.c_str(), NULL, cookie, 1, token);
                    // Creez cererea
                    send_to_server(sockfd, pack);  // Trimit cererea la server
                    char *aux = receive_from_server(sockfd);
                    int status = get_http_status_code(aux);
                    if (status >= 200 && status < 300) {
                        // In caz afirmativ afisez mesajul
                        printf("SUCCESS: Colectie stearsa\n");
                    } else {
                        printf("ERROR\n");
                    }
                    close(sockfd);
                }
            } else {
                printf("ERROR\n");
            }
        }
        if(!strcmp(command, "add_movie_to_collection")) {
            // Verific daca e comanda pentru adaugarea unui film intr-o colectie
            if(token) {
                char *packet[1];
                int collection_id, movie_id;
                json data;
                sockfd = open_connection((char *)ADDR_IP, PORT, AF_INET, SOCK_STREAM, 0);  // Deschid o conexiune la server
                printf("collection_id=");
                if(scanf("%d", &collection_id) == 0) {  // Citesc id ul colectiei
                    printf("ERROR: Input gresit\n");
                } else {
                    getchar();  // Curata buffer-ul de newline
                    printf("movie_id=");
                    if(scanf("%d", &movie_id) == 0) {  // Citesc id ul filmului
                        printf("ERROR: Input gresit\n");
                    } else {
                        getchar();
                        data["id"] = movie_id;
                        std::string data_str = data.dump();  // Convertesc json-ul in string
                        packet[0] = (char *)malloc(data_str.size());
                        strcpy(packet[0], data_str.c_str()); 
                        std::string ruta_acces = (char *)"/api/v1/tema/library/collections/" + std::to_string(collection_id) + (char *)"/movies";
                        pack = compute_post_request((char *)ADDR_IP, (char *)ruta_acces.c_str(), (char *)APP_JSON, packet, 1, cookie, 1, token);
                        // Creez cererea
                        send_to_server(sockfd, pack);  // Trimit cererea la server
                        char *aux = receive_from_server(sockfd);
                        int status = get_http_status_code(aux);
                        if (status >= 200 && status < 300) {
                            // In caz afirmativ afisez mesajul
                            printf("SUCCESS: Film adaugat in colectie\n");
                        } else {
                            printf("ERROR\n");
                        }
                        close(sockfd);
                    }
                }
            } else {
                printf("ERROR\n");
            }
        }
        if(!strcmp(command, "delete_movie_from_collection")) {
            // Verific daca e comanda pentru stergerea unui film dintr-o colectie
            if(token) {
                sockfd = open_connection((char *)ADDR_IP, PORT, AF_INET, SOCK_STREAM, 0);  // Deschid o conexiune la server
                int collection_id, movie_id;
                printf("collection_id="); 
                if(scanf("%d", &collection_id) == 0) {  // Citesc id-ul colectiei si daca e corect
                    printf("ERROR: Input gresit\n");
                } else {
                    getchar();
                    printf("movie_id="); 
                    if(scanf("%d", &movie_id) == 0) {  // Citesc id-ul filmului si daca e corect
                        printf("ERROR: Input gresit\n");
                    } else {
                        std::string ruta_acces = (char *)"/api/v1/tema/library/collections/" + std::to_string(collection_id) + (char *)"/movies/" + std::to_string(movie_id);
                        // Creez ruta de acces pentru cerere
                        pack = compute_delete_request((char *)ADDR_IP, (char *)ruta_acces.c_str(), NULL, cookie, 1, token);
                        // Creez cererea
                        send_to_server(sockfd, pack);  // Trimit cererea la server
                        char *aux = receive_from_server(sockfd);
                        int status = get_http_status_code(aux);
                        if (status >= 200 && status < 300) {
                            // In caz afirmativ afisez raspunsul
                            printf("SUCCESS: Film sters din colectie\n");
                        } else {
                            printf("ERROR\n");
                        }
                        close(sockfd);
                    }
                }
            } else {
                printf("ERROR\n");
            }
        }
        if(!strcmp(command, "logout_admin")) {  // Verific daca e comanda pentru delogare admin
            if(cookie) {
                sockfd = open_connection((char *)ADDR_IP, PORT, AF_INET, SOCK_STREAM, 0);  // Deschid o conexiune la server
                pack = compute_get_request((char *)ADDR_IP, (char *)"/api/v1/tema/admin/logout", NULL, cookie, 1, token);
                // Creez cererea
                send_to_server(sockfd, pack);  // Trimit cererea la server
                char *aux = receive_from_server(sockfd);
                int status = get_http_status_code(aux);
                if (status >= 200 && status < 300) {
                    // In caz afirmativ afisez mesajul
                    printf("SUCCESS: Admin delogat\n");
                    // Pun adminul pe false pentru a stii ca nu mai sunt logat ca admin
                    admin = false;
                } else {
                    printf("ERROR\n");
                }
                cookie = NULL;  // Reseteaza cookie-ul si token-ul
                token = NULL;
                close(sockfd);
            } else {
                printf("ERROR\n");
            }
        }
        if(!strcmp(command, "logout")) {  // Verific daca e comanda pentru delogare
            if(cookie) {
                sockfd = open_connection((char *)ADDR_IP, PORT, AF_INET, SOCK_STREAM, 0);  // Deschid o conexiune la server
                pack = compute_get_request((char *)ADDR_IP, (char *)"/api/v1/tema/user/logout", NULL, cookie, 1, token);
                // Creez cererea
                send_to_server(sockfd, pack);  // Trimit cererea la server
                char *aux = receive_from_server(sockfd);
                int status = get_http_status_code(aux);
                if (status >= 200 && status < 300) {
                    // In caz afirmativ afisez mesajul
                    printf("SUCCESS: Utilizator delogat\n");
                } else {
                    printf("ERROR\n");
                }
                cookie = NULL;  // Reseteaza cookie-ul si token-ul
                token = NULL;
                close(sockfd);
            } else {
                printf("ERROR\n");
            }
        }
        if(!strcmp(command, "exit")) {  // Verific daca e comanda pentru iesire
            break;
        }
    }

    return 0;
}
