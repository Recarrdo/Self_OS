[ORG 0x00]
[BITS 16]

SECTION .text

jmp 0x1000:START

SECTORCOUNT: dw 0x0000
TOTALSECTORCOUNT equ 1024

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 코드 영역
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

START:
    mov ax, cs 
    mov ds, ax  
    mov ax, 0xB800 

    mov es, ax 

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; 각 섹터 별로 코드를 생성
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    %assign i 0
    %rep TOTALSECTORCOUNT
        %assign i   i + 1

        ; 현재 실행 중인 코드가 포함된 석터의 위치를 화면 좌표로 변환

        mov ax, 2 
        mul word [ SECTORCOUNT ]
        mov si, ax 

        ; 계산된 결과를 비디오 메모리에 오프셋으로 삼아 세 번째 라인부터 화면에 0을 출력
        mov byte [ es: si + ( 160 * 2 ) ], '0' + (i % 10)
        add word [ SECTORCOUNT ], 1 

        ; 마지막 섹터이면 더 수행할 섹터가 없으니 무한 루프 수행, 그렇지 않으면
        ; 다음 섹터로 이동해서 코드 수행
        %if i == TOTALSECTORCOUNT
            jmp $

        %else
            jmp(0x1000 + i * 0x20): 0x0000
        
        %endif
         times (512 - ($ - $$) % 512) db 0x00
    %endrep