package teknic

/*
#include "go_teknic.h"
*/
import "C"

type Motor struct {
	m    C.Motor_t
	opts C.MoveOptions_t
}

func (m *Motor) SetTorque(percent float64) {
	if percent < 0 {
		percent = 100
	} else if percent > 100 {
		percent = 100
	}

	m.opts.TrqLimit = C.double(percent)
}

func (m *Motor) SetAcceleration(rpm float64) {
	m.opts.AccLimit = C.double(rpm)
}

func (m *Motor) SetVelocity(rpm float64) {
	m.opts.VelLimit = C.double(rpm)
}

func (m *Motor) Move(units int32) {
	C.motorMove(m.m, C.int32_t(units), false, m.opts)
}

func (m *Motor) MoveTo(position int32) {
	C.motorMove(m.m, C.int32_t(position), true, m.opts)
}

func (m *Motor) Wait() {
	for !C.motorMoveIsDone(m.m) {
	}
}

func (m *Motor) Halt() {
	C.motorHalt(m.m)
}

func (m *Motor) Clear() {
	C.motorClear(m.m)
}

func (m *Motor) Enable() {
	C.motorEnable(m.m)
}

func (m *Motor) Disable() {
	C.motorDisable(m.m)
}

func (m *Motor) IsReady() bool {
	return bool(C.motorReady(m.m))
}

func (m *Motor) Position() float64 {
	return float64(C.motorPosition(m.m))
}

func NewMotor(m C.Motor_t) *Motor {
	return &Motor{
		m: m,
		opts: C.MoveOptions_t{
			AccLimit: 100000,
			VelLimit: 60,
			TrqLimit: 10,
		},
	}
}
