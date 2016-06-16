#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
//#include "dll.h"
#include "struct.h"


#define PIPE_N_WRITE TEXT("\\\\.\\pipe\\ParaServidor")
#define PIPE_N_READT TEXT("\\\\.\\pipe\\ParaCLienteMsgInstant")
#define PIPE_N_READ TEXT("\\\\.\\pipe\\ParaCliente")
HANDLE  hMutexEspera;

DWORD WINAPI RecebeDoServidor(LPVOID param);
int emjogo = 0;
int JogoCriado = 0;
int EsperaDeComeçar = 0;
int fezlogin = 0;
int indice = 0;
int pid = 0;

void imprimeMundo(COMANDO_DO_SERVIDOR  c);
void imprimeJogador(COMANDO_DO_SERVIDOR cmd);
void atacarMonstro(COMANDO_DO_SERVIDOR *cmd, int Ax, int Ay, int posX, int posY);
void atacarJogador(COMANDO_DO_SERVIDOR *cmd, int Ax, int Ay, int posX, int posY);
void atacar(COMANDO_DO_SERVIDOR cmd);

int _tmain(int argc, LPTSTR argv[]) {
	TCHAR buf[256];
	HANDLE wPipe, rPipe, rrPipe;
	int comando;
	int aEsperaDeJogar = 0;
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

	rrPipe = CreateFile(PIPE_N_READT, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (rrPipe == NULL) {
		_tprintf(TEXT("[ERRO] Ligar ao pipe '%s'... (CreateFile)\n"), PIPE_N_READT);
		exit(-1);
	}
	_tprintf(TEXT("[CLIENTE] Pipe para leitura estabelecido...\n"));

	//Inicia_comunicacao(); //dll

	//Invocar a thread que recebe info do servidor
	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RecebeDoServidor, (LPVOID)rrPipe, 0, NULL);
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
		pid = cmd.jogador.pid;
		_tprintf(TEXT("[CLIENTE[%d]]Recebi o meu ID e o meu pid %d\n "), indice,pid);
	}

	

	while (1)
	{

		
		if (emjogo == 0) 
		{
		
					if (fezlogin == 0) 
					{
						_tprintf(TEXT("0 - login \n 1 - registar\n"));
						_tprintf(TEXT("[CLIENTE] comando: "));
						_fgetts(buf, 256, stdin);

						comando = _ttoi(buf);
			
						if (comando == 0 || comando == 1) 
						{
							_tprintf(TEXT("Utilizador: "));
							_fgetts(buf, 15, stdin);
							 
							 _tcscpy_s(cmd1.user.login, TAM_LOG, buf);
							
							// _tprintf(TEXT("o nome foi : %s "), cmd1.user.login);

							cmd1.tipoComando = comando;
							cmd1.ID = indice;


							ret = WriteFile(wPipe, &cmd1, sizeof(COMANDO_DO_CLIENTE), &n, NULL); //manda comando
							if (!ret || !n)
								break;

							ret = ReadFile(rPipe, &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);//recebe resposta
							if (!ret || !n)
								break;

							_tprintf(TEXT("[CLIENTE]Recebi %d bytes\n "), n);

							_tprintf(TEXT("msg : %s \n"),cmd.msg);


							if (cmd.resposta == 1)  //entra em jogo
							{
								fezlogin = 1;
								_tprintf(TEXT("[CLIENTE[%d] Fiz login!\n "), indice);
							}
						}

					}
					else 
					{
						//escreve e recebe auto para saber se 




						if (aEsperaDeJogar == 0) { 
							
							_tprintf(TEXT("0 - criar jogo\n 1 - Juntar ao jogo \n 5 - sair\n"));
							_tprintf(TEXT("[CLIENTE] comando: "));
							_fgetts(buf, 256, stdin);

							comando = _ttoi(buf);

							if (comando == 0){ //porque la no server o 2 é que cria jogo
								comando = 2;
							}else if (comando == 1){
								comando = 3;
							}else if (comando == 5){
								comando = 10;}
							

							

							if (comando == 10 || comando == 2 || comando == 3)
							{
								cmd1.tipoComando = comando;
								cmd1.ID = indice;

								ret = WriteFile(wPipe, &cmd1, sizeof(COMANDO_DO_CLIENTE), &n, NULL);

								if (!ret || !n)
									break;

								ret = ReadFile(rPipe, &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);
								if (!ret || !n)
									break;

								
								_tprintf(TEXT("msg : %s \n"), cmd.msg);


								if (cmd.tipoResposta== 1)//caso receba resposta de um pedido de criar jogo
								{

									 if (cmd.resposta == 1)
											{
												//caso tenha sido o primeiro a criar o jogo vai ficar com um menu de iniciar jogo
												_tprintf(TEXT("0 - começar\n"));
												_tprintf(TEXT("[CLIENTE] comando: "));

												do 
												{
													_fgetts(buf, 256, stdin);

													comando = _ttoi(buf);

													if (comando == 0) //porque la no server o 4 corresponde a começar o jogo
														comando = 4;

												} while (comando != 4);
													
												cmd1.tipoComando = comando;
												cmd1.ID = indice;

													ret = WriteFile(wPipe, &cmd1, sizeof(COMANDO_DO_CLIENTE), &n, NULL);

													if (!ret || !n)
														break;

													ret = ReadFile(rPipe, &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);
													if (!ret || !n)
														break;

													aEsperaDeJogar = 1;
													emjogo = 1;
												_tprintf(TEXT("msg : %s \n"), cmd.msg);
											}
									
									
								}
								else if (cmd.tipoResposta == 2)
								{
									if (cmd.resposta == 1) 
									{
										aEsperaDeJogar = 1;
									}
									else if (cmd.resposta == 2) 
									{
									
									}
								}

							}

						}
						else 
						{
							_tprintf(TEXT("A espera... "));
							while (emjogo != 1) { Sleep(500); }
						
						}
					




					}	//fim do else do login		

		}
		else
		{
			_tprintf(TEXT("[CLIENTE] cliente indice [%d]  \n"), indice);

			_tprintf(TEXT("0 - esquerda \n 1 - direita \n 2 - cima \n 3 - baixo\n"));
			_tprintf(TEXT("[CLIENTE] comando: "));

			//ReleaseMutex(hMutexEspera);
			_fgetts(buf, 256, stdin);

			comando = _ttoi(buf);



			if (comando >= 0 && comando <= 3)
			{
				if (comando == 0)
					comando = 5;
				if (comando == 1)
					comando = 6;
				if (comando == 2)
					comando = 8;
				if (comando == 3)
					comando = 7;

				cmd1.tipoComando = comando;
				cmd1.ID = indice;

				ret = WriteFile(wPipe, &cmd1, sizeof(COMANDO_DO_CLIENTE), &n, NULL);

				if (!ret || !n)
					break;
				_tprintf(TEXT("[CLIENTE](ReadFile) enviei %d bytes\n"), n);


				//ret = ReadFile(rPipe, &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);
				//imprimeMundo(cmd);

				//if (!ret || !n)
				//	break;


				//_tprintf(TEXT("[CLIENTE](ReadFile) Recebi %d bytes\n "), n);

				
			//	if (cmd.resposta == 1)
			//	{

			//		_tprintf(TEXT("Jogador[%d] : saude : %d \n lentidao : %d \n x = %d y = %d\n"),cmd.jogador.ID, cmd.jogador.saude, cmd.jogador.lentidao, cmd.jogador.pos.x, cmd.jogador.pos.y);
			//	}
			

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

		ret = ReadFile(rPipe, &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);
		if (!ret || !n)
			break;
		//_tprintf(TEXT("[CLIENTE]Recebi %d bytes\n "), n);

		if (cmd.tipoResposta == 0)//mensagem
		{
			_tprintf(TEXT("msg : %s \n"), cmd.msg);
		}
		else if (cmd.tipoResposta == 1) //actualizações 
		{
			

			if (cmd.resposta == 1) 
			{
			emjogo = 1;
			}
			else if (cmd.resposta == 2) 
			{
				_tprintf(TEXT("\n\nIMPRIMIR:\n"));
			imprimeJogador(cmd);
			imprimeMundo(cmd);	
			}

					
			
		}

	}

	return 0;
}


void imprimeJogador(COMANDO_DO_SERVIDOR cmd)
{

	_tprintf(TEXT("\n\nJogador:\n"));
	_tprintf(TEXT("nome: %s  \nid:%d\nsaude: %d \nx=%d, y=%d\n  "),cmd.jogador.username ,cmd.jogador.ID, cmd.jogador.saude, cmd.jogador.pos.x, cmd.jogador.pos.y);

}

void imprimeMundo(COMANDO_DO_SERVIDOR c)
{
	int i = 0, j = 0;

	_tprintf(TEXT("\n\n\tMundo:\n"));
	for (i = 0; i < L; i++) {
		for (j = 0; j < C; j++) {
			
			if (c.mapa[i][j].jogador.presente == 1)
				_tprintf(TEXT(" X "));
			else if (c.mapa[i][j].monstro.presente == 1)
				_tprintf(TEXT(" M "));
			else if (c.mapa[i][j].objeto.tipo != 0)
				_tprintf(TEXT(" Z "));
			else if (c.mapa[i][j].bloco.tipo == 0)
				_tprintf(TEXT(" O "));
			else if (c.mapa[i][j].bloco.tipo != 0)
				_tprintf(TEXT(" # "));

		}
		_tprintf(TEXT("\n"));
	}

	_tprintf(TEXT("\n\n"));
}




void atacarMonstro(COMANDO_DO_SERVIDOR *cmd, int Ax, int Ay, int posX, int posY) {
	if (cmd->mapa[posX][posY].jogador.presente == 1 && cmd->mapa[posX][posY].monstro.presente == 1) {
		cmd->mapa[posX][posY].jogador.vidas--;
		if (cmd->mapa[posX][posY].monstro.saude < cmd->mapa[posX][posY].monstro.limSaude)
			cmd->mapa[posX][posY].monstro.saude++;
		else {
			//envia pedido ao servidor para que seja criado outro monstro na pos adjacente da pos atual do monstro "pai"
		}
	}
	cmd->mapa[Ax][Ay].monstro.saude--;
}

void atacarJogador(COMANDO_DO_SERVIDOR *cmd, int Ax, int Ay, int posX, int posY) {

	if (cmd->mapa[posX][posY].jogador.modoAtaque == 1 && cmd->mapa[posX][posY].jogador.nPedras > 0) {
		cmd->mapa[Ax][Ay].jogador.vidas -= 2;
		cmd->mapa[Ax][Ay].jogador.nPedras--;
	}
	else {
		cmd->mapa[Ax][Ay].jogador.vidas--;
	}
}

void atacar(COMANDO_DO_SERVIDOR cmd) {
	int i, j;

	for (i = 0; i < L; i++) {
		for (j = 0; j < C; j++) {
			if (cmd.mapa[i][j].jogador.presente == 1) {

				//atacar um jogador
				if (cmd.mapa[i - 1][j].jogador.presente == 1)
					atacarJogador(&cmd, i - 1, j, i, j);
				else if (cmd.mapa[i + 1][j].jogador.presente == 1)
					atacarJogador(&cmd, i + 1, j, i, j);
				else if (cmd.mapa[i][j - 1].jogador.presente == 1)
					atacarJogador(&cmd, i, j - 1, i, j);
				else if (cmd.mapa[i][j + 1].jogador.presente == 1)
					atacarJogador(&cmd, i, j + 1, i, j);

				//atacar um monstro 
				if (cmd.mapa[i][j].monstro.presente == 1)
					atacarMonstro(&cmd, i, j, i, j);
				else if (cmd.mapa[i - 1][j].monstro.presente == 1)
					atacarMonstro(&cmd, i - 1, j, i, j);
				else if (cmd.mapa[i + 1][j].monstro.presente == 1)
					atacarMonstro(&cmd, i + 1, j, i, j);
				else if (cmd.mapa[i][j - 1].monstro.presente == 1)
					atacarMonstro(&cmd, i, j - 1, i, j);
				else if (cmd.mapa[i][j + 1].monstro.presente == 1)
					atacarMonstro(&cmd, i, j + 1, i, j);
			}
		}
	}
}
