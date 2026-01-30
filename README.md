# go-teknic
A CGO wrapper for teknic stepper motor api ( https://teknic.com/products/clearpath-brushless-dc-servo-motors/clearpath-sc/ )

This golang library provides a very rudimentary wrapper for the sFoundation C++ API provided by teknic. ( You can download the library from their website ).

# Before You Start
1. This has only been tested on Linux
2. Make sure you use the [ClearView UI](https://www.youtube.com/watch?v=u9h4_IizovY) to calibrate your motors for the load on them.   

# Requirements 

You need the latest version of sFoundation Libraries for your platform from Teknic's [Downloads Page](https://teknic.com/downloads/)

The libraries expect them to be placed into `/usr/local/teknic/`

IE:
```
/*
#cgo CPPFLAGS: -I./ -I/usr/local/teknic/inc/inc-pub/
#cgo CFLAGS: -I./ -I/usr/local/teknic/inc/inc-pub/
#cgo LDFLAGS: -L/usr/local/teknic/lib -lsFoundation20 -Wl,-rpath=/usr/local/teknic/lib
*/
```

*Note: You can change this in `teknic.go`*

# Example:
```
import "github.com/thirdmartini/go-teknic"

func main() {
	mgr, err := teknic.NewManager()
	if err != nil {
		panic(err)
	}
	defer mgr.Close()

    // fetch the first motor available
	motor, err := mgr.Motor(0)
	if err != nil {
		panic(err)
	}

    // set motor torque to 100%
    motor.SetTorque(100)
	// Set velocity to 1000RPM
	motor.SetVelocity(1000)
	// Begin Move 1000 steps clockwise
	motor.Move(1000)
	// Wait for move to complete
	motor.Wait(time.Second*15)
	// Move counterclockwise 1000 steps
	motor.Move(-1000)
	// Wait for move to complete
	motor.Wait(time.Second*15)
} 
```

# Legal

```
Copyright (c) 2026 Sebastian Sobolewski

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
```