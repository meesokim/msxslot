#include <asm/assembler.h>

.text
.global msxbus_fiq_handler
.global fiq_state
.global cmd_byte
.global addr
.global data_byte
.global mem
.global msxbus_read
.align 4

msxbus_fiq_handler:
    // 레지스터 저장
    push    {r0-r12, lr}

    // FIQ 상태에 따라 처리
    ldr     r0, =fiq_state
    ldr     r1, [r0]
    cmp     r1, #0
    beq     get_cmd
    cmp     r1, #1
    beq     get_addr_low
    cmp     r1, #2
    beq     get_addr_high
    cmp     r1, #3
    beq     get_data

get_cmd:
    // 명령어 바이트 읽기
    bl      gpio_get_value8
    ldr     r1, =cmd_byte
    str     r0, [r1]
    mov     r1, #1
    str     r1, [r0]
    b       fiq_done

get_addr_low:
    // 하위 주소 바이트 읽기
    bl      gpio_get_value8
    ldr     r1, =addr
    strb    r0, [r1]
    mov     r1, #2
    ldr     r0, =fiq_state
    str     r1, [r0]
    b       fiq_done

get_addr_high:
    // 상위 주소 바이트 읽기
    bl      gpio_get_value8
    ldr     r1, =addr
    strb    r0, [r1, #1]
    
    // cmd_byte 확인
    ldr     r2, =cmd_byte
    ldr     r2, [r2]
    and     r2, r2, #0x03    @ 하위 2비트만 사용
    
    cmp     r2, #0           @ Memory Read
    beq     setup_read
    cmp     r2, #1           @ Memory Write
    beq     setup_write
    cmp     r2, #2           @ I/O Read
    beq     setup_read
    cmp     r2, #3           @ I/O Write
    beq     setup_write
    b       fiq_done

setup_read:
    mov     r1, #3
    ldr     r0, =fiq_state
    str     r1, [r0]
    bl      msxbus_read      @ C 함수 호출
    ldr     r1, =data_byte
    str     r0, [r1]         @ 읽은 데이터 저장
    b       fiq_done

setup_write:
    mov     r1, #3
    ldr     r0, =fiq_state
    str     r1, [r0]
    b       get_data         @ 데이터 수신 대기

get_data:
    bl      gpio_get_value8
    ldr     r1, =data_byte
    str     r0, [r1]
    
    // 메모리에 데이터 저장
    ldr     r0, =addr
    ldr     r0, [r0]         @ 전체 주소
    ldr     r1, =mem         @ 메모리 배열 베이스 주소
    ldr     r2, =data_byte
    ldr     r2, [r2]         @ 데이터
    strb    r2, [r1, r0]     @ mem[addr] = data_byte
    
    bl      msxbus_write     @ C 함수 호출
    mov     r1, #0
    ldr     r0, =fiq_state
    str     r1, [r0]
    b       fiq_done
fiq_done:
    // 레지스터 복원 및 리턴
    pop     {r0-r12, lr}
    subs    pc, lr, #4