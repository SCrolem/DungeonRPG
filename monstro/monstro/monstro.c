#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#include "struct.h"


#define PIPE_N_WRITE TEXT("\\\\.\\pipe\\ParaServidor")
#define PIPE_N_READ TEXT("\\\\.\\pipe\\ParaCliente")
#define PIPE_N_READT TEXT("\\\\.\\pipe\\ParaCLienteMsgInstant")

MONSTRO monstro;
MEMORIA *ptrMapa;

TCHAR NomeMemoria[] = TEXT("Mapa Partilhado");
HANDLE hMemoria;


// FUNCOES
void PreparaComando(COMANDO_DO_CLIENTE *cmd);
int MovimentoIA(int tipo);
//boolean blocoNoCaminho(int x, int y);

//int VerificaComando(COMANDO *cmd,TCHAR *buf);
int EnviaComando();
HANDLE wPipe, rPipe, rrPipe;
int indice;

int _tmain(int argc, LPTSTR argv[]) {
	TCHAR buf[256];
	
	int comando;
	BOOL ret;
	DWORD n;

	COMANDO_DO_SERVIDOR cmd;


#ifdef UNICODE 
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif


	if (argc != 4)
		return -1;
	else {
		
		monstro.presente = 1;
		monstro.pos.x = _wtoi(argv[1]);
		monstro.pos.y = _wtoi(argv[2]);
		monstro.tipo_monstro = _wtoi(argv[3]);
		_tprintf(TEXT("[MONSTRO] recebi x= %d y= %d e tipo %d...\n"), monstro.pos.x, monstro.pos.y, monstro.tipo_monstro);
		if (monstro.tipo_monstro == 1) 
		{
		monstro.saude = 10;
		monstro.ataque = 5;
		monstro.defesa = 5;
		monstro.lentidao = 4;
		monstro.raioDeVisao = 3;
		}else if(monstro.tipo_monstro == 1)
		{
			monstro.saude = 15;
			monstro.ataque = 10;
			monstro.defesa = 10;
			monstro.lentidao = 15;
			monstro.raioDeVisao = 2;
		}
	}


	_tprintf(TEXT("[MONSTRO]Esperar pelo pipe '%s'(WaitNamedPipe)\n"), PIPE_N_WRITE);

	if (!WaitNamedPipe(PIPE_N_WRITE, NMPWAIT_WAIT_FOREVER)) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'... (WaitNamedPipe)\n"), PIPE_N_WRITE);
		exit(-1);
	}
	_tprintf(TEXT("[MONSTRO] Ligação ao SERVIDOR...\n"));

	wPipe = CreateFile(PIPE_N_WRITE, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); //cria ligaçao	
	if (wPipe == NULL) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'... (CreateFile)\n"), PIPE_N_WRITE);
		exit(-1);
	}
	_tprintf(TEXT("[MONSTRO] Pipe para escrita estabelecido...\n"));



	rPipe = CreateFile(PIPE_N_READ, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (rPipe == NULL) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'... (CreateFile)\n"), PIPE_N_READ);
		exit(-1);
	}
	_tprintf(TEXT("[CLIENTE] Pipe para leitura estabelecido...\n"));

	rrPipe = CreateFile(PIPE_N_READT, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (rrPipe == NULL) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'... (CreateFile)\n"), PIPE_N_READT);
		exit(-1);
	}
	_tprintf(TEXT("[CLIENTE] Pipe para leitura estabelecido...\n"));


	Sleep(100); // para o servidor ter tempo de criar o pipe

	_tprintf(TEXT("[MONSTRO] LIGUEI-ME...\n"));

	/////////////////////////////////

	hMemoria = CreateFileMapping(NULL, NULL, PAGE_READONLY, 0, sizeof(MEMORIA), NomeMemoria);//Cria HANDLE para a memoria partilhada

	if (hMemoria == NULL) {    //testa HANDLE

		_tprintf(TEXT("[ERRO] Criação de handle de memória - %d"), GetLastError());
		return -1;
	}

	ptrMapa = (MEMORIA *)MapViewOfFile(hMemoria, FILE_MAP_READ, 0, 0, sizeof(MEMORIA)); // Faz o mapeamento

	if (ptrMapa == NULL) {    //Testa o mapeamento
		_tprintf(TEXT("[ERRO] Mapeamento de Memoria partilhada - %d"), GetLastError());
		return -1;
	}

	ret = ReadFile(rPipe, &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);//recebe resposta
	if (!ret || !n)
	{
		
	}

	if (cmd.resposta == 20)
	{
		indice = cmd.jogador.ID;		
		_tprintf(TEXT("[MONSTRO]Recebi o meu ID e o meu pid %d\n "), indice);
	}

	do {
		//Sleep(monstro.lentidao * 100);
		_tprintf(TEXT("[MONSTRO] CHEGUEI...\n"));
		Sleep(300);


		EnviaComando();


	} while (monstro.saude != 0);

	UnmapViewOfFile(ptrMapa);
	CloseHandle(hMemoria);
	CloseHandle(rPipe);

	Sleep(200);
	return 0;
}


void PreparaComando(COMANDO_DO_CLIENTE *cmd1)
{
	cmd1->tipoComando = (int) 9;
	cmd1->ID = indice;
	cmd1->p_monstro.x = monstro.pos.x;
	cmd1->p_monstro.y = monstro.pos.y;
	cmd1->resposta = MovimentoIA(monstro.tipo_monstro);
	_tprintf(TEXT("a preencher movimento e foi %d\n"), cmd1->tipoComando);
}

int MovimentoIA(int tipo)
{
	
	int i;

	i = rand() % 4;
	_tprintf(TEXT("a calcular movimento e foi %d\n"), i);
	return i;
}

int EnviaComando()
{
	TCHAR buf[256];
	BOOL ret;
	DWORD n;
	COMANDO_DO_CLIENTE cmd1;
	COMANDO_DO_SERVIDOR cmd2;
	PreparaComando(&cmd1);

	_tprintf(TEXT("vou enviar ao sevidor\n"));
		ret = WriteFile(wPipe, &cmd1, sizeof(COMANDO_DO_CLIENTE), &n, NULL); //manda comando
		if (!ret || !n) 
		{}
	

		ret = ReadFile(rPipe, &cmd2, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);//recebe resposta
		if (!ret || !n) 
		{}
		monstro.pos = cmd2.p1;
       
}