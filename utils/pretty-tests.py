import json
import sys

# some JSON:
f=open(sys.argv[1])

# parse x:
y = json.loads(f.read())

print("Tests results: " + str(y["result"])) 

print("Tests duration: " + str(y["duration"])) 

print("Tests output:\n" + str(y["stdout"])) 
