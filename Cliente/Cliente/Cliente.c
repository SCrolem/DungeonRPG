#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "dll.h"


#define PIPE_N_WRITE TEXT("\\\\.\\pipe\\ParaServidor")
#define PIPE_N_READ TEXT("\\\\.\\pipe\\ParaCliente")

DWORD WINAPI RecebeDoServidor(LPVOID param);

int _tmain(int argc, LPTSTR argv[]) {
	TCHAR buf[256];
	HANDLE wPipe, rPipe;
	int i = 0;
	BOOL ret;
	DWORD n;
	HANDLE hThread;
	
#ifdef UNICODE 
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif
	/*
	_tprintf(TEXT("[CLIENTE]Esperar pelo pipe '%s'(WaitNamedPipe)\n"), PIPE_N_WRITE);

	if (!WaitNamedPipe(PIPE_N_WRITE, NMPWAIT_WAIT_FOREVER)) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'... (WaitNamedPipe)\n"), PIPE_N_WRITE);
		exit(-1);
	}
	_tprintf(TEXT("[CLIENTE] Ligação ao SERVIDOR...\n"));
	
	wPipe = CreateFile(PIPE_N_WRITE, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); //cria ligaçao	
	if (wPipe == NULL) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'... (CreateFile)\n"), PIPE_N_WRITE);
		exit(-1);
	}
	_tprintf(TEXT("[CLIENTE] Pipe para escrita estabelecido...\n"));


	rPipe = CreateFile(PIPE_N_READ, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);	
	if (rPipe == NULL) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'... (CreateFile)\n"), PIPE_N_READ);
		exit(-1);
	}
	_tprintf(TEXT("[CLIENTE] Pipe para leitura estabelecido...\n"));
*/

	Inicia_comunicacao();
	//Invocar a thread que recebe info do servidor
	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RecebeDoServidor, (LPVOID)rPipe, 0, NULL);	
	if (hThread == NULL) {
		_tprintf(TEXT("[ERRO] Thread não foi lançada\n"));
		exit(-1);
	}
	_tprintf(TEXT("[CLIENTE]Criei a Thread para receber do servidor...\n"));

	while (1)
	{
		Sleep(200); 
		_tprintf(TEXT("[CLIENTE] Frase: "));
		_fgetts(buf, 256, stdin);

		ret = WriteFile(wPipe, buf, _tcslen(buf) * sizeof(TCHAR), &n, NULL);
		buf[n / sizeof(TCHAR)] = '\0';
		
		if (!ret || !n)
			break;
		_tprintf(TEXT("[CLIENTE](ReadFile) enviei %d bytes: %s "), n, buf);

		if (!_tcsncmp(buf, TEXT("fim"), 3)) 
		{
			break;
		}
	}

	//CloseHandle(wPipe);
	//CloseHandle(rPipe);
	CloseHandle(hThread);
	Sleep(200);
	return 0;
}

DWORD WINAPI RecebeDoServidor(LPVOID param) { //recebe o pipe 
	HANDLE rPipe = (HANDLE)param; //atribui o pipe que recebe (pipe )
	TCHAR buf[256];
	int i = 0;
	BOOL ret;
	DWORD n;

		while (1) 
		{
			ret = ReadFile(rPipe, buf, sizeof(buf), &n, NULL);
			buf[n / sizeof(TCHAR)] = '\0';
			if (!ret || !n)
				break;

			_tprintf(TEXT("[CLIENTE](ReadFile) Recebi %d bytes: %s "), n, buf);
		}
		
	return 0;
}