#include "DynamicPwm.h"
#include "ErrorNo.h"
#include "ExternalEvents.h"
#include "ManagedString.h"
#include "MESEvents.h"
#include "MicroBitAccelerometer.h"
#include "MicroBitButton.h"
#include "MicroBitCompass.h"
#include "MicroBitCompat.h"
#include "MicroBitComponent.h"
#include "MicroBitConfig.h"
#include "MicroBitCoordinateSystem.h"
#include "MicroBitLightSensor.h"
#include "MicroBitDisplay.h"
#include "MicroBitEvent.h"
#include "MicroBitFiber.h"
#include "MicroBitFont.h"
#include "MicroBit.h"
#include "MicroBitHeapAllocator.h"
#include "MicroBitI2C.h"
#include "MicroBitImage.h"
#include "MicroBitIO.h"
#include "MicroBitListener.h"
#include "MicroBitMatrixMaps.h"
#include "MicroBitMessageBus.h"
#include "MicroBitMultiButton.h"
#include "MicroBitPanic.h"
#include "MicroBitPin.h"
#include "MicroBitSerial.h"
#include "MicroBitStorage.h"
#include "MicroBitThermometer.h"
#include "PacketBuffer.h"
#include "RefCounted.h"

#include "serial_api.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

// MicroBitAccelerometer.h
MicroBitAccelerometer::~MicroBitAccelerometer() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
int
MicroBitAccelerometer::configure() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitAccelerometer::update() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitAccelerometer::setPeriod(int period) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitAccelerometer::getPeriod() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitAccelerometer::setRange(int range) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitAccelerometer::getRange() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitAccelerometer::whoAmI() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitAccelerometer::getX(MicroBitCoordinateSystem system) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitAccelerometer::getY(MicroBitCoordinateSystem system) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitAccelerometer::getZ(MicroBitCoordinateSystem system) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitAccelerometer::getPitch() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
float
MicroBitAccelerometer::getPitchRadians() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
int
MicroBitAccelerometer::getRoll() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
float
MicroBitAccelerometer::getRollRadians() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
BasicGesture
MicroBitAccelerometer::getGesture() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
MicroBitAccelerometer::idleTick() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
int
MicroBitAccelerometer::isIdleCallbackNeeded() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
int
MicroBitAccelerometer::writeCommand(uint8_t reg, uint8_t value) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitAccelerometer::readCommand(uint8_t reg, uint8_t* buffer, int length) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
void
MicroBitAccelerometer::recalculatePitchRoll() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
MicroBitAccelerometer::updateGesture() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
int
MicroBitAccelerometer::instantaneousAccelerationSquared() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
BasicGesture
MicroBitAccelerometer::instantaneousPosture() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}

// MicroBitCompass.h
MicroBitCompass::~MicroBitCompass() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
int
MicroBitCompass::configure() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitCompass::setPeriod(int period) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitCompass::getPeriod() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitCompass::heading() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitCompass::whoAmI() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitCompass::getX(MicroBitCoordinateSystem system) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitCompass::getY(MicroBitCoordinateSystem system) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitCompass::getZ(MicroBitCoordinateSystem system) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitCompass::getFieldStrength() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitCompass::readTemperature() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitCompass::calibrate() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
void
MicroBitCompass::calibrateAsync() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
int
MicroBitCompass::calibrateStart() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
void
MicroBitCompass::calibrateEnd() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
MicroBitCompass::setCalibration(CompassSample calibration) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
CompassSample
MicroBitCompass::getCalibration() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
MicroBitCompass::idleTick() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
int
MicroBitCompass::isCalibrated() {
  return true;
}
int
MicroBitCompass::isCalibrating() {
  return false;
}
void
MicroBitCompass::clearCalibration() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
int
MicroBitCompass::isIdleCallbackNeeded() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
int
MicroBitCompass::writeCommand(uint8_t reg, uint8_t value) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitCompass::readCommand(uint8_t reg, uint8_t* buffer, int length) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitCompass::read16(uint8_t reg) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
int
MicroBitCompass::read8(uint8_t reg) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}

// MicroBitDisplay.h
MicroBitDisplay::~MicroBitDisplay() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
MicroBitDisplay::stopAnimation(int delay) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
MicroBitDisplay::animationUpdate() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
MicroBitDisplay::renderFinish() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
MicroBitDisplay::render() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
MicroBitDisplay::renderWithLightSense() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
MicroBitDisplay::renderGreyscale() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
MicroBitDisplay::updateScrollText() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
MicroBitDisplay::updatePrintText() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
MicroBitDisplay::updateScrollImage() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
MicroBitDisplay::updateAnimateImage() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
MicroBitDisplay::sendAnimationCompleteEvent() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
MicroBitDisplay::waitForFreeDisplay() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
MicroBitDisplay::stopAnimation() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
MicroBitDisplay::systemTick() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
int
MicroBitDisplay::printAsync(char c, int delay) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitDisplay::printAsync(ManagedString s, int delay) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitDisplay::printAsync(MicroBitImage i, int x, int y, int alpha, int delay) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitDisplay::print(char c, int delay) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitDisplay::print(ManagedString s, int delay) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitDisplay::print(MicroBitImage i, int x, int y, int alpha, int delay) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitDisplay::scrollAsync(ManagedString s, int delay) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitDisplay::scrollAsync(MicroBitImage image, int delay, int stride) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitDisplay::scroll(ManagedString s, int delay) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitDisplay::scroll(MicroBitImage image, int delay, int stride) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitDisplay::animateAsync(MicroBitImage image, int delay, int stride, int startingPosition) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitDisplay::animate(MicroBitImage image, int delay, int stride, int startingPosition) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitDisplay::setBrightness(int b) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
void
MicroBitDisplay::setDisplayMode(DisplayMode mode) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
int
MicroBitDisplay::getDisplayMode() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitDisplay::getBrightness() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
void
MicroBitDisplay::rotateTo(DisplayRotation position) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
MicroBitDisplay::enable() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
MicroBitDisplay::disable() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
MicroBitDisplay::clear() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
MicroBitDisplay::error(int statusCode) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
MicroBitDisplay::setErrorTimeout(int iterations) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
MicroBitDisplay::setFont(MicroBitFont font) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
MicroBitFont
MicroBitDisplay::getFont() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
MicroBitImage
MicroBitDisplay::screenShot() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
int
MicroBitDisplay::readLightLevel() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}

// MicroBit.h
MicroBit::MicroBit()
    : flags(0x00),
      i2c(MICROBIT_PIN_SDA, MICROBIT_PIN_SCL),
      serial(USBTX, USBRX),
      MessageBus(),
      display(MICROBIT_ID_DISPLAY, MICROBIT_DISPLAY_WIDTH, MICROBIT_DISPLAY_HEIGHT),
      buttonA(MICROBIT_ID_BUTTON_A, MICROBIT_PIN_BUTTON_A, MICROBIT_BUTTON_ALL_EVENTS),
      buttonB(MICROBIT_ID_BUTTON_B, MICROBIT_PIN_BUTTON_B, MICROBIT_BUTTON_ALL_EVENTS),
      buttonAB(MICROBIT_ID_BUTTON_AB, MICROBIT_ID_BUTTON_A, MICROBIT_ID_BUTTON_B),
      accelerometer(MICROBIT_ID_ACCELEROMETER, MMA8653_DEFAULT_ADDR),
      compass(MICROBIT_ID_COMPASS, MAG3110_DEFAULT_ADDR),
      thermometer(MICROBIT_ID_THERMOMETER),
      io(MICROBIT_ID_IO_P0, MICROBIT_ID_IO_P1, MICROBIT_ID_IO_P2, MICROBIT_ID_IO_P3, MICROBIT_ID_IO_P4, MICROBIT_ID_IO_P5,
         MICROBIT_ID_IO_P6, MICROBIT_ID_IO_P7, MICROBIT_ID_IO_P8, MICROBIT_ID_IO_P9, MICROBIT_ID_IO_P10, MICROBIT_ID_IO_P11,
         MICROBIT_ID_IO_P12, MICROBIT_ID_IO_P13, MICROBIT_ID_IO_P14, MICROBIT_ID_IO_P15, MICROBIT_ID_IO_P16, MICROBIT_ID_IO_P19,
         MICROBIT_ID_IO_P20) {
}

void
MicroBit::init() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}

ManagedString
MicroBit::getName() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
ManagedString
MicroBit::getSerial() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}

void
MicroBit::reset() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
int
MicroBit::sleep(int milliseconds) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
void
MicroBit::seedRandom() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
MicroBit::seedRandom(uint32_t seed) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
int
MicroBit::random(int max) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
void
MicroBit::systemTick() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
MicroBit::systemTasks() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
int
MicroBit::addSystemComponent(MicroBitComponent* component) {
  int i = 0;

  while (systemTickComponents[i] != NULL && i < MICROBIT_SYSTEM_COMPONENTS)
    i++;

  if (i == MICROBIT_SYSTEM_COMPONENTS)
    return MICROBIT_NO_RESOURCES;

  systemTickComponents[i] = component;

  return MICROBIT_OK;
}

int
MicroBit::removeSystemComponent(MicroBitComponent* component) {
  int i = 0;

  while (systemTickComponents[i] != component && i < MICROBIT_SYSTEM_COMPONENTS)
    i++;

  if (i == MICROBIT_SYSTEM_COMPONENTS)
    return MICROBIT_INVALID_PARAMETER;

  systemTickComponents[i] = NULL;

  return MICROBIT_OK;
}

int
MicroBit::addIdleComponent(MicroBitComponent* component) {
  int i = 0;

  while (idleThreadComponents[i] != NULL && i < MICROBIT_IDLE_COMPONENTS)
    i++;

  if (i == MICROBIT_IDLE_COMPONENTS)
    return MICROBIT_NO_RESOURCES;

  idleThreadComponents[i] = component;

  return MICROBIT_OK;
}

int
MicroBit::removeIdleComponent(MicroBitComponent* component) {
  int i = 0;

  while (idleThreadComponents[i] != component && i < MICROBIT_IDLE_COMPONENTS)
    i++;

  if (i == MICROBIT_IDLE_COMPONENTS)
    return MICROBIT_INVALID_PARAMETER;

  idleThreadComponents[i] = NULL;

  return MICROBIT_OK;
}

int
MicroBit::setTickPeriod(int speedMs) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBit::getTickPeriod() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}

unsigned long
MicroBit::systemTime() {
  struct timespec ts;
  clock_gettime(CLOCK_BOOTTIME, &ts);
  unsigned long ms = ts.tv_sec * 1000;
  ms += ts.tv_nsec / 1000000ULL;
  return ms;
}

const char*
MicroBit::systemVersion() {
  return MICROBIT_DAL_VERSION;
}

void
MicroBit::panic(int statusCode) {
  fprintf(stderr, "Panic: %d\n", statusCode);
  exit(1);
}

void MicroBit::onABListenerRegisteredEvent(MicroBitEvent) {
  // A user has registered to receive events from the buttonAB multibutton.
  // Disable click events from being generated by ButtonA and ButtonB, and defer the
  // control of this to the multibutton handler.
  //
  // This way, buttons look independent unless a buttonAB is requested, at which
  // point button A+B clicks can be correclty handled without breaking
  // causal ordering.

  buttonA.setEventConfiguration(MICROBIT_BUTTON_SIMPLE_EVENTS);
  buttonB.setEventConfiguration(MICROBIT_BUTTON_SIMPLE_EVENTS);
  buttonAB.setEventConfiguration(MICROBIT_BUTTON_ALL_EVENTS);
}

MicroBit uBit;

// MicroBitI2C.h
int
MicroBitI2C::read(int address, char* data, int length, bool repeated) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitI2C::write(int address, const char* data, int length, bool repeated) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}

// MicroBitImage.h
void
MicroBitImage::init(const int16_t x, const int16_t y, const uint8_t* bitmap) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
MicroBitImage::init_empty() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
ImageData*
MicroBitImage::leakData() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
MicroBitImage::MicroBitImage(const char* s) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
MicroBitImage::clear() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
int
MicroBitImage::setPixelValue(int16_t x, int16_t y, uint8_t value) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitImage::getPixelValue(int16_t x, int16_t y) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitImage::printImage(int16_t x, int16_t y, const uint8_t* bitmap) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitImage::paste(const MicroBitImage& image, int16_t x, int16_t y, uint8_t alpha) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitImage::print(char c, int16_t x, int16_t y) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitImage::shiftLeft(int16_t n) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitImage::shiftRight(int16_t n) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitImage::shiftUp(int16_t n) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitImage::shiftDown(int16_t n) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
ManagedString
MicroBitImage::toString() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
MicroBitImage
MicroBitImage::crop(int startx, int starty, int finx, int finy) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
bool
MicroBitImage::isReadOnly() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return false;
}
MicroBitImage
MicroBitImage::clone() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}

// MicroBitLightSensor.h
void
MicroBitLightSensor::analogReady() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
MicroBitLightSensor::analogDisable() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void MicroBitLightSensor::startSensing(MicroBitEvent) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
int
MicroBitLightSensor::read() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}

// MicroBitMultiButton.h
int
MicroBitMultiButton::isPressed() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
void
MicroBitMultiButton::setEventConfiguration(MicroBitButtonEventConfiguration config) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
MicroBitMultiButton::onButtonEvent(MicroBitEvent evt) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}

// MicroBitPin.h
void
MicroBitPin::disconnect() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
int
MicroBitPin::obtainAnalogChannel() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitPin::setDigitalValue(int value) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitPin::getDigitalValue() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitPin::setAnalogValue(int value) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitPin::setServoValue(int value, int range, int center) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitPin::getAnalogValue() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitPin::isInput() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitPin::isOutput() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitPin::isDigital() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitPin::isAnalog() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitPin::isTouched() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitPin::setServoPulseUs(int pulseWidth) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitPin::setAnalogPeriod(int period) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitPin::setAnalogPeriodUs(int period) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitPin::getAnalogPeriodUs() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitPin::getAnalogPeriod() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}

// MicroBitSerial.h
ssize_t
MicroBitSerial::readChars(void* buffer, size_t length, char eof) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
void
MicroBitSerial::sendString(ManagedString s) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
ManagedString
MicroBitSerial::readString(int len) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
MicroBitSerial::sendImage(MicroBitImage i) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
MicroBitImage
MicroBitSerial::readImage(int width, int height) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
MicroBitSerial::sendDisplayState() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
MicroBitSerial::readDisplayState() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}

// MicroBitStorage.h
int
MicroBitStorage::writeBytes(uint8_t* buffer, uint32_t address, int length) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
void
MicroBitStorage::flashPageErase(uint32_t* page_address) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
void
MicroBitStorage::flashWordWrite(uint32_t* address, uint32_t value) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
MicroBitConfigurationBlock*
MicroBitStorage::getConfigurationBlock() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
int
MicroBitStorage::setConfigurationBlock(MicroBitConfigurationBlock* block) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}

// MicroBitThermometer.h
void
MicroBitThermometer::setPeriod(int period) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
int
MicroBitThermometer::getPeriod() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitThermometer::getTemperature() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
void
MicroBitThermometer::idleTick() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
int
MicroBitThermometer::isIdleCallbackNeeded() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
int
MicroBitThermometer::isSampleNeeded() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
  return 0;
}
void
MicroBitThermometer::updateTemperature() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}

MicroBitAccelerometer::MicroBitAccelerometer(uint16_t id, uint16_t address) : sample(), int1(MICROBIT_PIN_ACCEL_DATA_READY) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
MicroBitCompass::MicroBitCompass(uint16_t id, uint16_t address) : average(), sample(), int1(MICROBIT_PIN_COMPASS_DATA_READY) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
MicroBitDisplay::MicroBitDisplay(uint16_t id, uint8_t x, uint8_t y) : font(), image(x * 2, y) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
MicroBitI2C::MicroBitI2C(PinName sda, PinName scl) : I2C(sda, scl) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
MicroBitImage::MicroBitImage(ImageData* ptr) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
MicroBitImage::MicroBitImage() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
MicroBitImage::MicroBitImage(const MicroBitImage& image) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
MicroBitImage::MicroBitImage(const int16_t x, const int16_t y) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
MicroBitImage::MicroBitImage(const int16_t x, const int16_t y, const uint8_t* bitmap) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
MicroBitImage::~MicroBitImage() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
MicroBitIO::MicroBitIO(int ID_P0, int ID_P1, int ID_P2, int ID_P3, int ID_P4, int ID_P5, int ID_P6, int ID_P7, int ID_P8, int ID_P9,
                       int ID_P10, int ID_P11, int ID_P12, int ID_P13, int ID_P14, int ID_P15, int ID_P16, int ID_P19, int ID_P20)
    : P0(ID_P0, MICROBIT_PIN_P0,
         PIN_CAPABILITY_ALL),  // P0 is the left most pad (ANALOG/DIGITAL/TOUCH)
      P1(ID_P1, MICROBIT_PIN_P1,
         PIN_CAPABILITY_ALL),  // P1 is the middle pad (ANALOG/DIGITAL/TOUCH)
      P2(ID_P2, MICROBIT_PIN_P2,
         PIN_CAPABILITY_ALL),                                 // P2 is the right most pad (ANALOG/DIGITAL/TOUCH)
      P3(ID_P3, MICROBIT_PIN_P3, PIN_CAPABILITY_AD),          // COL1 (ANALOG/DIGITAL)
      P4(ID_P4, MICROBIT_PIN_P4, PIN_CAPABILITY_AD),          // COL2 (ANALOG/DIGITAL)
      P5(ID_P5, MICROBIT_PIN_P5, PIN_CAPABILITY_DIGITAL),     // BTN_A
      P6(ID_P6, MICROBIT_PIN_P6, PIN_CAPABILITY_DIGITAL),     // ROW2
      P7(ID_P7, MICROBIT_PIN_P7, PIN_CAPABILITY_DIGITAL),     // ROW1
      P8(ID_P8, MICROBIT_PIN_P8, PIN_CAPABILITY_DIGITAL),     // PIN 18
      P9(ID_P9, MICROBIT_PIN_P9, PIN_CAPABILITY_DIGITAL),     // ROW3
      P10(ID_P10, MICROBIT_PIN_P10, PIN_CAPABILITY_AD),       // COL3 (ANALOG/DIGITAL)
      P11(ID_P11, MICROBIT_PIN_P11, PIN_CAPABILITY_DIGITAL),  // BTN_B
      P12(ID_P12, MICROBIT_PIN_P12, PIN_CAPABILITY_DIGITAL),  // PIN 20
      P13(ID_P13, MICROBIT_PIN_P13, PIN_CAPABILITY_DIGITAL),  // SCK
      P14(ID_P14, MICROBIT_PIN_P14, PIN_CAPABILITY_DIGITAL),  // MISO
      P15(ID_P15, MICROBIT_PIN_P15, PIN_CAPABILITY_DIGITAL),  // MOSI
      P16(ID_P16, MICROBIT_PIN_P16, PIN_CAPABILITY_DIGITAL),  // PIN 16
      P19(ID_P19, MICROBIT_PIN_P19, PIN_CAPABILITY_DIGITAL),  // SCL
      P20(ID_P20, MICROBIT_PIN_P20, PIN_CAPABILITY_DIGITAL)   // SDA
{
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
MicroBitLightSensor::MicroBitLightSensor() : analogTrigger() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
MicroBitMultiButton::MicroBitMultiButton(uint16_t id, uint16_t button1, uint16_t button2) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
MicroBitPin::MicroBitPin(int id, PinName name, PinCapability capability) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
MicroBitSerial::MicroBitSerial(PinName tx, PinName rx) : Serial(tx, rx) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
MicroBitStorage::MicroBitStorage() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
MicroBitThermometer::MicroBitThermometer(uint16_t id) {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}

ManagedString::ManagedString() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}
ManagedString::~ManagedString() {
  fprintf(stderr, "Unhandled: %s\n", __FUNCTION__);
}

int microbit_heap_init() {
  return MICROBIT_OK;
}

void *microbit_malloc(size_t size) {
  return malloc(size);
}

void
microbit_free(void* p) {
  return free(p);
}
