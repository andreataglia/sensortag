#include "contiki.h"
#include "net/rime/rime.h"
#include "random.h"

#include "dev/leds.h"

#include <stdlib.h>
#include <stdio.h>
/*---------------------------------------------------------------------------*/
PROCESS(tx_power_process, "p_tx");
AUTOSTART_PROCESSES(&tx_power_process);

#define TX_FREQ CLOCK_SECOND * 2
#define PACKET_SIZE 500//in byte
#define TIME_TO_BEGIN CLOCK_SECOND * 5
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
	PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

	static struct etimer et;
	static struct etimer begin;
	PROCESS_BEGIN();

	//wait before beginning tx
	etimer_set(&begin, TIME_TO_BEGIN);
	PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

	typedef struct {
		volatile uint8_t d[PACKET_SIZE];
	} data_packet_struct;

	static data_packet_struct packet;
	for (int i = 0; i < PACKET_SIZE; ++i) {
		packet.d[i] = i + 1;
	}

	broadcast_open(&broadcast, 129, &broadcast_call);

	while (1) {
		leds_on(LEDS_RED);
		packetbuf_copyfrom(&packet, (int)sizeof(packet));
		broadcast_send(&broadcast);
		leds_off(LEDS_RED);

		etimer_set(&et, TX_FREQ);
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
	}

	broadcast_close(&broadcast);

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
