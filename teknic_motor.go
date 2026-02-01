package teknic

/*
#include "go_teknic.h"
*/
import "C"
import "fmt"
import "os"
import "strings"
import "time"

// Motor wraps the CGO motor motor object
type Motor struct {
	m    C.Motor_t
	opts C.MoveOptions_t
}

// goError converts the CGO Error code into a golang error type
func (m *Motor) goError(cerr *C.Error) error {
	defer C.motorFreeError(cerr)

	if cerr == nil {
		return nil
	}

	code := cerr.Code
	if code != 0 {
		msg := C.GoString(cerr.Msg)
		// the message content is multi line
		// akin to:
		// ``
		// Node @ 0 error. Reported by function: virtual size_t sFnd::CPMmotion::MovePosnStart(int32_t, bool, bool, bool, bool).
		//
		// Error: <message could be here>
		// ```
		// we could try to parse the content after Error but imperically i never see anything there, so just grab the first line
		msg,_,_ = strings.Cut(msg, "\n")
		err := fmt.Errorf("%x:%s", code, msg)
		return err
	}
	return nil
}

// SetTorque sets the percent of motor torque to use
func (m *Motor) SetTorque(percent float64) {
	if percent < 0 {
		percent = 100
	} else if percent > 100 {
		percent = 100
	}
	m.opts.TrqLimit = C.double(percent)
}

// SetAcceleration sets the acceleration to use
func (m *Motor) SetAcceleration(rpm float64) {
	m.opts.AccLimit = C.double(rpm)
}

// SetVelocity sets the target velocity to use for the next move
func (m *Motor) SetVelocity(rpm float64) {
	m.opts.VelLimit = C.double(rpm)
}

// Move a given set steps in either direction
func (m *Motor) Move(units int32) error {
	cerr := C.motorMove(m.m, C.int32_t(units), false, m.opts)
	return m.goError(cerr)
}

// MoveTo a specific position
func (m *Motor) MoveTo(position int32) error {
	cerr := C.motorMove(m.m, C.int32_t(position), true, m.opts)
	return m.goError(cerr)
}

// Wait for the motor movement to complete or timeout
func (m *Motor) Wait(timeout time.Duration) error {
	expires := time.Now().Add(timeout)

	for !C.motorMoveIsDone(m.m) {
		if timeout != 0 && time.Now().After(expires) {
			return os.ErrDeadlineExceeded
		}
	}
	return nil
}

// MoveCancel stops the motor movement safely (with deceleration)
func (m *Motor) MoveCancel() error {
	cerr := C.motorMoveCancel(m.m)
	return m.goError(cerr)
}

// Motorhalt immediatelly stops the motor using full torque
func (m *Motor) Halt() error {
	cerr := C.motorHalt(m.m)
	return m.goError(cerr)
}

// Clear any error conditions
func (m *Motor) Clear() error {
	cerr := C.motorClear(m.m)
	return m.goError(cerr)
}

// Enable the motor, this applies power and holds the motor at its current position
func (m *Motor) Enable() error {
	cerr := C.motorEnable(m.m)
	return m.goError(cerr)
}

// Disable the motor, this removes power and hold from the motor and allows it to spinn freely
func (m *Motor) Disable() error {
	cerr := C.motorDisable(m.m)
	return m.goError(cerr)
}

// IsReady indicates the motor is ready for operation
func (m *Motor) IsReady() bool {
	return bool(C.motorReady(m.m))
}

// Position returns the current stepper position
func (m *Motor) Position() float64 {
	return float64(C.motorPosition(m.m))
}

// NewMotor returns a golang wrapper for the CGO motor object
func NewMotor(m C.Motor_t) *Motor {
	return &Motor{
		m: m,
		opts: C.MoveOptions_t{
			AccLimit: 1000,
			VelLimit: 1000,
			TrqLimit: 100,
			JrkLimit: 3,
		},
	}
}
