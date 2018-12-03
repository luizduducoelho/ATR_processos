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
#define	ESC				0x1B			//Tecla para encerrar o programa

// Declaração dos Handles para os semáforos
HANDLE hControla_leitura_clp;
HANDLE hControla_leitura_pcp;
HANDLE hControla_retirada_de_mensagens;
HANDLE hControla_sistema_de_gestao;
HANDLE hControla_sistema_de_exibicao_de_dados;

// Evento de Esc
HANDLE hEscEvent;

// Tarefa de leitura do teclado
int main() {

	// Abre semáforo
	DWORD status;
	LONG dwContagemPrevia;
	hControla_leitura_clp = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, "Controla_leitura_clp");
	CheckForError(hControla_leitura_clp);
	hControla_leitura_pcp = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, "Controla_leitura_pcp");
	CheckForError(hControla_leitura_pcp);
	hControla_retirada_de_mensagens = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, "Controla_retirada_de_mensagens");
	CheckForError(hControla_retirada_de_mensagens);
	hControla_sistema_de_gestao = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, "Controla_sistema_de_gestao");
	CheckForError(hControla_sistema_de_gestao);
	hControla_sistema_de_exibicao_de_dados = OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, "Controla_sistema_de_exibicao_de_dados");
	CheckForError(hControla_sistema_de_exibicao_de_dados);

	// Variáveis para controle das threads
	bool leitura_clp_ligada = TRUE;
	bool leitura_pcp_ligada = TRUE;
	bool retirada_de_mensagens_ligada = TRUE;
	bool sistema_de_gestao_ligada = TRUE;
	bool sistema_de_exibicao_de_dados_clp_ligada = TRUE;

	// Conecta com o Mailslot
	HANDLE hMailslot;
	DWORD dwBytesWritten;
	hMailslot = CreateFile(
		"\\\\.\\mailslot\\MyMailslot",
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	CheckForError(hMailslot != INVALID_HANDLE_VALUE);
	
	// Abre evento de Esc
	hEscEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, "EscEvent");
	CheckForError(hEscEvent);

	int nTecla;    // Guarda a tecla digitada pelo usuário
	while (TRUE) {
		//  Lê a tecla do usuário 
		nTecla = _getch();
		//std::cout << "nTecla: " << nTecla << std::endl;

		// Controla leitura do CLP
		if (nTecla == 'p') {
			if (leitura_clp_ligada) {
				status = WaitForSingleObject(hControla_leitura_clp, INFINITE);  // Bloqueia Thread
				CheckForError(status == WAIT_OBJECT_0);
				leitura_clp_ligada = FALSE;
				std::cout << "Desliga leitura do CLP" << std::endl;
			}
			else {
				CheckForError(ReleaseSemaphore(hControla_leitura_clp, 1, &dwContagemPrevia));
				leitura_clp_ligada = TRUE;
				std::cout << "Liga leitura do CLP" << std::endl;
			}
		}
		// Controle leitura de PCP
		else if (nTecla == 's') {
			if (leitura_pcp_ligada) {
				status = WaitForSingleObject(hControla_leitura_pcp, INFINITE);  // Bloqueia Thread
				CheckForError(status == WAIT_OBJECT_0);
				leitura_pcp_ligada = FALSE;
				std::cout << "Desliga leitura do PCP" << std::endl;
			}
			else {
				CheckForError(ReleaseSemaphore(hControla_leitura_pcp, 1, &dwContagemPrevia));
				leitura_pcp_ligada = TRUE;
				std::cout << "Liga leitura do PCP" << std::endl;
			}
		}
		// Controle retirada de mensagens
		else if (nTecla == 'r') {
			if (retirada_de_mensagens_ligada) {
				status = WaitForSingleObject(hControla_retirada_de_mensagens, INFINITE);  // Bloqueia Thread
				CheckForError(status == WAIT_OBJECT_0);
				retirada_de_mensagens_ligada = FALSE;
				std::cout << "Desliga retirada de mensagens" << std::endl;
			}
			else {
				CheckForError(ReleaseSemaphore(hControla_retirada_de_mensagens, 1, &dwContagemPrevia));
				retirada_de_mensagens_ligada = TRUE;
				std::cout << "Liga retirada de mensagens" << std::endl;
			}
		}
		// Controle sistema de gestao da producao
		else if (nTecla == 'g') {
			if (sistema_de_gestao_ligada) {
				status = WaitForSingleObject(hControla_sistema_de_gestao, INFINITE);  // Bloqueia Thread
				CheckForError(status == WAIT_OBJECT_0);
				sistema_de_gestao_ligada = FALSE;
				std::cout << "Desliga sistema de gestao da producao" << std::endl;
			}
			else {
				CheckForError(ReleaseSemaphore(hControla_sistema_de_gestao, 1, &dwContagemPrevia));
				sistema_de_gestao_ligada = TRUE;
				std::cout << "Liga sistema de gestao da producao" << std::endl;
			}
		}
		// Controle sistema de exibição de dados
		else if (nTecla == 'd') {
			if (sistema_de_exibicao_de_dados_clp_ligada) {
				status = WaitForSingleObject(hControla_sistema_de_exibicao_de_dados, INFINITE);  // Bloqueia Thread
				CheckForError(status == WAIT_OBJECT_0);
				sistema_de_exibicao_de_dados_clp_ligada = FALSE;
				std::cout << "Desliga sistema de exibicao de dados do clp" << std::endl;
			}
			else {
				CheckForError(ReleaseSemaphore(hControla_sistema_de_exibicao_de_dados, 1, &dwContagemPrevia));
				sistema_de_exibicao_de_dados_clp_ligada = TRUE;
				std::cout << "Liga sistema de exibicao de dados do clp" << std::endl;
			}
		}
		// Limpeza de tela gestão da produção
		else if (nTecla == 'c') {
			std::cout << "Limpa tela do sistema de gestao da producao" << std::endl;
			// Envia mensagem ao mailslot
			char mailslot_buffer[] = "Clear";
			WriteFile(hMailslot, &mailslot_buffer, sizeof(mailslot_buffer), &dwBytesWritten, NULL);

		}
		// Encerra todas as tarefas
		else if (nTecla == ESC) {
			std::cout << "Encerra todas as tarefas" << std::endl;
			SetEvent(hEscEvent);
		}
		// Tecla invalida
		else {
			std::cout << "Tecla invalida! Digite {p, s, r, g, d, c, ESC}" << std::endl;
		}

		// Imprime status na tela 
		std::cout << "**Leitura CLP***Leitura PCP***Retirada de mensagens***Sistema de gestao***Exibicao de Dados**" << std::endl;
		std::cout << "** " << leitura_clp_ligada << "         "
			<< "*** " << leitura_pcp_ligada << "         "
			<< "*** " << retirada_de_mensagens_ligada << "                   "
			<< "*** " << sistema_de_gestao_ligada << "               "
			<< "*** " << sistema_de_exibicao_de_dados_clp_ligada << "                **" << std::endl << std::endl;
	}

	// Fecha handles
	CloseHandle(hMailslot);
	CloseHandle(hControla_leitura_clp);
	CloseHandle(hControla_leitura_pcp);
	CloseHandle(hControla_retirada_de_mensagens);
	CloseHandle(hControla_sistema_de_gestao);
	CloseHandle(hControla_sistema_de_exibicao_de_dados);
	CloseHandle(hEscEvent);

	return 0;
}