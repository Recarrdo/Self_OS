#include "Types.h"
#include "AssemblyUtility.h"
#include "Keyboard.h"

///////////////////////////////////////////////////////////
// 키보드 컨트롤러와 키보드 제어에 관련된 함수
///////////////////////////////////////////////////////////
// 출력 버퍼에 수신된 데이터가 있는지 여부를 반환

BOOL kIsOutputBufferFull(void)
{
    if(kInPortByte(0x64) & 0x01)
    {
        return TRUE;
    }
    return FALSE;
}

//입력 버퍼에 프로세서가 쓴 데이턱 남아 있는지 여부를 반환
BOOL kIsInputBufferFull(void)
{
    if(kInPortByte(0x64) & 0x02)
    {
        return TRUE;
    }
    return FALSE;
}

//키보드 활성화
BOOL kActivateKeyboard(void)
{
    int i;
    int j;

    kOutPortByte(0x64, 0xAE);

    for(i = 0; i < 0xFFFF; i++)
    {
        if(kIsInputBufferFull() == FALSE)
        {
            break;
        }
    }

    kOutPortByte(0x60, 0xF4);

    for(j = 0; j < 100; j++)
    {
        for(i = 0; i < 0xFFFF; i++)
        {
            if(kIsOutputBufferFull == TRUE)
            {
                break;
            }
        }

        if(kInPortByte(0x60) == 0xFA)
        {
            return TRUE;
        }
    }
    return FALSE;
}


//키 읽기
BYTE kGetkeyboardScanCode(void)
{
    while(kIsOutputBufferFull() == FALSE)
    {
        ;
    }
    return kInPortByte(0x60);
}


//키보드 LED 전환
BOOL kChangeKeyboardLED(BOOL bCapsLockOn, BOOL bNumLockOn, BOOL bScrollLockOn)
{
    int i, j;

    for(i = 0; i < 0xFFFF; i++)
    {
        if(kIsInputBufferFull() == FALSE)
        {
            break;
        }
    }

    kOutPortByte(0x60, 0xED);
    for(i = 0; i < 0xFFFF; i++)
    {
        if(kIsInputBufferFull() == FALSE)
        {
            break;
        }
    }

    for(j = 0; j < 100; j++)
    {
        for(i = 0; i < 0xFFFF; i++)
        {
            if(kIsOutputBufferFull() == TRUE)
            {
                break;
            }
        }

        if(kInPortByte(0x60) == 0xFA)
        {
            break;
        }
    }

    if(j >= 100)
    {
        return FALSE;
    }

    kOutPortByte(0x60, (bCapsLockOn << 2) | (bNumLockOn << 1) | bScrollLockOn);
    for(i = 0; i < 0xFFFF; i++)
    {
        if(kIsInputBufferFull() == FALSE)
        {
            break;
        }
    }

    for(j = 0; j < 100; j++)
    {
        for(i = 0; i < 0xFFFF; i++)
        {
            if(kIsOutputBufferFull() == TRUE)
            {
                break;
            }
        }

        if(kInPortByte(0x60) == 0xFA)
        {
            break;
        }
    }
    if(j >= 100)
    {
        return FALSE;
    }
    return TRUE;
}


//A20 게이트 활성화
void kEnableA20Gate(void)
{
    BYTE bOutputPortData;
    int i;

    kOutPortByte(0x64, 0xD0);

    for(i = 0; i< 0xFFFF; i++)
    {
        if(kIsOutputBufferFull() == TRUE)
        {
            break;
        }
    }

    bOutputPortData = kInPortByte(0x60);

    bOutputPortData |= 0x01;

    for(i = 0; i < 0xFFFF; i++)
    {
        if(kIsInputBufferFull() == FALSE)
        {
            break;
        }
    }

    kOutPortByte(0x64, 0xD1);
    
    kOutPortByte(0x60, bOutputPortData);
}



//프로세스 리셋
void kReboot(void)
{
    int i;
    for(i = 0; i < 0xFFFF; i++)
    {
        if(kIsInputBufferFull() == FALSE)
        {
            break;
        }
    }

    kOutPortByte(0x64, 0xD1);

    kOutPortByte(0x60, 0x00);

    while(1)
    {
        ;
    }
}





///////////////////////////////////////////////////////////
// 스캔 코드를 ASCII 코드로 변환하는 기능에 관련된 함수들
///////////////////////////////////////////////////////////
// 키보드 상태를 관리하는 키보드 매니저

//키보드 상태를 관리하는 키보드 매니저
static KEYBOARD_MANAGER gs_stKeyboardManager = {0, };


//스캔 코드를 ASCII 코드로 변환하는 테이블
static KEYMAPPING_ENTRY gs_vstKeyMappingTable[KEYMAPPINGTABLEMAXCOUNT] = 
{
    /*0*/ {KEY_NONE, KEY_NONE},
    /*1*/ {KEY_ESC, KEY_ESC},
    /*2*/ {'1', '!'},
    /*3*/ {'2', '!'},
    /*4*/ {'3', '#'},
    /*5*/ {'4', '$'},
    /*6*/ {'5', '%'},
    /*7*/ {'6', '^'},
    /*8*/ {'7', '&'},
    /*9*/ {'8', '*'},
    /*10*/ {'9', '('},
    /*11*/ {'0', ')'},
    /*12*/ {'-', '_'},
    /*13*/ {'=', '+'},
    /*14*/ {KEY_BACKSPACE, KEY_BACKSPACE},
    /*15*/ {KEY_TAB, KEY_TAB},
    /*16*/ {'q', 'Q'},
    /*17*/ {'w', 'W'},
    /*18*/ {'e', 'E'},
    /*19*/ {'r', 'R'},
    /*20*/ {'t', 'T'},
    /*21*/ {'y', 'Y'},
    /*22*/ {'u', 'U'},
    /*23*/ {'i', 'I'},
    /*24*/ {'o', 'O'},
    /*25*/ {'p', 'P'},
    /*26*/ {'[', '{'},
    /*27*/ {']', '}'},
    /*28*/ {'\n', '\n'},
    /*29*/ {KEY_CTRL, KEY_CTRL},
    /*30*/ {'a', 'A'},
    /*31*/ {'s', 'S'},
    /*32*/ {'d', 'D'},
    /*33*/ {'f', 'F'},
    /*34*/ {'g', 'G'},
    /*35*/ {'h', 'H'},
    /*36*/ {'j', 'J'},
    /*37*/ {'k', 'K'},
    /*38*/ {'l', 'L'},
    /*39*/ {';', ':'},
    /*40*/ {'\'', '\"'},
    /*41*/ {'`', '~'},
    /*42*/ {KEY_LSHIFT, KEY_LSHIFT},
    /*43*/ {'\\', '|'},
    /*44*/ {'z', 'Z'},
    /*45*/ {'x', 'X'},
    /*46*/ {'c', 'C'},
    /*47*/ {'v', 'V'},
    /*48*/ {'b', 'B'},
    /*49*/ {'n', 'N'},
    /*50*/ {'m', 'M'},
    /*51*/ {',', '<'},
    /*52*/ {'.', '>'},
    /*53*/ {'/', '?'},
    /*54*/ {KEY_RSHIFT, KEY_RSHIFT},
    /*55*/ {'*', '*'},
    /*56*/ {KEY_LALT, KEY_LALT},
    /*57*/ {' ', ' '},
    /*58*/ {KEY_CAPSLOCK, KEY_CAPSLOCK},
    /*59*/ {KEY_F1, KEY_F1},
    /*60*/ {KEY_F2, KEY_F2},
    /*61*/ {KEY_F3, KEY_F3},
    /*62*/ {KEY_F4, KEY_F4},
    /*63*/ {KEY_F5, KEY_F5},
    /*64*/ {KEY_F6, KEY_F6},
    /*65*/ {KEY_F7, KEY_F7},
    /*66*/ {KEY_F8, KEY_F8},
    /*67*/ {KEY_F9, KEY_F9},
    /*68*/ {KEY_F10, KEY_F10},
    /*69*/ {KEY_NUMLOCK, KEY_NUMLOCK},
    /*70*/ {KEY_SCROLLLOCK, KEY_SCROLLLOCK},


    /*71*/ {KEY_HOME, '7'},
    /*72*/ {KEY_UP, '8'},
    /*73*/ {KEY_PAGEUP, '9'},
    /*74*/ {'-', '-'},
    /*75*/ {KEY_LEFT, '4'},
    /*76*/ {KEY_CENTER, '5'},
    /*77*/ {KEY_RIGHT, '6'},
    /*78*/ {'+', '+'},
    /*79*/ {KEY_END, '1'},
    /*80*/ {KEY_DOWN, '2'},
    /*81*/ {KEY_PAGEDOWN, '3'},
    /*82*/ {KEY_INS, '0'},
    /*83*/ {KEY_DEL, '.'},
    /*84*/ {KEY_NONE, KEY_NONE},
    /*85*/ {KEY_NONE, KEY_NONE},
    /*86*/ {KEY_NONE, KEY_NONE},
    /*87*/ {KEY_F11, KEY_F11},
    /*88*/ {KEY_F12, KEY_F12}
};



//스캔 코드가 알파벳 범위인지 여부를 변환
BOOL KIs_Alphabet_Scan_Code(BYTE bScanCode)
{
    if(('a' <= gs_vstKeyMappingTable[bScanCode].bNormalCode) && (gs_vstKeyMappingTable[bScanCode].bNormalCode <= 'z'))
    {
        return TRUE;
    }
    return FALSE;
}

//숫자 또는 기호 범위인지 여부를 반환
BOOL kIs_Number_Or_Symbol_Scan_Code(BYTE bScanCode)
{
    if((2 <= bScanCode) && (bScanCode <= 53) && (KIs_Alphabet_Scan_Code(bScanCode) == FALSE))
    {
        return TRUE;
    }
    return FALSE;
}

BOOL kIs_Number_Pad_Scan_Code(BYTE bScanCode)
{
    if((71 <= bScanCode) && (bScanCode <= 83))
    {
        return TRUE;
    }
    return FALSE;
}

BOOL kIs_Use_Combined_Code(BYTE bScanCode)
{
    BYTE bDownScanCode;
    BOOL bUseCombinedKey = FALSE;

    bDownScanCode = bScanCode & 0x7F;

    if(KIs_Alphabet_Scan_Code(bDownScanCode) == TRUE)
    {
        if(gs_stKeyboardManager.bShiftDown ^ gs_stKeyboardManager.bCapsLockOn)
        {
            bUseCombinedKey = TRUE;
        }
        else
        {
            bUseCombinedKey = FALSE;
        }
    }

    else if(kIs_Number_Or_Symbol_Scan_Code(bDownScanCode) == TRUE)
    {
        if(gs_stKeyboardManager.bShiftDown == TRUE)
        {
            bUseCombinedKey = TRUE;
        }
        else
        {
            bUseCombinedKey = FALSE;
        }
    }
    //숫자 패드 키라면 num lock 키의 영향을 받음
    //0xE0만 제외하면 확장 키 코드와 숫자 패드의 코드가 겹치므로, 
    //확장 키 코드가 수신되지 않았을 때만 처리 조합된 코드 사용

    else if((kIs_Number_Pad_Scan_Code(bDownScanCode) == TRUE) && (gs_stKeyboardManager.bExtendedCodeIn == FALSE))
    {
        if(gs_stKeyboardManager.bNumLockOn == TRUE)
        {
            bUseCombinedKey = TRUE;
        }
        else
        {
            bUseCombinedKey = FALSE;
        }
    }
    return bUseCombinedKey;
}

//조합키의 상태를 갱신하고 LED 상태도 동기화 함
void Update_Combination_Key_Status_And_LED(BYTE bScanCode)
{
    BOOL bDown;
    BYTE bDownScanCode;
    BOOL bLEDStatusChanges = FALSE;

    if(bScanCode & 0x80)
    {
        bDown = FALSE;
        bDownScanCode = bScanCode & 0x7F;
    }
    else
    {
        bDown = TRUE;
        bDownScanCode = bScanCode;
    }

    //조합 키 검색
    if((bDownScanCode == 42) || (bDownScanCode == 54))
    {
        gs_stKeyboardManager.bShiftDown = bDown;
    }

    //caps lock 키의 스캔코드면, caps lock의 상태 갱신하고 LED 상태 변경
    else if((bDownScanCode == 58) && (bDown == TRUE))
    {
        gs_stKeyboardManager.bCapsLockOn ^= TRUE;
        bLEDStatusChanges = TRUE;
    }

    //NUM lock 키의 스캔 코드이면 num lock의 상태를 갱신하고 LED 상태 변경
    else if((bDownScanCode == 69) && (bDown == TRUE))
    {
        gs_stKeyboardManager.bNumLockOn ^= TRUE;
        bLEDStatusChanges = TRUE;
    }

    //scroll lock 키의 스캔 코드이면 scroll lock의 상태를 갱신하고 LED 상태 변경
    else if((bDownScanCode == 70) && (bDown == TRUE))
    {
        gs_stKeyboardManager.bScrollLockOn ^= TRUE;
        bLEDStatusChanges = TRUE;
    }

    //LED 상태가 변했으면 키보드로 커맨드를 전송하여 LED 변경
    if(bLEDStatusChanges == TRUE)
    {
        kChangeKeyboardLED(gs_stKeyboardManager.bCapsLockOn, gs_stKeyboardManager.bNumLockOn, gs_stKeyboardManager.bScrollLockOn);
    }
}



//스캔 코드를 ASCII 코드로 변환
BOOL kConvert_Scan_Code_to_ASCII_Code(BYTE bScanCode, BYTE* pbASCIICode, BOOL* pbFlags)
{
    BOOL bUseCombinedKey;

    if(gs_stKeyboardManager.iSkipCountForPause > 0)
    {
        gs_stKeyboardManager.iSkipCountForPause--;
        return FALSE;
    }

    if(bScanCode == 0xE1)
    {
        *pbASCIICode = KEY_PAUSE;
        *pbFlags = KEY_FLAGS_DOWN;
        gs_stKeyboardManager.iSkipCountForPause = KEY_SKIPCOUNTFORPAUSE;
        return TRUE;
    }

    else if(bScanCode == 0xE0)
    {
        gs_stKeyboardManager.bExtendedCodeIn = TRUE;
        return FALSE;
    }

    bUseCombinedKey = kIs_Use_Combined_Code(bScanCode);

    if(bUseCombinedKey == TRUE)
    {
        *pbASCIICode = gs_vstKeyMappingTable[bScanCode & 0x7F].bCombinedCode;
    }
    else
    {
        *pbASCIICode = gs_vstKeyMappingTable[bScanCode & 0x7F].bNormalCode;
    }

    if(gs_stKeyboardManager.bExtendedCodeIn == TRUE)
    {
        *pbFlags = KEY_FLAGS_EXTENDEDKEY;
        gs_stKeyboardManager.bExtendedCodeIn = FALSE;
    }
    else
    {
        *pbFlags = 0;
    }

    if((bScanCode & 0x80) == 0)
    {
        *pbFlags |= KEY_FLAGS_DOWN;
    }

    Update_Combination_Key_Status_And_LED(bScanCode);
    return TRUE;
}
