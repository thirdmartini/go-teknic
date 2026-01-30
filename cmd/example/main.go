package main

import (
	"fmt"
	"sync"
	"time"

	teknic "github.com/thirdmartini/go-teknic"
)

func main() {
	mgr, err := teknic.NewManager()
	if err != nil {
		panic(err)
	}
	defer mgr.Close()

	rotation, err := mgr.Motor(0)
	if err != nil {
		panic(err)
	}

	for !rotation.IsReady() {
		fmt.Println("Waiting for rotation...")
		time.Sleep(1 * time.Second)
	}

	pitch, err := mgr.Motor(1)
	if err != nil {
		panic(err)
	}

	for !pitch.IsReady() {
		fmt.Println("Waiting for pitch...")
		time.Sleep(1 * time.Second)
	}

	fmt.Printf("Rotation: %v\n", rotation.Position())
	fmt.Printf("Pitch: %v\n", pitch.Position())

	wg := sync.WaitGroup{}
	wg.Add(2)

	pitch.SetVelocity(1000)
	pitch.MoveTo(0)
	pitch.Wait(1 * time.Second)
	rotation.SetVelocity(1000)
	rotation.MoveTo(0)
	rotation.Wait(time.Second*15)

	go func(m *teknic.Motor) {
		m.SetVelocity(1)
		m.MoveTo(100)
		m.Wait(10 * time.Second)
		time.Sleep(1 * time.Second)
		m.SetVelocity(1)
		m.MoveTo(0)
		m.Wait(10 * time.Second)
		m.Halt()
		fmt.Printf("Rotation: Done\n")
		wg.Done()
	}(rotation)

	go func(m *teknic.Motor) {
		m.SetTorque(100)
		m.SetVelocity(60)
		m.MoveTo(-8000)
		m.Wait(10 * time.Second)
		time.Sleep(2 * time.Second)
		m.MoveTo(0)
		m.Wait(10 * time.Second)
		m.Disable()
		fmt.Printf("Pitch: Done\n")
		wg.Done()
	}(pitch)

	q := make(chan bool, 1)
	go func() {
		for {
			select {
			case <-q:
				fmt.Printf("Exiting...")
				return
			case <-time.After(1 * time.Second):
				fmt.Printf("R:%v   | P:%v\n", rotation.Position(), pitch.Position())
			}
		}
	}()

	wg.Wait()

	q <- true
	time.Sleep(1 * time.Second)
}
