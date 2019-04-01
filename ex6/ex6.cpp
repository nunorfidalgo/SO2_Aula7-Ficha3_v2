#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <time.h>

#define MAX 256

#define N 5

DWORD WINAPI ThreadProdutor(LPVOID param);
DWORD WINAPI ThreadConsumidor(LPVOID param);
HANDLE hMutex;
TCHAR frase[MAX];
BOOL nova = 0;

HANDLE	hEvent;

int _tmain(int argc, LPTSTR argv[]) {
	TCHAR resp;
	DWORD threadId;
	HANDLE hThreadProd, hThreadCons[N];
	hMutex = CreateMutex(NULL, FALSE, NULL);
	//UNICODE: Por defeito, a consola Windows não processe caracteres wide.
	//A maneira mais fácil para ter esta funcionalidade é chamar _setmode:

#ifdef UNICODE
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
#endif

	// Event
	hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (hEvent == NULL) {
		_tprintf(TEXT("CreateEvent error: %d\n"), GetLastError());
		return 1;
	}

	_tprintf(TEXT("Lançar threads produtor-consumidor?"));
	_tscanf_s(TEXT("%c"), &resp, 1);
	if (resp == 'S' || resp == 's') {
		hThreadProd = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadProdutor, NULL, 0, &threadId);
		if (hThreadProd != NULL)
			_tprintf(TEXT("Lancei uma thread com id %d\n"), threadId);
		else {
			_tprintf(TEXT("Erro ao criar Thread\n"));
			return -1;
		}

		//Criar as N threads
		for (int i = 0; i < N; i++) {
			hThreadCons[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadConsumidor, NULL, 0, &threadId);
			if (hThreadCons[i] != NULL)
				_tprintf(TEXT("Lancei uma thread com id %d\n"), threadId);
			else {
				_tprintf(TEXT("Erro ao criar Thread\n"));
				return -1;
			}
		}

		WaitForSingleObject(hThreadProd, INFINITE);

		for (int i = 0; i < N - 1; i++) // Workarround....
			SetEvent(hEvent);

		WaitForMultipleObjects(N, hThreadCons, TRUE, INFINITE);
	}
	_tprintf(TEXT("[Thread Principal %d]Finalmente vou terminar..."), GetCurrentThreadId());

	CloseHandle(hEvent);
	CloseHandle(hMutex);
	return 0;
}

DWORD WINAPI ThreadProdutor(LPVOID param) {
	TCHAR strLocal[MAX];
	_tprintf(TEXT("[Produtor]Sou a thread %d e vou começar a trabalhar ...\n Prima\'fim\' para terminar...\n"), GetCurrentThreadId());
	Sleep(100);
	do {
		_fgetts(strLocal, MAX, stdin);
		fflush(stdin);
		WaitForSingleObject(hMutex, INFINITE);
		_tcscpy_s(frase, MAX, strLocal);
		SetEvent(hEvent);
		ReleaseMutex(hMutex);
	} while (_tcsncmp(strLocal, TEXT("fim"), 3));
	return 0;
}

DWORD WINAPI ThreadConsumidor(LPVOID param) {
	TCHAR strLocal[MAX];
	_tprintf(TEXT("[Consumidor]Sou a thread %d e vou começar a trabalhar ...\n"), GetCurrentThreadId());
	do {
		WaitForSingleObject(hEvent, INFINITE);
		WaitForSingleObject(hMutex, INFINITE);
		_tcscpy_s(strLocal, MAX, frase);
		_tprintf(TEXT("\n[Consumidor %d]: %s"), GetCurrentThreadId(), strLocal);
		ReleaseMutex(hMutex);
	} while (_tcsncmp(strLocal, TEXT("fim"), 3));
	return 0;
}