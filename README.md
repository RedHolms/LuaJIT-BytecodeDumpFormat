# LuaJIT Bytecode Dump format
This repository contains description of LuaJIT compiler bytecode dump format(`.luac` files)

## Bytecode Dump Structure  
Some description:  
`{...} - Variable, describes bellow or above`  
`(...:TYPE) - Part of bytecode with type TYPE(types written in win-style i.e. BYTE, BOOL, ULEB128 and etc.)`  
`[if (cond) then (...)] - If cond, then path (...) will be exist in bytecode`  
`\xHH - HEX value`  

Remark 1
> Type ULEB128_33 means than usefull data is in 32 top bits, and lowest bit is a mark of something  
> Lowest bit marks in structure as `..._LOWEST_BIT`, where ... - name of bytecode part with type ULEB128_33

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

kgcvalue[if TYPE>=BCDUMP_KGC_STR]=(buffer:BYTE[TYPE-BCDUMP_KGC_STR])
kgcvalue[if TYPE==BCDUMP_KGC_TAB]={ktab}
kgcvalue[if TYPE==BCDUMP_KGC_COMPLEX]=(low:ULEB128)(high:ULEB128)(ilow:ULEB128)(ihigh:ULEB128)
kgcvalue[if TYPE!=BCDUMP_KGC_CHILD]=(low:ULEB128)(high:ULEB128)

ktab=(karray_len:ULEB128)(khash_len:ULEB128){karray[karray_len]}{khash[khash_len]}
karray={ktabk} # Value
khash={ktabk}{ktabk} # Key, Value
ktabk=(ktabktype:ULEB128){ktabkvalue}

ktabkvalue= ... # Depends by type(TYPE)
ktabkvalue[if TYPE>=BCDUMP_KTAB_STR]=(buffer:BYTE[TYPE-BCDUMP_KTAB_STR])
ktabkvalue[if TYPE==BCDUMP_KTAB_INT]=(intv:ULEB128)
ktabkvalue[if TYPE==BCDUMP_KTAB_NUM]=(low:ULEB128)(high:ULEB128)
ktabkvalue[else]=NONE # Internal constant, check description

magic=\x1B\x4C\x4A
```

`version`  
Version of bytecode dump. Last version in moment of writing = `1`  
-----------------------------------------------------------------
`flags`  
Flags of bytecode.  
Acceptable Flags  
	- `BCDUMP_F_BE(=1)` - Describes, is bytecode in big-endian format  
	- `BCDUMP_F_STRIP(=2)` - Disable debug-info  
	- `BCDUMP_F_FFI(=4)` - Load FFI