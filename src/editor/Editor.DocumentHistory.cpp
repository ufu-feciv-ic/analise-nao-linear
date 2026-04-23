#include "editor/Editor.h"
#include "editor/ops/PropertySelectionOperations.h"

#include <cstring>
#include <functional>
#include <utility>

namespace
{
    constexpr std::size_t maxDocumentHistoryEntries = 20;

    void HashCombine(std::uint64_t& seed, std::uint64_t value)
    {
        seed ^= value + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2);
    }

    std::uint64_t HashDouble(double value)
    {
        std::uint64_t bits = 0;
        static_assert(sizeof(bits) == sizeof(value));
        std::memcpy(&bits, &value, sizeof(double));
        return bits;
    }
}

std::uint64_t Editor::ComputeDocumentSignature(const ProjectDocument& snapshot) const
{
    std::uint64_t signature = 1469598103934665603ULL;

    HashCombine(signature, std::hash<std::string>{}(snapshot.name));
    HashCombine(signature, static_cast<std::uint64_t>(snapshot.unitSystem));
    HashCombine(signature, static_cast<std::uint64_t>(snapshot.displayUnits.length));
    HashCombine(signature, static_cast<std::uint64_t>(snapshot.displayUnits.force));
    HashCombine(signature, static_cast<std::uint64_t>(snapshot.displayUnits.moment));
    HashCombine(signature, static_cast<std::uint64_t>(snapshot.displayUnits.distributedLoad));
    HashCombine(signature, static_cast<std::uint64_t>(snapshot.displayUnits.stress));
    HashCombine(signature, static_cast<std::uint64_t>(snapshot.displayUnits.elasticModulus));
    HashCombine(signature, static_cast<std::uint64_t>(snapshot.displayUnits.area));
    HashCombine(signature, static_cast<std::uint64_t>(snapshot.displayUnits.inertia));

    HashCombine(signature, static_cast<std::uint64_t>(snapshot.nextNodeId));
    HashCombine(signature, static_cast<std::uint64_t>(snapshot.nextBeamId));
    HashCombine(signature, static_cast<std::uint64_t>(snapshot.nextDistributedLoadId));
    HashCombine(signature, static_cast<std::uint64_t>(snapshot.nextDimensionId));
    HashCombine(signature, static_cast<std::uint64_t>(snapshot.nextMaterialId));
    HashCombine(signature, static_cast<std::uint64_t>(snapshot.nextSectionId));

    HashCombine(signature, static_cast<std::uint64_t>(snapshot.nodes.size()));
    for (const Node& node : snapshot.nodes)
    {
        HashCombine(signature, static_cast<std::uint64_t>(node.id));
        HashCombine(signature, HashDouble(node.position.x));
        HashCombine(signature, HashDouble(node.position.y));
        HashCombine(signature, static_cast<std::uint64_t>(node.support));
        HashCombine(signature, HashDouble(node.load.fx));
        HashCombine(signature, HashDouble(node.load.fy));
        HashCombine(signature, HashDouble(node.load.mz));
    }

    HashCombine(signature, static_cast<std::uint64_t>(snapshot.beams.size()));
    for (const Beam& beam : snapshot.beams)
    {
        HashCombine(signature, static_cast<std::uint64_t>(beam.id));
        HashCombine(signature, static_cast<std::uint64_t>(beam.startNodeId));
        HashCombine(signature, static_cast<std::uint64_t>(beam.endNodeId));
        HashCombine(signature, static_cast<std::uint64_t>(beam.materialId));
        HashCombine(signature, static_cast<std::uint64_t>(beam.sectionId));
    }

    HashCombine(signature, static_cast<std::uint64_t>(snapshot.distributedLoads.size()));
    for (const BeamDistributedLoad& load : snapshot.distributedLoads)
    {
        HashCombine(signature, static_cast<std::uint64_t>(load.id));
        HashCombine(signature, static_cast<std::uint64_t>(load.beamId));
        HashCombine(signature, HashDouble(load.value.qxStart));
        HashCombine(signature, HashDouble(load.value.qyStart));
        HashCombine(signature, HashDouble(load.value.qxEnd));
        HashCombine(signature, HashDouble(load.value.qyEnd));
    }

    HashCombine(signature, static_cast<std::uint64_t>(snapshot.dimensions.size()));
    for (const Dimension& dimension : snapshot.dimensions)
    {
        HashCombine(signature, static_cast<std::uint64_t>(dimension.id));
        HashCombine(signature, static_cast<std::uint64_t>(dimension.startNodeId));
        HashCombine(signature, static_cast<std::uint64_t>(dimension.endNodeId));
        HashCombine(signature, static_cast<std::uint64_t>(dimension.type));
        HashCombine(signature, static_cast<std::uint64_t>(dimension.lengthUnit));
        HashCombine(signature, static_cast<std::uint64_t>(dimension.offsetMode));
        HashCombine(signature, HashDouble(dimension.offsetPixels));
        HashCombine(signature, HashDouble(dimension.offsetWorld));
    }

    HashCombine(signature, static_cast<std::uint64_t>(snapshot.materials.size()));
    for (const StructuralMaterial& material : snapshot.materials)
    {
        HashCombine(signature, static_cast<std::uint64_t>(material.id));
        HashCombine(signature, std::hash<std::string>{}(material.name));
        HashCombine(signature, HashDouble(material.youngModulus));
        HashCombine(signature, HashDouble(material.thermalExpansion));
    }

    HashCombine(signature, static_cast<std::uint64_t>(snapshot.sections.size()));
    for (const Section& section : snapshot.sections)
    {
        HashCombine(signature, static_cast<std::uint64_t>(section.id));
        HashCombine(signature, std::hash<std::string>{}(section.name));
        HashCombine(signature, HashDouble(section.area));
        HashCombine(signature, HashDouble(section.inertia));
    }

    return signature;
}

void Editor::PushUndoSnapshot(const ProjectDocument& snapshot)
{
    undoHistory.push_back(snapshot);
    if (undoHistory.size() > maxDocumentHistoryEntries)
    {
        undoHistory.erase(undoHistory.begin());
    }
}

void Editor::PushUndoSnapshot(ProjectDocument&& snapshot)
{
    undoHistory.push_back(std::move(snapshot));
    if (undoHistory.size() > maxDocumentHistoryEntries)
    {
        undoHistory.erase(undoHistory.begin());
    }
}

void Editor::PushRedoSnapshot(const ProjectDocument& snapshot)
{
    redoHistory.push_back(snapshot);
    if (redoHistory.size() > maxDocumentHistoryEntries)
    {
        redoHistory.erase(redoHistory.begin());
    }
}

void Editor::RecordDocumentChange(const ProjectDocument& beforeDocument)
{
    if (ComputeDocumentSignature(beforeDocument) == ComputeDocumentSignature(document))
    {
        return;
    }

    PushUndoSnapshot(beforeDocument);
    redoHistory.clear();
    MarkDerivedDataDirty();
}

void Editor::RecordDocumentChange(ProjectDocument&& beforeDocument, bool assumeChanged)
{
    if (!assumeChanged &&
        ComputeDocumentSignature(beforeDocument) == ComputeDocumentSignature(document))
    {
        return;
    }

    PushUndoSnapshot(std::move(beforeDocument));
    redoHistory.clear();
    MarkDerivedDataDirty();
}

void Editor::RestoreDocumentSnapshot(const ProjectDocument& snapshot)
{
    document = snapshot;
    MarkDerivedDataDirty();

    state.CancelCurrentTool();
    state.selectionState.selection.Clear();
    state.hover.ClearEntity();

    state.currentMaterialId = PropertySelectionOperations::ResolveCurrentMaterialId(
        document,
        state.currentMaterialId);
    state.currentSectionId = PropertySelectionOperations::ResolveCurrentSectionId(
        document,
        state.currentSectionId);
}

bool Editor::UndoDocumentChange()
{
    if (undoHistory.empty())
    {
        return false;
    }

    PushRedoSnapshot(document);
    RestoreDocumentSnapshot(undoHistory.back());
    undoHistory.pop_back();
    return true;
}

bool Editor::RedoDocumentChange()
{
    if (redoHistory.empty())
    {
        return false;
    }

    PushUndoSnapshot(document);
    RestoreDocumentSnapshot(redoHistory.back());
    redoHistory.pop_back();
    return true;
}

void Editor::RecordExternalDocumentChange(const ProjectDocument& beforeDocument)
{
    RecordDocumentChange(beforeDocument);
}

void Editor::RecordExternalDocumentChange(ProjectDocument&& beforeDocument, bool assumeChanged)
{
    RecordDocumentChange(std::move(beforeDocument), assumeChanged);
}
