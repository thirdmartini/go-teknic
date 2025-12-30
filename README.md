# go-teknic
A CGO wrapper for teknic stepper motor api ( https://teknic.com/products/clearpath-brushless-dc-servo-motors/clearpath-sc/ )

This golang library provides a very rudimentary wrapper for the sFoundation C++ API provided by teknic. ( You can download the library from their website ).

# Using

Download sFoundation from teknic ( https://teknic.com/downloads/ ) and build the libraries for your platform.  I recommend running one of their example C++ programs
( Like ExampleMotion ) to verify that your motors and controllers are functioning properly.

This wrapper expects that the sFoundation20.so is then installed into ```/usr/local/teknic/lib``` and the header files into ```/usr/local/teknic/inc```

# Example
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
	motor.Wait()
	// Move counterclockwise 1000 steps
	motor.Move(-1000)
	// Wait for move to complete
	motor.Wait()
  
} 

```


