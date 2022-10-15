use std::io::{Read, Write};

const NAME: &str = "bf";

fn main() {
    if let Err(err) = run(std::env::args()) {
        eprintln!("{}: {}", NAME, err);
        std::process::exit(1)
    }
}

fn run(mut args: std::env::Args) -> Result<(), String> {
    let mut source = String::new();
    if let Some(file) = args.nth(1) {
        source = std::fs::read_to_string(file).map_err(|e| e.to_string())?;
    } else {
        std::io::stdin()
            .read_to_string(&mut source)
            .map_err(|e| e.to_string())?;
    }
    execute(parse(source)?)
}

const INCR: i64 = 0; // + -
const NEXT: i64 = 1; // > <
const PUTC: i64 = 2; // .
const GETC: i64 = 3; // ,
const JMPZ: i64 = 4; // [
const JMPNZ: i64 = 5; // ]
const SETZ: i64 = 6; // [-]
const MOVE: i64 = 7; // [->+<]
const OFFSET: i64 = 3;
const MASK: i64 = (1 << OFFSET) - 1;

fn parse(source: String) -> Result<Vec<i64>, String> {
    let mut codes: Vec<i64> = Vec::new();
    let mut jmps: Vec<usize> = Vec::new();
    for (i, c) in source.chars().enumerate() {
        match c {
            '+' => add(&mut codes, INCR, 1),
            '-' => add(&mut codes, INCR, -1),
            '>' => add(&mut codes, NEXT, 1),
            '<' => add(&mut codes, NEXT, -1),
            '.' => codes.push(PUTC),
            ',' => codes.push(GETC),
            '[' => {
                jmps.push(codes.len());
                codes.push(JMPZ)
            }
            ']' => {
                if let Some(&j) = jmps.last() {
                    let l = codes.len();
                    if j == l - 2 && codes[l - 1] == INCR | (-1 << OFFSET) {
                        codes[j] = SETZ;
                        codes.resize(j + 1, 0);
                    } else if j == l - 5
                        && codes[l - 4] == INCR | (-1 << OFFSET)
                        && codes[l - 3] & MASK == NEXT
                        && codes[l - 2] == INCR | (1 << OFFSET)
                        && codes[l - 1] & MASK == NEXT
                        && (codes[l - 3] >> OFFSET) + (codes[l - 1] >> OFFSET) == 0
                    {
                        codes[j] = MOVE | (codes[l - 3] & !MASK);
                        codes[j + 1] = SETZ;
                        codes.resize(j + 2, 0);
                    } else if j == l - 7
                        && codes[l - 6] == INCR | (-1 << OFFSET)
                        && codes[l - 5] & MASK == NEXT
                        && codes[l - 4] == INCR | (1 << OFFSET)
                        && codes[l - 3] & MASK == NEXT
                        && codes[l - 2] == INCR | (1 << OFFSET)
                        && codes[l - 1] & MASK == NEXT
                        && (codes[l - 5] >> OFFSET)
                            + (codes[l - 3] >> OFFSET)
                            + (codes[l - 1] >> OFFSET)
                            == 0
                    {
                        codes[j] = MOVE | (codes[l - 5] & !MASK);
                        codes[j + 1] = MOVE | -(codes[l - 1] & !MASK);
                        codes[j + 2] = SETZ;
                        codes.resize(j + 3, 0);
                    } else {
                        codes[j] |= (l as i64) << OFFSET;
                        codes.push(JMPNZ | ((j as i64) << OFFSET));
                    }
                    jmps.pop();
                } else {
                    return Err(format!("unmatched ] at byte {}", i + 1));
                }
            }
            _ => {}
        }
    }
    if !jmps.is_empty() {
        let mut depth: i64 = 0;
        for (i, c) in source.chars().rev().enumerate() {
            match c {
                '[' => {
                    depth -= 1;
                    if depth < 0 {
                        return Err(format!("unmatched [ at byte {}", source.len() - i));
                    }
                }
                ']' => depth += 1,
                _ => {}
            }
        }
    }
    Ok(codes)
}

fn add(codes: &mut Vec<i64>, code: i64, diff: i64) {
    if codes.last().map_or(true, |c| c & MASK != code) {
        codes.push(code | (diff << OFFSET));
    } else if let Some(last) = codes.last_mut() {
        *last += diff << OFFSET;
    }
}

fn execute(codes: Vec<i64>) -> Result<(), String> {
    let mut memory: Vec<u8> = vec![0; 1];
    let mut pointer: usize = 0;
    let mut i = 0;
    while i < codes.len() {
        let c = codes[i];
        match c & MASK {
            INCR => {
                memory[pointer] = memory[pointer].wrapping_add((c >> OFFSET) as u8);
            }
            NEXT => {
                let dest = pointer as i64 + (c >> OFFSET);
                if dest < 0 {
                    return Err("negative address access".to_string());
                }
                pointer = dest as usize;
                if pointer >= memory.len() {
                    memory.resize(pointer + 1, 0);
                }
            }
            PUTC => {
                std::io::stdout()
                    .write(&memory[pointer..pointer + 1])
                    .map_err(|e| e.to_string())?;
            }
            GETC => {
                if let Err(e) = std::io::stdin().read_exact(&mut memory[pointer..pointer + 1]) {
                    if e.kind() == std::io::ErrorKind::UnexpectedEof {
                        return Ok(());
                    }
                    return Err(e.to_string());
                };
            }
            JMPZ => {
                if memory[pointer] == 0 {
                    i = (c >> OFFSET) as usize;
                }
            }
            JMPNZ => {
                if memory[pointer] != 0 {
                    i = (c >> OFFSET) as usize;
                }
            }
            SETZ => {
                memory[pointer] = 0;
            }
            MOVE => {
                let dest = pointer as i64 + (c >> OFFSET);
                if dest < 0 {
                    return Err("negative address access".to_string());
                }
                if dest as usize >= memory.len() {
                    memory.resize(dest as usize + 1, 0);
                }
                memory[dest as usize] = memory[dest as usize].wrapping_add(memory[pointer]);
            }
            _ => {}
        }
        i += 1;
    }
    Ok(())
}
