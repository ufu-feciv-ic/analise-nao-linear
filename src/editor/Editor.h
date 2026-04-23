#pragma once

#include <cstdint>
#include <string>
#include <unordered_set>
#include <vector>

#include "editor/EditorCameraController.h"
#include "editor/cache/ProjectDerivedData.h"
#include "editor/tools/AddNodeToolController.h"
#include "editor/tools/BeamToolController.h"
#include "editor/tools/DeleteToolController.h"
#include "editor/tools/DimensionToolController.h"
#include "editor/tools/DistributedLoadToolController.h"
#include "editor/tools/EditorRequestController.h"
#include "editor/tools/PointLoadToolController.h"
#include "editor/tools/SelectToolController.h"
#include "editor/tools/SupportToolController.h"
#include "editor/tools/ToolDispatcher.h"
#include "editor/tools/TransformToolController.h"
#include "editor/SelectionMode.h"
#include "editor/EditorRenderer.h"
#include "editor/EditorState.h"
#include "editor/FrameRequests.h"
#include "model/ProjectDocument.h"

#include "raylib.h"

class Editor
{
    friend class TransformToolController;
    friend class BeamToolController;
    friend class DimensionToolController;
    friend class SupportToolController;
    friend class PointLoadToolController;
    friend class DistributedLoadToolController;
    friend class DeleteToolController;
    friend class EditorRequestController;
    friend class SelectToolController;
    friend class AddNodeToolController;

public:
    Editor();

    void Initialize();
    void SetViewportOverlayFont(Font font);
    void SetViewportOverlayTitleFont(Font font);
    void SetDimensionFont(Font font);
    void RecordExternalDocumentChange(const ProjectDocument& beforeDocument);
    void RecordExternalDocumentChange(ProjectDocument&& beforeDocument, bool assumeChanged);
    void Update(
        const FrameRequests& requests,
        bool mouseWorldInputAllowed,
        bool keyboardShortcutsAllowed,
        bool beamDistanceConfirmed,
        bool beamDistanceCancelled,
        Point2D beamDistanceCreatedNodeWorld);
    void Render();
    bool HandleEscape();

public:
    ProjectDocument document;
    ProjectDerivedData derivedData;
    EditorState state;
    Camera2D camera{};

private:
    void UpdateInteractionPreviews(Node* hoveredNode);
    void UpdateTransformPreviewState(Node* hoveredNode);
    void HandleActiveTool(
        Node* hoveredNode,
        Beam* hoveredBeam,
        Dimension* hoveredDimension,
        SelectionMode selectionMode);
    void InitializeDemoScene();
    void UpdateWorldPositions();
    Vector2 RoundPositionToGrid(Vector2 position, float spacing) const;
    float CalculateGridSpacing(float baseSpacing, float zoomLevel) const;
    Node* FindNodeByExactWorldPosition(Vector2 worldPosition);
    Node* GetOrCreateNodeAtWorldPosition(Vector2 worldPosition, bool allowAutoGuideCreation = true);
    bool SplitBeamsAtNode(int nodeId);
    bool TryAddBeamBetweenNodes(int startNodeId, int endNodeId);
    int AddBeamBetweenNodesWithProperties(int startNodeId, int endNodeId, int materialId, int sectionId);
    bool EnsureBeamPathBetweenNodes(int startNodeId, int endNodeId, int materialId, int sectionId);
    std::vector<int> CollectNodeIdsOnSegment(int startNodeId, int endNodeId) const;
    bool ResolveBeamDirection(int& startNodeId, int& endNodeId) const;
    Vector2 GetCurrentPlacementWorldPosition() const;
    SelectionMode GetSelectionMode() const;
    bool IsGuideInteractionToolActive() const;
    void ApplySelectionClick(Node* hoveredNode, Beam* hoveredBeam, Dimension* hoveredDimension, SelectionMode selectionMode);
    void HandleSelectionBox(SelectionMode selectionMode);
    bool IsRightToLeftBoxSelection() const;
    bool IsBeamInsideSelectionRectangle(const Beam& beam, Rectangle selectionRect) const;
    bool DoesBeamCrossSelectionRectangle(const Beam& beam, Rectangle selectionRect) const;
    bool IsMoveSelectionConfirmPressed() const;
    Point2D GetCurrentTransformTargetWorldPosition(Node* hoveredNode, bool allowSelectedNodeSnap) const;
    bool IsTransformTool(EditorTool tool) const;
    const char* GetTransformToolTitle(EditorTool tool) const;
    void ActivateTransformTool(EditorTool tool);
    void ApplyTransformTool(EditorTool tool, Point2D targetPointWorld);
    void BuildTransformToolStatusText(
        EditorTool tool,
        std::string& title,
        std::string& detail) const;
    void ApplySupportTypeToSelection(SupportType supportType);
    void ApplySupportTypeInSelectionRectangle(SupportType supportType);
    void ApplyPointLoadToSelection(const NodalLoad& load);
    void ApplyPointLoadInSelectionRectangle(const NodalLoad& load);
    void ClearPointLoadInSelectionRectangle();
    void ApplyDistributedLoadToSelection(const DistributedLoadValue& load);
    void ApplyDistributedLoadInSelectionRectangle(const DistributedLoadValue& load);
    void ClearDistributedLoadInSelectionRectangle();
    void BeginBeamCreationFromNode(Node* startNode);
    bool CompleteBeamCreationToNode(Node* endNode);
    bool ConsumeBeamDistanceInputResult(
        bool beamDistanceConfirmed,
        bool beamDistanceCancelled,
        Point2D beamDistanceCreatedNodeWorld);
    bool ShouldAutoCreateGuideForNewNodes() const;
    void EnsureGuideAtPosition(Vector2 position);
    void ResetGuideHoverState();
    void UpdateBeamGuides(Node* hoveredNode);
    void ToggleBeamGuideAtPosition(Vector2 position);
    void UpdateActiveBeamGuides();
    bool TryResolveTransformTargetFromGuides(Vector2& targetPointWorld);
    Node* ResolveAddNodePlacement(Node* hoveredNode, Vector2 targetPosition);
    Node* ResolveNodePlacementFromGuides(Vector2 targetPosition);
    void OpenBeamDistanceInputFromGuide(Vector2 guidePosition, Vector2 clickWorldPosition, bool locksX);
    void OpenBeamDistanceInputFromBeam(const Beam& beam, Vector2 clickWorldPosition);
    void DrawBeamGuides() const;
    void DrawBeamGuideHoverIndicator() const;
    void DrawActiveBeamGuideOriginSelectors() const;
    void DrawBeamStartIndicator() const;
    void DrawBeamDistanceReferenceIndicator() const;
    void DrawDimensionToolIndicators() const;
    void DrawMoveSnapIndicator() const;
    void DrawGridScaleOverlay() const;
    void AssignDistributedLoadAlongBeamPath(
        int originalStartNodeId,
        int originalEndNodeId,
        const DistributedLoadValue& loadValue);
    std::vector<int> CollectTransformNodeIds() const;
    bool HasTransformSelection() const;
    void BeginMoveSelection();
    void ApplyMoveSelection(Point2D targetPointWorld);
    void ApplyCopySelection(Point2D targetPointWorld);
    void ApplyMirrorSelection(Point2D axisEndPointWorld);
    bool AppendBeamPathBetweenNodesFast(
        int startNodeId,
        int endNodeId,
        int materialId,
        int sectionId,
        std::unordered_set<std::uint64_t>& existingBeamKeys);
    void CancelBeamCreationOperation();
    void CancelDimensionCreationOperation();
    void CancelDimensionMoveOperation();
    void NormalizeStructureAfterMove();
    void MergeCoincidentNodes();
    void RebuildBeamsAfterNodeChanges();
    void CleanupDimensionsAfterNodeChanges();
    bool TryGetBeamIntersectionSnap(Vector2& snappedPosition) const;
    bool TryGetBeamInteriorSnap(Vector2& snappedPosition, int& referenceBeamId) const;
    void BeginBoxSelection();
    void UpdateBoxSelection();
    void EndBoxSelection(SelectionMode selectionMode);
    bool FinalizeNodeBoxSelection();
    bool FinalizeBeamBoxSelection();
    bool DeleteNodesInSelectionRectangle();
    bool DeleteBeamsInSelectionRectangle();
    Rectangle GetSelectionRectangle() const;
    void HandleKeyboardShortcuts();
    void HandleBoxSelectionReleaseWithoutWorldInput(SelectionMode selectionMode);
    void DrawViewportStatusOverlay() const;
    void BuildViewportStatusText(std::string& title, std::string& detail) const;
    void CenterCameraOnStructure();
    void MarkDerivedDataDirty();
    void RefreshDerivedDataIfNeeded();

    void DeleteSelection();
    void DeleteSelectedNodes();
    void DeleteSelectedBeams();
    void DeleteSelectedDimensions();

    bool DeleteNodeById(int nodeId);
    bool DeleteBeamById(int beamId);
    bool DeleteDimensionById(int dimensionId);

    void PurgeInvalidSelection();

private:
    EditorCameraController cameraController;
    EditorRenderer renderer;
    SelectToolController selectToolController;
    AddNodeToolController addNodeToolController;
    TransformToolController transformToolController;
    BeamToolController beamToolController;
    DimensionToolController dimensionToolController;
    SupportToolController supportToolController;
    PointLoadToolController pointLoadToolController;
    DistributedLoadToolController distributedLoadToolController;
    DeleteToolController deleteToolController;
    EditorRequestController editorRequestController;
    ToolDispatcher toolDispatcher;
    Font viewportOverlayFont{};
    bool hasViewportOverlayFont = false;
    Font viewportOverlayTitleFont{};
    bool hasViewportOverlayTitleFont = false;
    std::vector<ProjectDocument> undoHistory;
    std::vector<ProjectDocument> redoHistory;

    Vector2 mouseScreenPosition{0.0f, 0.0f};
    Vector2 mouseWorldPosition{0.0f, 0.0f};
    Vector2 snappedWorldPosition{0.0f, 0.0f};
    bool derivedDataDirty = true;

private:
    std::uint64_t ComputeDocumentSignature(const ProjectDocument& snapshot) const;
    void RecordDocumentChange(const ProjectDocument& beforeDocument);
    void RecordDocumentChange(ProjectDocument&& beforeDocument, bool assumeChanged);
    void PushUndoSnapshot(const ProjectDocument& snapshot);
    void PushUndoSnapshot(ProjectDocument&& snapshot);
    void PushRedoSnapshot(const ProjectDocument& snapshot);
    bool UndoDocumentChange();
    bool RedoDocumentChange();
    void RestoreDocumentSnapshot(const ProjectDocument& snapshot);
    bool SaveDocumentToJson(const char* filePath) const;
    bool LoadDocumentFromJson(const char* filePath);
};
