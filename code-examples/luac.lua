function some_func()
   return 0, 543, 519, math.pi
end

function main()
   while not isSampAvailable() do wait(10) end
   sampRegisterChatCommand('test', function()
      sampAddChatMessage('Hello, World!', -1)
   end)
   foo(some_func())
   wait(-1)
end

function foo(...)
   print("foo", ...)
end