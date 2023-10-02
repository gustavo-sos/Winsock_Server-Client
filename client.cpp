#define WIN32_LEAN_AND_MEAN
#define _WINNT_WIN32 0x0601

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <time.h>
#include <stdbool.h>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"


int __cdecl main(int argc, char **argv) 
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    char string[100]={0}, nome[50]={0}, opcao[1]={0}, auxHORA[36]={0}, auxRECVBUFF[36]={0}, auxINFO[36]={0};
	char *auxiliar_nome={0}, *ponteiro={NULL};
    int tamanho_fila = 0;

    
    auxiliar_nome = (char*)malloc(sizeof(auxiliar_nome));
	
    auxiliar_nome = getenv("USERPROFILE");
    strcpy(nome, auxiliar_nome);
	int ABC = NULL;
	do{
	    printf("\nDigite sua mensagem: ");
	    fflush(stdin);
	    gets(string);
	    ABC = strlen(string);
	}while( ABC == NULL);

    printf("\nDeseja visualizar a lista de impressao (s/n): ");
    fflush(stdin);
    gets(opcao);

    char recvbuf[DEFAULT_BUFLEN], nome_server[DEFAULT_BUFLEN], msg_server[DEFAULT_BUFLEN];
    int iResult, iResult2, iResult3, iResult4;
    int recvbuflen = DEFAULT_BUFLEN;
    
    // Validate the parameters
    if (argc != 2) {
        printf("usage: %s server-name\n", argv[0]);
        return 1;
    }

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    // Send an initial buffer
    iResult = send( ConnectSocket, nome, (int)strlen(nome), 0);

    if (iResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }
    printf("Bytes Sent NOME: %ld\n", iResult);
    
    iResult2 = send( ConnectSocket, string, (int)strlen(string), 0);

    if (iResult2 == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    iResult3 = send( ConnectSocket, opcao, (int)strlen(opcao), 0);

    if (iResult3 == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    printf("Bytes Sent MSG: %ld\n", iResult2);

    // shutdown the connection since no more data will be sent
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }
    
    iResult2 = shutdown(ConnectSocket, SD_SEND);
    if (iResult2 == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    iResult3 = shutdown(ConnectSocket, SD_SEND);
    if (iResult3 == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }
    
    int j = 0;
    // Receive until the peer closes the connection
    do {

        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);

        if (j > 0)printf("\n*******\n");
        if(iResult > 0) printf("\nPOSICAO [%i]", j+1);
        j++;
        if ( iResult > 0 ){

            ponteiro = strtok(recvbuf, ";");
            int i = 0;
            while(ponteiro){

                if (i==0){
                    strcpy(auxRECVBUFF, ponteiro);
                    printf("\nNome: %s", auxRECVBUFF);
                }
                else if (i==1){
                    strcpy(auxINFO, ponteiro);
                    printf("\nMensagem: %s", auxINFO);
                }
                else if (i==2){
                    strcpy(auxHORA, ponteiro);
                    printf("\nHora: %s", auxHORA);
                }

                ponteiro = strtok(NULL, ";");
                if (i==2){
                    break;
                }
                i++;
                
            }
        }
        else if ( iResult == 0 )
            printf("\n\nConnection closed\n");
        else
            printf("recv failed with error: %d\n", WSAGetLastError());

    } while( iResult > 0 );
    memset(recvbuf,0,sizeof(recvbuf));

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();
    return 0;
}
