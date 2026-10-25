import re, subprocess, os
d="C:/Users/phamn/Documents/PlatformIO/quickshifter-esp32c3/scratch/kicad_smoke"
src=open(os.path.join(d,"min.kicad_sch"),encoding="utf-8").read()
variants = {
 "v1_strip_posfiles": src.replace("(in_pos_files yes)\n","").replace("(duplicate_pin_numbers_are_jumpers no)\n",""),
 "v2_strip_inpos_only": src.replace("(in_pos_files yes)\n",""),
 "v3_strip_dup_only": src.replace("(duplicate_pin_numbers_are_jumpers no)\n",""),
}
for name,txt in variants.items():
    p=os.path.join(d,name+".kicad_sch")
    open(p,"w",encoding="utf-8",newline="\n").write(txt)
    r=subprocess.run([r"C:\Program Files\KiCad\10.0\bin\kicad-cli.exe","sch","erc","--output",os.path.join(d,name+".rpt"),p],capture_output=True,text=True)
    print(name, "rc=",r.returncode, "|", (r.stdout+r.stderr).replace("\n"," ")[:150])
