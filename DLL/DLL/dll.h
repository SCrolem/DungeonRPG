//DLL.h
// O bloco ifdef seguinte é o modo standard de criar macros que tornam a exportação de
// funções e variáveis mais simples. Todos os ficheiros neste projecto DLL são
// compilados com o símbolo DLL_IMP_EXPORTS definido. Este símbolo não deve ser definido
// em nenhum projecto que use a DLL. Desta forma, qualquer outro projecto que inclua este
// este ficheiro irá ver as funções e variáveis DLL_IMP_API como sendo importadas de uma
// DLL.
#include <windows.h>
#include <tchar.h>
//#include "struct.h"

#define PIPE_N_WRITE TEXT("\\\\.\\pipe\\ParaServidor")
#define PIPE_N_READ TEXT("\\\\.\\pipe\\ParaCliente")

//Definir uma constante para facilitar a leitura do protótipo da função
//Este .h deve ser incluído no projecto que o vai usar (modo implícito)
//Esta macro é definida pelo sistema caso estejamos na DLL (<DLL_IMP>_EXPORTS definida)
//ou na app (<DLL_IMP>_EXPORTS não definida) onde DLL_IMP é o nome deste projecto
#ifdef DLL_EXPORTS
#define DLL_IMP_API __declspec(dllexport)
#else
#define DLL_IMP_API __declspec(dllimport)
#endif

extern "C"
{
	extern DLL_IMP_API HANDLE wPipe;
	extern DLL_IMP_API HANDLE rPipe;	

	//Funções a serem exportadas/importadas

	DLL_IMP_API void Inicia_comunicacao();
	
}