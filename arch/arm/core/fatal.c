/*
 * Copyright (c) 2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Kernel fatal error handler for ARM Cortex-M
 *
 * This module provides the _NanoFatalErrorHandler() routine for ARM Cortex-M.
 */

#include <toolchain.h>
#include <sections.h>
#include <inttypes.h>

#include <kernel.h>
#include <kernel_structs.h>
#include <misc/printk.h>

/*
 * Define a default ESF for use with _NanoFatalErrorHandler() in the event
 * the caller does not have a NANO_ESF to pass
 */
const NANO_ESF _default_esf = {
	{0xdeaddead}, /* r0/a1 */
	{0xdeaddead}, /* r1/a2 */
	{0xdeaddead}, /* r2/a3 */
	{0xdeaddead}, /* r3/a4 */
	{0xdeaddead}, /* r12/ip */
	{0xdeaddead}, /* r14/lr */
	{0xdeaddead}, /* r15/pc */
	 0xdeaddead,  /* xpsr */
#ifdef CONFIG_FLOAT
	{0xdeaddead, 0xdeaddead, 0xdeaddead, 0xdeaddead,   /* s0 .. s3 */
	 0xdeaddead, 0xdeaddead, 0xdeaddead, 0xdeaddead,    /* s4 .. s7 */
	 0xdeaddead, 0xdeaddead, 0xdeaddead, 0xdeaddead,    /* s8 .. s11 */
	 0xdeaddead, 0xdeaddead, 0xdeaddead, 0xdeaddead},   /* s12 .. s15 */
	0xdeaddead,  /* fpscr */
	0xdeaddead,  /* undefined */
#endif
};

/**
 *
 * @brief Kernel fatal error handler
 *
 * This routine is called when fatal error conditions are detected by software
 * and is responsible only for reporting the error. Once reported, it then
 * invokes the user provided routine _SysFatalErrorHandler() which is
 * responsible for implementing the error handling policy.
 *
 * The caller is expected to always provide a usable ESF. In the event that the
 * fatal error does not have a hardware generated ESF, the caller should either
 * create its own or use a pointer to the global default ESF <_default_esf>.
 *
 * Unlike other arches, this function may return if _SysFatalErrorHandler
 * determines that only the current thread should be aborted and the CPU
 * was in handler mode. PendSV will be asserted in this case and the current
 * thread taken off the run queue. Leaving the exception will immediately
 * trigger a context switch.
 *
 * @param reason the reason that the handler was called
 * @param pEsf pointer to the exception stack frame
 */
void _NanoFatalErrorHandler(unsigned int reason,
					  const NANO_ESF *pEsf)
{
	switch (reason) {
	case _NANO_ERR_INVALID_TASK_EXIT:
		printk("***** Invalid Exit Software Error! *****\n");
		break;

#if defined(CONFIG_STACK_CANARIES)
	case _NANO_ERR_STACK_CHK_FAIL:
		printk("***** Stack Check Fail! *****\n");
		break;
#endif /* CONFIG_STACK_CANARIES */

	case _NANO_ERR_ALLOCATION_FAIL:
		printk("**** Kernel Allocation Failure! ****\n");
		break;

	case _NANO_ERR_KERNEL_OOPS:
		printk("***** Kernel OOPS! *****\n");
		break;

	case _NANO_ERR_KERNEL_PANIC:
		printk("***** Kernel Panic! *****\n");
		break;

	default:
		printk("**** Unknown Fatal Error %d! ****\n", reason);
		break;
	}
	printk("Current thread ID = %p\n"
	       "Faulting instruction address = 0x%x\n",
	       k_current_get(), pEsf->pc);

	/*
	 * Now that the error has been reported, call the user implemented
	 * policy
	 * to respond to the error.  The decisions as to what responses are
	 * appropriate to the various errors are something the customer must
	 * decide.
	 */

	_SysFatalErrorHandler(reason, pEsf);
}
