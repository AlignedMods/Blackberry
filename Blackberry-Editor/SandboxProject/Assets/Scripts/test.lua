require("Core.Vector3")
require("Core.Log")
require("Core.Entity")
require("Core.Components")
require("Core.Input")

Script = {
    ScriptEntity = Entity.new()
}

function Script:OnAttach(entity)
    self.ScriptEntity = Entity.new(entity.Handle, entity.Scene)

    local transform = self.ScriptEntity:GetTransformComponent()
    print("Transform: ", transform.Position)
    transform.Position.z = 30
end

function Script:OnDetach()
    print("Script being detached!")
end

function Script:OnUpdate(dt)
    if Input.IsKeyPressed(KeyCode.Enter) then
        local rigidBody = self.ScriptEntity:GetRigidBodyComponent()

        rigidBody:SetLinearVelocity(Vector3.new(0, 10, 0))
    end
end

return Script
