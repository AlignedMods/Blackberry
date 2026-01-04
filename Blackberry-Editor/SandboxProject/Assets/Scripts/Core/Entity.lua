if Entity then return end

Entity = {
    Handle = 0, -- Entity handle (UUID)
    Scene = 0   -- Scene handle (Scene*)
}
Entity.__index = Entity

function Entity.new(handle, scene)
    local entity = {}
    setmetatable(entity, Entity)

    entity.Handle = handle
    entity.Scene = scene

    return entity
end

return Entity
