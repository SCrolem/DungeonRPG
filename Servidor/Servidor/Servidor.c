#include <windows.h>
#include <tchar.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#include "struct.h"


#define PIPE_N_READ TEXT("\\\\.\\pipe\\ParaServidor")
#define PIPE_N_WRITE TEXT("\\\\.\\pipe\\ParaCliente")





USER users[U_MAX] = { NULL };
//USER usersOnline[U_MAX] = { NULL };

//MEMORIA *ptrMapa;

TCHAR NomeMemoria[] = TEXT("Mapa Partilhado");
TCHAR NomeMutexMapa[] = TEXT("MapaMUTEX");

HANDLE hMemoria, hMutexMapa, hMutexUsers;

#define N_MAX_CLIENTES 10
#define TAM 256

JOGADOR semjogador;
JOGADOR *JogadoresOnline[N_MAX_CLIENTES];

HANDLE wPipeClientes[N_MAX_CLIENTES];
HANDLE rPipeClientes[N_MAX_CLIENTES];
int total = 0;
int obrigatorios = 10;
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
void CriaMapaParaEnvio(MAPACLIENTE *mapa);


void TrataComando(COMANDO_DO_CLIENTE *c);



void iniciaOsemjogador();
void VerificaJogo(COMANDO_DO_CLIENTE *cmd);
void Move(COMANDO_DO_CLIENTE *cmd);
JOGADOR JogadorMorre(JOGADOR j);
void ApanhaObjecto(POSICAO p, COMANDO_DO_CLIENTE *c);


int _tmain(int argc, LPTSTR argv[]) {
	HANDLE hThread;	
	iniciaOsemjogador();

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
	int i,o;
	
	//j.mochila[0]->tipo = 1;
	//j.mochila[0]->raridade
	j.pontos = 0;
	j.lentidao = 0;
	j.saude = 0;
	j.presente = 0;  //a verificar
	p.x = 0;
	p.y = 0;
	j.pos = p;

	WaitForSingleObject(hMutexUsers, INFINITE);
	for (i = 0; i <= N_MAX_CLIENTES; i++) 
	{
		JogadoresOnline[i] = malloc(sizeof(JOGADOR));
		
		for(o = 0; o<T_OBJ;o++)
		JogadoresOnline[i]->mochila[o] = malloc(sizeof(OBJETO));
	
		*JogadoresOnline[i] = j;
	}
	semjogador = j;
		ReleaseMutex(hMutexUsers);
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

/*void CriaMapaParaEnvio(MAPACLIENTE *mapa) {
	int i, j;
	WaitForSingleObject(hMutexMapa, INFINITE);
	for (i = 0; i < L; i++) {
		for (j = 0; j < C; j++) {//para cada posicao
			mapa->mapa[i][j].jogador = ptrMapa->mapa[i][j].jogador;
			mapa->mapa[i][j].monstro = FALSE;
			mapa->mapa[i][j].objecto = 0;


			if (ptrMapa->mapa[i][j].monstro.presente == 1) {
				mapa->mapa[i][j].monstro = TRUE;

			}
			else {
				if (ptrMapa->mapa[i][j].bloco.tipo == 2) {
					mapa->mapa[i][j].parede = ptrMapa->mapa[i][j].bloco.tipo;

				}
				//	
				//	else {
				//		if (ptrMapa->mapa[i][j].bloco.tipo == 1) {
				//			if (ptrMapa->mapa[i][j].bloco. == 0)
				//				mapa->mapa[i][j].bloco = ptrMapa->mapa[i][j].bloco.tipo = 0;
				//			else {
				//				if (ptrMapa->mapa[i][j].bloco.saude == 10)
				//					mapa->mapa[i][j].bloco = ptrMapa->mapa[i][j].bloco.tipo;
				//				else
				//					mapa->mapa[i][j].bloco = 3;
				//			}


				//			//
				//			if(ptrMapa->mapa[i][j].bloco.saude>0)
				//			mapa->mapa[i][j].bloco=ptrMapa->mapa[i][j].bloco.tipo;
				//			else
				//			mapa->mapa[i][j].bloco=ptrMapa->mapa[i][j].bloco.tipo=0;
				//			
				//		}
				//


			}
		}
	
	ReleaseMutex(hMutexMapa);
	}
}*/

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

/*void AdicionaJogadorAoJogo(COMANDO_DO_CLIENTE *c) {
	int resp = 0;
	POSICAO p;


	WaitForSingleObject(hMutexMapa, INFINITE);

	if (!game.criado) {
		c->resposta = 1;
		ReleaseMutex(hMutexMapa);
		return;
	}

	if (game.iniciado) {
		c->resposta = 2;
		ReleaseMutex(hMutexMapa);
		return;
	}

	do {
		CalculaPosicaoAleatoria(&p);
		resp = VerificaExisteAlgoPosicao(&p);

	} while (resp != 0);

	c->jogador.pos = p;
	ptrMapa->mapa[p.x][p.y].jogador = c->jogador;
	//switch (game.dificuldade)
	//{
	//case 1:
	//	c->jogador. = 20;
	//	break;
	//case 2:
	//	c->jogador. = 15;
	//	break;
	//case 3:
	//	c->jogador. = 10;
	//	break;
	//default:
	//	break;
	//}

	ReleaseMutex(hMutexMapa);
	c->resposta = 0;
}*/

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

void PreencheJogador(int Indice) 
{
	int x = 0; 
	int y = 0;

	//c->jogador.nome=            //entao e o nome? nao te esqueças que tens funçoes que usam esta var... (VerificaExisteAlgoPosicao) prefiro a ideia de nessa funçao em vez de verificar pelo nome, seria melhor pela var presente
//	JogadoresOnline[Indice]->mochila[0]->tipo = 1;
//	JogadoresOnline[Indice]->mochila[0]->raridade = 10;//inventei
	JogadoresOnline[Indice]->pontos = 0;
	JogadoresOnline[Indice]->lentidao = 5;
	JogadoresOnline[Indice]->saude = 10;
	JogadoresOnline[Indice]->presente = 1;  //a verificar

	do {
		srand((unsigned)time(NULL));
		x = rand() % 10; 
		y = rand() % 10;



		if (mundo[x][y].bloco.tipo == 0) { //a verificar
			POSICAO p; //
			p.x = x;//
			p.y = y;//
			JogadoresOnline[Indice]->pos = p;//
			mundo[x][y].jogador = *JogadoresOnline[Indice];
		}
	} while (mundo[x][y].bloco.tipo != 0);

}


void iniciajogo() 
{

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

		//_tprintf(TEXT("[Servidor]ACTUALIZEI MAPAS \n"));
		Sleep(400);
		WaitForSingleObject(hMutexUsers, INFINITE);
		CriaMundo(cmd); //so para teste
		_tprintf(TEXT("[Servidor]CrieiMUNDO \n"));
	//	PreencheJogador(cmd->ID);    //aqui ele ainda nao tem o ID
		_tprintf(TEXT("[Servidor]CrieioJogador \n"));
		ActualizaOsMapas();
		ReleaseMutex(hMutexUsers);
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
		PreencheJogador(cmd->ID);
		WaitForSingleObject(hMutexUsers, INFINITE);
		Move(cmd->ID, cmd->tipoComando);
		ReleaseMutex(hMutexUsers);
		break;

	case 10: // verifica jogo
		VerificaJogo(cmd);
		break;
	default:
		break;
	}

	return;
}

/*void LoginUser(COMANDO_DO_CLIENTE *c) {
	int i;
	int conta = 0;

	for (i = 0; i<U_MAX; i++) {
		if (_tcscmp(users[i].login, c->user.login) == 0) {
			if (_tcscmp(users[i].pass, c->user.pass) == 0) {
				//
				for (i = 0; i<U_MAX; i++) {
					if (_tcscmp(usersOnline[i].login, c->user.login) == 0) {
						c->resposta = 3;
						return; //já esta logado
					}
					if (_tcslen(usersOnline[i].login)>0)
						conta++;
				}
				_stprintf_s(usersOnline[conta].login, 15, c->user.login);
				_stprintf_s(usersOnline[conta].pass, 15, c->user.pass);
				_stprintf_s(c->jogador.nome, 15, c->user.login);
				c->jogador.pontos = 0;
				c->jogador.saude = 100;
				c->resposta = 0;
				return; // login com sucesso
			}
			else {
				c->resposta = 2;
				return; // password errada
			}
		}
	}
	c->resposta = 1;
	return; // User não existe
}
*/
void Move(int  indice_jogador, int accao) {
	int res;
	POSICAO posInicial;
	POSICAO posFinal;

	//WaitForSingleObject(hMutexUsers, INFINITE);
	
	posInicial = JogadoresOnline[indice_jogador]->pos;

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

	WaitForSingleObject(hMutexMapa, INFINITE);

	res = VerificaExisteAlgoPosicao(&posFinal);

	if (res == 0 || res == 3) 
	{   
		JogadoresOnline[indice_jogador]->pos = posFinal;
		mundo[posFinal.x][posFinal.y].jogador = *JogadoresOnline[indice_jogador];
		mundo[posInicial.x][posInicial.y].jogador = semjogador;
	}

	//caso seja um bloco que parte o que fazer?

	ReleaseMutex(hMutexMapa);
	//ReleaseMutex(hMutexUsers);
	return;
}



void RegistaUtilizador(COMANDO_DO_CLIENTE *c) {
	int i;
	int conta = 0;

	//WaitForSingleObject(hMutexUsers, INFINITE);

	for (i = 0; i<U_MAX; i++) { // verifica se está vazio ou quantos campos tem

		if (_tcslen(users[i].login)>0)
			conta++;
	}

	for (i = 0; i<conta; i++) {  // Para os campos preenchidos verifica se o login ja foi registado
		if (_tcscmp(users[i].login, c->user.login) == 0) {
			c->resposta = 1; //
			return;
		}
	}
	conta++;
	//se nao estiver ja registado, regista e adiciona à chave de registo
	if (conta < U_MAX) {
		_stprintf_s(users[conta].login, 15, c->user.login);
		_stprintf_s(users[conta].pass, 15, c->user.pass);
	}
	else
		; //envia mensagem de que excedeu o numero de users 

	//RegSetValueEx(regKey, TEXT("LOGINS"), 0, REG_BINARY, (LPBYTE)users, U_MAX * sizeof(UTILIZADOR));
	//ReleaseMutex(hMutexUsers);
	c->resposta = 0;
	return;

}


COMANDO_DO_SERVIDOR geraresposta(int cliente) //
{
	COMANDO_DO_SERVIDOR cmd;

	cmd.jogador = *JogadoresOnline[cliente];
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

	do {
		ret = ReadFile(rPipe, &cmd_cliente, sizeof(cmd_cliente), &n, NULL);

		if (!ret || !n)
			break;

		_tprintf(TEXT("[SERVIDOR-%d] Sou cliente... (ReadFile)\n"), GetCurrentThreadId());

		//calcular
		TrataComando(&cmd_cliente);
		//RespodeAoCliente();
		ActualizaOsMapas();
		
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
