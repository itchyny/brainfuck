# cat hello.bf | jq -sRrf bf.jq

def skip_loop:
  .input[.cursor:.cursor+1] as $c |
  .cursor += 1 |
  if $c == "[" then .depth += 1 | skip_loop
  elif $c == "]" then .depth -= 1 | if .saved_depth > .depth then . else skip_loop end
  elif $c == "" then error("unmatching loop")
  else skip_loop
  end;

def backward_loop:
  .input[.cursor:.cursor+1] as $c |
  .cursor -= 1 |
  if $c == "[" then .depth -= 1 | if .saved_depth >= .depth then .cursor += 1 else backward_loop end
  elif $c == "]" then .depth += 1 | backward_loop
  elif .cursor < 0 then error("unmatching loop")
  else backward_loop
  end;

{ input: ., cursor: 0, memory: [], pointer: 0, depth: 0, output: [] } |
  until(
    .cursor >= (.input | length);
    .input[.cursor:.cursor+1] as $c |
    .cursor += 1 |
    if $c == ">" then .pointer += 1
    elif $c == "<" then .pointer -= 1 | if .pointer < 0 then error("negative pointer") else . end
    elif $c == "+" then .memory[.pointer] |= (. + 1) % 256
    elif $c == "-" then .memory[.pointer] |= (. + 255) % 256
    elif $c == "." then .output += [.memory[.pointer]]
    elif $c == "," then error(", is not implemented")
    elif $c == "[" then .depth += 1 | if .memory[.pointer] > 0 then . else .saved_depth = .depth | skip_loop end
    elif $c == "]" then .depth -= 1 | .cursor -= 1 | .saved_depth = .depth | backward_loop
    else .
    end
  ) | .output | implode
