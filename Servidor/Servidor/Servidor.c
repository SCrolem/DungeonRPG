#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>


#define PIPE_N_READ TEXT("\\\\.\\pipe\\ParaServidor")
#define PIPE_N_WRITE TEXT("\\\\.\\pipe\\ParaCliente")


#define N_MAX_LEITORES 10
#define TAM 256

HANDLE PipeLeitores[N_MAX_LEITORES];
int total = 0;
BOOL fim = FALSE;

DWORD WINAPI RecebeLeitores(LPVOID param);
DWORD WINAPI AtendeCliente(LPVOID param);


int _tmain(int argc, LPTSTR argv[]) {
	DWORD n;
	HANDLE hThread;
	TCHAR buf[TAM];

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif
	//Invocar a thread que inscreve novos leitores
	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RecebeLeitores, NULL, 0, NULL);

	/*do {
		_tprintf(TEXT("[ESCRITOR] Frase: "));
		_fgetts(buf, 256, stdin);
		//Escrever para todos os leitores inscritos
		for (int i = 0; i < total; i++)
			if (!WriteFile(PipeLeitores[i], buf, _tcslen(buf) * sizeof(TCHAR), &n, NULL)) {
				_tperror(TEXT("[ERRO] Escrever no pipe... (WriteFile)\n"));
				exit(-1);
			}

		_tprintf(TEXT("[Servidor] Enviei %d bytes aos %d Clientes... (WriteFile)\n"), n, total);
	} while (_tcsncmp(buf, TEXT("fim"), 3));
	fim = TRUE;*/

	//Esperar a thread recebeLeitores terminar
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	exit(0);
}

DWORD WINAPI RecebeLeitores(LPVOID param) {
	HANDLE hPipe;

	while (!fim && total < N_MAX_LEITORES) 
	{
		_tprintf(TEXT("[Servidor] Vou passar à criação de uma cópia do pipe '%s' ... (CreateNamedPipe)\n"), PIPE_N_WRITE);


		PipeLeitores[total] = CreateNamedPipe(PIPE_N_WRITE, PIPE_ACCESS_OUTBOUND, PIPE_WAIT | PIPE_TYPE_MESSAGE
			| PIPE_READMODE_MESSAGE, N_MAX_LEITORES, TAM * sizeof(TCHAR), TAM * sizeof(TCHAR),
			1000, NULL);
		if (PipeLeitores[total] == INVALID_HANDLE_VALUE) {
			_tperror(TEXT("Erro na ligação ao leitor!"));
			exit(-1);
		}

		hPipe = CreateNamedPipe(PIPE_N_READ, PIPE_ACCESS_INBOUND, PIPE_WAIT | PIPE_TYPE_MESSAGE
			| PIPE_READMODE_MESSAGE, N_MAX_LEITORES, TAM * sizeof(TCHAR), TAM * sizeof(TCHAR),
			1000, NULL);

		if (hPipe == INVALID_HANDLE_VALUE) {
			_tperror(TEXT("Erro na ligação ao leitor!"));
			exit(-1);
		}
		


		_tprintf(TEXT("[Servidor] Esperar ligação de um leitor... (ConnectNamedPipe)\n"));

		if (!ConnectNamedPipe(PipeLeitores[total], NULL)) {
			_tperror(TEXT("Erro na ligação ao leitor!"));
			exit(-1);
		}

		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AtendeCliente, (LPVOID)hPipe, 0, NULL);

		total++;
	}

	for (int i = 0; i < total; i++) {
		DisconnectNamedPipe(PipeLeitores[i]);
		_tprintf(TEXT("[Servidor] Vou desligar o pipe... (CloseHandle)\n"));
		CloseHandle(PipeLeitores[i]);
	}
	return 0;
}

DWORD WINAPI AtendeCliente(LPVOID param)
{
	TCHAR buf[TAM];
	DWORD n;
	BOOL ret;
	HANDLE hPipe = (HANDLE)param;



	





	while (1)
	{    
		_tprintf(TEXT("[Servidor] a espera de receber algo do CLIENTE"));
		//ler do pipe do seu cliente
		ret = ReadFile(hPipe, buf, TAM, &n, NULL);
		buf[n / sizeof(TCHAR)] = '\0';

		if (!ret || !n)
			break;
		_tprintf(TEXT("[Servidor] recebi %d bytes: '%s'... (ReadFile)\n"), n, buf);

		//escrever para todos
		for (int i = 0; i < total; i++)
			WriteFile(PipeLeitores[i], buf, _tcslen(buf) * sizeof(TCHAR), &n, NULL);


	}
	return 0;
}
