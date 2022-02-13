# LuaJIT-BytecodeDumpFormat
 Description of LuaJIT compiler bytecode dump format(`.luac` files)

## LuaJIT Bytecode Dump format

Some description:  
`{...} - Variable, describes bellow`  
`(...:TYPE) - Part of bytecode`  
`[if (cond) then (...)] - If cond, then path (...) will be in bytecode`  
`\x.. - HEX value`  

```
{magic}{header}{proto[]}\x00
header=(version:BYTE)(flags:ULEB128)[if !(flags & BCDUMP_F_STRIP) then (chunkname_len:ULEB128)(chunkname:BYTE[])]
proto_head=(proto_len:ULEB128)(pflags:BYTE)
   (numparams:BYTE)(framesize:BYTE)
   (sizeuv:BYTE)(sizekgc:ULEB128)(sizekn:ULEB128)(sizebc:ULEB128)
   [if !(flags & BCDUMP_F_STRIP) then (sizedbg:ULEB128)
      [if (sizedbg != 0) then (firstline:ULEB128)(numline:ULEB128)]
   ]
proto={proto_head}(bc:INT32[])(uv:INT16[]){constants}[if !(flags & BCDUMP_F_STRIP) then (debug:BYTE*)]
constants={kgc[]}{knum[]}
kgc=(kgctype:ULEB128){kgcvalue}
knum=(low:ULEB128_33)[if (low & 0b1) then (high:ULEB128)]
kgcvalue= ... # Depends by type(TYPE)

kgcvalue[if TYPE>=BCDUMP_KGC_STR]=(buffer:BYTE[])
kgcvalue[if TYPE==BCDUMP_KGC_TAB]={ktab}
kgcvalue[if TYPE==BCDUMP_KGC_COMPLEX]=(low:ULEB128)(high:ULEB128)(ilow:ULEB128)(ihigh:ULEB128)
kgcvalue[if TYPE!=BCDUMP_KGC_CHILD]=(low:ULEB128)(high:ULEB128)

ktab=(karray_len:ULEB128)(khash_len:ULEB128){karray[]}{khash[]}
karray={ktabk} # Value
khash={ktabk}{ktabk} # Key, Value
ktabk=(ktabktype:ULEB128){ktabkvalue}

ktabkvalue= ... # Depends by type(TYPE)
ktabkvalue[if TYPE>=BCDUMP_KTAB_STR]=(buffer:BYTE*)
ktabkvalue[if TYPE==BCDUMP_KTAB_INT]=(intv:ULEB128)
ktabkvalue[if TYPE==BCDUMP_KTAB_NUM]=(low:ULEB128)(high:ULEB128)
ktabkvalue[else]=NONE # Internal constant, check description bellow

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