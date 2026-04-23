#include "editor/Editor.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>

void Editor::InitializeDemoScene()
{
    constexpr float demoScale = 1.0f / 30.0f;

    document.Clear();
    document.name = "Novo Projeto";

    const int concreteMaterialId = document.AddMaterial("Concreto", 30.0e9, 1.0e-5);
    const int steelMaterialId = document.AddMaterial("Aço", 200.0e9, 1.2e-5);
    const int frameSectionId = document.AddSection("Seção 20x40 cm", 0.20 * 0.40, 0.20 * 0.40 * 0.40 * 0.40 / 12.0);
    const int trussSectionId = document.AddSection("Perfil tubular", 0.012, 2.5e-5);

    state.currentMaterialId = concreteMaterialId;
    state.currentSectionId = frameSectionId;
    state.selectionState.selection.Clear();
    state.beamTool.beamGuides.clear();
    state.hover.ClearEntity();

    document.nodes.reserve(220);
    document.beams.reserve(220);

    std::unordered_map<int, Vector2> nodePositionsById;
    std::unordered_set<std::uint64_t> beamKeys;
    std::vector<int> frameRoofBeamIds;
    int demoRectangularTrussBottomBeamId = -1;

    auto addNode = [&](float x, float y)
    {
        const int nodeId = document.nextNodeId++;
        document.AppendNode(Node(nodeId, static_cast<double>(x), static_cast<double>(y)));
        nodePositionsById[nodeId] = Vector2{x, y};
        return nodeId;
    };

    auto normalizeBeamDirection = [&](int& startNodeId, int& endNodeId)
    {
        const Vector2 startPosition = nodePositionsById[startNodeId];
        const Vector2 endPosition = nodePositionsById[endNodeId];
        const bool isVertical = std::abs(startPosition.x - endPosition.x) <= 1.0e-6f;
        const bool shouldSwap =
            isVertical
                ? (startPosition.y < endPosition.y)
                : (startPosition.x > endPosition.x);

        if (shouldSwap)
        {
            std::swap(startNodeId, endNodeId);
        }
    };

    auto makeBeamKey = [](int startNodeId, int endNodeId)
    {
        return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(startNodeId)) << 32) |
               static_cast<std::uint32_t>(endNodeId);
    };

    auto addBeam = [&](int startNodeId, int endNodeId, int materialId, int sectionId)
    {
        if (startNodeId < 0 || endNodeId < 0 || startNodeId == endNodeId)
        {
            return -1;
        }

        normalizeBeamDirection(startNodeId, endNodeId);
        const std::uint64_t beamKey = makeBeamKey(startNodeId, endNodeId);
        if (!beamKeys.insert(beamKey).second)
        {
            return -1;
        }

        const int beamId = document.nextBeamId++;
        document.AppendBeam(Beam(beamId, startNodeId, endNodeId, materialId, sectionId));
        return beamId;
    };

    auto setNodeSupport = [&](int nodeId, SupportType supportType)
    {
        for (Node& node : document.nodes)
        {
            if (node.id == nodeId)
            {
                node.support = supportType;
                break;
            }
        }
    };

    auto setNodeLoad = [&](int nodeId, double fx, double fy, double mz)
    {
        for (Node& node : document.nodes)
        {
            if (node.id == nodeId)
            {
                node.load = NodalLoad{fx, fy, mz};
                break;
            }
        }
    };

    auto addFrameBuilding = [&](float originX, float baseY)
    {
        constexpr int columnCount = 3;
        constexpr int storyCount = 5;
        constexpr float bayWidth = 120.0f * demoScale;
        constexpr float storyHeight = 90.0f * demoScale;

        int nodes[columnCount][storyCount + 1]{};

        for (int column = 0; column < columnCount; ++column)
        {
            for (int level = 0; level <= storyCount; ++level)
            {
                nodes[column][level] = addNode(
                    originX + static_cast<float>(column) * bayWidth,
                    baseY - static_cast<float>(level) * storyHeight);
            }
        }

        for (int column = 0; column < columnCount; ++column)
        {
            for (int level = 0; level < storyCount; ++level)
            {
                addBeam(nodes[column][level], nodes[column][level + 1], concreteMaterialId, frameSectionId);
            }
        }

        for (int level = 1; level <= storyCount; ++level)
        {
            for (int column = 0; column < columnCount - 1; ++column)
            {
                const int beamId = addBeam(
                    nodes[column][level],
                    nodes[column + 1][level],
                    concreteMaterialId,
                    frameSectionId);
                if (level == storyCount && beamId >= 0)
                {
                    frameRoofBeamIds.push_back(beamId);
                }
            }
        }

        setNodeSupport(nodes[0][0], SupportType::RestrainedXY);
        setNodeSupport(nodes[1][0], SupportType::RestrainedY);
        setNodeSupport(nodes[2][0], SupportType::RestrainedY);

        setNodeLoad(nodes[0][storyCount], 9000.0, 0.0, 0.0);
        setNodeLoad(nodes[1][storyCount], 0.0, -18000.0, 0.0);
        setNodeLoad(nodes[2][storyCount], 0.0, 0.0, 12000.0);
    };

    enum class DemoTrussStyle
    {
        Rectangular,
        Pratt,
        Howe
    };

    auto addTruss = [&](float originX, float baseY, DemoTrussStyle style)
    {
        constexpr int panelCount = 6;
        constexpr float panelWidth = 48.0f * demoScale;
        constexpr float rectangularTrussHeight = 82.0f * demoScale;
        constexpr float roofEaveHeight = 56.0f * demoScale;
        constexpr float roofRidgeRise = 34.0f * demoScale;

        std::vector<int> bottomNodes;
        std::vector<int> topNodes;
        bottomNodes.reserve(panelCount + 1);
        topNodes.reserve(panelCount + 1);

        for (int i = 0; i <= panelCount; ++i)
        {
            const float x = originX + static_cast<float>(i) * panelWidth;
            float topY = baseY - rectangularTrussHeight;
            if (style != DemoTrussStyle::Rectangular)
            {
                const float halfPanelCount = static_cast<float>(panelCount) * 0.5f;
                const float distanceToCenter = std::abs(static_cast<float>(i) - halfPanelCount);
                const float roofFactor = 1.0f - distanceToCenter / halfPanelCount;
                topY = baseY - roofEaveHeight - roofRidgeRise * roofFactor;
            }

            bottomNodes.push_back(addNode(x, baseY));
            topNodes.push_back(addNode(x, topY));
        }

        for (int i = 0; i < panelCount; ++i)
        {
            const int bottomBeamId = addBeam(bottomNodes[i], bottomNodes[i + 1], steelMaterialId, trussSectionId);
            addBeam(topNodes[i], topNodes[i + 1], steelMaterialId, trussSectionId);
            if (style == DemoTrussStyle::Rectangular && i == 0)
            {
                demoRectangularTrussBottomBeamId = bottomBeamId;
            }
        }

        for (int i = 0; i <= panelCount; ++i)
        {
            addBeam(bottomNodes[i], topNodes[i], steelMaterialId, trussSectionId);
        }

        if (style == DemoTrussStyle::Rectangular)
        {
            for (int i = 0; i < panelCount; ++i)
            {
                if ((i % 2) == 0)
                {
                    addBeam(bottomNodes[i], topNodes[i + 1], steelMaterialId, trussSectionId);
                }
                else
                {
                    addBeam(topNodes[i], bottomNodes[i + 1], steelMaterialId, trussSectionId);
                }
            }
            return;
        }

        const int halfPanel = panelCount / 2;
        for (int i = 0; i < panelCount; ++i)
        {
            const bool isLeftSide = i < halfPanel;

            if (style == DemoTrussStyle::Pratt)
            {
                if (isLeftSide)
                {
                    addBeam(topNodes[i], bottomNodes[i + 1], steelMaterialId, trussSectionId);
                }
                else
                {
                    addBeam(bottomNodes[i], topNodes[i + 1], steelMaterialId, trussSectionId);
                }
            }
            else
            {
                if (isLeftSide)
                {
                    addBeam(bottomNodes[i], topNodes[i + 1], steelMaterialId, trussSectionId);
                }
                else
                {
                    addBeam(topNodes[i], bottomNodes[i + 1], steelMaterialId, trussSectionId);
                }
            }
        }

        setNodeSupport(bottomNodes.front(), SupportType::RestrainedXY);
        setNodeSupport(bottomNodes.back(), SupportType::RestrainedY);

        setNodeLoad(topNodes[panelCount / 2], 0.0, -10000.0, 0.0);
    };

    auto addRing = [&](float centerX, float centerY, float radius, int segmentCount)
    {
        std::vector<int> ringNodeIds;
        ringNodeIds.reserve(segmentCount);
        constexpr double ringLoadMagnitude = 9000.0;

        for (int i = 0; i < segmentCount; ++i)
        {
            const float angle = static_cast<float>((2.0 * PI * i) / segmentCount);
            const float cosine = cosf(angle);
            const float sine = sinf(angle);
            const int nodeId = addNode(
                centerX + radius * cosine,
                centerY + radius * sine);
            ringNodeIds.push_back(nodeId);
            setNodeLoad(
                nodeId,
                -ringLoadMagnitude * static_cast<double>(cosine),
                ringLoadMagnitude * static_cast<double>(sine),
                0.0);
        }

        for (int i = 0; i < segmentCount; ++i)
        {
            addBeam(
                ringNodeIds[i],
                ringNodeIds[(i + 1) % segmentCount],
                steelMaterialId,
                trussSectionId);
        }
    };

    auto addRadialArray = [&](float centerX, float centerY, int radialCount)
    {
        constexpr float innerRadius = 42.0f * demoScale;
        constexpr float middleRadius = 94.0f * demoScale;
        constexpr float outerRadius = 146.0f * demoScale;

        for (int i = 0; i < radialCount; ++i)
        {
            const float angle = static_cast<float>((2.0 * PI * i) / radialCount);
            const float cosine = cosf(angle);
            const float sine = sinf(angle);

            const int innerNodeId = addNode(centerX + innerRadius * cosine, centerY + innerRadius * sine);
            const int middleNodeId = addNode(centerX + middleRadius * cosine, centerY + middleRadius * sine);
            const int outerNodeId = addNode(centerX + outerRadius * cosine, centerY + outerRadius * sine);

            addBeam(innerNodeId, middleNodeId, steelMaterialId, trussSectionId);
            addBeam(middleNodeId, outerNodeId, steelMaterialId, trussSectionId);

            if (i == 0)
            {
                setNodeLoad(outerNodeId, 6000.0, -6000.0, 0.0);
            }
        }
    };

    addFrameBuilding(0.0f * demoScale, 0.0f * demoScale);
    addTruss(420.0f * demoScale, 0.0f * demoScale, DemoTrussStyle::Rectangular);
    addTruss(760.0f * demoScale, 0.0f * demoScale, DemoTrussStyle::Pratt);
    addTruss(1100.0f * demoScale, 0.0f * demoScale, DemoTrussStyle::Howe);
    addRing(1520.0f * demoScale, -220.0f * demoScale, 135.0f * demoScale, 32);
    addRadialArray(1880.0f * demoScale, -220.0f * demoScale, 32);

    if (!frameRoofBeamIds.empty())
    {
        document.AppendDistributedLoad(BeamDistributedLoad(
            document.nextDistributedLoadId++,
            frameRoofBeamIds.front(),
            DistributedLoadValue{0.0, -18000.0, 0.0, -18000.0}));
    }

    if (frameRoofBeamIds.size() > 1)
    {
        document.AppendDistributedLoad(BeamDistributedLoad(
            document.nextDistributedLoadId++,
            frameRoofBeamIds[1],
            DistributedLoadValue{0.0, -10000.0, 0.0, -22000.0}));
    }

    if (demoRectangularTrussBottomBeamId >= 0)
    {
        document.AppendDistributedLoad(BeamDistributedLoad(
            document.nextDistributedLoadId++,
            demoRectangularTrussBottomBeamId,
            DistributedLoadValue{9000.0, 0.0, 9000.0, 0.0}));
    }
}
