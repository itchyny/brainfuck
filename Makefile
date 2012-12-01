
combine = $(foreach i, $1, $(addprefix $i, $2))
stripzero = $(patsubst 0%,%,$1)
generate = $(call stripzero, \
           $(call stripzero, \
           $(call stripzero, \
           $(call combine, $1, \
           $(call combine, $1, \
           $(call combine, $1, $1))))))
number_line := $(call generate,0 1 2 3 4 5 6 7 8 9)
length := $(word $(words $(number_line)), $(number_line))
plus = $(strip \
       $(if $(call eq,$1,0),$2, \
       $(if $(call eq,$2,0),$1, \
       $(word $2, \
       $(wordlist $1, $(length), \
       $(wordlist 3, $(length), $(number_line)))))))
backwards := $(call generate, 9 8 7 6 5 4 3 2 1 0)
reverse = $(strip \
          $(foreach n, \
          $(wordlist 1, $(length), $(backwards)), \
          $(word $n, $1)))
minus = $(strip \
        $(if $(call eq,$2,0),$1,\
        $(word $2, \
        $(call reverse, \
        $(wordlist 1, $1, $(number_line))))))
eq = $(filter $1,$2)

insertspace = $(strip \
              $(subst +,+ , \
              $(subst -,- , \
              $(subst >,> , \
              $(subst <,< , \
              $(subst [,[ , \
              $(subst ],] , \
              $(subst .,. ,$1))))))))


## Brainfuck interpreter in Makefile

### source code
input := ++++++++++[>+++++++>++++++++++>+++>+<<<<-]>++.>+.+++++++..+++.>++.<<+++++++++++++++.>.+++.------.--------.>+.>.
ascii = $(if $(call minus,$1,31), \
        $(if $(call eq,$1,32),  ,\
        $(word $(call minus,$1,32),\
        ! " \# $$ % & ' ( ) * + , - .  / 0 1 2 3 4 5 6 7 8 9 : ; < = > ?  @ A B C D E F G H I J K L M N O P Q R S T U V W X Y Z [ \ ] ^ _ ` a b c d e f g h i j k l m n o p q r s t u v w x y z { | } ~)))

i := 1
p := 1
text:=$(call insertspace,$(input))
is = $(findstring $(word $i,$(text)),$1)
forward = $(eval i:=$(call plus,$i,1))
backward = $(eval i:=$(call minus,$i,1))
bfplus = $(eval p$p:=$(if $(p$p),$(call plus,$(p$p),1),1))
bfminus = $(eval p$p:=$(if $(p$p),$(call minus,$(p$p),1),1))
bfopen = $(saveopen)$(if $(call eq,$(p$p),0),$(recoverclose),)
saveopen = $(eval iopen:=$i)
saveclose = $(eval iclose:=$i)
recoveropen = $(eval i:=$(iopen))
recoverclose = $(eval i:=$(iclose))
bfclose = $(saveclose)$(recoveropen)$(backward)
bfright = $(eval p:=$(call plus,$p,1))
bfleft = $(eval p:=$(call minus,$p,1))
bfoutput = $(warning $(call ascii,$(p$p)))$(call ascii,$(p$p))
debug = $(warning $(p1) $(p2) $(p3) $(p4) $(p5))
bf = $(if $(call is,+),$(bfplus),\
     $(if $(call is,-),$(bfminus),\
     $(if $(call is,[),$(bfopen),\
     $(if $(call is,]),$(bfclose),\
     $(if $(call is,>),$(bfright),\
     $(if $(call is,<),$(bfleft),\
     $(if $(call is,.),$(bfoutput))))))))$(forward)


all:
	@echo $(foreach j,$(wordlist 1,500,$(number_line)),$(bf))
