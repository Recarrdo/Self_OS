[ORG 0x00]
[BITS 16]

SECTION .text

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 코드 영역
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

START:
    mov ax, 0x1000
    mov ds, ax
    mov es, ax

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; A20 게이트를 활성화
    ; BIOS를 이용한 전환이 실패했을 때 시스템 컨트롤 포트로 전환 시도
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ;BIOS 서비스를 사용해서 A20 게이트를 활성화
    mov ax, 0x2401
    int 0x15

    jc .A20GATEERROR
    jmp .A20GATESUCCESS
    
.A20GATEERROR:
    in al, 0x92
    or al, 0x02
    and al, 0xFE
    out 0x92, al
.A20GATESUCCESS:
    cli
    lgdt[GDTR]

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; 보호 모드로 진입
    ; Disable Paging, Disable Cache, Internal FPU, Disable Align Check,
    ; Enable ProtectedMode
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    mov eax, 0x4000003B
    mov cr0, eax

    ;커널 코드 세그먼트를 0x00을 기준으로 하는 것으로 교체하고 EIP의 값을 0x00을 기준으로 재설정
    ; CS 세그먼트 셀렉터: EIP의

    jmp dword 0x08: (PROTECTEDMODE - $$ + 0x10000)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;보호 모드로 진입
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

[BITS 32]
PROTECTEDMODE:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; 스택을 0x00000000 - 0x0000FFFF 영역에 64KB 크기로 생성

    mov ss, ax
    mov esp, 0xFFFE
    mov ebp, 0xFFFE

    ; 화면에 보호 모드로 전환됐다는 메시지

    push(SWITCHSUCCESSMESSAGE - $$ + 0x10000)
    push 2
    push 0
    call PRINTMESSAGE
    add esp, 12

    jmp dword 0x08: 0x10200

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 함수 코드 영역
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 메시지를 출력하는 함수
; 스택에 x 좌표, y 좌표, 문자열

PRINTMESSAGE:
    push ebp
    mov ebp, esp
    push esi
    push edi
    push eax
    push ecx
    push edx

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; X, Y의 좌표로 비디어 메모리의 어드레스를 계산함
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; Y 좌표를 이용해서 먼저 라인 어드레스 구함

    mov eax, dword[ebp + 12]
    mov esi, 160
    mul esi
    mov edi, eax

    ;X 좌표를 이용해서 2를 곱한 후 최종 어드레스 구함

    mov eax, dword[ebp + 8]
    mov esi, 2
    mul esi
    add edi, eax

    ; 출력할 문자열의 어드레스

    mov esi, dword[ebp + 16]

.MESSAGELOOP:
    mov cl, byte[esi]
    cmp cl, 0
    je .MESSAGEEND
    mov byte[edi + 0xB8000], cl
    add esi, 1
    add edi, 2

    jmp .MESSAGELOOP

.MESSAGEEND:
    pop edx
    pop ecx
    pop eax
    pop edi
    pop esi
    pop ebp
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 데이터 영역
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 아래의 데이터들을 8바이트에 맞춰 정렬하기 위해 추가

align 8, db 0

; GDTR의 끝을 8byte로 정렬하기 위해 추가

dw 0x0000

; GDTR 자료구조 정의

GDTR:
    dw GDTEND - GDT - 1
    dd(GDT - $$ + 0x10000)

;GDT 테이블 정의

GDT:
    ; 널 디스크립터, 반드시 0으로 초기화
    NULLDescriptor:
        dw 0x0000
        dw 0x0000
        db 0x00
        db 0x00
        db 0x00
        db 0x00

    ; 보호 모드 커널용 코드 세그먼트 디스크립터

    CODEDESCRIPTOR:
        dw 0xFFFF
        dw 0x0000
        db 0x00
        db 0x9A
        db 0xCF
        db 0x00

    ; 보호 모드 커널용 데이터 세그먼트 디스크립터

    DATADESCRIPTOR:
        dw 0xFFFF
        dw 0x0000
        db 0x00
        db 0x92
        db 0xCF
        db 0x00

GDTEND:


; 보호 모드로 전환됐다는 메시지
SWITCHSUCCESSMESSAGE: db 'Switch To Protected Mode Success~!!', 0

times 512 - ($ - $$) db 0x00
