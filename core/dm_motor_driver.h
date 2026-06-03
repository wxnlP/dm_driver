#pragma once

#include "canfd_bus_base.h"
#include <cstdint>

using MotorType = float;

constexpr MotorType P_MAX{12.5};
constexpr MotorType V_MAX{30.0};
constexpr MotorType T_MAX{10.0};
constexpr MotorType KP_MAX{500.0};
constexpr MotorType KP_MIN{0.0};
constexpr MotorType KD_MAX{5.0};
constexpr MotorType KD_MIN{0.0};

class DmMotorDriver {
public:
  enum class CtrlMode {
    MIT = 1, // 混合控制（力、位、速、刚度、阻尼）
    POS,     // 位置控制
    VEL,     // 速度控制
  };

  struct MotorState {
    uint32_t id;      // motor id
    int state;        // motor state
    MotorType pos;    // real position
    MotorType vel;    // real velocity
    MotorType torque; // real torque
  };

  struct CtrlCmd {
    CtrlCmd() = default;
    CtrlCmd(MotorType _pos, MotorType _vel, MotorType _torque,
            MotorType _stiffness, MotorType _damping)
        : pos(_pos), vel(_vel), torque(_torque), stiffness(_stiffness), damping(_damping) {}

    MotorType pos; // 位置 rad
    MotorType vel; // 速度 rad/s
    MotorType torque;    // 力矩 N.M
    MotorType stiffness; // 刚度 N/r
    MotorType damping;   // 阻尼 N*s/r
  };

  DmMotorDriver(CanFdBusBase &_canfd_bus, uint8_t _motor_id)
      : canfd_bus_(_canfd_bus), motor_id_(_motor_id) {}
  DmMotorDriver(CanFdBusBase &_canfd_bus, uint8_t _motor_id,
                MotorType _angle_min_limit, MotorType _angle_max_limit)
      : canfd_bus_(_canfd_bus), motor_id_(_motor_id),
        angle_min_limit_(_angle_min_limit), angle_max_limit_(_angle_max_limit) {
    SetAngleLimits(_angle_min_limit, _angle_max_limit);
  }
  virtual ~DmMotorDriver() = default;

  int Enable(bool enable);
  int SetCtrlMode(CtrlMode mode, bool save);
  int SetZeroPos();
  int SetMotorID(uint32_t id, bool save);
  int MoveCtrl(const CtrlCmd &cmd);
  int RequestState();
  int GetLatestState(MotorState &state);
  int ClearError(CtrlMode mode);
  void SetAngleLimits(MotorType min_limit, MotorType max_limit);

private:
  int MITCtrl(MotorType vel, MotorType pos, MotorType torque,
              MotorType stiffness, MotorType damping);
  int PosCtrl(MotorType pos, MotorType vel);
  int VelCtrl(MotorType vel);
  // Register access helpers.
  int ReadRegister(uint8_t reg);
  int WriteRegister(uint8_t reg, const uint8_t *data);
  int SaveRegister(uint8_t reg);

  CanFdBusBase &canfd_bus_;
  uint32_t motor_id_{0x01};
  CtrlMode current_mode_{CtrlMode::MIT};
  MotorType angle_min_limit_{-3.1415};
  MotorType angle_max_limit_{3.1415};
};
