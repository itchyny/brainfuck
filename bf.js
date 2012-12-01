
var clone = function(arr){
  var ans = new Array();
  for(var i = -1, arrlen = arr.length; ++i <= arrlen; ){
    ans.push(arr[i]);
  }
  return ans;
}

var run = function(string, data){
  var ans = '',
      index = -1,
      pointer = 0,
      life = 10000,
      memorysize = 100,
      memory = new Array(0, 0),
      error = '',
      dataindex = -1,
      memoryarr = new Array(),
      memoryappend = true,
      memoryarrindex = -1,
      stringlen = string.length;
W:  while (++index < stringlen){
    switch(string[index]){
      case '+':
        memoryappend = true;
        if (pointer < memory.length){
          ++memory[pointer];
        } else {
          memory[pointer] = 1;
        }
        break;
      case '-':
        memoryappend = true;
        if (pointer < memory.length){
          --memory[pointer];
        } else {
          memory[pointer] = -1;
        }
        break;
      case '>':
        if (memoryappend){
          memoryarr[++memoryarrindex] = clone(memory);
          memoryappend = false;
        }
        if (++pointer >= memory.length){
          memory[pointer] = 0;
        }
        break;
      case '<':
        if (memoryappend){
          memoryarr[++memoryarrindex] = clone(memory);
          memoryappend = false;
        }
        if (--pointer < 0){
          error = 'Negative address accessed!'
        }
        break;
      case ',':
        memoryappend = true;
        memory[pointer] = data[++dataindex];
        memoryarr[++memoryarrindex] = 'input ' + memory[pointer] + ' ' + String.fromCharCode(memory[pointer]);
        break;
      case '.':
        memoryappend = true;
        if (pointer < memory.length){
          ans += String.fromCharCode(memory[pointer]);
          memoryarr[++memoryarrindex] = 'output ' + memory[pointer] + ' ' + ans[ans.length - 1];
        }
        break;
      case '[':
        memoryappend = true;
        --life;
        if (memory[pointer] == 0){
          var num = 0;
          while (true){
            if (++index >= string.length){
              error = 'Invalid loop!';
              break W;
            }
            if (string[index] == '['){
              ++num;
            } else if (string[index] == ']'){
              if (num == 0){
                break;
              }
              --num;
            }
          }
        }
        break;
      case ']':
        memoryappend = true;
        --life;
        if (memory[pointer] != 0){
          var num = 0;
          while (true){
            if (--index < 0){
              error = 'Invalid loop!';
              break W;
            }
            if (string[index] == ']'){
              ++num;
            } else if (string[index] == '['){
              if (num == 0){
                  break;
              }
              --num;
            }
          }
        }
        break;
      default:
        /* Do nothing if any other chars */
        break;
    }
    if (life == 0){
      error = 'Too many loop!';
      break W;
    };
    if (memorysize < memory.length){
      error = 'Memory size exceeded!';
      break W;
    };
  };
  return {'ans' : ans, 'memory' : memory, 'memoryarr' : memoryarr, 'error' : error};
}


console.log(run('+++++++++[>++++++++>+++++++++++>+++++<<<-]>.>++.+++++++..+++.>-.------------.<++++++++.--------.+++.------.--------.>+.'));


