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
proto=(proto_len:ULEB128)(proto_flags:BYTE)
magic=\x1B\x4C\x4A
```

`version`  
	Version of bytecode dump. Last version in moment of writing = `1`

`flags`  
	Flags of bytecode.  
	Acceptable Flags:  
	- `BCDUMP_F_BE(1)`  
		Describes, is bytecode in big-endian format  
	- `BCDUMP_F_STRIP`  
		Don't use debug-info  
	- `BCDUMP_F_FFI`  
		Load FFI