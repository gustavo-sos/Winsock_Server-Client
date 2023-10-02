#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"
#define TAM50 50 
#define TAM100 100

typedef char NOME[TAM50];
typedef char INFO[TAM100];


typedef struct NODO{
 NOME nome;
 INFO info;
 NODO *proximo;
 tm time;
}NODO;

typedef struct FILA{
 int tamanho;
 NODO *primeiro,*ultimo; 
}FILA;

FILA *cria();
void aloca(FILA *elemento, char *info, char *nome, tm time);
void usa(FILA *elemento);
void mostraFILA(FILA *elemento, SOCKET ClientSocket, int x);
int quantidade(FILA *elemento);


int __cdecl main(void) 
{
    WSADATA wsaData;
    int iResult, iResult2, iResult3, x=0;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo *result = NULL;
    struct addrinfo hints;

    int iSendResult, iSendResult2, iSendResult3, qtd_fila, cont_mensagem=0;
    char recvbuf[DEFAULT_BUFLEN], nome[DEFAULT_BUFLEN], opcao[DEFAULT_BUFLEN], string_aux[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN, nomeLEN = DEFAULT_BUFLEN, opcaoLEN = DEFAULT_BUFLEN;
    
    // Receive until the peer shuts down the connection
    bool abresocket=true;
    system("cls");
    printf("Digite qual sera a quantidade de itens na fila: ");
    scanf("%d", &qtd_fila);
    system("cls");
    printf("\nAguardando conexao...\n");
    FILA *fila;
    fila=cria();
    FILE *ARQ;
    
    
    do {
        
        if (abresocket){
	        // Initialize Winsock
		    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
		    if (iResult != 0) {
		        printf("WSAStartup failed with error: %d\n", iResult);
		        return 1;
		    }
		
		    ZeroMemory(&hints, sizeof(hints));
		    hints.ai_family = AF_INET;
		    hints.ai_socktype = SOCK_STREAM;
		    hints.ai_protocol = IPPROTO_TCP;
		    hints.ai_flags = AI_PASSIVE;
		
		    // Resolve the server address and port
		    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
		    if ( iResult != 0 ) {
		        printf("getaddrinfo failed with error: %d\n", iResult);
		        WSACleanup();
		        return 1;
		    }
		
		    // Create a SOCKET for the server to listen for client connections.
		    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		    if (ListenSocket == INVALID_SOCKET) {
		        printf("socket failed with error: %ld\n", WSAGetLastError());
		        freeaddrinfo(result);
		        WSACleanup();
		        return 1;
		    }
		
		    // Setup the TCP listening socket
		    iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
		    if (iResult == SOCKET_ERROR) {
		        printf("bind failed with error: %d\n", WSAGetLastError());
		        freeaddrinfo(result);
		        closesocket(ListenSocket);
		        WSACleanup();
		        return 1;
		    }
		
		    freeaddrinfo(result);
		
		    iResult = listen(ListenSocket, SOMAXCONN);
		    if (iResult == SOCKET_ERROR) {
		        printf("listen failed with error: %d\n", WSAGetLastError());
		        closesocket(ListenSocket);
		        WSACleanup();
		        return 1;
		    }
		
		    // Accept a client socket
		    ClientSocket = accept(ListenSocket, NULL, NULL);
		    if (ClientSocket == INVALID_SOCKET) {
		        printf("accept failed with error: %d\n", WSAGetLastError());
		        closesocket(ListenSocket);
		        WSACleanup();
		        return 1;
		    }
		
		    // No longer need server socket
		    closesocket(ListenSocket);
    	}
		
    	abresocket = false;
    	memset(nome,0,sizeof(nome));
		memset(recvbuf,0,sizeof(recvbuf));
		memset(opcao,0,sizeof(opcao));
		
        iResult = recv(ClientSocket, nome, nomeLEN, 0);
		iResult2 = recv(ClientSocket, recvbuf, recvbuflen, 0);
		iResult3 = recv(ClientSocket, opcao, opcaoLEN, 0);
		
        if (iResult > 0) {
        	
			printf("\n===================================");
            printf("\nBytes received NOME: %d", iResult);
            printf("\nBytes received MSG: %d\n", iResult2);
			
            printf("\nNome recebida: %s", nome);
            
            x = strspn(opcao, "s");
            
			printf("\nMensagem recebido: %s", recvbuf);
			time_t t = time(NULL);
  			struct tm tm = *localtime(&t);
  			printf("\nHORA: %02d-%02d-%d %02d:%02d:%02d\n\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);
  			
			aloca(fila,recvbuf,nome,tm);  /// adiciona a mensagem e o nome na fila
            
			memset(nome,0,sizeof(nome));
			memset(recvbuf,0,sizeof(recvbuf));
			
			printf("\nBytes sent NOME: %d", iResult);
            printf("\nBytes sent MSG: %d", iResult2);

			printf("\n===================================\n");
            printf("\n*******\n");
            mostraFILA(fila, ClientSocket, x);
            printf("\n*******\n");
            
			if(qtd_fila==quantidade(fila)){ ///se a fila estiver cheia
        		/////criar arquivo e limpar a fila para armazernar novamente;
        		printf("\n\nFILA CHEIA! IMPRIMINDO ARQUIVO\n\n");
        		ARQ=fopen("impressao.txt","a");
   				if(!ARQ)
   				{
					printf("Erro ao abrir o arquivo.");
   				}
   				while(quantidade(fila)>0){
   					fprintf(ARQ, "=========================\n");
   					fprintf(ARQ,"\nMENSAGEM %d\n\n",cont_mensagem+1);
   					fprintf(ARQ,"Usu�rio: %s\n",fila->primeiro->nome);
   					fprintf(ARQ,"Mensagem: %s\n",fila->primeiro->info);
   					fprintf(ARQ,"Hora: %02d-%02d-%d %02d:%02d:%02d\n", fila->primeiro->time.tm_mday, fila->primeiro->time.tm_mon, fila->primeiro->time.tm_year, fila->primeiro->time.tm_hour, fila->primeiro->time.tm_min, fila->primeiro->time.tm_sec);
   					fprintf(ARQ, "\n=========================\n");
					usa(fila);
					cont_mensagem++;
				}
   				fclose(ARQ);
        		printf("\nARQUIVO IMPRESSO\n");
        		printf("\n===================================\n");
			}
            
            // shutdown the connection since we're done
		    iResult = shutdown(ClientSocket, SD_SEND);
		    if (iResult == SOCKET_ERROR) {
		        printf("shutdown failed with error: %d\n", WSAGetLastError());
		        closesocket(ClientSocket);
		        WSACleanup();
		        return 1;
		    }

            iResult2 = shutdown(ClientSocket, SD_SEND);
		    if (iResult2 == SOCKET_ERROR) {
		        printf("shutdown failed with error: %d\n", WSAGetLastError());
		        closesocket(ClientSocket);
		        WSACleanup();
		        return 1;
		    }
		    
		    iResult3 = shutdown(ClientSocket, SD_SEND);
		    if (iResult3 == SOCKET_ERROR) {
		        printf("shutdown failed with error: %d\n", WSAGetLastError());
		        closesocket(ClientSocket);
		        WSACleanup();
		        return 1;
		    }
		
		    // cleanup
		    closesocket(ClientSocket);
		    WSACleanup();
		    abresocket=true;
        }
        else if (iResult == 0)
            printf("Connection closing...\n");
        else  {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }
        
         //iResult > 0
    } while (true);

    // shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    closesocket(ClientSocket);
    WSACleanup();

    return 0;
}

//fun��o que cria a FILA
FILA *cria(){
    FILA *p;
    p=(FILA *)malloc(sizeof(FILA)); 
    p->tamanho=0;
    p->primeiro=NULL;
    p->ultimo=NULL;    
    return(p);
}

//aloca novo elemento na FILA
void aloca(FILA *elemento, char *info, char *nome, tm tm){
    NODO *p,*n;//n � o ponteiro para alocar o novo elemento
    
    p=elemento->ultimo;//p � o ponteiro para navega��o na FILA
    n=(NODO *)malloc(sizeof(NODO));
    
    strcpy(n->nome,nome);
    strcpy(n->info,info);
    
    n->time.tm_mday=tm.tm_mday;
    n->time.tm_mon=tm.tm_mon+1;
    n->time.tm_year=tm.tm_year + 1900;
    n->time.tm_hour=tm.tm_hour;
    n->time.tm_min=tm.tm_min;
    n->time.tm_sec=tm.tm_sec;
    //////////////////////////////
    n->proximo=NULL;//o novo elemento n�o aponta para ninguem, por isso atribui-se NULL     
    elemento->ultimo=n;//o novo elemento � o �ltimo da FILA 
	if (p==NULL){//para guardar o primeiro elemento da FILA 
      elemento->primeiro=n;
      elemento->tamanho=1;
    }
    else{//para alocar os outros nodos da FILA
      p->proximo=n;
      elemento->tamanho = elemento->tamanho + 1;
    }     
}

//usa e retorna o elemento do recurso da FILA
void usa(FILA *elemento){
    NODO *p;
    p=elemento->primeiro;
    if (p==NULL){//n�o h� elementos na FILA 
      printf("\nFILA VAZIA\n");
    }
    else{
      //remover o primeiro elemento da FILA
      elemento->primeiro=p->proximo;
      free(p);
      elemento->tamanho = elemento->tamanho - 1;
      if (elemento->primeiro == NULL){//foi removido o ultimo elemento
      	  elemento->ultimo=NULL;
      	  elemento->tamanho=0;
	  }
    }
}

//aloca novo elemento na FILA
void mostraFILA(FILA *elemento, SOCKET ClientSocket, int x){
    NODO *p;
    int conta=0, iSendResult;
	char aux[36]={0}, auxNOME[36]={0}, auxINFO[36]={0}, conteudo[DEFAULT_BUFLEN]={0};

    p=elemento->primeiro;//p � o ponteiro para navega��o na FILA
    while(p!=NULL){
       conta++;
       memset(conteudo,0,sizeof(conteudo));
       memset(auxNOME,0,sizeof(auxNOME));
       memset(auxINFO,0,sizeof(auxINFO));
       memset(aux,0,sizeof(aux));
       
       printf("\nDADOS[%d]",conta);
       sprintf(auxNOME,"Nome: %s",p->nome);
       sprintf(auxINFO,"Mensagem: %s",p->info);
   	   sprintf(aux,"Hora: %02d-%02d-%d %02d:%02d:%02d", p->time.tm_mday, p->time.tm_mon, p->time.tm_year, p->time.tm_hour, p->time.tm_min, p->time.tm_sec);
   	   
   	   printf("\n%s", auxNOME);
   	   printf("\n%s", auxINFO);
   	   printf("\n%s", aux);
	   
	   
	   
	   if (x == 1){
			strcat(p->nome, ";");
			strcat(p->nome, "\0");

			strcat(p->info, ";");
			strcat(p->info, "\0");

			strcat(aux, ";");
			strcat(aux, "\0");

			strcat(conteudo, p->nome);
			strcat(conteudo, p->info);
			strcat(conteudo, aux);
			strcat(conteudo, "\0");

			// Essa é a string que será enviada para o client
			//printf("\nCONTEUDO == %s", conteudo);
			//printf("\n==============");
            iSendResult = send( ClientSocket, conteudo, (int)strlen(conteudo), 0);
			
			if (iSendResult == SOCKET_ERROR) {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
            }
       } 

       p=p->proximo;//navega��o na FILA, p recebe o ponteiro do pr�ximo
    }
}


int quantidade(FILA *elemento){
	return(elemento->tamanho);
}
