require("Core.Vector3")
require("Core.Log")
require("Core.Entity")

Script = {
    ScriptEntity = Entity.new(),
    ScaleOffset = 1.0
}

function Script:OnAttach(entity)
    self.ScriptEntity = Entity.new(entity.Handle, entity.Scene)
end

function Script:OnDetach()
    print("Script being detached!")
end

function Script:OnUpdate(dt, entity)
    self.ScaleOffset = self.ScaleOffset + dt

    local transform = self.ScriptEntity:GetTransformComponent()
    transform.Scale.x = self.ScaleOffset
    transform.Scale.y = self.ScaleOffset
    transform.Scale.z = self.ScaleOffset

    self.ScriptEntity:SetTransformComponent(transform)
end

return Script
