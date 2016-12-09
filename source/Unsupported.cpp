/*
The MIT License (MIT)

Copyright (c) 2016 Grok Learning

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

// Things that we need to provide a definition for to satisfy the linker (e.g. because
// they're used by the default microbit-dal implementation, but those code paths are
// never called by microbit-micropython).
// Either make them completely silent no-ops, or if it's surprising that they're called
// then log to stderr (to catch changes in the microbit-micropython code).

#include "DynamicPwm.h"
#include "ManagedString.h"
#include "RefCounted.h"
#include "PacketBuffer.h"

#include "MicroBit.h"
#include "MicroBitImage.h"
#include "MicroBitDisplay.h"

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
ManagedString::ManagedString() {
}
ManagedString::~ManagedString() {
}
StringData*
ManagedString::leakData() {
}
ManagedString
ManagedString::substring(int16_t start, int16_t length) {
}
char
ManagedString::charAt(int16_t index) {
  return 0;
}
void
ManagedString::initEmpty() {
}
void
ManagedString::initString(const char* str) {
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

// MicroBitImage.h
MicroBitImage::MicroBitImage(const char* s) {
}
MicroBitImage::MicroBitImage() {
}
MicroBitImage::MicroBitImage(const MicroBitImage& image) {
}
MicroBitImage::MicroBitImage(const int16_t x, const int16_t y) {
}
MicroBitImage::MicroBitImage(const int16_t x, const int16_t y, const uint8_t* bitmap) {
}
MicroBitImage::~MicroBitImage() {
}
void
MicroBitImage::init(const int16_t x, const int16_t y, const uint8_t* bitmap) {
}
void
MicroBitImage::init_empty() {
}
ImageData*
MicroBitImage::leakData() {
}
void
MicroBitImage::clear() {
}
int
MicroBitImage::setPixelValue(int16_t x, int16_t y, uint8_t value) {
  return 0;
}
int
MicroBitImage::getPixelValue(int16_t x, int16_t y) {
  return 0;
}
int
MicroBitImage::printImage(int16_t x, int16_t y, const uint8_t* bitmap) {
  return 0;
}
int
MicroBitImage::paste(const MicroBitImage& image, int16_t x, int16_t y, uint8_t alpha) {
  return 0;
}
int
MicroBitImage::print(char c, int16_t x, int16_t y) {
  return 0;
}
int
MicroBitImage::shiftLeft(int16_t n) {
  return 0;
}
int
MicroBitImage::shiftRight(int16_t n) {
  return 0;
}
int
MicroBitImage::shiftUp(int16_t n) {
  return 0;
}
int
MicroBitImage::shiftDown(int16_t n) {
  return 0;
}
ManagedString
MicroBitImage::toString() {
}
MicroBitImage
MicroBitImage::crop(int startx, int starty, int finx, int finy) {
}
bool
MicroBitImage::isReadOnly() {
  return false;
}
MicroBitImage
MicroBitImage::clone() {
}


// MicroBitDisplay.h
MicroBitDisplay::MicroBitDisplay(uint16_t id, uint8_t x, uint8_t y) : font(), image(x * 2, y) {
}
MicroBitDisplay::~MicroBitDisplay() {
}
void
MicroBitDisplay::systemTick() {
}
void
MicroBitDisplay::enable() {
}
