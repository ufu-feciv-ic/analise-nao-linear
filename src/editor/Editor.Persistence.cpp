#include "editor/Editor.h"

#include <algorithm>
#include <cstdio>
#include <string>

#include "app/AppVersion.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/prettywriter.h"

namespace
{
    bool TryReadInt(const rapidjson::Value& object, const char* name, int& value)
    {
        if (!object.IsObject() || !object.HasMember(name) || !object[name].IsInt())
        {
            return false;
        }

        value = object[name].GetInt();
        return true;
    }

    bool TryReadDouble(const rapidjson::Value& object, const char* name, double& value)
    {
        if (!object.IsObject() || !object.HasMember(name) || !object[name].IsNumber())
        {
            return false;
        }

        value = object[name].GetDouble();
        return true;
    }

    bool TryReadString(const rapidjson::Value& object, const char* name, std::string& value)
    {
        if (!object.IsObject() || !object.HasMember(name) || !object[name].IsString())
        {
            return false;
        }

        value = object[name].GetString();
        return true;
    }

    template <typename EnumType>
    bool TryReadEnum(const rapidjson::Value& object, const char* name, EnumType& value)
    {
        int rawValue = 0;
        if (!TryReadInt(object, name, rawValue))
        {
            return false;
        }

        value = static_cast<EnumType>(rawValue);
        return true;
    }

    bool SerializeProjectDocumentJson(FILE* file, const ProjectDocument& document)
    {
        char buffer[65536];
        rapidjson::FileWriteStream stream(file, buffer, sizeof(buffer));
        rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(stream);
        writer.SetIndent(' ', 2);

        writer.StartObject();

        writer.Key("name");
        writer.String(document.name.c_str());

        writer.Key("appVersion");
        writer.String(AppVersion::String);

        writer.Key("projectFileFormatVersion");
        writer.Int(AppVersion::CurrentProjectFileFormatVersion);

        writer.Key("unitSystem");
        writer.Int(static_cast<int>(document.unitSystem));

        writer.Key("displayUnits");
        writer.StartObject();
        writer.Key("length");
        writer.Int(static_cast<int>(document.displayUnits.length));
        writer.Key("force");
        writer.Int(static_cast<int>(document.displayUnits.force));
        writer.Key("moment");
        writer.Int(static_cast<int>(document.displayUnits.moment));
        writer.Key("distributedLoad");
        writer.Int(static_cast<int>(document.displayUnits.distributedLoad));
        writer.Key("stress");
        writer.Int(static_cast<int>(document.displayUnits.stress));
        writer.Key("elasticModulus");
        writer.Int(static_cast<int>(document.displayUnits.elasticModulus));
        writer.Key("area");
        writer.Int(static_cast<int>(document.displayUnits.area));
        writer.Key("inertia");
        writer.Int(static_cast<int>(document.displayUnits.inertia));
        writer.EndObject();

        writer.Key("nextIds");
        writer.StartObject();
        writer.Key("node");
        writer.Int(document.nextNodeId);
        writer.Key("beam");
        writer.Int(document.nextBeamId);
        writer.Key("distributedLoad");
        writer.Int(document.nextDistributedLoadId);
        writer.Key("dimension");
        writer.Int(document.nextDimensionId);
        writer.Key("material");
        writer.Int(document.nextMaterialId);
        writer.Key("section");
        writer.Int(document.nextSectionId);
        writer.EndObject();

        writer.Key("nodes");
        writer.StartArray();
        for (const Node& node : document.nodes)
        {
            writer.StartObject();
            writer.Key("id");
            writer.Int(node.id);
            writer.Key("x");
            writer.Double(node.position.x);
            writer.Key("y");
            writer.Double(node.position.y);
            writer.Key("support");
            writer.Int(static_cast<int>(node.support));
            writer.Key("load");
            writer.StartObject();
            writer.Key("fx");
            writer.Double(node.load.fx);
            writer.Key("fy");
            writer.Double(node.load.fy);
            writer.Key("mz");
            writer.Double(node.load.mz);
            writer.EndObject();
            writer.EndObject();
        }
        writer.EndArray();

        writer.Key("beams");
        writer.StartArray();
        for (const Beam& beam : document.beams)
        {
            writer.StartObject();
            writer.Key("id");
            writer.Int(beam.id);
            writer.Key("startNodeId");
            writer.Int(beam.startNodeId);
            writer.Key("endNodeId");
            writer.Int(beam.endNodeId);
            writer.Key("materialId");
            writer.Int(beam.materialId);
            writer.Key("sectionId");
            writer.Int(beam.sectionId);
            writer.EndObject();
        }
        writer.EndArray();

        writer.Key("distributedLoads");
        writer.StartArray();
        for (const BeamDistributedLoad& load : document.distributedLoads)
        {
            writer.StartObject();
            writer.Key("id");
            writer.Int(load.id);
            writer.Key("beamId");
            writer.Int(load.beamId);
            writer.Key("qxStart");
            writer.Double(load.value.qxStart);
            writer.Key("qyStart");
            writer.Double(load.value.qyStart);
            writer.Key("qxEnd");
            writer.Double(load.value.qxEnd);
            writer.Key("qyEnd");
            writer.Double(load.value.qyEnd);
            writer.EndObject();
        }
        writer.EndArray();

        writer.Key("dimensions");
        writer.StartArray();
        for (const Dimension& dimension : document.dimensions)
        {
            writer.StartObject();
            writer.Key("id");
            writer.Int(dimension.id);
            writer.Key("startNodeId");
            writer.Int(dimension.startNodeId);
            writer.Key("endNodeId");
            writer.Int(dimension.endNodeId);
            writer.Key("type");
            writer.Int(static_cast<int>(dimension.type));
            writer.Key("lengthUnit");
            writer.Int(static_cast<int>(dimension.lengthUnit));
            writer.Key("offsetMode");
            writer.Int(static_cast<int>(dimension.offsetMode));
            writer.Key("offsetPixels");
            writer.Double(dimension.offsetPixels);
            writer.Key("offsetWorld");
            writer.Double(dimension.offsetWorld);
            writer.EndObject();
        }
        writer.EndArray();

        writer.Key("materials");
        writer.StartArray();
        for (const StructuralMaterial& material : document.materials)
        {
            writer.StartObject();
            writer.Key("id");
            writer.Int(material.id);
            writer.Key("name");
            writer.String(material.name.c_str());
            writer.Key("youngModulus");
            writer.Double(material.youngModulus);
            writer.Key("thermalExpansion");
            writer.Double(material.thermalExpansion);
            writer.EndObject();
        }
        writer.EndArray();

        writer.Key("sections");
        writer.StartArray();
        for (const Section& section : document.sections)
        {
            writer.StartObject();
            writer.Key("id");
            writer.Int(section.id);
            writer.Key("name");
            writer.String(section.name.c_str());
            writer.Key("area");
            writer.Double(section.area);
            writer.Key("inertia");
            writer.Double(section.inertia);
            writer.EndObject();
        }
        writer.EndArray();

        writer.EndObject();
        return !writer.IsComplete() ? false : !ferror(file);
    }

    bool DeserializeProjectDocumentJson(const rapidjson::Document& root, ProjectDocument& document)
    {
        if (!root.IsObject())
        {
            return false;
        }

        int projectFileFormatVersion = 0;
        if (!root.HasMember("projectFileFormatVersion"))
        {
            return false;
        }

        if (!TryReadInt(root, "projectFileFormatVersion", projectFileFormatVersion))
        {
            return false;
        }

        if (!AppVersion::IsSupportedProjectFileFormatVersion(projectFileFormatVersion))
        {
            return false;
        }

        ProjectDocument loadedDocument;

        if (!TryReadString(root, "name", loadedDocument.name))
        {
            return false;
        }

        if (!TryReadEnum(root, "unitSystem", loadedDocument.unitSystem))
        {
            return false;
        }

        if (!root.HasMember("displayUnits") || !root["displayUnits"].IsObject())
        {
            return false;
        }

        const rapidjson::Value& displayUnits = root["displayUnits"];
        if (!TryReadEnum(displayUnits, "length", loadedDocument.displayUnits.length) ||
            !TryReadEnum(displayUnits, "force", loadedDocument.displayUnits.force) ||
            !TryReadEnum(displayUnits, "moment", loadedDocument.displayUnits.moment) ||
            !TryReadEnum(displayUnits, "distributedLoad", loadedDocument.displayUnits.distributedLoad) ||
            !TryReadEnum(displayUnits, "stress", loadedDocument.displayUnits.stress) ||
            !TryReadEnum(displayUnits, "elasticModulus", loadedDocument.displayUnits.elasticModulus) ||
            !TryReadEnum(displayUnits, "area", loadedDocument.displayUnits.area) ||
            !TryReadEnum(displayUnits, "inertia", loadedDocument.displayUnits.inertia))
        {
            return false;
        }

        if (!root.HasMember("nextIds") || !root["nextIds"].IsObject())
        {
            return false;
        }

        const rapidjson::Value& nextIds = root["nextIds"];
        if (!TryReadInt(nextIds, "node", loadedDocument.nextNodeId) ||
            !TryReadInt(nextIds, "beam", loadedDocument.nextBeamId) ||
            !TryReadInt(nextIds, "distributedLoad", loadedDocument.nextDistributedLoadId) ||
            !TryReadInt(nextIds, "dimension", loadedDocument.nextDimensionId) ||
            !TryReadInt(nextIds, "material", loadedDocument.nextMaterialId) ||
            !TryReadInt(nextIds, "section", loadedDocument.nextSectionId))
        {
            return false;
        }

        if (!root.HasMember("nodes") || !root["nodes"].IsArray())
        {
            return false;
        }

        for (const rapidjson::Value& nodeValue : root["nodes"].GetArray())
        {
            if (!nodeValue.IsObject())
            {
                return false;
            }

            Node node;
            double x = 0.0;
            double y = 0.0;
            if (!TryReadInt(nodeValue, "id", node.id) ||
                !TryReadDouble(nodeValue, "x", x) ||
                !TryReadDouble(nodeValue, "y", y) ||
                !TryReadEnum(nodeValue, "support", node.support))
            {
                return false;
            }

            node.position = Point2D{x, y};

            if (!nodeValue.HasMember("load") || !nodeValue["load"].IsObject())
            {
                return false;
            }

            const rapidjson::Value& load = nodeValue["load"];
            if (!TryReadDouble(load, "fx", node.load.fx) ||
                !TryReadDouble(load, "fy", node.load.fy) ||
                !TryReadDouble(load, "mz", node.load.mz))
            {
                return false;
            }

            loadedDocument.AppendNode(node);
        }

        if (!root.HasMember("beams") || !root["beams"].IsArray())
        {
            return false;
        }

        for (const rapidjson::Value& beamValue : root["beams"].GetArray())
        {
            if (!beamValue.IsObject())
            {
                return false;
            }

            Beam beam;
            if (!TryReadInt(beamValue, "id", beam.id) ||
                !TryReadInt(beamValue, "startNodeId", beam.startNodeId) ||
                !TryReadInt(beamValue, "endNodeId", beam.endNodeId) ||
                !TryReadInt(beamValue, "materialId", beam.materialId) ||
                !TryReadInt(beamValue, "sectionId", beam.sectionId))
            {
                return false;
            }

            loadedDocument.AppendBeam(beam);
        }

        if (!root.HasMember("distributedLoads") || !root["distributedLoads"].IsArray())
        {
            return false;
        }

        for (const rapidjson::Value& loadValue : root["distributedLoads"].GetArray())
        {
            if (!loadValue.IsObject())
            {
                return false;
            }

            BeamDistributedLoad load;
            if (!TryReadInt(loadValue, "id", load.id) ||
                !TryReadInt(loadValue, "beamId", load.beamId) ||
                !TryReadDouble(loadValue, "qxStart", load.value.qxStart) ||
                !TryReadDouble(loadValue, "qyStart", load.value.qyStart) ||
                !TryReadDouble(loadValue, "qxEnd", load.value.qxEnd) ||
                !TryReadDouble(loadValue, "qyEnd", load.value.qyEnd))
            {
                return false;
            }

            loadedDocument.AppendDistributedLoad(load);
        }

        if (!root.HasMember("dimensions") || !root["dimensions"].IsArray())
        {
            return false;
        }

        for (const rapidjson::Value& dimensionValue : root["dimensions"].GetArray())
        {
            if (!dimensionValue.IsObject())
            {
                return false;
            }

            Dimension dimension;
            if (!TryReadInt(dimensionValue, "id", dimension.id) ||
                !TryReadInt(dimensionValue, "startNodeId", dimension.startNodeId) ||
                !TryReadInt(dimensionValue, "endNodeId", dimension.endNodeId) ||
                !TryReadEnum(dimensionValue, "type", dimension.type) ||
                !TryReadEnum(dimensionValue, "lengthUnit", dimension.lengthUnit) ||
                !TryReadEnum(dimensionValue, "offsetMode", dimension.offsetMode) ||
                !TryReadDouble(dimensionValue, "offsetPixels", dimension.offsetPixels) ||
                !TryReadDouble(dimensionValue, "offsetWorld", dimension.offsetWorld))
            {
                return false;
            }

            loadedDocument.AppendDimension(dimension);
        }

        if (!root.HasMember("materials") || !root["materials"].IsArray())
        {
            return false;
        }

        for (const rapidjson::Value& materialValue : root["materials"].GetArray())
        {
            if (!materialValue.IsObject())
            {
                return false;
            }

            StructuralMaterial material;
            if (!TryReadInt(materialValue, "id", material.id) ||
                !TryReadString(materialValue, "name", material.name) ||
                !TryReadDouble(materialValue, "youngModulus", material.youngModulus) ||
                !TryReadDouble(materialValue, "thermalExpansion", material.thermalExpansion))
            {
                return false;
            }

            loadedDocument.AppendMaterial(material);
        }

        if (!root.HasMember("sections") || !root["sections"].IsArray())
        {
            return false;
        }

        for (const rapidjson::Value& sectionValue : root["sections"].GetArray())
        {
            if (!sectionValue.IsObject())
            {
                return false;
            }

            Section section;
            if (!TryReadInt(sectionValue, "id", section.id) ||
                !TryReadString(sectionValue, "name", section.name) ||
                !TryReadDouble(sectionValue, "area", section.area) ||
                !TryReadDouble(sectionValue, "inertia", section.inertia))
            {
                return false;
            }

            loadedDocument.AppendSection(section);
        }

        document = std::move(loadedDocument);
        return true;
    }
}

void Editor::CenterCameraOnStructure()
{
    if (document.nodes.empty())
    {
        camera.target = Vector2{0.0f, 0.0f};
        return;
    }

    double minX = document.nodes.front().position.x;
    double maxX = minX;
    double minY = document.nodes.front().position.y;
    double maxY = minY;

    for (const Node& node : document.nodes)
    {
        minX = std::min(minX, node.position.x);
        maxX = std::max(maxX, node.position.x);
        minY = std::min(minY, node.position.y);
        maxY = std::max(maxY, node.position.y);
    }

    const double centerX = (minX + maxX) * 0.5;
    const double centerY = (minY + maxY) * 0.5;
    camera.target = Vector2{
        static_cast<float>(centerX),
        static_cast<float>(centerY)};
}

bool Editor::SaveDocumentToJson(const char* filePath) const
{
    FILE* file = fopen(filePath, "wb");
    if (file == nullptr)
    {
        TraceLog(LOG_WARNING, "Failed to open project JSON for writing: %s", filePath);
        return false;
    }

    const bool saved = SerializeProjectDocumentJson(file, document);
    fclose(file);

    if (!saved)
    {
        TraceLog(LOG_WARNING, "Failed to serialize project JSON: %s", filePath);
    }

    return saved;
}

bool Editor::LoadDocumentFromJson(const char* filePath)
{
    FILE* file = fopen(filePath, "rb");
    if (file == nullptr)
    {
        TraceLog(LOG_WARNING, "Failed to open project JSON for reading: %s", filePath);
        return false;
    }

    char buffer[65536];
    rapidjson::FileReadStream stream(file, buffer, sizeof(buffer));
    rapidjson::Document root;
    root.ParseStream(stream);
    fclose(file);

    if (root.HasParseError())
    {
        TraceLog(
            LOG_WARNING,
            "Failed to parse project JSON %s: %s",
            filePath,
            rapidjson::GetParseError_En(root.GetParseError()));
        return false;
    }

    ProjectDocument loadedDocument;
    if (!DeserializeProjectDocumentJson(root, loadedDocument))
    {
        TraceLog(LOG_WARNING, "Project JSON has invalid structure: %s", filePath);
        return false;
    }

    RestoreDocumentSnapshot(loadedDocument);
    CenterCameraOnStructure();
    undoHistory.clear();
    redoHistory.clear();
    return true;
}
