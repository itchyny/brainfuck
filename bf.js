const fs = require('fs');

const name = 'bf';

function main(args) {
    try {
        execute(parse(fs.readFileSync(args.length > 0 ? args[0] : 0, 'utf-8')));
    } catch (err) {
        console.log(`${name}: ${err.message}`);
        process.exit(1);
    }
}

const bf_incr = 0; // + -
const bf_next = 1; // > <
const bf_putc = 2; // .
const bf_getc = 3; // ,
const bf_jmpz = 4; // [
const bf_jmpnz = 5; // ]
const bf_setz = 6; // [-]
const bf_mult = 7; // [->+>++<<]
const offset = 3;
const mask = (1 << offset) - 1;
const offset_mult = offset + 8;

function parse(source) {
    const codes = [], jmps = [];
    for (let i = 0; i < source.length; i++) {
        switch (source[i]) {
        case '+':
            add(codes, bf_incr, 1);
            break;
        case '-':
            add(codes, bf_incr, -1);
            break;
        case '>':
            add(codes, bf_next, 1);
            break;
        case '<':
            add(codes, bf_next, -1);
            break;
        case '.':
            codes.push(bf_putc);
            break;
        case ',':
            codes.push(bf_getc);
            break;
        case '[':
            jmps.push(codes.length);
            codes.push(bf_jmpz);
            break;
        case ']':
            if (jmps.length === 0) {
                throw new Error(`unmatched ] at byte ${i + 1}`);
            }
            let j = jmps.pop();
            const l = codes.length;
            if (j + 2 === l && codes[j + 1] === (bf_incr | (-1 << offset))) {
                codes[j] = bf_setz;
                codes.splice(-1);
            } else if (j + 2 < l && (l - j) % 2 === 1 && isMults(codes, j)) {
                for (let k = j + 2, p = 0; k < l - 1; j += 1, k += 2) {
                    codes[j] = bf_mult |
                        (p += codes[k] >> offset) << offset_mult |
                        mod(codes[k + 1] >> offset, 256) << offset;
                }
                codes[j] = bf_setz;
                codes.splice(j + 1);
            } else {
                codes[j] |= l << offset;
                codes.push(bf_jmpnz | (j << offset));
            }
            break;
        }
    }
    if (jmps.length > 0) {
        for (let i = source.length - 1, depth = 0; i >= 0; i--) {
            switch (source[i]) {
            case '[':
                if (--depth < 0) {
                    throw new Error(`unmatched [ at byte ${i + 1}`);
                }
                break;
            case ']':
                depth++;
                break;
            }
        }
    }
    return codes;
}

function add(codes, code, diff) {
    if (codes.length === 0 || (codes.at(-1) & mask) !== code) {
        codes.push(code | (diff << offset));
    } else {
        codes.push(codes.pop() + (diff << offset));
    }
}

function mod(n, m) {
    return ((n % m) + m) % m;
}

function isMults(codes, start) {
    let p = 0;
    for (let i = 0, k = start + 1; k < codes.length; i += 1, k += 2) {
        if (
            (codes[k] & mask) === bf_incr &&
            (i > 0 || codes[k] >> offset === -1) &&
            (codes[k + 1] & mask) === bf_next
        ) {
            p += codes[k + 1] >> offset;
        } else {
            return false;
        }
    }
    return p === 0;
}

function execute(codes) {
    let memory = new Uint8Array(1), pointer = 0;
    for (let i = 0; i < codes.length; i++) {
        const c = codes[i];
        switch (c & mask) {
        case bf_incr:
            memory[pointer] += c >> offset;
            break;
        case bf_next:
            pointer += c >> offset;
            if (pointer < 0) {
                throw new Error('negative address access');
            }
            if (pointer >= memory.length) {
                memory = resize(memory, pointer * 2);
            }
            break;
        case bf_putc:
            process.stdout.write(memory.slice(pointer, pointer + 1));
            break;
        case bf_getc:
            if (fs.readSync(0, memory, pointer, 1) === 0) {
                memory[pointer] = -1;
            }
            break;
        case bf_jmpz:
            if (memory[pointer] === 0) {
                i = c >> offset;
            }
            break;
        case bf_jmpnz:
            if (memory[pointer] !== 0) {
                i = c >> offset;
            }
            break;
        case bf_setz:
            memory[pointer] = 0;
            break;
        case bf_mult:
            const v = memory[pointer];
            if (v !== 0) {
                const dest = pointer + (c >> offset_mult);
                if (dest < 0) {
                    throw new Error('negative address access');
                }
                if (dest >= memory.length) {
                    memory = resize(memory, dest * 2);
                }
                memory[dest] += v * ((c >> offset) & 255);
            }
            break;
        }
    }
}

function resize(xs, size) {
    const ys = new Uint8Array(size);
    ys.set(xs);
    return ys;
}

main(process.argv.slice(2));
