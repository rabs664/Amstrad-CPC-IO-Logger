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

Z80

; Somewhere in upper 32K of memory
IFDEF Z80	
	T01_RDATA	equ	#9000
	T03_RDATA	equ	#9100
ENDIF		

PRTREGBUF	equ	50

IFDEF Z80	
	org #0000

	ld sp,#FFFF
ENDIF

IFDEF CPC
	org #8000
ENDIF

T01
;
; TEST A1
;
; T01
;
; Write CRTC Register values, read back and compare. OUT E1 if pass, F1 if fail.
;
T01_WRITE

;
; Write to the CRTC Registers
;
	ld b,16				; 16 CRTC Registers

	ld a,0

	ld hl,T01_WDATA

T01_L01
	push bc				; Save loop counter
         
		ld bc,CRTCIA
		out (C),a

		ld D, (hl)		; Get the byte to write
		ld bc,CRTCDA
		out (C),D

		INC a			; Next Register
		INC hl			; Next Data

	pop bc				; Restore the loop counter
	djnz T01_L01

T01_READ
;
; Read back the registers
;
	ld bc,IOLOGA
	ld a,0
	out (C),a			; Reset the IO Logger read index

	ld b,17				; 16 CRTC Register + Last Register Written 
	ld hl,T01_RDATA
	
T01_L02
	push bc

		ld bc,IOLOGA
		in a,(C)
		ld (hl),a
		INC hl

	pop bc
	djnz T01_L02
			
T01_COMPARE
;
; Compare the Registers read with that written
;

    	ld hl,T01_RDATA		
    	ld de,T01_WDATA
    	ld b,17			

T01_L03
    	ld a,(de)        		 	
    	cp (hl)               	
    	jr nz, T01_FAIL
   		
    	inc de            		
		inc hl
	 	
    djnz T01_L03		

T01_PASS
	ld bc,IOLOGA
	ld a,PRTREGBUF
	out (C),a
	ld a,#E1			; Test Passed
	out (C),a
	
    jp T02				; Goto next test

T01_FAIL
	ld bc,IOLOGA
	ld a,PRTREGBUF
	out (C),a
	ld a,#F1			; Test Failed
	out (C),a
	
	IFDEF Z80
    	halt
	ENDIF

	IFDEF CPC
		ret
	ENDIF


T02
; 
;
; TEST A1
;
; T02
;
; Write to CRTC Register 16 an out of range regsiter, only R0-R15. Check for ERR value 1, OUT E2 if pass, F2 if fail.
;

T02_WRITE
	ld a,16				; Out of range CRTC Register, only 0..15 valid
	ld bc,CRTCIA
	out (C),a

T02_READ
	ld bc,IOLOGA
	ld a,43
	out (C),a			; Reset the IO Logger read index to the error 43
	in a,(C)

T02_COMPARE
	cp #1				; Expect an error value of 1
	jr nz, T02_FAIL

T02_PASS
	ld bc,IOLOGA
	ld a,PRTREGBUF
	out (C),a
	ld a,#E2			; Test Passed
	out (C),a
	
   	jp T03				; Goto next test

T02_FAIL
	ld bc,IOLOGA
	ld a,PRTREGBUF
	out (C),a
	ld a,#F2			; Test Failed
	out (C),a
	
	IFDEF Z80
    	halt
	ENDIF

	IFDEF CPC
		ret
	ENDIF


T03
; 
;
; TEST A1
;
; T03
;
; Write to GA PEN Colours. read back and compare. OUT E3 if pass, F3 if fail..
;
T03_WRITE
	ld b,17				; 17 Pen Colours, including the border colour

	ld hl,T03_PENWDATA	; Pen data to write
	ld de,T03_COLWDATA  ; Colour data to write

T03_L01
	push bc				; Save loop counter
         
		ld a,(hl) 		; Select the pen
		ld bc,GATEAA
		out (c),a

		ld a,(de)		; Get the colour to write
		ld bc,GATEAA
		out (c),a
		
		INC de			; Next Colour	
		INC hl			; Next pen

	pop bc				; Restore the loop counter
	djnz T03_L01

T03_READ
	ld bc,IOLOGA
	ld a,17
	out (C),a			; Reset the IO Logger read index to point to the pen colour data

	ld b,18				; 16 pens + border + last pen written
	ld hl,T03_RDATA
	
T03_L02
	push bc

		ld bc,IOLOGA
		in a,(C)
		ld (hl),a
		INC hl

	pop bc
	djnz T03_L02

T03_COMPARE
    	ld hl,T03_RDATA		
    	ld de,T03_COLCDATA
    	ld b,18				; 16 pens + border + last pen written			

T03_L03
    	ld a,(de)        		 	
    	cp (hl)               	
    	jr nz, T03_FAIL
   		
    	inc de            		
		inc hl
	 	
    djnz T03_L03		

T03_PASS
	ld bc,IOLOGA
	ld a,PRTREGBUF
	out (C),a
	ld a,#E3
	out (C),a
	
	jp T04				; Goto next test	


T03_FAIL
	ld bc,IOLOGA
	ld a,PRTREGBUF
	out (C),a
	ld a,#F3
	out (C),a
	
	IFDEF Z80
    	halt
	ENDIF

	IFDEF CPC
		ret
	ENDIF

T04
; 
;
; TEST A1
;
; T04
;
; Write to GA Pen out of range regsiter, only Pens 0-15 + 16 for the border are supported. Check for ERR value 2, OUT E2 if pass, F2 if fail.
;

T04_WRITE
	ld a,17		; Out of range Pen 17, only 0..15 + 16 for the border are valid
	ld bc,GATEAA
	out (C),a

T04_READ
	ld bc,IOLOGA
	ld a,43
	out (C),a			; Reset the IO Logger read index to the error 43
	in a,(C)

T04_COMPARE
	cp #2				; Expect an error value of 2
	jr nz, T04_FAIL

T04_PASS
	ld bc,IOLOGA
	ld a,PRTREGBUF
	out (C),a
	ld a,#E4			; Test Passed
	out (C),a
	
	IFDEF Z80
    	halt
	ENDIF

	IFDEF CPC
		ret
	ENDIF

T04_FAIL
	ld bc,IOLOGA
	ld a,PRTREGBUF
	out (C),a
	ld a,#F4			; Test Failed
	out (C),a
	
	IFDEF Z80
    	halt
	ENDIF

	IFDEF CPC
		ret
	ENDIF







	; 16 CRTC Registers + Last Pen (LP) Written
	;             0    1	2    3    4    5    6    7    8    9   10   11   12   13   14   15   LP
T01_WDATA DEFB	#3F, #28, #2E, #8E, #26, #00, #19, #1E, #00, #07, #00, #00, #31, #B8, #00, #00, #0F

IFDEF CPC
	; 16 CRTC Registers + Last Register Written
	;               1    2	  3    4    5    6    7    8    9    10   11   12   13   14   15   16   17
	T01_RDATA DEFS #00, 17
	
	; 16 CRTC Registers + Border + Last Register Written
	T03_RDATA DEFS #00, 18
ENDIF	

/*
Bit	Value	Function
7	0	Gate Array function "Pen Selection"
6	0
5	x	not used
4	1	Select border
3	x	ignored
2	x	ignored
1	x	ignored
0	x	ignored
*/
;Pen 0 to 15 and Border (16)
T03_PENWDATA DEFB 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16

/*
Bit	Value	Function
7	0	Gate Array function "Colour Selection"
6	1
5	x	not used
4	x	Colour number
3	x
2	x
1	x
0	x
*/
T03_COLWDATA DEFB #40+#04, #40+#0A, #40+#13, #40+#0C, #40+#0B, #40+#14, #40+#15, #40+#0D, #40+#06, #40+#1E, #40+#1F, #40+#07, #40+#12, #40+#19, #40+#04, #40+#17, #40+#04

; Just the coloour number to compare with the read back value + Last Pen Written
T03_COLCDATA DEFB #04, #0A, #13, #0C, #0B, #14, #15, #0D, #06, #1E, #1F, #07, #12, #19, #04, #17, #04, #10

IFDEF CPC
	save 'test1A.bin',#8000,2300,DSK,'iolog.dsk'
ENDIF
