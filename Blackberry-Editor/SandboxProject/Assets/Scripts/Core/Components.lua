if Components then return end

Components = {} -- Just here as a essentially a "header guard"

local Entity = require("Core.Entity")

TransformComponent = {
    Entity = Entity.new(0, 0),

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

RigidBodyComponent = {
    Entity = Entity.new(0, 0),

    Type = "Static",
    Restitution = 0,
    Friction = 0
}
RigidBodyComponent.__index = RigidBodyComponent

function RigidBodyComponent.new()
    local rigidBody = {}
    setmetatable(rigidBody, RigidBodyComponent)

    return rigidBody
end

function RigidBodyComponent:AddImpulse(impulse)
    InternalCalls.Entity.RigidBodyAddImpulse(self.Entity.Handle, self.Entity.Scene, impulse)
end

function RigidBodyComponent:SetLinearVelocity(velocity)
    InternalCalls.Entity.RigidBodySetLinearVelocity(self.Entity.Handle, self.Entity.Scene, velocity)
end

function Entity:GetTransformComponent()
    local transform = TransformComponent.new()

    transform.Entity = self

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

function Entity:GetRigidBodyComponent()
    local rigidBody = RigidBodyComponent.new()

    rigidBody.Entity = self

    rigidBody.Type = InternalCalls.Entity.GetRigidBodyType(self.Handle, self.Scene)
    rigidBody.Restitution = InternalCalls.Entity.GetRigidBodyRestitution(self.Handle, self.Scene)
    rigidBody.Friction = InternalCalls.Entity.GetRigidBodyFriction(self.Handle, self.Scene)

    return rigidBody
end
