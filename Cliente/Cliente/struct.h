#pragma once

#include <Windows.h>
#include <tchar.h>

#define L 10
#define C 10
#define LT 20
#define CT 20
#define TAM_LOG 15
#define TAM_PASS 15
#define T_OBJ 5
#define U_MAX 30
#define MAX_SAUDE 20
#define TAM_MSG 200



typedef struct {
	int x, y;
}POSICAO;

typedef struct {
	int tipo;
	int raridade;
	int presente;
}OBJETO;

typedef struct {
	int tipo;
}BLOCO;

typedef struct {
	int ID;
	int pid;
	int Ajogar;
	TCHAR username[TAM_LOG];
	int presente;
	POSICAO pos;
	int nPedras;
	int lentidao;
	int saude;
	int vidas;
	int pontos;
	OBJETO mochila[T_OBJ];
	int modoAtaque;
	int limSaude;
}JOGADOR;

typedef struct {
	TCHAR login[TAM_LOG];
}USER;

typedef struct {
	int presente;
	TCHAR NOME[TAM_LOG];
	int ataque;
	int defesa;
	int tipo_monstro;
	POSICAO pos;
	int saude;
	int lentidao;
	int raioDeVisao;
	int limSaude;
}MONSTRO;

typedef struct {
	JOGADOR jogador;
	MONSTRO monstro;
	OBJETO objeto;
	BLOCO bloco;
	int parede;
	int obj;
}CELULA;

typedef struct {
	int ID;
	int tipoComando;
	int resposta;
	USER user;
	TCHAR msg[TAM_MSG];
	POSICAO p_monstro;
}COMANDO_DO_CLIENTE;

typedef struct {
	int tipoResposta;
	int resposta;
	TCHAR msg[TAM_MSG];
	JOGADOR jogador;
	CELULA mapa[10][10];
	POSICAO p1;
	POSICAO p2;

}COMANDO_DO_SERVIDOR;

typedef struct {
	MONSTRO monstro;
}ATACAR;






typedef struct {
	CELULA mundo[LT][CT];
}MEMORIA;

/*typedef struct {
JOGADOR jogador;
BOOL monstro;
int objecto;
int parede;
}CELULACLI;
*/
typedef struct {
CELULA mapa[L][C];
}MAPACLIENTE;

typedef struct {
	BOOL criado;
	BOOL iniciado;
	int dificuldade;
}JOGO;
