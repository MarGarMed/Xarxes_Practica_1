// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#define DEFAULT_PORT 8080
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
	int sock, new_socket;
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	char buffer[BUFFER_SIZE] = {0};

	int port;
	char *endptr;

	char cims[100][BUFFER_SIZE]; //Array per guardar fins a 100 conjunts de dades
	int total_cims = 0; //Comptador per saber quants cims portem guardats

	//Creem variables de sumatori generals per poder anar sumant conforme s'introdueixen els cims
	float km_t = 0;
	int desnivell_t = 0; 


	if (argc == 1) {
		// Cap argument -> port per defecte
		port = DEFAULT_PORT;
		printf("Cap port indicat. Utilitzant valor per defecte: %d\n", port);
	}
	else if (argc == 2) {
		errno = 0;
		port = strtol(argv[1], &endptr, 10);

		// Comprovacions
		if (errno != 0 || *endptr != '\0' || port < 1 || port > 65535) {
			fprintf(stderr, "Error: el port ha de ser un enter entre 1 i 65535.\n");
			exit(EXIT_FAILURE);
		}
	}
	else {
		fprintf(stderr, "Ús: %s [port]\n", argv[0]);
		exit(EXIT_FAILURE);
	}


	// Crear el socket
	// Afegiu comentari explicant els arguments
	
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		perror("Error en crear el socket");
		exit(EXIT_FAILURE);
	}


	// Afegiu comentari explicant què són aquests 3 paràmetres,
	// per què s'utilitzen htons() i htonl(),
	// i per què s'utilitzen els valors que hi ha

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);	// Per a INADDR_ANY no faria falta htonl(), però li posem per coherència
	address.sin_port = htons(port);

	// Enllaçar el socket al port especificat
	// Afegiu comentari explicant els arguments
	if (bind(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
		perror("Error en fer el bind");
		close(sock);
		exit(EXIT_FAILURE);
	}

	// Escoltar connexions entrants
	// Afegiu comentari explicant els arguments
	
	if (listen(sock, 3) < 0) {
		perror("Error en escoltar");
		close(sock);
		exit(EXIT_FAILURE);
	}

	printf("Servidor en funcionament, esperant connexions...\n");

	while (1) {
		// Acceptar connexions de clients
		// Afegiu comentari explicant els arguments i per què hi ha i cal new_socket si ja tenim sock
		
		if ((new_socket = accept(sock, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
			perror("Error en acceptar la connexió");
			close(sock);
			exit(EXIT_FAILURE);
		}

		int pid = fork(); //Bifurcació
		if (pid == -1){
			printf("Hi ha hagut un error creant el fill\n");
			close(new_socket);

		} else if (pid == 0) {
			close(sock);
			while (1) {
				memset(buffer, 0, BUFFER_SIZE);
				
				// Llegir missatge del client
				if (recv(new_socket, buffer, BUFFER_SIZE, 0) <= 0) {
					printf("Client desconnectat (PID: %d)\n", getpid());
					break;
				}

				// Aquí haureu d'implementar l'anàlisi de la cadena rebuda per saber l'operació,
				// i si n'hi ha els arguments, executar-la i tornar el(s) resultat(s)

				// A continuació hi ha el codi corresponent a l'opció d'Enviar missatge, amb el retorn d'una cadena fixa,
				// i la de Sortida de la connexió/bucle si és el cas.
				// Modifiqueu la cadena de retorn per tal que sigui la que diu l'enunciat.

				printf("Missatge rebut del client (PID: %d): %s\n", getpid(), buffer);

				// Comprovar si el client vol tancar la connexió
				if (strcmp(buffer, "EXIT") == 0) {
					printf("Tancant connexió amb el client (PID: %d)...\n", getpid());
					send(new_socket, "Connexió tancada\n", strlen("Connexió tancada\n"), 0);
					break;	// Sortir del bucle
				} 
				else if (buffer[0] == '2'){
					char nom[100];
					float km = 0, temps = 0; 
					int desnivell = 0;

					//Extreure les dades separades per ";" que envia el client
					sscanf(buffer, "2;%99[^;];%f;%d;%f", nom, &km, &desnivell, &temps);

					//Guardar tot el conjunt de dades dins d'una posició de l'array
					snprintf(cims[total_cims], BUFFER_SIZE, "Cim: %s | Km: %.2f | Desnivell: %d | Temps: %.2f", nom, km, desnivell, temps);
					total_cims++;
					km_t += km;
					desnivell_t += desnivell;

					printf("Dades guardades correctament a la posició %d: %s\n", total_cims -1, cims[total_cims -1]);

					char send_buffer[] = "Les dades han sigut guardades correctament\n";
					send(new_socket, send_buffer, strlen(send_buffer), 0);
				}
				else if (buffer[0] == '3'){
					char resposta[BUFFER_SIZE * 10]; //Llista molt gran per poder ficar molts cims

					if (total_cims == 0){
						strcpy(resposta, "¡La llista de cims està buida! - Insereix un cim per poder visualitzar-lo.\n");

					} else {
						strcpy(resposta, "=== LLISTA DE CIMS ===\n");
					}
					
					for (int i=0; i<total_cims; i++){
						char linea[BUFFER_SIZE +64]; //Afegim uns bits de més per evitar truncaments
						//Formategem cada cim amb les seves dades
						snprintf(linea, sizeof(linea), "%d. %s\n", i + 1, cims[i]);
						strcat(resposta, linea);
					}

					//Enviament de la resposta
					send(new_socket, resposta, strlen(resposta), 0);
				}
				else if (buffer[0] == '4'){
					char send_buffer[BUFFER_SIZE];

					if(total_cims == 0){
						strcpy(send_buffer, "¡La llista de cims està buida! - Insereix un cim per poder visualitzar-lo.\n");
					} else{
						snprintf(send_buffer, BUFFER_SIZE, "==ESTADÍSTIQUES==\n - Km TOTALS: %.2f\n - DESNIVELL TOTAL: %d\n", km_t, desnivell_t);
					}

					//Enviem les dades al client
					send(new_socket, send_buffer, BUFFER_SIZE, 0);
				}
				else if (buffer[0] == '5'){
					int index;
					char send_buffer[BUFFER_SIZE];
					sscanf(buffer, "5;%d", &index);
					
					for(int i = index; i<total_cims; i++){
						strcpy(cims[i-1], cims[i]);
					}
					total_cims--;

					strcpy(send_buffer, "Les dades s'han esborrat correctament\n");
					send(new_socket, send_buffer, BUFFER_SIZE, 0);

				}
				else {
					memset(buffer, 0, BUFFER_SIZE);

					char resposta[BUFFER_SIZE + 1024];
					snprintf(resposta, sizeof(resposta),"%s\n", buffer);

					send(new_socket, resposta, strlen(resposta), 0);
				}
			}

			close(new_socket); // Tancar la connexió fill amb el client
		}
	}
	close(sock);
	return 0;
}
