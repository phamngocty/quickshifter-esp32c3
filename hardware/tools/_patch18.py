p="build_lib.py"; s=open(p,encoding="utf-8").read()
s=s.replace('''    fp.append(["fp_text", "user", '"G  C  E"', ["at", "0", "3.6", "0"], ["layer", '"F.SilkS"'],''',
            '''    fp.append(["fp_text", "user", '"G  C  E"', ["at", "0", "3.6", "0"], ["layer", '"F.Fab"'],''')
open(p,"w",encoding="utf-8",newline="\n").write(s); print("lib ok")
