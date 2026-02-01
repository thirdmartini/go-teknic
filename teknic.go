package teknic

import (
	"errors"
)

/*
#cgo CPPFLAGS: -I./ -I/usr/local/teknic/inc/inc-pub/
#cgo CFLAGS: -I./ -I/usr/local/teknic/inc/inc-pub/
#cgo LDFLAGS: -L/usr/local/teknic/lib -lsFoundation20 -Wl,-rpath=/usr/local/teknic/lib
#include <stdlib.h>
#include "go_teknic.h"
*/
import "C"

// Manager represents the tecnic  motor manager
type Manager struct {
	mgr C.Manager_t
}

func (m *Manager) MotorCount() uint {
	return uint(C.mgrGetMotorCount(m.mgr))
}

// Motor returns the motor by order
func (m *Manager) Motor(i uint) (*Motor, error) {
	motor := C.mgrGetMotor(m.mgr, C.uint(i))
	if motor == nil {
		return nil, errors.New("failed to get motor")
	}

	return NewMotor(motor), nil
}

// Close the motor controller, this will disabble all attached motors
func (m *Manager) Close() error {
	C.mgrClose(m.mgr)
	m.mgr = nil
	return nil
}

// NewManager returns an instance of the motor controller manager
func NewManager() (*Manager, error) {
	m := Manager{}

	m.mgr = C.NewManager()
	if m.mgr == nil {
		return nil, errors.New("failed to create manager")
	}

	return &m, nil
}
