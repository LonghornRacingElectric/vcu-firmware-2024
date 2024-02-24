#include "vcu.h"
#include "VcuModel.h"

void vcu_execute(AnalogVoltages &analogVoltages, DriveSwitchState &driveSwitchState, HvcStatus &hvcStatus,
                 PduStatus &pduStatus, InverterStatus &inverterStatus, WheelDisplacements &wheelDisplacements,
                 ImuData &imuData, GpsData &gpsData, VcuOutput &vcuOutput, float deltaTime) {

  bool inverterReady = (!hvcStatus.ams) && (!hvcStatus.imd);

  vcuInput = {
    analogVoltages.apps1,
    analogVoltages.apps2,

    analogVoltages.bse1,
    analogVoltages.bse2,

    analogVoltages.steer,

    wheelDisplacements.fl,
    wheelDisplacements.fr,
    wheelDisplacements.bl,
    wheelDisplacements.br,

    inverterStatus.motorTemp,
    inverterStatus.inverterTemp,
    hvcStatus.packTempMean,

    hvcStatus.stateOfCharge,
    inverterReady,

    hvcStatus.packVoltage,
    hvcStatus.packCurrent,

    pduStatus.lvVoltage,
    pduStatus.lvCurrent,

    driveSwitchState,

    gpsData.latitude,
    gpsData.longitude,
    gpsData.speed,
    gpsData.heading,

    imuData.accel1,
    imuData.accel2,
    imuData.accel3,
    imuData.gyro1,
    imuData.gyro2,
    imuData.gyro3,
  };

  vcuModel.evaluate(&vcuInput, &vcuOutput, deltaTime);
}

void vcu_updateParameters(VcuParameters& params) {
  vcuModel.setParameters(&params);
}
