   .file	"ref.c"
   .option nopic
   .text
   .text
   .align	1
   .globl	func
   .type	func, @function
func:
    sw	s0,-4(sp)
    sw	ra,-8(sp)
    addi	sp,sp,-20
    addi	s0,sp,20
    lw	a5,-16(s0)
    addi	s1,a5,-1
    mv    a5,s1
    sw	a5,-16(s0)
    lw	a5,-16(s0)
    mv    a0,a5
    lw	ra,-8(s0)
    lw	s0,-4(s0)
    addi	sp,sp,20
    jr    ra
    li	a0,0
    lw	ra,-8(s0)
    lw	s0,-4(s0)
    addi	sp,sp,20
    jr    ra
	.size	func, .-func
	.ident	"GCC: (GNU) 9.2.0"
	.section	.note.GNU-stack,"",@progbits
   .globl	main
   .type	main, @function
main:
    sw	s0,-4(sp)
    sw	ra,-8(sp)
    addi	sp,sp,-24
    addi	s0,sp,24
    li	a5,0
    lui    a5,%hi(b_1)
    sw	,%lo(b_1)(a5)
    li	a5,10
    lui    a5,%hi(a_0)
    sw	,%lo(a_0)(a5)
    mv    a5,s1
    lui    a5,%hi(b_1)
    sw	,%lo(b_1)(a5)
    lui    a5,%hi(b_1)
    lw	a5,%lo(b_1)(a5)
    mv    a0,a5
    lw	ra,-8(s0)
    lw	s0,-4(s0)
    addi	sp,sp,24
    jr    ra
    li	a0,0
    lw	ra,-8(s0)
    lw	s0,-4(s0)
    addi	sp,sp,24
    jr    ra
	.size	main, .-main
	.ident	"GCC: (GNU) 9.2.0"
	.section	.note.GNU-stack,"",@progbits
