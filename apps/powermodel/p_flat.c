#include "contiki.h"
#include "dev/watchdog.h"
#include "net/netstack.h"
#include "ti-lib.h"

#include <stdio.h>

/*---------------------------------------------------------------------------*/
PROCESS(p_sensors_process, "power process");
AUTOSTART_PROCESSES(&p_sensors_process);

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(p_sensors_process, ev, data) {

	PROCESS_BEGIN();

	static struct etimer et_periodic;

	printf("-----------> starting flat experiment...\n");
	NETSTACK_MAC.off(0);


	while (1) {
		etimer_set(&et_periodic, CLOCK_SECOND * 20);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		printf("-----------> rebooting...\n");
		ti_lib_sys_ctrl_system_reset();
	}

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
