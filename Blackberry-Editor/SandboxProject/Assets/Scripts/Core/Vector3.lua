if Vector3 then return end -- Avoid including the same module twice

Vector3 = {
    x = 0,
    y = 0,
    z = 0
}
Vector3.__index = Vector3

function Vector3.new(x, y, z)
    local vec = {}
    setmetatable(vec, Vector3)

    vec.x = x
    vec.y = y
    vec.z = z

    return vec
end

return Vector3
