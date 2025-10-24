local Script = {}

function Script:OnAttach()
    print("Script 2 attached!")
end

function Script:OnDetach()
    print("Script detached!")
end

function Script:OnUpdate()
    -- print("Script 2 updating")
end

return Script