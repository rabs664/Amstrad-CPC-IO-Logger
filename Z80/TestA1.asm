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

IOLOGA		equ	#F9E0

Z80

IFDEF Z80	
	T01_RDATA	equ	#9000
ENDIF		

PRTREGBUF	equ	50

IFDEF Z80	
	org #0000

	ld sp,#FFFF
ENDIF

IFDEF CPC
	org #8000
ENDIF


;
; TEST A1
;
; Write CRTC Register vakues, read back and compare. out E1 if pass, F1 if fail.
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
	
	IFDEF Z80
    	halt
	ENDIF

	IFDEF CPC
		ret
	ENDIF

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



T01_WDATA DEFB	#3F, #28, #2E, #8E, #26, #00, #19, #1E, #00, #07, #00, #00, #31, #B8, #00, #00, #0F

IFDEF CPC
	T01_RDATA DEFB #3F, #28, #2E, #8E, #26, #00, #19, #1E, #00, #07, #00, #00, #31, #B8, #00, #00, #0F
ENDIF

IFDEF CPC
	save 'test1A.bin',#8000,2300,DSK,'iolog.dsk'
ENDIF
