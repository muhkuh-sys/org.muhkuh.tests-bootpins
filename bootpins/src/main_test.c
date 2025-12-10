#include <string.h>

#include "detect.h"
#include "netx_test.h"
#include "rdy_run.h"
#include "systime.h"
#include "uprintf.h"
#include "version.h"


/*-----------------------------------*/


TEST_RESULT_T test_main(TEST_PARAMETER_T *ptTestParam);
TEST_RESULT_T test_main(TEST_PARAMETER_T *ptTestParam)
{
	TEST_RESULT_T tTestResult;
	BOOTPINS_PARAMETER_T *ptTestParams;
	unsigned long ulTimer;
	int iElapsed;


	systime_init();

	uprintf("\f. *** Bootpins test by doc_bacardi@users.sourceforge.net ***\n");
	uprintf("V" VERSION_ALL "\n\n");

	/* Switch off SYS led. */
	rdy_run_setLEDs(RDYRUN_OFF);

	/* Delay for 500ms to decharge the RDY/RUN lines. */
	ulTimer = systime_get_ms();
	do
	{
		iElapsed = systime_elapsed(ulTimer, 500);
	} while( iElapsed==0 );

	/* Get the test parameter. */
	ptTestParams = (BOOTPINS_PARAMETER_T*)(ptTestParam->pvInitParams);
	uprintf("Flags: 0x%08x\n", ptTestParams->ulFlags);
	get_values(ptTestParams);

	rdy_run_setLEDs(RDYRUN_GREEN);
	tTestResult = TEST_RESULT_OK;

	return tTestResult;
}

/*-----------------------------------*/

