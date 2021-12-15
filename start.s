
;@-------------------------------------------------------------------
;@-------------------------------------------------------------------

.globl _start
_start:
    b reset
    b hang
    b hang
    b hang
    b hang
    b hang
    b hang
    b hang

reset:

    ;@ enable fpu
    mrc p15, 0, r0, c1, c0, 2
    orr r0,r0,#0x300000 ;@ single precision
    orr r0,r0,#0xC00000 ;@ double precision
    mcr p15, 0, r0, c1, c0, 2
    mov r0,#0x40000000
    fmxr fpexc,r0

  // ---------------------------------------------------------------------- Init cache
  mov r12,#0
  mcr p15, 0, r12, c7, c10, 1   // DCCMVAC - Clean data or unified cache line by virtual address to PoC.
  mov r12, #0
  mcr p15, 0, r12, c7, c5, 0    // ICIALLU - Invalidate all instruction caches to PoU. If branch predictors are architecturally visible, also flush them.
  mov r12, #0
  mcr p15, 0, r12, c7, c5, 6    // BPIALL  - Invalidate all entries from branch predictors.


  // ---------------------------------------------------------------------- Set secure state
  mrc p15, 0, r1, c1, c1, 0   // Get SCR  (Secure Configuration Register)
  bic r1, r1, #1              // Reset 'Non-secure' bit (set secure state)
  mcr p15, 0, r1, c1, c1, 0   // Write to SCR

  // ---------------------------------------------------------------------- Turn on Instruction cache
  mrc p15,0,r2,c1,c0,0
  orr r2, #0x1000    // Instruction cache
  //orr r2, #0x0800    // Branch prediction (does not affect this result)
  //orr r2, #0x0004    // Data cache        (does not affect this result)
  //orr r2, #0x0001  // MMU               (does not work! don't know why yet)
  mcr p15,0,r2,c1,c0,0

    mov sp,#0x8000
    bl main
hang: b hang


;@-------------------------------------------------------------------
;@-------------------------------------------------------------------

;@-------------------------------------------------------------------
;@
;@ Copyright (c) 2012 David Welch dwelch@dwelch.com
;@
;@ Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
;@
;@ The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
;@
;@ THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
;@
;@-------------------------------------------------------------------
