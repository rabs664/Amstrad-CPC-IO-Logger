;
; Amstrad CPC IO Logger Test Program
;
; Version 	Date 		Changes
; 1.0		03 APR 26	First Version
;
;
;
CRTCIA		equ	#BC00		
CRTCDA		equ	#BD00		
GATEAA		equ	#7F00

IOLOGA		equ	#F9E0


Z80 equ 1

IFDEF Z80	
	org #0000

	ld sp,#FFFF
ENDIF

T01
;
; TEST A1
;
; T01
;
; Write CRTC Register 1 with an incementing value in tight loop.
;

; Loop a a slow seed, with a deplay loop 55.75usecs			
        ld d,#00
		ld e,#01
		ld bc,500
T01_L01
		push bc

		ld bc,CRTCIA
		out (c),e

		ld bc,CRTCDA
		out (c),d

        inc d
		ld b,#8
T01_L02
		NOP				; Delay
		djnz T01_L02

		pop bc
		dec bc
		jp nz,T01_L01

; Loop at a fast seed, i.e. no delay 20.25 usec
			
        ld d,#00
		ld e,#02
		ld bc,500
T01_L03
		push bc

		ld bc,CRTCIA
		out (c),e

		ld bc,CRTCDA
		out (c),d

		inc d
;		ld b,#2
;T01_L04
;		NOP				; Delay
;		djnz T01_L04

		pop bc
		dec bc
		jp nz,T01_L03

		; repeat
        jp T01_L01
