# OS-HW1

---

echo -e "test\npipeline\nexecution" | tr 'a-z' 'A-Z'

should output ->

TEST
PIPELINE
EXECUTION

---

echo -e "test\npipeline\nexecution" | grep "pipeline" | tr 'a-z' 'A-Z'

should output ->

PIPELINE

---

cat eshell.cpp | grep pipe

sequential:
echo "hello" |grep o ; echo "zab" |grep b ; echo "zab" |grep a ; echo "finally" ; echo "zab" |grep z
echo "Hello"; sleep 5; echo "World"

parallel:
echo "Hello", sleep 5, echo "World"

subshell:
(cat eshell.cpp)
(echo "Hello" ; sleep 2; echo "World"; echo "Hello"; sleep 2; echo "World")
(echo "Hello", sleep 2, echo "Worldd", echo "zink", echo ":'(", echo "World")

(echo "Hello",  echo "Worldd", echo "zink", echo ":'(", echo "World")


(echo "Hello" | grep "o" | grep "o", sleep 2, echo "Worldd" | grep "d", echo "zink" | grep "i", sleep 2, echo "World" | grep "o")


(cat eshell.cpp | grep "free_parsed" | grep "input")
