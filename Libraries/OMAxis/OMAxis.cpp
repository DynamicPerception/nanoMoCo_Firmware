/*
 * OMAxis.cpp
 *
 *  Created on: 08.05.2012
 *      Author: perepelitsa
 */

#include "OMAxis.h"
#include "OMAxisCommands.h"
#include "OMByteBuffer.h"

OMAxis::OMAxis(HardwareSerial& c_serObj, uint8_t c_dePin)
:OMMoCoMaster(c_serObj, c_dePin)
{
}

OMAxis::~OMAxis() {
	// TODO Auto-generated destructor stub
}

/** Set Target Node Address for Subsequent Commands
 
 @param p_addr
 Target node address
 
 */

void OMAxis::target(uint8_t p_addr) {
    m_slaveAddr = p_addr;
}

/** Get Target Node Address for Subsequent Commands
 
 @return
 Target node address
 */

uint8_t OMAxis::target() {
    return m_slaveAddr;
}

/**!
 * Command CMD_CD_INTERVAL.
 * cmd_format:8,8,(16|16)
 **/
bool OMAxis::cdInterval(uint16_t interval) {
	uint32_t param = (CMD_CD_INTERVAL << 16) | (interval);
	return command(m_slaveAddr, OM_PCODE_PDS, param);
}
/**!
 * Command CMD_CD_EXPOSURE
 * cmd_format:8,8,16,32
 **/
bool OMAxis::cdExposure(uint32_t exposure) {
	return command(m_slaveAddr, OM_PCODE_PDS, (uint16_t) CMD_CD_EXPOSURE, exposure);

}
/**!
 * Command
 * cmd_format:8,8,(16|16)
 **/
bool OMAxis::cdFocus(uint16_t focus) {
	uint32_t param = (CMD_CD_FOCUS << 16) | (focus);
	return command(m_slaveAddr, OM_PCODE_PDS, param);
}
/**!
 * Command
 * cmd_format:8,8,(16|16)
 **/
bool OMAxis::cdMaxShots(uint16_t count) {
	uint32_t param = (CMD_CD_MAX_SHOTS << 16) | (count);
	return command(m_slaveAddr, OM_PCODE_PDS, param);

}
/**!
 * Command
 * cmd_format:8,8,(16|16)
 **/
bool OMAxis::cdExposureDelay(uint16_t delay) {
	uint32_t param = (CMD_CD_EXPOSURE_DLY << 16) | (delay);
	return command(m_slaveAddr, OM_PCODE_PDS, param);
}
/**!
 * Command
 * cmd_format:8,8,16,8
 **/
bool OMAxis::cdFocusWShutter(bool isOn) {
	return command(m_slaveAddr, OM_PCODE_PDS, (uint16_t) CMD_CD_FOCUS_WSHUTTER, (uint8_t)isOn);
}
/**!
 * Command
 * cmd_format:8,8,16,8
 **/
bool OMAxis::cdRepeatCycles(uint8_t count) {
	return command(m_slaveAddr, OM_PCODE_PDS, (uint16_t) CMD_CD_REPEAT_CYCLES, count);
}
/**!
 * Command
 * cmd_format:8,8,(16|16)
 **/
bool OMAxis::mdStepsEachInterval(uint16_t steps) {
	uint32_t param = (CMD_MD_STEPS_EACH << 16) | (steps);
	return command(m_slaveAddr, OM_PCODE_PDS, param);
}
/**!
 * Command not implemented
 *
 **/
bool OMAxis::mdNOOP(void) {
 return false;
}
/**!
 * Command
 * cmd_format:8,8,16,8
 **/
bool OMAxis::mdDirection(bool dir) {
	return command(m_slaveAddr, OM_PCODE_PDS, (uint16_t) CMD_MD_DIRECTION, (uint8_t)dir);
}
/**!
 * Command
 * cmd_format:8,8,16,32
 **/
bool OMAxis::mdMaximumTravelSteps(uint32_t steps) {
	return command(m_slaveAddr, OM_PCODE_PDS, (uint16_t) CMD_MD_MAX_TRAVEL_STEPS, steps);
}
/**!
 * Command
 * cmd_format:8,8,16
 **/
bool OMAxis::mdMotorEnable(void) {
	return command(m_slaveAddr, OM_PCODE_PDS, (uint16_t) CMD_MD_MOTOR_ENABLE);
}
/**!
 * Command
 * cmd_format:8,8,16
 **/
bool OMAxis::mdMotorDisable(void) {
	return command(m_slaveAddr, OM_PCODE_PDS, (uint16_t) CMD_MD_MOTOR_DISABLE);
}
/**!
 * Command
 * cmd_format:8,8,16
 **/
bool OMAxis::mdSetHomePositionHere(void) {
	return command(m_slaveAddr, OM_PCODE_PDS,(uint16_t) CMD_MD_SET_HOME_HERE);
}
/**!
 * Command
 * cmd_format:8,8,16,8
 **/
bool OMAxis::mdBackslashAmount(uint8_t steps) {
	return command(m_slaveAddr, OM_PCODE_PDS,(uint16_t) CMD_MD_BACKSLASH_AMO, steps);
}
/**!
 * Command
 * cmd_format:8,8,16,8
 **/
bool OMAxis::mdEasingMode(uint8_t mode) {
    return command(m_slaveAddr, OM_PCODE_PDS, (uint16_t) CMD_MD_EASING_MODE, mode);
}
/**!
 * Command
 * cmd_format:8,8,16,8
 **/
bool OMAxis::mdContinuousMode(bool isOn) {
	return command(m_slaveAddr, OM_PCODE_PDS, (uint16_t) CMD_MD_CONTINUOUS_MODE, (uint8_t) isOn);
}

/**!
 * Command
 * cmd_format:8,8,8
 **/
bool OMAxis::nop() {
	return command(m_slaveAddr, OM_PCODE_PC, (uint8_t) CMD_PC_NOOP);
}

/**!
 * Command
 * cmd_format:8,8,8
 **/
bool OMAxis::start() {
	return command(m_slaveAddr, OM_PCODE_PC, (uint8_t) CMD_PC_START);
}

/**!
 * Command
 * cmd_format:8,8,8
 **/
bool OMAxis::pause() {
	return command(m_slaveAddr, OM_PCODE_PC, (uint8_t) CMD_PC_PAUSE);
}

/**!
 * Command
 * cmd_format:8,8,8
 **/
bool OMAxis::cameraEnable() {
	return command(m_slaveAddr, OM_PCODE_PC, (uint8_t) CMD_PC_CAM_ENABLE);
}

/**!
 * Command
 * cmd_format:8,8,8
 **/
bool OMAxis::cameraDisable() {
	return command(m_slaveAddr, OM_PCODE_PC, (uint8_t) CMD_PC_CAM_DISABLE);

}

/**!
 * Command
 * cmd_format:8,8,(8|8|16)
 **/
bool OMAxis::moveMotor(bool dir, uint16_t steps) {
	uint32_t param = (CMD_PC_MOVE_MOTOR_SMPL << 24) | ((uint32_t)dir << 16) | (steps);
	return command(m_slaveAddr, OM_PCODE_PC, param);
}

/**!
 * Command
 * cmd_format:8,8,8
 **/
bool OMAxis::sendMotorHome() {
	return command(m_slaveAddr, OM_PCODE_PC, (uint8_t) CMD_PC_SEND_MOTOR_HOME);
}

/**!
 * Command
 * cmd_format:8,8,8,32
 **/
bool OMAxis::exposeNow(uint32_t exposure) {
	return command(m_slaveAddr, OM_PCODE_PC, (uint8_t) CMD_PC_EXPOSE_NOW, exposure);
}

/**!
 * Command
 * cmd_format:8,8,16
 **/
bool OMAxis::motorSleep(bool isSleep) {
	//repack args:
	uint16_t param = (CMD_PC_MOTOR_SLEEP << 8) | ((uint8_t)isSleep);
	return command(m_slaveAddr, OM_PCODE_PC, param);
}

/**!
 * Command
 * cmd_format:8,8,8,16
 **/
bool OMAxis::motorMaxStepRate(uint16_t stepsPerSecond) {
	return command(m_slaveAddr, OM_PCODE_PC, (uint8_t) CMD_PC_MOTOR_MAX_RATE, stepsPerSecond);

}

/**!
 * Command
 * cmd_format:8,8,8,16
 **/
bool OMAxis::motorMaxStepSpeed(uint16_t stepsPerSecond) {
	return command(m_slaveAddr, OM_PCODE_PC, (uint8_t) CMD_PC_MOTOR_MAX_SPEED, stepsPerSecond);
}

/**!
 * Command
 * cmd_format:8,8,8
 **/
bool OMAxis::nop1() {
	return command(m_slaveAddr, OM_PCODE_PC, (uint8_t) CMD_PC_NOOP);
}
/**!
 * Command
 * cmd_format:8,8,8
 **/
bool OMAxis::nop2() {
	return command(m_slaveAddr, OM_PCODE_PC, (uint8_t) CMD_PC_NOOP);
}

/**!
 * Command
 * cmd_format:8,8,8
 **/
bool OMAxis::nop3() {
	return command(m_slaveAddr, OM_PCODE_PC, (uint8_t) CMD_PC_NOOP);
}

/**!
 * Command DebugLED
 * cmd_format:8,8,16
 * **/
bool OMAxis::debugLed(bool isOn) {
	//repack
	uint16_t param = (CMD_PC_DEBUG_LED << 8) | ((uint8_t)isOn);
	return command(m_slaveAddr, OM_PCODE_PC, param);
}

/**!
 * Command Stop Motor.
 * cmd_format:8,8,8
 **/
bool OMAxis::stopMotorNow() {
	return command(m_slaveAddr, OM_PCODE_PC, (uint8_t) CMD_PC_STOP_MOTOR_NOW);
}

/**!
 * Command Micro Step Level.
 * cmd_format:8,8,16
 **/
bool OMAxis::microStepLevel(uint8_t level) {
	//repack
	uint16_t param = (CMD_PC_MICROSTEP_LVL << 8) | (level);
	return command(m_slaveAddr, OM_PCODE_PC, param);
}
/**!
 * Command
 * cmd_format:8,8,16
 **/
bool OMAxis::timingMaster(bool isMaster) {
	//repack
	uint16_t param = (CMD_PC_TIMING_MASTER << 8) |((uint8_t) isMaster);
	return command(m_slaveAddr, OM_PCODE_PC, param);
}
/**!
 * Command
 * use ByteBuffer
 **/
bool OMAxis::moveMotor(bool dir, uint32_t steps,
		uint32_t totalTravelTime, uint32_t accelTime, uint32_t decelTime) {
	OMByteBuffer sendBuf;
	uint8_t _buf[22];
	sendBuf.assign(_buf, sizeof(_buf));
	sendBuf.append((uint8_t) CMD_PC_MOVE_MOTOR_CPLX);
	sendBuf.append(dir);
	sendBuf.append(htonl(steps));
	sendBuf.append((uint8_t)OM_PDU_ESC);
	sendBuf.append(htonl(totalTravelTime));
	sendBuf.append((uint8_t)OM_PDU_ESC);
	sendBuf.append(htonl(accelTime));
	sendBuf.append((uint8_t)OM_PDU_ESC);
	sendBuf.append(htonl(decelTime));
	int res = command(m_slaveAddr, OM_PCODE_PC, sendBuf.getBuf(),
			sendBuf.size());
	if (res == 0)
		return true;
	else
		return false;
}
/**!
 * Command
 * use OMByteBuffer
 **/
bool OMAxis::planInterleavedTravel(bool dir, uint32_t steps,
		uint32_t totalTravelIntervals, uint32_t accelIntervals,
		uint32_t decelIntervals) {
	OMByteBuffer sendBuf;
	uint8_t _buf[22];
	sendBuf.assign(_buf, sizeof(_buf));
	sendBuf.append((uint8_t) CMD_PC_PLAN_INTER_TRAVEL);
	sendBuf.append(dir);
	sendBuf.append(htonl(steps));
	sendBuf.append((uint8_t)OM_PDU_ESC);
	sendBuf.append(htonl(totalTravelIntervals));
	sendBuf.append((uint8_t)OM_PDU_ESC);
	sendBuf.append(htonl(accelIntervals));
	sendBuf.append((uint8_t)OM_PDU_ESC);
	sendBuf.append(htonl(decelIntervals));
	int res = command(m_slaveAddr, OM_PCODE_PC, sendBuf.getBuf(),
			sendBuf.size());
	if (res == 0)
		return true;
	else
		return false;
}
/**!
 * Command
 * cmd_format:8,8,8
 **/
bool OMAxis::cleanPlannedTravel() {
	return command(m_slaveAddr, OM_PCODE_PC, (uint8_t) CMD_PC_CLEAR_PLAN_TRAVEL);
}

/**!
 * Command
 * cmd_format:8,8,16 with response
 **/
bool OMAxis::statusRequest(uint8_t statusType, unsigned char* blob) {
	uint16_t param = ((CMD_PC_STATUS_REQ << 8 ) | (statusType));
	int res = command(m_slaveAddr, OM_PCODE_PC, param);
	if (res == 0) {
		memcpy(blob, buffer(), bufferLen());
		return true;
	}
	else
		return false;
}


