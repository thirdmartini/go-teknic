package teknic

import "testing"

func TestNewManager(t *testing.T) {
	mgr, err := NewManager()
	if err != nil {
		t.Fatal(err)
	}
	mgr.Close()
}
