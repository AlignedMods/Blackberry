local bl = require("Blackberry")

Script = {}

function Script:OnAttach(entity)
end

function Script:OnDetach()

end

function Script:OnUpdate(dt, entity)
    local trans = bl.Entity.GetComponent(entity, "Transform")
    trans.Position.x = trans.Position.x + 200.0 * dt

    bl.Entity.SetComponent(entity, "Transform", trans)
end

return Script