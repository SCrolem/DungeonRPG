#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "struct.h"

#define PIPE_N_READ TEXT("\\\\.\\pipe\\ParaServidor")
#define PIPE_N_WRITE TEXT("\\\\.\\pipe\\ParaCliente")
#define PIPE_N_WRITET TEXT("\\\\.\\pipe\\ParaCLienteMsgInstant")

#define N_MAX_CLIENTES 10
#define TAM 256

USER users[U_MAX] = { NULL };
MEMORIA *ptrMapa;


TCHAR NomeMemoria[] = TEXT("Mapa Partilhado");
TCHAR NomeMutexMapa[] = TEXT("MapaMUTEX");
JOGADOR semjogador;

STARTUPINFO si[10];
PROCESS_INFORMATION pi[10];
//MUTEXS
HANDLE hMemoria, hMutexMapa, hMutexEnvio, hMutexEnviaID;

//DADOS DOS JOGADORES
JOGADOR JogadoresOnline[N_MAX_CLIENTES];
JOGADOR JogadoresRegistados[N_MAX_CLIENTES];

//PIPES
HANDLE wPipeClientes[N_MAX_CLIENTES];
HANDLE rPipeClientes[N_MAX_CLIENTES];
HANDLE wSemEsperarPipeClientes[N_MAX_CLIENTES];

int total = 0;
int obrigatorios = 10;
int registados = -1;
int logados = -1;
int jogadoresEmJogo = 0;

int jogoEmEspera = 1;
BOOL sair = FALSE;
//CELULA mundo[LT][CT];

BOOL encerrar = FALSE;
JOGO game;

// THREADS
DWORD WINAPI RecebeClientes(LPVOID param);
DWORD WINAPI AtendeCliente(LPVOID param);


//atribui uma identidade ao cliente
void enviaID(int indice_deste_cliente);

//parte do login e registar
void RegistaUtilizador(COMANDO_DO_CLIENTE *c);			
void LoginUtilizador(COMANDO_DO_CLIENTE *c);

//cria��o do jogo e entrada no jogo
void CriaMundo();
void CriaJogo(COMANDO_DO_CLIENTE *c);
void ComecaJogo(COMANDO_DO_CLIENTE *c);


//adi��o do jogador ao jogo
void AdicionaJogadorAoJogo(COMANDO_DO_CLIENTE *c);
void PreencheJogador(COMANDO_DO_CLIENTE *c);
void CalculaPosicaoAleatoria(POSICAO *p);

//verifica��es
int VerificaExisteAlgoPosicao(POSICAO *p);
void VerificaJogo(COMANDO_DO_CLIENTE *cmd); //??????

//cria info para envio coletivo

void EnviaJogadorEMapaActual(COMANDO_DO_CLIENTE *cmd);
void CriaMapaParaEnvio(COMANDO_DO_SERVIDOR *cmd);
void MandarMensagens(COMANDO_DO_CLIENTE *c, int tipo_mensagem);
void AbrirJogoATodosClientesEmJogo();
void CriaInfoSobreOJogador(COMANDO_DO_SERVIDOR *cmd, int indice);


//trata do comandos
void TrataComando(COMANDO_DO_CLIENTE *c);

//comandos de a��o
void Move(COMANDO_DO_CLIENTE *cmd);
void MoveMonstro(COMANDO_DO_CLIENTE * c);


//consequencias
JOGADOR JogadorMorre(JOGADOR j);
void apanhaObjeto();
//void ApanhaObjecto(POSICAO p, COMANDO_DO_CLIENTE *c);

//zera vars
void iniciaOsemjogador();
void zeramonstros();
void zerajogadores();

int _tmain(int argc, LPTSTR argv[]) {
	HANDLE hThread;	
	iniciaOsemjogador();
	game.iniciado = 0;
	game.criado = 0;

	//cria��o dos mutexes
	hMutexEnviaID = CreateMutex(NULL, FALSE, NULL);
	hMutexMapa = CreateMutex(NULL, FALSE, NULL);
	hMutexEnvio = CreateMutex(NULL, FALSE, NULL);


#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	/////////////////////////Mapeamento de mem�ria/////////////////////////////////////////////

	hMemoria = CreateFileMapping(NULL, NULL, PAGE_READWRITE, 0, sizeof(MEMORIA), NomeMemoria);//Cria HANDLE para a memoria partilhada

	if (hMemoria == NULL) {    //testa HANDLE
		_tprintf(TEXT("[ERRO] Cria��o de handle de mem�ria - %d"), GetLastError());
		return -1;
	}

	ptrMapa = (MEMORIA *)MapViewOfFile(hMemoria, FILE_MAP_WRITE, 0, 0, sizeof(MEMORIA)); // Faz o mapeamento

	if (ptrMapa == NULL) {    //Testa o mapeamento
		_tprintf(TEXT("[ERRO] Mapeamento de Memoria partilhada - %d"), GetLastError());
		return -1;
	}
	//////////////////////////////////////////////////////////////////////


	//Invocar a thread que inscreve novos clientes
	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RecebeClientes, NULL, 0, NULL);
	if (hThread == NULL) {
		_tprintf(TEXT("[ERRO] Thread n�o foi lan�ada\n"));
		exit(-1);
	}

	//Esperar a thread recebeLeitores terminar
	WaitForSingleObject(hThread, INFINITE);
	UnmapViewOfFile(ptrMapa);
	CloseHandle(hMemoria);
	CloseHandle(hThread);
	exit(0);
}

void iniciaOsemjogador() 
{
	JOGADOR j;
	POSICAO p;
	int i;
	
	j.ID = 0;
	j.nPedras = 0;
	j.pontos = 0;
	j.lentidao = 0;
	j.saude = 0;
	j.presente = 0;  //a verificar
	p.x = 0;
	p.y = 0;
	j.pos = p;

	//WaitForSingleObject(hMutexUsers, INFINITE);
	for (i = 0; i < N_MAX_CLIENTES; i++) 
	{
		JogadoresOnline[i] = j;
	}

	semjogador = j;
		//ReleaseMutex(hMutexUsers);
}

/*void ApanhaObjecto(POSICAO p, COMANDO_DO_CLIENTE *c) {
	int obj, i, j;
	

	WaitForSingleObject(hMutexMapa, INFINITE);

	obj = ptrMapa->mapa[p.x][p.y].obj;


	switch (obj)
	{
	case 1: // obrigatorios
		
		obrigatorios--;
		c->jogador.pontos += 10;
		ptrMapa->mapa[p.x][p.y].obj = 0;

		if (obrigatorios == 0) {
			for (i = 1; i<L - 1; i++) {
				for (j = 1; j<C - 1; j++) {
					if (ptrMapa->mapa[i][j].obj == 4)
						ptrMapa->mapa[i][j].obj = 5;
				}
			}
		}
		//MandaDifusao(20,&jog);
		break;
	case 2: //vidas
		c->jogador.vidas++;
		ptrMapa->mapa[p.x][p.y].obj = 0;
		break;
	
	case 5:
	//	MandaDifusao(21, NULL);
		break;

	default:
		break;
	}


	ptrMapa->mapa[p.x][p.y].jogador = c->jogador;
	ReleaseMutex(hMutexMapa);
}*/

JOGADOR JogadorMorre(JOGADOR j) {
	int res;
	POSICAO p;

	if (j.vidas>0) { //se ainda tiver vidas

		j.vidas--;
		j.saude = 100;


		//j.pos = p;

	}
	else {// senao morre
		j.vidas--;
		j.saude = 0;

	}

	do {
		CalculaPosicaoAleatoria(&p);
		res = VerificaExisteAlgoPosicao(&p);
	} while (res != 0);

	//j.pos = p;

	return j;

}

void VerificaJogo(COMANDO_DO_CLIENTE *cmd) {
	if (!game.criado) {
		cmd->resposta = 0;
		return;
	}
	else {
		if (!game.iniciado) {
			cmd->resposta = 1;
			return;
		}
		cmd->resposta = 2;
		return;
	}
}

void CriaMapaParaEnvio(COMANDO_DO_SERVIDOR *cmd) {
	int i=0, j=0;
	int x = 0, y = 0;
	POSICAO p1, p2;
	DWORD n;



	/*if ((cmd->jogador.pos.x - 5) >= 0)
		cmd->p1.x = (cmd->jogador.pos.x - 5);
	else 
		cmd->p1.x = 0;

	if ((cmd->jogador.pos.y - 5) >= 0)
		cmd->p1.y= (cmd->jogador.pos.y - 5);
	else cmd->p1.y = 0;

	if((cmd->jogador.pos.x + 5) >= 0)
		cmd->p2.x = (cmd->jogador.pos.x + 5);
	else
		cmd->p2.x = L;
	if ((cmd->jogador.pos.y + 5) >= 0)
		cmd->p2.y = (cmd->jogador.pos.y + 5);
	else cmd->p2.y = C;*/

	if ((cmd->jogador.pos.x - 5) < 0 && (cmd->jogador.pos.y - 5) < 0)  //cantos
	{
		cmd->p1.x = 0;
		cmd->p1.y = 0;
		cmd->p2.x = L;
		cmd->p2.y = C;
	}
	else if ((cmd->jogador.pos.x - 5) < 0 && (cmd->jogador.pos.y + 5) > CT)
	{
		cmd->p1.x = 0;
		cmd->p1.y = CT - C;
		cmd->p2.x = L;
		cmd->p2.y = CT;			
	}
	else if ((cmd->jogador.pos.x + 5) > LT && (cmd->jogador.pos.y + 5) > CT)
	{
		cmd->p1.x = LT-L;
		cmd->p1.y = CT-C;
		cmd->p2.x = LT;
		cmd->p2.y = CT;
	}
	else if ((cmd->jogador.pos.x + 5) > LT && (cmd->jogador.pos.y - 5) < 0)
	{
		cmd->p1.x = LT - L;
		cmd->p1.y = 0;
		cmd->p2.x = LT;
		cmd->p2.y = C;
	}
	else 
	{
		if ((cmd->jogador.pos.x + 5) > LT) //paredes laterais
		{
			cmd->p1.x = LT - L;
			cmd->p1.y = (cmd->jogador.pos.y - 5) ;
			cmd->p2.x = LT;
			cmd->p2.y = (cmd->jogador.pos.y + 5);

		}else if ((cmd->jogador.pos.x - 5) < 0)
		{
			cmd->p1.x = 0;
			cmd->p1.y = (cmd->jogador.pos.y - 5);
			cmd->p2.x = L;
			cmd->p2.y = (cmd->jogador.pos.y + 5);
		}
		else if ((cmd->jogador.pos.y + 5) > CT)
		{
			cmd->p1.x = (cmd->jogador.pos.x - 5);
			cmd->p1.y = CT - C;
			cmd->p2.x = (cmd->jogador.pos.x + 5);
			cmd->p2.y = CT;

		}
		else if ((cmd->jogador.pos.y - 5) < 0)
		{
			cmd->p1.x = (cmd->jogador.pos.x - 5);
			cmd->p1.y = 0;
			cmd->p2.x = (cmd->jogador.pos.x + 5);
			cmd->p2.y = C;
		}
		else 
		{
			cmd->p1.x = (cmd->jogador.pos.x - 5); //o normal 
			cmd->p1.y = (cmd->jogador.pos.y - 5);
			cmd->p2.x = (cmd->jogador.pos.x + 5);
			cmd->p2.y = (cmd->jogador.pos.y + 5);		
		}
	}
		


	for (i = cmd->p1.x; i < cmd->p2.x; i++) {
		y = 0;
		for (j = cmd->p1.y; j <cmd->p2.y; j++) {
			cmd->mapa[x][y] = ptrMapa->mundo[i][j];
			y++;
		}
		x++;
	}
	
}

int VerificaExisteAlgoPosicao(POSICAO *p) {

	if (ptrMapa->mundo[p->x][p->y].jogador.presente == 1) {
		return 1;	//existe um jogador
	}
	else {
		if (ptrMapa->mundo[p->x][p->y].monstro.presente == 1) {
			//if (_tcscmp(ptrMapa->mapa[p->x][p->y].inimigo.nome,TEXT(""))!=0){
			return 2; // existe um inimigo
		}
		else {
			if (ptrMapa->mundo[p->x][p->y].bloco.tipo == 1) {
				return 3; // existe um bloco mole
			}
			else {
				if (ptrMapa->mundo[p->x][p->y].bloco.tipo == 2) {
					return 4; // existe um bloco duro
				}
				else {
					//if (ptrMapa->mapa[p->x][p->y].bomba == 2)
						//return 5;

				}
			}
		}
	}
	return 0; // est� livre
}

void CalculaPosicaoAleatoria(POSICAO *p) {
	int x, y;
	x = rand() % (L - 1) + 1;
	y = rand() % (C - 1) + 1;
	p->x = x;
	p->y = y;
}

void CriaMundo() {

	//float perc;
	int i, j, k;
	int x, y;
	int g, l, p;

	/* --------------------------------------------------------- CRIA�AO DO MUNDO --------------------------------------------------------*/

	srand((unsigned)time(NULL));

	zeramonstros();
	zerajogadores();

	for (i = 0; i<LT; i++) {			//criacao do mundo a funcionar
		for (j = 0; j<CT; j++) {
			ptrMapa->mundo[i][j].bloco.tipo = 0;

			if (i == 0)								// paredes nas extremidades
				ptrMapa->mundo[i][j].bloco.tipo = 1;
			if (i == (CT -1))
				ptrMapa->mundo[i][j].bloco.tipo = 1;
			if (j == 0)
				ptrMapa->mundo[i][j].bloco.tipo = 1;
			if (j == (LT - 1))
				ptrMapa->mundo[i][j].bloco.tipo = 1;

			if (ptrMapa->mundo[i][j].bloco.tipo != 1) {
				for (g = 0; g < 4; g++) //CRIACAO DE ITEMS
				{
					x = 0 + (rand() % 100);

					if (x >= 0 && x < 10) {
						ptrMapa->mundo[i][j].obj = 1;
					}
					else if (x >= 10 && x < 30) {
						ptrMapa->mundo[i][j].obj = 2;
					}
					else if (x >= 30 && x < 34) {
						ptrMapa->mundo[i][j].obj=3;
						
					}
					else if (x >= 34 && x < 39) {
						ptrMapa->mundo[i][j].obj=4;
					}
					else if (x >= 39 && x < 99) {
						ptrMapa->mundo[i][j].obj = 0;
					}
					
				}
			}
		}
	}
}

void PreencheJogador(COMANDO_DO_CLIENTE *cmd)
{
	int x = 0; 
	int y = 0;
	int Indice = cmd->ID;
	COMANDO_DO_SERVIDOR c;


	_tprintf(TEXT("[SERVIDOR] Jogador [%d] preenchido...\n"), Indice);

	JogadoresOnline[Indice].Ajogar = 1;
	JogadoresOnline[Indice].pontos = 0;
	JogadoresOnline[Indice].lentidao = 3;
	JogadoresOnline[Indice].saude = 10;
	JogadoresOnline[Indice].presente = 1;  //a verificar
	JogadoresOnline[Indice].pid = _getpid();

	do {
		srand((unsigned)time(NULL));
		x = rand() % 10; 
		y = rand() % 10;

		if (ptrMapa->mundo[x][y].bloco.tipo == 0) {
			POSICAO p; //
			p.x = x;//
			p.y = y;//
			JogadoresOnline[Indice].pos = p;//
			ptrMapa->mundo[x][y].jogador = JogadoresOnline[Indice];
		}
	} while (ptrMapa->mundo[x][y].bloco.tipo != 0);
	//c.jogador = JogadoresOnline[Indice];
}

void EnviaJogadorEMapaActual(COMANDO_DO_CLIENTE *cmd)
{
	WaitForSingleObject(hMutexEnvio, INFINITE);

	DWORD n;
//	int indice = cmd->ID;
	COMANDO_DO_SERVIDOR msg_a_enviar;
	int i, j;
	
	msg_a_enviar.tipoResposta = 1;
	msg_a_enviar.resposta = 2;

	for (i = 0; i < N_MAX_CLIENTES; i++)
	{
		if (wSemEsperarPipeClientes[i] != NULL && JogadoresOnline[i].Ajogar == 1) 
		{
		CriaInfoSobreOJogador(&msg_a_enviar, i);
		CriaMapaParaEnvio(&msg_a_enviar);
        WriteFile(wSemEsperarPipeClientes[i], &msg_a_enviar, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);
		}
	}

	ReleaseMutex(hMutexEnvio);
}

void CriaInfoSobreOJogador(COMANDO_DO_SERVIDOR *cmd, int indice) 
{
	int i = indice;
	_tcscpy_s(cmd->jogador.username, 15, JogadoresOnline[indice].username);  
	cmd->jogador.ID = JogadoresOnline[i].ID;
	cmd->jogador.lentidao = JogadoresOnline[i].lentidao;
	cmd->jogador.nPedras = JogadoresOnline[i].nPedras;
	cmd->jogador.saude = JogadoresOnline[i].saude;
	cmd->jogador.pos.x = JogadoresOnline[i].pos.x;
	cmd->jogador.pos.y = JogadoresOnline[i].pos.y;

}

void TrataComando(COMANDO_DO_CLIENTE *cmd) {

	//_tprintf(TEXT("recebi o comando : %d do cliente :%d   com o nome : %s"), cmd->tipoComando, cmd->ID, cmd->user.login);

	switch (cmd->tipoComando)
	{
	case 0:  //AUTENTICAR
		LoginUtilizador(cmd);		
		break;

	case 1: //REGISTAR 
		RegistaUtilizador(cmd);
		break;

	case 2:  // Criar Jogo
		CriaJogo(cmd);
		break;

	case 3:  // Juntar Ao Jogo
		AdicionaJogadorAoJogo(cmd);
		break;

	case 4:  // Inicia o Jogo
		ComecaJogo(cmd);
		break;

	case 5:  //move
	case 6:
	case 7:
	case 8:
		WaitForSingleObject(hMutexMapa, INFINITE);
		Move(cmd);		
		ReleaseMutex(hMutexMapa);
		break;
	case 9:
		WaitForSingleObject(hMutexMapa, INFINITE);
		_tprintf(TEXT("monstro fez movimento...\n"));
		MoveMonstro(cmd);
		ReleaseMutex(hMutexMapa);
		break;
	case 10: // verifica jogo
		VerificaJogo(cmd);
		break;
	default:
		break;
	}

	return;
}

void LoginUtilizador(COMANDO_DO_CLIENTE *c) {
	int i=0,flag=0;
	DWORD n;
	COMANDO_DO_SERVIDOR cmd;
	

	cmd.resposta = 0;

	if (logados == -1 && registados != (-1)) 
	{


		for (i = 0; i <= registados; i++) {
			
			if (_tcscmp(JogadoresRegistados[i].username, c->user.login) == 0)
			{				
				_tcscpy_s(cmd.msg, TAM_MSG, TEXT("Login feito com sucesso!"));
				_tcscpy_s(JogadoresOnline[c->ID].username, TAM_LOG, c->user.login);
				logados++;
				cmd.resposta = 1;
				flag = 1;
				break;
			}
		}
		if (flag == 0) {
			cmd.resposta = 2;
			_tcscpy_s(cmd.msg, TAM_MSG, TEXT("Utilizador nao registado!"));
		}
	}
	else
	{
		if (logados >= 0 && logados < (N_MAX_CLIENTES-1) && registados != (-1))
		{
			for (i = 0; i < N_MAX_CLIENTES; i++)
			{
				if (_tcscmp(JogadoresOnline[i].username, c->user.login) == 0) {
					flag = 1;
					cmd.resposta = 3;
					_tcscpy_s(cmd.msg, TAM_MSG, TEXT("Esse user j� esta em jogo!"));
					break;
				}
			}

			if (flag == 0)
			{
				for (i = 0; i <= registados; i++)
				{
					if (_tcscmp(JogadoresRegistados[i].username, c->user.login) == 0)
					{
						_tcscpy_s(JogadoresOnline[c->ID].username, TAM_LOG, c->user.login);
						_tcscpy_s(cmd.msg, TAM_MSG, TEXT("Login feito com sucesso!"));
						flag = 1;
						logados++;
						cmd.resposta = 1;
						break;
					}
				}

				if(flag == 0){
				_tcscpy_s(cmd.msg, TAM_MSG, TEXT("Utilizador nao registado!"));
				}
			}

		}else
		{
			_tcscpy_s(cmd.msg, TAM_MSG, TEXT("numero de logados atingiu o limite ou tem de se registar primeiro!"));
		}
	}

	WriteFile(wPipeClientes[c->ID], &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);	
}

void Move(COMANDO_DO_CLIENTE *c) {

	int indice_jogador = c->ID;
	int accao = c->tipoComando;
	int res;
	POSICAO posInicial;
	POSICAO posFinal;

	
	
	posInicial = JogadoresOnline[indice_jogador].pos;

	posFinal = posInicial;

	switch (accao)
	{
	case 5:  //esquerda
		posFinal.y = posInicial.y - 1;
		break;
	case 6: //direita
		posFinal.y = posInicial.y + 1;
		break;
	case 7: //baixo
		posFinal.x = posInicial.x + 1;
		break;
	case 8: //cima
		posFinal.x = posInicial.x - 1;
		break;
	default:
		break;
	}

	if (posFinal.x < 0)
		posFinal.x = 0;
	if (posFinal.y < 0)
		posFinal.y = 0;

	if (posFinal.x > LT)
		posFinal.x = LT ;
	if (posFinal.y > CT)
		posFinal.y = CT ;

	res = VerificaExisteAlgoPosicao(&posFinal);

	if (res == 0 || res == 3) 
	{   
		JogadoresOnline[indice_jogador].pos = posFinal;
		ptrMapa->mundo[posFinal.x][posFinal.y].jogador = JogadoresOnline[indice_jogador];
		ptrMapa->mundo[posInicial.x][posInicial.y].jogador = semjogador;
	}

	//caso seja um bloco que parte o que fazer?
	EnviaJogadorEMapaActual(c);
	
	return;
}

void MoveMonstro(COMANDO_DO_CLIENTE * c) 
{
	
	int accao = c->resposta;
	int res;
	POSICAO posInicial;
	POSICAO posFinal;
	COMANDO_DO_SERVIDOR cmd;
	DWORD n;


	
	posInicial = c->p_monstro;

	posFinal = posInicial;

	switch (accao)
	{
	case 0:  //esquerda
		posFinal.y = posInicial.y - 1;
		break;
	case 1: //direita
		posFinal.y = posInicial.y + 1;
		break;
	case 2: //baixo
		posFinal.x = posInicial.x + 1;
		break;
	case 3: //cima
		posFinal.x = posInicial.x - 1;
		break;
	default:
		break;
	}

	if (posFinal.x < 0)
		posFinal.x = 0;
	if (posFinal.y < 0)
		posFinal.y = 0;

	if (posFinal.x > LT)
		posFinal.x = LT;
	if (posFinal.y > CT)
		posFinal.y = CT;

	res = VerificaExisteAlgoPosicao(&posFinal);

	if (res == 0 || res == 3)
	{
		cmd.p1 = posFinal;
		ptrMapa->mundo[posFinal.x][posFinal.y].monstro = ptrMapa->mundo[posInicial.x][posInicial.y].monstro;
		ptrMapa->mundo[posFinal.x][posFinal.y].monstro.presente = 1;
		ptrMapa->mundo[posInicial.x][posInicial.y].monstro.presente = 0;
	}
	else 
	{
		cmd.p1 = posInicial;
	}

	//caso seja um bloco que parte o que fazer?
	
	EnviaJogadorEMapaActual(c);
    WriteFile(wPipeClientes[c->ID], &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);
	
	return;

}

void RegistaUtilizador(COMANDO_DO_CLIENTE *c) {
	COMANDO_DO_SERVIDOR cmd;
	int i;
	int id;
	DWORD n;
	int conta = 0;
	int flag = 0;
	cmd.resposta = 0;
	

	if (registados == -1) {
		registados+=1;
		_tcscpy_s(JogadoresRegistados[registados].username, TAM_LOG, c->user.login);
		
		_tcscpy_s(cmd.msg, TAM_MSG, TEXT("Registado com sucesso"));
		
	}
	else if (registados>=0 && registados < (N_MAX_CLIENTES-1))
	{
		for (i = 0; i <= registados; i++)
		{
			if (_tcscmp(c->user.login, JogadoresRegistados[i].username) == 0) {

				flag = 1;
				cmd.resposta = 3;
				_tcscpy_s(cmd.msg, TAM_MSG, TEXT("Tal username j� existe!"));
				break;
			}
		}
		if (flag == 0) {
			registados+=1;
			_tcscpy_s(JogadoresRegistados[registados].username, TAM_LOG, c->user.login);
			_tcscpy_s(cmd.msg, TAM_MSG, TEXT("Registado com sucesso"));
		}
	}
	else {
		cmd.resposta = 2;
		_tcscpy_s(cmd.msg, TAM_MSG, TEXT("Excedeu o numero de inscritos!"));
	
	}

	
	id = c->ID;
	WriteFile(wPipeClientes[id], &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);
	
}

DWORD WINAPI RecebeClientes(LPVOID param) {
	
	int i = 0;
	HANDLE ThreadAtendeCli[N_MAX_CLIENTES];
   

	while (!sair && total < N_MAX_CLIENTES)
	{
		
		_tprintf(TEXT("[Servidor](CreateNamedPipe)'%s' envia ao cliente \n"), PIPE_N_WRITE);

		for (i = 0; i < N_MAX_CLIENTES; i++) 
		{
			if (wPipeClientes[i] == 0)
				break;
		}
        
		wPipeClientes[i] = CreateNamedPipe(PIPE_N_WRITE, PIPE_ACCESS_OUTBOUND, PIPE_WAIT | PIPE_TYPE_MESSAGE
			| PIPE_READMODE_MESSAGE, N_MAX_CLIENTES,sizeof(COMANDO_DO_SERVIDOR), sizeof(COMANDO_DO_SERVIDOR),
			1000, NULL);		

		_tprintf(TEXT("[Servidor](CreateNamedPipe) '%s' recebe do cliente \n"), PIPE_N_READ);

		rPipeClientes[i] = CreateNamedPipe(PIPE_N_READ, PIPE_ACCESS_INBOUND, PIPE_WAIT | PIPE_TYPE_MESSAGE
			| PIPE_READMODE_MESSAGE, N_MAX_CLIENTES,sizeof(COMANDO_DO_CLIENTE), sizeof(COMANDO_DO_CLIENTE),
			1000, NULL);

		_tprintf(TEXT("[Servidor](CreateNamedPipe) '%s' envia instantaneamente \n"), PIPE_N_WRITET);

		wSemEsperarPipeClientes[i] = CreateNamedPipe(PIPE_N_WRITET, PIPE_ACCESS_OUTBOUND, PIPE_WAIT | PIPE_TYPE_MESSAGE
			| PIPE_READMODE_MESSAGE, N_MAX_CLIENTES, sizeof(COMANDO_DO_SERVIDOR), sizeof(COMANDO_DO_SERVIDOR),
			1000, NULL);


		//verifica pipes
		if ((rPipeClientes[i] == INVALID_HANDLE_VALUE) && (wPipeClientes[i] == INVALID_HANDLE_VALUE) && (wSemEsperarPipeClientes[i] == INVALID_HANDLE_VALUE)) {
			_tperror(TEXT("[ERRO] N�o s�o permitidos mais clientes!"));
			exit(-1);
		}		

		
		_tprintf(TEXT("[Servidor](ConnectNamedPipe) Esperar liga��o de um Cliente...\n"));

		if (!ConnectNamedPipe(wPipeClientes[i], NULL)) {
			_tperror(TEXT("Erro na liga��o ao Cliente!"));
			exit(-1);
		}
		
		if (!ConnectNamedPipe(wSemEsperarPipeClientes[i], NULL)) {
			_tperror(TEXT("Erro na liga��o do envio coletivo Cliente!"));
			exit(-1);
		}

		

		ThreadAtendeCli[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AtendeCliente,(LPVOID)i, 0, NULL);
		if (ThreadAtendeCli == NULL) {
			_tprintf(TEXT("[ERRO] ThreadAtendeCli n�o foi lan�ada\n"));
			exit(-1);
		}
		Sleep(200);
		total++;
	}



	for (i = 0; i < total; i++) {
		if (!wPipeClientes[i] == NULL){
		DisconnectNamedPipe(wPipeClientes[i]);
		DisconnectNamedPipe(wSemEsperarPipeClientes[i]);
		_tprintf(TEXT("[Servidor] Vou desligar o pipe %d... (CloseHandle)\n"), i);
		CloseHandle(wPipeClientes[i]);
		CloseHandle(wSemEsperarPipeClientes[i]);
		CloseHandle(ThreadAtendeCli[i]);}
	}	
	return 0;
}

void enviaID(int indice_deste_cliente) 
{
	WaitForSingleObject(hMutexEnviaID, INFINITE);
	
	COMANDO_DO_SERVIDOR cmd;
	DWORD n;

	cmd.jogador.ID = indice_deste_cliente;
	cmd.jogador.pid = _getpid();
	cmd.resposta = 20;

	WriteFile(wPipeClientes[indice_deste_cliente], &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);
	_tprintf(TEXT("[SERVIDOR] Mandei o ID %d...\n"), indice_deste_cliente);

	ReleaseMutex(hMutexEnviaID);
}

DWORD WINAPI AtendeCliente(LPVOID indice)
{
	TCHAR buf[TAM];
	DWORD n;
	BOOL ret;
	int indice_deste_cliente = indice;

	HANDLE rPipe = (HANDLE) rPipeClientes[indice_deste_cliente];	
	COMANDO_DO_CLIENTE cmd_cliente;
	COMANDO_DO_SERVIDOR cmd_servidor;
	int tipoDeResposta;


	_tprintf(TEXT("[SERVIDOR-%d] Um cliente ligou-se... iD %d\n"), GetCurrentThreadId(),indice_deste_cliente);

	//envia ID ao jogador

	enviaID(indice_deste_cliente);
	
	do {
		ret = ReadFile(rPipe, &cmd_cliente, sizeof(cmd_cliente), &n, NULL);

		if (!ret || !n)
			break;

        _tprintf(TEXT("[SERVIDOR] o comando do cliente indice [%d] foi %d \n"),indice_deste_cliente, cmd_cliente.tipoComando);
		//calcular
		TrataComando(&cmd_cliente);
		
		
	} while (!(cmd_cliente.tipoComando == (-1)));


	_tprintf(TEXT("[SERVIDOR-%d] Vou desligar os pipes do Cliente : %s ... \n"), GetCurrentThreadId(), JogadoresOnline[indice_deste_cliente].username);
	if (!DisconnectNamedPipe(wPipeClientes[indice_deste_cliente])) {
		_tperror(TEXT("Erro ao desligar o pipe!"));
		ExitThread(-1);
	}
	if (!DisconnectNamedPipe(wSemEsperarPipeClientes[indice_deste_cliente])) {
		_tperror(TEXT("Erro ao desligar o pipe!"));
		ExitThread(-1);
	}

	CloseHandle(wPipeClientes[indice_deste_cliente]);
	CloseHandle(wSemEsperarPipeClientes[indice_deste_cliente]);
	wPipeClientes[indice_deste_cliente] = NULL;
	wSemEsperarPipeClientes[indice_deste_cliente] = NULL;
	total -= 1;
	//retiraJogadorOnline();
	//retiraJogadorEmJogo();
	return 0;
}

void AdicionaJogadorAoJogo(COMANDO_DO_CLIENTE * c) 
{
	COMANDO_DO_SERVIDOR cmd;
	DWORD n;
	cmd.tipoResposta = 2;
	if(jogadoresEmJogo < (N_MAX_CLIENTES-1))
	{
		if (game.iniciado == 0 && game.criado == 1) //logo esta no estado de espera
		{
	  
			PreencheJogador(c);
			jogadoresEmJogo += 1; //adiciona o jogador			
			cmd.resposta = 1;
		
		 _tcscpy_s(cmd.msg, TAM_MSG, TEXT("Jogador adicionado, favor esperar at� o jogo come�ar"));
		}
		else {
			if (game.iniciado == 1 && game.criado == 1)
			{
				_tcscpy_s(cmd.msg, TAM_MSG, TEXT("Jogo ja come�ou"));
				cmd.resposta = 0;
			}
			else if (game.iniciado == 0 && game.criado == 0) 
			{
				cmd.resposta = 2;
				_tcscpy_s(cmd.msg, TAM_MSG, TEXT("Nenhum jogo ainda criado"));
			}
		}
	}else{
		cmd.resposta = 2;
		_tcscpy_s(cmd.msg, TAM_MSG, TEXT("Limite de jogadores ja atingido"));
	}
	
	WriteFile(wPipeClientes[c->ID], &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);

}

void CriaJogo(COMANDO_DO_CLIENTE *c) 
{
	COMANDO_DO_SERVIDOR cmd;
	DWORD n;
	cmd.tipoResposta = 1; //tipo para a resposta de jogo

	if (game.criado == 0) 
	{
		game.criado = 1; //jogo criado
		CriaMundo();

		PreencheJogador(c); //adiciona o jogador, visto que � o primeiro a criar o jogo	
		jogadoresEmJogo += 1;

		cmd.resposta = 1;
		_tcscpy_s(cmd.msg, TAM_MSG, TEXT("Jogo criado!"));
	}
	else 
	{
		if (game.criado == 1)
		{			
			cmd.resposta = 0;
			_tcscpy_s(cmd.msg, TAM_MSG, TEXT("Jogo ja criado"));
		}else 
		{
			cmd.resposta = 0;
			_tcscpy_s(cmd.msg, TAM_MSG, TEXT("Manuten��o aguarde por favor"));
		}
	}

	WriteFile(wPipeClientes[c->ID], &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);
	_tprintf(TEXT("\n[SERVIDOR_CRIARJOGO] Enviei %d bytes ao cliente [%d] a resposta %d...\n"), n, c->ID, c->resposta);
}

void ComecaJogo(COMANDO_DO_CLIENTE *c)
{
	int i;
	COMANDO_DO_SERVIDOR cmd;
	DWORD n;

	//add monstros 	
		TCHAR monstroPath[100];
		BLOCO mole;
		POSICAO p;
		int resp;

		ZeroMemory(&si, sizeof(si));
		ZeroMemory(&pi, sizeof(pi));

		for (i = 0; i<10; i++) {
			si[i].cb = sizeof(si);
			//si[i].dwFlags = STARTF_USESHOWWINDOW;
			//si[i].wShowWindow = SW_HIDE;
		}

		game.dificuldade = 1;

        WaitForSingleObject(hMutexMapa, INFINITE);

		switch (game.dificuldade)//Coloca pe�as mediante difuculdade escolhida
		{
		case 1: //coloca em jogo 3 inimigos 
        
			for (i = 0; i<0; i++) {
				do {
					CalculaPosicaoAleatoria(&p);
					resp = VerificaExisteAlgoPosicao(&p);
				} while (resp != 0);

				_stprintf_s(monstroPath, 100, TEXT("C:\\Users\\Sergio\\Desktop\\DungeonRPG\\DungeonRPG\\monstro\\Debug\\monstro.exe %d %d 1"), p.x, p.y);
				Sleep(500);

				if (!CreateProcess(NULL, monstroPath, NULL, NULL, 0, CREATE_NEW_CONSOLE, NULL, NULL, &si[i], &pi[i])) {
					MessageBox(NULL, _T("Unable to create process."), _T("Error"), MB_OK);
					break;
				}
				else {//tipo 1

				
						ptrMapa->mundo[p.x][p.y].monstro.saude = 10;
						ptrMapa->mundo[p.x][p.y].monstro.ataque = 5;
						ptrMapa->mundo[p.x][p.y].monstro.defesa = 5;
						ptrMapa->mundo[p.x][p.y].monstro.lentidao = 4;
						ptrMapa->mundo[p.x][p.y].monstro.raioDeVisao = 3;
						ptrMapa->mundo[p.x][p.y].monstro.limSaude = 16;
					
				}
			}

		/*	for (i = 0; i<1; i++) {
				do {
					CalculaPosicaoAleatoria(&p);
					resp = VerificaExisteAlgoPosicao(&p);
				} while (resp != 0);

				_stprintf_s(monstroPath, 100, TEXT("C:\\Users\\Sergio\\Desktop\\DungeonRPG\\DungeonRPG\\monstro\\Debug\\monstro.exe %d %d 2"), p.x, p.y);
				Sleep(500);

				if (!CreateProcess(NULL, monstroPath, NULL, NULL, 0, CREATE_NEW_CONSOLE, NULL, NULL, &si[i], &pi[i])) {
					MessageBox(NULL, _T("Unable to create process."), _T("Error"), MB_OK);
					break;
				}
				else {//tipo 2

					ptrMapa->mundo[p.x][p.y].monstro.saude = 15;
					ptrMapa->mundo[p.x][p.y].monstro.ataque = 10;
					ptrMapa->mundo[p.x][p.y].monstro.defesa = 10;
					ptrMapa->mundo[p.x][p.y].monstro.lentidao = 15;
					ptrMapa->mundo[p.x][p.y].monstro.raioDeVisao = 2;
					ptrMapa->mundo[p.x][p.y].monstro.limSaude = 24;
				}
			}
			*/


			break;
		case 2:
			break;
		default:
			break;
		}
	
	ReleaseMutex(hMutexMapa);


	game.iniciado = 1; //jogo passa a estar a decorrer
	cmd.tipoResposta = 1;

	_tcscpy_s(cmd.msg, TAM_MSG, TEXT("Jogo come�ou"));
	WriteFile(wPipeClientes[c->ID], &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL); //responde ao jogador que criou o jogo
	
	_tprintf(TEXT("[SERVIDOR] Enviei %d bytes ao cliente [%d]...\n"), n, c->ID);

	MandarMensagens(c, 1);
	AbrirJogoATodosClientesEmJogo();
}

void MandarMensagens(COMANDO_DO_CLIENTE *c, int tipo_mensagem)
{
	int i;
	COMANDO_DO_SERVIDOR cmd;
	DWORD n;
	TCHAR buf[200];
	cmd.tipoResposta = 0;

	if(tipo_mensagem == 1) //mensagens ja pr� definidas
	{

	/*_tcscat_s(buf, TAM_MSG,TEXT("O cliente "));
	_tcscat_s(buf, TAM_MSG, (TEXT("%s"), JogadoresOnline[c->ID].username));
	_tcscat_s(buf, TAM_MSG, TEXT(" iniciou o jogo"));
	_tcscpy_s(cmd.msg, TAM_MSG, buf);*/
		_tcscpy_s(cmd.msg, TAM_MSG, TEXT("Come�ou o jogo! "));
	for (i = 0; i < N_MAX_CLIENTES; i++)
	{
		if (wSemEsperarPipeClientes[i] != NULL && i != c->ID)
			WriteFile(wSemEsperarPipeClientes[i], &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);
	}
	}
	else if (tipo_mensagem == 2) //mensagens do utilizador
	{
		_tcscpy_s(cmd.msg, TAM_MSG,(TEXT("%s disse: %s "),JogadoresOnline[c->ID].username, c->msg));

		for (i = 0; i < N_MAX_CLIENTES; i++)
		{
			if (wSemEsperarPipeClientes[i] != NULL && i != c->ID)
				WriteFile(wSemEsperarPipeClientes[i], &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);
		}
	}
}

void AbrirJogoATodosClientesEmJogo() { // o que faz � actualizar a var em jogo aos clientes que estao a espera de jogar
	int i;
	COMANDO_DO_SERVIDOR cmd;
	DWORD n;

	cmd.tipoResposta = 1;
	cmd.resposta = 1;
	for (i = 0; i < N_MAX_CLIENTES; i++)
	{
		_tprintf(TEXT("[SERVIDOR] ENVIEI"));
		if (wSemEsperarPipeClientes[i] != NULL && JogadoresOnline[i].Ajogar == 1) {
			WriteFile(wSemEsperarPipeClientes[i], &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);
		}
	}
}

void zeramonstros() 
{
	int i, j;
	for (i = 0; i < LT; i++) {			//criacao do mundo a funcionar
		for (j = 0; j < CT; j++) {
			ptrMapa->mundo[i][j].monstro.presente = 0;			
		}
	}
}

void zerajogadores()
{
	int i, j;
	for (i = 0; i < LT; i++) {			//criacao do mundo a funcionar
		for (j = 0; j < CT; j++) {
			ptrMapa->mundo[i][j].jogador = semjogador;
		}
	}
}


void apanhaObjeto() {
	int i, j;

	for (i = 0; i < LT; i++) {
		for (j = 0; j < CT; j++) {

			if (ptrMapa->mundo[i][j].jogador.presente == 1) {
				if (ptrMapa->mundo[i][j].objeto.presente == 1) {  // se for do tipo pedra, guarda
					if (ptrMapa->mundo[i][j].objeto.tipo == 0)
						ptrMapa->mundo[i][j].jogador.nPedras++;
					else if (ptrMapa->mundo[i][j].objeto.tipo == 1) {
						ptrMapa->mundo[i][j].jogador.limSaude = 20;
						if (ptrMapa->mundo[i][j].jogador.saude<ptrMapa->mundo[i][j].jogador.limSaude)
							ptrMapa->mundo[i][j].jogador.saude++;

					}
					else if (ptrMapa->mundo[i][j].objeto.tipo == 2) {
						ptrMapa->mundo[i][j].jogador.limSaude = 20;
						if (ptrMapa->mundo[i][j].jogador.saude<ptrMapa->mundo[i][j].jogador.limSaude)
							ptrMapa->mundo[i][j].jogador.saude += 3;
					}
					else if (ptrMapa->mundo[i][j].objeto.tipo == 3) {

					}
					ptrMapa->mundo[i][j].objeto.presente = 0;
				}

			}
		}
	}
}