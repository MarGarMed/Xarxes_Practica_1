// client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

#define DEFAULT_PORT 8080
#define DEFAULT_DOMAIN "localhost"
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
	int sock = 0;
	struct sockaddr_in serv_addr;
	char buffer[BUFFER_SIZE] = {0};
	char cadena[BUFFER_SIZE] = "";

	char *domini;
	int port;
	char *endptr;

	if (argc == 1) {
		// Cap argument -> valors per defecte
		domini = DEFAULT_DOMAIN;
		port = DEFAULT_PORT;
		printf("Cap argument indicat. Utilitzant valors per defecte: %s %d\n", domini, port);
	} 
	else if (argc == 3) {
		domini = argv[1];

		errno = 0;
		port = strtol(argv[2], &endptr, 10);

		// Comprovacions de validesa
		if (errno != 0 || *endptr != '\0' || port < 1 || port > 65535) {
			fprintf(stderr, "Error: el port ha de ser un enter entre 1 i 65535.\n");
			exit(EXIT_FAILURE);
		}
	} 
	else {
		fprintf(stderr, "Ús: %s [nom_de_domini port]\n", argv[0]);
		exit(EXIT_FAILURE);
	}



	// Crear el socket
	// Afegiu comentari explicant els arguments, i de quines altres opcions hi ha
	// per al segon d'ells (ara SOCK_STREAM)
	
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("\nError en crear el socket\n");
		return -1;
	}

	// Afegiu comentari sobre què són aquests 3 paràmetres,
	// per què s'utilitzen els valors que hi ha,
	// i per què s'utilitza htons()

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	inet_pton(AF_INET, domini, &serv_addr.sin_addr);	// Convertir adreça IPv4 a binari
   

	// Connectar amb el servidor
	// Afegiu comentari explicant els arguments
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		printf("\nConnexio fallida\n");
		return -1;
	}

	printf("Connectat al servidor. Podeu començar a enviar missatges.\n");

	int option;

	while (1) {
		memset(buffer, 0, sizeof(buffer)); // Netegem el buffer

		// Menú principal
		// Cal que implementeu un petit servei remot amb almenys 4 funcionalitats noves
		// Definiu vosaltres mateixos les dades a enviar (demanar en el client, i analitzar al servidor) i rebre
		// Si per donar més sentit al servei cal alguna opció més, la podeu afegir
		// Deixeu l'opció 1 com a Enviar missatge i l'última per Sortir

		printf("Menú principal:\n");
		printf("1. Enviar missatge\n");
		printf("2. Inserir cim\n");
		printf("3. Mostrar llista de cims\n");
		printf("4. Mostrar estadístiques\n");
		printf("5. Eliminar un cim\n");
		printf("6. Sortir\n");
		printf("Opció: ");

		scanf("%d", &option);
		while (getchar() != '\n');  // buida el buffer fins al salt de línia

		switch (option)
		{
		case 1:
		case 6:
			if (option==1){
				printf("Introdueix el missatge a enviar ('EXIT' per tancar el servidor i sortir): ");
				fgets(cadena, BUFFER_SIZE, stdin);
				cadena[strcspn(cadena, "\n")] = '\0';  // Eliminar \n final
			} else {
				strcpy(cadena, "EXIT");
			}

			// Enviar missatge al servidor
			// Afegiu control d'errors
			// Afegiu comentari explicant els arguments

			send(sock, cadena, strlen(cadena), 0);

			// Llegir resposta del servidor
			// Afegiu un control d'errors al recv()
			// Afegiu comentari explicant què fa i per què s'utilitza memset
			// Afegiu comentari explicant els arguments de la crida a recv()

			memset(buffer, 0, BUFFER_SIZE);
			recv(sock, buffer, BUFFER_SIZE,0);
			printf("Resposta del servidor: %s\n", buffer);

			if (strcmp(cadena, "EXIT") == 0) {
				close(sock);
				return 0;
			}
			break;
		// Demanar a l'usuari una muntanya i les seves característiques
		case 2:
			float km =0, temps = 0;
			int desnivell = 0;
			char nom[1000];
		
			//Escanejar dades inserides en variables diferents
			printf("Insereix el cim i les seves característiques (Nom, km, desnivell [m], temps [min]): ");
			scanf("%s %f %d %f", nom, &km, &desnivell, &temps);

			//Enviar les dades al servidor
			memset(buffer, 0, BUFFER_SIZE);
			snprintf(buffer, BUFFER_SIZE, "2;%s;%f;%d;%f", nom, km, desnivell, temps);
			send(sock, buffer, strlen(buffer), 0);

			//Rebre la confirmació del servidor
			memset(buffer, 0, BUFFER_SIZE);
			recv(sock, buffer, BUFFER_SIZE, 0);
			printf("%s\n", buffer);
			break;

		//Mostrar llista de cims
		case 3:
			// Enviar al servidor la peticó de llistar els cims
			memset(buffer, 0, BUFFER_SIZE);
			strcpy(buffer, "3");
			send(sock, buffer, BUFFER_SIZE, 0);

			// Rebre resposta del servidor
			memset(buffer, 0, BUFFER_SIZE);
			recv(sock, buffer, BUFFER_SIZE, 0);
			
			//Mostrar per pantalla
			printf("\n%s\n", buffer);
			break;
		
		//Mostrar estadístiques
		case 4:
			//Enviar al servidor la petició
			memset(buffer, 0, BUFFER_SIZE);
			strcpy(buffer, "4");
			send(sock, buffer, BUFFER_SIZE, 0);
		
			//Obtenir i mostrar per pantalla les estadístiques enviades del servidor
			memset(buffer, 0, BUFFER_SIZE);
			recv(sock, buffer, BUFFER_SIZE, 0);
			printf("\n%s\n", buffer);
			break;

		//Esborrar un cim de la llista a escollir
		case 5:
			int index;
			
			//Demanar la posició a la llista del cim que es vol eliminar
			printf("Insereix el número del cim que vols eliminar: ");
			scanf("%d", &index);

			//Enviar al servidor la opció escollida i l'index
			memset(buffer, 0, BUFFER_SIZE);
			snprintf(buffer, BUFFER_SIZE, "5;%d", index);
			send(sock, buffer, BUFFER_SIZE, 0);

			//Mostrar per pantalla la confirmació
			memset(buffer, 0, BUFFER_SIZE);
			recv(sock, buffer, BUFFER_SIZE, 0);
			printf("%s\n", buffer);
			break;

		default:
			printf("Opció invàlida\n");
			break;
		}
	}

	close(sock);
	return 0;
}
