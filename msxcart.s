	.arch armv6
	.eabi_attribute 27, 3
	.eabi_attribute 28, 1
	.fpu vfp
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 2
	.eabi_attribute 30, 6
	.eabi_attribute 34, 1
	.eabi_attribute 18, 4
	.file	"msxcart.c"
	.comm	mem_fd,4,4
	.comm	gpio_map,4,4
	.comm	gpio,4,4
	.section	.rodata
	.align	2
.LC0:
	.ascii	"rb\000"
	.align	2
.LC1:
	.ascii	"MSX Mega Box activated.\000"
	.align	2
.LC2:
	.ascii	"%08x\012\000"
	.text
	.align	2
	.global	main
	.type	main, %function
main:
	@ args = 0, pretend = 0, frame = 65576
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {r4, fp, lr}
	add	fp, sp, #8
	sub	sp, sp, #65536
	sub	sp, sp, #44
	sub	r3, fp, #65536
	sub	r3, r3, #12
	str	r0, [r3, #-36]
	sub	r3, fp, #65536
	sub	r3, r3, #12
	str	r1, [r3, #-40]
	mov	r3, #4
	str	r3, [fp, #-40]
	mov	r3, #0
	str	r3, [fp, #-24]
	mov	r3, #16384
	str	r3, [fp, #-32]
	mov	r3, #0
	str	r3, [fp, #-36]
	sub	r3, fp, #65536
	sub	r3, r3, #12
	ldr	r3, [r3, #-36]
	cmp	r3, #1
	ble	.L2
	sub	r3, fp, #65536
	sub	r3, r3, #12
	ldr	r3, [r3, #-40]
	add	r3, r3, #4
	ldr	r3, [r3]
	mov	r0, r3
	ldr	r1, .L12
	bl	fopen
	str	r0, [fp, #-36]
	ldr	r0, [fp, #-36]
	mov	r1, #0
	mov	r2, #2
	bl	fseek
	ldr	r0, [fp, #-36]
	bl	ftell
	str	r0, [fp, #-32]
	ldr	r0, [fp, #-36]
	bl	rewind
.L2:
	sub	r3, fp, #65536
	sub	r3, r3, #12
	ldr	r3, [r3, #-36]
	cmp	r3, #2
	ble	.L3
	sub	r3, fp, #65536
	sub	r3, r3, #12
	ldr	r3, [r3, #-40]
	add	r3, r3, #8
	ldr	r3, [r3]
	mov	r0, r3
	bl	atoi
	str	r0, [fp, #-24]
.L3:
	mov	r3, #0
	str	r3, [fp, #-20]
	b	.L4
.L5:
	ldr	r2, [fp, #-24]
	ldr	r3, [fp, #-20]
	add	r4, r2, r3
	ldr	r0, [fp, #-36]
	bl	fgetc
	mov	r3, r0
	uxtb	r2, r3
	sub	r3, fp, #65536
	sub	r3, r3, #12
	add	r3, r3, r4
	strb	r2, [r3, #-32]
	ldr	r3, [fp, #-20]
	add	r3, r3, #1
	str	r3, [fp, #-20]
.L4:
	ldr	r2, [fp, #-20]
	ldr	r3, [fp, #-32]
	cmp	r2, r3
	blt	.L5
	bl	setup_io
	ldr	r0, .L12+4
	bl	puts
	ldr	r3, .L12+8
	ldr	r3, [r3]
	add	r3, r3, #4
	ldr	r2, .L12+8
	ldr	r2, [r2]
	add	r2, r2, #4
	ldr	r2, [r2]
	bic	r2, r2, #14680064
	str	r2, [r3]
	ldr	r3, .L12+8
	ldr	r3, [r3]
	add	r3, r3, #4
	ldr	r2, .L12+8
	ldr	r2, [r2]
	add	r2, r2, #4
	ldr	r2, [r2]
	bic	r2, r2, #1835008
	str	r2, [r3]
	ldr	r3, .L12+8
	ldr	r3, [r3]
	add	r3, r3, #4
	ldr	r2, .L12+8
	ldr	r2, [r2]
	add	r2, r2, #4
	ldr	r2, [r2]
	orr	r2, r2, #262144
	str	r2, [r3]
	ldr	r3, .L12+8
	ldr	r3, [r3]
	add	r3, r3, #4
	ldr	r2, .L12+8
	ldr	r2, [r2]
	add	r2, r2, #4
	ldr	r2, [r2]
	bic	r2, r2, #117440512
	str	r2, [r3]
	ldr	r3, .L12+8
	ldr	r3, [r3]
	add	r3, r3, #4
	ldr	r2, .L12+8
	ldr	r2, [r2]
	add	r2, r2, #4
	ldr	r2, [r2]
	bic	r2, r2, #939524096
	str	r2, [r3]
	ldr	r3, .L12+8
	ldr	r3, [r3]
	add	r3, r3, #8
	ldr	r2, .L12+8
	ldr	r2, [r2]
	add	r2, r2, #8
	ldr	r2, [r2]
	bic	r2, r2, #7
	str	r2, [r3]
	ldr	r3, .L12+8
	ldr	r3, [r3]
	add	r3, r3, #8
	ldr	r2, .L12+8
	ldr	r2, [r2]
	add	r2, r2, #8
	ldr	r2, [r2]
	orr	r2, r2, #1
	str	r2, [r3]
	ldr	r3, .L12+8
	ldr	r3, [r3]
	add	r3, r3, #8
	ldr	r2, .L12+8
	ldr	r2, [r2]
	add	r2, r2, #8
	ldr	r2, [r2]
	bic	r2, r2, #56
	str	r2, [r3]
	ldr	r3, .L12+8
	ldr	r3, [r3]
	add	r3, r3, #8
	ldr	r2, .L12+8
	ldr	r2, [r2]
	add	r2, r2, #8
	ldr	r2, [r2]
	orr	r2, r2, #8
	str	r2, [r3]
	ldr	r3, .L12+8
	ldr	r3, [r3]
	add	r3, r3, #8
	ldr	r2, .L12+8
	ldr	r2, [r2]
	add	r2, r2, #8
	ldr	r2, [r2]
	bic	r2, r2, #448
	str	r2, [r3]
	ldr	r3, .L12+8
	ldr	r3, [r3]
	add	r3, r3, #8
	ldr	r2, .L12+8
	ldr	r2, [r2]
	add	r2, r2, #8
	ldr	r2, [r2]
	orr	r2, r2, #64
	str	r2, [r3]
	ldr	r3, .L12+8
	ldr	r3, [r3]
	add	r3, r3, #8
	ldr	r2, .L12+8
	ldr	r2, [r2]
	add	r2, r2, #8
	ldr	r2, [r2]
	bic	r2, r2, #3584
	str	r2, [r3]
	ldr	r3, .L12+8
	ldr	r3, [r3]
	add	r3, r3, #8
	ldr	r2, .L12+8
	ldr	r2, [r2]
	add	r2, r2, #8
	ldr	r2, [r2]
	orr	r2, r2, #512
	str	r2, [r3]
	ldr	r3, .L12+8
	ldr	r3, [r3]
	add	r3, r3, #28
	mov	r2, #15728640
	str	r2, [r3]
	ldr	r3, .L12+8
	ldr	r3, [r3]
	add	r3, r3, #40
	mov	r2, #65536
	str	r2, [r3]
	mov	r3, #0
	str	r3, [fp, #-16]
	b	.L6
.L7:
	ldr	r3, .L12+8
	ldr	r2, [r3]
	ldr	r3, [fp, #-16]
	ldr	r1, .L12+12
	smull	r0, r1, r1, r3
	mov	r1, r1, asr #2
	mov	r3, r3, asr #31
	rsb	r1, r3, r1
	mov	r3, r1
	mov	r3, r3, asl #2
	add	r0, r2, r3
	ldr	r3, .L12+8
	ldr	r2, [r3]
	mov	r3, r1
	mov	r3, r3, asl #2
	add	r3, r2, r3
	ldr	ip, [r3]
	ldr	r1, [fp, #-16]
	ldr	r3, .L12+12
	smull	r2, r3, r3, r1
	mov	r2, r3, asr #2
	mov	r3, r1, asr #31
	rsb	r2, r3, r2
	mov	r3, r2
	mov	r3, r3, asl #2
	add	r3, r3, r2
	mov	r3, r3, asl #1
	rsb	r2, r3, r1
	mov	r3, r2
	mov	r3, r3, asl #1
	add	r3, r3, r2
	mov	r2, #7
	mov	r3, r2, asl r3
	mvn	r3, r3
	and	r3, r3, ip
	str	r3, [r0]
	ldr	r3, [fp, #-16]
	add	r3, r3, #1
	str	r3, [fp, #-16]
.L6:
	ldr	r3, [fp, #-16]
	cmp	r3, #15
	ble	.L7
	ldr	r3, .L12+8
	ldr	r3, [r3]
	add	r3, r3, #28
	mov	r2, #65536
	str	r2, [r3]
	ldr	r0, .L12+16
	mov	r1, #0
	bl	printf
	ldr	r3, .L12+8
	ldr	r3, [r3]
	add	r3, r3, #40
	mov	r2, #65536
	str	r2, [r3]
.L11:
	ldr	r3, .L12+8
	ldr	r3, [r3]
	add	r3, r3, #52
	ldr	r3, [r3]
	str	r3, [fp, #-44]
	ldr	r2, [fp, #-28]
	ldr	r3, [fp, #-44]
	cmp	r2, r3
	beq	.L8
	ldr	r3, [fp, #-44]
	str	r3, [fp, #-28]
	ldr	r3, [fp, #-44]
	and	r3, r3, #655360
	cmp	r3, #0
	bne	.L8
	ldr	r3, [fp, #-44]
	uxth	r3, r3
	str	r3, [fp, #-20]
	ldr	r3, [fp, #-44]
	and	r3, r3, #262144
	cmp	r3, #0
	bne	.L9
	ldr	r3, .L12+8
	ldr	r3, [r3]
	add	r3, r3, #40
	mov	r2, #255
	str	r2, [r3]
	ldr	r3, .L12+8
	ldr	r3, [r3]
	ldr	r2, .L12+20
	str	r2, [r3]
	ldr	r3, .L12+8
	ldr	r3, [r3]
	add	r3, r3, #28
	sub	r2, fp, #65536
	sub	r2, r2, #12
	sub	r1, r2, #32
	ldr	r2, [fp, #-20]
	add	r2, r1, r2
	ldrb	r2, [r2]	@ zero_extendqisi2
	orr	r2, r2, #65536
	str	r2, [r3]
	mov	r0, r0	@ nop
	ldr	r3, .L12+8
	ldr	r3, [r3]
	add	r3, r3, #52
	ldr	r3, [r3]
	ldr	r3, .L12+8
	ldr	r3, [r3]
	mov	r2, #0
	str	r2, [r3]
	b	.L10
.L9:
	ldr	r3, .L12+8
	ldr	r3, [r3]
	add	r3, r3, #28
	mov	r2, #65536
	str	r2, [r3]
	mov	r0, r0	@ nop
	ldr	r3, .L12+8
	ldr	r3, [r3]
	add	r3, r3, #52
	ldr	r3, [r3]
.L10:
	ldr	r3, .L12+8
	ldr	r3, [r3]
	add	r3, r3, #40
	mov	r2, #65536
	str	r2, [r3]
.L8:
	b	.L11
.L13:
	.align	2
.L12:
	.word	.LC0
	.word	.LC1
	.word	gpio
	.word	1717986919
	.word	.LC2
	.word	2396745
	.size	main, .-main
	.section	.rodata
	.align	2
.LC3:
	.ascii	"/dev/mem\000"
	.align	2
.LC4:
	.ascii	"can't open /dev/mem \000"
	.align	2
.LC5:
	.ascii	"mmap error %d\012\000"
	.text
	.align	2
	.global	setup_io
	.type	setup_io, %function
setup_io:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
	stmfd	sp!, {fp, lr}
	add	fp, sp, #4
	sub	sp, sp, #8
	ldr	r0, .L17
	ldr	r1, .L17+4
	bl	open
	mov	r2, r0
	ldr	r3, .L17+8
	str	r2, [r3]
	ldr	r3, .L17+8
	ldr	r3, [r3]
	cmp	r3, #0
	bge	.L15
	ldr	r0, .L17+12
	bl	puts
	mvn	r0, #0
	bl	exit
.L15:
	ldr	r3, .L17+8
	ldr	r3, [r3]
	str	r3, [sp]
	ldr	r3, .L17+16
	str	r3, [sp, #4]
	mov	r0, #0
	mov	r1, #4096
	mov	r2, #3
	mov	r3, #1
	bl	mmap
	mov	r2, r0
	ldr	r3, .L17+20
	str	r2, [r3]
	ldr	r3, .L17+8
	ldr	r3, [r3]
	mov	r0, r3
	bl	close
	ldr	r3, .L17+20
	ldr	r3, [r3]
	cmn	r3, #1
	bne	.L16
	ldr	r3, .L17+20
	ldr	r3, [r3]
	ldr	r0, .L17+24
	mov	r1, r3
	bl	printf
	mvn	r0, #0
	bl	exit
.L16:
	ldr	r3, .L17+20
	ldr	r3, [r3]
	ldr	r2, .L17+28
	str	r3, [r2]
	sub	sp, fp, #4
	@ sp needed
	ldmfd	sp!, {fp, pc}
.L18:
	.align	2
.L17:
	.word	.LC3
	.word	1052674
	.word	mem_fd
	.word	.LC4
	.word	1059061760
	.word	gpio_map
	.word	.LC5
	.word	gpio
	.size	setup_io, .-setup_io
	.align	2
	.global	clear_io
	.type	clear_io, %function
clear_io:
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 1, uses_anonymous_args = 0
	@ link register save eliminated.
	str	fp, [sp, #-4]!
	add	fp, sp, #0
	sub	sp, fp, #0
	@ sp needed
	ldr	fp, [sp], #4
	bx	lr
	.size	clear_io, .-clear_io
	.ident	"GCC: (Raspbian 4.9.2-10) 4.9.2"
	.section	.note.GNU-stack,"",%progbits
