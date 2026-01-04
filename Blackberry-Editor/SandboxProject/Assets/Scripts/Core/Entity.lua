if Entity then return end

Vector3 = require("Core.Vector3")

TransformComponent = {
    Position = Vector3.new(),
    Rotation = Vector3.new(),
    Scale = Vector3.new(),
}
TransformComponent.__index = TransformComponent

function TransformComponent.new()
    local transform = {}
    setmetatable(transform, TransformComponent)

    return transform
end

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

function Entity:GetTransformComponent()
    local transform = TransformComponent.new()

    print("scene: ", self.Scene)
    transform.Position = InternalCalls.Entity.GetTransformPosition(self.Handle, self.Scene)
    transform.Rotation = InternalCalls.Entity.GetTransformRotation(self.Handle, self.Scene)
    transform.Scale = InternalCalls.Entity.GetTransformScale(self.Handle, self.Scene)

    return transform
end

function Entity:SetTransformComponent(transform)
    InternalCalls.Entity.SetTransformPosition(self.Handle, self.Scene, transform.Position)
    InternalCalls.Entity.SetTransformRotation(self.Handle, self.Scene, transform.Rotation)
    InternalCalls.Entity.SetTransformScale(self.Handle, self.Scene, transform.Scale)
end

return Entity
