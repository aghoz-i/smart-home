/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# "Insert system clock initialization code here" comment
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */
#include <asf.h>
#include <stdio.h>
#include <ioport.h>
#include <board.h>
#include <math.h>
#include <adc_sensors/adc_sensors.h>
#include <avr/io.h>

// Set pin J4_PIN0 sebagai pin untuk Fan
#define PORT_FAN 	J4_PIN0

static char strbuf[128];

// Inisiasi PWM untuk servo
void PWM_Init(void)
{
	/* Set output */
	PORTC.DIR |= PIN0_bm;
	PORTA.DIR = PIN0_bm;

	/* Set Register */
	TCC0.CTRLA = (PIN2_bm) | (PIN0_bm);
	TCC0.CTRLB = (PIN4_bm) | (PIN2_bm) | (PIN1_bm);
	
	/* Set Period */
	TCC0.PER = 1000;

	/* Set Compare Register value*/
	TCC0.CCA = 375;
}

// Menampilkan waktu ke layar
void print_time(int sec) {
	if (sec == -1) sec = 0;
	int jam = sec/3600;
	int menit = (sec % 3600) / 60;
	int detik = sec % 60;
	snprintf(strbuf, sizeof(strbuf), "Timer: %2dh %2dm %2ds", jam, menit, detik);
	gfx_mono_draw_string(strbuf, 0, 8, &sysfont);
}


int main (void)
{
	/* Inisiasi board, pwm, dan sensor. */
	board_init();
	PWM_Init();
	gfx_mono_init();
	adc_sensors_init();
	pmic_init();
	cpu_irq_enable();

	// Menyalakan backlight layar
	gpio_set_pin_high(LCD_BACKLIGHT_ENABLE_PIN);

	// Set sec = -1 sebagai flag
	int sec = -1;

	// Set PORTE sebagai output
	PORTE.DIR = 0b11111111;
	// SET CCA = 7 untuk menentukkan arah Servo
	TCC0.CCA = 7;

	// Membaca suhu dari NTC sensor
	ntc_measure();
	while(!ntc_data_is_ready());
	// Mengambil suhu dalam satuan celsius
	volatile int8_t temperature = ntc_get_temperature();

	// State
	// -1: Init state 
	// 0: Temperature < 35
	// 1: Temperature >= 35, Timer < 1 menit
	// 2: Temperature >= 35, Timer >= 1 menit
	int state = -1;

	while(1){
		ntc_measure();
		if (ntc_data_is_ready()) temperature = ntc_get_temperature();

		// Menampilkan suhu di layar
		snprintf(strbuf, sizeof(strbuf), "Tempr : %3d",temperature);
		gfx_mono_draw_string(strbuf,0, 24, &sysfont);

		// Cek apakah suhu kurang dari 35 derajat celsius
		if (temperature < 35) {
			sec = -1;
			if (state != 0) {
				state = 0;

				// Mengatur arah servo
				TCC0.CCA = 7;
				// Mematikan fan
				ioport_set_pin_high(PORT_FAN);
				// Menyalakan LED
				LED_On(LED0);
				LED_On(LED1);

				// Menampilkan tulisan ke layar
				snprintf(strbuf, sizeof(strbuf), "SUHU Normal");
				gfx_mono_draw_string(strbuf, 0, 0, &sysfont);
				snprintf(strbuf, sizeof(strbuf), "Timer mati        ");
				gfx_mono_draw_string(strbuf, 0, 8, &sysfont);
				snprintf(strbuf, sizeof(strbuf), "                         ");
				gfx_mono_draw_string(strbuf, 0, 16, &sysfont);
			}
		} else if (temperature >= 35) {
			if (state == 0) {
				state = 1;

				// Menyalakan kipas
				ioport_set_pin_low(PORT_FAN);
				// Menyalakan LED
				LED_On(LED0);
				LED_Off(LED1);

				// Menampilkan tulisan ke layar
				snprintf(strbuf, sizeof(strbuf), "SUHU Tinggi");
				gfx_mono_draw_string(strbuf, 0, 0, &sysfont);
			} else if (state == 1 && sec >= 60) {
				state = 2;

				// Mengatur arah servo
				TCC0.CCA = 37;

				// Menampilkan tulisan ke layar
				snprintf(strbuf, sizeof(strbuf), "!! > 1 MENIT !!");
				gfx_mono_draw_string(strbuf, 0, 16, &sysfont);
			}
			// Menampilkan waktu ke layar
			print_time(sec);
		}

		// Delay dan increment total waktu berjalan
		delay_ms(350);
		sec++;
	}
}