if Log then return end

Log = {}

function Log.Trace(...)
    InternalCalls.Log.Trace(...)
end

function Log.Info(fmt, ...)
    InternalCalls.Log.Info(fmt)
end

function Log.Warn(fmt, ...)
    InternalCalls.Log.Warn(fmt)
end

function Log.Error(fmt, ...)
    InternalCalls.Log.Error(fmt)
end

function Log.Critical(fmt, ...)
    InternalCalls.Log.Critical(fmt)
end

return Log
