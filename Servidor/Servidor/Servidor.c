#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>


#define PIPE_N_READ TEXT("\\\\.\\pipe\\ParaServidor")
#define PIPE_N_WRITE TEXT("\\\\.\\pipe\\ParaCliente")


#define N_MAX_CLIENTES 10
#define TAM 256

HANDLE wPipeClientes[N_MAX_CLIENTES];
int total = 0;
BOOL sair = FALSE;

DWORD WINAPI RecebeClientes(LPVOID param);
DWORD WINAPI AtendeCliente(LPVOID param);


int _tmain(int argc, LPTSTR argv[]) {
	HANDLE hThread;	

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	//Invocar a thread que inscreve novos clientes
	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RecebeClientes, NULL, 0, NULL);
	if (hThread == NULL) {
		_tprintf(TEXT("[ERRO] Thread não foi lançada\n"));
		exit(-1);
	}


	//Esperar a thread recebeLeitores terminar
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	exit(0);
}

DWORD WINAPI RecebeClientes(LPVOID param) {
	HANDLE rPipe;
	int i;
	HANDLE ThreadAtendeCli[N_MAX_CLIENTES];

	while (!sair && total < N_MAX_CLIENTES)
	{
		_tprintf(TEXT("[Servidor](CreateNamedPipe) Copiar pipe '%s' \n"), PIPE_N_WRITE);

		wPipeClientes[total] = CreateNamedPipe(PIPE_N_WRITE, PIPE_ACCESS_OUTBOUND, PIPE_WAIT | PIPE_TYPE_MESSAGE
			| PIPE_READMODE_MESSAGE, N_MAX_CLIENTES, TAM * sizeof(TCHAR), TAM * sizeof(TCHAR),
			1000, NULL);		

		_tprintf(TEXT("[Servidor](CreateNamedPipe) Copiar pipe '%s' \n"), PIPE_N_READ);

		rPipe = CreateNamedPipe(PIPE_N_READ, PIPE_ACCESS_INBOUND, PIPE_WAIT | PIPE_TYPE_MESSAGE
			| PIPE_READMODE_MESSAGE, N_MAX_CLIENTES, TAM * sizeof(TCHAR), TAM * sizeof(TCHAR),
			1000, NULL);

		//verifica pipes
		if ((rPipe == INVALID_HANDLE_VALUE) && (wPipeClientes[total] == INVALID_HANDLE_VALUE)) {
			_tperror(TEXT("[ERRO] Não são permitidos mais clientes!"));
			exit(-1);
		}		

		_tprintf(TEXT("[Servidor] Esperar ligação de um cliente... (ConnectNamedPipe)\n"));

		if (!ConnectNamedPipe(wPipeClientes[total], NULL)) {
			_tperror(TEXT("Erro na ligação ao Cliente!"));
			exit(-1);
		}
		

		ThreadAtendeCli[total++] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AtendeCliente, (LPVOID)rPipe, 0, NULL);
		if (ThreadAtendeCli == NULL) {
			_tprintf(TEXT("[ERRO] ThreadAtendeCli não foi lançada\n"));
			exit(-1);
		}

		//total++;
	}

	/*
	//espera que todos os clientes saiam
	for (i = 0; i < total; i++) {
		WaitForSingleObject(ThreadAtendeCli[i], INFINITE);
	}*/


	for (i = 0; i < total; i++) {
		DisconnectNamedPipe(wPipeClientes[i]);
		_tprintf(TEXT("[Servidor] Vou desligar o pipe... (CloseHandle)\n"));
		CloseHandle(wPipeClientes[i]);
		CloseHandle(ThreadAtendeCli[i]);
	}	
	return 0;
}

DWORD WINAPI AtendeCliente(LPVOID param)
{
	TCHAR buf[TAM];
	DWORD n;
	BOOL ret;
	HANDLE rPipe = (HANDLE)param;

	while (1)
	{    
		_tprintf(TEXT("[Servidor] a espera de resposta de cliente...\n"));
		//ler do pipe do seu cliente
		ret = ReadFile(rPipe, buf, TAM, &n, NULL);
		buf[n / sizeof(TCHAR)] = '\0';

		if (!ret || !n)
			break;
		_tprintf(TEXT("[Servidor](ReadFile) recebi %d bytes: %s \n"), n, buf);

		//escrever para todos
		for (int i = 0; i < total; i++)
			WriteFile(wPipeClientes[i], buf, _tcslen(buf) * sizeof(TCHAR), &n, NULL);

	}
	return 0;
}
