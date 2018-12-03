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
#define _CHECKERROR	1	// Ativa fun��o CheckForError
#include "CheckForError.h"

// Handle do semaforo de bloqueio da thread
HANDLE hControla_sistema_de_gestao;

// Tarefa de gest�o da produ��o
int main(){	
	std::string TIMESTAMP, OP1, OP2, OP3, SLOT1, SLOT2, SLOT3, full_console_message;
	
	// Cria um pipe de uma inst�ncia - Servidor de um pipe nomeado
	HANDLE hPipe;
	hPipe = CreateNamedPipe(
		"\\\\.\\pipe\\PipeGestaoDaProducao",
		PIPE_ACCESS_DUPLEX,	// Comunica��o Full Duplex
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, // Opera��es s�ncronas
		1,			// N�mero de inst�ncias
		0,			// nOutBufferSize
		0,			// nInBufferSize
		NMPWAIT_WAIT_FOREVER,		// Sem timeout
		NULL);		// Atributos de seguran�a
	CheckForError(hPipe != INVALID_HANDLE_VALUE);
	if (hPipe == INVALID_HANDLE_VALUE) {
		std::cout << "Erro, nao foi possivel criar o pipe" << std::endl;
		//exit(0);
	}
	else {
		std::cout << "Pipe criado com sucesso" << std::endl;
	}

	// Aguarda conex�o do cliente
	int bStatus;
	DWORD dwErrorCode;	// C�digo de erro retornado por GetLastError()
	bStatus = ConnectNamedPipe(hPipe, NULL);
	if (bStatus) {
		printf("Cliente se conectou com sucesso\n");
	}
	else {
		dwErrorCode = GetLastError();
		if (dwErrorCode == ERROR_PIPE_CONNECTED) {
			printf("Cliente j� havia se conectado\n");
		} 
		else if (dwErrorCode == ERROR_NO_DATA) {
			printf("Cliente fechou seu handle\n");
			return 0;
		} 
		else {
			printf("Erro desconhecido\n");
			CheckForError(FALSE);
		}
	}
	std::cout << "Conectou!!" << std::endl;

	// Abre sem�foro
	DWORD status;
	LONG dwContagemPrevia;
	hControla_sistema_de_gestao = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, "Controla_sistema_de_gestao");
	CheckForError(hControla_sistema_de_gestao);

	while (TRUE) {
		// Conquista sem�foro
		status = WaitForSingleObject(hControla_sistema_de_gestao, INFINITE);	 // Verifica se pode executar
		CheckForError(status == WAIT_OBJECT_0);

		// Libera sem�foro
		CheckForError(ReleaseSemaphore(hControla_sistema_de_gestao, 1, &dwContagemPrevia));

		// L� dados do cliente
		DWORD dwBytesRead;
		char buffer[56];
		bStatus = ReadFile(hPipe, &buffer, 56, &dwBytesRead, NULL);
		CheckForError(bStatus);
		std::string message(buffer);

		// Extrai campos da mensagem
		TIMESTAMP = message.substr(5, 8);
		OP1 = message.substr(14, 8);
		SLOT1 = message.substr(23, 4);
		OP2 = message.substr(28, 8);
		SLOT2 = message.substr(37, 4);
		OP3 = message.substr(42, 8);
		SLOT3 = message.substr(51, 4);

		// Constroi mensagem
		full_console_message = "REF:" + TIMESTAMP + " "
							   + "OP1:" + OP1 + " [" + SLOT1 + "]" + " "
							   + "OP2:" + OP2 + " [" + SLOT2 + "]" + " "
							   + "OP3:" + OP2 + " [" + SLOT2 + "]" + " ";

		// Exibe mensagem no console
		std::cout << full_console_message << std::endl;

	}


	// Fecha handles
	CloseHandle(hPipe);
	CloseHandle(hControla_sistema_de_gestao);

	system("pause");
    return 0;
}

