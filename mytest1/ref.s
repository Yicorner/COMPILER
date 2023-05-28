	.file	"ref.c"
	.option nopic
	.text
	.align	1
	.globl	get_one
	.type	get_one, @function
get_one:
	addi	sp,sp,-32
	sw	s0,28(sp)
	addi	s0,sp,32
	sw	a0,-20(s0)
	li	a5,1
	mv	a0,a5
	lw	s0,28(sp)
	addi	sp,sp,32
	jr	ra
	.size	get_one, .-get_one
	.align	1
	.globl	deepWhileBr
	.type	deepWhileBr, @function
deepWhileBr:
	addi	sp,sp,-48
	sw	ra,44(sp)
	sw	s0,40(sp)
	addi	s0,sp,48
	sw	a0,-36(s0)
	sw	a1,-40(s0)
	lw	a4,-36(s0)
	lw	a5,-40(s0)
	add	a5,a4,a5
	sw	a5,-20(s0)
	j	.L4
.L5:
	li	a5,42
	sw	a5,-24(s0)
	lw	a4,-20(s0)
	li	a5,99
	bgt	a4,a5,.L4
	lw	a4,-20(s0)
	lw	a5,-24(s0)
	add	a5,a4,a5
	sw	a5,-20(s0)
	lw	a4,-20(s0)
	li	a5,99
	ble	a4,a5,.L4
	lw	a5,-24(s0)
	slli	a5,a5,1
	sw	a5,-28(s0)
	li	a0,0
	call	get_one
	mv	a4,a0
	li	a5,1
	bne	a4,a5,.L4
	lw	a5,-28(s0)
	slli	a5,a5,1
	sw	a5,-20(s0)
.L4:
	lw	a4,-20(s0)
	li	a5,74
	ble	a4,a5,.L5
	lw	a5,-20(s0)
	mv	a0,a5
	lw	ra,44(sp)
	lw	s0,40(sp)
	addi	sp,sp,48
	jr	ra
	.size	deepWhileBr, .-deepWhileBr
	.align	1
	.globl	main
	.type	main, @function
main:
	addi	sp,sp,-32
	sw	ra,28(sp)
	sw	s0,24(sp)
	addi	s0,sp,32
	li	a5,2
	sw	a5,-20(s0)
	lw	a1,-20(s0)
	lw	a0,-20(s0)
	call	deepWhileBr
	sw	a0,-20(s0)
	lw	a0,-20(s0)
	call	putint
	li	a5,0
	mv	a0,a5
	lw	ra,28(sp)
	lw	s0,24(sp)
	addi	sp,sp,32
	jr	ra
	.size	main, .-main
	.ident	"GCC: (GNU) 9.2.0"
	.section	.note.GNU-stack,"",@progbits
