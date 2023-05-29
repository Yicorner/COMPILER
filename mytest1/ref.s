	.file	"ref.c"
	.option nopic
	.text
	.comm	n,4,4
	.align	1
	.globl	bubblesort
	.type	bubblesort, @function
bubblesort:
	addi	sp,sp,-48
	sw	s0,44(sp)
	addi	s0,sp,48
	sw	a0,-36(s0)
	sw	zero,-20(s0)
	j	.L2
.L6:
	sw	zero,-24(s0)
	j	.L3
.L5:
	lw	a5,-24(s0)
	slli	a5,a5,2
	lw	a4,-36(s0)
	add	a5,a4,a5
	lw	a4,0(a5)
	lw	a5,-24(s0)
	addi	a5,a5,1
	slli	a5,a5,2
	lw	a3,-36(s0)
	add	a5,a3,a5
	lw	a5,0(a5)
	ble	a4,a5,.L4
	lw	a5,-24(s0)
	addi	a5,a5,1
	slli	a5,a5,2
	lw	a4,-36(s0)
	add	a5,a4,a5
	lw	a5,0(a5)
	sw	a5,-28(s0)
	lw	a5,-24(s0)
	slli	a5,a5,2
	lw	a4,-36(s0)
	add	a4,a4,a5
	lw	a5,-24(s0)
	addi	a5,a5,1
	slli	a5,a5,2
	lw	a3,-36(s0)
	add	a5,a3,a5
	lw	a4,0(a4)
	sw	a4,0(a5)
	lw	a5,-24(s0)
	slli	a5,a5,2
	lw	a4,-36(s0)
	add	a5,a4,a5
	lw	a4,-28(s0)
	sw	a4,0(a5)
.L4:
	lw	a5,-24(s0)
	addi	a5,a5,1
	sw	a5,-24(s0)
.L3:
	lui	a5,%hi(n)
	lw	a4,%lo(n)(a5)
	lw	a5,-20(s0)
	sub	a5,a4,a5
	addi	a5,a5,-1
	lw	a4,-24(s0)
	blt	a4,a5,.L5
	lw	a5,-20(s0)
	addi	a5,a5,1
	sw	a5,-20(s0)
.L2:
	lui	a5,%hi(n)
	lw	a5,%lo(n)(a5)
	addi	a5,a5,-1
	lw	a4,-20(s0)
	blt	a4,a5,.L6
	li	a5,0
	mv	a0,a5
	lw	s0,44(sp)
	addi	sp,sp,48
	jr	ra
	.size	bubblesort, .-bubblesort
	.align	1
	.globl	main
	.type	main, @function
main:
	addi	sp,sp,-64
	sw	ra,60(sp)
	sw	s0,56(sp)
	addi	s0,sp,64
	lui	a5,%hi(n)
	li	a4,10
	sw	a4,%lo(n)(a5)
	li	a5,4
	sw	a5,-64(s0)
	li	a5,3
	sw	a5,-60(s0)
	li	a5,9
	sw	a5,-56(s0)
	li	a5,2
	sw	a5,-52(s0)
	sw	zero,-48(s0)
	li	a5,1
	sw	a5,-44(s0)
	li	a5,6
	sw	a5,-40(s0)
	li	a5,5
	sw	a5,-36(s0)
	li	a5,7
	sw	a5,-32(s0)
	li	a5,8
	sw	a5,-28(s0)
	addi	a5,s0,-64
	mv	a0,a5
	call	bubblesort
	sw	a0,-20(s0)
	j	.L9
.L10:
	lw	a5,-20(s0)
	slli	a5,a5,2
	addi	a4,s0,-16
	add	a5,a4,a5
	lw	a5,-48(a5)
	sw	a5,-24(s0)
	lw	a0,-24(s0)
	call	putint
	li	a5,10
	sw	a5,-24(s0)
	lw	a0,-24(s0)
	call	putch
	lw	a5,-20(s0)
	addi	a5,a5,1
	sw	a5,-20(s0)
.L9:
	lui	a5,%hi(n)
	lw	a5,%lo(n)(a5)
	lw	a4,-20(s0)
	blt	a4,a5,.L10
	li	a5,0
	mv	a0,a5
	lw	ra,60(sp)
	lw	s0,56(sp)
	addi	sp,sp,64
	jr	ra
	.size	main, .-main
	.ident	"GCC: (GNU) 9.2.0"
	.section	.note.GNU-stack,"",@progbits
