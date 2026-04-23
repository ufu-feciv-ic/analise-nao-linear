#pragma once

enum class EditorTool
{
    Select,

    AddNode,
    MoveNode,
    CopySelection,
    MirrorSelection,
    RemoveNode,

    AddBeam,
    RemoveBeam,
    AddDimension,
    MoveDimension,

    SetSupportNone,
    SetSupportX,
    SetSupportY,
    SetSupportXY,
    SetSupportFixed,

    AddPointLoad,
    RemovePointLoad,
    AddDistributedLoad,
    RemoveDistributedLoad
};
