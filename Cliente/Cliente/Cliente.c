#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>


#define PIPE_NAME1 TEXT("\\\\.\\pipe\\envia")
#define PIPE_NAME2 TEXT("\\\\.\\pipe\\recebe")

int _tmain(int argc, LPTSTR argv[]) {
	TCHAR buf[256];
	HANDLE hPipe;
	int i = 0;
	BOOL ret;
	DWORD n;
#ifdef UNICODE 
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	_tprintf(TEXT("[LEITOR]Esperar pelo pipe '%s'(WaitNamedPipe)\n"), PIPE_NAME1);
	
	if (!WaitNamedPipe(PIPE_NAME1, NMPWAIT_WAIT_FOREVER)) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'... (WaitNamedPipe)\n"), PIPE_NAME1);
		exit(-1);
	}

	_tprintf(TEXT("[LEITOR] Ligação ao escritor... (CreateFile)\n"));

	hPipe = CreateFile(PIPE_NAME1, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hPipe == NULL) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'... (CreateFile)\n"), PIPE_NAME1);
		exit(-1);
	}
	_tprintf(TEXT("[LEITOR]Liguei-me...\n"));
	while (1) 
	{   
		_tprintf(TEXT("[ESCRITOR] Frase: "));
		_fgetts(buf, 256, stdin);


		ret = ReadFile(hPipe, buf, sizeof(buf), &n, NULL);
		buf[n / sizeof(TCHAR)] = '\0';
		if (!ret || !n)
			break;
		_tprintf(TEXT("[LEITOR] Recebi %d bytes: '%s'... (ReadFile)\n"), n, buf);
	}
	CloseHandle(hPipe);

	Sleep(200);
	return 0;
}
