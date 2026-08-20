; Author: Karol Synowiec, Album ID: 479864
global _start
; This program generates a fractal consisting of ASCII (33-126) characters.
; The program's parameter is a number of iterations n between 0 to 2^32 - 1.
; The program reads standard input and validates data.
; First line is a initial sequence that is later transformed.
; Following lines (if present) describe replacament rules.
; After that if performs n iterations of replacements and prints final sequence.

; Constants section.
; System constants.
SYS_READ        equ     0
SYS_WRITE       equ     1
SYS_MMAP        equ     9
SYS_MUNMAP      equ     11
SYS_MREMAP      equ     25
SYS_EXIT        equ     60

; Input, output constants.
STDIN           equ     0
STDOUT          equ     1

; System functions constants.
PROT_READ       equ     1
PROT_WRITE      equ     2
MAP_PRIVATE     equ     2
MAP_ANONYMOUS   equ     32
MREMAP_MAYMOVE  equ     1

; Sizes constants.  
IN_BUF_CAP      equ     4096                    ; Input buffer capacity: 4 KB
INIT_STACK_CAP  equ     1048576                 ; Initial custom stack capacity: 1 MB
OUT_BUF_CAP     equ     65536                   ; Output buffer capacity: 64 KB

; Other constants.
ASCII_LOW       equ     33                      ; Bound given in specification.
ASCII_HIGH      equ     126                     ; Bound given in specification.
NEWLINE         equ     10                      ; '\n' = 10

; Macros section.
; Parameter %1 is the return value.
%macro exit_macro 1
        mov     rdi, %1
        mov     rax, SYS_EXIT
        syscall
%endmacro

; Unitialized memory section.
section .bss
n_iters         resd    1                       ; Number of iterations.

in_buf_ptr      resq    1                       ; Input buffer pointer
in_buf_cap      resq    1                       ; Input buffer capacity
in_buf_len      resq    1                       ; Input buffer length

init_seq_ptr    resq    1                       ; Initial sequence pointer
init_seq_len    resq    1                       ; Initial sequence length

stack_buf       resq    1                       ; Custom stack buffer pointer.
stack_end       resq    1                       ; Custom stack end
stack_cap       resq    1                       ; Custom stack capacity

out_buf         resq    1                       ; Output buffer
out_len         resq    1                       ; Output buffer length

; There is a maximum of 94 replacement rules (ASCII 33-126).
rules           resq    94

; Section with the actual code of the program.
; Working registers:
; rax - Syscall numbers. Also stores values returned by the system.
; rdi - 1st argument (STDIN, STDOUT, exit code in sys_exit).
; rsi - 2nd argument (pointers to input/output/stack buffers).
; rdx - 3rd argument (buffer sizes for read/write, protection flags).
; r10 - 4th argument (mapping flags: MAP_ANONYMOUS, MREMAP_MAYMOVE).
; r8  - 5th argument (file descriptor: -1 for anonymous mmap).
; r9  - 6th argument (memory offset: 0).
; r15 - Custom stack pointer. Used at the end to store the exit code.
; rdi - Stores the ASCII code of the character currently popped from the stack.
; rsi - Stores the current iteration depth extracted from the stack frame.
; rcx - Stores dynamically calculated length of the currently processed rule.
; r8  - Pointer to the start of the replacement string for the matched rule.
; r10 - Used as an iterator scanning memory for the '\n' character to calculate
;       the rule length.
; r11 - Fast base pointer loading the address of the [rel rules] array.
; rbx - Used in I/O procedures.
; rsp - Hardware stack pointer. Used at startup to get argc and argv[1],
;       and in the resize_stack function to safely preserve register states
;       during the sys_mremap syscall.
section .text

_start:
        ; Step 1: Check number of arguments given in rsp.
        mov     rdi, [rsp]
        cmp     rdi, 2
        jne     error_exit

        mov     rsi, [rsp + 16]                 ; rsi = n (as string)

        ; Step 2: Parse n (string to int).
        xor     rax, rax                        ; rax stores n as an integer.
        xor     rcx, rcx                        ; rcx stores number of digits in n.
        mov     r8, 0xFFFFFFFF                  ; r8 holds maximal possible value.
.parse_n_loop:
        movzx   r9, byte [rsi]                  ; r9 now holds single digit of n.
        test    r9, r9
        jz      .parse_n_done                   ; 0 means there is an end of string.
        ; Check if it is a char between '0' and '9'.
        cmp     r9, '0'
        jl      error_exit
        cmp     r9, '9'
        jg      error_exit
        sub     r9, '0'                         ; Convert ASCII to a digit
        imul    rax, 10                         ; Shift alreadt calculated part.
        add     rax, r9                         ; Add last processed digit.
        cmp     rax, r8                         ; Check limits of 2^32 - 1.
        ja      error_exit
        inc     rcx
        inc     rsi
        jmp     .parse_n_loop
.parse_n_done:
        test    rcx, rcx
        jz      error_exit                      ; 0 means we've processed no digits.
        mov     [rel n_iters], eax              ; Save parsed n in memory.

        ; Step 3: Initial allocations.
        ; Allocation of input buffer.
        mov     rsi, IN_BUF_CAP
        call    alloc_mem
        mov     [rel in_buf_ptr], rax
        mov     qword [rel in_buf_cap], IN_BUF_CAP
        ; Allocation of stack.
        mov     rsi, INIT_STACK_CAP
        call    alloc_mem
        mov     [rel stack_buf], rax             ; Save ptr to allocated memory.
        mov     qword [rel stack_cap], INIT_STACK_CAP
        add     rax, INIT_STACK_CAP
        mov     [rel stack_end], rax            ; Save end of stack.
        ; Allocation of output buffer.
        mov     rsi, OUT_BUF_CAP
        call    alloc_mem
        mov     [rel out_buf], rax

.read_loop:
        ; Step 4: Read input.
        mov     rsi, [rel in_buf_ptr]
        add     rsi, [rel in_buf_len]
        mov     rdx, [rel in_buf_cap]
        sub     rdx, [rel in_buf_len]
        mov     rax, SYS_READ
        mov     rdi, STDIN
        syscall
        test    rax, rax
        jl      error_exit                      ; If rax < 0 there was an error.
        jz      .read_eof                       ; If rax == 0 there is end of file.

        add     [rel in_buf_len], rax           ; Increase number of read bytes.
        mov     rcx, [rel in_buf_len]
        cmp     rcx, [rel in_buf_cap]           ; Check if the buffer is full.
        jne     .read_loop
        ; When the buffer is full we need to allocate more memory.
        mov     rdi, [rel in_buf_ptr]
        mov     rsi, [rel in_buf_cap]
        mov     rdx, rsi                        ; rdx = old_cap
        shl     rdx, 1                          ; new_cap = old_cap * 2.
        mov     r10, MREMAP_MAYMOVE
        xor     r8, r8                          ; No additional arguments.
        mov     rax, SYS_MREMAP
        syscall
        test    rax, rax                        ; Check for allocations error.
        jl      error_exit
        mov     [rel in_buf_ptr], rax           ; Update in_buf_ptr
        shl     qword [rel in_buf_cap], 1       ; Update in_buf_cap
        jmp     .read_loop

.read_eof:
        ; When read is finished we prepare it for further parsing.
        mov     rsi, [rel in_buf_ptr]           ; rsi = in_buf_ptr
        mov     rcx, [rel in_buf_len]           ; rcx = in_buf_len
        test    rcx, rcx
        jz      error_exit                      ; If we've parsed no characters.
        xor     rdx, rdx                        ; Index for parsing.

.parse_seq_loop:
        ; Step 5: Parse initial sequence and replacement rules.
        cmp     rdx, rcx                        ; Check if we've reached end.
        jae     error_exit

        movzx   rax, byte [rsi + rdx]           ; Get digit from [base + index].
        inc     rdx
        cmp     rax, NEWLINE                    ; Check if it is '\n'.
        je      .parse_seq_done
        ; Check if char is between 33 and 126.
        cmp     rax, ASCII_LOW
        jl      error_exit
        cmp     rax, ASCII_HIGH
        jg      error_exit
        jmp     .parse_seq_loop

.parse_seq_done:
        ; Save initial sequence parameters.
        mov     rax, [rel in_buf_ptr]           ; Get pointer to initial sequence.
        mov     [rel init_seq_ptr], rax
        mov     rax, rdx                        ; Get length of sequence.
        dec     rax                             ; Exclude '\n' from length.
        mov     [rel init_seq_len], rax         ; Save init_seq_len

.parse_rules_loop:
        cmp     rdx, rcx
        ; If we've processed the whole file we can begin expansion.
        jae     expansion_start

        movzx   rdi, byte [rsi + rdx]           ; Get sign that is to be replaced.
        inc     rdx

        cmp     rdi, NEWLINE
        je      error_exit                      ; Double newline means an error.

        ; Validate range of a character.
        cmp     rdi, ASCII_LOW
        jl      error_exit
        cmp     rdi, ASCII_HIGH
        jg      error_exit

        ; Check for duplicates.
        lea     r11, [rel rules]
        mov     r8, [r11 + rdi * 8 - 264]       ; 264 = 33 * 8, calculates offset.
        test    r8, r8                          ; Check if pointer is empty.
        jnz     error_exit

        mov     r8, rsi
        add     r8, rdx                         ; r8 rembers index of rule in file.

.parse_replacement_loop:
        cmp     rdx, rcx                        ; Check if file is broken.
        jae     error_exit

        movzx   rax, byte [rsi + rdx]           ; Save char of replacement seq.
        inc     rdx

        cmp     rax, NEWLINE
        je      .replacement_done

        ; Validate rule chars.
        cmp     rax, ASCII_LOW
        jl      error_exit
        cmp     rax, ASCII_HIGH
        jg      error_exit
        jmp     .parse_replacement_loop

.replacement_done:
        mov     [r11 + rdi * 8 - 264], r8       ; Save pointer to rule.
        jmp     .parse_rules_loop

expansion_start:
        ; Step 6: Expand signs of the initial sequence.
        mov     r15, [rel stack_buf]
        mov     rcx, [rel init_seq_len]
        test    rcx, rcx
        jz      clean_exit                      ; Exit if sequence is empty.

        mov     rsi, [rel init_seq_ptr]
        mov     edx, dword [rel n_iters]
        lea     r8, [rsi + rcx - 1]             ; r8 = ptr to end of seq.

.push_char:
        ; Push all character from initial sequence onto the stack. 
        mov     al, byte [r8]                   ; Get char from the end of seq.
        mov     byte [r15], al                  ; Save char.
        mov     dword [r15 + 4], edx            ; Save present depth.
        add     r15, 8                          ; Move stack_ptr
        dec     r8                              ; Move index left.
        dec     rcx                             ; Decrease char counter.
        jnz     .push_char

expansion_loop:
        cmp     r15, [rel stack_buf]            ; Check if stack is empty.
        je      clean_exit

        sub     r15, 8                          ; Pop stack.
        movzx   edi, byte [r15]                 ; Get char.
        mov     esi, dword [r15 + 4]            ; Get depth.

        test    esi, esi
        jz      print_char                      ; If depth is 0.

        lea     r11, [rel rules]
        mov     r8, [r11 + rdi * 8 - 264]       ; r8 = rule_ptr.
        test    r8, r8
        jz      print_char                      ; If there is no rule.

        ; Calculate rule length.
        mov     r10, r8                         ; r10 gets ptr to rule.
.find_length:
        cmp     byte [r10], 10                  ; Look for newline char.
        je      .length_found
        inc     r10
        jmp     .find_length
.length_found:
        mov     rcx, r10                        ; rcx = end
        sub     rcx, r8                         ; rcx = end - start

        test    rcx, rcx
        jz      expansion_loop                  ; Rule is empty so we skip it.

        lea     r9, [r15 + rcx * 8]             ; Calculate new end of stack.
        cmp     r9, [rel stack_end]             ; Check if there is enough space.
        jae     resize_stack                    ; Resize if there isn't enough.

do_push:
        dec     esi                             ; new_depth = old_depth - 1
        lea     r10, [r8 + rcx - 1]             ; r10 = ptr to last char of rule.
.push_rule:
        mov     al, byte [r10]                  ; Get last sign of rule.
        mov     byte [r15], al                  ; Save char.
        mov     dword [r15 + 4], esi            ; Save new depth.
        add     r15, 8                          ; Move top of stack.
        dec     r10                             ; Move index left.
        dec     rcx                             ; Decrease rule char counter.
        jnz     .push_rule                      ; Do for all rule characters.

        jmp     expansion_loop

print_char:
        mov     rbx, [rel out_buf]
        mov     rcx, [rel out_len]
        mov     [rbx + rcx], dil                ; [out_buf + counter] = char_to_print
        inc     rcx                             ; Increase counter of characters.
        mov     [rel out_len], rcx              ; Save updated counter.
        cmp     rcx, OUT_BUF_CAP
        ; If output buffer is full we print it.
        je      flush_out_buf
        jmp     expansion_loop

flush_out_buf:
        mov     rdx, rcx                        ; rdx = number of bytes to print
        mov     rsi, [rel out_buf]              ; rsi = ptr to data to print
.write_loop:
        mov     rdi, STDOUT
        mov     rax, SYS_WRITE
        syscall
        test    rax, rax
        jl      error_exit                      ; rax < 0 means there was an error.
        sub     rdx, rax                        ; Update number of bytes to write.
        add     rsi, rax                        ; Move ptr to next batch to write.
        test    rdx, rdx                        ; Check if there is something left.
        jg      .write_loop
        mov     qword [rel out_len], 0          ; Buffer is reseted.
        jmp     expansion_loop

resize_stack:
        ; Secure registers.
        push    r8
        push    rcx
        push    rsi
        ; We need to save offset of r15 from top of stack.
        mov     rax, r15
        sub     rax, [rel stack_buf]
        push    rax

        mov     rsi, [rel stack_cap]
        mov     rdx, rsi
        shl     rdx, 1                          ; new_stack_cap = old_stack_cap * 2

.check_cap:
        mov     r9, [rsp + 16]                  ; rcx = rule_size
        shl     r9, 3                           ; r9 = required space, rcx * 8 bytes
        add     r9, rax                         ; offset + required space
        cmp     r9, rdx
        jbe     .cap_ok                         ; There is enough space.
        shl     rdx, 1                          ; More space is required.
        jmp     .check_cap
.cap_ok:
        mov     [rel stack_cap], rdx            ; Save new capacity.
        mov     rdi, [rel stack_buf]
        mov     r10, MREMAP_MAYMOVE
        xor     r8, r8
        mov     rax, SYS_MREMAP
        syscall

        test    rax, rax
        jl      error_exit                      ; rax < 0 means there was an error.

        mov     [rel stack_buf], rax            ; Save new ptr to stack.
        add     rax, [rel stack_cap]            ; Calculate stack end.
        mov     [rel stack_end], rax            ; Update stack end.

        ; Restore registers.
        pop     rax
        mov     r15, [rel stack_buf]
        add     r15, rax

        pop     rsi
        pop     rcx
        pop     r8
        jmp     do_push

; Helper function to allocate memory.
alloc_mem:
        xor     rdi, rdi                        ; Don't specify place in memory.
        mov     rdx, PROT_READ | PROT_WRITE
        mov     r10, MAP_PRIVATE | MAP_ANONYMOUS
        mov     r8, -1                          ; There is no descriptor of file.
        xor     r9, r9                          ; Offset in file is 0.
        mov     rax, SYS_MMAP
        syscall

        test    rax, rax
        jl      error_exit                      ; rax < 0 means there was an error.
        ret

clean_exit:
        mov     rcx, [rel out_len]
        test    rcx, rcx                        ; Check if out_buf is empty.
        jz      .print_final_nl                 ; If so print newline.
        ; If not print the last batch.
        mov     rdx, rcx                        ; rdx = number of bytes to write.
        mov     rsi, [rel out_buf]
.flush_res:
        ; Print remainders of output buffer.
        mov     rdi, STDOUT
        mov     rax, SYS_WRITE
        syscall

        test    rax, rax
        jl      error_exit                      ; rax < 0 means there was an error.
        sub     rdx, rax                        ; Update number of bytes to write.
        add     rsi, rax                        ; Update pointer.
        test    rdx, rdx                        ; Check for remainders to print.
        jg      .flush_res

.print_final_nl:
        mov     rbx, [rel out_buf]
        mov     byte [rbx], NEWLINE
        mov     rdi, STDOUT
        mov     rsi, rbx
        mov     rdx, 1                          ; We want to print 1 byte.
        mov     rax, SYS_WRITE
        syscall

        xor     r15, r15                        ; Success means we want 0 in r15.
        jmp     do_cleanup

error_exit:
        ; Exit in case of a failure.
        mov     r15, 1

do_cleanup:
        ; Exit in case of a successful execution.
        exit_macro r15