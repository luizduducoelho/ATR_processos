#define WIN32_LEAN_AND_MEAN 
#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <conio.h>
#include <iostream>
#include <string>
#include <vector>
#include <time.h>  
#define _CHECKERROR	1	// Ativa função CheckForError
#include "CheckForError.h"
#define	ESC			0x1B

typedef unsigned (WINAPI *CAST_FUNCTION)(LPVOID);	//Casting para terceiro e sexto parâmetros da função
													//_beginthreadex
typedef unsigned *CAST_LPDWORD;

// Declaracao das Threads
DWORD WINAPI ThreadLeituraCLP(int);			 //Thread representando a leitura do CLP
DWORD WINAPI ThreadLeituraPCP(int);			 //Thread representando a leitura do sistema de PCP
DWORD WINAPI ThreadCapturaDeMensagens(int);  //Thread representando a captura de mensagens do CLP e do PCP
DWORD WINAPI ThreadExibicaoDeDados(int);	 //Thread representando a exibição de dados da sala de controle


											 // Lista circular em memória 1
int tamanho_lista_circular_1 = 200;
std::vector<std::string> lista_circular_CLP_PCP(tamanho_lista_circular_1);
HANDLE hLivres_lista1;			// Semáforo contador que indica posições livres na lista circular 1
HANDLE hOcupados_lista1;		// Semáforo contafor que indica posições ocupadas na lista circular 1
HANDLE hMutex_lista1;			// Semáforo binário para exclusão mútua da lista circular 1
int p_livre_lista1 = 0;
int p_ocupado_lista1 = 0;

// Lista circular em memória 2
int tamanho_lista_circular_2 = 100;
std::vector<std::string> lista_circular_exibicao_dados(tamanho_lista_circular_2);
HANDLE hLivres_lista2;			// Semáforo contador que indica posições livres na lista circular 1
HANDLE hOcupados_lista2;		// Semáforo contafor que indica posições ocupadas na lista circular 1
HANDLE hMutex_lista2;			// Semáforo binário para exclusão mútua da lista circular 1
int p_livre_lista2, p_ocupado_lista2 = 0;

// Declaração dos Handles para os semáforos de leitura do teclado
HANDLE hControla_leitura_clp;
HANDLE hControla_leitura_pcp;
HANDLE hControla_retirada_de_mensagens;
HANDLE hControla_sistema_de_gestao;
HANDLE hControla_sistema_de_exibicao_de_dados;

// Declaração do Handle para escrita assíncrona
HANDLE hPipeEvent;

// Função para retornar uma string alfanumérica
static const char alphanum[] =
"0123456789"
"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
int stringLength = sizeof(alphanum) - 1;
std::string genRandomString(int size_of_string)  // Random string generator function.
{
	std::string random_string(1, alphanum[rand() % stringLength]);
	for (int i = 1; i<8; i++) {
		random_string += alphanum[rand() % stringLength];
	}
	return random_string;
}

// Função para escrever na lista circular 1
void escreve_lista_circular1(std::string message) {
	DWORD status;
	LONG dwContagemPrevia;
	status = WaitForSingleObject(hLivres_lista1, INFINITE);				     // Há posições livres na lista
	CheckForError(status == WAIT_OBJECT_0);
	status = WaitForSingleObject(hMutex_lista1, INFINITE);				     // Garantir exclusão mútua
	CheckForError(status == WAIT_OBJECT_0);
	lista_circular_CLP_PCP[p_livre_lista1] = message;
	p_livre_lista1 = (p_livre_lista1 + 1) % tamanho_lista_circular_1;
	//std::cout << message << std::endl;
	CheckForError(ReleaseSemaphore(hMutex_lista1, 1, &dwContagemPrevia));    // Libera o "mutex"
	CheckForError(ReleaseSemaphore(hOcupados_lista1, 1, &dwContagemPrevia)); // Ocupa uma vaga na lista
}

// Função para ler da lista circular 1
std::string le_lista_circular1() {
	DWORD status;
	LONG dwContagemPrevia;
	std::string message;
	status = WaitForSingleObject(hOcupados_lista1, INFINITE);				 // Há posições para serem lidas da lista
	CheckForError(status == WAIT_OBJECT_0);
	status = WaitForSingleObject(hMutex_lista1, INFINITE);				     // Garantir exclusão mútua
	CheckForError(status == WAIT_OBJECT_0);
	message = lista_circular_CLP_PCP[p_ocupado_lista1];
	p_ocupado_lista1 = (p_ocupado_lista1 + 1) % tamanho_lista_circular_1;
	//std::cout << message << std::endl;
	CheckForError(ReleaseSemaphore(hMutex_lista1, 1, &dwContagemPrevia));    // Libera o "mutex"
	CheckForError(ReleaseSemaphore(hLivres_lista1, 1, &dwContagemPrevia));   // Libera uma vaga na lista
	return message;
}

// Função para escrever na lista circular2
void escreve_lista_circular2(std::string message) {
	DWORD status;
	LONG dwContagemPrevia;
	status = WaitForSingleObject(hLivres_lista2, INFINITE);				     // Há posições livres na lista
	CheckForError(status == WAIT_OBJECT_0);
	status = WaitForSingleObject(hMutex_lista2, INFINITE);				     // Garantir exclusão mútua
	CheckForError(status == WAIT_OBJECT_0);
	lista_circular_exibicao_dados[p_livre_lista2] = message;
	p_livre_lista2 = (p_livre_lista2 + 1) % tamanho_lista_circular_2;
	CheckForError(ReleaseSemaphore(hMutex_lista2, 1, &dwContagemPrevia));    // Libera o "mutex"
																			 //std::cout << message << std::endl;
	CheckForError(ReleaseSemaphore(hOcupados_lista2, 1, &dwContagemPrevia)); // Ocupa uma vaga na lista
}

// Função para ler da lista circular 2
std::string le_lista_circular2() {
	DWORD status;
	LONG dwContagemPrevia;
	std::string message;
	status = WaitForSingleObject(hOcupados_lista2, INFINITE);				 // Há posições para serem lidas da lista
	CheckForError(status == WAIT_OBJECT_0);
	status = WaitForSingleObject(hMutex_lista2, INFINITE);				     // Garantir exclusão mútua
	CheckForError(status == WAIT_OBJECT_0);
	message = lista_circular_exibicao_dados[p_ocupado_lista2];
	p_ocupado_lista2 = (p_ocupado_lista2 + 1) % tamanho_lista_circular_2;
	//std::cout << message << std::endl;
	CheckForError(ReleaseSemaphore(hMutex_lista2, 1, &dwContagemPrevia));    // Libera o "mutex"
	CheckForError(ReleaseSemaphore(hLivres_lista2, 1, &dwContagemPrevia));   // Libera uma vaga na lista
	return message;
}

// Thread primária
int main() {
	// Handle das Threads
	int NThreads = 4;
	HANDLE hThreads[4];
	DWORD dwThreadLeituraCLP;
	DWORD dwThreadLeituraPCP;
	DWORD dwThreadCapturaDeMensagens;
	DWORD dwThreadExibicaoDeDados;
	DWORD dwRet;

	// Criação dos objetos de sincronização da lista circular 1
	hLivres_lista1 = CreateSemaphore(NULL, tamanho_lista_circular_1, tamanho_lista_circular_1, "Livres_lista1");
	CheckForError(hLivres_lista1);
	hOcupados_lista1 = CreateSemaphore(NULL, 0, tamanho_lista_circular_1, "Ocupados_lista1");
	CheckForError(hOcupados_lista1);
	hMutex_lista1 = CreateSemaphore(NULL, 1, 1, "Mutex_lista1");
	CheckForError(hMutex_lista1);

	// Criação dos objetos de sincronização da lista circular 2
	hLivres_lista2 = CreateSemaphore(NULL, tamanho_lista_circular_2, tamanho_lista_circular_2, "Livres_lista2");
	CheckForError(hLivres_lista1);
	hOcupados_lista2 = CreateSemaphore(NULL, 0, tamanho_lista_circular_2, "Ocupados_lista2");
	CheckForError(hOcupados_lista2);
	hMutex_lista2 = CreateSemaphore(NULL, 1, 1, "Mutex_lista2");
	CheckForError(hMutex_lista2);

	// Criação do evento para escrita assincrona
	hPipeEvent = CreateEvent(NULL, TRUE, FALSE, "PipeEvent");
	CheckForError(hPipeEvent);

	// Criação da thread de CLP
	hThreads[0] = (HANDLE)_beginthreadex(NULL,
		0,
		(CAST_FUNCTION)ThreadLeituraCLP,	//Casting necessário
		(LPVOID)0,
		0,
		(CAST_LPDWORD)&dwThreadLeituraCLP);		//Casting necessário
	if (hThreads[0])
		printf("Thread LeituraCLP criada com sucesso! Id=%0x\n", dwThreadLeituraCLP);
	else {
		printf("Erro na criacao da thread LeituraPCP! Erro = %d\n", errno);
		exit(0);
	}

	// Criação dos semáforos para leitura do teclado
	hControla_leitura_clp = CreateSemaphore(NULL, 1, 1, "Controla_leitura_clp");
	CheckForError(hControla_leitura_clp);
	hControla_leitura_pcp = CreateSemaphore(NULL, 1, 1, "Controla_leitura_pcp");
	CheckForError(hControla_leitura_pcp);
	hControla_retirada_de_mensagens = CreateSemaphore(NULL, 1, 1, "Controla_retirada_de_mensagens");
	CheckForError(hControla_retirada_de_mensagens);
	hControla_sistema_de_gestao = CreateSemaphore(NULL, 1, 1, "Controla_sistema_de_gestao");
	CheckForError(hControla_sistema_de_gestao);
	hControla_sistema_de_exibicao_de_dados = CreateSemaphore(NULL, 1, 1, "Controla_sistema_de_exibicao_de_dados");
	CheckForError(hControla_sistema_de_exibicao_de_dados);

	// Criação da Thread PCP
	hThreads[1] = (HANDLE)_beginthreadex(NULL,
		0,
		(CAST_FUNCTION)ThreadLeituraPCP,
		(LPVOID)0,
		0,
		(CAST_LPDWORD)&dwThreadLeituraPCP);
	if (hThreads[1])
		printf("Thread LeituraPCP criada com sucesso! Id=%0x\n", dwThreadLeituraPCP);
	else {
		printf("Erro na criacao da thread LeituraPCP! Erro = %d\n", errno);
		exit(0);
	}

	// Criação da Thread de Captura de Mensagens
	hThreads[2] = (HANDLE)_beginthreadex(NULL,
		0,
		(CAST_FUNCTION)ThreadCapturaDeMensagens,
		(LPVOID)0,
		0,
		(CAST_LPDWORD)&dwThreadCapturaDeMensagens);
	if (hThreads[2])
		printf("Thread de Captura de mensagens criada com sucesso! Id=%0x\n", dwThreadCapturaDeMensagens);
	else {
		printf("Erro na criacao da thread Captura de mensagens! Erro = %d\n", errno);
		exit(0);
	}

	// Criação da Thread de Exibição De Dados
	hThreads[3] = (HANDLE)_beginthreadex(NULL,
		0,
		(CAST_FUNCTION)ThreadExibicaoDeDados,
		(LPVOID)0,
		0,
		(CAST_LPDWORD)&dwThreadExibicaoDeDados);
	if (hThreads[3])
		printf("Thread Exibição de dados criada com sucesso! Id=%0x\n", dwThreadExibicaoDeDados);
	else {
		printf("Erro na criacao da thread de Exibição de dados! Erro = %d\n", errno);
		exit(0);
	}

	// Criação do processo de gestão da produção
	int status;
	STARTUPINFO si;				    // StartUpInformation para novo processo
	PROCESS_INFORMATION NewProcess;	// Informações sobre novo processo criado
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);	// Tamanho da estrutura em bytes
	status = CreateProcess(
		//"C:\\Users\\User\\Documents\\atr\\tp\\TrabalhoPratico\\Debug\\processo_gestao_da_producao.exe", // Caminho do arquivo executável
		"..\\Debug\\processo_gestao_da_producao.exe",
		NULL,                       // Apontador p/ parâmetros de linha de comando
		NULL,                       // Apontador p/ descritor de segurança
		NULL,                       // Idem, threads do processo
		FALSE,	                    // Herança de handles
		CREATE_NEW_CONSOLE,			// Flags de criação
		NULL,	                    // Herança do ambiente de execução
		"..\\Debug\\",      // Diretório do arquivo executável
							//"C:\\Program Files\\Mozilla Firefox",
		&si,			            // lpStartUpInfo
		&NewProcess);	            // lpProcessInformation
	if (!status) printf("Erro na criacao do Processo de gestao da producao! Codigo = %d\n", GetLastError());
	else printf("Processo de gestao da producao criado com sucesso!! \n");

	// Criação do processo de leitura do teclado
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);	// Tamanho da estrutura em bytes
	status = CreateProcess(
		//"C:\\Users\\User\\Documents\\atr\\tp\\TrabalhoPratico\\Debug\\processo_gestao_da_producao.exe", // Caminho do arquivo executável
		"..\\Debug\\processo_leitura_do_teclado.exe",
		NULL,                       // Apontador p/ parâmetros de linha de comando
		NULL,                       // Apontador p/ descritor de segurança
		NULL,                       // Idem, threads do processo
		FALSE,	                    // Herança de handles
		CREATE_NEW_CONSOLE,			// Flags de criação
		NULL,	                    // Herança do ambiente de execução
		"..\\Debug\\",      // Diretório do arquivo executável
							//"C:\\Program Files\\Mozilla Firefox",
		&si,			            // lpStartUpInfo
		&NewProcess);	            // lpProcessInformation
	if (!status) printf("Erro na criacao do Processo de leitura do teclado! Codigo = %d\n", GetLastError());
	else printf("Processo de leitura do teclado criado com sucesso!! \n");

	// Aguarda as threads terminarem
	dwRet = WaitForMultipleObjects(NThreads, hThreads, TRUE, INFINITE);
	CheckForError(dwRet == WAIT_OBJECT_0);

	// Fechamento dos Handles de criação das Threads
	for (int i = 0; i < NThreads; i++) {
		CloseHandle(hThreads[i]);
	}

	// Fechamento dos Handles da lista circular 1
	CloseHandle(hLivres_lista1);
	CloseHandle(hOcupados_lista1);
	CloseHandle(hMutex_lista1);

	// Fechamento dos Handles da lista circular 2
	CloseHandle(hLivres_lista2);
	CloseHandle(hOcupados_lista2);
	CloseHandle(hMutex_lista2);

	// Fecha Handles para os semáforos de leitura do teclado
	CloseHandle(hControla_leitura_clp);
	CloseHandle(hControla_leitura_pcp);
	CloseHandle(hControla_retirada_de_mensagens);
	CloseHandle(hControla_sistema_de_gestao);
	CloseHandle(hControla_sistema_de_exibicao_de_dados);
	
	// Fecha o Handle para o evento de escrita assíncrona
	CloseHandle(hPipeEvent);

	return 0;
}

DWORD WINAPI ThreadLeituraCLP(int i) {
	std::cout << "Inside CLP Thread!" << std::endl;
	HANDLE hEvent;
	DWORD status;
	int NSEQ = 0;
	float TZONA1, TZONA2, TZONA3, VOLUME, PRESSAO;
	int TEMPO;
	srand((unsigned int)time(NULL));
	int j = 0;

	// Abre semáforo
	LONG dwContagemPrevia;
	hControla_leitura_clp = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, "Controla_leitura_clp");
	CheckForError(hControla_sistema_de_gestao);

	while (TRUE) {

		// Conquista semáforo
		status = WaitForSingleObject(hControla_leitura_clp, INFINITE);	 // Verifica se pode executar
		CheckForError(status == WAIT_OBJECT_0);

		// Libera semáforo
		CheckForError(ReleaseSemaphore(hControla_leitura_clp, 1, &dwContagemPrevia));

		// Usando WaitForSingleObject como um temporizador de 500ms
		hEvent = CreateEvent(NULL, TRUE, FALSE, "EvTimeout");
		CheckForError(hEvent);
		status = WaitForSingleObject(hEvent, 500);
		if (status != WAIT_TIMEOUT) {
			std::cout << "Error in ThreadLeituraCLP Timeout !!!" << std::endl;
		}

		// Atribuindo variáveis
		j += 1;
		NSEQ = (NSEQ + 1) % 1000000;		      // Incrementa numero de sequencia em 1
		TZONA1 = (float)(rand() % 100000) / 10;	  // Número entre 0000.0 e 9999.9
		TZONA2 = (float)(rand() % 100000) / 10;
		TZONA3 = (float)(rand() % 100000) / 10;
		VOLUME = (float)(rand() % 10000) / 10;	  // Número entre 000.0 e 999.9
		PRESSAO = (float)(rand() % 10000) / 10;
		TEMPO = rand() % 10000;					 // Número entre 0000 e 9999
												 // Data corrente
		time_t now = time(0);
		// Cinverter para strig
		char str_buffer[26];
		ctime_s(str_buffer, sizeof str_buffer, &now);
		std::string timestamp = std::string(str_buffer);
		timestamp = timestamp.substr(11, 8); // slice string to get only hour:minute:second

											 // Creating message
		int n = 54;
		char buff[54];
		snprintf(buff, sizeof(buff), "%06d/%06.1f/%06.1f/%06.1f/%05.1f/%05.1f/%04d/%s\n", NSEQ, TZONA1, TZONA2, TZONA3, VOLUME, PRESSAO, TEMPO, timestamp.c_str());
		std::string message(buff);

		// Salvando na lista circular em memória 1
		escreve_lista_circular1(message);
	}

	_endthreadex(0);
	return(0);
}


DWORD WINAPI ThreadLeituraPCP(int i) {
	std::cout << "Inside PCP Thread!" << std::endl;
	HANDLE hEvent;
	DWORD status;
	int NSEQ = 0;
	std::string OP1, OP2, OP3;
	int SLOT1, SLOT2, SLOT3;
	int size_of_OP_string = 8;
	srand((unsigned int)time(NULL));

	// Abre semáforo
	LONG dwContagemPrevia;
	hControla_leitura_pcp = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, "Controla_leitura_pcp");
	CheckForError(hControla_leitura_pcp);

	while (TRUE) {

		// Conquista semáforo
		status = WaitForSingleObject(hControla_leitura_pcp, INFINITE);	 // Verifica se pode executar
		CheckForError(status == WAIT_OBJECT_0);

		// Libera semáforo
		CheckForError(ReleaseSemaphore(hControla_leitura_pcp, 1, &dwContagemPrevia));

		// Temporização usando a temporização de WaitForSingleObject()
		int milisegundos = 1000 + rand() % 4001;  // Tempo entre 1000 e 5000 milisegundos
		hEvent = CreateEvent(NULL, TRUE, FALSE, "EvTimeout");
		CheckForError(hEvent);
		status = WaitForSingleObject(hEvent, milisegundos);
		if (status != WAIT_TIMEOUT) {
			std::cout << "Error in ThreadLeituraPCP Timeout !!!" << std::endl;
		}

		// Atribui variáveis
		NSEQ = (NSEQ + 1) % 10000;		 // Incrementa numero de sequencia em 1
		OP1 = genRandomString(size_of_OP_string);
		OP2 = genRandomString(size_of_OP_string);
		OP3 = genRandomString(size_of_OP_string);
		SLOT1 = rand() % 10000;				// Número entre 0000 e 9999
		SLOT2 = rand() % 10000;
		SLOT3 = rand() % 10000;
		// Data corrente
		time_t now = time(0);
		// convertendo para string
		char str_buffer[26];
		ctime_s(str_buffer, sizeof str_buffer, &now);
		std::string timestamp = std::string(str_buffer);
		timestamp = timestamp.substr(11, 8); // slice string to get only hour:minute:second

											 // Cria mensagem
		int n = 56;
		char buff[56];
		snprintf(buff, sizeof(buff), "%04d|%s|%s|%04d|%s|%04d|%s|%04d", NSEQ, timestamp.c_str(), OP1.c_str(), SLOT1, OP2.c_str(), SLOT2, OP3.c_str(), SLOT3);
		std::string message(buff);
		//std::cout << message << std::endl;

		// Salvando na lista circular em memória 1
		escreve_lista_circular1(message);
	}

	_endthreadex(0);
	return(0);
}

DWORD WINAPI ThreadCapturaDeMensagens(int i) {
	std::cout << "Inside Captura de Mensagens Thread!" << std::endl;
	std::string message;
	DWORD dwBytesWritten;
	HANDLE hPipe;
	OVERLAPPED overlap;
	// Aguarda um Pipe
	LPTSTR lpszPipename = "\\\\.\\pipe\\PipeGestaoDaProducao";
	WaitNamedPipe(lpszPipename, NMPWAIT_WAIT_FOREVER);

	// Conecta-se a um pipe
	while (TRUE) // Espera conexão
	{
		hPipe = CreateFile(
			lpszPipename,   // nome do pipe 
			GENERIC_READ |  // acesso para leitura e escrita 
			GENERIC_WRITE,
			0,              // sem compartilhamento 
			NULL,           // lpSecurityAttributes
			OPEN_EXISTING,  // dwCreationDistribution 
			FILE_FLAG_OVERLAPPED, // acesso assíncrono			//0,  // dwFlagsAndAttributes 
			NULL);          // hTemplate
							//CheckForError(hPipe != INVALID_HANDLE_VALUE);

							// Se o handle é válido pode usar o pipe
		if (hPipe != INVALID_HANDLE_VALUE)
			break;

		// Aguarda um pipe
		if (WaitNamedPipe(lpszPipename, NMPWAIT_WAIT_FOREVER) == 0)
			printf("\nEsperando por uma instancia do pipe..."); // Temporização abortada: o pipe ainda não foi criado
	}

	// Conecta com o Mailslot
	HANDLE hMailslot_gestao;
	hMailslot_gestao = CreateFile(
		"\\\\.\\mailslot\\MyMailslot_gestao",
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	CheckForError(hMailslot_gestao != INVALID_HANDLE_VALUE);

	// Abre semáforo
	DWORD status;
	LONG dwContagemPrevia;
	hControla_retirada_de_mensagens = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, "Controla_retirada_de_mensagens");
	CheckForError(hControla_retirada_de_mensagens);

	while (TRUE) {

		// Conquista semáforo
		status = WaitForSingleObject(hControla_retirada_de_mensagens, INFINITE);	 // Verifica se pode executar
		CheckForError(status == WAIT_OBJECT_0);

		// Libera semáforo
		CheckForError(ReleaseSemaphore(hControla_retirada_de_mensagens, 1, &dwContagemPrevia));

		// Capturando mensagens da lista circular 1
		message = le_lista_circular1();

		// Tratamento da mensagem
		if (message[6] == '/') {		// Mensagem de dado de processo
										// Salvando na lista circular em memória 2
			escreve_lista_circular2(message);
		}
		else if (message[4] == '|') {	// Mensagem de escalonamento de processo
										//std::cout << "Mensagem de escalonamento do processo" << std::endl;
										// Escreve no Pipe 
			char buffer[56];
			strcpy(buffer, message.c_str());
			overlap.OffsetHigh = 0;
			overlap.Offset = 56;
			overlap.hEvent = hPipeEvent;
			//WriteFile(hPipe, &buffer, sizeof(buffer), &dwBytesWritten, NULL);
			WriteFile(hPipe, &buffer, sizeof(buffer), &dwBytesWritten, &overlap);
			//std::cout << "Bytes escritos " << dwBytesWritten << std::endl;

			// Escreve no mailslot
			WriteFile(hMailslot_gestao, &buffer, sizeof(buffer), &dwBytesWritten, NULL);
			std::cout << "Bytes escritos " << dwBytesWritten << std::endl;

		}
		else {
			std::cout << "CAPTURA DE MENSAGEM FALHOU!!! MENSAGEM CORROMPIDA " << std::endl;
		}
	}

	// Fecha o Pipe
	FlushFileBuffers(hPipe);
	DisconnectNamedPipe(hPipe);

	// Fecha handles
	CloseHandle(hPipe);

	_endthreadex(0);
	return(0);
}


DWORD WINAPI ThreadExibicaoDeDados(int i) {
	std::cout << "Inside Exibicao de Dados Thread!" << std::endl;
	std::string message;
	std::string NSEQ, TIMESTAMP, TZ1, TZ2, TZ3, V, P, TEMPO, full_console_message;

	// Abre semáforo
	DWORD status;
	LONG dwContagemPrevia;
	hControla_sistema_de_exibicao_de_dados = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, "Controla_sistema_de_exibicao_de_dados");
	CheckForError(hControla_sistema_de_exibicao_de_dados);

	while (TRUE) {

		// Conquista semáforo
		status = WaitForSingleObject(hControla_sistema_de_exibicao_de_dados, INFINITE);	 // Verifica se pode executar
		CheckForError(status == WAIT_OBJECT_0);

		// Libera semáforo
		CheckForError(ReleaseSemaphore(hControla_sistema_de_exibicao_de_dados, 1, &dwContagemPrevia));

		//Retira mensagens da lista circular
		message = le_lista_circular2();

		// Extrai campos da mensagem
		NSEQ = message.substr(0, 6);
		TZ1 = message.substr(7, 6);
		TZ2 = message.substr(14, 6);
		TZ3 = message.substr(21, 6);
		V = message.substr(28, 5);
		P = message.substr(34, 5);
		TEMPO = message.substr(40, 4);
		TIMESTAMP = message.substr(45, 8);

		// Cria mensagem de console
		full_console_message = "NSEQ:" + NSEQ + " "
			+ TIMESTAMP + " "
			+ "TZ1:" + TZ1 + " "
			+ "TZ2:" + TZ2 + " "
			+ "TZ3:" + TZ3 + " "
			+ "V:" + V + " "
			+ "P:" + P + " "
			+ "TEMPO: " + TEMPO;

		// Exibe mensagem no console
		std::cout << full_console_message << std::endl;
	}

	_endthreadex(0);
	return(0);
}