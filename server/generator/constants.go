package main

import (
	"fmt"
	"os"
	"os/exec"
	"strings"
	"time"
)

func main() {
	commit := strings.TrimSpace(run("git", "rev-parse", "HEAD"))
	repo := normaliseRepoURL(strings.TrimSpace(run("git", "config", "--get", "remote.origin.url")))
	if repo == "" {
		repo = "https://github.com/mickael-kerjean/filestash"
	}

	f, err := os.OpenFile("../common/constants_generated.go", os.O_CREATE|os.O_WRONLY, os.ModePerm)
	if err != nil {
		fmt.Fprintf(os.Stderr, "error: %v\n", err)
		os.Exit(1)
		return
	}
	f.Write([]byte(fmt.Sprintf(`
package common

func init() {
	BUILD_REF = "%s"
	BUILD_REPO = "%s"
	BUILD_DATE = "%s"
}
		`, commit, repo, time.Now().Format("20060102"))))
	f.Close()
}

func run(name string, args ...string) string {
	cmd, b := exec.Command(name, args...), new(strings.Builder)
	cmd.Stdout = b
	cmd.Run()
	return b.String()
}

func normaliseRepoURL(raw string) string {
	raw = strings.TrimSpace(raw)
	if raw == "" {
		return ""
	}
	raw = strings.TrimSuffix(raw, ".git")
	if strings.HasPrefix(raw, "git@github.com:") {
		return "https://github.com/" + strings.TrimPrefix(raw, "git@github.com:")
	}
	if strings.HasPrefix(raw, "https://github.com/") {
		return raw
	}
	return raw
}
