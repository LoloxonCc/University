; Author: Karol Synowiec, Album ID: 479864
global arithmetic_sequence

; Function calculates k-th number of an arithmetic sequence.
; Function arguments:
; rdi  - const uint64_t *A0 (pointer to the zero sequence element)
; rsi  - const uint64_t *A1 (pointer to the first sequence element)
; rdx  - uint64_t *Ak       (pointer to the output array)
; rcx  - size_t n           (number of 64-bit words)
; r8   - int64_t k          (the sequence index multiplier)

; Working registers:
; r9   - copy of *Ak pointer (saves it from mul)
; r10  - loop counter (index i)
; r11  - multiplication carry / sign of A0
; xmm0 - sgn(A1-A0) / later lower 64-bit of return struct
; xmm1 - upper 64-bit of return struct / sign extension

; Return value (int128_t):
; rax  - lower 64 bits of the 128-bit result structure
; rdx  - upper 64 bits of the 128-bit result structure

arithmetic_sequence:
        mov     r9, rdx                         ; r9 stores *Ak instead of rdx
        xor     r10, r10                        ; r10 = index i (0), cf = 0

; Step 1: Ak = A1 - A0
.sub_loop:
        mov     rax, [rsi + 8*r10]              ; rax = A1[i]
        sbb     rax, [rdi + 8*r10]              ; rax = A1[i] - A0[i] - cf
        mov     [r9 + 8*r10], rax               ; Ak[i] = A1[i] - A0[i]

        inc     r10
        loop    .sub_loop

        ; Save sign of the result, which can be determined from formula:
        ; sgn(A1-A0) = sgn(A1) - sgn(A0) - cf
        mov     rax, [rsi + 8*r10 - 8]          ; rax = A1[n-1]
        cqo                                     ; rdx = sgn(A1)
        sbb     rdx, 0                          ; rdx = sgn(A1) - cf
        mov     rax, [rdi + 8*r10 - 8]          ; rax = A0[n-1]
        sar     rax, 63                         ; rax = sgn(A0)
        sub     rdx, rax                        ; rdx = sgn(A1) - sgn(A0) - cf
        movq    xmm0, rdx                       ; xmm0.lo = sgn(A1-A0)

        ; After loop r10 = n, rcx = 0 so we restore initial values:
        mov     rcx, r10                        ; rcx = n
        xor     r10, r10                        ; i = 0
        xor     r11, r11                        ; mul_carry = 0

; Step 2: Ak = Ak * k
.mul_loop:
        mov     rax, [r9 + 8*r10]               ; rax = Ak[i]
        mul     r8                              ; rdx:rax = Ak[i] * k

        add     rax, r11                        ; Ak[i] * k.lo += mul_carry
        adc     rdx, 0                          ; Ak[i] * k.hi += cf

        ; If mul_carry is negative we need to extend sign to Ak[i] * k.hi
        test    r11, r11
        jns     .no_sign_ext                    ; jump if r11 >= 0
        dec     rdx                             ; Ak[i] * k.hi--

.no_sign_ext:
        ; To get signed multiplication we need to correct upper 64 bits
        ; by subtracting Ak[i] if k is negative
        test    r8, r8
        jns     .no_k_corr                      ; jump if k >= 0
        sub     rdx, [r9 + 8*r10]               ; Ak[i] * k.hi - Ak[i]

.no_k_corr:
        mov     [r9 + 8*r10], rax
        mov     r11, rdx                        ; update mul_carry

        inc     r10
        loop    .mul_loop

        ; Save the eldest 128 bits of Ak * k
        mov     rax, r11                        ; rax = mul_carry
        movq    rcx, xmm0                       ; rcx = sgn(A1-A0)

        ; If A1-A0>=0 then there is no need for correction,
        ; else we need to subtract Ak[n-1] from mul_carry
        and     rcx, r8
        sub     rax, rcx
        cqo                                     ; rdx = sgn(Ak * k)

        movq    xmm0, rax                       ; xmm0.lo = mul_carry
        movq    xmm1, rdx                       ; xmm1.lo = sgn(Ak * k)

        ; r11 now stores sgn(A1)
        mov     r11, [rdi + 8*r10 - 8]
        sar     r11, 63

        ; After loop r10 = n, rcx = 0 so we restore initial values.
        mov     rcx, r10                        ; rcx = n
        xor     r10, r10                        ; i = 0

; Step 3: Ak = Ak + A0
.add_loop:
        mov     rax, [rdi + 8*r10]              ; rax = Ak[i]
        adc     [r9 + 8*r10], rax               ; Ak[i] += A0[i]

        inc     r10
        loop    .add_loop

        movq    rax, xmm0                       ; rax = Ak[n]
        movq    rdx, xmm1                       ; rdx = sgn(Ak)

        ; Correct the eldest 128 bits of Ak with the sign of A0
        adc     rax, r11
        adc     rdx, r11

        ret