    .title "EE308 Experiment Board"
    .sbttl "Event Counter 0..9"
    .equ __24FJ256GB110, 1
    .include "p24FJ256GB110.inc"

    .global __reset          ;The label for the first line of code. 
    .global __T1Interrupt    ;Declare Timer 1 ISR name global

.bss
    LCD_line1:	.space 16
    LCD_line2:	.space 16
    LCD_ptr:	.space 2
    LCD_cmd:	.space 2
    LCD_offset: .space 2
	cnt:		.space 2
.section .const,psv
    line1:	.ascii "Event Counter   "
    line2:	.ascii "Count ---->    00"
    lookup:	.ascii "0123456789ABCDEF"
.text				;Start of Code section
__reset:
    mov	    #__SP_init, W15	; Initalize the Stack Pointer
    mov	    #__SPLIM_init, W0	; Initialize the Stack Pointer Limit Register
    mov	    W0, SPLIM
    nop				; Add NOP to follow SPLIM initialization

    call    init_PSV
    call    init_LED
    call    init_LCD
    call    init_message
    call    init_keypad
    call    init_buzzer

    call    init_timer
		
	
	
	
cnt_loop:
	mov #0x0099, W10	; contains 99
	mov #0x001F, W11	;	To and get 5 last bits
check_keypad:
    btss    PORTD, #6		    ; keypad entry?
    bra	    check_keypad
	mov		#0x0014, W0
	mov		#0x000F, W1
	mov		#0x00F0, W11
	and		W0, W1, W1
	and 	W0, W11, W11
	mov		#'0', W2
	daw.b 	W1
	add		W0, W2, W0
	add 	W11, W2, W11
	mov		#LCD_line2, W3
	mov.b		W11,[W3+14]
	mov.b 		W1, [W3+15]
	bra delay_start
countdown:
	add 	W0, W10, W0
	and 	W0, W11, W0
	bclr 	W0, #2
	mov		#0x000F, W1
	mov		#0x00F0, W11
	and		W0, W1, W1
	and 	W0, W11, W11
	mov		#'0', W2
	daw.b 	W1
	add		W0, W2, W0
	add 	W11, W2, W11
	mov		#LCD_line2, W3
	mov.b		W11, [W3+14]
	mov.b 		W1, [W3+15]
	cp W0, #0
    bra NZ delay_start		    ; wait 1 second
	bra Z check_keypad			; countdown done go to start.
	
	
delay_start:    
    mov	#0x0000, W4
delayContinued:
    mov #0x0000, W5
    mov #0xFFFF, W6
delay_Loop:
    inc W5, W5	    ;increment W5
    ;bset PORTD, #13 was for debugging purposes
    cp   W5,W6		;Compare W5, with W6 
    bra NZ, delay_Loop	;If the zero flag is not set continue incrementing...
    inc W4, W4		;But if the zero flag is set, increment W4
    cp W4, #5		;Check if the W4 is equal to 5, basically check if we have ran the delay Loop 5 times in total or not
    bra Z, countdown	;If the delay Loop has been run for 5 times, then go back to the start of the program.
    bra NZ, delayContinued  ;If not continue the delay operation.
    
 
    
    
done:
    bra	    done		; Place holder for last line of executed code

; -----------------------------------------------------
; !!!!!!!!!!!!!!!!!! Functions !!!!!!!!!!!!!!!!!!!!!!!!
; -----------------------------------------------------

init_PSV:
    mov	    #psvpage(line1), W0
    mov	    W0, PSVPAG		; set PSVPAG to page that contains hello
    bset.b  CORCONL,#PSV	; enable Program Space Visibility
    return

init_timer:
    bclr    T1CON, #TON		; turn timer1 OFF
	
    bset    T1CON, #TCKPS1
    bset    T1CON, #TCKPS0	; set prescaler to 256

    bclr    T1CON, #TCS		; select internal clock

    mov	    #0x0000, W0 
    mov	    W0, TMR1		; clear TMR1 register
    mov	    #0x0040, W0
    mov	    W0, PR1		; set timer1 period to 0x0040 -> f=2e6/256/64=122 Hz

    bclr    IFS0, #T1IF		; clear timer1 interrupt status flag
    bset    IEC0, #T1IE		; enable timer1 interrupts

    bset    T1CON, #TON		; turn timer1 ON
    return

init_LED:
    bclr    TRISF, #0
    bclr    TRISF, #1
    bclr    TRISF, #2
    bclr    TRISF, #3		; LED array
    return

init_LCD:
    bclr    TRISB, #15
    bclr    PORTD, #4		; make sure LCD is disabled before port is set to output mode
    bclr    TRISD, #4
    bclr    TRISD, #5
    mov	    #0xFF00, W0
    mov	    W0, TRISE

    bclr    PORTD, #5		; select LCD WR mode

    mov	    #0x0038, W0		; init LCD
    call    sendcomm
    call    dly
    call    dly
    call    dly

    mov	    #0x000E, W0		; LCD on, cursor on
    call    sendcomm
    mov	    #0x0001,W0		; clear LCD
    call    sendcomm
    return

sendcomm:
    bclr    PORTB,#15	; select LCD command register
    mov	    W0, PORTE	; output command
    bset    PORTD, #4
    call    dly
    nop
    bclr    PORTD, #4
    call    dly
    return

dly:
    mov	    #0x2000,W0
dlyloop:
    sub	    W0, #1, W0
    bra	    NZ, dlyloop
    return

init_message:
    mov	    #0x0000, W0
    mov	    W0, LCD_ptr
    mov	    W0, LCD_offset
    mov	    #0x00C0, W0
    mov	    W0, LCD_cmd
    mov	    #psvoffset(line1), W1
    mov	    #LCD_line1, W2
    repeat  #15
    mov	   [W1++], [W2++]
    return

init_keypad:
    bset    TRISD,#0	; DATA A
    bset    TRISD,#1	; DATA B
    bset    TRISD,#2	; DATA C
    bset    TRISD,#3	; DATA D
    bset    TRISD,#6	; DATA Available
    return

init_buzzer:
    bclr    PORTD, #13	; buzzer initially OFF
    bclr    TRISD, #13	; enable output
    return
	

;..............................................................................
; Timer 1 Interrupt Service Routine
; This ISR controls the LCD display
;..............................................................................
__T1Interrupt:
    push.d  W0
    push.d  W2		; Save context using double-word PUSH

    bclr    IFS0, #T1IF	; Clear the Timer1 Interrupt flag status bit

    mov	    LCD_ptr, W2
    mov	    #0x0010, W1
    cp	    W1, W2
    bra	    NZ, send_LCD_data
    mov	    LCD_cmd, W0
    bclr    PORTB, #15		; select LCD command register
    mov	    W0, PORTE		; output command
    bset    PORTD, #4
    nop
    bclr    PORTD, #4
    btg	    W0, #6
    mov	    W0, LCD_cmd
    mov	    #0x0000, W2
    mov	    W2, LCD_ptr
    mov	    LCD_offset, W0
    btg	    W0, #4
    mov	    W0, LCD_offset
    btg	    PORTF, #2
    bra	    done_T1interrupt
send_LCD_data:
    mov	    LCD_offset, W3
    add	    W3, W2, W3
    mov	    #LCD_line1, W1
    mov.b   [W1+W3], W0
    bset    PORTB, #15		; select LCD data register
    mov	    W0, PORTE		; output command
    bset    PORTD, #4
    nop
    bclr    PORTD, #4
    inc	    W2, W2
    mov	    W2, LCD_ptr

done_T1interrupt:
    pop.d   W2		    ; Retrieve context POP-ping from Stack
    pop.d   W0
    retfie		    ; Return from Interrupt Service routine

.end			    ; End of program code in this file

