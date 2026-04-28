;
; Amstrad CPC IO Logger Test Program
;
; Version 	Date 		Changes
; 1.0		17 APR 26	First Version
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


;
; TEST A2
;
; T01
;
; Write CRTC Register 1 500 times at around 56usecs between writes then Register 2 at 22usecs.
;
T01
; Loop 500 times with a slow seed, deplay loop gives 55.75usecs			
        ld d,#00
		ld bc,500
T01_L01
		push bc			; Save loop counter
		ld e,#01		; Set Register 1

		ld bc,CRTCIA
		out (c),e

		ld bc,CRTCDA	; Write incrementing value
		out (c),d

        inc d			; inc the value being written

; Delay loop		
		ld b,#8
T01_L02
		NOP
		djnz T01_L02

; Test for end of main loop
		pop bc
		dec bc
		jp nz,T01_L01



T02
; Loop 500 times with at a fast seed, deplay loop gives 55.75usecs	
;
        ld d,#00
		ld bc,500

T02_L01
		push bc
		ld e,#02

		ld bc,CRTCIA
		out (c),e

		ld bc,CRTCDA
		out (c),d

		inc d			; inc the value being written

		pop bc
		dec bc
		jp nz,T02_L01




		; repeat
        jp T01_L01
