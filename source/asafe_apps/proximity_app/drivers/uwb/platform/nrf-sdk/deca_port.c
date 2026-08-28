#include "deca_version.h"
#include "deca_interface.h"

#include "api.h"
#include "node_configuration.h"
#include "app_scheduler.h"
#include "board.h"

#include "port_platform.h"

/* This file implements the functions required by decadriver */

static void delay(uint32_t uSeconds)
{
	app_lib_time_timestamp_hp_t start = lib_time->getTimestampHp();
	while (lib_time->getTimeDiffUs(lib_time->getTimestampHp(), start) < uSeconds)
		;
}

void wakeup_device_with_io(void)
{
	/* this would only be used if you use the dwt_wakeup_ic() API, but it's
	 * easier to directly call our own dw3000_hw_wakeup() function */
	// dw3000_hw_wakeup();
	port_wakeup_dw1000();
}

decaIrqStatus_t decamutexon(void)
{
	// bool s = dw3000_hw_interrupt_is_enabled();
	// if (s)
	// {
	// 	dw3000_hw_interrupt_disable();
	// }
	// return s;
	return false;
}

void decamutexoff(decaIrqStatus_t s)
{
	// if (s)
	// {
	// 	dw3000_hw_interrupt_enable();
	// }
}

void deca_sleep(unsigned int time_ms)
{
	// nrf_delay_ms(time_ms);
	delay(time_ms * 100);
}

void deca_usleep(unsigned long time_us)
{
	delay(time_us);
}
