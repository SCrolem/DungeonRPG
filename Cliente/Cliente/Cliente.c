#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>


#define PIPE_N_WRITE TEXT("\\\\.\\pipe\\ParaServidor")
#define PIPE_N_READ TEXT("\\\\.\\pipe\\ParaCliente")

DWORD WINAPI RecebeDoServidor(LPVOID param);

int _tmain(int argc, LPTSTR argv[]) {
	TCHAR buf[256];
	HANDLE hPipe, hPipe2;
	int i = 0;
	BOOL ret;
	DWORD n;
	HANDLE hThread;
	
#ifdef UNICODE 
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	_tprintf(TEXT("[CLIENTE]Esperar pelo pipe '%s'(WaitNamedPipe)\n"), PIPE_N_WRITE);

	if (!WaitNamedPipe(PIPE_N_WRITE, NMPWAIT_WAIT_FOREVER)) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'... (WaitNamedPipe)\n"), PIPE_N_WRITE);
		exit(-1);
	}

	_tprintf(TEXT("[CLIENTE] Ligação ao SERVIDOR... (CreateFile)\n"));
	hPipe = CreateFile(PIPE_N_WRITE, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); //cria ligaçao
	
	if (hPipe == NULL) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'... (CreateFile)\n"), PIPE_N_WRITE);
		exit(-1);
	}

	hPipe2 = CreateFile(PIPE_N_READ, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hPipe2 == NULL) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'... (CreateFile)\n"), PIPE_N_READ);
		exit(-1);
	}


	//Invocar a thread que recebe info do servidor
	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RecebeDoServidor, (LPVOID)hPipe2, 0, NULL);
	//Sleep(200);
	//_tprintf(TEXT("[CLIENTE]Liguei-me...\n"));

	while (1) 
	{
		_tprintf(TEXT("[Cliente] Frase: "));
		_fgetts(buf, 256, stdin);

		ret = WriteFile(hPipe, buf, _tcslen(buf) * sizeof(TCHAR), &n, NULL);
		//buf[n / sizeof(TCHAR)] = '\0';
		
		if (!ret || !n)
			break;
		_tprintf(TEXT("[CLIENTE] enviei %d bytes: '%s'... (ReadFile)\n"), n, buf);


	//:::::::::::::::::::::::::::::::::::
		/*
		ret = ReadFile(hPipe2, buf, sizeof(buf), &n, NULL);
		buf[n / sizeof(TCHAR)] = '\0';
		if (!ret || !n)
			break;

		_tprintf(TEXT("[Cliente] Recebi %d bytes: '%s'... (ReadFile)\n"), n, buf);*/

	}
	CloseHandle(hPipe);

	Sleep(200);
	return 0;
}

DWORD WINAPI RecebeDoServidor(LPVOID param) {
	HANDLE hPipe = (HANDLE)param;
	TCHAR buf[256];
	int i = 0;
	BOOL ret;
	DWORD n;

	
		//hPipe = CreateFile(PIPE_N_READ, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		
		//if (hPipe == NULL) {
		//	_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'... (CreateFile)\n"), PIPE_N_READ);
		//	exit(-1);
		//}

		//_tprintf(TEXT("[Cliente]Liguei-me...\n"));

		while (1) 
		{
			ret = ReadFile(hPipe, buf, sizeof(buf), &n, NULL);
			buf[n / sizeof(TCHAR)] = '\0';
			if (!ret || !n)
				break;

			_tprintf(TEXT("[Cliente] Recebi %d bytes: '%s'... (ReadFile)\n"), n, buf);
		}
	
	CloseHandle(hPipe);
	return 0;
}