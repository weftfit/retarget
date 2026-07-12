// Implementation of the wf_mesh payload declared in ports/mesh.h. This is the
// concrete carrier shared by the core and every adapter — positions plus
// polygonal face topology (ngons permitted), in the canonical Godot frame.
// Generalized from the proven cloth_fit_usd bridge mesh representation.
#include "../ports/mesh.h"

#include <algorithm>
#include <cstddef>
#include <vector>

struct wf_mesh
{
    std::vector<double> xyz;              // 3 * vertex_count
    std::vector<int32_t> face_vertex_counts; // per-face valence
    std::vector<int32_t> face_vertex_indices; // flattened
};

extern "C" {

wf_mesh *wf_mesh_create(void) { return new wf_mesh(); }
void wf_mesh_destroy(wf_mesh *m) { delete m; }

void wf_mesh_set_positions(wf_mesh *m, int32_t vertex_count, const double *xyz)
{
    if (m == nullptr || xyz == nullptr || vertex_count < 0)
    {
        return;
    }
    m->xyz.assign(xyz, xyz + std::size_t(vertex_count) * 3);
}

void wf_mesh_set_faces(wf_mesh *m, int32_t face_count,
                       const int32_t *counts, int32_t index_count,
                       const int32_t *indices)
{
    if (m == nullptr || face_count < 0 || index_count < 0)
    {
        return;
    }
    m->face_vertex_counts.assign(counts, counts + face_count);
    m->face_vertex_indices.assign(indices, indices + index_count);
}

int32_t wf_mesh_vertex_count(const wf_mesh *m)
{
    return m ? int32_t(m->xyz.size() / 3) : 0;
}
void wf_mesh_positions(const wf_mesh *m, double *out)
{
    if (m != nullptr && out != nullptr)
    {
        std::copy(m->xyz.begin(), m->xyz.end(), out);
    }
}
int32_t wf_mesh_face_count(const wf_mesh *m)
{
    return m ? int32_t(m->face_vertex_counts.size()) : 0;
}
int32_t wf_mesh_index_count(const wf_mesh *m)
{
    return m ? int32_t(m->face_vertex_indices.size()) : 0;
}
void wf_mesh_face_vertex_counts(const wf_mesh *m, int32_t *out)
{
    if (m != nullptr && out != nullptr)
    {
        std::copy(m->face_vertex_counts.begin(), m->face_vertex_counts.end(), out);
    }
}
void wf_mesh_face_vertex_indices(const wf_mesh *m, int32_t *out)
{
    if (m != nullptr && out != nullptr)
    {
        std::copy(m->face_vertex_indices.begin(), m->face_vertex_indices.end(), out);
    }
}

} // extern "C"
