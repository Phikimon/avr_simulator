	.file	"blink.c"
__SP_L__ = 0x3d
__SREG__ = 0x3f
__tmp_reg__ = 0
__zero_reg__ = 1
	.section	.text.startup,"ax",@progbits
.global	main
	.type	main, @function
main:
/* prologue: function */
/* frame size = 0 */
/* stack size = 0 */
.L__stack_usage = 0
	ldi r24,lo8(-1)
	out 0x17,r24
.L2:
	out 0x18,r24
	out 0x18,__zero_reg__
	rjmp .L2
	.size	main, .-main
	.ident	"GCC: (GNU) 4.9.2"
