print("Checking is LUAJIT is available")
if jit ~= nil then
    print("LUAJIT is available")
    print("LUAJIT version " .. jit.version)
    print("LUAJIT engine status is " .. tostring(jit.status()))
end

package.path="../Resources/?.lua;./?.lua"
local profiler = require("profiler")

local function sleep(s)
    if type(s) ~= "number" then
        error("Unable to wait if parameter 'seconds' isn't a number: " .. type(s))
    end
    -- http://lua-users.org/wiki/SleepFunction
    local ntime = os.clock() + s/10
    repeat until os.clock() > ntime
end

local function doSomething(n)
    print(tostring(n))
end

profiler.start("l10s")
print("test from the LuaJIT profiler")
for i = 1, 10 do
    doSomething(i)
    sleep(0.200)
end
profiler.stop()