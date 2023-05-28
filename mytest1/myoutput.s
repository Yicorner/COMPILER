	.file	"ref.c"
	.option nopic
	.text
	.globl	a_0
	.data
	.align 2
	.type	a_0, @object
	.size	a_0, 4
a_0:
	.word	7
	.text
	.align	1
	.globl	func
	.type	func, @function
func:
    sw	s0,-4(sp)
    sw	ra,-8(sp)
    addi	sp,sp,-32
    addi	s0,sp,32
    lui    a4,%hi(a_0)
    lw	a4,%lo(a_0)(a4)
    mv    a5,a4
    sw	a5,-24(s0)
    li	a5,1
    sw	a5,-28(s0)
    lw	t5,-24(s0)
    lw	t4,-28(s0)
    sub    s1,t5,t4
    seqz    s1,s1
    mv    s10,s1
    bne    s10,zero,.L1
    j    .L2
.L1:
    lw	a5,-28(s0)
    addi	s1,a5,1
    mv    a5,s1
    sw	a5,-28(s0)
    li	a0,1
    lw	ra,-8(s0)
    lw	s0,-4(s0)
    addi	sp,sp,32
    jr    ra
    j    .L3
.L2:
    li	a0,0
    lw	ra,-8(s0)
    lw	s0,-4(s0)
    addi	sp,sp,32
    jr    ra
.L3:
    li	a0,0
    lw	ra,-8(s0)
    lw	s0,-4(s0)
    addi	sp,sp,32
    jr    ra
	.size	func, .-func
	.globl	main
	.type	main, @function
main:
    sw	s0,-4(sp)
    sw	ra,-8(sp)
    addi	sp,sp,-76
    addi	s0,sp,76
    li	a5,0
    sw	a5,-68(s0)
    li	a5,0
    sw	a5,-72(s0)
.L8:
    lw	t5,-72(s0)
    li	t4,100
    slt    s1,t5,t4
    mv    s10,s1
    mv    s1,s10
    bne    s1,zero,.L4
    j    .L5
.L4:
    sw	s1,-64(s0)
    sw	s10,-60(s0)
    call    func
    lw	s1,-64(s0)
    lw	s10,-60(s0)
    mv    s1,a0
    li	t5,1
    mv    t4,s1
    sub    s10,t5,t4
    seqz    s10,s10
    mv    s1,s10
    bne    s1,zero,.L6
    j    .L7
.L6:
    lw	a5,-68(s0)
    addi	s1,a5,1
    mv    a5,s1
    sw	a5,-68(s0)
.L7:
    lw	a5,-72(s0)
    addi	s1,a5,1
    mv    a5,s1
    sw	a5,-72(s0)
    j    .L8
.L5:
    lw	t5,-68(s0)
    li	t4,100
    slt    s1,t5,t4
    mv    s10,s1
    mv    s1,s10
    bne    s1,zero,.L9
    j    .L10
.L9:
    li	a0,1
    call    putint
    mv    s1,a0
    j    .L11
.L10:
    li	a0,0
    call    putint
    mv    s1,a0
.L11:
    li	a0,0
    lw	ra,-8(s0)
    lw	s0,-4(s0)
    addi	sp,sp,76
    jr    ra
    li	a0,0
    lw	ra,-8(s0)
    lw	s0,-4(s0)
    addi	sp,sp,76
    jr    ra
	.size	main, .-main
	.ident	"GCC: (GNU) 9.2.0"
	.section	.note.GNU-stack,"",@progbits
