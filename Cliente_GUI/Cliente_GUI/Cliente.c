/* ===================================================== */
/* BASE.C                                                */
/* Programa base (esqueleto) para aplicações Windows     */
/* ===================================================== */
// Cria uma janela de nome "Janela Principal" e pinta fundo
// de branco

// Modelo para programas Windows:
//  Composto por 2 funções: 
//	   WinMain() = Ponto de entrada dos programas windows
//			1) Define, cria e mostra a janela
//			2) Loop de recepção de mensagens provenientes do Windows
//     WinProc() = Processamentos da janela (pode ter outro nome)
//			1) É chamada pelo Windows (callback) 
//			2) Executa código em função da mensagem recebida

//	   WinMain()
//	1. Definir características de uma classe de janela
//  2. Registar a classe no Windows NT
//  3. Criar uma janela dessa classe
//  4. Mostrar a janela
//  5. Iniciar a execução do loop de mensagens
//    
//     WinProc()
//  1. Switch em função da mensagem recebida do Windows

// ============================================================================
// Início do programa
// ============================================================================
// Este header tem de se incluir sempre porque define os protótipos das funções 
// do Windows API e os tipos usados na programação Windows
#include <windows.h>
#include <tchar.h>
#include "C:\Users\Sergio\Desktop\DungeonRPG\DungeonRPG\Cliente_GUI\Cliente_GUI\resource.h"
#include "struct.h"

// Pré-declaração da função WndProc (a que executa os procedimentos da janela por
// "callback") 

#define PIPE_N_WRITE TEXT("\\\\.\\pipe\\ParaServidor")
#define PIPE_N_READT TEXT("\\\\.\\pipe\\ParaCLienteMsgInstant")
#define PIPE_N_READ TEXT("\\\\.\\pipe\\ParaCliente")

HANDLE wPipe, rPipe, rrPipe;


int Autenticar(TCHAR *login, TCHAR *pass, TCHAR * ip);
int Registar(TCHAR *login, TCHAR *pass);
int Cria_Jogo();
int Comecar_jogo();

BOOL CALLBACK funcaotrataLogin(HWND hDlg, UINT m, WPARAM w, LPARAM l);
BOOL CALLBACK funcaotrataLogin(HWND hDlg, UINT m, WPARAM w, LPARAM l);
BOOL CALLBACK funcaotrataCriarJogoECamecar(HWND hDlg, UINT m, WPARAM w, LPARAM l);
LRESULT CALLBACK TrataEventos(HWND, UINT, WPARAM, LPARAM);




HANDLE  hMutexEspera;



int aEsperaDeJogar = 0;

DWORD WINAPI RecebeDoServidor(LPVOID param);
void Inicia_comunicacao();


int emjogo = 0;
int JogoCriado = 0;
int EsperaDeComeçar = 0;
int fezlogin = 0;
int indice = 0;
int pid = 0;

static HMENU hMenu;
//void imprimeMundo(COMANDO_DO_SERVIDOR  c);
//void imprimeJogador(COMANDO_DO_SERVIDOR cmd);


int y = 0, x = 0;
// Nome da classe da janela (para programas de uma só janela, normalmente este 
// nome é igual ao do próprio programa)
// "szprogName" é usado mais abaixo na definição das propriedades 
// da classe da janela
TCHAR *szProgName = TEXT("Base");

// ============================================================================
// FUNÇÂO DE INÍCIO DO PROGRAMA: WinMain()
// ============================================================================
// Em Windows, o programa começa sempre a sua execução na função WinMain()
// que desempenha o papel da função main() do C em modo consola
// WINAPI indica o "tipo da função" (WINAPI para todas as declaradas nos headers
// do Windows e CALLBACK para as funções de processamento da janela)
// Parâmetros:
//   hInst: Gerado pelo Windows, é o handle (número) da instância deste programa 
//   hPrevInst: Gerado pelo Windows, é sempre NULL para o NT (era usado no Windows 3.1)
//   lpCmdLine: Gerado pelo Windows, é um ponteiro para uma string terminada por 0
//              destinada a conter parâmetros para o programa 
//   nCmdShow:  Parâmetro que especifica o modo de exibição da janela (usado em  
//				ShowWindow()

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst,
	LPSTR lpCmdLine, int nCmdShow) {
	HWND hWnd;			// hWnd é o handler da janela, gerado mais abaixo 
						// por CreateWindow()
	MSG lpMsg;			// MSG é uma estrutura definida no Windows para as mensagens
	WNDCLASSEX wcApp;	// WNDCLASSEX é uma estrutura cujos membros servem para 
						// definir as características da classe da janela

						// ============================================================================
						// 1. Definição das características da janela "wcApp" 
						//    (Valores dos elementos da estrutura "wcApp" do tipo WNDCLASSEX)
						// ============================================================================

	wcApp.cbSize = sizeof(WNDCLASSEX);	// Tamanho da estrutura WNDCLASSEX
	wcApp.hInstance = hInst;			// Instância da janela actualmente exibida 
										// ("hInst" é parâmetro de WinMain e vem 
										// inicializada daí)
	wcApp.lpszClassName = szProgName;	// Nome da janela (neste caso = nome do programa)
	wcApp.lpfnWndProc = TrataEventos;		// Endereço da função de processamento da janela 
											// ("WndProc" foi declarada no início e encontra-se

											// mais abaixo)

	wcApp.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	//wcApp.hIcon = LoadIcon(hInit)


	wcApp.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;			// Estilo da janela: Fazer o redraw
																// se for modificada horizontal ou
																// verticalmente
																/*OUTROS ESTILOS POSSíVEIS: CS_DBLCLKS			permite a captura de duplo cliques do rato
																CS_NOCLOSE			retira o 'X' na barra para fechar a aplicação*/

	wcApp.hIcon = LoadIcon(NULL, IDI_APPLICATION);	// "hIcon" = handler do ícon normal
													// "NULL" = Icon definido no Windows
													// "IDI_AP..." Ícone "aplicação"
	wcApp.hIconSm = LoadIcon(NULL, IDI_INFORMATION);	// "hIcon" = handler do ícon pequeno
														// "NULL" = Icon definido no Windows
														// "IDI_WIN..." Ícon "Wind.NT logo"
														/*OUTROS TIPOS DE ICONES:	IDI_ASTERISK		Same as IDI_INFORMATION.
														IDI_ERROR			Hand-shaped icon.
														IDI_EXCLAMATION		Same as IDI_WARNING.
														IDI_HAND			Same as IDI_ERROR.
														IDI_INFORMATION		Asterisk icon.
														IDI_QUESTION		Question mark icon.
														IDI_WARNING			Exclamation point icon*/

	wcApp.hCursor = LoadCursor(NULL, IDC_NO);	// "hCursor" = handler do cursor (rato)
												// "NULL" = Forma definida no Windows
												// "IDC_ARROW" Aspecto "seta" 
												/*OUTROS TIPOS DE CURSORES:	IDC_CROSS IDC_HAND IDC_HELP IDC_UPARROW IDC_WAIT */



												//wcApp.lpszMenuName = NULL;						// Classe do menu que a janela pode ter
												// (NULL = não tem menu)
	wcApp.cbClsExtra = 0;							// Livre, para uso particular
	wcApp.cbWndExtra = 0;							// Livre, para uso particular

	wcApp.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH); // "hbrBackground" = handler para 
															   // "brush" de pintura do fundo da
															   // janela. Devolvido por
															   // "GetStockObject". Neste caso o
															   // fundo vai ser branco
															   /*OUTRAS CORES DE BRUSH:	BLACK_BRUSH  DKGRAY_BRUSH GRAY_BRUSH LTGRAY_BRUSH  */

															   // ============================================================================
															   // 2. Registar a classe "wcApp" no Windows
															   // ============================================================================
	if (!RegisterClassEx(&wcApp))
		return(0);


	// ============================================================================
	// 3. Criar a janela
	// ============================================================================
	hWnd = CreateWindow(
		szProgName,				// Nome da janela (programa) definido acima
		TEXT("Exemplo de Janela Principal em C"),	// Texto que figura na barra da janela
		WS_OVERLAPPEDWINDOW,	// Estilo da janela (WS_OVERLAPPED= normal)
								// Outros valores: WS_HSCROLL, WS_VSCROLL
								// (Fazer o OR "|" do que se pretender)
		CW_USEDEFAULT,			// Posição x pixels (default=à direita da última)
		CW_USEDEFAULT,			// Posição y pixels (default=abaixo da última)
		CW_USEDEFAULT,			// Largura da janela (em pixels)
		CW_USEDEFAULT,			// Altura da janela (em pixels)
		(HWND)HWND_DESKTOP,	// handle da janela pai (se se criar uma a partir 
							// de outra) ou HWND_DESKTOP se a janela for
							// a primeira, criada a partir do "desktop"
		(HMENU)NULL,			// handle do menu da janela (se tiver menu)
		(HINSTANCE)hInst,			// handle da instância do programa actual
									// ("hInst" é declarado num dos parâmetros
									// de WinMain(), valor atribuído pelo Windows)
		0);			// Não há parâmetros adicionais para a janela

					// ============================================================================
					// 4. Mostrar a janela
					// ============================================================================
	ShowWindow(hWnd, nCmdShow);	// "hWnd"= handler da janela, devolvido 
								// por "CreateWindow"; "nCmdShow"= modo de
								// exibição (p.e. normal, modal); é passado
								// como parâmetro de WinMain()

	UpdateWindow(hWnd);			// Refrescar a janela (Windows envia à janela
								// uma mensagem para pintar, mostrar dados,
								// (refrescar), etc)

								// ============================================================================
								// 5. Loop de Mensagens
								// ============================================================================
								// O Windows envia mensagens às janelas (programas). Estas mensagens ficam numa
								// fila de espera até que GetMessage(...) possa ler "a mensagem seguinte"	

								// Parâmetros de "getMessage":
								//  1)	"&lpMsg"=Endereço de uma estrutura do tipo MSG ("MSG lpMsg" ja foi 
								//		declarada no início de WinMain()):
								/*			HWND hwnd		handler da janela a que se destina a mensagem
								UINT message	Identificador da mensagem
								WPARAM wParam	Parâmetro, p.e. código da tecla premida
								LPARAM lParam	Parâmetro, p.e. se ALT também estava premida
								DWORD time		Hora a que a mensagem foi enviada pelo Windows
								POINT pt		Localização do mouse (x, y)
								2)   handle da window para a qual se pretendem receber mensagens
								(=NULL se se pretendem receber as mensagens para todas as janelas
								pertencentes ao thread actual)
								3)	 Código limite inferior das mensganes que se pretendem receber
								4)   Código limite superior das mensagens que se pretendem receber
								*/

								// NOTA: GetMessage() devolve 0 quando for recebida a mensagem de fecho da janela,
								// 	     terminando então o loop de recepção de mensagens, e o programa 

	EnableMenuItem(hMenu, ID_JOGO_CRIARJOGO, MF_GRAYED);
	EnableMenuItem(hMenu, ID_JOGO_ENTRAREMJOGO, MF_GRAYED);
	

	Inicia_comunicacao();

	//Invocar a thread que recebe info do servidor
	CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RecebeDoServidor, (LPVOID)rrPipe, 0, NULL);
	

	while (GetMessage(&lpMsg, NULL, 0, 0)) {
		TranslateMessage(&lpMsg);			// Pré-processamento da mensagem
											// p.e. obter código ASCII da tecla
											// premida
		DispatchMessage(&lpMsg);			// Enviar a mensagem traduzida de volta
											// ao Windows, que aguarda até que a 
											// possa reenviar à função de tratamento
											// da janela, CALLBACK TrataEventos (mais 
											// abaixo)
	}
	/*
	//É mais SEGURO o seguinte ciclo de recepção de mensagens, para saber se houve um erro
	BOOL bRet;
	while( (bRet = GetMessage( &lpMsg, NULL, 0, 0 )) != 0)
	{
	if (bRet == -1)
	{
	// handle the error and possibly exit
	}
	else
	{
	TranslateMessage(&lpMsg);
	DispatchMessage(&lpMsg);
	}
	}*/
	// ============================================================================
	// 6. Fim do programa
	// ============================================================================
	return((int)lpMsg.wParam);		// Retorna-se sempre o parâmetro "wParam" da
									// estrutura "lpMsg"
}


BOOL CALLBACK funcaotrataLogin(HWND hDlg, UINT m, WPARAM w, LPARAM l)
{
	TCHAR utilizador[100];
	int i, percorre;
	int resposta;

	switch (m)
	{
	case WM_CLOSE:
		EndDialog(hDlg, 1);
		return 1;

	case WM_COMMAND:
		if (w == IDOKL) {
			GetDlgItemText(hDlg, IDC_EDITUtilizador_LG, utilizador, 100);
		
			
			resposta = Autenticar(utilizador, "0000", "ip");
			
			if (resposta == 1)  //entra em jogo
			{
				fezlogin = 1;
				EnableMenuItem(hMenu, ID_AUTENTICAR_LOGIN, MF_GRAYED);
				EnableMenuItem(hMenu, ID_JOGO_CRIARJOGO, MF_ENABLED);
				EnableMenuItem(hMenu, ID_JOGO_ENTRAREMJOGO, MF_ENABLED);
				
				MessageBox(hDlg, TEXT("fez login !"), TEXT("Mensagem"), MB_OK);
				return 1;
			}else if (resposta == 2)  //entra em jogo
			{
				
				MessageBox(hDlg, TEXT("Utilizador nao registado!"), TEXT("Mensagem"), MB_OK);
			}else if (resposta == 3)  //entra em jogo
			{
				
				MessageBox(hDlg, TEXT("Esse user já esta em jogo!"), TEXT("Mensagem"), MB_OK);
			}

		
	
		}

		break;

	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_EDITUtilizador_LG, TEXT("Utilizador"));
		SetDlgItemText(hDlg, IDC_EDITPassword_LG, TEXT("Password"));

		
		return 1;
	}

	return 0;
}

BOOL CALLBACK funcaotrataRegistar(HWND hDlg, UINT m, WPARAM w, LPARAM l)
{
	TCHAR utilizador[100];
	int resposta;

	switch (m)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_EDITUtilizador_RG, TEXT("Utilizador"));
		SetDlgItemText(hDlg, IDC_EDITPassword_RG, TEXT("Password"));
		

		return 1;

	case WM_COMMAND:
		switch (LOWORD(w))
		{
		case IDOKR:
			GetDlgItemText(hDlg, IDC_EDITUtilizador_RG, utilizador, TAM_LOG);

			resposta = Registar(utilizador, "0000");

			switch(resposta){
			case 0:

				MessageBox(hDlg, TEXT("registou com sucesso !"), TEXT("Mensagem"), MB_OK);
				EnableMenuItem(hMenu, ID_AUTENTICAR_REGISTAR, MF_GRAYED);
				//EndDialog(hDlg, 0);
				break;
			case 3:  
				MessageBox(hDlg, TEXT("Utilizador ja existe! !"), TEXT("Mensagem"), MB_OK);
				break;

			default:
				MessageBox(hDlg, TEXT("Exedeu o numero de inscritos !"), TEXT("Mensagem"), MB_OK);
			}
			
			return 1;

		case IDCANCELR:
			EndDialog(hDlg, 0);
			return 1;

		}
		return 1;
			
	case WM_CLOSE:
		EndDialog(hDlg, 0);
		return 1;
	}

	return 0;
}

BOOL CALLBACK funcaotrataCriarJogoECamecar(HWND hDlg, UINT m, WPARAM w, LPARAM l)
{
	int resposta;


	switch (m)
	{
	case WM_COMMAND:
		switch (LOWORD(w))
		{
		case ID_CriarJogo:
			

			resposta = Cria_Jogo();

			switch (resposta) {
			case 1:

				//ShowWindow(IDC_BUTTON_Comecar, SW_SHOW);//mostrar botao
				ShowWindow(GetDlgItem(hDlg, ID_CriarJogo), SW_SHOW);
				ShowWindow(GetDlgItem(hDlg, IDC_BUTTON_Comecar), SW_SHOW);
				//ShowWindow(IDCANCELCJ, SW_HIDE);
				MessageBox(hDlg, TEXT("Jogo criado!"), TEXT("Mensagem"), MB_OK);
				break;
			case 0:
				MessageBox(hDlg, TEXT("Jogo ja criado!"), TEXT("Mensagem"), MB_OK);
				break;

			default:
				MessageBox(hDlg, TEXT("Jogo ja criado"), TEXT("Mensagem"), MB_OK);
			}

			return 1;

		case IDC_BUTTON_Comecar:
			 Comecar_jogo();

		case IDCANCELCJ:
			EndDialog(hDlg, 0);
			return 1;

		}
		return 1;

	case WM_CLOSE:
		EndDialog(hDlg, 0);
		return 1;
}

return 0;

}

BOOL CALLBACK funcaotrataEntrarEmJogo(HWND hDlg, UINT m, WPARAM w, LPARAM l)
{
}

// ============================================================================
// FUNÇÂO DE PROCESSAMENTO DA JANELA
// Esta função pode ter um nome qualquer: Apenas é necesário que na 
// inicialização da estrutura "wcApp", feita no início de WinMain(), se 
// identifique essa função. Neste caso "wcApp.lpfnWndProc = WndProc"
//
// WndProc recebe as mensagens enviadas pelo Windows (depois de lidas e pré-
// processadas no loop "while" da função WinMain()
//
// Parâmetros:
//		hWnd	O handler da janela, obtido no CreateWindow()
//		messg	Ponteiro para a estrutura mensagem (ver estrutura em 5. Loop...
//		wParam	O parâmetro wParam da estrutura messg (a mensagem)
//		lParam	O parâmetro lParam desta mesma estrutura
//
// NOTA: Estes parâmetros aparecem aqui directamente acessíveis o que 
//		 simplifica o acesso aos seus valores
//
// A função EndProc é sempre do tipo "switch..." com "cases" que descriminam
// a mensagem recebida e a tratar. Estas mensagens são identificadas por macros
// (p.e. WM_DESTROY, WM_CHAR, WM_KEYDOWN, WM_PAINT...) definidas em windows.h
// (cada WM_... corresponde a um número)
// ============================================================================
LRESULT CALLBACK TrataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	static int x, y;
	static int xi, yi;
	static int xf, yf;
	PAINTSTRUCT p;
	TCHAR texto[100];
	int resposta;
	switch (messg) {

	case WM_CREATE:
		hMenu = GetMenu(hWnd);
		//hdc = GetDC(hWnd);
		
		//ReleaseDC(hWnd, hdc);
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &p);
		_stprintf_s(texto, 100, TEXT("Area afetada«: %d %d a %d %d        "), p.rcPaint.left, p.rcPaint.top, p.rcPaint.right, p.rcPaint.bottom);
		TextOut(hdc, 0, 0, texto, _tcslen(texto));
		Rectangle(hdc, xi, yi, xf, yf);


		for (int i = xi; i<xf; i += 10)
			for (int j = yi; j<yf; j += 10)
				Rectangle(hdc, i, j, (i + 10), (j + 10));
		EndPaint(hWnd, &p);
		break;

		case WM_COMMAND:

		switch (LOWORD(wParam))
		{
		case ID_AUTENTICAR_REGISTAR:
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOGRegistar), hWnd, (DLGPROC)funcaotrataRegistar);
			return 1;

		case ID_AUTENTICAR_LOGIN:
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOGLogin), hWnd, (DLGPROC)funcaotrataLogin);
			return 1;

		case ID_JOGO_CRIARJOGO:
			DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOGCriarJogo), hWnd, (DLGPROC)funcaotrataCriarJogoECamecar);
			return 1;
		}


		break;


	case WM_LBUTTONUP:
		x = LOWORD(lParam);
		y = HIWORD(lParam);
		xf = x;
		yf = y;
		hdc = GetDC(hWnd);
		TextOut(hdc, x, y, TEXT("Botão esquerdo precionado"), _tcslen(TEXT("Botão esquerdo precionado")));
		ReleaseDC(hdc, hWnd);
		break;

	case WM_RBUTTONUP:
		x = LOWORD(lParam);
		y = HIWORD(lParam);
		xf = x;
		yf = y;
		//hdc = GetDC(hWnd);		
		//Rectangle(hdc, xi, yi, xf,yf);
		//ReleaseDC(hdc, hWnd);
		InvalidateRect(hWnd, NULL, 1);//gera wm_paint

		break;

	case WM_RBUTTONDOWN:
		hdc = GetDC(hWnd);
		x = LOWORD(lParam);
		y = HIWORD(lParam);
		xi = x;
		yi = y;
		//TextOut(hdc, xi, yi, TEXT("Botão direito precionado"), _tcslen(TEXT("Botão direiro precionado")));
		ReleaseDC(hdc, hWnd);
		break;

	case WM_RBUTTONDBLCLK:

		hdc = GetDC(hWnd);
		x = LOWORD(lParam);
		y = HIWORD(lParam);
		xi = x;
		yi = y;
		TextOut(hdc, xi, yi, TEXT("double click"), _tcslen(TEXT("doucle click")));
		ReleaseDC(hdc, hWnd);

		break;

	case WM_KEYDOWN:
		hdc = GetDC(hWnd);
		Rectangle(hdc, 0, 0, 100, 200);
		//Elipse(hdc, 150, 150, 250, 300);
		ReleaseDC(hdc, hWnd);
		break;


	case WM_KEYUP:

		hdc = GetDC(hWnd);

		switch (wParam)
		{
		case VK_DOWN:
			y += 5;
			TextOut(hdc, x, y, TEXT("DOWN"), _tcslen(TEXT("DOWN")));
			break;

		case VK_UP:
			y -= 5;
			if (y < 0)
				y = 0;
			TextOut(hdc, x, y, TEXT("UP"), _tcslen(TEXT("UP")));
			break;

		default:
			break;
		}

		ReleaseDC(hdc, hWnd);
		break;

	case WM_CLOSE:

		hdc = GetDC(hWnd);
		TextOut(hdc, 0, 0, TEXT("OLA"), 3);
		ReleaseDC(hdc, hWnd);

		if (MessageBox(hWnd, TEXT("Deseja mesmo sair??"), TEXT("SAIR?"), MB_YESNO | MB_ICONERROR) == IDYES)
			PostQuitMessage(0);
		break;
	case WM_DESTROY:	// Destruir a janela e terminar o programa
						// "PostQuitMessage(Exit Status)"		
		PostQuitMessage(0);
		break;

	default:

		return(DefWindowProc(hWnd, messg, wParam, lParam));
		break;
	}
	return(0);
}

int Autenticar(TCHAR *login, TCHAR *pass, TCHAR *ip) {
	DWORD n;
	COMANDO_DO_CLIENTE cmd1;
	COMANDO_DO_SERVIDOR cmd;
	//int resposta;
	BOOL ret;
	int comando;

	comando = 0;

		_tcscpy_s(cmd1.user.login, TAM_LOG, login);

		cmd1.tipoComando = comando;
		cmd1.ID = indice;


		ret = WriteFile(wPipe, &cmd1, sizeof(COMANDO_DO_CLIENTE), &n, NULL); //manda comando

		ret = ReadFile(rPipe, &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);//recebe resposta

		//_tprintf(TEXT("msg : %s \n"), cmd.msg);


		if (cmd.resposta == 1)  //entra em jogo
		{
			fezlogin = 1;
			//_tprintf(TEXT("[CLIENTE[%d] Fiz login!\n "), indice);
		}

	return cmd.resposta;
}

int Registar(TCHAR *login, TCHAR *pass) {
	DWORD n;
	COMANDO_DO_CLIENTE cmd1;
	COMANDO_DO_SERVIDOR serv;
	int resposta;
	int comando;
	BOOL ret;

	
	comando = 1;
	cmd1.ID = indice;
	cmd1.tipoComando = comando;

	//_stprintf_s(cmd1.user.login, 15, login);
	_tcscpy_s(cmd1.user.login, TAM_LOG, login);

	ret = WriteFile(wPipe, &cmd1, sizeof(COMANDO_DO_CLIENTE), &n, NULL); //manda comando

	ret = ReadFile(rPipe, &serv, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);//recebe resposta
	

	return serv.resposta;
}

void Inicia_comunicacao()
{
	TCHAR buf[256];
	//HANDLE wPipe, rPipe, rrPipe;

	int aEsperaDeJogar = 0;
	BOOL ret;
	DWORD n;
	HANDLE hThread;
	COMANDO_DO_CLIENTE cmd1;
	COMANDO_DO_SERVIDOR cmd;

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

	/*//Invocar a thread que recebe info do servidor
	hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RecebeDoServidor, (LPVOID)rrPipe, 0, NULL);
	if (hThread == NULL) {
		_tprintf(TEXT("[ERRO] Thread não foi lançada\n"));
		exit(-1);
	}
	_tprintf(TEXT("[CLIENTE]Criei a Thread para receber do servidor...\n"));
	*/

	ret = ReadFile(rPipe, &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);
	if (!ret || !n)

		_tprintf(TEXT("[CLIENTE]Recebi %d bytes\n "), n);

	if (cmd.resposta == 20)
	{
		indice = cmd.jogador.ID;
		pid = cmd.jogador.pid;
		_tprintf(TEXT("[CLIENTE[%d]]Recebi o meu ID e o meu pid %d\n "), indice, pid);
	}
}

int Cria_Jogo() 
{    
	DWORD n;
	COMANDO_DO_CLIENTE cmd1;
	COMANDO_DO_SERVIDOR cmd;
	int resposta;
	int comando;
	BOOL ret;


	comando = 2;
	cmd1.ID = indice;
	cmd1.tipoComando = comando;
	_tcscpy_s(cmd1.user.login, TAM_LOG, "sem var");

			ret = WriteFile(wPipe, &cmd1, sizeof(COMANDO_DO_CLIENTE), &n, NULL);

			ret = ReadFile(rPipe, &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);


			return cmd.resposta;
}

int Comecar_jogo() 
{
	DWORD n;
	COMANDO_DO_CLIENTE cmd1;
	COMANDO_DO_SERVIDOR cmd;
	int resposta;
	int comando;
	BOOL ret;


	comando = 4;
	cmd1.ID = indice;
	cmd1.tipoComando = comando;


		ret = WriteFile(wPipe, &cmd1, sizeof(COMANDO_DO_CLIENTE), &n, NULL);


		ret = ReadFile(rPipe, &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);


		return cmd.resposta;
}

DWORD WINAPI RecebeDoServidor(LPVOID param) { //recebe o pipe 
	//HANDLE rPipe = (HANDLE)param; //atribui o pipe que recebe (pipe )
	TCHAR buf[256];
	//COMANDO resposta;
	int i = 0;
	BOOL ret;
	DWORD n;
	COMANDO_DO_SERVIDOR cmd;

	while (1)
	{

		ret = ReadFile(rrPipe, &cmd, sizeof(COMANDO_DO_SERVIDOR), &n, NULL);
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
				//imprimeJogador(cmd);
				//imprimeMundo(cmd);
			}
		}
	}

	return 0;
}

/*
void imprimeJogador(COMANDO_DO_SERVIDOR cmd)
{

	_tprintf(TEXT("\n\nJogador:\n"));
	_tprintf(TEXT("nome: %s  \nid:%d\nsaude: %d \nx=%d, y=%d\n  "), cmd.jogador.username, cmd.jogador.ID, cmd.jogador.saude, cmd.jogador.pos.x, cmd.jogador.pos.y);

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
}*/