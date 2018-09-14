#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"

#include "dev/leds.h"

#include <stdlib.h>
#include <stdio.h>
/*---------------------------------------------------------------------------*/
PROCESS(tx_power_process, "p_tx");
AUTOSTART_PROCESSES(&tx_power_process);

#define TX_FREQ CLOCK_SECOND * 20
// #define TX_SIZE 10//in byte
/*---------------------------------------------------------------------------*/
static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from) {
	leds_toggle(LEDS_YELLOW);
	printf("broadcast message received from %d.%d: '%s'\n",
	       from->u8[0], from->u8[1], (char *)packetbuf_dataptr());
}
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;
/*---------------------------------------------------------------------------*/

PROCESS_THREAD(tx_power_process, ev, data) {
	static struct etimer et;

	PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

	PROCESS_BEGIN();

	// char *str = malloc(TX_SIZE);
	// memset(str, '5', TX_SIZE - 1);
	// str[TX_SIZE - 1] = '\0';

	while (1) {
		etimer_set(&et, TX_FREQ);

		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

		broadcast_open(&broadcast, 129, &broadcast_call);
		packetbuf_copyfrom("provaprova", 10);
		broadcast_send(&broadcast);
		printf("broadcast message sent\n");
		broadcast_close(&broadcast);

	}

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
