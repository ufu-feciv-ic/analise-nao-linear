#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include "model/Beam.h"
#include "model/Dimension.h"
#include "model/DistributedLoad.h"
#include "model/Node.h"
#include "model/Section.h"
#include "model/StructuralMaterial.h"
#include "model/DisplayUnits.h"

enum class UnitSystem
{
    SI
};

class ProjectDocument
{
public:
    std::string name = "Untitled";
    UnitSystem unitSystem = UnitSystem::SI;
    DisplayUnits displayUnits;

    std::vector<Node> nodes;
    std::vector<Beam> beams;
    std::vector<BeamDistributedLoad> distributedLoads;
    std::vector<Dimension> dimensions;
    std::vector<StructuralMaterial> materials;
    std::vector<Section> sections;

    int nextNodeId = 1;
    int nextBeamId = 1;
    int nextDistributedLoadId = 1;
    int nextDimensionId = 1;
    int nextMaterialId = 1;
    int nextSectionId = 1;

    void Clear();
    bool IsEmpty();
    void InvalidateLookupIndices();

    Node* AppendNode(const Node& node);
    Beam* AppendBeam(const Beam& beam);
    BeamDistributedLoad* AppendDistributedLoad(const BeamDistributedLoad& distributedLoad);
    Dimension* AppendDimension(const Dimension& dimension);
    StructuralMaterial* AppendMaterial(const StructuralMaterial& material);
    Section* AppendSection(const Section& section);

    Node* FindNodeById(int id);
    Beam* FindBeamById(int id);
    Beam* FindBeamBetweenNodes(int startNodeId, int endNodeId);
    BeamDistributedLoad* FindDistributedLoadByBeamId(int beamId);
    Dimension* FindDimensionById(int id);
    const Node* FindNodeById(int id) const;
    const Beam* FindBeamById(int id) const;
    const Beam* FindBeamBetweenNodes(int startNodeId, int endNodeId) const;
    const BeamDistributedLoad* FindDistributedLoadByBeamId(int beamId) const;
    const Dimension* FindDimensionById(int id) const;
    StructuralMaterial* FindMaterialById(int id);
    Section* FindSectionById(int id);
    const StructuralMaterial* FindMaterialById(int id) const;
    const Section* FindSectionById(int id) const;
    int AddMaterial(const std::string& name, double youngModulus, double thermalExpansion);
    int AddSection(const std::string& name, double area, double inertia);
    int AddBeam(int startNodeId, int endNodeId, int materialId, int sectionId);
    int AddDimension(
        int startNodeId,
        int endNodeId,
        DimensionType type,
        LengthUnit lengthUnit,
        DimensionOffsetMode offsetMode,
        double offsetPixels,
        double offsetWorld);

    bool RemoveNode(int id);
    bool RemoveNodes(const std::vector<int>& ids);

    bool RemoveBeam(int id);
    bool RemoveBeams(const std::vector<int>& ids);
    bool RemoveDimension(int id);
    bool RemoveDimensions(const std::vector<int>& ids);

    bool RemoveMaterial(int id);
    bool RemoveSection(int id);

    bool HasBeamBetweenNodes(int startNodeId, int endNodeId) const;
    bool IsMaterialUsed(int id);
    bool IsSectionUsed(int id);

private:
    void RebuildLookupIndices() const;

private:
    mutable bool lookupIndicesDirty = true;
    mutable std::unordered_map<int, std::size_t> nodeIndexById;
    mutable std::unordered_map<int, std::size_t> beamIndexById;
    mutable std::unordered_map<std::uint64_t, std::size_t> beamIndexBySegmentKey;
    mutable std::unordered_map<int, std::size_t> distributedLoadIndexByBeamId;
    mutable std::unordered_map<int, std::size_t> dimensionIndexById;
    mutable std::unordered_map<int, std::size_t> materialIndexById;
    mutable std::unordered_map<int, std::size_t> sectionIndexById;
};
