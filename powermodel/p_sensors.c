#include "contiki.h"
#include "sys/etimer.h"
#include "sys/ctimer.h"
#include "dev/leds.h"
#include "dev/watchdog.h"
#include "random.h"
#include "button-sensor.h"
#include "batmon-sensor.h"
#include "board-peripherals.h"
#include "rf-core/rf-ble.h"
#include "net/netstack.h"

#include "ti-lib.h"

#include <stdio.h>
#include <stdint.h>
/*---------------------------------------------------------------------------*/
#define CC26XX_DEMO_LEDS_PERIODIC       LEDS_YELLOW
#define CC26XX_DEMO_LEDS_BUTTON         LEDS_RED
#define CC26XX_DEMO_LEDS_REBOOT         LEDS_ALL
/*---------------------------------------------------------------------------*/
static int sensors_to_run[] = {2}; //1 tmp, 2 opt, 3 bmp, 4 hdc, 5 mpu
#define SENSING_FREQ (CLOCK_SECOND * 15)
#define ALLOW_PRINTF false
#define ALLOW_LED_ON_SENSING false

/*---------------------------------------------------------------------------*/
PROCESS(p_sensors_process, "power process");
AUTOSTART_PROCESSES(&p_sensors_process);

static struct ctimer bmp_timer, opt_timer, hdc_timer, tmp_timer, mpu_timer;
/*---------------------------------------------------------------------------*/
static void init_bmp_reading(void *not_used);
static void init_opt_reading(void *not_used);
static void init_hdc_reading(void *not_used);
static void init_tmp_reading(void *not_used);
static void init_mpu_reading(void *not_used);
/*---------------------------------------------------------------------------*/
static void print_mpu_reading(int reading) {
	if (reading < 0) {
		printf("-");
		reading = -reading;
	}

	printf("%d.%02d", reading / 100, reading % 100);
}
/*---------------------------------------------------------------------------*/
static void get_bmp_reading() {
	int value;

	value = bmp_280_sensor.value(BMP_280_SENSOR_TYPE_PRESS);
	if (ALLOW_PRINTF) {
		if (value != CC26XX_SENSOR_READING_ERROR) {
			printf("BAR: Pressure=%d.%02d hPa\n", value / 100, value % 100);
		} else {
			printf("BAR: Pressure Read Error\n");
		}
	}

	value = bmp_280_sensor.value(BMP_280_SENSOR_TYPE_TEMP);
	if (ALLOW_PRINTF) {
		if (value != CC26XX_SENSOR_READING_ERROR) {
			printf("BAR: Temp=%d.%02d C\n", value / 100, value % 100);
		} else {
			printf("BAR: Temperature Read Error\n");
		}
	}

	SENSORS_DEACTIVATE(bmp_280_sensor);

	ctimer_set(&bmp_timer, SENSING_FREQ, init_bmp_reading, NULL);
}
/*---------------------------------------------------------------------------*/
static void get_tmp_reading() {
	int value;

	value = tmp_007_sensor.value(TMP_007_SENSOR_TYPE_ALL);

	if (value == CC26XX_SENSOR_READING_ERROR) {
		printf("TMP: Ambient Read Error\n");
		return;
	}

	value = tmp_007_sensor.value(TMP_007_SENSOR_TYPE_AMBIENT);
	if (ALLOW_PRINTF) {
		printf("TMP: Ambient=%d.%03d C\n", value / 1000, value % 1000);
	}

	value = tmp_007_sensor.value(TMP_007_SENSOR_TYPE_OBJECT);
	if (ALLOW_PRINTF) {
		printf("TMP: Object=%d.%03d C\n", value / 1000, value % 1000);
	}

	SENSORS_DEACTIVATE(tmp_007_sensor);

	ctimer_set(&tmp_timer, SENSING_FREQ, init_tmp_reading, NULL);
}
/*---------------------------------------------------------------------------*/
static void get_hdc_reading() {
	int value;

	value = hdc_1000_sensor.value(HDC_1000_SENSOR_TYPE_TEMP);
	if (ALLOW_PRINTF) {
		if (value != CC26XX_SENSOR_READING_ERROR) {
			printf("HDC: Temp=%d.%02d C\n", value / 100, value % 100);
		} else {
			printf("HDC: Temp Read Error\n");
		}
	}

	value = hdc_1000_sensor.value(HDC_1000_SENSOR_TYPE_HUMIDITY);
	if (ALLOW_PRINTF) {
		if (value != CC26XX_SENSOR_READING_ERROR) {
			printf("HDC: Humidity=%d.%02d %%RH\n", value / 100, value % 100);
		} else {
			printf("HDC: Humidity Read Error\n");
		}
	}

	ctimer_set(&hdc_timer, SENSING_FREQ, init_hdc_reading, NULL);
}
/*---------------------------------------------------------------------------*/
static void get_light_reading() {
	int value;
	value = opt_3001_sensor.value(0);
	if (ALLOW_PRINTF) {
		if (value != CC26XX_SENSOR_READING_ERROR) {
			printf("OPT: Light=%d.%02d lux\n", value / 100, value % 100);
		} else {
			printf("OPT: Light Read Error\n");
		}
	}

	/* The OPT will turn itself off, so we don't need to call its DEACTIVATE */
	ctimer_set(&opt_timer, SENSING_FREQ, init_opt_reading, NULL);
}
/*---------------------------------------------------------------------------*/
static void get_mpu_reading() {
	int value;

	value = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_GYRO_X);
	if (ALLOW_PRINTF) {

	}
	printf("MPU Gyro: X=");
	print_mpu_reading(value);
	printf(" deg/sec\n");

	value = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_GYRO_Y);
	if (ALLOW_PRINTF) {
		printf("MPU Gyro: Y=");
		print_mpu_reading(value);
		printf(" deg/sec\n");
	}

	value = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_GYRO_Z);
	if (ALLOW_PRINTF) {
		printf("MPU Gyro: Z=");
		print_mpu_reading(value);
		printf(" deg/sec\n");
	}

	value = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_X);
	if (ALLOW_PRINTF) {
		printf("MPU Acc: X=");
		print_mpu_reading(value);
		printf(" G\n");
	}

	value = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_Y);
	if (ALLOW_PRINTF) {
		printf("MPU Acc: Y=");
		print_mpu_reading(value);
		printf(" G\n");
	}

	value = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_Z);
	if (ALLOW_PRINTF) {
		printf("MPU Acc: Z=");
		print_mpu_reading(value);
		printf(" G\n");
	}

	SENSORS_DEACTIVATE(mpu_9250_sensor);

	ctimer_set(&mpu_timer, SENSING_FREQ, init_mpu_reading, NULL);
}
/*---------------------------------------------------------------------------*/
static void init_bmp_reading(void *not_used) {
	SENSORS_ACTIVATE(bmp_280_sensor);
}
/*---------------------------------------------------------------------------*/
static void init_opt_reading(void *not_used) {
	SENSORS_ACTIVATE(opt_3001_sensor);
}
/*---------------------------------------------------------------------------*/
static void init_hdc_reading(void *not_used) {
	SENSORS_ACTIVATE(hdc_1000_sensor);
}
/*---------------------------------------------------------------------------*/
static void init_tmp_reading(void *not_used) {
	SENSORS_ACTIVATE(tmp_007_sensor);
}
/*---------------------------------------------------------------------------*/
static void init_mpu_reading(void *not_used) {
	mpu_9250_sensor.configure(SENSORS_ACTIVE, MPU_9250_SENSOR_TYPE_ALL);
}

/*---------------------------------------------------------------------------*/
/*------------------------Battery Monitoring---------------------------------*/
/*---------------------------------------------------------------------------*/
static void get_sync_sensor_readings(void) {
	int value;

	printf("-----------------------------------------\n");

	value = batmon_sensor.value(BATMON_SENSOR_TYPE_TEMP);
	printf("Bat: Temp=%d C\n", value);

	value = batmon_sensor.value(BATMON_SENSOR_TYPE_VOLT);
	printf("Bat: Volt=%d mV\n", (value * 125) >> 5);

	return;
}
/*---------------------------------------------------------------------------*/
static void init_sensor(int sensor_id) {
	switch (sensor_id) {
	case 1:
		printf("Testing TMP sensor at frequency:%d seconds\n", SENSING_FREQ / CLOCK_SECOND);
		SENSORS_ACTIVATE(tmp_007_sensor);
		break;
	case 2:
		printf("Testing OPT sensor at frequency:%d seconds\n", SENSING_FREQ / CLOCK_SECOND);
		SENSORS_ACTIVATE(opt_3001_sensor);
		break;
	case 3:
		printf("Testing BPM sensor at frequency:%d seconds\n", SENSING_FREQ / CLOCK_SECOND);
		SENSORS_ACTIVATE(bmp_280_sensor);
		break;
	case 4:
		printf("Testing HDC sensor at frequency:%d seconds\n", SENSING_FREQ / CLOCK_SECOND);
		SENSORS_ACTIVATE(hdc_1000_sensor);
		break;
	case 5:
		printf("Testing MPU sensor at frequency:%d seconds\n", SENSING_FREQ / CLOCK_SECOND);
		init_mpu_reading(NULL);
		break;
	default:
		printf("Wrong sensor number\n");
		break;

	}
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(p_sensors_process, ev, data) {

	PROCESS_BEGIN();
	NETSTACK_MAC.off(0);

	printf("-----------> starting sensing...\n");
	int length = sizeof(sensors_to_run) / sizeof(sensors_to_run[0]);

	for (int i = 0; i < length; ++i) {
		init_sensor(sensors_to_run[i]);
	}

	while (1) {

		PROCESS_YIELD();

		if (ev == sensors_event && data == &bmp_280_sensor) {
			get_bmp_reading();
			if (ALLOW_LED_ON_SENSING) {
				leds_toggle(CC26XX_DEMO_LEDS_PERIODIC);
			}
		} else if (ev == sensors_event && data == &opt_3001_sensor) {
			get_light_reading();
			if (ALLOW_LED_ON_SENSING) {
				leds_toggle(CC26XX_DEMO_LEDS_PERIODIC);
			}
		} else if (ev == sensors_event && data == &hdc_1000_sensor) {
			get_hdc_reading();
			if (ALLOW_LED_ON_SENSING) {
				leds_toggle(CC26XX_DEMO_LEDS_PERIODIC);
			}
		} else if (ev == sensors_event && data == &tmp_007_sensor) {
			get_tmp_reading();
			if (ALLOW_LED_ON_SENSING) {
				leds_toggle(CC26XX_DEMO_LEDS_PERIODIC);
			}
		} else if (ev == sensors_event && data == &mpu_9250_sensor) {
			get_mpu_reading();
			if (ALLOW_LED_ON_SENSING) {
				leds_toggle(CC26XX_DEMO_LEDS_PERIODIC);
			}
		}

	}

	PROCESS_END();
}
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 * @}
 */
