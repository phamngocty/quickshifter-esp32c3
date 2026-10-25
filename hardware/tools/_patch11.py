p="build_lib.py"; s=open(p,encoding="utf-8").read()
s=s.replace('["effects", ["font", ["size", "0.7", "0.7"], ["thickness", "0.12"]]]',
            '["effects", ["font", ["size", "0.8", "0.8"], ["thickness", "0.13"]]]')
open(p,"w",encoding="utf-8",newline="\n").write(s); print("lib ok")
