/*
 * 25-JUL-2024
 * STM32 HAL NRF24 LIBRARY
 */

#include <stdio.h>
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_spi.h"
#include "stm32f1xx_hal_tim.h"
#include "NRF24_reg_addresses.h"
#include "NRF24.h"
#include <string.h>

static TIM_HandleTypeDef* micros_timer = NULL;

void csn_high(struct NRFConfig radio){
	HAL_GPIO_WritePin(radio.csn_gpio_port, radio.csn_gpio_pin, 1);
}

void csn_low(struct NRFConfig radio){
	HAL_GPIO_WritePin(radio.csn_gpio_port, radio.csn_gpio_pin, 0);
}

void ce_high(struct NRFConfig radio){
	HAL_GPIO_WritePin(radio.ce_gpio_port, radio.ce_gpio_pin, 1);
}

void ce_low(struct NRFConfig radio){
	HAL_GPIO_WritePin(radio.ce_gpio_port, radio.ce_gpio_pin, 0);
}

void nrf24_w_reg(struct NRFConfig radio, uint8_t reg, uint8_t *data, uint8_t size){

	uint8_t cmd = W_REGISTER | reg;

	uint8_t temp_buffer[size+1];
	temp_buffer[0] = cmd;
	memcpy(&temp_buffer[1], data, size);

	csn_low(radio);
	HAL_SPI_Transmit(radio.hspiX, temp_buffer, size + 1, radio.spi_w_timeout);
	csn_high(radio);
}

uint8_t nrf24_r_reg(struct NRFConfig radio, uint8_t reg){
	uint8_t cmd = R_REGISTER | reg;

	uint8_t out_buffer[2];
	out_buffer[0] = cmd;
	out_buffer[1] = 0;
	uint8_t in_buffer[2];

	csn_low(radio);
	HAL_SPI_TransmitReceive(radio.hspiX, out_buffer, in_buffer, 2, radio.spi_rw_timeout);
	csn_high(radio);

	return in_buffer[1];
}

void nrf24_w_spec_cmd(struct NRFConfig radio, uint8_t cmd){
	HAL_SPI_Transmit(radio.hspiX, &cmd, 1, radio.spi_w_timeout);
}

void nrf24_w_spec_reg(struct NRFConfig radio, uint8_t *data, uint8_t size){
	HAL_SPI_Transmit(radio.hspiX, data, size, radio.spi_w_timeout);
}

void nrf24_r_spec_reg(struct NRFConfig radio, uint8_t *data, uint8_t size){
	HAL_SPI_Receive(radio.hspiX, data, size, radio.spi_r_timeout);
}

void nrf24_pwr_up(struct NRFConfig radio){
	uint8_t data = 0;

	data = nrf24_r_reg(radio, CONFIG);

	data |= (1 << PWR_UP);

	nrf24_w_reg(radio, CONFIG, &data, 1);
}

void nrf24_pwr_dwn(struct NRFConfig radio){
	uint8_t data = 0;

	data = nrf24_r_reg(radio, CONFIG);

	data &= ~(1 << PWR_UP);

	nrf24_w_reg(radio, CONFIG, &data, 1);
}

void nrf24_tx_pwr(struct NRFConfig radio, uint8_t pwr){
	uint8_t data = 0;

	data = nrf24_r_reg(radio, RF_SETUP);

	data &= 0xF8;

	data |= (pwr << RF_PWR) + 1; // Always have LNA on

	nrf24_w_reg(radio, RF_SETUP, &data, 1);
}

void nrf24_data_rate(struct NRFConfig radio, uint8_t bps){
	uint8_t data = 0;

	data = nrf24_r_reg(radio, RF_SETUP);

	data &= ~(1 << RF_DR_LOW) & ~(1 << RF_DR_HIGH);

	if(bps == _2mbps){
		data |= (1 << RF_DR_HIGH);
	}else if(bps == _250kbps){
		data |= (1 << RF_DR_LOW);
	}

	nrf24_w_reg(radio, RF_SETUP, &data, 1);
}

void nrf24_set_channel(struct NRFConfig radio, uint8_t ch){
	nrf24_w_reg(radio, RF_CH, &ch, 1);
}

void nrf24_open_tx_pipe(struct NRFConfig radio, uint8_t *addr){
	nrf24_w_reg(radio, TX_ADDR, addr, 5);
}

void nrf24_pipe_pld_size(struct NRFConfig radio, uint8_t pipe, uint8_t size){
	if(size > 32){
		size = 32;
	}

	switch(pipe){
	case 0:
		nrf24_w_reg(radio, RX_PW_P0, &size, 1);

		break;
	case 1:
		nrf24_w_reg(radio, RX_PW_P1, &size, 1);

		break;
	case 2:
		nrf24_w_reg(radio, RX_PW_P2, &size, 1);

		break;
	case 3:
		nrf24_w_reg(radio, RX_PW_P3, &size, 1);

		break;
	case 4:
		nrf24_w_reg(radio, RX_PW_P4, &size, 1);

		break;
	case 5:
		nrf24_w_reg(radio, RX_PW_P5, &size, 1);

		break;
	}
}

void nrf24_open_rx_pipe(struct NRFConfig radio, uint8_t pipe, uint8_t *addr){

	uint8_t data = 0;

	data = nrf24_r_reg(radio, EN_RXADDR);

	switch(pipe){
	case 0:
		nrf24_w_reg(radio, RX_ADDR_P0, addr, 5);

		data |= (1 << ERX_P0);
		break;
	case 1:
		nrf24_w_reg(radio, RX_ADDR_P1, addr, 5);

		data |= (1 << ERX_P1);
		break;
	case 2:
		nrf24_w_reg(radio, RX_ADDR_P2, addr, 1);

		data |= (1 << ERX_P2);
		break;
	case 3:
		nrf24_w_reg(radio, RX_ADDR_P3, addr, 1);

		data |= (1 << ERX_P3);
		break;
	case 4:
		nrf24_w_reg(radio, RX_ADDR_P4, addr, 1);

		data |= (1 << ERX_P4);
		break;
	case 5:
		nrf24_w_reg(radio, RX_ADDR_P5, addr, 1);

		data |= (1 << ERX_P5);
		break;
	}

	nrf24_w_reg(radio, EN_RXADDR, &data, 1);
}

void nrf24_cls_rx_pipe(struct NRFConfig radio, uint8_t pipe){
	uint8_t data = nrf24_r_reg(radio, EN_RXADDR);

	data &= ~(1 << pipe);

	nrf24_w_reg(radio, EN_RXADDR, &data, 1);
}

void nrf24_set_crc(struct NRFConfig radio, uint8_t en_crc, uint8_t crc0){
	uint8_t data = nrf24_r_reg(radio, CONFIG);

	data &= ~(1 << EN_CRC) & ~(1 << CRCO);

	data |= (en_crc << EN_CRC) | (crc0 << CRCO);

	nrf24_w_reg(radio, CONFIG, &data, 1);
}

void nrf24_set_addr_width(struct NRFConfig radio, uint8_t bytes){
	bytes -= 2;
	nrf24_w_reg(radio, SETUP_AW, &bytes, 1);
}

void nrf24_flush_tx(struct NRFConfig radio){
	csn_low(radio);
	nrf24_w_spec_cmd(radio, FLUSH_TX);
	csn_high(radio);
}

void nrf24_flush_rx(struct NRFConfig radio){
	csn_low(radio);
	nrf24_w_spec_cmd(radio, FLUSH_RX);
	csn_high(radio);
}

uint8_t nrf24_r_status(struct NRFConfig radio){
	uint8_t data = 0;
	uint8_t cmd = NOP_CMD;

	csn_low(radio);
	HAL_SPI_TransmitReceive(radio.hspiX, &cmd, &data, 1, radio.spi_rw_timeout);
	csn_high(radio);

	return data;
}

void nrf24_clear_rx_dr(struct NRFConfig radio){
	uint8_t data = 0;

	data = nrf24_r_status(radio);

	data |= (1 << RX_DR);

	nrf24_w_reg(radio, STATUS, &data, 1);
}

void nrf24_clear_tx_ds(struct NRFConfig radio){
	uint8_t data = 0;

	data = nrf24_r_status(radio);

	data |= (1 << TX_DS);

    nrf24_w_reg(radio, STATUS, &data, 1);
}

void nrf24_clear_max_rt(struct NRFConfig radio){
	uint8_t data = 0;

	data = nrf24_r_status(radio);

	data |= (1 << MAX_RT);

    nrf24_w_reg(radio, STATUS, &data, 1);
}

uint8_t nrf24_read_bit(struct NRFConfig radio, uint8_t reg, uint8_t bit){

	if(nrf24_r_reg(radio, reg) & (1 << bit)){
		return 1;
	}

	return 0;
}

void nrf24_set_bit(struct NRFConfig radio, uint8_t reg, uint8_t bit, uint8_t val){
	uint8_t data = 0;

	data = nrf24_r_reg(radio, reg);

	if(val){
		data |= (1 << bit);
	}else{
		data &= ~(1 << bit);
	}

    nrf24_w_reg(radio, reg, &data, 1);
}

uint8_t nrf24_r_pld_wid(struct NRFConfig radio){
	uint8_t width = 0;

	csn_low(radio);
	nrf24_w_spec_cmd(radio, R_RX_PL_WID);
	nrf24_r_spec_reg(radio, &width, 1);
	csn_high(radio);

	return width;
}

void nrf24_listen(struct NRFConfig radio){
	uint8_t data = 0;

	data = nrf24_r_reg(radio, CONFIG);

	data |= (1 << PRIM_RX);

	nrf24_w_reg(radio, CONFIG, &data, 1);

	ce_high(radio);
}

void nrf24_stop_listen(struct NRFConfig radio){
	uint8_t data = 0;

	data = nrf24_r_reg(radio, CONFIG);

	data &= ~(1 << PRIM_RX);

	nrf24_w_reg(radio, CONFIG, &data, 1);
}

void nrf24_dpl(struct NRFConfig radio, uint8_t en){
	uint8_t feature = nrf24_r_reg(radio, FEATURE);

	if(en == enable){
		feature |= (1 << EN_DPL);
	}else{
		feature &= ~(1 << EN_DPL);
	}

	nrf24_w_reg(radio, FEATURE, &feature, 1);
}

void nrf24_set_rx_dpl(struct NRFConfig radio, uint8_t pipe, uint8_t en){

	uint8_t dynpd = nrf24_r_reg(radio, DYNPD);

	if(pipe > 5){
		pipe = 5;
	}

	if(en){
		dynpd |= (1 << pipe);
	}else{
		dynpd &= ~(1 << pipe);
	}

	nrf24_w_reg(radio, DYNPD, &dynpd, 1);
}

void nrf24_auto_ack(struct NRFConfig radio, uint8_t pipe, uint8_t ack){

	if(pipe > 5){
		pipe = 5;
	}

	uint8_t enaa = nrf24_r_reg(radio, EN_AA);

	if(ack){
		enaa |= (1 << pipe);
	}else{
		enaa &= ~(1 << pipe);
	}

	nrf24_w_reg(radio, EN_AA, &enaa, 1);
}

void nrf24_auto_ack_all(struct NRFConfig radio, uint8_t ack){
	uint8_t enaa = nrf24_r_reg(radio, EN_AA);

	if(ack){
		enaa = 63;
	}else{
		enaa = 0;
	}

	nrf24_w_reg(radio, EN_AA, &enaa, 1);
}

void nrf24_en_ack_pld(struct NRFConfig radio, uint8_t en){
	uint8_t feature = nrf24_r_reg(radio, FEATURE);

	if(en){
		feature |= (1 << EN_ACK_PAY);
	}else{
		feature &= ~(1 << EN_ACK_PAY);
	}

	nrf24_w_reg(radio, FEATURE, &feature, 1);
}

void nrf24_en_dyn_ack(struct NRFConfig radio, uint8_t en){
	uint8_t feature = nrf24_r_reg(radio, FEATURE);

	if(en){
		feature |= (1 << EN_DYN_ACK);
	}else{
		feature &= ~(1 << EN_DYN_ACK);
	}

	nrf24_w_reg(radio, FEATURE, &feature, 1);
}

void nrf24_auto_retr_delay(struct NRFConfig radio, uint8_t delay){
	uint8_t data = nrf24_r_reg(radio, SETUP_RETR);

	data &= 15;

	data |= (delay << ARD);

	nrf24_w_reg(radio, SETUP_RETR, &data, 1);
}

void nrf24_auto_retr_limit(struct NRFConfig radio, uint8_t limit){
	uint8_t data = nrf24_r_reg(radio, SETUP_RETR);

	data &= 240;

	data |= (limit << ARC);

	nrf24_w_reg(radio, SETUP_RETR, &data, 1);
}

void nrf24_type_to_uint8_t(size_t in, uint8_t* out, uint16_t size){
	for(uint16_t i = 0; i < size; i++){
		out[i] = (((in & (255 << (i*8)))) >> (i*8));
	}
}

size_t nrf24_uint8_t_to_type(uint8_t* in, uint16_t size){
	size_t out = 0;

	for(uint16_t i = 0; i < size; i++){
		out |= (in[i] << (8*i));
	}

	return out;
}


uint8_t nrf24_transmit(struct NRFConfig radio, uint8_t *data, uint8_t size){

	ce_low(radio);

	uint8_t cmd = W_TX_PAYLOAD;
	uint8_t out_buffer[size + 1];
	out_buffer[0] = cmd;
	memcpy(&out_buffer[1], data, size);

	csn_low(radio);
	HAL_SPI_Transmit(radio.hspiX, out_buffer, size + 1, radio.spi_w_timeout);
	csn_high(radio);

	ce_high(radio);
	delay_us(1000);
	ce_low(radio);

	if(nrf24_r_status(radio) & (1 << MAX_RT)){
		nrf24_clear_max_rt(radio);
		nrf24_flush_tx(radio);
		return 1;
	}

	return 0;
}

void nrf24_transmit_no_ack(struct NRFConfig radio, uint8_t *data, uint8_t size){

	ce_low(radio);

	uint8_t cmd = W_TX_PAYLOAD;
	uint8_t out_buffer[size + 1];
	out_buffer[0] = cmd;
	memcpy(&out_buffer[1], data, size);

	csn_low(radio);
	HAL_SPI_Transmit(radio.hspiX, out_buffer, size + 1, radio.spi_w_timeout);
	csn_high(radio);

	ce_high(radio);
	HAL_Delay(1);
	ce_low(radio);
}

void nrf24_transmit_rx_ack_pld(struct NRFConfig radio, uint8_t pipe, uint8_t *data, uint8_t size){

	if(pipe > 5){
		pipe = 5;
	}

	uint8_t cmd = (W_ACK_PAYLOAD | pipe);
	uint8_t out_buffer[size + 1];
	out_buffer[0] = cmd;
	memcpy(&out_buffer[1], data, size);

	csn_low(radio);
	HAL_SPI_Transmit(radio.hspiX, out_buffer, size + 1, radio.spi_w_timeout);
	csn_high(radio);
}

uint8_t nrf24_carrier_detect(struct NRFConfig radio){
	return nrf24_r_reg(radio, RPD);
}

uint8_t nrf24_data_available(struct NRFConfig radio){

 	uint8_t reg_dt = nrf24_r_reg(radio, FIFO_STATUS);

	if(!(reg_dt & (1 << RX_EMPTY))){
		return 1;
	}

	return 0;
}

void nrf24_receive(struct NRFConfig radio, uint8_t *data, uint8_t size){
	uint8_t cmd = R_RX_PAYLOAD;

	csn_low(radio);
	HAL_SPI_Transmit(radio.hspiX, &cmd, 1, radio.spi_w_timeout);
	HAL_SPI_Receive(radio.hspiX, data, size, radio.spi_r_timeout);
	csn_high(radio);

	nrf24_clear_rx_dr(radio);
}

void delay_us(uint16_t del_time){
	if (micros_timer == NULL) return;

	__HAL_TIM_SET_COUNTER(micros_timer, 0);
	uint16_t tmp_t = __HAL_TIM_GET_COUNTER(micros_timer);
	while((__HAL_TIM_GET_COUNTER(micros_timer)-tmp_t) < del_time){
		;
	}
}

void set_us_clock(TIM_HandleTypeDef* us_timer) {
	micros_timer = us_timer;
}

void nrf24_start_const_carrier(){

}

void nrf24_stop_const_carrier(){

}

void nrf24_defaults(struct NRFConfig radio){
	ce_low(radio);

	nrf24_tx_pwr(radio, _0dbm);
	nrf24_data_rate(radio, _1mbps);
	nrf24_set_channel(radio, 76);
	nrf24_set_crc(radio, en_crc, _2byte);
	nrf24_set_addr_width(radio, 5);
	nrf24_flush_tx(radio);
	nrf24_flush_rx(radio);
	nrf24_clear_rx_dr(radio);
	nrf24_clear_tx_ds(radio);
	nrf24_clear_max_rt(radio);
	nrf24_stop_listen(radio);
	nrf24_dpl(radio, disable);
	nrf24_en_ack_pld(radio, enable);
	nrf24_en_dyn_ack(radio, disable);
	nrf24_auto_retr_delay(radio, 2); // Delay is in increments of 250us
	nrf24_auto_retr_limit(radio, 15);

	for(uint8_t i = 0; i < 6; i++){
		nrf24_pipe_pld_size(radio, i, 0);
		nrf24_cls_rx_pipe(radio, i);
		nrf24_set_rx_dpl(radio, i, disable);
		nrf24_auto_ack(radio, i, enable);
	}
}

void nrf24_init(struct NRFConfig radio, TIM_HandleTypeDef* us_timer){

	if (micros_timer == NULL) {
		micros_timer = us_timer;
		if(HAL_TIM_Base_Start(micros_timer) != HAL_OK){
			Error_Handler();
		}
	}

	ce_high(radio);
	HAL_Delay(5);
	ce_low(radio);

	nrf24_pwr_up(radio);

	HAL_Delay(50);

	uint8_t test_expected = 1;
	const uint8_t TEST_REG = RF_CH; // Use channel register for testing since it is never touched by the chip itself
	nrf24_w_reg(radio, TEST_REG, &test_expected, 1);
	HAL_Delay(10);
	uint8_t test_returned = nrf24_r_reg(radio, TEST_REG);
	if (test_expected != test_returned) {
		printf("Failed to read back register on radio. Sent %02X got %02X\r\n", test_expected, test_returned);
		Error_Handler();
	}

	nrf24_defaults(radio);
}

