package main

import (
	"errors"
	"fmt"
	"io"
	"os"
)

const name = "bf"

func main() {
	if err := run(os.Args[1:]); err != nil {
		fmt.Fprintf(os.Stderr, "%s: %s\n", name, err)
		os.Exit(1)
	}
}

func run(args []string) error {
	var (
		source []byte
		err    error
	)
	if len(args) == 0 {
		source, err = io.ReadAll(os.Stdin)
	} else {
		source, err = os.ReadFile(args[0])
	}
	if err != nil {
		return err
	}
	codes, err := parse(source)
	if err != nil {
		return err
	}
	return execute(codes)
}

const (
	incr   = iota // + -
	next          // > <
	putc          // .
	getc          // ,
	jmpz          // [
	jmpnz         // ]
	setz          // [-]
	move          // [->+<]
	offset = 3
	mask   = 1<<offset - 1
)

func parse(source []byte) ([]int, error) {
	var codes []int
	var jmps []int
	for i := 0; i < len(source); i++ {
		switch source[i] {
		case '+':
			codes = add(codes, incr, 1)
		case '-':
			codes = add(codes, incr, -1)
		case '>':
			codes = add(codes, next, 1)
		case '<':
			codes = add(codes, next, -1)
		case '.':
			codes = append(codes, putc)
		case ',':
			codes = append(codes, getc)
		case '[':
			jmps = append(jmps, len(codes))
			codes = append(codes, jmpz)
		case ']':
			if len(jmps) == 0 {
				return nil, fmt.Errorf("unmatched ] at byte %d", i+1)
			}
			if j, l := jmps[len(jmps)-1], len(codes); j == l-2 &&
				codes[l-1] == incr|-1<<offset {
				codes = append(codes[:j], setz)
			} else if j == l-5 &&
				codes[l-4] == incr|-1<<offset && codes[l-3]&mask == next &&
				codes[l-2] == incr|1<<offset && codes[l-1]&mask == next &&
				codes[l-3]>>offset+codes[l-1]>>offset == 0 {
				codes = append(codes[:j], move|codes[l-3]&^mask)
			} else {
				codes[j] |= l << offset
				codes = append(codes, jmpnz|j<<offset)
			}
			jmps = jmps[:len(jmps)-1]
		}
	}
	if len(jmps) != 0 {
		for i, depth := len(source)-1, 0; i >= 0; i-- {
			switch source[i] {
			case '[':
				if depth--; depth < 0 {
					return nil, fmt.Errorf("unmatched [ at byte %d", i+1)
				}
			case ']':
				depth++
			}
		}
	}
	return codes, nil
}

func add(codes []int, code int, diff int) []int {
	if len(codes) == 0 || codes[len(codes)-1]&mask != code {
		return append(codes, code|diff<<offset)
	}
	codes[len(codes)-1] += diff << offset
	return codes
}

func execute(codes []int) error {
	memory, pointer := []byte{0}, 0
	for i := 0; i < len(codes); i++ {
		switch c := codes[i]; c & mask {
		case incr:
			memory[pointer] += byte(c >> offset)
		case next:
			if pointer += c >> offset; pointer < 0 {
				return errors.New("negative address access")
			}
			for pointer >= len(memory) {
				memory = append(memory, 0)
			}
		case putc:
			if _, err := os.Stdout.Write(memory[pointer : pointer+1]); err != nil {
				return err
			}
		case getc:
			if _, err := os.Stdin.Read(memory[pointer : pointer+1]); err != nil {
				if err == io.EOF {
					return nil
				}
				return err
			}
		case jmpz:
			if memory[pointer] == 0 {
				i = c >> offset
			}
		case jmpnz:
			if memory[pointer] != 0 {
				i = c >> offset
			}
		case setz:
			memory[pointer] = 0
		case move:
			dest := pointer + c>>offset
			if dest < 0 {
				return errors.New("negative address access")
			}
			for dest >= len(memory) {
				memory = append(memory, 0)
			}
			memory[dest] += memory[pointer]
			memory[pointer] = 0
		}
	}
	return nil
}
