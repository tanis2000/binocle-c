print("Checking is LUAJIT is available")
if jit ~= nil then
    print("LUAJIT is available")
    print("LUAJIT version " .. jit.version)
    print("LUAJIT engine status is " .. tostring(jit.status()))
end

address = '255.255.255.255'
port = 67
