/**
 * Copyright (C) 2017 Chalmers Revere
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <algorithm>
#include <fstream>
#include <iostream>
#include <math.h>

#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <sys/mman.h> // mmap
#include <stdio.h>

#include <opendavinci/odcore/base/KeyValueConfiguration.h>
#include <opendavinci/odcore/data/Container.h>
#include <opendavinci/odcore/strings/StringToolbox.h>
#include <opendavinci/generated/odcockpit/SimplePlot.h>
#include <opendavinci/generated/odcockpit/RuntimeConfiguration.h>
#include <opendavinci/odcore/base/Lock.h>

#include <odvdminiature/GeneratedHeaders_ODVDMiniature.h>

#include "PwmMotor.h"



namespace opendlv {
namespace proxy {
namespace miniature {

PwmMotor::PwmMotor(const int &argc, char **argv)
    : TimeTriggeredConferenceClientModule(argc, argv, "proxy-miniature-pwm-motor")
    , m_motor()
    , m_mutex()
    , m_prusharedMemInt32_ptr(NULL)
{
  initialisePru();
}

void PwmMotor::initialisePru()
{
  int32_t  bindFileDescriptor;
  uint32_t  *pru;   // Points to start of PRU memory.
  int32_t fileDescriptor;
  
  // reset memory pointer to NULL so if init fails it doesn't point somewhere bad
  m_prusharedMemInt32_ptr = NULL;

  // open file descriptors for pru rproc driver
  bindFileDescriptor = open(PRU_BIND_PATH.c_str(), O_WRONLY);
  if (bindFileDescriptor == -1) {
    std::cerr << "[" << getName() << "] ERROR: pru-rproc driver missing" << std::endl;
  }

  // if pru0 is not loaded, load it
  if (access(PRU0_UEVENT.c_str(), F_OK) != 0) {
    if (write(bindFileDescriptor, PRU0_NAME.c_str(), PRU_NAME_LEN) < 0) {
      std::cerr << "[" << getName() << "] ERROR: pru0 bind failed\n" << std::endl;
    }
  }
  // if pru1 is not loaded, load it
  if(access(PRU1_UEVENT.c_str(), F_OK)!=0){
    if(write(bindFileDescriptor, PRU1_NAME.c_str(), PRU_NAME_LEN)<0){
      std::cerr << "[" << getName() << "] ERROR: pru1 bind failed\n" << std::endl;
    }
  }
  close(bindFileDescriptor);

  // start mmaping shared memory
  fileDescriptor = open("/dev/mem", O_RDWR | O_SYNC);
  if (fileDescriptor == -1) {
    std::cerr << "[" << getName() << "] ERROR: could not open /dev/mem." << std::endl;
  }
  if (isVerbose()) {
    std::cout << "[" << getName() << "] mmap'ing PRU shared memory" << std::endl;
  }
  
  pru = static_cast<unsigned int*>(mmap(0, PRU_LEN, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, PRU_ADDR));
  if (pru == MAP_FAILED) {
    std::cerr << "[" << getName() << "] ERROR: could not map memory." << std::endl;
  }
  close(fileDescriptor);

  m_prusharedMemInt32_ptr = pru + PRU_SHAREDMEM/4; // Points to start of shared memory

  // zero out the 8 servo channels and encoder channel
  memset(m_prusharedMemInt32_ptr, 0, 9*4);

  // zero out 4th encoder, eQEP encoders are already zero'd previously
  setPruEncoderPos(0);
}

PwmMotor::~PwmMotor() 
{
  // delete m_prusharedMemInt32_ptr;
  m_prusharedMemInt32_ptr = NULL;
}

void PwmMotor::setUp()
{
  odcore::base::KeyValueConfiguration kv = getKeyValueConfiguration();

  std::string const channelStr = kv.getValue<std::string>(getName() + ".channels");
  std::string const nameStr = kv.getValue<std::string>(getName() + ".names");
  std::string const typeStr = kv.getValue<std::string>(getName() + ".types");

  std::vector<std::string> listOfChannels = odcore::strings::StringToolbox::split(channelStr,',');
  std::vector<std::string> listOfNames = odcore::strings::StringToolbox::split(nameStr,',');
  std::vector<std::string> listOfTypes = odcore::strings::StringToolbox::split(typeStr,',');

  if (listOfChannels.size() == listOfNames.size() && listOfChannels.size() == listOfTypes.size()
      && (signed) listOfChannels.size() <= SERVO_CHANNELS && listOfChannels.size() > 0) {
    for (uint8_t i = 0; i < listOfChannels.size(); ++i) {
      std::string name = listOfNames.at(i);
      std::string type  = listOfTypes.at(i);
      uint8_t channel = std::stoi(listOfChannels.at(i));
      Motor::MotorType motorType;
      std::transform(type.begin(), type.end(), type.begin(), ::toupper);
      if (type.compare("SERVO") == 0) {
        motorType = Motor::MotorType::Servo;
      } else if (type.compare("ESC") == 0) {
        motorType = Motor::MotorType::Esc;
      } else {
        cerr << "[" << getName() << "] Incorrect configuration for motor type.\n";
      }
      m_motor.push_back(Motor(name, motorType, channel));
    }
  } else {
    cerr << "[" << getName() << "] Invalid configurations.\n";
  }
  powerServoRail(true);
}

void PwmMotor::tearDown()
{
  setServoNormalizedAll(0);
  if (isVerbose()) {
    std::cout << "[" << getName() << "] Turning off servo power rail.\n";
  }
  powerServoRail(false);
}

void PwmMotor::nextContainer(odcore::data::Container &a_container)
{
  // (void) a_container;
  if (a_container.getDataType() == opendlv::proxy::GroundSteeringRequest::ID()) {
    opendlv::proxy::GroundSteeringRequest gsr = 
        a_container.getData<opendlv::proxy::GroundSteeringRequest>();
    float const MAX_STEERING_ANGLE = 22.0f;
    float const MAX_ANGLE = 90.0f;
    float angle = gsr.getSteeringAngle() * 180 / static_cast<float>(M_PI);
    if (angle > MAX_STEERING_ANGLE) {
      angle = MAX_STEERING_ANGLE;
    } else if (angle < -MAX_STEERING_ANGLE) {
      angle = -MAX_STEERING_ANGLE;
    }
    float servoSignal = angle / MAX_ANGLE;
    setMotorPower("steering", servoSignal);
  } else if (a_container.getDataType() == opendlv::proxy::PedalPositionReading::ID()){
    opendlv::proxy::PedalPositionReading ppr = 
        a_container.getData<opendlv::proxy::PedalPositionReading>();
    float power;
    if(a_container.getSenderStamp() == 1) {
      power = ppr.getPercent();
    } else if (a_container.getSenderStamp() == 2) {
      power = -ppr.getPercent();
    }
    power += 1;
    power *= 0.5f;
    setMotorPower("propulsion", power);
  }
  // else if (a_container.getDataType() == 
  //       odcockpit::RuntimeConfiguration::ID()) {
  //   odcockpit::RuntimeConfiguration rc = 
  //       a_container.getData<odcockpit::RuntimeConfiguration>();
  //   if (rc.containsKey_MapOfParameters(m_motorName + ":power")) {
  //     double power = rc.getValueForKey_MapOfParameters(m_motorName + ":power");
  //     SetDutyCycleNs(Power2Pwm(power));
  //   }
  // }
}

void PwmMotor::setMotorPower(std::string a_name, float a_power) {
  for (uint8_t i = 0; i < m_motor.size(); ++i) {
    if (a_name.compare(m_motor.at(i).getName()) == 0) {
      odcore::base::Lock l(m_mutex);
      m_motor.at(i).setPower(a_power);
      break;
    }
  }
}

odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode PwmMotor::body()
{
  uint32_t counter = 0;
  while (getModuleStateAndWaitForRemainingTimeInTimeslice() == 
      odcore::data::dmcp::ModuleStateMessage::RUNNING) {
    actuate();

    if (counter%5 == 0) {
      std::cout << "[" << getName() << "]" << std::endl;
      for (uint8_t i = 0; i < m_motor.size(); ++i) {
        std::cout << "\t" << m_motor[i].toString() << std::endl;
      }
    }
    counter++;
  }
  return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
}

void PwmMotor::actuate()
{
  for (uint8_t i = 0; i < m_motor.size(); ++i) {
    odcore::base::Lock l(m_mutex);
    switch (m_motor.at(i).getType()) {
      case Motor::MotorType::Esc:
        setEscNormalized(m_motor.at(i).getChannel(), m_motor.at(i).getPower());
        break;
      case Motor::MotorType::Servo:
        setServoNormalized(m_motor.at(i).getChannel(), m_motor.at(i).getPower());
        break;
      default:
        break;
    }
  }
}

void PwmMotor::powerServoRail(bool const a_val)
{
  std::string gpioValueFilename = "/sys/class/gpio/gpio80/value";

  std::ofstream gpioValueFile(gpioValueFilename, std::ofstream::out);
  if (gpioValueFile.is_open()) {
    gpioValueFile << static_cast<uint16_t>(a_val);
    gpioValueFile.flush();
  } else {
    cerr << "[" << getName() << "] Could not open " << gpioValueFilename 
        << "." << std::endl;
  }
  gpioValueFile.close();
} 

int32_t PwmMotor::getPruEncoderPos()
{
  if (m_prusharedMemInt32_ptr == NULL) {
    return -1;
  } else {
    return static_cast<int32_t>(m_prusharedMemInt32_ptr[CNT_OFFSET/4]);
  }
}

/*******************************************************************************
* int set_pru_encoder_pos(int val)
* 
* Set the encoder position, return 0 on success, -1 on failure.
*******************************************************************************/
int8_t PwmMotor::setPruEncoderPos(int32_t val)
{
  if (m_prusharedMemInt32_ptr == NULL) {
    return -1;
  } else {
    m_prusharedMemInt32_ptr[CNT_OFFSET/4] = val;
    return 0;
  }
}
/*******************************************************************************
* Sends a single pulse of duration us (microseconds) to a channels.
* This must be called regularly (>40hz) to keep servos or ESCs awake.
*******************************************************************************/
int8_t PwmMotor::setPwmMicroSeconds(uint8_t const a_ch, uint32_t const a_us)
{
  // Sanity Checks
  if(a_ch < 1 || a_ch > SERVO_CHANNELS){
    std::cout << "[" << getName() << "] ERROR: Channel must be between 1 and " << SERVO_CHANNELS << ". \n";
    return -2;
  } if(m_prusharedMemInt32_ptr == NULL){
    std::cout << "[" << getName() << "] ERROR: PRU servo Controller not initialized.\n";
    return -2;
  }

  // first check to make sure no pulse is currently being sent
  if(m_prusharedMemInt32_ptr[a_ch-1] != 0){
    std::cout << "[" << getName() << "] WARNING: Tried to start a new pulse amidst another.\n";
    return -1;
  }

  // PRU runs at 200Mhz. find #loops needed
  uint32_t numLoops = static_cast<uint32_t>((a_us * 200.0f) / PRU_SERVO_LOOP_INSTRUCTIONS); 
  // write to PRU shared memory
  m_prusharedMemInt32_ptr[a_ch-1] = numLoops;
  return 0;
}

/*******************************************************************************
* Sends a single pulse of duration us (microseconds) to all channels.
* This must be called regularly (>40hz) to keep servos or ESCs awake.
*******************************************************************************/
int8_t PwmMotor::setPwmMicroSecondsAll(uint32_t const a_us)
{
  int ret = 0;
  for (uint8_t i = 1; i <= SERVO_CHANNELS; i++) {
    int8_t retCh = setPwmMicroSeconds(i, a_us);
    if (retCh == -2) {
      return -2;
    } else if(retCh == -1) {
      ret = -1;
    }
  }
  return ret;
}

int8_t PwmMotor::setServoNormalized(uint8_t const a_ch, float const a_input)
{
  if (a_ch < 1 || a_ch > SERVO_CHANNELS) {
    std::cout << "[" << getName() << "] ERROR: Channel must be between 1 and " << SERVO_CHANNELS << ". \n";
    return -1;
  }
  if (a_input < -1.5f || a_input > 1.5f) {
    std::cout << "[" << getName() << "] ERROR: Servo normalized input must be between -1.5 and 1.5\n";
    return -1;
  }
  uint32_t us = static_cast<uint32_t>(SERVO_MID_US + (a_input * (SERVO_NORMAL_RANGE / 2)));
  return setPwmMicroSeconds(a_ch, us);
}

int8_t PwmMotor::setServoNormalizedAll(float const a_input)
{
  int ret = 0;
  for (uint8_t i = 1; i <= SERVO_CHANNELS; i++) {
    int8_t retCh = setServoNormalized(i, a_input);
    if (retCh == -2) {
      return -2;
    } else if(retCh == -1) {
      ret = -1;
    }
  }
  return ret;
}

/*******************************************************************************
* normalized input of 0-1 corresponds to output pulse from 1000-2000 us
* input is allowed to go down to -0.1 so ESC can be armed below minimum throttle
*******************************************************************************/
int8_t PwmMotor::setEscNormalized(uint8_t const a_ch, float const a_input)
{
  if (a_ch < 1 || a_ch > SERVO_CHANNELS){
    std::cout << "[" << getName() << "] ERROR: Channel must be between 1 and " << SERVO_CHANNELS << ". \n";
    return -1;
  }
  if (a_input < -0.1f || a_input > 1.0f){
    std::cout << "[" << getName() << "] ERROR: ESC normalized input must be between -0.1 & 1\n";
    return -1;
  }
  uint32_t micros = static_cast<uint32_t>(1000 + (a_input * 1000.0f));
  return setPwmMicroSeconds(a_ch, micros);
}

int8_t PwmMotor::setEscNormalizedAll(float const a_input)
{
  int ret = 0;
  for (uint8_t i = 1; i <= SERVO_CHANNELS; i++) {
    int8_t retCh = setEscNormalized(i, a_input);
    if (retCh == -2) {
      return -2;
    } else if(retCh == -1) {
      ret = -1;
    }
  }
  return ret;
}


/*******************************************************************************
* int rc_send_oneshot_pulse_normalized(int ch, float input)
* 
* normalized input of 0-1 corresponds to output pulse from 125-250 us
* input is allowed to go down to -0.1 so ESC can be armed below minimum throttle
*******************************************************************************/
int8_t PwmMotor::setEscOneshotNormalized(uint8_t const a_ch, float const a_input)
{
  if (a_ch < 1 || a_ch > SERVO_CHANNELS) {
    std::cout << "[" << getName() << "] ERROR: Channel must be between 1 and " << SERVO_CHANNELS << ". \n";
    return -1;
  }
  if(a_input < -0.1f || a_input > 1.0f) {
    std::cout << "[" << getName() << "] ERROR: ESC normalized input must be between -0.1 & 1\n";
    return -1;
  }
  uint32_t micros = static_cast<uint32_t>(125.0f + (a_input*125.0f));
  return setPwmMicroSeconds(a_ch, micros);
}

/*******************************************************************************
* int rc_send_oneshot_pulse_normalized_all(float input)
* 
* 
*******************************************************************************/
int8_t PwmMotor::setEscOneshotNormalizedAll(float const a_input)
{
  int ret = 0;
  for (uint8_t i = 1; i <= SERVO_CHANNELS; i++) {
    int8_t retCh = setEscOneshotNormalized(i, a_input);
    if (retCh == -2) {
      return -2;
    } else if(retCh == -1) {
      ret = -1;
    }
  }
  return ret;
}


}
}
}
