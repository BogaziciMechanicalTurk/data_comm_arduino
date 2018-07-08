#ifndef MSG_HPP
#define MSG_HPP

#include "stdint.h"

const uint8_t batt_temp_cmd = 0x00;
const uint8_t batt_volt_cmd = 0x01;
const uint8_t volt1_cmd = 0x02;
const uint8_t volt2_cmd = 0x03;
const uint8_t tuketim_cmd = 0x04;
const uint8_t speed_cmd = 0x05;

const uint8_t cmds[] = {
  batt_temp_cmd,
  batt_volt_cmd,
  volt1_cmd,
  volt2_cmd,
  tuketim_cmd,
  speed_cmd
};

struct Msg{
  uint8_t len;
  uint8_t buff[256];
  uint16_t crc;

  Msg clone();
};

void recvd_byte(uint8_t byte);

void send_msg(uint8_t *datas, uint8_t len);
void send_only_cmd(uint8_t cmd);
uint8_t get_cmd(Msg &msg);
uint16_t get_uint16(Msg &msg, uint8_t offset);
uint32_t get_uint32(Msg &msg, uint8_t offset);

void process_msg();

extern double batt_temp;
extern double batt_volt;
extern double volt1;
extern double volt2;
extern double tuketim;
extern double speed;

#endif // MSG_HPP
