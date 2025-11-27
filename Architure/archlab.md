[TOC]

## Archlab

### Part A Y86-64程序

规则：给定你一段C语言代码，需要使用$Y86-64$汇编代码写出与其函数上等价的代码

由于$Y86-64$功能有限，你需要将测试的输入也写在代码中

目录下使用 `./yas test.ys` 编译 `./yis test.yo`得到运行的结果

要求对于一个链表进行元素求和，其中C代码如下

```c++
/* $begin examples */
/* linked list element */
typedef struct ELE {
    long val;
    struct ELE *next;
} *list_ptr;

/* sum_list - Sum the elements of a linked list */
long sum_list(list_ptr ls)
{
    long val = 0;
    while (ls) {
	val += ls->val;
	ls = ls->next;
    }
    return val;
}

/* rsum_list - Recursive version of sum_list */
long rsum_list(list_ptr ls)
{
    if (!ls)
	return 0;
    else {
	long val = ls->val;
	long rest = rsum_list(ls->next);
	return val + rest;
    }
}
```



比较简单，直接实现即可

```assembly
#initialization

.pos 0
irmovq stack, %rsp
call main
halt


#sample linked list

.align 8
ele1:
    .quad 0x00a
    .quad ele2
ele2:
    .quad 0x0b0
    .quad ele3
ele3:
    .quad 0xc00
    .quad 0

#main function

main:
    irmovq ele1, %rdi
    call sum_list
    ret

#sum_list function

sum_list:
    irmovq $0, %rax
    jmp test

loop:
    mrmovq (%rdi), %rsi
    addq %rsi, %rax
    mrmovq 8(%rdi), %rdi

test:
    andq %rdi, %rdi
    jne loop
    ret

#set initial adress of %rsp

    .pos 0x200
stack:

```

最后需要多打一个换行才能过编译，我也不知道为什么（

```assembly
#initialization

.pos 0
irmovq stack, %rsp
call main
halt


#sample linked list

.align 8
ele1:
    .quad 0x00a
    .quad ele2
ele2:
    .quad 0x0b0
    .quad ele3
ele3:
    .quad 0xc00
    .quad 0

#main function
main:
    irmovq ele1, %rdi
    irmovq $0, %rax
    call rsum_list
    ret

#recursively calculate the sum of a list

rsum_list:
    pushq %rbp
    andq %rdi, %rdi
    je return
    mrmovq (%rdi), %rbp
    addq %rbp, %rax
    mrmovq 8(%rdi), %rdi
    call rsum_list
return:
    popq %rbp
    ret

#set initial adress of %rsp

    .pos 0x200
stack:

```

注意递归结束的时候，需要恢复被调用者保存寄存器的原始值

```c
/* copy_block - Copy src to dest and return xor checksum of src */
long copy_block(long *src, long *dest, long len)
{
    long result = 0;
    while (len > 0) {
	long val = *src++;
	*dest++ = val;
	result ^= val;
	len--;
    }
    return result;
}
/* $end examples */
```

```assembly
#initialization

.pos 0
irmovq stack, %rsp
call main
halt

#sample

.align 8
# Source block
src:
    .quad 0x00a
    .quad 0x0b0
    .quad 0xc00
# Destination block
dest:
    .quad 0x111
    .quad 0x222
    .quad 0x333

#main function

main:
    irmovq src, %rdi
    irmovq dest, %rsi
    irmovq $3, %rdx
    irmovq $0, %rax
    irmovq $8, %rcx
    irmovq $1, %r8
    call copy_block
    ret

#copy function

copy_block:
    pushq %rbx
test:
    andq %rdx, %rdx
    je return
loop:
    mrmovq (%rdi), %rbx
    xorq %rbx, %rax
    rmmovq %rbx, (%rsi)
    addq %rcx, %rdi
    addq %rcx, %rsi
    subq %r8, %rdx
    jmp test
return:
    popq %rbx
    ret

    .pos 0x200
stack:

```

