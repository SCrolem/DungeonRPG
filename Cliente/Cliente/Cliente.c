#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>


#define PIPE_N_WRITE TEXT("\\\\.\\pipe\\envia")
#define PIPE_N_READ TEXT("\\\\.\\pipe\\recebe")

int _tmain(int argc, LPTSTR argv[]) {
	TCHAR buf[256];
	HANDLE hPipe;
	int i = 0;
	BOOL ret;
	DWORD n;
	HANDLE hTread;
	
#ifdef UNICODE 
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif
	

	_tprintf(TEXT("[CLIENTE]Esperar pelo pipe '%s'(WaitNamedPipe)\n"), PIPE_N_WRITE);
	if (!WaitNamedPipe(PIPE_N_WRITE, NMPWAIT_WAIT_FOREVER)) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'... (WaitNamedPipe)\n"), PIPE_N_WRITE);
		exit(-1);
	}

	hTread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RecebeDoServidor, NULL, 0, NULL);

	_tprintf(TEXT("[CLIENTE] Ligação ao SERVIDOR... (CreateFile)\n"));
	hPipe = CreateFile(PIPE_N_WRITE, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hPipe == NULL) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'... (CreateFile)\n"), PIPE_N_WRITE);
		exit(-1);
	}
	_tprintf(TEXT("[CLIENTE]Liguei-me...\n"));
	while (1) {
		ret = ReadFile(hPipe, buf, sizeof(buf), &n, NULL);
		buf[n / sizeof(TCHAR)] = '\0';
		if (!ret || !n)
			break;
		_tprintf(TEXT("[CLIENTE] Recebi %d bytes: '%s'... (ReadFile)\n"), n, buf);
	}
	CloseHandle(hPipe);

	Sleep(200);
	return 0;
}

DWORD WINAPI RecebeDoServidor(LPVOID param) {
	HANDLE hPipe;
	TCHAR buf[256];
	int i = 0;
	BOOL ret;
	DWORD n;

	while (1) {
		hPipe = CreateFile(PIPE_N_READ, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hPipe == NULL) {
			_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'... (CreateFile)\n"), PIPE_N_READ);
			exit(-1);
		}
		_tprintf(TEXT("[LEITOR]Liguei-me...\n"));
		while (1) {
			ret = ReadFile(hPipe, buf, sizeof(buf), &n, NULL);
			buf[n / sizeof(TCHAR)] = '\0';
			if (!ret || !n)
				break;
			_tprintf(TEXT("[LEITOR] Recebi %d bytes: '%s'... (ReadFile)\n"), n, buf);
		}
	
	}
	return 0;


}
