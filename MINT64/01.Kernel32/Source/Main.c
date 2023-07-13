#include "Types.h"
#include "Page.h"



// Assuming that Types.h contains definitions for BOOL, DWORD, and CHARACTER.

void kPrintString(int iX, int iY, const char* pcString);

BOOL kInitializeKernel64Area( void );
BOOL kIsMemoryEnough(void);

//Main 함수
void Main(void)
{   
    DWORD i;
    kPrintString(0, 3, "C Language Kernel Strarted!");

    // 최소 메모리 크기를 만족하는 지 검사
    kPrintString(0, 4, "Checking Memory Size........................[    ]");
    if(kIsMemoryEnough() == FALSE)
    {
        kPrintString(45, 4, "Fail");
        kPrintString(0, 5, "Not Enough Memory~!! MINT64 OS Requires Over");
        while (1);
    }

    else
    {
        kPrintString(45, 4, "Pass");
    }
    
    //IA-32e 모드의 커널 영역을 초기화
    kPrintString(0, 5, "IA-32e Kernel Area Initialize...............[    ]");
    if(kInitializeKernel64Area() == FALSE)
    {
        kPrintString(45, 5, "Fail");
        kPrintString(0, 6, "IA-32e Kernel Area Initialization Fail..");
        while(1);
    }
    kPrintString(45, 5, "Pass");

    //Making a Page Tabel for IA-32e mode kernel
    kPrintString(0, 6, "IA-32e Page Tables Initialize...............[    ]");
    kInitializePageTables();
    kPrintString(45, 6, "Pass");

    while(1);
}

//문자열 출력 함수
void kPrintString(int iX, int iY, const char* pcString)
{
    CHARACTER* pstScreen = (CHARACTER*) 0xB8000;
    int i;

    pstScreen += (iY * 80) + iX;
    for(i = 0; pcString[i] != 0; i++)
    {
        pstScreen[i].bCharactor = pcString[i];
    }
}

//IA-32e 모드용 커널 영역을 0으로 초기화
BOOL kInitializeKernel64Area( void )
{
    DWORD* pdwCurrentAddress;

    pdwCurrentAddress = (DWORD*) 0x100000;

    while((DWORD) pdwCurrentAddress < 0x600000)
    {
        *pdwCurrentAddress = 0x00;
        if(*pdwCurrentAddress != 0)
        {
            return FALSE;
        }
        pdwCurrentAddress++;
    }
    return TRUE;
}

//MINT64 OS를 실행하기에 충분한 메모리를 가지고 있는지 체크
BOOL kIsMemoryEnough(void)
{
    DWORD* pdwCurrentAddress;

    pdwCurrentAddress = (DWORD*) 0x100000;

    while((DWORD) pdwCurrentAddress < 0x4000000)
    {
        *pdwCurrentAddress = 0x12345678;

        if(*pdwCurrentAddress != 0x12345678)
        {
            return FALSE;
        }

        pdwCurrentAddress += (0x100000 / 4);
    }
    return TRUE;
}
