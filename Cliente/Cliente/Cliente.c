#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
//#include "dll.h"
#include "struct.h"


#define PIPE_N_WRITE TEXT("\\\\.\\pipe\\ParaServidor")
#define PIPE_N_READ TEXT("\\\\.\\pipe\\ParaCliente")
HANDLE  hMutexEspera;

DWORD WINAPI RecebeDoServidor(LPVOID param);
int emjogo = 0;
int indice = 0;

int _tmain(int argc, LPTSTR argv[]) {
	TCHAR buf[256];
	HANDLE wPipe, rPipe;
	int comando;
	BOOL ret;
	DWORD n;
	HANDLE hThread;
	COMANDO_DO_CLIENTE cmd1;
	COMANDO_DO_SERVIDOR cmd;
    
	hMutexEspera = CreateMutex(NULL, TRUE, NULL);

#ifdef UNICODE 
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif
	
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


	//Inicia_comunicacao();

	//Invocar a thread que recebe info do servidor
	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RecebeDoServidor, (LPVOID)rPipe, 0, NULL);	
	if (hThread == NULL) {
		_tprintf(TEXT("[ERRO] Thread não foi lançada\n"));
		exit(-1);
	}
	_tprintf(TEXT("[CLIENTE]Criei a Thread para receber do servidor...\n"));

	
	ret = ReadFile(rPipe, &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);
	if (!ret || !n)

	_tprintf(TEXT("[CLIENTE]Recebi %d bytes\n "), n);

	if (cmd.resposta == 20)
	{
		indice = cmd.jogador.ID;
		_tprintf(TEXT("[CLIENTE[%d]]Recebi o meu ID\n "), indice);
	}

	while (1)
	{
		//Sleep(400); 
		//WaitForSingleObject(hMutexEspera, INFINITE);
		if (emjogo == 0) 
		{
		_tprintf(TEXT("0 - criar jogo \n 1 - sair\n"));
		_tprintf(TEXT("[CLIENTE] comando: "));
		_fgetts(buf, 256, stdin);
		 
		comando = _ttoi(buf);
	
		

		if (comando == 0) //porque la no server o 2 é que cria jogo
			comando = 2;
		
				if(comando >=1 && comando <= 2)
				{
				cmd1.tipoComando = comando;
				cmd1.ID = indice;

				ret = WriteFile(wPipe, &cmd1, sizeof(COMANDO_DO_CLIENTE), &n, NULL);
		
		
				if (!ret || !n)
					break;


				
					ret = ReadFile(rPipe, &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);
					if (!ret || !n)
						break;

					_tprintf(TEXT("[CLIENTE]Recebi %d bytes\n "), n);



					if (cmd.resposta == 1)  //entra em jogo
					{
						emjogo = 1;
						_tprintf(TEXT("[CLIENTE[%d]]Entrei em jogo\n "), indice);
					}
				}				
		}
		else 
		{   
			_tprintf(TEXT("[CLIENTE] cliente indice [%d]  \n"), indice);

			_tprintf(TEXT("0 - direita \n 1 - esquerda \n 2 - cima \n 3 - baixo\n"));
			_tprintf(TEXT("[CLIENTE] comando: "));
			
			//ReleaseMutex(hMutexEspera);
			_fgetts(buf, 256, stdin);

			comando = _ttoi(buf);
			
			

			if (comando >=0 && comando <= 3)
			{
			if (comando == 0)
				comando = 5;
			if (comando == 1)
				comando = 6;
			if (comando == 2)
				comando = 7;
			if (comando == 3)
				comando = 8;

				cmd1.tipoComando = comando;
				cmd1.ID = indice;

				ret = WriteFile(wPipe, &cmd1, sizeof(COMANDO_DO_CLIENTE), &n, NULL);


				if (!ret || !n)
					break;
				_tprintf(TEXT("[CLIENTE](ReadFile) enviei %d bytes\n"), n);


				ret = ReadFile(rPipe, &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);

				if (!ret || !n)
					break;


				_tprintf(TEXT("[CLIENTE](ReadFile) Recebi %d bytes\n "), n);

				if (cmd.resposta == 2) {} //actualizaMapa
				if (cmd.resposta == 1)
				{
					_tprintf(TEXT("Jogador[%d] : saude : %d \n lentidao : %d \n x = %d y = %d\n",cmd.jogador.ID, cmd.jogador.saude, cmd.jogador.lentidao, cmd.jogador.pos.x, cmd.jogador.pos.y));
				}
				//...

			}
		}
		






		if (comando == -1) 
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
	//COMANDO resposta;
	int i = 0;
	BOOL ret;
	DWORD n;
	COMANDO_DO_SERVIDOR cmd;

		while (1) 
		{
			//WaitForSingleObject(hMutexEspera, INFINITE);
			
		/*	if (emjogo == 0)
			{		   
				ret = ReadFile(rPipe, &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);
				if (!ret || !n)
					break;

				_tprintf(TEXT("[CLIENTE]Recebi %d bytes\n "), n );				
			  
				if (cmd.resposta == 20) 
				{
					indice = cmd.jogador.ID;
					_tprintf(TEXT("[CLIENTE[%d]]Recebi o meu ID\n "), indice);
				}

				if (cmd.resposta == 1)  //entra em jogo
				{
					emjogo = 1;
					_tprintf(TEXT("[CLIENTE[%d]]Entrei em jogo\n "), indice);
				}

			}
			else 
			{
			
				ret = ReadFile(rPipe, &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);

				if (!ret || !n)
					break;


				_tprintf(TEXT("[CLIENTE](ReadFile) Recebi %d bytes\n "), n);

				if (cmd.resposta == 2) {} //actualizaMapa
				if (cmd.resposta == 1) 
				{
					_tprintf(TEXT("Jogador : saude : %d \n lentidao : %d \n x = %d y = %d",cmd.jogador.saude,cmd.jogador.lentidao,cmd.jogador.pos.x,cmd.jogador.pos.y));				
				}
				//...
			
			}*/

			//ReleaseMutex(hMutexEspera);
			
		}
		
	return 0;
}