#include "Types.h"
#include "Keyboard.h"
#include "Descriptor.h"

// 함수 선언
void kPrintString( int iX, int iY, const char* pcString );

/**
 *  아래 함수는 C 언어 커널의 시작 부분임
 */
void Main( void )
{
    char vcTemp[2] = {0, };
    BYTE bFlags;
    BYTE bTemp;
    int i = 0;

    kPrintString( 0, 10, "Switch To IA-32e Mode Success~!!" );
    kPrintString( 0, 11, "IA-32e C Language Kernel Start..............[Pass]" );
    kPrintString( 0, 12, "GDT Initialize And Switch For IA-32e Mode...[ ]" );

    kInitializeGDTTableAndTSS();
    kLoadGDTR(GDTR_STARTADDRESS);
    kPrintString(45, 12, "Pass");

    kPrintString(0, 13, "TSS Segment Load............................[    ]");
    kLoadTR(GDT_TSSSEGMENT);
    kPrintString(45, 13, "Pass");

    kPrintString( 0, 14, "IDT Initialize..............................[    ]");
    kInitializeIDTTables(); 
    kLoadIDTR(IDTR_STARTADDRESS);
    kPrintString( 45, 14, "Pass" );




    kPrintString( 0, 12, "MK's Keyboard Activate........................[    ]");
    if(kActivateKeyboard() == TRUE)
    {
        kPrintString(47, 12, "Pass");
        kChangeKeyboardLED(FALSE, FALSE, FALSE);
    }
    else
    {
        kPrintString(45, 12, "Fail");
        while(1);
    }

    while(1)
    {
        if(kIsOutputBufferFull() == TRUE)
        {
            bTemp = kGetkeyboardScanCode();

            if(kConvert_Scan_Code_to_ASCII_Code(bTemp, &(vcTemp[0]), &bFlags) == TRUE)
            {
                if(bFlags & KEY_FLAGS_DOWN)
                {
                    kPrintString(i++, 13, vcTemp);

                    if(vcTemp[0] == '0')
                    {
                        bTemp - bTemp / 0;
                    }
                }
            }
        }
    }
}

/**
 *  문자열을 X, Y 위치에 출력
 */
void kPrintString( int iX, int iY, const char* pcString )
{
    CHARACTER* pstScreen = ( CHARACTER* ) 0xB8000;
    int i;
    
    // X, Y 좌표를 이용해서 문자열을 출력할 어드레스를 계산
    pstScreen += ( iY * 80 ) + iX;

    // NULL이 나올 때까지 문자열 출력
    for( i = 0 ; pcString[ i ] != 0 ; i++ )
    {
        pstScreen[ i ].bCharactor = pcString[ i ];
    }
}