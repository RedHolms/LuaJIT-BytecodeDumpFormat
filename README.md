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
header=(version:BYTE)(flags:ULEB128)[if !(flags & BCDUMP_F_STRIP) then (chunkname_len:ULEB128)(chunkname)]
proto=(proto_len:ULEB128)(pflags:BYTE)(numparams:BYTE)(framesize:BYTE)(sizeuv:BYTE)(sizekgc:ULEB128)(sizekn:ULEB128)(sizebc:ULEB128)[if !(flags & BCDUMP_F_STRIP) then (sizedbg:ULEB128)(firstline:ULEB128)(numline:ULEB128)]
magic=\x1B\x4C\x4A
```

`version`  
	Version of bytecode dump. Last version in moment of writing = `1`

`flags`  
	Flags of bytecode.  
Acceptable Flags  
	- `BCDUMP_F_BE(=1)` - Describes, is bytecode in big-endian format  
	- `BCDUMP_F_STRIP(=2)` - Don't use debug-info  
	- `BCDUMP_F_FFI(=4)` - Load FFI