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

#define PORT_FAN 	J4_PIN0

static char strbuf[128];

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

int main (void)
{
	/* Insert system clock initialization code here (sysclk_init()). */
	board_init();
	PWM_Init();
	gfx_mono_init();
	adc_sensors_init();

	pmic_init();
	cpu_irq_enable();

	gpio_set_pin_high(LCD_BACKLIGHT_ENABLE_PIN);

	int sec = -1;

	// Set PORTE sebagai output
	PORTE.DIR = 0b11111111;
	TCC0.CCA = 7;
	ntc_measure();
	while(!ntc_data_is_ready());
	volatile int8_t temperature = ntc_get_temperature();
	
	while(1){		
		ntc_measure();
		if (ntc_data_is_ready()) temperature = ntc_get_temperature();

		if (temperature >= 30){
			snprintf(strbuf, sizeof(strbuf), "SUHU Tinggi");
			gfx_mono_draw_string(strbuf, 0, 0, &sysfont);
			ioport_set_pin_low(PORT_FAN);
			LED_On(LED0);
			LED_Off(LED1);
			if (sec == -1) sec = 0;
			int jam = sec/3600;
			int menit = (sec % 3600) / 60;
			int detik = sec % 60;
			if (sec > 300 ) {
				TCC0.CCA = 37;
			}
			snprintf(strbuf, sizeof(strbuf), "Timer: %2d %2d %2d", jam, menit, detik);
			gfx_mono_draw_string(strbuf, 0, 8, &sysfont);
		}
		else if (temperature < 30) {
			sec = -1;
			snprintf(strbuf, sizeof(strbuf), "SUHU Normal");
			gfx_mono_draw_string(strbuf, 0, 0, &sysfont);
			ioport_set_pin_high(PORT_FAN);
			LED_On(LED0);
			LED_On(LED1);
			TCC0.CCA = 7;
			snprintf(strbuf, sizeof(strbuf), "Timer mati        ");
			gfx_mono_draw_string(strbuf, 0, 8, &sysfont);
		}

		delay_ms(250);
		sec++;
		snprintf(strbuf, sizeof(strbuf), "Tempr : %3d",temperature);
		gfx_mono_draw_string(strbuf,0, 24, &sysfont);
	}

	/* Insert application code here, after the board has been initialized. */
}