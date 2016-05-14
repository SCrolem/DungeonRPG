//DLL.cpp
#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "dll.h"


HANDLE wPipe;
HANDLE rPipe;
//Exportar a função para ser utilizada fora da DLL

void Inicia_comunicacao() {
	DWORD n;

#ifdef UNICODE 
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif
	
	_tprintf(TEXT("[CLIENTE]Esperar pelo pipe '%s'(WaitNamedPipe)\n"), PIPE_N_WRITE);

	//verifica se há pipe para escrita
	if (!WaitNamedPipe(PIPE_N_WRITE, NMPWAIT_WAIT_FOREVER)) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'... (WaitNamedPipe)\n"), PIPE_N_WRITE);
		exit(-1);
	}	

    //tenta connectar-se ao 1ºpipe (para escrita)
	wPipe = CreateFile(PIPE_N_WRITE, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); //cria ligaçao	
	if (wPipe == NULL) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'... (CreateFile)\n"), PIPE_N_WRITE);
		exit(-1);
	}
	_tprintf(TEXT("[CLIENTE] Pipe para escrita estabelecido...\n"));
	
	Sleep(100); // para o servidor ter tempo de criar o pipe

	//verifica se há pipe para leitura
	if (!WaitNamedPipe(PIPE_N_READ, NMPWAIT_WAIT_FOREVER)) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'... (WaitNamedPipe)\n"), PIPE_N_READ);
		exit(-1);
	}

	//tenta connectar-se ao 2ºpipe (para leitura)
	rPipe = CreateFile(PIPE_N_READ, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (rPipe == NULL) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'... (CreateFile)\n"), PIPE_N_READ);
		exit(-1);
	}
	_tprintf(TEXT("[CLIENTE] Pipe para leitura estabelecido...\n"));


	_tprintf(TEXT("[CLIENTE] Ligação ao SERVIDOR...\n"));
}