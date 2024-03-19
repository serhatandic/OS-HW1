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

