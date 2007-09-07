
/* Copyright 2007 Anders Waldenborg <anders@0x63.nu> */

#include <dlfcn.h>
#include <stdio.h>
#include "CUnit/Basic.h"
#include "scu.h"

#define ST_NE(x) #x
#define ST(x) ST_NE(x)


int
load_symbol (void *mod, const char *name, void **symbol)
{
	char *e;

	dlerror ();
	*symbol = dlsym (mod, name);
	e = dlerror ();

	if (e != NULL) {
		fprintf (stderr, "Error resolving symbol: %s\n", e);
		return 1;
	}
	return 0;

}

void
load_suite (char *module)
{
	void *mod, *start, *end, *init, *cleanup, *name;
	struct scutestcase *tc;
	CU_pSuite ste = NULL;
	
	mod = dlopen (module, RTLD_NOW);
	if (!mod) {
		fprintf (stderr, "Couldn't open test '%s': %s\n", module, dlerror ());
		return;
	}
	
	if (load_symbol (mod, ST(SCU_LIST_START_NAME), &start))
	    return;
	if (load_symbol (mod, ST(SCU_LIST_END_NAME), &end))
	    return;
	if (load_symbol (mod, ST(SCU_NAME_NAME), &name))
	    return;
	if (load_symbol (mod, ST(SCU_INIT_NAME), &init))
	    return;
	if (load_symbol (mod, ST(SCU_CLEANUP_NAME), &cleanup))
	    return;
	
	ste = CU_add_suite((char *)name, (CU_InitializeFunc)init, (CU_CleanupFunc)cleanup);
	if (!ste) {
		fprintf (stderr, "Couldn't create suite\n");
		return;
	}
	
	tc = (struct scutestcase *)start;
	while (tc < (struct scutestcase *)end) {
		CU_add_test(ste, tc->desc, tc->func);
		tc++;
	}
}

int
main (int argc, char **argv)
{
	int i;

	if (CU_initialize_registry() != CUE_SUCCESS)
		return CU_get_error();

	for (i = 1; i < argc; i++) {
		load_suite (argv[i]);
	}
		
	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();

	return CU_get_error();
}
