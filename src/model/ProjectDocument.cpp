#include "model/ProjectDocument.h"

#include <algorithm>
#include <limits>
#include <unordered_set>

namespace
{
std::uint64_t MakeBeamSegmentKey(int startNodeId, int endNodeId)
{
    if (startNodeId > endNodeId)
    {
        std::swap(startNodeId, endNodeId);
    }

    return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(startNodeId)) << 32) |
           static_cast<std::uint32_t>(endNodeId);
}
}

void ProjectDocument::Clear()
{
    name = "Untitled";
    unitSystem = UnitSystem::SI;

    nodes.clear();
    beams.clear();
    distributedLoads.clear();
    dimensions.clear();
    materials.clear();
    sections.clear();

    nextNodeId = 1;
    nextBeamId = 1;
    nextDistributedLoadId = 1;
    nextDimensionId = 1;
    nextMaterialId = 1;
    nextSectionId = 1;
    InvalidateLookupIndices();
}

bool ProjectDocument::IsEmpty()
{
    return nodes.empty() &&
           beams.empty() &&
           distributedLoads.empty() &&
           dimensions.empty() &&
           materials.empty() &&
           sections.empty();
}

void ProjectDocument::InvalidateLookupIndices()
{
    lookupIndicesDirty = true;
}

Node* ProjectDocument::AppendNode(const Node& node)
{
    nodes.push_back(node);
    if (!lookupIndicesDirty)
    {
        nodeIndexById[node.id] = nodes.size() - 1;
    }

    return &nodes.back();
}

Beam* ProjectDocument::AppendBeam(const Beam& beam)
{
    beams.push_back(beam);
    if (!lookupIndicesDirty)
    {
        const std::size_t index = beams.size() - 1;
        beamIndexById[beam.id] = index;
        beamIndexBySegmentKey[MakeBeamSegmentKey(beam.startNodeId, beam.endNodeId)] = index;
    }

    return &beams.back();
}

BeamDistributedLoad* ProjectDocument::AppendDistributedLoad(const BeamDistributedLoad& distributedLoad)
{
    distributedLoads.push_back(distributedLoad);
    if (!lookupIndicesDirty)
    {
        distributedLoadIndexByBeamId[distributedLoad.beamId] = distributedLoads.size() - 1;
    }

    return &distributedLoads.back();
}

Dimension* ProjectDocument::AppendDimension(const Dimension& dimension)
{
    dimensions.push_back(dimension);
    if (!lookupIndicesDirty)
    {
        dimensionIndexById[dimension.id] = dimensions.size() - 1;
    }

    return &dimensions.back();
}

StructuralMaterial* ProjectDocument::AppendMaterial(const StructuralMaterial& material)
{
    materials.push_back(material);
    if (!lookupIndicesDirty)
    {
        materialIndexById[material.id] = materials.size() - 1;
    }

    return &materials.back();
}

Section* ProjectDocument::AppendSection(const Section& section)
{
    sections.push_back(section);
    if (!lookupIndicesDirty)
    {
        sectionIndexById[section.id] = sections.size() - 1;
    }

    return &sections.back();
}

void ProjectDocument::RebuildLookupIndices() const
{
    if (!lookupIndicesDirty)
    {
        return;
    }

    nodeIndexById.clear();
    nodeIndexById.reserve(nodes.size());
    for (std::size_t i = 0; i < nodes.size(); ++i)
    {
        nodeIndexById[nodes[i].id] = i;
    }

    beamIndexById.clear();
    beamIndexById.reserve(beams.size());
    beamIndexBySegmentKey.clear();
    beamIndexBySegmentKey.reserve(beams.size());
    for (std::size_t i = 0; i < beams.size(); ++i)
    {
        const Beam& beam = beams[i];
        beamIndexById[beam.id] = i;
        beamIndexBySegmentKey[MakeBeamSegmentKey(beam.startNodeId, beam.endNodeId)] = i;
    }

    distributedLoadIndexByBeamId.clear();
    distributedLoadIndexByBeamId.reserve(distributedLoads.size());
    for (std::size_t i = 0; i < distributedLoads.size(); ++i)
    {
        distributedLoadIndexByBeamId[distributedLoads[i].beamId] = i;
    }

    dimensionIndexById.clear();
    dimensionIndexById.reserve(dimensions.size());
    for (std::size_t i = 0; i < dimensions.size(); ++i)
    {
        dimensionIndexById[dimensions[i].id] = i;
    }

    materialIndexById.clear();
    materialIndexById.reserve(materials.size());
    for (std::size_t i = 0; i < materials.size(); ++i)
    {
        materialIndexById[materials[i].id] = i;
    }

    sectionIndexById.clear();
    sectionIndexById.reserve(sections.size());
    for (std::size_t i = 0; i < sections.size(); ++i)
    {
        sectionIndexById[sections[i].id] = i;
    }

    lookupIndicesDirty = false;
}

Node* ProjectDocument::FindNodeById(int id)
{
    RebuildLookupIndices();
    const auto it = nodeIndexById.find(id);
    if (it == nodeIndexById.end())
    {
        return nullptr;
    }

    return &nodes[it->second];
}

Beam* ProjectDocument::FindBeamById(int id)
{
    RebuildLookupIndices();
    const auto it = beamIndexById.find(id);
    if (it == beamIndexById.end())
    {
        return nullptr;
    }

    return &beams[it->second];
}

Beam* ProjectDocument::FindBeamBetweenNodes(int startNodeId, int endNodeId)
{
    RebuildLookupIndices();
    const auto it = beamIndexBySegmentKey.find(MakeBeamSegmentKey(startNodeId, endNodeId));
    if (it == beamIndexBySegmentKey.end())
    {
        return nullptr;
    }

    return &beams[it->second];
}

BeamDistributedLoad* ProjectDocument::FindDistributedLoadByBeamId(int beamId)
{
    RebuildLookupIndices();
    const auto it = distributedLoadIndexByBeamId.find(beamId);
    if (it == distributedLoadIndexByBeamId.end())
    {
        return nullptr;
    }

    return &distributedLoads[it->second];
}

Dimension* ProjectDocument::FindDimensionById(int id)
{
    RebuildLookupIndices();
    const auto it = dimensionIndexById.find(id);
    if (it == dimensionIndexById.end())
    {
        return nullptr;
    }

    return &dimensions[it->second];
}

const Node* ProjectDocument::FindNodeById(int id) const
{
    RebuildLookupIndices();
    const auto it = nodeIndexById.find(id);
    if (it == nodeIndexById.end())
    {
        return nullptr;
    }

    return &nodes[it->second];
}

const Beam* ProjectDocument::FindBeamById(int id) const
{
    RebuildLookupIndices();
    const auto it = beamIndexById.find(id);
    if (it == beamIndexById.end())
    {
        return nullptr;
    }

    return &beams[it->second];
}

const Beam* ProjectDocument::FindBeamBetweenNodes(int startNodeId, int endNodeId) const
{
    RebuildLookupIndices();
    const auto it = beamIndexBySegmentKey.find(MakeBeamSegmentKey(startNodeId, endNodeId));
    if (it == beamIndexBySegmentKey.end())
    {
        return nullptr;
    }

    return &beams[it->second];
}

const BeamDistributedLoad* ProjectDocument::FindDistributedLoadByBeamId(int beamId) const
{
    RebuildLookupIndices();
    const auto it = distributedLoadIndexByBeamId.find(beamId);
    if (it == distributedLoadIndexByBeamId.end())
    {
        return nullptr;
    }

    return &distributedLoads[it->second];
}

const Dimension* ProjectDocument::FindDimensionById(int id) const
{
    RebuildLookupIndices();
    const auto it = dimensionIndexById.find(id);
    if (it == dimensionIndexById.end())
    {
        return nullptr;
    }

    return &dimensions[it->second];
}

StructuralMaterial* ProjectDocument::FindMaterialById(int id)
{
    RebuildLookupIndices();
    const auto it = materialIndexById.find(id);
    if (it == materialIndexById.end())
    {
        return nullptr;
    }

    return &materials[it->second];
}

Section* ProjectDocument::FindSectionById(int id)
{
    RebuildLookupIndices();
    const auto it = sectionIndexById.find(id);
    if (it == sectionIndexById.end())
    {
        return nullptr;
    }

    return &sections[it->second];
}

const StructuralMaterial* ProjectDocument::FindMaterialById(int id) const
{
    RebuildLookupIndices();
    const auto it = materialIndexById.find(id);
    if (it == materialIndexById.end())
    {
        return nullptr;
    }

    return &materials[it->second];
}

const Section* ProjectDocument::FindSectionById(int id) const
{
    RebuildLookupIndices();
    const auto it = sectionIndexById.find(id);
    if (it == sectionIndexById.end())
    {
        return nullptr;
    }

    return &sections[it->second];
}

int ProjectDocument::AddMaterial(const std::string& newName, double youngModulus, double thermalExpansion)
{
    const int newId = nextMaterialId++;
    AppendMaterial(StructuralMaterial(newId, newName, youngModulus, thermalExpansion));
    return newId;
}

int ProjectDocument::AddSection(const std::string& newName, double area, double inertia)
{
    const int newId = nextSectionId++;
    AppendSection(Section(newId, newName, area, inertia));
    return newId;
}

int ProjectDocument::AddBeam(int startNodeId, int endNodeId, int materialId, int sectionId)
{
    if (startNodeId == endNodeId || HasBeamBetweenNodes(startNodeId, endNodeId))
    {
        return -1;
    }

    const int newId = nextBeamId++;
    AppendBeam(Beam(newId, startNodeId, endNodeId, materialId, sectionId));
    return newId;
}

int ProjectDocument::AddDimension(
    int startNodeId,
    int endNodeId,
    DimensionType type,
    LengthUnit lengthUnit,
    DimensionOffsetMode offsetMode,
    double offsetPixels,
    double offsetWorld)
{
    if (startNodeId == endNodeId)
    {
        return -1;
    }

    const int newId = nextDimensionId++;
    AppendDimension(Dimension(newId, startNodeId, endNodeId, type, lengthUnit, offsetMode, offsetPixels, offsetWorld));
    return newId;
}

bool ProjectDocument::RemoveMaterial(int id)
{
    if (IsMaterialUsed(id))
    {
        return false;
    }

    const auto oldSize = materials.size();

    materials.erase(
        std::remove_if(
            materials.begin(),
            materials.end(),
            [id](const StructuralMaterial& material)
            {
                return material.id == id;
            }),
        materials.end());

    InvalidateLookupIndices();
    return materials.size() != oldSize;
}

bool ProjectDocument::RemoveSection(int id)
{
    if (IsSectionUsed(id))
    {
        return false;
    }

    const auto oldSize = sections.size();

    sections.erase(
        std::remove_if(
            sections.begin(),
            sections.end(),
            [id](const Section& section)
            {
                return section.id == id;
            }),
        sections.end());

    InvalidateLookupIndices();
    return sections.size() != oldSize;
}

bool ProjectDocument::IsMaterialUsed(int id)
{
    for (const Beam& beam : beams)
    {
        if (beam.materialId == id)
        {
            return true;
        }
    }

    return false;
}

bool ProjectDocument::IsSectionUsed(int id)
{
    for (const Beam& beam : beams)
    {
        if (beam.sectionId == id)
        {
            return true;
        }
    }

    return false;
}

bool ProjectDocument::RemoveNode(int id)
{
    if (FindNodeById(id) == nullptr)
    {
        return false;
    }

    nodes.erase(
        std::remove_if(
            nodes.begin(),
            nodes.end(),
            [id](const Node& node)
            {
                return node.id == id;
            }),
        nodes.end());

    beams.erase(
        std::remove_if(
            beams.begin(),
            beams.end(),
            [id](const Beam& beam)
            {
                return beam.startNodeId == id || beam.endNodeId == id;
            }),
        beams.end());

    dimensions.erase(
        std::remove_if(
            dimensions.begin(),
            dimensions.end(),
            [id](const Dimension& dimension)
            {
                return dimension.startNodeId == id || dimension.endNodeId == id;
            }),
        dimensions.end());

    std::unordered_set<int> remainingBeamIds;
    remainingBeamIds.reserve(beams.size());
    for (const Beam& beam : beams)
    {
        remainingBeamIds.insert(beam.id);
    }

    distributedLoads.erase(
        std::remove_if(
            distributedLoads.begin(),
            distributedLoads.end(),
            [&remainingBeamIds](const BeamDistributedLoad& load)
            {
                return remainingBeamIds.find(load.beamId) == remainingBeamIds.end();
            }),
        distributedLoads.end());

    InvalidateLookupIndices();
    return true;
}

bool ProjectDocument::RemoveNodes(const std::vector<int>& ids)
{
    if (ids.empty())
    {
        return false;
    }

    const std::unordered_set<int> idsToRemove(ids.begin(), ids.end());

    const std::size_t oldNodeCount = nodes.size();
    const std::size_t oldBeamCount = beams.size();

    nodes.erase(
        std::remove_if(
            nodes.begin(),
            nodes.end(),
            [&idsToRemove](const Node& node)
            {
                return idsToRemove.find(node.id) != idsToRemove.end();
            }),
        nodes.end());

    beams.erase(
        std::remove_if(
            beams.begin(),
            beams.end(),
            [&idsToRemove](const Beam& beam)
            {
                return idsToRemove.find(beam.startNodeId) != idsToRemove.end() ||
                       idsToRemove.find(beam.endNodeId) != idsToRemove.end();
            }),
        beams.end());

    dimensions.erase(
        std::remove_if(
            dimensions.begin(),
            dimensions.end(),
            [&idsToRemove](const Dimension& dimension)
            {
                return idsToRemove.find(dimension.startNodeId) != idsToRemove.end() ||
                       idsToRemove.find(dimension.endNodeId) != idsToRemove.end();
            }),
        dimensions.end());

    std::unordered_set<int> remainingBeamIds;
    remainingBeamIds.reserve(beams.size());
    for (const Beam& beam : beams)
    {
        remainingBeamIds.insert(beam.id);
    }

    distributedLoads.erase(
        std::remove_if(
            distributedLoads.begin(),
            distributedLoads.end(),
            [&remainingBeamIds](const BeamDistributedLoad& load)
            {
                return remainingBeamIds.find(load.beamId) == remainingBeamIds.end();
            }),
        distributedLoads.end());

    InvalidateLookupIndices();
    return (nodes.size() != oldNodeCount) || (beams.size() != oldBeamCount);
}

bool ProjectDocument::RemoveBeam(int id)
{
    const auto oldSize = beams.size();

    beams.erase(
        std::remove_if(
            beams.begin(),
            beams.end(),
            [id](const Beam& beam)
            {
                return beam.id == id;
            }),
        beams.end());

    distributedLoads.erase(
        std::remove_if(
            distributedLoads.begin(),
            distributedLoads.end(),
            [id](const BeamDistributedLoad& load)
            {
                return load.beamId == id;
            }),
        distributedLoads.end());

    InvalidateLookupIndices();
    return beams.size() != oldSize;
}

bool ProjectDocument::RemoveBeams(const std::vector<int>& ids)
{
    if (ids.empty())
    {
        return false;
    }

    const std::unordered_set<int> idsToRemove(ids.begin(), ids.end());

    const auto oldSize = beams.size();

    beams.erase(
        std::remove_if(
            beams.begin(),
            beams.end(),
            [&idsToRemove](const Beam& beam)
            {
                return idsToRemove.find(beam.id) != idsToRemove.end();
            }),
        beams.end());

    distributedLoads.erase(
        std::remove_if(
            distributedLoads.begin(),
            distributedLoads.end(),
            [&idsToRemove](const BeamDistributedLoad& load)
            {
                return idsToRemove.find(load.beamId) != idsToRemove.end();
            }),
        distributedLoads.end());

    InvalidateLookupIndices();
    return beams.size() != oldSize;
}

bool ProjectDocument::RemoveDimension(int id)
{
    const auto oldSize = dimensions.size();

    dimensions.erase(
        std::remove_if(
            dimensions.begin(),
            dimensions.end(),
            [id](const Dimension& dimension)
            {
                return dimension.id == id;
            }),
        dimensions.end());

    InvalidateLookupIndices();
    return dimensions.size() != oldSize;
}

bool ProjectDocument::RemoveDimensions(const std::vector<int>& ids)
{
    if (ids.empty())
    {
        return false;
    }

    const std::unordered_set<int> idsToRemove(ids.begin(), ids.end());
    const auto oldSize = dimensions.size();

    dimensions.erase(
        std::remove_if(
            dimensions.begin(),
            dimensions.end(),
            [&idsToRemove](const Dimension& dimension)
            {
                return idsToRemove.find(dimension.id) != idsToRemove.end();
            }),
        dimensions.end());

    InvalidateLookupIndices();
    return dimensions.size() != oldSize;
}

bool ProjectDocument::HasBeamBetweenNodes(int startNodeId, int endNodeId) const
{
    return FindBeamBetweenNodes(startNodeId, endNodeId) != nullptr;
}
