# LuaJIT Bytecode Dump format
This repository contains description of LuaJIT compiler bytecode dump format(`.luac` files)

## Bytecode Dump Structure  

### Some description  
`{...} - Variable, describes bellow or above`  
`(...:TYPE) - Part of bytecode with type TYPE(types written in win-style i.e. BYTE, BOOL, ULEB128 and etc.)`  
`[if (cond) then (...)] - If cond, then path (...) will be exist in bytecode`  
`\xHH - HEX value`  

### Remark 1
> Type ULEB128_33 means than usefull data is in 32 top bits, and lowest bit is a mark of something  
> Lowest bit marks in structure as `..._LOWEST_BIT`, where ... - name of bytecode part with type ULEB128_33


### Structure
```
{magic}{header}{proto[]}\x00
header=(version:BYTE)(flags:ULEB128)[if !(flags & BCDUMP_F_STRIP) then (chunkname_len:ULEB128)(chunkname:BYTE[chunkname_len])]
proto_head=(proto_len:ULEB128)(pflags:BYTE)
   (numparams:BYTE)(framesize:BYTE)
   (sizeuv:BYTE)(sizekgc:ULEB128)(sizekn:ULEB128)(sizebc:ULEB128)
   [if !(flags & BCDUMP_F_STRIP) then (sizedbg:ULEB128)
      [if (sizedbg != 0) then (firstline:ULEB128)(numline:ULEB128)]
   ]
proto={proto_head}(bytecode:INT32[sizebc])(uvdata:INT16[sizeuv]){constants}[if !(flags & BCDUMP_F_STRIP) then (debug:BYTE[sizedbg])]
constants={kgc[sizekgc]}{knum[sizekn]}
kgc=(kgctype:ULEB128){kgcvalue}
knum=(low:ULEB128_33)[if (low_LOWEST_BIT) then (high:ULEB128)]
kgcvalue= ... # Depends by type(TYPE)

kgcvalue[if TYPE>=BCDUMP_KGC_STR]=(kgcbuffer:BYTE[TYPE-BCDUMP_KGC_STR])
kgcvalue[if TYPE==BCDUMP_KGC_TAB]={ktab}
kgcvalue[if TYPE==BCDUMP_KGC_COMPLEX]=(low:ULEB128)(high:ULEB128)(ilow:ULEB128)(ihigh:ULEB128)
kgcvalue[if TYPE==BCDUMP_KGC_CHILD]=NONE # idk how it works, so just ignore this
kgcvalue[else]=(low:ULEB128)(high:ULEB128)

ktab=(karray_len:ULEB128)(khash_len:ULEB128){karray[karray_len]}{khash[khash_len]}
karray={ktabk} # Value
khash={ktabk}{ktabk} # Key, Value
ktabk=(ktabktype:ULEB128){ktabkvalue}

ktabkvalue= ... # Depends by type(TYPE)
ktabkvalue[if TYPE>=BCDUMP_KTAB_STR]=(ktabkbuffer:BYTE[TYPE-BCDUMP_KTAB_STR])
ktabkvalue[if TYPE==BCDUMP_KTAB_INT]=(intv:ULEB128)
ktabkvalue[if TYPE==BCDUMP_KTAB_NUM]=(low:ULEB128)(high:ULEB128)
ktabkvalue[else]=NONE # Internal constant, check description

magic=\x1B\x4C\x4A
```

### Description of bytecode parts
`version` : `BYTE`  
Version of bytecode dump. Last version in moment of writing = `1`  

-----------------------------------------------------------------

`flags` : `ULEB128`  
Flags of bytecode  
Acceptable flags:  
   - `BCDUMP_F_BE(=1)` - Describes, is bytecode in big-endian format  
   - `BCDUMP_F_STRIP(=2)` - Disable debug-info  
   - `BCDUMP_F_FFI(=4)` - Has FFI

-----------------------------------------------------------------

`chunkname_len` : `ULEB128`  
Length of `chunkname`  
  
**!!! Exists only if `BCDUMP_F_STRIP` flag not defined**

-----------------------------------------------------------------

`chunkname` : `BYTE[chunkname_len]`  
Name of the chunk this proto was defined in
  
**!!! Uses only for debug and tracebacks**  
**!!! Exists only if `BCDUMP_F_STRIP` flag not defined**

-----------------------------------------------------------------

`proto_len` : `ULEB128`  
Length of proto

-----------------------------------------------------------------

`pflags` : `BYTE`  
Flags of proto  
Acceptable flags:  
   - `PROTO_CHILD(=1)` - Has child prototypes
   - `PROTO_VARARG(=2)` - Vararg function
   - `PROTO_FFI(=4)` - Uses BC_KCDATA for FFI datatypes

-----------------------------------------------------------------

`numparams` : `BYTE`  
Number of proto arguments

-----------------------------------------------------------------

`framesize` : `BYTE`  
Fixed frame size of proto(uses for opcodes `FUNCF` and `FUNCV`)

-----------------------------------------------------------------

`sizeuv` : `BYTE`  
Length of `uvdata`

-----------------------------------------------------------------

`sizekgc` : `ULEB128`  
Length of `kgc`

-----------------------------------------------------------------

`sizekn` : `ULEB128`  
Length of `knum`

-----------------------------------------------------------------

`sizebc` : `ULEB128`  
Length of `bytecode`

-----------------------------------------------------------------

`sizedbg` : `ULEB128`  
Length of `debug`  
  
**!!! Exists only if `BCDUMP_F_STRIP` flag not defined**

-----------------------------------------------------------------

`firstline` : `ULEB128`  
First line of the function definition  
  
**!!! Uses only for debug and tracebacks**  
**!!! Exists only if `BCDUMP_F_STRIP` flag not defined and `sizedbg` != 0**

-----------------------------------------------------------------

`numline` : `ULEB128`  
Number of lines for the function definition  
  
**!!! Uses only for debug and tracebacks**  
**!!! Exists only if `BCDUMP_F_STRIP` flag not defined and `sizedbg` != 0**

-----------------------------------------------------------------

`bytecode` : `INT32[sizebc]`  
Bytecode instructions

-----------------------------------------------------------------

`uvdata` : `INT16[sizeuv]`  
Upvalue list. local slot|0x8000 or parent uv idx

-----------------------------------------------------------------

`kgctype` : `ULEB128`  
Type of garbace colletable constant  
Acceptable types:
   - `BCDUMP_KGC_CHILD(=0)` - Proto child
   - `BCDUMP_KGC_TAB(=1)` - Table value
   - `BCDUMP_KGC_I64(=2)` - Int64 value
   - `BCDUMP_KGC_U64(=3)` - Unsigned Int64 value
   - `BCDUMP_KGC_COMPLEX(=4)` - Complex(128bit) value
   - `BCDUMP_KGC_STR(=5)` - String(string length = TYPE - BCDUMP_KGC_STR)

-----------------------------------------------------------------

`kgcbuffer` : `BYTE[TYPE-BCDUMP_KGC_STR]`  
String constant

-----------------------------------------------------------------

`[i]low` : `ULEB128`  
Low 32 bit of data

-----------------------------------------------------------------

`[i]high` : `ULEB128`  
High 32 bit of data  

-----------------------------------------------------------------

`karray_len` : `ULEB128`  
Length of `karray`

-----------------------------------------------------------------

`khash_len` : `ULEB128`
Length of `khash`

-----------------------------------------------------------------

`karray`  
Array part of table(number indices)

-----------------------------------------------------------------

`khash`  
Hashable part of rable(hashable indices, i.e. strings)

-----------------------------------------------------------------

`ktabktype` : `ULEB128`  
Type of constant table value(or key)  
Acceptable types:
   - `BCDUMP_KTAB_NIL(=0)` - `nil`, internal constant
   - `BCDUMP_KTAB_FALSE(=1)` - `false`, internal constant
   - `BCDUMP_KTAB_TRUE(=2)`- `true`, internal constant
   - `BCDUMP_KTAB_INT(=3)` - Int32 value
   - `BCDUMP_KTAB_NUM(=4)` - Int64 value
   - `BCDUMP_KTAB_STR(=5)` - String(string length = TYPE - BCDUMP_KTAB_STR)

-----------------------------------------------------------------

`ktabkbuffer` : `BYTE[TYPE-BCDUMP_KTAB_STR]`  
Constant table value(or key) string

-----------------------------------------------------------------

`intv` : `ULEB128`  
Int32 value

-----------------------------------------------------------------

`ktabkvalue[else]`  
Equal internal const(i.e. `nil`, `false`, or `true`)  
See `ktabktype`

-----------------------------------------------------------------