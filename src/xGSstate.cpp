#include "xGSstate.h"
#include "xGSgeometrybuffer.h"
#include "xGSutil.h"


xGSstateImpl::xGSstateImpl() :
    p_program(0),
    p_vao(0)
{}

xGSstateImpl::~xGSstateImpl()
{
    if (p_program) {
        glDeleteProgram(p_program);
    }

    if (p_vao) {
        glDeleteVertexArrays(1, &p_vao);
    }
}

static void attachShaders(std::vector<GLuint> &shaders, GLenum type, const char **sources)
{
    if (!sources) {
        return;
    }

    GLuint shader = glCreateShader(type);

    int count = 0;
    const char **s = sources;
    while (*s) {
        glShaderSource(shader, 1, s, nullptr);
        ++s;
    }

    glCompileShader(shader);

    // TODO: get compile status/errors

#ifdef _DEBUG
    char log[4096];
    GLsizei len = 0;
    glGetShaderInfoLog(shader, sizeof(log), &len, log);
    OutputDebugStringA(log);
#endif

    shaders.emplace_back(shader);
}

bool xGSstateImpl::Allocate(const GSstatedesc &desc)
{
    p_program = glCreateProgram();

    std::vector<GLuint> shaders;
    attachShaders(shaders, GL_VERTEX_SHADER, desc.vs);
    attachShaders(shaders, GL_TESS_CONTROL_SHADER, desc.cs);
    attachShaders(shaders, GL_TESS_EVALUATION_SHADER, desc.es);
    attachShaders(shaders, GL_GEOMETRY_SHADER, desc.gs);
    attachShaders(shaders, GL_FRAGMENT_SHADER, desc.ps);

    for (auto &s : shaders) {
        glAttachShader(p_program, s);
        glDeleteShader(s);
    }

    glLinkProgram(p_program);

    GLint status = 0;
    glGetProgramiv(p_program, GL_LINK_STATUS, &status);
    if (status == 0) {
        // TODO: handle error

#ifdef _DEBUG
        char log[4096];
        GLsizei len = 0;
        glGetProgramInfoLog(p_program, sizeof(log), &len, log);
        OutputDebugStringA(log);
#endif
    }

    EnumProgramInputs();

    // find and bind static input
    GSinputslot *slot = desc.input;
    while (slot->type != GS_LAST_SLOT) {
        if (slot->type == GS_STATIC) {
            // TODO: check for only one static slot
            // TODO: ensure buffer is correct and allocated by a GS instance
            xGSgeometrybufferImpl *buffer = static_cast<xGSgeometrybufferImpl*>(slot->buffer);

            glCreateVertexArrays(1, &p_vao);
            glBindVertexArray(p_vao);

            // bind buffers from attached geometry buffer
            glBindBuffer(GL_ARRAY_BUFFER, buffer->vertexBufferId());
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->indexBufferId());

            // set up vertex attrib pointers (according to slot vertex components)
            GSvertexcomponent *comp = slot->decl;
            GLsizei offset = 0;
            while (comp->type != GS_LAST_COMPONENT) {
                // TODO: resolve index/name bindings
                int index = glGetAttribLocation(p_program, comp->name);
                if (index != -1) {
                    glEnableVertexAttribArray(GLuint(index));

                    GLint size = vertexcomponentcount(comp->type);

                    glVertexAttribPointer(
                        index, size, GL_FLOAT, GL_FALSE, offset,
                        reinterpret_cast<void*>(offset)
                    );

                    offset += size * sizeof(float);
                }
                ++comp;
            }
        }

        ++slot;
    }

    return true;
}

void xGSstateImpl::Apply()
{
    glUseProgram(p_program);
    glBindVertexArray(p_vao);
}

void xGSstateImpl::EnumProgramInputs()
{
    GLint activeattrs = 0;
    glGetProgramiv(p_program, GL_ACTIVE_ATTRIBUTES, &activeattrs);

    p_attribs.reserve(activeattrs);


    // NOTE: for debugging
#ifdef _DEBUG
    char buf[4096];
    OutputDebugStringA("Program active attribs:\n");
#endif

    for (GLint n = 0; n < activeattrs; ++n) {
        p_attribs.emplace_back();
        Attrib &attr = p_attribs.back();

        glGetActiveAttrib(
            p_program, n, sizeof(attr.name), &attr.len,
            &attr.size, &attr.type, attr.name
        );

        attr.location = glGetAttribLocation(p_program, attr.name);

        // NOTE: for debugging
#ifdef _DEBUG
        sprintf_s(
            buf,
            "       #%d %s %d %d %d\n",
            n, attr.name, attr.location, attr.size, attr.type
        );
        OutputDebugStringA(buf);
#endif
    }
}

