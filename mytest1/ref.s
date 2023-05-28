	.file	"ref.c"
	.option nopic
	.text
	.globl	a
	.section	.sdata,"aw"
	.align	2
	.type	a, @object
	.size	a, 4
a:
	.word	7
	.text
	.align	1
	.globl	func
	.type	func, @function
func:
	addi	sp,sp,-32
	sw	s0,28(sp)
	addi	s0,sp,32
	lui	a5,%hi(a)
	lw	a5,%lo(a)(a5)
	sw	a5,-20(s0)
	li	a5,1
	sw	a5,-24(s0)
	lw	a4,-24(s0)
	lw	a5,-20(s0)
	bne	a4,a5,.L2
	lw	a5,-24(s0)
	addi	a5,a5,1
	sw	a5,-24(s0)
	li	a5,1
	j	.L3
.L2:
	li	a5,0
.L3:
	mv	a0,a5
	lw	s0,28(sp)
	addi	sp,sp,32
	jr	ra
	.size	func, .-func
	.align	1
	.globl	main
	.type	main, @function
main:
	addi	sp,sp,-32
	sw	ra,28(sp)
	sw	s0,24(sp)
	addi	s0,sp,32
	sw	zero,-20(s0)
	sw	zero,-24(s0)
	j	.L5
.L7:
	call	func
	mv	a4,a0
	li	a5,1
	bne	a4,a5,.L6
	lw	a5,-20(s0)
	addi	a5,a5,1
	sw	a5,-20(s0)
.L6:
	lw	a5,-24(s0)
	addi	a5,a5,1
	sw	a5,-24(s0)
.L5:
	lw	a4,-24(s0)
	li	a5,99
	ble	a4,a5,.L7
	lw	a4,-20(s0)
	li	a5,99
	bgt	a4,a5,.L8
	li	a0,1
	call	putint
	j	.L9
.L8:
	li	a0,0
	call	putint
.L9:
	li	a5,0
	mv	a0,a5
	lw	ra,28(sp)
	lw	s0,24(sp)
	addi	sp,sp,32
	jr	ra
	.size	main, .-main
	.ident	"GCC: (GNU) 9.2.0"
	.section	.note.GNU-stack,"",@progbits
