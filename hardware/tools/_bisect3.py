import os, subprocess, sys
sys.path.insert(0,".")
from kicadgen import sexp_parse, sexp_dump, head
d="C:/Users/phamn/Documents/PlatformIO/quickshifter-esp32c3/scratch/kicad_smoke"
tree=sexp_parse(open(os.path.join(d,"min.kicad_sch"),encoding="utf-8").read())[0]

def run(name, t):
    p=os.path.join(d,name+".kicad_sch")
    open(p,"w",encoding="utf-8",newline="\n").write(sexp_dump(t)+"\n")
    r=subprocess.run([r"C:\Program Files\KiCad\10.0\bin\kicad-cli.exe","sch","erc","--output",os.path.join(d,name+".rpt"),p],capture_output=True,text=True)
    ok = "Failed to load" not in (r.stdout+r.stderr)
    print(f"{name:28s} rc={r.returncode} {'LOAD-OK' if ok else 'LOAD-FAIL'}")

def without(t, names):
    return [x for x in t if not (isinstance(x,list) and head(x) in names)]

run("b0_asis", tree)
run("b1_no_title", without(tree, {"title_block"}))
run("b2_no_lib", without(tree, {"lib_symbols"}))
run("b3_no_inst", without(tree, {"symbol"}))
run("b4_no_label", without(tree, {"label"}))
run("b5_no_wire", without(tree, {"wire"}))

# strip embedded_fonts from lib symbols
import copy, json
def strip_tok(node, name):
    out=[]
    for c in node:
        if isinstance(c,list):
            if head(c)==name: continue
            out.append(strip_tok(c,name))
        else: out.append(c)
    return out
t2 = copy.deepcopy(tree)
for n in t2:
    if isinstance(n,list) and head(n)=="lib_symbols":
        for s in n:
            if isinstance(s,list) and head(s)=="symbol":
                s[:] = strip_tok(s,"embedded_fonts")
run("b6_no_embedded_fonts", t2)
