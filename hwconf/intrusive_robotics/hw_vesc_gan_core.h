/*
 /*
     Copyright 2016 Benjamin Vedder    benjamin@vedder.se

     This file is part of the VESC firmware.

     The VESC firmware is free software: you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation, either version 3 of the License, or
     (at your option) any later version.

     The VESC firmware is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with this program.  If not, see <http://www.gnu.org/licenses/>.

	Hardware configuration for the Intrusive Robotics VESC GaN ESC:
	  - STM32F405RGT6, 8 MHz HSE
	  - 3x LMG2100R026 GaN half-bridges (TTL PWM, integrated gate driver)
	  - 3x INA241A2 external current-sense amps, 0.5 mOhm shunts, 20 V/V
	  - Three-shunt sensing on PC0/PC1/PC2 (ADC1_IN10 / ADC2_IN11 / ADC3_IN12)
	  - 6S-12S LiPo (22-50.4 V), 60 A continuous target
	  - Sensorless FOC, no Hall sensors, no IMU, no SPI gate driver

	This file is part of the VESC firmware.

	The VESC firmware is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    The VESC firmware is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    */

#ifndef HW_VESC_GAN_CORE_H_
#define HW_VESC_GAN_CORE_H_

#define HW_NAME					"IR_FW1_5"

// HW properties
#define HW_HAS_3_SHUNTS
// INA241 with REF1=VS, REF2=GND outputs (VS/2) + (gain * Rshunt * I), so positive
// motor current produces a rising ADC reading. Do NOT define INVERTED_SHUNT_POLARITY.

// Phase voltage-sense filters on PD2 (single GPIO drives all three analog
// switches), matching the VESC6 reference design. Driven HIGH to close the
// switches (clean phase voltage to the observer at low ERPM). Runtime gating
// still requires foc_phase_filter_enable in the mcconf.
#define HW_HAS_PHASE_FILTERS
#define PHASE_FILTER_GPIO		GPIOD
#define PHASE_FILTER_PIN		2
#define PHASE_FILTER_ON()		palSetPad(PHASE_FILTER_GPIO, PHASE_FILTER_PIN)
#define PHASE_FILTER_OFF()		palClearPad(PHASE_FILTER_GPIO, PHASE_FILTER_PIN)

// LEDs -- TODO: confirm against schematic
#define LED_GREEN_GPIO			GPIOB
#define LED_GREEN_PIN			1
#define LED_RED_GPIO			GPIOB
#define LED_RED_PIN				0

// LEDs are active-LOW: anode to 3_3VCC, cathode returns to the GPIO through
// R9/R10, so the pin must sink current (drive LOW) to light the LED.
#define LED_GREEN_ON()			palClearPad(LED_GREEN_GPIO, LED_GREEN_PIN)
#define LED_GREEN_OFF()			palSetPad(LED_GREEN_GPIO, LED_GREEN_PIN)
#define LED_RED_ON()			palClearPad(LED_RED_GPIO, LED_RED_PIN)
#define LED_RED_OFF()			palSetPad(LED_RED_GPIO, LED_RED_PIN)

/*
 * ADC Vector
 *
 * 0  (1):	IN10	CURR1     (CS_U, PC0)
 * 1  (2):	IN11	CURR2     (CS_V, PC1)
 * 2  (3):	IN12	CURR3     (CS_W, PC2)
 * 3  (1):	IN0		SENS1     (phase U voltage, PA0)
 * 4  (2):	IN1		SENS2     (phase V voltage, PA1)
 * 5  (3):	IN2		SENS3     (phase W voltage, PA2)
 * 6  (1):	IN5		ADC_EXT1  (PA5 -- aux/throttle)
 * 7  (2):	IN6		ADC_EXT2  (PA6 -- aux)
 * 8  (3):	IN3		TEMP_MOS  (board NTC, PA3)
 * 9  (1):	IN14	TEMP_MOTOR (PC4)
 * 10 (2):	IN15	UNUSED    (PC5)
 * 11 (3):	IN13	VIN_SENS  (VBAT divider, PC3)
 * 12 (1):	Vrefint
 * 13 (2):	IN0		(PA0 dup)
 * 14 (3):	IN1		(PA1 dup)
 * 15 (1):	IN8		(PB0)
 * 16 (2):	IN9		(PB1)
 * 17 (3):	IN2		(PA2 dup)
 */

#define HW_ADC_CHANNELS			18
#define HW_ADC_INJ_CHANNELS		3
#define HW_ADC_NBR_CONV			6

// ADC Indexes
#define ADC_IND_SENS1			3
#define ADC_IND_SENS2			4
#define ADC_IND_SENS3			5
#define ADC_IND_CURR1			0
#define ADC_IND_CURR2			1
#define ADC_IND_CURR3			2
#define ADC_IND_VIN_SENS		11
#define ADC_IND_EXT				6
#define ADC_IND_EXT2			7
#define ADC_IND_TEMP_MOS		8
#define ADC_IND_TEMP_MOTOR		9
#define ADC_IND_VREFINT			12

// Component parameters
#ifndef V_REG
#define V_REG					3.3
#endif
// VBAT divider: confirm values against Analog_Power schematic. With 12S charged
// at 50.4 V, target ADC swing < 3.0 V. 39k / 2.7k -> 50.4 V * (2.7 / 41.7) = 3.26 V.
#ifndef VIN_R1
#define VIN_R1					43200.0
#endif
#ifndef VIN_R2
#define VIN_R2					2200.0
#endif
#ifndef CURRENT_AMP_GAIN
#define CURRENT_AMP_GAIN		20.0	// INA241A2
#endif
#ifndef CURRENT_SHUNT_RES
#define CURRENT_SHUNT_RES		0.0005	// 0.5 mOhm
#endif

// Input voltage
#define GET_INPUT_VOLTAGE()		((V_REG / 4095.0) * (float)ADC_Value[ADC_IND_VIN_SENS] * ((VIN_R1 + VIN_R2) / VIN_R2))

// NTC -- TDK NTCG164BH103FT1S, 10k @ 25C, beta = 3380 K. If using NTCG103JF103FT1
// instead, change beta to 3435.
// Board NTC is wired HIGH-side: 3.3V -> NTC -> ADC node (PA3) -> 10k -> GND.
// Heating drops R_ntc, which RAISES the ADC count, so resistance recovery is
// R = 10k * (4095/adc - 1)  (note '*', not '/'). The low-side form read
// temperature backwards (reported temp fell as the board heated).
#define NTC_RES(adc_val)		(10000.0 * ((4095.0 / (float)adc_val) - 1.0))
#define NTC_TEMP(adc_ind)		(1.0 / ((logf(NTC_RES(ADC_Value[adc_ind]) / 10000.0) / 3380.0) + (1.0 / 298.15)) - 273.15)

#define NTC_RES_MOTOR(adc_val)	(10000.0 / ((4095.0 / (float)adc_val) - 1.0))
#define NTC_TEMP_MOTOR(beta)	(1.0 / ((logf(NTC_RES_MOTOR(ADC_Value[ADC_IND_TEMP_MOTOR]) / 10000.0) / beta) + (1.0 / 298.15)) - 273.15)

// Voltage on ADC channel
#define ADC_VOLTS(ch)			((float)ADC_Value[ch] / 4095.0 * V_REG)

// COMM-port ADC GPIOs
#define HW_ADC_EXT_GPIO			GPIOA
#define HW_ADC_EXT_PIN			5
#define HW_ADC_EXT2_GPIO		GPIOA
#define HW_ADC_EXT2_PIN			6

// UART -- TODO: confirm UART3 PB10/PB11 against MCU.kicad_sch
#define HW_UART_DEV				SD3
#define HW_UART_GPIO_AF			GPIO_AF_USART3
#define HW_UART_TX_PORT			GPIOB
#define HW_UART_TX_PIN			10
#define HW_UART_RX_PORT			GPIOB
#define HW_UART_RX_PIN			11

// ICU (servo / PPM input) -- TODO: confirm pin
#define HW_USE_SERVO_TIM4
#define HW_ICU_TIMER			TIM4
#define HW_ICU_TIM_CLK_EN()		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE)
#define HW_ICU_DEV				ICUD4
#define HW_ICU_CHANNEL			ICU_CHANNEL_1
#define HW_ICU_GPIO_AF			GPIO_AF_TIM4
#define HW_ICU_GPIO				GPIOB
#define HW_ICU_PIN				6

// I2C (defined for completeness; design has no I2C peripherals)
#define HW_I2C_DEV				I2CD2
#define HW_I2C_GPIO_AF			GPIO_AF_I2C2
#define HW_I2C_SCL_PORT			GPIOB
#define HW_I2C_SCL_PIN			10
#define HW_I2C_SDA_PORT			GPIOB
#define HW_I2C_SDA_PIN			11

// Hall / encoder pins (sensorless FOC, but firmware references them)
#define HW_HALL_ENC_GPIO1		GPIOC
#define HW_HALL_ENC_PIN1		6
#define HW_HALL_ENC_GPIO2		GPIOC
#define HW_HALL_ENC_PIN2		7
#define HW_HALL_ENC_GPIO3		GPIOC
#define HW_HALL_ENC_PIN3		8
#define HW_ENC_TIM				TIM3
#define HW_ENC_TIM_AF			GPIO_AF_TIM3
#define HW_ENC_TIM_CLK_EN()		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE)
#define HW_ENC_EXTI_PORTSRC		EXTI_PortSourceGPIOC
#define HW_ENC_EXTI_PINSRC		EXTI_PinSource8
#define HW_ENC_EXTI_CH			EXTI9_5_IRQn
#define HW_ENC_EXTI_LINE		EXTI_Line8
#define HW_ENC_EXTI_ISR_VEC		EXTI9_5_IRQHandler
#define HW_ENC_TIM_ISR_CH		TIM3_IRQn
#define HW_ENC_TIM_ISR_VEC		TIM3_IRQHandler

// SPI pins (unused; defined so SPI macros in hw.h resolve)
#define HW_SPI_DEV				SPID1
#define HW_SPI_GPIO_AF			GPIO_AF_SPI1
#define HW_SPI_PORT_NSS			GPIOB
#define HW_SPI_PIN_NSS			11
#define HW_SPI_PORT_SCK			GPIOA
#define HW_SPI_PIN_SCK			5
#define HW_SPI_PORT_MOSI		GPIOB
#define HW_SPI_PIN_MOSI			2
#define HW_SPI_PORT_MISO		GPIOA
#define HW_SPI_PIN_MISO			6

// Phase voltage measurement
#define ADC_V_L1				ADC_Value[ADC_IND_SENS1]
#define ADC_V_L2				ADC_Value[ADC_IND_SENS2]
#define ADC_V_L3				ADC_Value[ADC_IND_SENS3]
#define ADC_V_ZERO				(ADC_Value[ADC_IND_VIN_SENS] / 2)

#define READ_HALL1()			palReadPad(HW_HALL_ENC_GPIO1, HW_HALL_ENC_PIN1)
#define READ_HALL2()			palReadPad(HW_HALL_ENC_GPIO2, HW_HALL_ENC_PIN2)
#define READ_HALL3()			palReadPad(HW_HALL_ENC_GPIO3, HW_HALL_ENC_PIN3)

// GaN dead time. LMG2100R026 has no body diode, so any dead-time conduction
// flows through the GaN reverse-channel at ~2-3 V drop -- expensive at 60 A.
// Bring up at 100 ns, scope switch nodes for shoot-through, then ratchet down
// toward 40 ns (per CLAUDE.md target).
#define HW_DEAD_TIME_NSEC		60.0

// Default mc_configuration overrides for 6S-12S / 60 A target
#ifndef MCCONF_L_MIN_VOLTAGE
#define MCCONF_L_MIN_VOLTAGE			9.0
#endif
#ifndef MCCONF_L_MAX_VOLTAGE
#define MCCONF_L_MAX_VOLTAGE			80.0	// 12S charged = 50.4 V
#endif
#ifndef MCCONF_DEFAULT_MOTOR_TYPE
#define MCCONF_DEFAULT_MOTOR_TYPE		MOTOR_TYPE_FOC
#endif
#ifndef MCCONF_FOC_F_ZV
#define MCCONF_FOC_F_ZV					60000.0
#endif
#ifndef MCCONF_L_MAX_ABS_CURRENT
#define MCCONF_L_MAX_ABS_CURRENT		80.0
#endif
#ifndef MCCONF_FOC_SAMPLE_V0_V7
#define MCCONF_FOC_SAMPLE_V0_V7			false
#endif
#ifndef MCCONF_L_IN_CURRENT_MAX
#define MCCONF_L_IN_CURRENT_MAX			60.0
#endif
#ifndef MCCONF_L_IN_CURRENT_MIN
#define MCCONF_L_IN_CURRENT_MIN			-60.0
#endif

// Hardware limits (upper clamps that the user cannot exceed in VESC Tool)
// LMG2100R026 is 53 A continuous / 93 V continuous. Keep healthy margin.
#define HW_LIM_CURRENT			-100.0, 100.0
#define HW_LIM_CURRENT_IN		-100.0, 100.0
#define HW_LIM_CURRENT_ABS		0.0, 135.0
#define HW_LIM_VIN				8.0, 93.0
#define HW_LIM_ERPM				-200e3, 200e3
#define HW_LIM_DUTY_MIN			0.0, 0.05
#define HW_LIM_DUTY_MAX			0.0, 0.99
#define HW_LIM_TEMP_FET			-40.0, 160.0

#endif /* HW_VESC_GAN_CORE_H_ */
