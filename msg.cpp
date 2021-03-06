#include "msg.hpp"
#include <Arduino.h>

double batt_temp = 0.0;
double batt_volt = 0.0;
double volt1 = 0.0;
double volt2 = 0.0;
double tuketim = 0.0;
double speed = 0.0;

const uint8_t start_byte = 0xAA;
Msg current_msg;
uint8_t current_idx = 0;

enum class State{
  BEGIN, LEN1, LEN2, DATAS, CRC1, CRC2
};
State state = State::BEGIN;

const uint16_t CRC_PRESET_VALUE = 0xFFFF;
const uint16_t POLYNOMIAL       = 0x8408; // x^16 + x^12 + x^5 + 1
uint16_t CRCCalc(uint8_t *datas, uint16_t size){
  uint16_t current_crc_value = CRC_PRESET_VALUE;
  for (uint16_t i = 0; i < size; i++){
    current_crc_value = current_crc_value ^ ((uint16_t) datas[i]);
    for (uint16_t j = 0; j < 8; j++){
      if (current_crc_value & 0x0001)
        current_crc_value = (current_crc_value >> 1) ^ POLYNOMIAL;
      else
        current_crc_value = (current_crc_value >> 1);
    }
  }
  current_crc_value = ~current_crc_value;
  return current_crc_value;
}
bool crc_check(){
  return CRCCalc(current_msg.buff, current_msg.len) == current_msg.crc;
}

void recvd_byte(uint8_t byte){
  if (state == State::BEGIN){
    if (byte == start_byte)
      state = State::LEN1;
  }
  else if (state == State::LEN1){
    current_msg.len = byte;
    state = State::LEN2;
  }
  else if (state == State::LEN2){
    if (byte == current_msg.len){
      state = State::DATAS;
      current_idx = 0;
    }
    else state = State::BEGIN;
  }
  else if (state == State::DATAS){
    current_msg.buff[current_idx] = byte;
    current_idx++;
    if (current_idx == current_msg.len) state = State::CRC1;
  }
  else if (state == State::CRC1){
    current_msg.crc = byte << 8;
    state = State::CRC2;
  }
  else if (state == State::CRC2){
    current_msg.crc |= byte;
    if (crc_check()){
      process_msg();
    }
    state = State::BEGIN;
  }
}

void send_msg(uint8_t *datas, uint8_t len){
  uint8_t send_buff[260];
  send_buff[0] = start_byte;
  send_buff[1] = len;
  send_buff[2] = len;
  for (uint8_t i = 0; i < len; i++) send_buff[i + 3] = datas[i];
  uint16_t crc = CRCCalc(datas, len);
  send_buff[len + 3] = crc >> 8;
  send_buff[len + 4] = crc & 0xFF;
  Serial.write(send_buff, len + 5);
}

void send_only_cmd(uint8_t cmd){
  uint8_t datas[1];
  datas[0] = cmd;
  send_msg(datas, 1);
}

uint8_t get_cmd(Msg &msg){
  return msg.buff[0];
}

uint16_t get_uint16(Msg &msg, uint8_t offset){
  uint16_t res = msg.buff[offset + 1] << 8;
  res |= msg.buff[offset + 2] << 0;

  return res;
}

uint32_t get_uint32(Msg &msg, uint8_t offset){
  uint16_t res = msg.buff[offset + 1] << 24;
  res |= msg.buff[offset + 2] << 16;
  res |= msg.buff[offset + 3] << 8;
  res |= msg.buff[offset + 4] << 0;

  return res;
}

void fill_bytes_uint16(uint8_t *bytes, uint16_t data){
  bytes[0] = data >> 8;
  bytes[1] = data & 0XFF;
}
void fill_bytes_uint32(uint8_t *bytes, uint32_t data){
  bytes[0] = data >> 24;
  bytes[1] = (data >> 16) & 0XFF;
  bytes[2] = (data >> 8) & 0XFF;
  bytes[3] = (data >> 0) & 0XFF;
}

void process_msg(){
  uint8_t cmd = get_cmd(current_msg);
  uint8_t resp_bytes[5];
  if (cmd == batt_temp_cmd){
    resp_bytes[0] = batt_temp_cmd;
    fill_bytes_uint16(resp_bytes + 1, batt_temp * 100.0);
    send_msg(resp_bytes, 3);
  }
  else if (cmd == batt_volt_cmd){
    resp_bytes[0] = batt_volt_cmd;
    fill_bytes_uint32(resp_bytes + 1, batt_volt * 100.0);
    send_msg(resp_bytes, 5);
  }
  else if (cmd == volt1_cmd){
    resp_bytes[0] = volt1_cmd;
    fill_bytes_uint32(resp_bytes + 1, volt1 * 100.0);
    send_msg(resp_bytes, 5);
  }
  else if (cmd == volt2_cmd){
    resp_bytes[0] = volt2_cmd;
    fill_bytes_uint32(resp_bytes + 1, volt2 * 100.0);
    send_msg(resp_bytes, 5);
  }
  else if (cmd == tuketim_cmd){
    resp_bytes[0] = tuketim_cmd;
    fill_bytes_uint32(resp_bytes + 1, tuketim * 100.0);
    send_msg(resp_bytes, 5);
  }
  else if (cmd == speed_cmd){
    resp_bytes[0] = speed_cmd;
    fill_bytes_uint16(resp_bytes + 1, speed * 10.0);
    send_msg(resp_bytes, 3);
  }
}

Msg Msg::clone(){
  Msg res;
  res.len = len;
  res.crc = crc;
  for (uint8_t i = 0; i < len; i++) res.buff[i] = buff[i];

  return res;
}
