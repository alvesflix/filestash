package ctrl

import "testing"

func TestShortBuildRef(t *testing.T) {
	tests := []struct {
		name string
		ref  string
		want string
	}{
		{name: "empty", ref: "", want: "unknown"},
		{name: "short", ref: "abc", want: "abc"},
		{name: "seven characters", ref: "1234567", want: "1234567"},
		{name: "full commit", ref: "1234567890abcdef", want: "1234567"},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			if got := shortBuildRef(tt.ref); got != tt.want {
				t.Fatalf("shortBuildRef(%q) = %q, want %q", tt.ref, got, tt.want)
			}
		})
	}
}
