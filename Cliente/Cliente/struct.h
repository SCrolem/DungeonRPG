#pragma once

#include <Windows.h>
#include <tchar.h>

#define L 10
#define C 10
#define TAM_LOG 15
#define TAM_PASS 15
#define T_OBJ 5
#define U_MAX 30

typedef struct {
	int x, y;
}POSICAO;

typedef struct {
	int presente;
	TCHAR nome[TAM_LOG];
	POSICAO pos;
	OBJETO obj[T_OBJ];
	int lentidao;
	int saude;
	int vidas;
	int pontos;
}JOGADOR;

typedef struct {
	TCHAR login[TAM_LOG];
	TCHAR pass[TAM_PASS];
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
}MONSTRO;

typedef struct {
	TCHAR nome[20];
	int tipo;
	int resposta;
	int dificuldade;
	MONSTRO monstro;
	USER user;
	JOGADOR jogador;
}COMANDO;

typedef struct {
	int tipo;
	int raridade;
}OBJETO;

typedef struct {
	int tipo;
}BLOCO;

typedef struct {
	JOGADOR jogador;
	MONSTRO monstro;
	OBJETO objeto;
	BLOCO bloco;
	int parede;
	int obj;
}CELULA;

typedef struct {
	CELULA mapa[L][C];
}MEMORIA;

typedef struct {
	JOGADOR jogador;
	BOOL monstro;
	int objecto;
	int parede;
}CELULACLI;

typedef struct {
	CELULACLI mapa[L][C];
}MAPACLIENTE;

typedef struct {
	BOOL criado;
	BOOL iniciado;
	int dificuldade;
}JOGO;
