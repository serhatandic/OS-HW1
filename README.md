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