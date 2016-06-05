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

TCHAR NomeMemoria[] = TEXT("Mapa Partilhado");
TCHAR NomeMutexMapa[] = TEXT("MapaMUTEX");
JOGADOR semjogador;


//MUTEXS
HANDLE hMemoria, hMutexMapa, hMutexUsers, hMutexEnviaID;

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
CELULA mundo[L][C];

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

//criação do jogo e entrada no jogo
void CriaMundo();
void CriaJogo(COMANDO_DO_CLIENTE *c);
void ComecaJogo(COMANDO_DO_CLIENTE *c);


//adição do jogador ao jogo
void AdicionaJogadorAoJogo(COMANDO_DO_CLIENTE *c);
void PreencheJogador(COMANDO_DO_CLIENTE *c);
void CalculaPosicaoAleatoria(POSICAO *p);

//verificações
int VerificaExisteAlgoPosicao(POSICAO *p);
int VerificaJogadorNoMapa(TCHAR *nome); //?????????
void VerificaJogo(COMANDO_DO_CLIENTE *cmd); //??????

//cria info para envio coletivo
void ActualizaOsMapas();
void CriaMapaParaEnvio(COMANDO_DO_CLIENTE *cmd);
void MandarMensagens(COMANDO_DO_CLIENTE *c, int tipo_mensagem);
void AbrirJogoATodosClientesEmJogo();


//trata do comandos
void TrataComando(COMANDO_DO_CLIENTE *c);

//comandos de ação
void Move(COMANDO_DO_CLIENTE *cmd);


//consequencias
JOGADOR JogadorMorre(JOGADOR j);
void ApanhaObjecto(POSICAO p, COMANDO_DO_CLIENTE *c);
void iniciaOsemjogador();

int _tmain(int argc, LPTSTR argv[]) {
	HANDLE hThread;	
	//iniciaOsemjogador();
	game.iniciado = 0;
	game.criado = 0;

	//criação dos mutexes
	hMutexEnviaID = CreateMutex(NULL, FALSE, NULL);


#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	//Invocar a thread que inscreve novos clientes
	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RecebeClientes, NULL, 0, NULL);
	if (hThread == NULL) {
		_tprintf(TEXT("[ERRO] Thread não foi lançada\n"));
		exit(-1);
	}

	//Esperar a thread recebeLeitores terminar
	WaitForSingleObject(hThread, INFINITE);
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
	//COMANDO_DO_SERVIDOR c;
	DWORD n;

	//    cmd->p1.x

	if ((cmd->jogador.pos.x - 5) >= 0)
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
	else cmd->p2.y = C;

	for (i = cmd->p1.x; i < cmd->p2.x; i++) {
		for (j = cmd->p1.y; j <cmd->p2.y; j++) {
			cmd->mapa[x][y] = mundo[i][j];
			y++;
		}
		x++;
	}
	
}

int VerificaExisteAlgoPosicao(POSICAO *p) {

	if (mundo[p->x][p->y].jogador.presente == 1) {
		return 1;	//existe um jogador
	}
	else {
		if (mundo[p->x][p->y].monstro.presente == 1) {
			//if (_tcscmp(ptrMapa->mapa[p->x][p->y].inimigo.nome,TEXT(""))!=0){
			return 2; // existe um inimigo
		}
		else {
			if (mundo[p->x][p->y].bloco.tipo == 1) {
				return 3; // existe um bloco mole
			}
			else {
				if (mundo[p->x][p->y].bloco.tipo == 2) {
					return 4; // existe um bloco duro
				}
				else {
					//if (ptrMapa->mapa[p->x][p->y].bomba == 2)
						//return 5;

				}
			}
		}
	}
	return 0; // está livre
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

	/* --------------------------------------------------------- CRIAÇAO DO MUNDO --------------------------------------------------------*/

	srand((unsigned)time(NULL));

	for (i = 0; i<L; i++) {			//criacao do mundo a funcionar
		for (j = 0; j<C; j++) {
			mundo[i][j].bloco.tipo = 0;

			if (i == 0)								// paredes nas extremidades
				mundo[i][j].bloco.tipo = 1;
			if (i == 9)
				mundo[i][j].bloco.tipo = 1;
			if (j == 0)
				mundo[i][j].bloco.tipo = 1;
			if (j == 9)
				mundo[i][j].bloco.tipo = 1;

			if (mundo[i][j].bloco.tipo != 1) {
				for (g = 0; g < 4; g++) //CRIACAO DE ITEMS
				{
					x = 0 + (rand() % 100);

					if (x >= 0 && x < 10) {
						mundo[i][j].obj = 1;
					}
					else if (x >= 10 && x < 30) {
						mundo[i][j].obj = 2;
					}
					else if (x >= 30 && x < 34) {
						mundo[i][j].obj=3;
						
					}
					else if (x >= 34 && x < 39) {
						mundo[i][j].obj=4;
					}
					else if (x >= 39 && x < 99) {
						mundo[i][j].obj = 0;
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
	JogadoresOnline[Indice].lentidao = 5;
	JogadoresOnline[Indice].saude = 10;
	JogadoresOnline[Indice].presente = 1;  //a verificar
	JogadoresOnline[Indice].pid = _getpid();

	do {
		srand((unsigned)time(NULL));
		x = rand() % 10; 
		y = rand() % 10;

		if (mundo[x][y].bloco.tipo == 0) { 
			POSICAO p; //
			p.x = x;//
			p.y = y;//
			JogadoresOnline[Indice].pos = p;//
			mundo[x][y].jogador = JogadoresOnline[Indice];
		}
	} while (mundo[x][y].bloco.tipo != 0);
	//c.jogador = JogadoresOnline[Indice];
}

void enviaRestpostamovimento(COMANDO_DO_CLIENTE *cmd, int tipo)
{
	DWORD n;
	int indice = cmd->ID;
	COMANDO_DO_SERVIDOR msg_a_enviar;

	_tprintf(TEXT("[SERVIDOR]VOU enviar ao cliente [%d]...\n"), indice);
	int i, j;
	msg_a_enviar.jogador.ID =  JogadoresOnline[indice].ID;
	msg_a_enviar.jogador.lentidao = JogadoresOnline[indice].lentidao;
	msg_a_enviar.jogador.nPedras = JogadoresOnline[indice].nPedras;
	msg_a_enviar.jogador.saude = JogadoresOnline[indice].saude;
	msg_a_enviar.jogador.pos.x = JogadoresOnline[indice].pos.x;
	msg_a_enviar.jogador.pos.y = JogadoresOnline[indice].pos.y;

	msg_a_enviar.resposta = 1;

	CriaMapaParaEnvio(&msg_a_enviar);

	WriteFile(wPipeClientes[indice], &msg_a_enviar, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);
	_tprintf(TEXT("[SERVIDOR] Enviei %d bytes ao cliente [%d]...\n"), n, indice);

}

void TrataComando(COMANDO_DO_CLIENTE *cmd) {

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

		Move(cmd->ID, cmd->tipoComando);		
		enviaRestpostamovimento(cmd, 1);
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
					_tcscpy_s(cmd.msg, TAM_MSG, TEXT("Esse user já esta em jogo!"));
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

void Move(int  indice_jogador, int accao) {
	int res;
	POSICAO posInicial;
	POSICAO posFinal;

	//WaitForSingleObject(hMutexUsers, INFINITE);
	
	posInicial = JogadoresOnline[indice_jogador].pos;

	posFinal = posInicial;

	switch (accao)
	{
	case 5:  //esquerda
		posFinal.x = posInicial.x - 1;
		break;
	case 6: //direita
		posFinal.x = posInicial.x + 1;
		break;
	case 7: //baixo
		posFinal.y = posInicial.y - 1;
		break;
	case 8: //cima
		posFinal.y = posInicial.y + 1;
		break;
	default:
		break;
	}

	if (posFinal.x < 0)
		posFinal.x = 0;
	if (posFinal.y < 0)
		posFinal.y = 0;

	res = VerificaExisteAlgoPosicao(&posFinal);

	if (res == 0 || res == 3) 
	{   
		JogadoresOnline[indice_jogador].pos = posFinal;
		mundo[posFinal.x][posFinal.y].jogador = JogadoresOnline[indice_jogador];
		mundo[posInicial.x][posInicial.y].jogador = semjogador;
	}

	//caso seja um bloco que parte o que fazer?

	ReleaseMutex(hMutexMapa);
	
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
				_tcscpy_s(cmd.msg, TAM_MSG, TEXT("Tal username já existe!"));
				break;
			}
		}
		if (flag == 0) {
			registados+=1;
			_tcscpy_s(JogadoresRegistados[registados].username, TAM_LOG, c->user.login);
			_tcscpy_s(cmd.msg, TAM_MSG, TEXT("Registado com sucesso"));
		}
	}
	else 
		_tcscpy_s(cmd.msg, TAM_MSG, TEXT("Excedeu o numero de inscritos!"));
	
	id = c->ID;
	WriteFile(wPipeClientes[id], &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);

}

COMANDO_DO_SERVIDOR geraresposta(int cliente) //
{
	COMANDO_DO_SERVIDOR cmd;

	cmd.jogador = JogadoresOnline[cliente];
	cmd.resposta = 1;

	return cmd;
}

void ActualizaOsMapas() //apenas actualiza os jogadores por enquanto
{
	int i;
	COMANDO_DO_SERVIDOR cmd;
	DWORD n;

	cmd.tipoResposta = 1;
/*
	for (i = 0; i < N_MAX_CLIENTES; i++)
	{
		if (wSemEsperarPipeClientes[i] != NULL && JogadoresOnline[i].Ajogar == 1){
			//cmd.mapa = CriaMapaParaEnvio(JogadorOnline[i]);
			WriteFile(wSemEsperarPipeClientes[i], &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);
		}
	}*/
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
			_tperror(TEXT("[ERRO] Não são permitidos mais clientes!"));
			exit(-1);
		}		

		
		_tprintf(TEXT("[Servidor](ConnectNamedPipe) Esperar ligação de um Cliente...\n"));

		if (!ConnectNamedPipe(wPipeClientes[i], NULL)) {
			_tperror(TEXT("Erro na ligação ao Cliente!"));
			exit(-1);
		}
		
		if (!ConnectNamedPipe(wSemEsperarPipeClientes[i], NULL)) {
			_tperror(TEXT("Erro na ligação do envio coletivo Cliente!"));
			exit(-1);
		}

		

		ThreadAtendeCli[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AtendeCliente,(LPVOID)i, 0, NULL);
		if (ThreadAtendeCli == NULL) {
			_tprintf(TEXT("[ERRO] ThreadAtendeCli não foi lançada\n"));
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

		_tprintf(TEXT("[SERVIDOR-%d] Sou cliente... (ReadFile)\n"), GetCurrentThreadId());

		//calcular
		TrataComando(&cmd_cliente);
		
		_tprintf(TEXT("[SERVIDOR] o comando do cliente indice [%d] foi %d \n"),indice_deste_cliente, cmd_cliente.tipoComando);
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
		
		 _tcscpy_s(cmd.msg, TAM_MSG, TEXT("Jogador adicionado, favor esperar até o jogo começar"));
		}
		else {
			if (game.iniciado == 1 && game.criado == 1)
			{
				_tcscpy_s(cmd.msg, TAM_MSG, TEXT("Jogo ja começou"));
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

		PreencheJogador(c); //adiciona o jogador, visto que é o primeiro a criar o jogo	
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
			_tcscpy_s(cmd.msg, TAM_MSG, TEXT("Manutenção aguarde por favor"));
		}
	}

	WriteFile(wPipeClientes[c->ID], &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);
	_tprintf(TEXT("[SERVIDOR] Enviei %d bytes ao cliente [%d]...\n"), n, c->ID);
}

void ComecaJogo(COMANDO_DO_CLIENTE *c)
{
	int i;
	COMANDO_DO_SERVIDOR cmd;
	DWORD n;

	cmd.tipoResposta = 1;

	_tcscpy_s(cmd.msg, TAM_MSG, TEXT("Jogo começou"));
	WriteFile(wPipeClientes[c->ID], &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL); //responde ao jogador que criou o jogo
	
	_tprintf(TEXT("[SERVIDOR] Enviei %d bytes ao cliente [%d]...\n"), n, c->ID);

	MandarMensagens(c, 1);
	AbrirJogoATodosClientesEmJogo();

	//ActualizaOsMapas();
}

void MandarMensagens(COMANDO_DO_CLIENTE *c, int tipo_mensagem)
{
	int i;
	COMANDO_DO_SERVIDOR cmd;
	DWORD n;
	cmd.tipoResposta = 0;

	if(tipo_mensagem == 1) //mensagens ja pré definidas
	{
	_tcscpy_s(cmd.msg, TAM_MSG, TEXT("O %s iniciou o jogo"), JogadoresOnline[c->ID].username);

	for (i = 0; i < N_MAX_CLIENTES; i++)
	{
		if (wSemEsperarPipeClientes[i] != NULL && i != c->ID)
			WriteFile(wSemEsperarPipeClientes[i], &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);
	}

	}
	else if (tipo_mensagem == 2) //mensagens do utilizador
	{
		_tcscpy_s(cmd.msg, TAM_MSG, TEXT("%s disse: %s "),JogadoresOnline[c->ID].username, c->msg);

		for (i = 0; i < N_MAX_CLIENTES; i++)
		{
			if (wSemEsperarPipeClientes[i] != NULL && i != c->ID)
				WriteFile(wSemEsperarPipeClientes[i], &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);
		}
	}
}

void AbrirJogoATodosClientesEmJogo() {
	int i;
	COMANDO_DO_SERVIDOR cmd;
	DWORD n;

	cmd.tipoResposta = 1;
	cmd.resposta = 1;
	for (i = 0; i < N_MAX_CLIENTES; i++)
	{
		if (wSemEsperarPipeClientes[i] != NULL && JogadoresOnline[i].Ajogar == 1) {
			WriteFile(wSemEsperarPipeClientes[i], &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);
		}
	}
}