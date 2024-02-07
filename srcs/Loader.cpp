/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Loader.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nflan <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/02/06 18:51:41 by nflan             #+#    #+#             */
/*   Updated: 2024/02/06 18:51:43 by nflan            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../incs/Loader.hpp"

#define OBJL_CONSOLE_OUTPUT

bool Loader::LoadFile(const std::string& Path)
{
    // If the file is not an .obj file return false
    if (Path.substr(Path.size() - 4, 4) != ".obj")
		throw std::runtime_error("'" + Path + "' is not a .obj file!");

    std::ifstream file(Path);

    if (!file)
		throw std::runtime_error("Failed to open: '" + Path + "'!");

    LoadedMeshes.clear();
    LoadedVertices.clear();
    LoadedIndices.clear();

    std::vector<glm::vec3> Positions;
    std::vector<glm::vec2> TCoords;
    std::vector<glm::vec3> Normals;

    std::vector<Vertex> _vertices;
    std::vector<unsigned int> _indices;

    std::vector<std::string> MeshMatNames;

    bool listening = false;
    std::string meshname;

    Mesh tempMesh;

    #ifdef OBJL_CONSOLE_OUTPUT
    const unsigned int outputEveryNth = 1000;
    unsigned int outputIndicator = outputEveryNth;
    #endif

    std::string curline;
    while (std::getline(file, curline))
    {
        #ifdef OBJL_CONSOLE_OUTPUT
        if ((outputIndicator = ((outputIndicator + 1) % outputEveryNth)) == 1)
        {
            if (!meshname.empty())
            {
                std::cout
                    << "\r- " << meshname
                    << "\t| vertices > " << Positions.size()
                    << "\t| texcoords > " << TCoords.size()
                    << "\t| normals > " << Normals.size()
                    << "\t| triangles > " << (_vertices.size() / 3)
                    << (!MeshMatNames.empty() ? "\t| material: " + MeshMatNames.back() : "");
            }
        }
        #endif

        // Generate a Mesh Object or Prepare for an object to be created
        if (tools::ft_firstToken(curline) == "o" || tools::ft_firstToken(curline) == "g" || curline[0] == 'g')
        {
            if (!listening)
            {
                listening = true;

                if (tools::ft_firstToken(curline) == "o" || tools::ft_firstToken(curline) == "g")
                    meshname = tools::ft_tail(curline);
                else
                    meshname = "unnamed";
            }
            else
            {
                // Generate the mesh to put into the array

                if (!_indices.empty() && !_vertices.empty())
                {
                    // Create Mesh
                    tempMesh = Mesh(_vertices, _indices);
                    tempMesh._meshName = meshname;

                    // Insert Mesh
                    LoadedMeshes.push_back(tempMesh);

                    // Cleanup
                    _vertices.clear();
                    _indices.clear();
                    meshname.clear();

                    meshname = tools::ft_tail(curline);
                }
                else
                {
                    if (tools::ft_firstToken(curline) == "o" || tools::ft_firstToken(curline) == "g")
                    {
                        meshname = tools::ft_tail(curline);
                    }
                    else
                    {
                        meshname = "unnamed";
                    }
                }
            }
            #ifdef OBJL_CONSOLE_OUTPUT
            std::cout << std::endl;
            outputIndicator = 0;
            #endif
        }
        // Generate a Vertex position
        if (tools::ft_firstToken(curline) == "v")
        {
            std::vector<std::string> spos;
            glm::vec3 vpos;
            tools::ft_split(tools::ft_tail(curline), spos, " ");

            vpos.x = std::stof(spos[0]);
            vpos.y = std::stof(spos[1]);
            vpos.z = std::stof(spos[2]);

            Positions.push_back(vpos);
        }
        // Generate a Vertex Texture Coordinate
        if (tools::ft_firstToken(curline) == "vt")
        {
            std::vector<std::string> stex;
            glm::vec2 vtex;
            tools::ft_split(tools::ft_tail(curline), stex, " ");

            vtex.x = std::stof(stex[0]);
            vtex.y = std::stof(stex[1]);

            TCoords.push_back(vtex);
        }
        // Generate a Vertex normal;
        if (tools::ft_firstToken(curline) == "vn")
        {
            std::vector<std::string> snor;
            glm::vec3 vnor;
            tools::ft_split(tools::ft_tail(curline), snor, " ");

            vnor.x = std::stof(snor[0]);
            vnor.y = std::stof(snor[1]);
            vnor.z = std::stof(snor[2]);

            Normals.push_back(vnor);
        }
        // Generate a Face (vertices & indices)
        if (tools::ft_firstToken(curline) == "f")
        {
            // Generate the vertices
            std::vector<Vertex> vVerts;
            GenVerticesFromRawOBJ(vVerts, Positions, TCoords, Normals, curline);

            // Add _vertices
            for (uint64_t i = 0; i < vVerts.size(); i++)
            {
                _vertices.push_back(vVerts[i]);

                LoadedVertices.push_back(vVerts[i]);
            }

            std::vector<unsigned int> iIndices;

            VertexTriangluation(iIndices, vVerts);

            // Add _indices
            for (uint64_t i = 0; i < iIndices.size(); i++)
            {
                uint64_t indnum = (_vertices.size()) - vVerts.size() + iIndices[i];
                _indices.push_back(indnum);

                indnum = (LoadedVertices.size()) - vVerts.size() + iIndices[i];
                LoadedIndices.push_back(indnum);
            }
        }
        // Get Mesh Material Name
        if (tools::ft_firstToken(curline) == "usemtl")
        {
            MeshMatNames.push_back(tools::ft_tail(curline));

            // Create new Mesh, if Material changes within a group
            if (!_indices.empty() && !_vertices.empty())
            {
                // Create Mesh
                tempMesh = Mesh(_vertices, _indices);
                tempMesh._meshName = meshname;
                int i = 2;
                while(1) {
                    tempMesh._meshName = meshname + "_" + std::to_string(i);

                    for (auto &m : LoadedMeshes)
                        if (m._meshName == tempMesh._meshName)
                            continue;
                    break;
                }

                // Insert Mesh
                LoadedMeshes.push_back(tempMesh);

                // Cleanup
                _vertices.clear();
                _indices.clear();
            }

            #ifdef OBJL_CONSOLE_OUTPUT
            outputIndicator = 0;
            #endif
        }
        // Load Materials
        if (tools::ft_firstToken(curline) == "mtllib")
        {
            // Generate LoadedMaterial

            // Generate a path to the material file
            std::vector<std::string> temp;
            tools::ft_split(Path, temp, "/");

            std::string pathtomat = "";

            if (temp.size() != 1)
                for (int i = 0; i < temp.size() - 1; i++)
                    pathtomat += temp[i] + "/";
            while (!isalpha(curline[curline.size() - 1]))
                curline.pop_back();
            pathtomat += tools::ft_tail(curline);
            #ifdef OBJL_CONSOLE_OUTPUT
            std::cout << std::endl << "- find materials in: " << pathtomat << std::endl;
            #endif
            // Load Materials
            LoadMaterials(pathtomat);
            std::cout << _mtlFile << std::endl;
        }
    }

    #ifdef OBJL_CONSOLE_OUTPUT
    std::cout << std::endl;
    #endif

    // Deal with last mesh
    if (!_indices.empty() && !_vertices.empty())
    { 
        uint64_t    i = 0;
        float       y = 0.f;
        for (Vertex& v : _vertices)
        {
            std::cerr << "i = " << i << std::endl;
            std::cerr << "y = " << y << std::endl;
            if (i % 3 == 0)
                y += .2;
            if (y >= 1.2)
                y = 0.f;
            std::cerr << "y = " << y << std::endl;
            // v.color = {static_cast <float> (rand()) / static_cast <float> (RAND_MAX), static_cast <float> (rand()) / static_cast <float> (RAND_MAX), static_cast <float> (rand()) / static_cast <float> (RAND_MAX)};
            v.color = glm::vec3(y);
            i++;
        }
        // Create Mesh
        tempMesh = Mesh(_vertices, _indices);
        tempMesh._meshName = meshname;

        // Insert Mesh
        LoadedMeshes.push_back(tempMesh);
    }

    file.close();

    // Set Materials for each Mesh
    for (size_t i = 0; i < MeshMatNames.size(); i++)
    {
        std::string matname = MeshMatNames[i];

        // Find corresponding material name in loaded materials
        // when found copy material variables into mesh material
        for (int j = 0; j < LoadedMaterials.size(); j++)
        {
            if (LoadedMaterials[j]._name == matname)
            {
                LoadedMeshes[i]._meshMaterial = LoadedMaterials[j];
                break;
            }
        }
    }

    if (LoadedMeshes.empty() && LoadedVertices.empty() && LoadedIndices.empty())
        return false;
    else
        return true;
}

// Generate vertices from a list of positions, 
//	tcoords, normals and a face line
void    Loader::GenVerticesFromRawOBJ(std::vector<Vertex>& oVerts,
    const std::vector<glm::vec3>& iPositions,
    const std::vector<glm::vec2>& iTCoords,
    const std::vector<glm::vec3>& iNormals,
    std::string icurline)
{
    std::vector<std::string> sface, svert;
    Vertex vVert;

    tools::ft_split(tools::ft_tail(icurline), sface, " ");

    bool noNormal = false;

    // For every given vertex do this
    for (uint64_t i = 0; i < sface.size(); i++)
    {
        // See What type the vertex is.
        int vtype = 0;

        tools::ft_split(sface[i], svert, "/");

        // Check for just position - v1
        if (svert.size() == 1 && isdigit(svert[0][0]))
        {
            // Only position
            vtype = 1;
        }

        // Check for position & texture - v1/vt1
        if (svert.size() == 2)
        {
            // position & Texture
            vtype = 2;
        }

        // Check for position, Texture and normal - v1/vt1/vn1
        // or if position and normal - v1//vn1
        if (svert.size() == 3)
        {
            if (svert[1] != "")
            {
                // position, Texture, and normal
                vtype = 4;
            } 
            else
            {
                // position & normal
                vtype = 3;
            }
        }

        // Calculate and store the vertex
        switch (vtype)
        {
            case 1: // P
            {
                vVert.position = tools::ft_getElement(iPositions, svert[0]);
                vVert.uv = glm::vec2(tools::ft_getElement(iPositions, svert[0]).x, 1.f - tools::ft_getElement(iPositions, svert[0]).y);
                noNormal = true;
                oVerts.push_back(vVert);
                break;
            }
            case 2: // P/T
            {
                vVert.position = tools::ft_getElement(iPositions, svert[0]);
                vVert.uv = glm::vec2(tools::ft_getElement(iTCoords, svert[1]).x, 1.f - tools::ft_getElement(iTCoords, svert[1]).y);
                noNormal = true;
                oVerts.push_back(vVert);
                break;
            }
            case 3: // P//N
            {
                vVert.position = tools::ft_getElement(iPositions, svert[0]);
                vVert.uv = glm::vec2(tools::ft_getElement(iPositions, svert[0]).x, 1.f - tools::ft_getElement(iPositions, svert[0]).y);
                vVert.normal = tools::ft_getElement(iNormals, svert[2]);
                oVerts.push_back(vVert);
                break;
            }
            case 4: // P/T/N
            {
                vVert.position = tools::ft_getElement(iPositions, svert[0]);
                vVert.uv = glm::vec2(tools::ft_getElement(iTCoords, svert[1]).x, 1.f - tools::ft_getElement(iTCoords, svert[1]).y);
                vVert.normal = tools::ft_getElement(iNormals, svert[2]);
                oVerts.push_back(vVert);
                break;
            }
            default:
            {
                break;
            }
        }
    }

    // take care of missing normals
    // these may not be truly acurate but it is the 
    // best they get for not compiling a mesh with normals	
    if (noNormal)
    {
        glm::vec3 A = oVerts[0].position - oVerts[1].position;
        glm::vec3 B = oVerts[2].position - oVerts[1].position;

        glm::vec3 normal = tools::ft_CrossV3(A, B);

        for (size_t i = 0; i < oVerts.size(); i++)
        {
            oVerts[i].normal = -normal;
        }
    }
}

// Triangulate a list of vertices into a face by printing
//	inducies corresponding with triangles within it
void    Loader::VertexTriangluation(std::vector<unsigned int>& oIndices,
    const std::vector<Vertex>& iVerts)
{
    // If there are 2 or less verts,
    // no triangle can be created,
    // so exit
    if (iVerts.size() < 3)
    {
        return;
    }
    // If it is a triangle no need to calculate it
    if (iVerts.size() == 3)
    {
        oIndices.push_back(0);
        oIndices.push_back(1);
        oIndices.push_back(2);
        return;
    }

    // Create a list of vertices
    std::vector<Vertex> tVerts = iVerts;

    while (true)
    {
        // For every vertex
        for (int i = 0; i < int(tVerts.size()); i++)
        {
            // pPrev = the previous vertex in the list
            Vertex pPrev;
            if (i == 0)
                pPrev = tVerts[tVerts.size() - 1];
            else
                pPrev = tVerts[i - 1];

            // pCur = the current vertex;
            Vertex pCur = tVerts[i];

            // pNext = the next vertex in the list
            Vertex pNext;
            if (i == tVerts.size() - 1)
                pNext = tVerts[0];
            else
                pNext = tVerts[i + 1];

            // Check to see if there are only 3 verts left
            // if so this is the last triangle
            if (tVerts.size() == 3)
            {
                // Create a triangle from pCur, pPrev, pNext
                for (int j = 0; j < int(tVerts.size()); j++)
                {
                    if (iVerts[j].position == pCur.position)
                        oIndices.push_back(j);
                    if (iVerts[j].position == pPrev.position)
                        oIndices.push_back(j);
                    if (iVerts[j].position == pNext.position)
                        oIndices.push_back(j);
                }

                tVerts.clear();
                break;
            }
            if (tVerts.size() == 4)
            {
                // Create a triangle from pCur, pPrev, pNext
                for (int j = 0; j < int(iVerts.size()); j++)
                {
                    if (iVerts[j].position == pCur.position)
                        oIndices.push_back(j);
                    if (iVerts[j].position == pPrev.position)
                        oIndices.push_back(j);
                    if (iVerts[j].position == pNext.position)
                        oIndices.push_back(j);
                }

                glm::vec3 tempVec;
                for (int j = 0; j < int(tVerts.size()); j++)
                {
                    if (tVerts[j].position != pCur.position
                        && tVerts[j].position != pPrev.position
                        && tVerts[j].position != pNext.position)
                    {
                        tempVec = tVerts[j].position;
                        break;
                    }
                }

                // Create a triangle from pCur, pPrev, pNext
                for (int j = 0; j < int(iVerts.size()); j++)
                {
                    if (iVerts[j].position == pPrev.position)
                        oIndices.push_back(j);
                    if (iVerts[j].position == pNext.position)
                        oIndices.push_back(j);
                    if (iVerts[j].position == tempVec)
                        oIndices.push_back(j);
                }

                tVerts.clear();
                break;
            }

            // If Vertex is not an interior vertex
            float angle = tools::ft_AngleBetweenV3(pPrev.position - pCur.position, pNext.position - pCur.position) * (180 / 3.14159265359);
            if (angle <= 0 && angle >= 180)
                continue;

            // If any vertices are within this triangle
            bool inTri = false;
            for (int j = 0; j < int(iVerts.size()); j++)
            {
                if (tools::ft_inTriangle(iVerts[j].position, pPrev.position, pCur.position, pNext.position)
                    && iVerts[j].position != pPrev.position
                    && iVerts[j].position != pCur.position
                    && iVerts[j].position != pNext.position)
                {
                    inTri = true;
                    break;
                }
            }
            if (inTri)
                continue;

            // Create a triangle from pCur, pPrev, pNext
            for (int j = 0; j < int(iVerts.size()); j++)
            {
                if (iVerts[j].position == pCur.position)
                    oIndices.push_back(j);
                if (iVerts[j].position == pPrev.position)
                    oIndices.push_back(j);
                if (iVerts[j].position == pNext.position)
                    oIndices.push_back(j);
            }

            // Delete pCur from the list
            for (int j = 0; j < int(tVerts.size()); j++)
            {
                if (tVerts[j].position == pCur.position)
                {
                    tVerts.erase(tVerts.begin() + j);
                    break;
                }
            }

            // reset i to the start
            // -1 since loop will add 1 to it
            i = -1;
        }

        // if no triangles were created
        if (oIndices.size() == 0)
            break;

        // if no more vertices
        if (tVerts.size() == 0)
            break;
    }
}

// Load Materials from .mtl file
bool    Loader::LoadMaterials(std::string path)
{
    // If the file is not a material file return false
    if (path.substr(path.find_last_of('.'), path.size()) != ".mtl")
    {
        std::cerr << "False path: " << path << std::endl;
        return false;
    }

    std::ifstream file(path);

    // If the file is not found return false
    if (!file.is_open())
    {
        std::cerr << "File not open" << std::endl;
        return false;
    }

    Material tempMaterial;

    bool listening = false;

    // Go through each line looking for material variables
    std::string curline;
    while (std::getline(file, curline))
    {
        // new material and material name
        if (tools::ft_firstToken(curline) == "newmtl")
        {
            if (!listening)
            {
                listening = true;

                if (curline.size() > 7)
                {
                    tempMaterial._name = tools::ft_tail(curline);
                }
                else
                {
                    tempMaterial._name = "none";
                }
            }
            else
            {
                // Generate the material

                // Push Back loaded Material
                LoadedMaterials.push_back(tempMaterial);

                // Clear Loaded Material
                tempMaterial = Material();

                if (curline.size() > 7)
                {
                    tempMaterial._name = tools::ft_tail(curline);
                }
                else
                {
                    tempMaterial._name = "none";
                }
            }
        }
        // Ambient Color
        if (tools::ft_firstToken(curline) == "Ka")
        {
            std::vector<std::string> temp;
            tools::ft_split(tools::ft_tail(curline), temp, " ");

            if (temp.size() != 3)
                continue;

            tempMaterial._ka.x = std::stof(temp[0]);
            tempMaterial._ka.y = std::stof(temp[1]);
            tempMaterial._ka.z = std::stof(temp[2]);
        }
        // Diffuse Color
        if (tools::ft_firstToken(curline) == "Kd")
        {
            std::vector<std::string> temp;
            tools::ft_split(tools::ft_tail(curline), temp, " ");

            if (temp.size() != 3)
                continue;

            tempMaterial._kd.x = std::stof(temp[0]);
            tempMaterial._kd.y = std::stof(temp[1]);
            tempMaterial._kd.z = std::stof(temp[2]);
        }
        // Specular Color
        if (tools::ft_firstToken(curline) == "Ks")
        {
            std::vector<std::string> temp;
            tools::ft_split(tools::ft_tail(curline), temp, " ");

            if (temp.size() != 3)
                continue;

            tempMaterial._ks.x = std::stof(temp[0]);
            tempMaterial._ks.y = std::stof(temp[1]);
            tempMaterial._ks.z = std::stof(temp[2]);
        }
        if (tools::ft_firstToken(curline) == "Ke")
        {
            std::vector<std::string> temp;
            tools::ft_split(tools::ft_tail(curline), temp, " ");

            if (temp.size() != 3)
                continue;

            tempMaterial._ke.x = std::stof(temp[0]);
            tempMaterial._ke.y = std::stof(temp[1]);
            tempMaterial._ke.z = std::stof(temp[2]);
        }
        // Specular Exponent
        if (tools::ft_firstToken(curline) == "Ns")
        {
            tempMaterial._ns = std::stof(tools::ft_tail(curline));
        }
        // Optical Density
        if (tools::ft_firstToken(curline) == "Ni")
        {
            tempMaterial._ni = std::stof(tools::ft_tail(curline));
        }
        // Dissolve
        if (tools::ft_firstToken(curline) == "d")
        {
            tempMaterial._d = std::stof(tools::ft_tail(curline));
        }
        // Illumination
        if (tools::ft_firstToken(curline) == "illum")
        {
            tempMaterial._illum = std::stoi(tools::ft_tail(curline));
        }
        // Ambient Texture Map
        if (tools::ft_firstToken(curline) == "map_Ka")
        {
            tempMaterial._mapKa = tools::ft_tail(curline);
        }
        // Diffuse Texture Map
        if (tools::ft_firstToken(curline) == "map_Kd")
        {
            tempMaterial._mapKd = tools::ft_tail(curline);
        }
        // Specular Texture Map
        if (tools::ft_firstToken(curline) == "map_Ks")
        {
            tempMaterial._mapKs = tools::ft_tail(curline);
        }
        // Specular Hightlight Map
        if (tools::ft_firstToken(curline) == "map_Ns")
        {
            tempMaterial._mapNs = tools::ft_tail(curline);
        }
        // Alpha Texture Map
        if (tools::ft_firstToken(curline) == "map_d")
        {
            tempMaterial._mapD = tools::ft_tail(curline);
        }
        // Bump Map
        if (tools::ft_firstToken(curline) == "map_Bump" || tools::ft_firstToken(curline) == "map_bump" || tools::ft_firstToken(curline) == "bump")
        {
            tempMaterial._mapBump = tools::ft_tail(curline);
        }
    }

    // Deal with last material

    // Push Back loaded Material
    LoadedMaterials.push_back(tempMaterial);

    // Test to see if anything was loaded
    // If not return false
    if (LoadedMaterials.empty())
        return false;
    // If so return true
    else
    {
        this->_mtlFile = path;
        return (true);
    }
}

namespace tools {
    // glm::vec3 Cross Product
    glm::vec3 ft_CrossV3(const glm::vec3 a, const glm::vec3 b)
    {
        return glm::vec3(a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x);
    }

    // glm::vec3 Magnitude Calculation
    float ft_MagnitudeV3(const glm::vec3 in)
    {
        return (sqrtf(powf(in.x, 2) + powf(in.y, 2) + powf(in.z, 2)));
    }

    // glm::vec3 DotProduct
    float ft_DotV3(const glm::vec3 a, const glm::vec3 b)
    {
        return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
    }

    // Angle between 2 glm::vec3 Objects
    float ft_AngleBetweenV3(const glm::vec3 a, const glm::vec3 b)
    {
        float angle = tools::ft_DotV3(a, b);
        angle /= (tools::ft_MagnitudeV3(a) * tools::ft_MagnitudeV3(b));
        return angle = acosf(angle);
    }

    // Projection Calculation of a onto b
    glm::vec3 ft_ProjV3(const glm::vec3 a, const glm::vec3 b)
    {
        glm::vec3 bn = b / tools::ft_MagnitudeV3(b);
        return bn * tools::ft_DotV3(a, bn);
    }

    // glm::vec3 Multiplication Opertor Overload
    glm::vec3 operator*(const float& left, const glm::vec3& right)
    {
        return glm::vec3(right.x * left, right.y * left, right.z * left);
    }

    // A test to see if P1 is on the same side as P2 of a line segment ab
    bool ft_SameSide(glm::vec3 p1, glm::vec3 p2, glm::vec3 a, glm::vec3 b)
    {
        glm::vec3 cp1 = tools::ft_CrossV3(b - a, p1 - a);
        glm::vec3 cp2 = tools::ft_CrossV3(b - a, p2 - a);

        if (tools::ft_DotV3(cp1, cp2) >= 0)
            return true;
        else
            return false;
    }

    // Generate a cross produect normal for a triangle
    glm::vec3 ft_GenTriNormal(glm::vec3 t1, glm::vec3 t2, glm::vec3 t3)
    {
        glm::vec3 u = t2 - t1;
        glm::vec3 v = t3 - t1;

        glm::vec3 normal = tools::ft_CrossV3(u,v);

        return normal;
    }

    // Check to see if a glm::vec3 Point is within a 3 glm::vec3 Triangle
    bool ft_inTriangle(glm::vec3 point, glm::vec3 tri1, glm::vec3 tri2, glm::vec3 tri3)
    {
        // Test to see if it is within an infinite prism that the triangle outlines.
        bool within_tri_prisim = tools::ft_SameSide(point, tri1, tri2, tri3) && tools::ft_SameSide(point, tri2, tri1, tri3)
            && tools::ft_SameSide(point, tri3, tri1, tri2);

        // If it isn't it will never be on the triangle
        if (!within_tri_prisim)
            return false;

        // Calulate Triangle's Normal
        glm::vec3 n = tools::ft_GenTriNormal(tri1, tri2, tri3);

        // Project the point onto this normal
        glm::vec3 proj = tools::ft_ProjV3(point, n);

        // If the distance from the triangle to the point is 0
        //	it lies on the triangle
        if (tools::ft_MagnitudeV3(proj) == 0)
            return true;
        else
            return false;
    }

    // Split a String into a string array at a given token
    void    ft_split(const std::string &in, std::vector<std::string> &out, std::string token)
    {
        out.clear();

        std::string temp;

        for (std::size_t i = 0; i < in.size(); i++)
        {
            std::string test = in.substr(i, token.size());

            if (test == token)
            {
                if (!temp.empty())
                {
                    out.push_back(temp);
                    temp.clear();
                    i += token.size() - 1;
                }
                else
                {
                    out.push_back("");
                }
            }
            else if (i + token.size() >= in.size())
            {
                temp += in.substr(i, token.size());
                out.push_back(temp);
                break;
            }
            else
            {
                temp += in[i];
            }
        }
    }

    // Get tail of string after first token and possibly following spaces
    std::string ft_tail(const std::string &in)
    {
        size_t token_start = in.find_first_not_of(" \t");
        size_t space_start = in.find_first_of(" \t", token_start);
        size_t tail_start = in.find_first_not_of(" \t", space_start);
        size_t tail_end = in.find_last_not_of(" \t");
        if (tail_start != std::string::npos && tail_end != std::string::npos)
        {
            return in.substr(tail_start, tail_end - tail_start + 1);
        }
        else if (tail_start != std::string::npos)
        {
            return in.substr(tail_start);
        }
        return "";
    }

    // Get first token of string
    std::string ft_firstToken(const std::string &in)
    {
        if (!in.empty())
        {
            size_t token_start = in.find_first_not_of(" \t");
            size_t token_end = in.find_first_of(" \t", token_start);
            if (token_start != std::string::npos && token_end != std::string::npos)
            {
                return in.substr(token_start, token_end - token_start);
            }
            else if (token_start != std::string::npos)
            {
                return in.substr(token_start);
            }
        }
        return "";
    }

    // Get element at given index position
    template <class T>
    const T & ft_getElement(const std::vector<T> &elements, std::string &index)
    {
        unsigned long idx = 0;
        try {
            idx = std::stoul(index);
        }
        catch (std::exception())
        {
            idx = 0;
        }
        if (idx < 0)
            idx = elements.size() + idx;
        else
            idx--;
        return elements[idx];
    }
}
