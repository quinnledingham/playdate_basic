internal struct Mesh
get_rect_mesh() {
    struct Mesh mesh = { 0 };
    mesh.vertices_count = 4;
    
    mesh.vertices = ARRAY_MALLOC(struct Vertex_XNU, mesh.vertices_count);
    struct Vertex_XNU *vertices = (struct Vertex_XNU *)mesh.vertices;
/*
    mesh.vertices[0] = Vertex_XNU{ {-0.5, -0.5, 0}, {0, 0, 1}, {0, 0} };
    mesh.vertices[1] = Vertex_XNU{ {-0.5,  0.5, 0}, {0, 0, 1}, {0, 1} };
    mesh.vertices[2] = Vertex_XNU{ { 0.5, -0.5, 0}, {0, 0, 1}, {1, 0} };
    mesh.vertices[3] = Vertex_XNU{ { 0.5,  0.5, 0}, {0, 0, 1}, {1, 1} };
*/
    vertices[0] = (struct Vertex_XNU) { {-0.5, -0.5, 0}, {0, 0, -1}, {0, 0} };
    vertices[1] = (struct Vertex_XNU){ {-0.5,  0.5, 0}, {0, 0, -1}, {0, 1} };
    vertices[2] = (struct Vertex_XNU){ { 0.5, -0.5, 0}, {0, 0, -1}, {1, 0} };
    vertices[3] = (struct Vertex_XNU){ { 0.5,  0.5, 0}, {0, 0, -1}, {1, 1} };

    mesh.indices_count = 6;
    mesh.indices = ARRAY_MALLOC(u32, mesh.indices_count);

    // vertex locations
    u32 top_left = 0, top_right = 2, bottom_left = 1, bottom_right = 3;
   
    mesh.indices[0] = top_left;
    mesh.indices[1] = bottom_left;
    mesh.indices[2] = bottom_right;
    mesh.indices[3] = top_left;
    mesh.indices[4] = bottom_right;
    mesh.indices[5] = top_right;
    
/*
    mesh.indices[0] = top_left;
    mesh.indices[1] = bottom_right;
    mesh.indices[2] = bottom_left;
    mesh.indices[3] = top_left;
    mesh.indices[4] = top_right;
    mesh.indices[5] = bottom_right;
*/

    mesh.vertex_info = get_vertex_xnu_info();
    //render_init_mesh(&mesh);

    return mesh;
}