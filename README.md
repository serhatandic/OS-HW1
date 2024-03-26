# OS-HW1

single command:
echo -e "test\npipeline\nexecution" | tr 'a-z' 'A-Z'
echo -e "test\npipeline\nexecution" | grep "pipeline" | tr 'a-z' 'A-Z'

pipeline:
cat eshell.cpp | grep "pipe"

sequential:
echo "hello" |grep o ; echo "zab" |grep b ; echo "zab" |grep a ; echo "finally" ; echo "zab" |grep z
echo "Hello"; sleep 5; echo "World"

parallel:
echo "Hello", sleep 5, echo "World"
echo "hello" |grep o , echo "zab" |grep b ,echo "zab" |grep a , echo "finally" , echo "zab" |grep z

subshell:
(cat eshell.cpp)
(echo "Hello" ; sleep 2; echo "World"; echo "Hello"; sleep 2; echo "World")
(echo "Hello", sleep 2, echo "Worldd", echo "zink", echo ":'(", echo "World")
(echo "Hello", echo "Worldd", echo "zink", echo ":'(", echo "World")
(echo "hello" |grep o , echo "hello" |grep o , echo "zink" | grep "i", echo ":'(", echo "World")
(echo "hello" |grep o ; echo "hello" |grep o ; echo "zink" | grep "i" ; echo ":'(" ; echo "World")
(echo "Hello" | grep "o" | grep "z", sleep 2, echo "Worldd" | grep "d", echo "zink" | grep "i", sleep 2, echo "World" | grep "o")
(echo "Hello" | grep "z" , sleep 2, echo "Worldd" | grep "d", echo "zink" | grep "i", sleep 2, echo "World" | grep "o")
(cat eshell.cpp | grep "free_parsed" | grep "input")

(cat input.txt | grep "a") | (tr /a-z/ /A-Z/ , tr /a-z/ /A-Z/ )
(cat input.txt | grep "a") | (tr /a-z/ /A-Z/ , grep "p" ) | (grep "l" , grep "h" )

(cat input.txt | grep "a") | (grep "a" ,tr /a-z/ /A-Z/ )

(cat input.txt | grep "a") | (cat input.txt | grep "a") | (grep "k" ,tr /a-z/ /A-Z/ )
(cat input.txt | grep "a") | (cat input.txt | grep "a") | (grep "a" ,tr /a-z/ /A-Z/ )

(cat input.txt | grep "a") | (grep "k" , grep "p" )
(cat input.txt | grep "a") | (grep "a" ; grep "p"; grep "p" ; grep "p" ; grep "a" )
(cat input.txt | grep "a") | (grep "a" , grep "p", grep "p" , grep "p" , grep "a" )

# gpt generated
Given that you have a shell wrapper that supports these specifications, including parallel execution with a comma operator, here are the commands aligned with your requirements:

1. **Single Command: 5 pts**
   - `echo "Hello, World!"`

2. **Single Pipeline: 10 pts**
   - `echo "hello world" | tr '[:lower:]' '[:upper:]' | awk '{print $2, $1}'`

3. **Sequential Execution with commands: 5 pts**
   - `cd ~ ; ls ; pwd`

4. **Sequential Execution with mixed commands and pipelines: 15 pts**
   - `echo "foo bar baz" | tr ' ' '\n' ; echo "next command" | grep 'next' | cut -d' ' -f2 ; echo "done"`

5. **Parallel Execution with commands: 5 pts**
   - `echo "Process A" , echo "Process B" , echo "Process C"`

6. **Parallel Execution with mixed commands and pipelines: 15 pts**
   - `echo "parallel1" | tr '[:lower:]' '[:upper:]' , echo "parallel2" | tr '[:lower:]' '[:upper:]' , echo "parallel3"`

7. **Subshell with a single pipeline or sequential or parallel execution: 5 pts**
   - Pipeline: `(echo "hello world" | tr ' ' '\n' | sort)`
   - Sequential: `(cd /tmp ; touch file.txt ; ls)`
   - Parallel: `(echo "A" , echo "B" , echo "C")`

8. **Subshell with mixed types(sequential-pipeline or parallel pipeline): 10 pts**
   - `(echo "start" | tee start.txt ; cat start.txt | grep 'start' ; rm start.txt) , (echo "parallel1" | tr '[:lower:]' '[:upper:]')`

9. **Subshell pipeline with mixed types(sequential-pipeline or parallel pipeline) without the need for repeaters: 15 pts**
   - `(echo "A B" | tr ' ' '\n' , echo "C") | (echo "D" | tr '[:lower:]' '[:upper:]') | (echo "prepending" ; cat ; echo "appending" | tr '[:lower:]' '[:upper:]')`

10. **Subshell pipeline with mixed types(sequential-pipeline or parallel pipeline) with repeaters necessary: 15 pts**
   - `(echo "A B" | tr ' ' '\n' , echo "C") | (echo "D E" , echo "F") | (echo "G" , echo "H" | tr '[:lower:]' '[:upper:]') | tr '[:lower:]' '[:upper:]'`

