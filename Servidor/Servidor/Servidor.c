#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>


#define PIPE_NAME1 TEXT("\\\\.\\pipe\\envia")
#define PIPE_NAME2 TEXT("\\\\.\\pipe\\recebe")


#define N_MAX_LEITORES 10
#define TAM 256

HANDLE PipeLeitores[N_MAX_LEITORES];
int total;
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

	do {
		_tprintf(TEXT("[ESCRITOR] Frase: "));
		_fgetts(buf, 256, stdin);
		//Escrever para todos os leitores inscritos
		for (int i = 0; i < total; i++)
			if (!WriteFile(PipeLeitores[i], buf, _tcslen(buf) * sizeof(TCHAR), &n, NULL)) {
				_tperror(TEXT("[ERRO] Escrever no pipe... (WriteFile)\n"));
				exit(-1);
			}

		_tprintf(TEXT("[ESCRITOR] Enviei %d bytes aos %d leitores... (WriteFile)\n"), n, total);
	} while (_tcsncmp(buf, TEXT("fim"), 3));
	fim = TRUE;

	//Esperar a thread recebeLeitores terminar
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	exit(0);
}

DWORD WINAPI RecebeLeitores(LPVOID param) {
	HANDLE hPipe;

	while (!fim && total < N_MAX_LEITORES) {
		_tprintf(TEXT("[ESCRITOR] Vou passar à criação de uma cópia do pipe '%s' e '%s' ... (CreateNamedPipe)\n"), PIPE_NAME1, PIPE_NAME2);


		PipeLeitores[total] = CreateNamedPipe(PIPE_NAME1, PIPE_ACCESS_OUTBOUND, PIPE_WAIT | PIPE_TYPE_MESSAGE
			| PIPE_READMODE_MESSAGE, N_MAX_LEITORES, TAM * sizeof(TCHAR), TAM * sizeof(TCHAR),
			1000, NULL);
		if (PipeLeitores[total] == INVALID_HANDLE_VALUE) {
			_tperror(TEXT("Erro na ligação ao leitor!"));
			exit(-1);
		}


		hPipe = CreateNamedPipe(PIPE_NAME2, PIPE_ACCESS_INBOUND, PIPE_WAIT | PIPE_TYPE_MESSAGE
			| PIPE_READMODE_MESSAGE, N_MAX_LEITORES, TAM * sizeof(TCHAR), TAM * sizeof(TCHAR),
			1000, NULL);

		if (hPipe == INVALID_HANDLE_VALUE) {
			_tperror(TEXT("Erro na ligação ao leitor!"));
			exit(-1);
		}


		_tprintf(TEXT("[ESCRITOR] Esperar ligação de um leitor... (ConnectNamedPipe)\n"));
		if (!ConnectNamedPipe(PipeLeitores[total], NULL)) {
			_tperror(TEXT("Erro na ligação ao leitor!"));
			exit(-1);
		}

		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AtendeCliente, (LPVOID)PipeLeitores[total], 0, NULL);

		total++;
	}
	for (int i = 0; i < total; i++) {
		DisconnectNamedPipe(PipeLeitores[i]);
		_tprintf(TEXT("[ESCRITOR] Vou desligar o pipe... (CloseHandle)\n"));
		CloseHandle(PipeLeitores[i]);
	}
	return 0;
}

DWORD WINAPI AtendeCliente(LPVOID param)
{
	TCHAR buf[TAM];
	DWORD n;
	HANDLE pipe = (HANDLE)param;

	while (1)
	{
		//ler do pipe do seu cliente
		ReadFile(pipe, buf, TAM, &n, NULL);
		//escrever para todos
		for (int i = 0; i < total; i++)
			WriteFile(PipeLeitores[i], buf, _tcslen(buf) * sizeof(TCHAR), &n, NULL);


	}
	return 0;
}
