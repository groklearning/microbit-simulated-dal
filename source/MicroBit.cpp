// Everything that we can't use the default implementation for goes here.
// Where possible, copy the 'real' implementation's .cpp file directly to this directory.
// This file should mostly forward all logic to Hardware.h.
// For things that we need to no-op but not implement, see Unsupported.cpp.

#include "MicroBit.h"
#include "DynamicPwm.h"
#include "ErrorNo.h"
#include "ExternalEvents.h"
#include "MESEvents.h"
#include "ManagedString.h"
#include "MicroBitAccelerometer.h"
#include "MicroBitButton.h"
#include "MicroBitCompass.h"
#include "MicroBitCompat.h"
#include "MicroBitComponent.h"
#include "MicroBitConfig.h"
#include "MicroBitCoordinateSystem.h"
#include "MicroBitDisplay.h"
#include "MicroBitEvent.h"
#include "MicroBitFiber.h"
#include "MicroBitFont.h"
#include "MicroBitHeapAllocator.h"
#include "MicroBitI2C.h"
#include "MicroBitIO.h"
#include "MicroBitImage.h"
#include "MicroBitLightSensor.h"
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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "Hardware.h"

// MicroBitAccelerometer.h
MicroBitAccelerometer::MicroBitAccelerometer(uint16_t id, uint16_t address)
    : sample(), int1(MICROBIT_PIN_ACCEL_DATA_READY) {
}
MicroBitAccelerometer::~MicroBitAccelerometer() {
}
int
MicroBitAccelerometer::update() {
  BasicGesture g;
  get_accelerometer(&sample.x, &sample.y, &sample.z, &g);
  if (g != currentGesture) {
    currentGesture = g;
    MicroBitEvent e(MICROBIT_ID_GESTURE, currentGesture);
  }
  recalculatePitchRoll();
  return 0;
}
int
MicroBitAccelerometer::getX(MicroBitCoordinateSystem system) {
  switch (system) {
    case SIMPLE_CARTESIAN:
      return -sample.x;

    case NORTH_EAST_DOWN:
      return sample.y;

    case RAW:
    default:
      return sample.x;
  }
}
int
MicroBitAccelerometer::getY(MicroBitCoordinateSystem system) {
  switch (system) {
    case SIMPLE_CARTESIAN:
      return -sample.y;

    case NORTH_EAST_DOWN:
      return -sample.x;

    case RAW:
    default:
      return sample.y;
  }
}
int
MicroBitAccelerometer::getZ(MicroBitCoordinateSystem system) {
  switch (system) {
    case NORTH_EAST_DOWN:
      return -sample.z;

    case SIMPLE_CARTESIAN:
    case RAW:
    default:
      return sample.z;
  }
}
BasicGesture
MicroBitAccelerometer::getGesture() {
  return currentGesture;
}
void
MicroBitAccelerometer::idleTick() {
  // Called by the micro:bit micropython code before calls to get*().
  update();
}
int
MicroBitAccelerometer::isIdleCallbackNeeded() {
  return true;
}
void
MicroBitAccelerometer::recalculatePitchRoll() {
  float x = (float)getX(NORTH_EAST_DOWN);
  float y = (float)getY(NORTH_EAST_DOWN);
  float z = (float)getZ(NORTH_EAST_DOWN);

  roll = atan2(getY(NORTH_EAST_DOWN), getZ(NORTH_EAST_DOWN));
  pitch = atan(-x / (y * sin(roll) + z * cos(roll)));
}
float
MicroBitAccelerometer::getRollRadians() {
  return roll;
}
float
MicroBitAccelerometer::getPitchRadians() {
  return pitch;
}

// MicroBitCompass.h
MicroBitCompass::MicroBitCompass(uint16_t id, uint16_t address)
    : average(), sample(), int1(MICROBIT_PIN_COMPASS_DATA_READY) {
}
MicroBitCompass::~MicroBitCompass() {
}
int
MicroBitCompass::configure() {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitCompass::setPeriod(int period) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitCompass::getPeriod() {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitCompass::heading() {
  // Precompute the tilt compensation parameters to improve readability.
  float phi = uBit.accelerometer.getRollRadians();
  float theta = uBit.accelerometer.getPitchRadians();
  float x = (float)getX(NORTH_EAST_DOWN);
  float y = (float)getY(NORTH_EAST_DOWN);
  float z = (float)getZ(NORTH_EAST_DOWN);

  // Precompute cos and sin of pitch and roll angles to make the calculation a little more
  // efficient.
  float sinPhi = sin(phi);
  float cosPhi = cos(phi);
  float sinTheta = sin(theta);
  float cosTheta = cos(theta);

  float bearing = (360 * atan2(z * sinPhi - y * cosPhi,
                               x * cosTheta + y * sinTheta * sinPhi + z * sinTheta * cosPhi)) /
                  (2 * PI);

  if (bearing < 0)
    bearing += 360.0;

  return (int)bearing;
}
int
MicroBitCompass::whoAmI() {
  return 0;
}
int
MicroBitCompass::getX(MicroBitCoordinateSystem system) {
  switch (system)
    {
        case SIMPLE_CARTESIAN:
            return sample.x;

        case NORTH_EAST_DOWN:
            return -sample.y;

        case RAW:
        default:
            return sample.x;
    }
}
int
MicroBitCompass::getY(MicroBitCoordinateSystem system) {
  switch (system)
    {
        case SIMPLE_CARTESIAN:
            return -sample.y;

        case NORTH_EAST_DOWN:
            return sample.x;

        case RAW:
        default:
            return sample.y;
    }
}
int
MicroBitCompass::getZ(MicroBitCoordinateSystem system) {
  switch (system)
    {
        case SIMPLE_CARTESIAN:
        case NORTH_EAST_DOWN:
            return -sample.z;

        case RAW:
        default:
            return sample.z;
    }
}
int
MicroBitCompass::getFieldStrength() {
  double x = getX();
  double y = getY();
  double z = getZ();

  return (int)sqrt(x * x + y * y + z * z);
}
int
MicroBitCompass::readTemperature() {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitCompass::calibrate() {
  return 0;
}
void
MicroBitCompass::calibrateAsync() {
}
int
MicroBitCompass::calibrateStart() {
  return 0;
}
void
MicroBitCompass::calibrateEnd() {
}
void
MicroBitCompass::setCalibration(CompassSample calibration) {
}
CompassSample
MicroBitCompass::getCalibration() {
  return {0, 0, 0};
}
void
MicroBitCompass::idleTick() {
  // Called by the micro:bit micropython code before calls to get*().
  get_magnetometer(&sample.x, &sample.y, &sample.z);
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
}
int
MicroBitCompass::isIdleCallbackNeeded() {
  return true;
}

namespace {
unsigned long _boot_time = 0;
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
      io(MICROBIT_ID_IO_P0, MICROBIT_ID_IO_P1, MICROBIT_ID_IO_P2, MICROBIT_ID_IO_P3,
         MICROBIT_ID_IO_P4, MICROBIT_ID_IO_P5, MICROBIT_ID_IO_P6, MICROBIT_ID_IO_P7,
         MICROBIT_ID_IO_P8, MICROBIT_ID_IO_P9, MICROBIT_ID_IO_P10, MICROBIT_ID_IO_P11,
         MICROBIT_ID_IO_P12, MICROBIT_ID_IO_P13, MICROBIT_ID_IO_P14, MICROBIT_ID_IO_P15,
         MICROBIT_ID_IO_P16, MICROBIT_ID_IO_P19, MICROBIT_ID_IO_P20) {
  _boot_time = systemTime();
}
MicroBit::~MicroBit() {
}

void
MicroBit::init() {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}

void
MicroBit::reset() {
  set_reset_flag();
}
int
MicroBit::sleep(int milliseconds) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
  return 0;
}
void
MicroBit::seedRandom() {
  seedRandom(static_cast<uint32_t>(time(NULL)));
}
void
MicroBit::seedRandom(uint32_t seed) {
  set_random_seed(seed);
}
int
MicroBit::random(int max) {
  return get_random() % max;
}
void
MicroBit::systemTick() {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}
void
MicroBit::systemTasks() {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
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
  int x = MICROBIT_IDLE_COMPONENTS;

  while (idleThreadComponents[i] != component && i < x)
    i++;

  if (i == MICROBIT_IDLE_COMPONENTS)
    return MICROBIT_INVALID_PARAMETER;

  idleThreadComponents[i] = NULL;

  return MICROBIT_OK;
}

int
MicroBit::setTickPeriod(int speedMs) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBit::getTickPeriod() {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
  return 0;
}

unsigned long
MicroBit::systemTime() {
  unsigned long ms = get_macro_ticks() * 6;
  return ms - _boot_time;
}

const char*
MicroBit::systemVersion() {
  return MICROBIT_DAL_VERSION;
}

void
MicroBit::panic(int statusCode) {
  fprintf(stderr, "micro:bit panic: %d\n", statusCode);
  set_panic_flag();
}

ManagedString
MicroBit::getName() {
}
ManagedString
MicroBit::getSerial() {
}

MicroBit uBit;

// MicroBitI2C.h
int
MicroBitI2C::read(int address, char* data, int length, bool repeated) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitI2C::write(int address, const char* data, int length, bool repeated) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
  return 0;
}

// MicroBitLightSensor.h
MicroBitLightSensor::MicroBitLightSensor() : analogTrigger() {
}
void
MicroBitLightSensor::analogReady() {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}
void
MicroBitLightSensor::analogDisable() {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}
void MicroBitLightSensor::startSensing(MicroBitEvent) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}
int
MicroBitLightSensor::read() {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
  return 0;
}

// MicroBitMultiButton.h
MicroBitMultiButton::MicroBitMultiButton(uint16_t id, uint16_t button1, uint16_t button2) {
}
int
MicroBitMultiButton::isPressed() {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
  return 0;
}
void
MicroBitMultiButton::onButtonEvent(MicroBitEvent evt) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}

// MicroBitPin.h
MicroBitPin::MicroBitPin(int id, PinName name, PinCapability capability)
    : capability(capability), name(name) {
}
int
MicroBitPin::setDigitalValue(int value) {
  get_gpio_pin(name).set_output_mode();
  get_gpio_pin(name).set_digital(value);
  return value;
}
int
MicroBitPin::getDigitalValue() {
  get_gpio_pin(name).set_input_mode(PullDown);
  return get_gpio_pin(name).is_high();
}
int
MicroBitPin::setAnalogValue(int value) {
  if (value == 0) {
    setDigitalValue(0);
  } else {
    get_gpio_pin(name).set_pwm(value);
  }
  return 0;
}
int
MicroBitPin::setServoValue(int value, int range, int center) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitPin::getAnalogValue() {
  get_gpio_pin(name).set_input_mode(PullNone);
  return floor((get_gpio_pin(name).get_voltage() / 3.3) * 1023.9);
}
int
MicroBitPin::isInput() {
  return get_gpio_pin(name).is_input();
}
int
MicroBitPin::isOutput() {
  return get_gpio_pin(name).is_output();
}
int
MicroBitPin::isDigital() {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
  return true;
}
int
MicroBitPin::isAnalog() {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
  return false;
}
int
MicroBitPin::isTouched() {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitPin::setServoPulseUs(int pulseWidth) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitPin::setAnalogPeriod(int period) {
  get_gpio_pin(name).set_pwm_period(period * 1000);
  return 0;
}
int
MicroBitPin::setAnalogPeriodUs(int period) {
  get_gpio_pin(name).set_pwm_period(period);
  return 0;
}
int
MicroBitPin::getAnalogPeriodUs() {
  return get_gpio_pin(name).get_pwm_period();
}
int
MicroBitPin::getAnalogPeriod() {
  return get_gpio_pin(name).get_pwm_period() / 1000;
}

// MicroBitSerial.h
MicroBitSerial::MicroBitSerial(PinName tx, PinName rx) : Serial(tx, rx) {
}
ssize_t
MicroBitSerial::readChars(void* buffer, size_t length, char eof) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
  return 0;
}
void
MicroBitSerial::sendString(ManagedString s) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}
ManagedString
MicroBitSerial::readString(int len) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}
void
MicroBitSerial::sendImage(MicroBitImage i) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}
MicroBitImage
MicroBitSerial::readImage(int width, int height) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}
void
MicroBitSerial::sendDisplayState() {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}
void
MicroBitSerial::readDisplayState() {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}

// MicroBitStorage.h
MicroBitStorage::MicroBitStorage() {
}
int
MicroBitStorage::writeBytes(uint8_t* buffer, uint32_t address, int length) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
  return 0;
}
void
MicroBitStorage::flashPageErase(uint32_t* page_address) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}
void
MicroBitStorage::flashWordWrite(uint32_t* address, uint32_t value) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}
MicroBitConfigurationBlock*
MicroBitStorage::getConfigurationBlock() {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}
int
MicroBitStorage::setConfigurationBlock(MicroBitConfigurationBlock* block) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
  return 0;
}

// MicroBitThermometer.h
MicroBitThermometer::MicroBitThermometer(uint16_t id) {
}
void
MicroBitThermometer::setPeriod(int period) {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}
int
MicroBitThermometer::getPeriod() {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
  return 0;
}
int
MicroBitThermometer::getTemperature() {
  int32_t t;
  get_temperature(&t);
  return t;
}
void
MicroBitThermometer::idleTick() {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}
int
MicroBitThermometer::isIdleCallbackNeeded() {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}
int
MicroBitThermometer::isSampleNeeded() {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
  return 0;
}
void
MicroBitThermometer::updateTemperature() {
  fprintf(stderr, "Unsupported: %s\n", __FUNCTION__);
}

MicroBitI2C::MicroBitI2C(PinName sda, PinName scl) : I2C(sda, scl) {
}
MicroBitIO::MicroBitIO(int ID_P0, int ID_P1, int ID_P2, int ID_P3, int ID_P4, int ID_P5, int ID_P6,
                       int ID_P7, int ID_P8, int ID_P9, int ID_P10, int ID_P11, int ID_P12,
                       int ID_P13, int ID_P14, int ID_P15, int ID_P16, int ID_P19, int ID_P20)
    : P0(ID_P0, MICROBIT_PIN_P0,
         PIN_CAPABILITY_ALL),  // P0 is the left most pad (ANALOG/DIGITAL/TOUCH)
      P1(ID_P1, MICROBIT_PIN_P1,
         PIN_CAPABILITY_ALL),  // P1 is the middle pad (ANALOG/DIGITAL/TOUCH)
      P2(ID_P2, MICROBIT_PIN_P2,
         PIN_CAPABILITY_ALL),  // P2 is the right most pad (ANALOG/DIGITAL/TOUCH)
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
}

void
MicroBitDisplay::disable() {
  // The micro:bit code has its own display handling.
  // In microbit_init, it disables the DAL's imlementation.
  // But this also happens on every iteration of mp_run, so we use this to hook
  // the soft reboot (triggered by Ctrl-D).
  static int reboot_count = 0;
  ++reboot_count;
  if (reboot_count >= 2) {
    set_disconnect_flag();
  }
}

// These aren't heavily used, and only for global lifetime objects, so forwarding them
// to system malloc isn't going to affect the realism of the simulation in any noticable
// way.
int
microbit_heap_init() {
  return MICROBIT_OK;
}

void*
microbit_malloc(size_t size) {
  return malloc(size);
}

void
microbit_free(void* p) {
  return free(p);
}
