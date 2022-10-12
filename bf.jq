# jq -sRrjf bf.jq hello.bf

try (
  {
    input: ., index: 0, length: length, jumps: [],
    depth: 0, memory: [], pointer: 0, output: [],
  } |
  until(
    .index >= .length;
    .input[.index:.index+1] as $c |
    .index += 1 |
    if $c == ">" then
      .pointer += 1
    elif $c == "<" then
      .pointer -= 1 |
      if .pointer < 0 then
        error("negative address access")
      end
    elif $c == "+" then
      .memory[.pointer] |= (. + 1) % 256
    elif $c == "-" then
      .memory[.pointer] |= (. + 255) % 256
    elif $c == "." then
      .output += [.memory[.pointer] // 0]
    elif $c == "," then
      error(", is not implemented")
    elif $c == "[" then
      .jumps[.depth] = .index |
      .depth += 1 |
      if .memory[.pointer] <= 0 then
        .saved_index = .index |
        .saved_depth = .depth |
        until(
          .saved_depth > .depth or .index >= .length;
          .input[.index:.index+1] as $c |
          .index += 1 |
          if $c == "[" then
            .depth += 1
          elif $c == "]" then
            .depth -= 1
          else
            .
          end
        ) |
        .jumps = .jumps[:.depth]
      end
    elif $c == "]" then
      .depth -= 1 |
      if .jumps[.depth] | not then
        error("unmatched ] at byte \(.index)")
      elif .memory[.pointer] > 0 then
        .index = .jumps[.depth] |
        .depth += 1
      else
        .jumps = .jumps[:.depth]
      end
    else
      .
    end
  ) |
  if .jumps != [] then
    error("unmatched [ at byte \(.jumps[-1])")
  end |
  .output | implode
) catch ("bf: \(.)\n" | halt_error(1))
