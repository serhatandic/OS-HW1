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
