#!/usr/bin/python

import re
import secrets
import string

def random_string(length):
  characters = string.ascii_letters + string.digits
  return "".join(
    secrets.choice(characters) for _ in range(length)
  )

XSP=13
XPC=14
XZR=15


def get_actual_opcode(inst):
    match inst:
        case ['hlt']:
            return 0x00
        case ['add', Xd, Xn, Xm]:
            return (0x20 << 24) + ((int(Xm) & 0xF) << 16) + ((int(Xn) & 0xF) << 4) + (int(Xd) & 0xF)
        case ['sub', Xd, Xn, Xm]:
            return (0x21 << 24) + ((int(Xm) & 0xF) << 16) + ((int(Xn) & 0xF) << 4) + (int(Xd) & 0xF)
        case ['div', Xd, Xn, Xm]:
            return (0x22 << 24) + ((int(Xm) & 0xF) << 16) + ((int(Xn) & 0xF) << 4) + (int(Xd) & 0xF)
        case ['mod', Xd, Xn, Xm]:
            return (0x23 << 24) + ((int(Xm) & 0xF) << 16) + ((int(Xn) & 0xF) << 4) + (int(Xd) & 0xF)
        case ['mul', Xd, Xn, Xm]:
            return (0x24 << 24) + ((int(Xm) & 0xF) << 16) + ((int(Xn) & 0xF) << 4) + (int(Xd) & 0xF)
        case ['and', Xd, Xn, Xm]:
            return (0x25 << 24) + ((int(Xm) & 0xF) << 16) + ((int(Xn) & 0xF) << 4) + (int(Xd) & 0xF)
        case ['orr', Xd, Xn, Xm]:
            return (0x26 << 24) + ((int(Xm) & 0xF) << 16) + ((int(Xn) & 0xF) << 4) + (int(Xd) & 0xF)
        case ['eor', Xd, Xn, Xm]:
            return (0x27 << 24) + ((int(Xm) & 0xF) << 16) + ((int(Xn) & 0xF) << 4) + (int(Xd) & 0xF)
        case ['br', Xd, Xn, Xm]:
            return (0x3A << 24) + ((int(Xm) & 0xF) << 16) + ((int(Xn) & 0xF) << 4) + (int(Xd) & 0xF)
        case ['lsl', Xd, Xn, Sh]:
            return (0x28 << 24) + ((int(Sh) & 0x1F) << 8) + ((int(Xn) & 0xF) << 4) + (int(Xd) & 0xF)
        case ['lsr', Xd, Xn, Sh]:
            return (0x29 << 24) + ((int(Sh) & 0x1F) << 8) + ((int(Xn) & 0xF) << 4) + (int(Xd) & 0xF)
        case ['addi', Xd, Xn, Imm]:
            return (0x40 << 24) + ((int(Imm) & 0xFFFF) << 8) + ((int(Xn) & 0xF) << 4) + (int(Xd) & 0xF)
        case ['subi', Xd, Xn, Imm]:
            return (0x41 << 24) + ((int(Imm) & 0xFFFF) << 8) + ((int(Xn) & 0xF) << 4) + (int(Xd) & 0xF)
        case ['andi', Xd, Xn, Imm]:
            return (0x42 << 24) + ((int(Imm) & 0xFFFF) << 8) + ((int(Xn) & 0xF) << 4) + (int(Xd) & 0xF)
        case ['orri', Xd, Xn, Imm]:
            return (0x43 << 24) + ((int(Imm) & 0xFFFF) << 8) + ((int(Xn) & 0xF) << 4) + (int(Xd) & 0xF)
        case ['eori', Xd, Xn, Imm]:
            return (0x44 << 24) + ((int(Imm) & 0xFFFF) << 8) + ((int(Xn) & 0xF) << 4) + (int(Xd) & 0xF)
        case ['ldur', Xd, Xn, Imm]:
            return (0x45 << 24) + ((int(Imm) & 0xFFFF) << 8) + ((int(Xn) & 0xF) << 4) + (int(Xd) & 0xF)
        case ['ldurb', Xd, Xn, Imm]:
            return (0x46 << 24) + ((int(Imm) & 0xFFFF) << 8) + ((int(Xn) & 0xF) << 4) + (int(Xd) & 0xF)
        case ['stur', Xd, Xn, Imm]:
            return (0x50 << 24) + ((int(Imm) & 0xFFFF) << 8) + ((int(Xn) & 0xF) << 4) + (int(Xd) & 0xF)
        case ['sturb', Xd, Xn, Imm]:
            return (0x51 << 24) + ((int(Imm) & 0xFFFF) << 8) + ((int(Xn) & 0xF) << 4) + (int(Xd) & 0xF)
        case ['b', Imm]:
            return (0x80 << 24) + (int(Imm) & 0xFFFFFF)
        case ['beq', Imm]:
            return (0x81 << 24) + (int(Imm) & 0xFFFFFF)
        case ['bne', Imm]:
            return (0x83 << 24) + (int(Imm) & 0xFFFFFF)
        case ['blt', Imm]:
            return (0x84 << 24) + (int(Imm) & 0xFFFFFF)
        case ['ble', Imm]:
            return (0x85 << 24) + (int(Imm) & 0xFFFFFF)
        case ['bgt', Imm]:
            return (0x86 << 24) + (int(Imm) & 0xFFFFFF)
        case ['bge', Imm]:
            return (0x87 << 24) + (int(Imm) & 0xFFFFFF)
        case ['blo', Imm]:
            return (0x88 << 24) + (int(Imm) & 0xFFFFFF)
        case ['bls', Imm]:
            return (0x89 << 24) + (int(Imm) & 0xFFFFFF)
        case ['bhi', Imm]:
            return (0x8A << 24) + (int(Imm) & 0xFFFFFF)
        case ['bhs', Imm]:
            return (0x8B << 24) + (int(Imm) & 0xFFFFFF)
        case ['movk',  Xd, Imm, 'sh', Sh]:
            return (0xA0 << 24) + ((int(Imm) & 0xFFFF) << 8) + ((int(Sh) & 0b1) << 4) + (int(Xd) & 0xF)
        case ['movk',  Xd, Imm]:
            return (0xA0 << 24) + ((int(Imm) & 0xFFFF) << 8) + (int(Xd) & 0xF)
        case ['movz',  Xd, Imm, 'sh', Sh]:
            return (0xA1 << 24) + ((int(Imm) & 0xFFFF) << 8) + ((int(Sh) & 0b1) << 4) + (int(Xd) & 0xF)
        case ['movz',  Xd, Imm]:
            return (0xA1 << 24) + ((int(Imm) & 0xFFFF) << 8) + (int(Xd) & 0xF)
        case ['ppu']:
            return 0xC0 << 24
        case ['flu']:
            return 0xC1 << 24
        case ['wai', Imm]:
            return (0xC2 << 24) + (int(Imm) & 0xFFFF) 
        case['inp', Xd]:
            return (0xC3 << 24) + (int(Xd) & 0xF) 
        case ['nop']:
            return 0xE0 << 24
        case ['cmp', Xn, Xm]:
            return get_actual_opcode(['sub', XZR, Xn, Xm])
        case ['cmpi', Xn, Imm]:
            return get_actual_opcode(['subi', XZR, Xn, Imm])
        case ['mov', Xd, Xn]:
            return get_actual_opcode(['orr', Xd, XZR, Xn])
        case ['lda', Xd, Imm]:
            return get_actual_opcode(['orri', Xd, XZR, Imm])
        case ['not', Xd, Xn]:
            return get_actual_opcode(['eori', Xd, Xn, '-1'])
rformat_dict={'H': 0, 'R': 1, 'I': 2, 'D': 2, 'B': 4, 'IM': 5, 'ME': 6, 'NOP': 7}
opcode_dict={0: 'hlt', 32: 'add', 33: 'sub', 34: 'div', 35: 'mod', 36: 'mul', 37: 'and', 38: 'orr', 39: 'eor', 40: 'lsl', 41: 'lsr', 58: 'br', 64: 'addi', 65: 'subi', 66: 'andi', 67: 'orri', 68: 'eori', 69: 'ldur', 70: 'ldurb', 80: 'stur', 81: 'sturb', 128: 'b', 129: 'beq', 131: 'bne', 132: 'blt', 133: 'ble', 134: 'bgt', 135: 'bge', 136: 'blo', 137: 'bls', 138: 'bhi', 139: 'bhs', 160: 'movk', 161: 'movz', 192: 'ppu', 193: 'flu', 194: 'wai', 195: 'inp', 224: 'nop'}
format_dict={0: 'H', 1: 'R', 2: 'D', 4: 'B', 5: 'IM', 6: 'ME', 7: 'NOP'}
registers = { "x0" : 0, "x1" : 1, "x2" : 2, "x3" : 3, "x4" : 4, "x5" : 5, "x6" : 6, "x7" : 7, "x8" : 8, "x9" : 9, "x10" : 10, "x11" : 11, "x12" : 12, "x13" : 13, "x14" : 14, "x15" : 15, "xzr" : XZR, "xpc" : XPC, "xsp" : XSP}


class X:
    H = rformat_dict["H"]
    R = rformat_dict["R"]
    I = rformat_dict["I"]
    D = rformat_dict["D"]
    B = rformat_dict["B"]
    IM = rformat_dict["IM"]

loop_count = 0
st = 0

def stackgen(enter, exit, extras=()):
    st = 0
    def stack(T):
        nonlocal st
        m = T.group(1)
        if m in exit:
            st-=1
        if m in extras:
            j = T.group(1) + str(st - 1)
        else:
            j = T.group(1) + str(st)
        if m in enter:
            st+=1
        return j
    return stack

def convert_hex_to_dec(match):
      hex_value = match.group(0)
      decimal_value = int(hex_value, 16)
      return str(decimal_value)

def convert_bin_to_dec(match):
    hex_value = match.group(0)
    decimal_value = int(hex_value, 2)
    return str(decimal_value)
def str_sum(match):
    return str(int(match.group(1)) + int(match.group(2)))
def str_sub(match):
    return str(int(match.group(1)) - int(match.group(2)))

# A = Add
# S = Sub
# M = Mult
# D = Div

WORD = r"[a-z0-9\+\-]+"
NOTWORD = r"[^a-z0-9\+\-]"

def clean(input_string):
      input_string = re.sub(r'/\*[\s\S]*?\*/', '', input_string, flags=re.MULTILINE)
      input_string = re.sub(r'\s*;.*', '\n', input_string)
      input_string = re.sub(r'\s*,\s*', ' ', input_string)
      input_string = re.sub(r'[^\S\r\n]+', ' ', input_string)
      input_string = re.sub(r":\s*\n|:\n", ': ', input_string)
      input_string = re.sub(r"\[|\]", '', input_string)
      input_string = re.sub(r"\n\s*", '\n', input_string)
      input_string = re.sub(r'\n+', '\n', input_string)
      def simp_replace(match):
        return match.group(1)
      input_string = re.sub(r'\s*(\=|\+|-\=|\+\=|\>|\>\=|\<|\<\=|\=\=|\!\=)\s*', simp_replace, input_string)
      return input_string

def precomp(input_string):
    #complicated regexstuff
    pading = False
    input_string = input_string.lower().strip()
    

    input_string = clean(input_string)
    input_string = input_string.strip()
    #preproccesing macros

    if '.padding' in input_string:
      input_string = input_string.replace('.padding\n', '')
      pading = True

    forloopregex = re.compile(
      rf"\.for(\d+) ({WORD})=({WORD}) ({WORD})(\>|\>\=|\<|\<\=|\=\=|\!\=)({WORD}) ({WORD})(\-\=|\+\=)({WORD})\n(.*?)\.endfor\1"
      , flags=re.DOTALL
    )
    
    def printret(string):
        print(string)
        return string

    def forloop(match):
        global loop_count
        reg = match.group(2)
        init_val = match.group(3)
        cmpreg = match.group(4)
        cmptype = match.group(5)
        cmpval = match.group(6)
        if init_val in registers.keys():
            mov = "mov"
        else:
            mov = "movz"
        if cmpval in registers.keys():
            cmp = "cmp"
        else:
            cmp = "cmpi"
        modreg = match.group(7)
        modsign = match.group(8)
        modval = match.group(9)
        looped = forloopregex.sub(forloop, match.group(10))
        branch_type = None
        mod_type = None
        loop_name = f"{random_string(12)}{loop_count}"
        loop_count += 1
        match(cmptype):
            case ">":
                branch_type = "ble"
            case ">=":
                branch_type = "blt"
            case "<":
                branch_type = "bge"
            case "<=":
                branch_type = "bgt"
            case "==":
                branch_type = "bne"
            case "!=":
                branch_type = "beq"
        match(modsign):
            case "+=":
                mod_type = "addi"
            case "-=":
                mod_type = "subi"
        return printret(f"""
        {mov} {reg} {init_val}
        {loop_name}: {cmp} {cmpreg} {cmpval}
        {branch_type} {loop_name}end
        {looped}
        {mod_type} {modreg} {modreg} {modval}
        b {loop_name}
        {loop_name}end:
        """)
    
    forifregex = re.compile(
      fr"\.if(\d+) ({WORD})(\>|\>\=|\<|\<\=|\=\=|\!\=)({WORD})(.*?)\.endif\1"
      , flags=re.DOTALL
    )
    def forif(match):
        global loop_count
        cmpreg = match.group(2)
        cmptype = match.group(3)
        cmpval = match.group(4)
        if cmpval in registers.keys():
            cmp = "cmp"
        else:
            cmp = "cmpi"
        looped = forifregex.sub(forif, match.group(5)).split(f".else{match.group(1)}", 1)
        elsetext = None
        if len(looped) == 1:
            iftext = looped[0]
        else:
            iftext, elsetext = looped
        branch_type = None
        loop_name = f"{random_string(12)}{loop_count}"
        loop_count += 1
        #print("MATCH")
        #print("REG", cmpreg)
        #print("TYPE", cmptype)
        #print("VAL", cmpval)
        #print()
        #print("looped", looped)
        #print()
        match(cmptype):
            case ">":
                branch_type = "ble"
            case ">=":
                branch_type = "blt"
            case "<":
                branch_type = "bge"
            case "<=":
                branch_type = "bgt"
            case "==":
                branch_type = "bne"
            case "!=":
                branch_type = "beq"
        if elsetext is None:
            return f"""
            {cmp} {cmpreg} {cmpval}
            {branch_type} {loop_name}
            {iftext}
            {loop_name}:
            """
        else:
            return f"""
            {cmp} {cmpreg} {cmpval}
            {branch_type} {loop_name}
            {iftext}
            b {loop_name}end
            {loop_name}:
            {elsetext}
            {loop_name}end:
            """
    def replace_aliases(match):
      return alias.get(match.group(0), match.group(0))
    
    alias = {al: da for al, da in re.findall(fr'\s*\.eqv (\w+) ({WORD})\s', input_string)}
    pattern = r'\b(' + '|'.join(map(re.escape, alias.keys())) + r')\b'
    input_string = re.sub(pattern, replace_aliases, input_string)
    input_string = clean(input_string)
    input_string = re.sub(r"\s*\.eqv.*", '', input_string)

    def instruct_split(op, dest, *rest):
        s = f"{op}{dest}{rest[0]}{rest[1]}"
        for v in rest[2:]:
            s += f"\n{op}{dest}{dest}{v}"
        return s
    
    for ops in ("add", "addi", "sub", "subi", "mul", "mod", "div"
        "orr", "eor", "and", "orri", "eori", "andi"):
        func = lambda T: instruct_split(ops, T.group(1), *T.groups()[1:])
        input_string = re.sub(fr"\b(?>{ops})( {WORD})( {WORD})( {WORD})( {WORD})+", func, input_string)

    push_sub = lambda T: f"subi x{XSP} x{XSP} 4\nstur {T.group(1)} x{XSP} 0"
    input_string = re.sub(fr"(?:\b(?>push) ({WORD}))", push_sub, input_string)
    pop_sub = lambda T: f"ldur {T.group(1)} x{XSP} 0\naddi x{XSP} x{XSP} 4"
    input_string = re.sub(fr"(?:\b(?>pop) ({WORD}))", pop_sub, input_string)
    regb_sub = lambda T: f"br 0 0 {T.group(1)}"
    input_string = re.sub(fr"(?:\b(?>br) ({WORD}))", regb_sub, input_string)
    input_string = clean(input_string)
    #print("ACTUAL")
    #print(input_string)

    # finding all non-std numbers and converting to dec for easy proccesing
    input_string = re.sub(r"(\.for|\.endfor)", stackgen((".for"), (".endfor")), input_string)
    input_string = re.sub(r"(\.if|\.else|\.endif)", stackgen((".if"), (".endif"), (".else")), input_string)
    #print("B")
    #print(input_string)
    input_string = forloopregex.sub(forloop, input_string)
    input_string = clean(input_string)
    #print("A")
    #print(input_string)
    input_string = forifregex.sub(forif, input_string)
    input_string = clean(input_string)
    input_string = re.sub(r"(?=\b|\+|\=)0x[0-9a-f]+(?=\b|\+|\=)", convert_hex_to_dec, input_string)
    input_string = re.sub(r"(?=\b|\+|\=)0b[01]+(?=\b|\+|\=)", convert_bin_to_dec, input_string)
    input_string = clean(input_string)

    #replacing register macros with actual values
    for u, v in registers.items():
      input_string = re.sub(fr"\b{u}\b", str(v), input_string)

    #removing non-need data
    data = '\n'.join(re.findall(r'\.data(?:.|\n)+?(?=\.text|$)', input_string)).replace('.data','').strip()
    text = '\n'.join(re.findall(r'\.text(?:.|\n)+?(?=\.data|$)', input_string)).replace('.text','').strip()
    
    if text.isspace():
      print("[WARNING NO TEXT SECION IN CODE]")
    data = clean(data)
    text = clean(text)
    text_list = []
    temp = {}
    for i, line in enumerate(text.split('\n')):
      if ':' in line:
        A = line.split(':')
        print(A)
        for x in A[:-1]:
          temp[x.strip()] = i * 4
        line = A[-1]
        if not line:
          line = 'hlt'
      text_list.append(line.strip())
    t = []
    for i, line in enumerate(text_list):
      for u in temp.keys():
        line = re.sub(fr"\b{u}\b", str(temp[u] - i*4), line)
      t.append(line.strip())
    text = '\n'.join(t)
    text = text.split('\n')
    # enforces hlt at the end of code to avoid executing data

    if text[-1] != 'hlt' :
      text.append('hlt')

    if pading and len(text)%8 != 0:
      text.extend(['hlt']*(8-(len(text)%8)))
    return "\n".join(text), data



def dat_comp(_data):
  _data = _data.replace('\n', ' ').strip()
  raw_data = b''
  labels = {}
  for l, ty, data in re.findall(fr'(\w+):\s*(\.\w+)\s*([-?\d+\s]+)', _data, flags=re.MULTILINE):
    data = data.strip()
    ty = ty.strip()
    if ty == '.word':
      mask = 0xFFFFFFFF
      ty = 4
    elif ty == '.byte':
      mask = 0xFF
      ty = 1
    else:
        continue
    data = b''.join((int(x) & mask).to_bytes(ty, byteorder='big') for x in data.split(' '))
    if len(data) % 4 != 0:
      data += b'\x00' * (4 - len(data) % 4)
    labels[l] = len(raw_data)
    raw_data += data
  return raw_data, labels

def text_comp(text, raw_data, lables):
    lines = text.count('\n') + 1
    # data lables are replaced during actual assembly
    for u, v in lables.items():
        text = re.sub(fr"(?>\b|\+|\=){u}(?>\b|\+|\=)", str(v + 4*(lines)), text)
    print(text)
    text = re.sub(r"(?=\b|\+\-|\=)(\d+)\+(\d+)(?=\b|\+\-|\=)", str_sum, text)
    text = re.sub(r"(?=\b|\+\-|\=)(\d+)\-(\d+)(?=\b|\-|\=)", str_sub, text)
    for i, line in enumerate(text.split('\n')):
        print(line)
        print(i*4, line.split(), f"{get_actual_opcode(line.split()):08X}")
    op = [ get_actual_opcode(line.split()).to_bytes(4, byteorder='big') for line in text.split('\n') ]
    return b''.join(op) + raw_data

def comp(src):
  text, data = precomp(src)
  raw_data, data_labels = dat_comp(data)
  print("DATA LABLES")
  print(data_labels)
  print("TEXT")
  print(text)
  print()
  print("DAT")
  print(data)
  print()
  return bytearray(text_comp(text, raw_data, data_labels))


def twocomp(value):
  if value & 0x800000:
    return -(((~value) & 0xFFFFFF) + 1)
  return value

def registers_translate(regnum):
    global XSP, XPC, XZR
    if XSP == regnum:
        return f"XSP"
    if XPC == regnum:
        return f"XPC"    
    if XZR == regnum:
        return f"XZR"
    return f"X{regnum}"

def extract_data(i, word):
  match (word[0] >> 5):
    case X.H:
      return f"{word[3]}",
    case X.R: # R
      return 'Rm', word[1], '\tSHAMT', word[2], '\tRn', registers_translate(word[3] >> 4), '\tRd', registers_translate(word[3] & 0xF)
    case X.D | X.I: # D I
      return 'Imm', (word[1] << 8) + word[2], '\tRn', registers_translate(word[3] >> 4), '\tRd', registers_translate(word[3] & 0xF)
    case X.B: # B
      return 'branch to', f'0x{i + twocomp((word[1] << 16) + (word[2] << 8) + word[3]):08X}',
    case X.IM: #
      return 'Imm', f"{(word[1] << 8) + word[2]:04X}" , '\tSH', (word[3] >> 4) & 3, '\tRt', registers_translate(word[3] & 0xF)
    case _:
      return (word[0] << 24) + (word[1] << 16) + (word[2] << 8) + word[3],

def dissasembler(i, word):
  if opcode_dict.get(word[0]) is not None:
    l = len(opcode_dict.get(word[0]))
  else:
    l = 0
  return f"\t\t{opcode_dict.get(word[0])}{' '*(5 - l)}\t{format_dict.get(word[0] >> 5)}\t", *extract_data(i, word)


import getopt, sys, os
if __name__ == "__main__":
    opts, _ = getopt.getopt(sys.argv[1:], "adpo:i:")
    mode = 0
    infile = None
    outputfile = 'out'
    for opt, arg in opts:
      print(opt, arg)
      if opt in ['-a']:
        mode = 0
      if opt in ['-d']:
        mode = 1
      if opt in ['-i']:
        infile = arg
      if opt in ['-o']:
        outputfile = arg
      if opt in ['-p']:
          mode = 2
    if infile is None:
      print("no input file")
      exit()
    if not os.path.exists(infile):
       print("file does not exist")
       exit()
    if mode == 0:
        DIR = os.path.dirname(os.path.abspath(infile))
        with open(infile, 'r') as f:
            src = f.read()
        if '.include' in src:
            incfiles = [da for da in re.findall(r'\s*\.include ([A-Za-z0-9_\\\/\.]+)\s', src)]
            src = re.sub(r'\s*\.include ([A-Za-z0-9_\\\/\.]+)\s', '', src)
            for file in incfiles:
                with open(os.path.join(DIR, file), 'r') as f:
                    t = f.read()
                    src = t + src
        data = comp(src)
        with open(outputfile, 'wb') as f:
            f.write(data)
    elif mode == 1:
        with open(infile, 'rb') as f:
            x6 = f.read()
        for i in range(0, len(x6), 4):
            print(f"0x{i:03X}({i:03}):", f"0x{int.from_bytes(x6[i:i+4], byteorder='big'):08x}", *dissasembler(i, x6[i:i+4]))
    elif mode == 2:
        with open(infile, 'r') as f:
            src = f.read()
        with open(outputfile, 'w') as f:
            a, b = precomp(src)
            print(b)
            f.write('.text\n' + a + '\n.data\n' + b + '\n')
    else:
        print("no mode selected")
    exit()

#for b in x6:
#  print(f"{b:02X}", end=' ')
#print()
#for b in data:
#  print(f"{b:02X}", end=' ')
#print()
