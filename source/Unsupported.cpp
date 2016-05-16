#include "DynamicPwm.h"
#include "ManagedString.h"
#include "RefCounted.h"
#include "PacketBuffer.h"

#include <stdio.h>
#include <stdlib.h>

// DynamicPwm.h
void
DynamicPwm::redirect(PinName pin) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
DynamicPwm*
DynamicPwm::allocate(PinName pin, PwmPersistence persistence) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
DynamicPwm::release() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
int
DynamicPwm::write(float value) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
PinName
DynamicPwm::getPinName() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
int
DynamicPwm::getValue() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
DynamicPwm::getPeriodUs() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
DynamicPwm::getPeriod() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
DynamicPwm::setPeriodUs(int period) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
DynamicPwm::setPeriod(int period) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}

// ManagedString.h
StringData*
ManagedString::leakData() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
ManagedString
ManagedString::substring(int16_t start, int16_t length) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
char
ManagedString::charAt(int16_t index) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
void
ManagedString::initEmpty() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
ManagedString::initString(const char* str) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}


// PacketBuffer.h
uint8_t*
PacketBuffer::getBytes() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
PacketBuffer::init(uint8_t* data, int length, int rssi) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
int
PacketBuffer::setByte(int position, uint8_t value) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
PacketBuffer::getByte(int position) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
PacketBuffer::length() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
PacketBuffer::getRSSI() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
void
PacketBuffer::setRSSI(uint8_t rssi) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}

// RefCounted.h
void
RefCounted::incr() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
RefCounted::decr() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
RefCounted::init() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
bool
RefCounted::isReadOnly() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return false;
}
