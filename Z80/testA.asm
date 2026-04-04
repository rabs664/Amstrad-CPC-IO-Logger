;
; Amstrad CPC IO Logger Test Program
;
; Version 	Date 		Changes
; 1.0		03 APR 26	First Version
;
;
;

CRTCIA:		.EQU	0BC00h		
CRTCDA:		.EQU 	0BD00h		

IOLOGA:		.EQU	0F9E0h		


T01_RDATA:	.EQU	08000h


PRTREGBUF:	.EQU	50

.ORG 	00000h

	LD SP,0FFFFh

; TEST 01
;
; Write CRTC Register vakues, read back and compare. OUT E1 if pass, F1 if fail.
;


T01:

;
; Write to the CRTC Registers
;
	LD B,16			; 16 CRTC Registers

	LD A,0

	LD HL,T01_WDATA

T01_L01:
	PUSH BC			; Save loop counter
         
	LD BC,CRTCIA
	OUT (C),A

	LD D, (HL)		; Get the byte to write
	LD BC,CRTCDA
	OUT (C),D

	INC A			; Next Register
	INC HL			; Next Data

	POP BC			; Restore the loop counter
	DJNZ T01_L01


;
; Read back the registers
;
	LD BC,IOLOGA
	LD A,0
	OUT (C),A		; Reset the IO Logger read index


	LD B,17			; 16 CRTC Register + Last Register Written 
	LD HL,T01_RDATA
	
T01_L02:
	PUSH BC

	LD BC,IOLOGA
	IN A,(C)
	LD (HL),A
	INC HL

	POP BC

	DJNZ T01_L02
			
;
; Compare the Registers read with that written
;

    	LD HL,T01_RDATA		
    	LD DE,T01_WDATA
    	LD BC,17			

T01_L03:
	PUSH BC
	LD A,(HL)
	LD BC,IOLOGA
	OUT (C),A
	POP BC

    	LD A,(DE)        		 	
    	CP (HL)               	
    	JR NZ, T01_FAIL
   		
    	INC DE            		
	INC HL
	 	
    	DJNZ T01_L03		

T01_PASS:
	LD BC,IOLOGA
	LD A,PRTREGBUF
	OUT (C),A
	LD A,0E1h			; Test Passed
	OUT (C),A
    	HALT

T01_FAIL:
	LD BC,IOLOGA
	LD A,PRTREGBUF
	OUT (C),A
	LD A,0F1h			; Test Failed
	OUT (C),A
    	HALT



T01_WDATA:	
.DB	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 15


.END