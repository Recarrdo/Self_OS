[ORG 0x00]       
[BITS 16]           

SECTION .text      

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   코드 영역
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
START:
    mov ax, 0x1000             
    mov ds, ax      
    mov es, ax     
    
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; A20 게이트를 활성화
    ; BIOS를 이용한 전환이 실패했을 때 시스템 컨트롤 포트로 전환 시도
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; BIOS 서비스를 사용해서 A20 게이트를 활성화

    mov ax, 0x2401          ; A20 게이트 활성화 서비스 설정
    int 0x15                ; BIOS 인터럽트 서비스 호출

    jc .A20GATEERROR        ; A20 게이트 활성화가 성공했는지 확인
    jmp .A20GATESUCCESS

.A20GATEERROR:
    ; 에러 발생 시, 시스템 컨트롤 포트로 전환 시도
    in al, 0x92    
    or al, 0x02     
    and al, 0xFE    
    out 0x92, al    
    
.A20GATESUCCESS:
    cli             
    lgdt [ GDTR ]   

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; 보호모드로 진입
    ; Disable Paging, Disable Cache, Internal FPU, Disable Align Check, 
    ; Enable ProtectedMode
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    mov eax, 0x4000003B ; PG=0, CD=1, NW=0, AM=0, WP=0, NE=1, ET=1, TS=1, EM=0, MP=1, PE=1
    mov cr0, eax        ; CR0 컨트롤 레지스터에 위에서 저장한 플래그를 설정하여 
                        ; 보호 모드로 전환

    ; 커널 코드 세그먼트를 0x00을 기준으로 하는 것으로 교체하고 EIP의 값을 0x00을 기준으로 재설정
    ; CS 세그먼트 셀렉터 : EIP
    jmp dword 0x18: ( PROTECTEDMODE - $$ + 0x10000 )

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
; 보호 모드로 진입
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
[BITS 32]               ; 이하의 코드는 32비트 코드로 설정
PROTECTEDMODE:
    mov ax, 0x20       
    mov ds, ax         
    mov es, ax          
    mov fs, ax          
    mov gs, ax         
    
    mov ss, ax        
    mov esp, 0xFFFE    
    mov ebp, 0xFFFE     
    
    ; 화면에 보호 모드로 전환되었다는 메시지를 찍는다.
    push ( SWITCHSUCCESSMESSAGE - $$ + 0x10000 )  
    push 2                                          
    push 0                                          
    call PRINTMESSAGE                          
    add esp, 12                                    

    jmp dword 0x18: 0x10200 ; C 언어 커널이 존재하는 0x10200 어드레스로 이동하여 C 언어 커널 수행


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   함수 코드 영역
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 메시지를 출력하는 함수
;   스택에 x 좌표, y 좌표, 문자열
PRINTMESSAGE:
    push ebp        
    mov ebp, esp    
    push esi        
    push edi      
    push eax
    push ecx
    push edx
    
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; X, Y의 좌표로 비디오 메모리의 어드레스를 계산함
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; Y 좌표를 이용해서 먼저 라인 어드레스를 구함
    mov eax, dword [ ebp + 12 ]
    mov esi, 160               
    mul esi                     
    mov edi, eax               
    
    ; X 좌료를 이용해서 2를 곱한 후 최종 어드레스를 구함
    mov eax, dword [ ebp + 8 ]  
    mov esi, 2                 
    mul esi                     
    add edi, eax               
                               
                                
    ; 출력할 문자열의 어드레스      
    mov esi, dword [ ebp + 16 ] 

.MESSAGELOOP:               
    mov cl, byte [ esi ]    
    cmp cl, 0               
    je .MESSAGEEND          
    mov byte [ edi + 0xB8000 ], cl  
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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;   데이터 영역
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 아래의 데이터들을 8byte에 맞춰 정렬하기 위해 추가
align 8, db 0

; GDTR의 끝을 8byte로 정렬하기 위해 추가
dw 0x0000
; GDTR 자료구조 정의
GDTR:
    dw GDTEND - GDT - 1         
    dd ( GDT - $$ + 0x10000 )  

; GDT 테이블 정의
GDT:
    ; 널(NULL) 디스크립터, 반드시 0으로 초기화해야 함
    NULLDescriptor:
        dw 0x0000
        dw 0x0000
        db 0x00
        db 0x00
        db 0x00
        db 0x00

    ; IA-32e 모드 커널용 코드 세그먼트 디스크립터
    IA_32eCODEDESCRIPTOR:     
        dw 0xFFFF       
        dw 0x0000       
        db 0x00         
        db 0x9A         
        db 0xAF        
        db 0x00        
        
    ; IA-32e 모드 커널용 데이터 세그먼트 디스크립터
    IA_32eDATADESCRIPTOR:
        dw 0xFFFF       
        dw 0x0000      
        db 0x00         
        db 0x92         
        db 0xAF         
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

; 보호 모드로 전환되었다는 메시지
SWITCHSUCCESSMESSAGE: db 'Switch To Protected Mode Success~!!', 0

times 512 - ( $ - $$ )  db  0x00    ; 512바이트를 맞추기 위해 남은 부분을 0으로 채움