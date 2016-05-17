#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#include "struct.h"


#define PIPE_N_READ TEXT("\\\\.\\pipe\\ParaServidor")
#define PIPE_N_WRITE TEXT("\\\\.\\pipe\\ParaCliente")

#define N_MAX_CLIENTES 10
#define TAM 256



USER users[U_MAX] = { NULL };
//USER usersOnline[U_MAX] = { NULL };

//MEMORIA *ptrMapa;

TCHAR NomeMemoria[] = TEXT("Mapa Partilhado");
TCHAR NomeMutexMapa[] = TEXT("MapaMUTEX");

HANDLE hMemoria, hMutexMapa, hMutexUsers, hMutexEnviaID;



JOGADOR semjogador;

JOGADOR JogadoresOnline[N_MAX_CLIENTES];
JOGADOR JogadoresRegistados[N_MAX_CLIENTES];


HANDLE wPipeClientes[N_MAX_CLIENTES];
HANDLE rPipeClientes[N_MAX_CLIENTES];
int total = 0;
int obrigatorios = 10;
int logados = 0;
BOOL sair = FALSE;
CELULA mundo[L][C];

DWORD WINAPI RecebeClientes(LPVOID param);
DWORD WINAPI AtendeCliente(LPVOID param);

BOOL encerrar = FALSE;

JOGO game;

// THREADS


//OUTRAS COISAS


void ActualizaOsMapas();
void RegistaUtilizador(COMANDO_DO_CLIENTE *c);			//FEITO
void LoginUser(COMANDO_DO_CLIENTE *c);					
//void CriaJogo(COMANDO_DO_CLIENTE *c);
void CriaMundo(COMANDO_DO_CLIENTE *c);
void PreencheJogador(COMANDO_DO_CLIENTE *c);


void AdicionaJogadorAoJogo(COMANDO_DO_CLIENTE*c);
void IniciaJogo(COMANDO_DO_CLIENTE *c);
void CalculaPosicaoAleatoria(POSICAO *p);
int VerificaExisteAlgoPosicao(POSICAO *p);
int VerificaJogadorNoMapa(TCHAR *nome);
void CriaMapaParaEnvio(/*MAPACLIENTE *mapa*/);


void TrataComando(COMANDO_DO_CLIENTE *c);



void iniciaOsemjogador();
void VerificaJogo(COMANDO_DO_CLIENTE *cmd);
void Move(COMANDO_DO_CLIENTE *cmd);
JOGADOR JogadorMorre(JOGADOR j);
void ApanhaObjecto(POSICAO p, COMANDO_DO_CLIENTE *c);


int _tmain(int argc, LPTSTR argv[]) {
	HANDLE hThread;	
	//iniciaOsemjogador();

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
		_tprintf(TEXT("Jogador[%d] : saude : %d \n lentidao : %d \n x = %d y = %d\n", JogadoresOnline[i].jogador.ID, JogadoresOnline[i].jogador.saude, JogadoresOnline[i].jogador.lentidao, JogadoresOnline[i].jogador.pos.x, JogadoresOnline[i].jogador.pos.y));

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



void CriaMapaParaEnvio(COMANDO_DO_CLIENTE *cmd) {
	int i, j;
	CELULA m[5][5];
	COMANDO_DO_SERVIDOR c;

	int x = 0, y = 0;
	for (i = (JogadoresOnline[cmd->ID].pos.x - 5); i < (JogadoresOnline[cmd->ID].pos.x + 5); i++) {
		for (j = (JogadoresOnline[cmd->ID].pos.y - 5); j < (JogadoresOnline[cmd->ID].pos.y + 5); j++) {
			c.mapa[x][y] = mundo[i][j];
			y++;
		}
		x++;
	}


	// FALTAM AS VERIFICAÇÕES PARA SABER ONDE ESTA E SE NÃO ULTRAPASSA OS LIMITES


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


void CriaMundo(COMANDO_DO_CLIENTE *c) {

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


	_tprintf(TEXT("[SERVIDOR] ID do jogador alterado %d...\n"), Indice);


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
	c.jogador = JogadoresOnline[Indice];

}


void iniciajogo() 
{

}



void enviaRestposta(COMANDO_DO_CLIENTE *cmd,int  tipo) {
	DWORD n;
	COMANDO_DO_SERVIDOR msg_a_enviar;

	msg_a_enviar.resposta = tipo;

	WriteFile(wPipeClientes[cmd->ID], &msg_a_enviar, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);
	_tprintf(TEXT("[SERVIDOR] Enviei %d bytes ao cliente [%d]...\n"), n, cmd->ID);


}

void enviaRestpostamovimento(COMANDO_DO_CLIENTE *cmd, int tipo)
{
	DWORD n;
	int indice = cmd->ID;
	COMANDO_DO_SERVIDOR msg_a_enviar;

	_tprintf(TEXT("[SERVIDOR]VOU enviar ao cliente [%d]...\n"), indice);

	msg_a_enviar.jogador.ID =  JogadoresOnline[indice].ID;
	msg_a_enviar.jogador.lentidao = JogadoresOnline[indice].lentidao;
	msg_a_enviar.jogador.nPedras = JogadoresOnline[indice].nPedras;
	msg_a_enviar.jogador.saude = JogadoresOnline[indice].saude;
	msg_a_enviar.jogador.pos.x = JogadoresOnline[indice].pos.x;
	msg_a_enviar.jogador.pos.y = JogadoresOnline[indice].pos.y;


	_tprintf(TEXT("Jogador[%d] : saude : %d \n lentidao : %d \n x = %d y = %d\n", msg_a_enviar.jogador.ID, msg_a_enviar.jogador.saude, msg_a_enviar.jogador.lentidao, msg_a_enviar.jogador.pos.x, msg_a_enviar.jogador.pos.y));




	msg_a_enviar.resposta = 1;

	WriteFile(wPipeClientes[indice], &msg_a_enviar, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);
	_tprintf(TEXT("[SERVIDOR] Enviei %d bytes ao cliente [%d]...\n"), n, indice);

}

void TrataComando(COMANDO_DO_CLIENTE *cmd) {

	//if(!game.criado){//JOGO NÃO CRIADO E NÃO INICIADO

	switch (cmd->tipoComando)
	{
	case 0:  //AUTENTICAR
		//LoginUtilizador(cmd);
		
		break;

	case 1: //REGISTAR 
		//RegistaUtilizador(cmd);
		break;

	case 2:  // Criar Jogo
		//CriarJogo(cmd);
		CriaMundo(cmd); //so para teste
		//_tprintf(TEXT("[Servidor]ACTUALIZEI MAPAS \n"));
		PreencheJogador(cmd);
	//	WaitForSingleObject(hMutexUsers, INFINITE);
		
		_tprintf(TEXT("[Servidor]CrieiMUNDO \n"));
	//	PreencheJogador(cmd->ID);    //aqui ele ainda nao tem o ID
		_tprintf(TEXT("[Servidor]CrieioJogador \n"));
		//ActualizaOsMapas();
	//	ReleaseMutex(hMutexUsers);
		enviaRestposta(cmd, 1);
		break;

	case 3:  // Juntar Ao Jogo
		//PreencheJogador();
		break;

	case 4:  // Inicia o Jogo
		//IniciaJogo(cmd);
		break;

	case 5:  //move
	case 6:
	case 7:
	case 8:
		_tprintf(TEXT("[Servidor]MOVER \n"));
	

		//PreencheJogador(cmd);
		
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

void LoginUser(COMANDO_DO_CLIENTE *c) {
	int i,flag=0;
	DWORD n;
	COMANDO_DO_SERVIDOR cmd;
	if (logados == 0) {
		_tcscpy_s(JogadoresOnline[logados].username, TAM_LOG,c->user.login);
		_tcscpy_s(cmd.msg,TAM_MSG, "Login feito com sucesso!");
		
	}
	else if (logados > 0 && logados < N_MAX_CLIENTES) {
		for (i = 0; i < logados; i++)
			if (_tcscmp(JogadoresOnline[i].username, c->user.login) == 0) {
				break;
				flag = 1;
			}
		if (flag == 0) {
			_tcscpy_s(JogadoresOnline[logados].username, TAM_LOG, c->user.login);
			_tcscpy_s(cmd.msg, TAM_MSG, "Login feito com sucesso!");
			
		}
	}
	else {
		_tcscpy_s(cmd.msg, TAM_MSG, "Excedeu o numero de utilizadores");
		
	}
	WriteFile(wPipeClientes[i], &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);
	
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


	res = VerificaExisteAlgoPosicao(&posFinal);

	if (res == 0 || res == 3) 
	{   
		JogadoresOnline[indice_jogador].pos = posFinal;
		mundo[posFinal.x][posFinal.y].jogador = JogadoresOnline[indice_jogador];
		mundo[posInicial.x][posInicial.y].jogador = semjogador;
	}

	//caso seja um bloco que parte o que fazer?

	ReleaseMutex(hMutexMapa);
	//ReleaseMutex(hMutexUsers);
	return;
}



void RegistaUtilizador(COMANDO_DO_CLIENTE *c) {
	COMANDO_DO_SERVIDOR cmd;
	int i;
	DWORD n;
	int conta = 0;
	int flag = 0;
	if (total < N_MAX_CLIENTES) {
		do {

			for (i = 0; i < total; i++) {
				if (_tcscmp(c->user.login, JogadoresRegistados[i].username) == 0) {

					flag = 1;
					_tcscpy_s(cmd.msg,TAM_MSG, "Tal username já existe!");
					break;
				}
			}

		} while (_tcscmp(c->user.login, JogadoresRegistados[i].username) != 0);
		if (flag == 0) {
			_tcscpy_s(JogadoresRegistados[total].username,TAM_LOG, c->user.login);
		}
	}
	else{
		_tcscpy_s(cmd.msg,TAM_MSG, "Registado com sucesso");
	}
	WriteFile(wPipeClientes[i], &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);

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

	for (i = 0; i < total; i++) 
	{

		cmd = geraresposta(i);

		WriteFile(wPipeClientes[i], &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);
	}

}

DWORD WINAPI RecebeClientes(LPVOID param) {
	
	int i;
	HANDLE ThreadAtendeCli[N_MAX_CLIENTES];

	while (!sair && total < N_MAX_CLIENTES)
	{
		_tprintf(TEXT("[Servidor](CreateNamedPipe) Copiar pipe '%s' \n"), PIPE_N_WRITE);

		wPipeClientes[total] = CreateNamedPipe(PIPE_N_WRITE, PIPE_ACCESS_OUTBOUND, PIPE_WAIT | PIPE_TYPE_MESSAGE
			| PIPE_READMODE_MESSAGE, N_MAX_CLIENTES,sizeof(COMANDO_DO_SERVIDOR), sizeof(COMANDO_DO_SERVIDOR),
			1000, NULL);		

		_tprintf(TEXT("[Servidor](CreateNamedPipe) Copiar pipe '%s' \n"), PIPE_N_READ);

		rPipeClientes[total] = CreateNamedPipe(PIPE_N_READ, PIPE_ACCESS_INBOUND, PIPE_WAIT | PIPE_TYPE_MESSAGE
			| PIPE_READMODE_MESSAGE, N_MAX_CLIENTES,sizeof(COMANDO_DO_CLIENTE), sizeof(COMANDO_DO_CLIENTE),
			1000, NULL);

		//verifica pipes
		if ((rPipeClientes[total] == INVALID_HANDLE_VALUE) && (wPipeClientes[total] == INVALID_HANDLE_VALUE)) {
			_tperror(TEXT("[ERRO] Não são permitidos mais clientes!"));
			exit(-1);
		}		

		_tprintf(TEXT("[Servidor] Esperar ligação de um cliente... (ConnectNamedPipe)\n"));

		if (!ConnectNamedPipe(wPipeClientes[total], NULL)) {
			_tperror(TEXT("Erro na ligação ao Cliente!"));
			exit(-1);
		}
		

		ThreadAtendeCli[total] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AtendeCliente, (LPVOID)rPipeClientes[total], 0, NULL);
		if (ThreadAtendeCli == NULL) {
			_tprintf(TEXT("[ERRO] ThreadAtendeCli não foi lançada\n"));
			exit(-1);
		}
		Sleep(200);
		total++;
	}

	/*
	//espera que todos os clientes saiam
	for (i = 0; i < total; i++) {
		WaitForSingleObject(ThreadAtendeCli[i], INFINITE);
	}*/


	for (i = 0; i < total; i++) {
		DisconnectNamedPipe(wPipeClientes[i]);
		_tprintf(TEXT("[Servidor] Vou desligar o pipe... (CloseHandle)\n"));
		CloseHandle(wPipeClientes[i]);
		CloseHandle(ThreadAtendeCli[i]);
	}	
	return 0;
}

void enviaID(indice_deste_cliente) 
{
	WaitForSingleObject(hMutexEnviaID, INFINITE);
	
	COMANDO_DO_SERVIDOR cmd;
	DWORD n;

	cmd.jogador.ID = indice_deste_cliente;
	cmd.resposta = 20;

	WriteFile(wPipeClientes[indice_deste_cliente], &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);
	_tprintf(TEXT("[SERVIDOR] Mandei o ID %d...\n"), indice_deste_cliente);

	ReleaseMutex(hMutexEnviaID);
}


DWORD WINAPI AtendeCliente(LPVOID rparam)
{
	TCHAR buf[TAM];
	DWORD n;
	BOOL ret;
	int indice_deste_cliente = total;
	HANDLE rPipe = (HANDLE)rparam;	
	COMANDO_DO_CLIENTE cmd_cliente;
	COMANDO_DO_SERVIDOR cmd_servidor;
	int tipoDeResposta;
	_tprintf(TEXT("[SERVIDOR-%d] Um cliente ligou-se...\n"), GetCurrentThreadId());

	//envia ID ao jogador

	enviaID(indice_deste_cliente);
	
	do {
		ret = ReadFile(rPipe, &cmd_cliente, sizeof(cmd_cliente), &n, NULL);

		if (!ret || !n)
			break;

		_tprintf(TEXT("[SERVIDOR-%d] Sou cliente... (ReadFile)\n"), GetCurrentThreadId());

		//calcular
		TrataComando(&cmd_cliente);
		
		//ActualizaOsMapas();
		
		_tprintf(TEXT("[SERVIDOR] o comando do cliente indice [%d] foi %d \n"),indice_deste_cliente, cmd_cliente.tipoComando);
	} while (!(cmd_cliente.tipoComando == (-1)));


	_tprintf(TEXT("[SERVIDOR-%d] Vou desligar o pipe... (DisconnectNamedPipe/CloseHandle)\n"), GetCurrentThreadId());
	if (!DisconnectNamedPipe(rPipe)) {
		_tperror(TEXT("Erro ao desligar o pipe!"));
		ExitThread(-1);
	}

	CloseHandle(rPipe);
	return 0;
}
