#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <winscard.h>

//#define WINCE_OS
#define LINUX_OS

#define MIFDTEST_VERSION	"1.1"
#define MIFDTEST_DATE	"Feb 17, 2005"

#define MAX_READER_NUMBER	10
#define MAX_RECV_BUFFER_SIZE 4096


#define PRINTD(str)	{ if(debug) printf str;}
#define PRINTERR(str)	{ printf str;}

#ifdef LINUX_OS
#include <unistd.h>
#define TRUE	1
#define FALSE	0
#define Sleep(s) usleep(s * 1000)
#endif
//////////////////////////////////////////////////////////////////////////
BYTE ReadBinaryCommand[] = {0x00, 0xB0, 0x00, 0x00, 0x10};

BYTE ReadMEMCommand[] = {0xFF, 0xB0, 0x00, 0x00, 0x10};

SCARDCONTEXT ScardContext;
DWORD	ReaderNameLength;
DWORD	ReadersNum = 0;
LPTSTR  ReaderName;
SCARD_READERSTATE ScardReaderState[MAX_READER_NUMBER];
SCARDHANDLE ScardHandle[MAX_READER_NUMBER];
SCARD_IO_REQUEST ScardIoRequest[MAX_READER_NUMBER];
DWORD  ScardProtocol[MAX_READER_NUMBER];
int CurrentReader;

BYTE SendBuffer[256+5];
DWORD SendLength;
BYTE RecvBuffer[MAX_RECV_BUFFER_SIZE];
DWORD RecvLength;

int debug = 0;
int isexception = 0;
//////////////////////////////////////////////////////////////////////////

void PrintBuffer(const char* msg, BYTE* buf, DWORD len)
{
	DWORD i;

	PRINTD((msg));
	for(i = 0; i < len; ++i)
	{
		PRINTD(("%02X ", buf[i]));
	}
	PRINTD(("\n"));
}

int Init()
{
	long result;
	int i;	
	LPTSTR readerName;


	result = SCardEstablishContext(SCARD_SCOPE_SYSTEM,
                                 NULL,
                                 NULL,
                                 &ScardContext);

	if(result != SCARD_S_SUCCESS)
	{
		PRINTERR(("SCardEstablishContext Fail : %08X\n", result));
		return 0;
	}
    
	PRINTD(("SCardEstablishContext Success\n"));

	// Find out ReaderNameLength
	result = SCardListReaders(ScardContext, NULL, NULL, &ReaderNameLength);

	if(result != SCARD_S_SUCCESS)
	{
		PRINTERR(("SCardListReadersA Fail : %08X\n", result));
		ReaderNameLength = 100;
		return 0;
	}
    
	PRINTD(("SCardListReaders Success : %d\n", ReaderNameLength));

	// Allcoate Memory for ReaderName
	ReaderName = (LPTSTR)malloc(ReaderNameLength); //new char[ReaderNameLength];    

	// List Reader Name
	ReadersNum = 0;
	result = SCardListReaders(ScardContext,
                              NULL,
                              ReaderName,
                              &ReaderNameLength);

	if(result == SCARD_S_SUCCESS)
	{

		readerName = ReaderName;

		while((*readerName != '\0') && (ReadersNum < MAX_READER_NUMBER))
		{

			ScardReaderState[ReadersNum].szReader = (LPTSTR)readerName;
			ScardReaderState[ReadersNum].dwCurrentState = SCARD_STATE_UNAWARE;
			++ReadersNum;
			readerName += strlen(readerName) + 1;
			//readerName += wcslen(readerName) + 1;
		}
	}
	else
	{
		PRINTERR(("SCardListReadersA Fail : %08X\n", result));

		return 0;
	}

	printf("Reader List (%d Readers): \n", ReadersNum);
	for(i = 0; i < (int)ReadersNum; ++i)
	{
#ifdef WINCE_OS
		wprintf("%d)%s\n", i, ScardReaderState[i].szReader);
#else
		printf("  (%d) %s\n", i, ScardReaderState[i].szReader);
#endif
	}
	printf("\n");

	return 1;
}

long 
MySCardGetStatusChange(
	SCARDCONTEXT hContext,
    DWORD dwTimeout,
    LPSCARD_READERSTATE_A rgReaderStates,
    DWORD cReaders)
{
	long result;
	DWORD timecnt;
	DWORD timeslot;
	
	timecnt = 0;

#ifdef LINUX_OS
	fflush(stdout);
#endif

	if(dwTimeout > 100)
		timeslot = 100;
	else
		timeslot = 1;

	do
	{
		result = SCardGetStatusChange(hContext, dwTimeout, rgReaderStates, cReaders);

		if(result != SCARD_S_SUCCESS)
		{
			PRINTERR(("SCardGetStatusChange Fail : %08X\n", result));
			return result;
		}

		if(((rgReaderStates->dwCurrentState & SCARD_STATE_PRESENT) && !(rgReaderStates->dwEventState & SCARD_STATE_PRESENT)) ||
			(!(rgReaderStates->dwCurrentState & SCARD_STATE_PRESENT) && (rgReaderStates->dwEventState & SCARD_STATE_PRESENT)) )
		{
			break;
		}
		else
		{
			if(dwTimeout)
			{
				Sleep(timeslot);
				timecnt += timeslot;
			}
		}

	} while(timecnt < dwTimeout || dwTimeout == INFINITE);

	return result;
}

int
Test_CardTracking()
{
	int trycnt;
	long result;

	printf("==============================================================\n");
	printf("Part A : Card Insert/Remove\n");
	printf("==============================================================\n");
	trycnt = 0;

	result = MySCardGetStatusChange(ScardContext, 0, &ScardReaderState[CurrentReader], 1);

	if(result != SCARD_S_SUCCESS)
	{
		PRINTERR(("SCardGetStatusChange Fail : %08X\n", result));
		return FALSE;
	}

	if(!(ScardReaderState[CurrentReader].dwEventState & SCARD_STATE_PRESENT))
	{
		printf("<<   Please insert card\n");

		ScardReaderState[CurrentReader].dwCurrentState = SCARD_STATE_EMPTY;
			
		result = MySCardGetStatusChange(ScardContext, INFINITE, &ScardReaderState[CurrentReader], 1);

		if(result != SCARD_S_SUCCESS)
		{
			PRINTERR(("SCardGetStatusChange Fail : %08X\n", result));
			return FALSE;
		}						
	}

	do {

		printf("Please remove card");
		ScardReaderState[CurrentReader].dwCurrentState = SCARD_STATE_PRESENT;
		result = MySCardGetStatusChange(ScardContext, INFINITE, &ScardReaderState[CurrentReader], 1);

		if(result != SCARD_S_SUCCESS)
		{
			printf("                                     Failed\n");
			PRINTERR(("SCardGetStatusChange Fail : %08X\n", result));			
			return FALSE;
		}
		printf("                                     Passed\n");

		printf("Please insert card");
		ScardReaderState[CurrentReader].dwCurrentState = SCARD_STATE_EMPTY;
		result = MySCardGetStatusChange(ScardContext, INFINITE, &ScardReaderState[CurrentReader], 1);

		if(result != SCARD_S_SUCCESS)
		{
			printf("                                     Failed\n");
			PRINTERR(("SCardGetStatusChange Fail : %08X\n", result));
			return FALSE;
		}
		printf("                                     Passed\n");
		
	} while(++trycnt < 3);
	

	printf("\n                Part A Test Successfully\n\n");
	return TRUE;
}

void
MyPrintf(int offset, int c, int times, char* str)
{
	int i;
	int limit = strlen("==============================================================");
	int space;

	space = limit - offset - strlen(str) - c * times;	
	for(i = 0; i < space; ++i)
		printf(" ");
	printf(str);
}

int
Test_CardConnecting()
{
	int trycnt;
	long result;

	printf("==============================================================\n");
	printf("Part B : Card Connect/Disconnect\n");
	printf("==============================================================\n");
	trycnt = 0;
	printf("Testing");
	while(trycnt < 5)
	{
		trycnt++;
		printf("...");
#ifdef LINUX_OS
		fflush(stdout);
#endif
		result = SCardConnect(	 ScardContext,
							 ScardReaderState[CurrentReader].szReader,
							 SCARD_SHARE_EXCLUSIVE,
							 SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
							 &ScardHandle[CurrentReader],
							 &ScardProtocol[CurrentReader]);

		if(result != SCARD_S_SUCCESS)
		{
			MyPrintf(strlen("Testing"), strlen("..."), trycnt * 2 - 1, "Failed\n");
			PRINTERR(("SCardConnect Fail : %08X\n", result));
			return FALSE;
		}
		
		printf("...");
#ifdef LINUX_OS
		fflush(stdout);
#endif
		result = SCardDisconnect(ScardHandle[CurrentReader],
										   SCARD_UNPOWER_CARD);

		if(result != SCARD_S_SUCCESS)
		{
			MyPrintf(strlen("Testing"), strlen("..."), trycnt * 2, "Failed\n");
			PRINTERR(("SCardDisconnect Fail : %08X\n", result));
			return FALSE;
		}
	}
	MyPrintf(strlen("Testing"), strlen("..."), trycnt * 2, "Passed\n");

	printf("\n                Part B Test Successfully\n\n");

	return TRUE;
}

int
Test_CardTransmiting()
{
	int trycnt,i;
	long result;

	printf("==============================================================\n");
	printf("Part C : Card Transmiting\n");
	printf("==============================================================\n");
	trycnt = 0;

	result = SCardConnect(	 ScardContext,
							 ScardReaderState[CurrentReader].szReader,
							 SCARD_SHARE_EXCLUSIVE,
							 SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
							 &ScardHandle[CurrentReader],
							 &ScardProtocol[CurrentReader]);

	if(result != SCARD_S_SUCCESS)
	{		
		PRINTERR(("SCardConnect Fail : %08X\n", result));
		return FALSE;
	}

	printf("Transmit");
	while(trycnt < 10)
	{
		trycnt++;
		ScardIoRequest[CurrentReader].dwProtocol = ScardProtocol[CurrentReader];
		ScardIoRequest[CurrentReader].cbPciLength = sizeof(SCARD_IO_REQUEST);

		RecvLength = MAX_RECV_BUFFER_SIZE;

		printf("...");
#ifdef LINUX_OS
		fflush(stdout);
#endif
		result = SCardTransmit(ScardHandle[CurrentReader],
								&ScardIoRequest[CurrentReader],
								ReadMEMCommand,
								sizeof(ReadMEMCommand),
								&ScardIoRequest[CurrentReader],
								RecvBuffer,
								&RecvLength);

		if(result != SCARD_S_SUCCESS)
		{
			MyPrintf(strlen("Transmit"), strlen("..."), trycnt, "Failed\n");
			PRINTERR(("SCardTransmit Fail : %08X\n", result));
			return FALSE;	
		}


		if(RecvLength != 18)
		{
			isexception = 1;
		}
	}

	if(isexception)
	{
		MyPrintf(strlen("Transmit"), strlen("..."), trycnt, "*Passed\n");
	}
	else
	{
		MyPrintf(strlen("Transmit"), strlen("..."), trycnt, "Passed\n");
	}
	
	printf("\n                Part C Test Successfully\n\n");

	return TRUE;
}

int main(int argc, char** argv)
{
	int i;
	int help_shown = FALSE;
	int reader_selected = FALSE;
	int isdefault = TRUE;
	int isez220pu = FALSE;

	printf("Manufacturer PC/SC IFD Test\n");
	printf("Version : %s\n", MIFDTEST_VERSION);
	printf("Release Date : %s\n", MIFDTEST_DATE);	
	printf("==============================================================\n");

	if(Init())
	{
		if(argc >= 2)
		{
			for(i = 1; i < argc; ++i)
			{

				if(strcmp(argv[i], "-h") == 0)
				{
					if(help_shown == FALSE)
					{
						help_shown = TRUE;

						printf("Argument : \n");
						printf("           [-h] [-ez220pu] CurrentReader\n");
						printf("\n");
						printf("-h : help\n");
						printf("-ez220pu : focus on ez220pu\n\n");
						isdefault = FALSE;	
					
					}
				}
				else if(strcmp(argv[i], "-ez220pu") == 0)
				{
					isez220pu = TRUE;
					isdefault = TRUE;
				}
				else
				{
					sscanf(argv[i], "%d", &CurrentReader);
					reader_selected = TRUE;
					isdefault = TRUE;
				}
			}
		}

		if(!isdefault)
		{
			return 0;
		}
		
		if(reader_selected == FALSE && ReadersNum > 1 && !isez220pu)
		{
			printf("Select : ");
			scanf("%d", &CurrentReader);
		}
		
		if(reader_selected == FALSE && ReadersNum > 2 && isez220pu)
		{
			printf("Select : ");
			scanf("%d", &CurrentReader);
		}

		if(CurrentReader >= (int)ReadersNum)
		{
			printf("Select Out of Range\n");
			return -1;
		}

		if(isez220pu)
		{
			if(strncmp(ScardReaderState[CurrentReader].szReader, "CASTLES EZ220PU", strlen("CASTLES EZ220PU")))
			{
				printf("**Not EZ220PU Reader\n");
				return -1;
			}
		}
		
		if(isez220pu && CurrentReader >= (int)ReadersNum - 1)
		{
			printf("Select Out of Range\n");
			return -1;
		}

		if(Test_CardTracking() && Test_CardConnecting() && Test_CardTransmiting())
		{
			if(!isez220pu)
			{
				printf("\nAll Parts Test Successfully\n\n");
			}
			else
			{
				printf("\n                Test the second card\n");
				CurrentReader++;
				if(Test_CardConnecting() && Test_CardTransmiting())
				{
					printf("\nAll Parts Test Successfully\n\n");
				}
			}
		}

		if(isexception)
		{
			printf("[*] Not default testing card\n\n");
		}
	}

	return 0;
}
