//
// Created by Valerio Santinelli on 19/04/18.
//

#include <string.h>
#include "binocle_gd.h"
#include "binocle_vpct.h"
#include "binocle_material.h"
#include "binocle_viewport_adapter.h"
#include "binocle_shader.h"
#include "binocle_texture.h"

void binocle_gd_gl_check_error(const char *file, unsigned int line, const char *expression) {
    // Get the last error
    GLenum errorCode = glGetError();

    if (errorCode != GL_NO_ERROR) {
        char error[1024] = "Unknown error";
        char description[1024] = "No description";

        // Decode the error code
        switch (errorCode) {
            case GL_INVALID_ENUM: {
                strcpy(error, "GL_INVALID_ENUM");
                strcpy(description, "An unacceptable value has been specified for an enumerated argument.");
                break;
            }

            case GL_INVALID_VALUE: {
                strcpy(error, "GL_INVALID_VALUE");
                strcpy(description, "A numeric argument is out of range.");
                break;
            }

            case GL_INVALID_OPERATION: {
                strcpy(error, "GL_INVALID_OPERATION");
                strcpy(description, "The specified operation is not allowed in the current state.");
                break;
            }

            case GL_STACK_OVERFLOW: {
                strcpy(error, "GL_STACK_OVERFLOW");
                strcpy(description, "This command would cause a stack overflow.");
                break;
            }

            case GL_STACK_UNDERFLOW: {
                strcpy(error, "GL_STACK_UNDERFLOW");
                strcpy(description, "This command would cause a stack underflow.");
                break;
            }

            case GL_OUT_OF_MEMORY: {
                strcpy(error, "GL_OUT_OF_MEMORY");
                strcpy(description, "There is not enough memory left to execute the command.");
                break;
            }

                /*case GLEXT_GL_INVALID_FRAMEBUFFER_OPERATION:
                {
                  error = "GL_INVALID_FRAMEBUFFER_OPERATION";
                  description = "The object bound to FRAMEBUFFER_BINDING is not \"framebuffer complete\".";
                  break;
                }*/
        }

        // Log the error
        SDL_Log("An internal OpenGL call failed in %s (%d).\nExpression:\n%s\nError description:\n %s\n%s", file, line, expression, error, description);
    }
}

binocle_gd binocle_gd_new() {
    binocle_gd res = {};
    return res;
}

void binocle_gd_init(binocle_gd *gd) {
    // Create a new vertex buffer object
    glCheck(glGenBuffers(1, &gd->vbo));
}

void binocle_gd_draw(binocle_gd *gd, const binocle_vpct *vertices, size_t vertex_count, binocle_material material, kmAABB2 viewport) {
    binocle_gd_apply_gl_states();
    binocle_gd_apply_viewport(viewport);
    binocle_gd_apply_blend_mode(material.blend_mode);
    binocle_gd_apply_shader(gd, *material.shader);
    binocle_gd_apply_texture(*material.texture);

    kmMat4 projectionMatrix = binocle_math_create_orthographic_matrix_off_center(0.0f, viewport.max.x, 0.0, viewport.max.y, -1000.0f, 1000.0f);
    //kmMat4 modelViewMatrix = binocle_gd_create_model_view_matrix(0.0f, 0.0f, 1.0f, 0.0f);
    kmMat4 viewMatrix;
    kmMat4Identity(&viewMatrix);
    // TODO: apply the viewport_adapter scale_matrix here
    //kmMat4Scaling(&viewMatrix, 2.0f, 2.0f, 2.0f);
    kmMat4 modelMatrix;
    kmMat4Identity(&modelMatrix);

    glCheck(glEnableVertexAttribArray (gd->vertex_attribute));
    glCheck(glEnableVertexAttribArray (gd->color_attribute));
    glCheck(glEnableVertexAttribArray (gd->tex_coord_attribute));

    glCheck(glBindBuffer(GL_ARRAY_BUFFER, gd->vbo));
    glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(binocle_vpct) * vertex_count, vertices, GL_STATIC_DRAW));

    glCheck(glVertexAttribPointer(gd->vertex_attribute, 2, GL_FLOAT, GL_FALSE, sizeof(binocle_vpct), 0));
    glCheck(glVertexAttribPointer(gd->color_attribute, 4, GL_FLOAT, GL_FALSE, sizeof(binocle_vpct), (void *) (2 * sizeof(GLfloat))));
    glCheck(glVertexAttribPointer(gd->tex_coord_attribute, 2, GL_FLOAT, GL_FALSE, sizeof(binocle_vpct),(void *) (4 * sizeof(GLfloat) + 2 * sizeof(GLfloat))));

    //kmMat4 finalMatrix = Matrix4::mul(state.transform,projectionMatrix);
    //kmMat4 inverseMatrix: Matrix4<f32> = Matrix4::from_nonuniform_scale(1.0, 1.0, 1.0);

    glCheck(glUniformMatrix4fv(gd->projection_matrix_uniform, 1, GL_FALSE, projectionMatrix.mat));
    //glCheck(glUniformMatrix4fv(gd->model_view_matrix_uniform, 1, GL_FALSE, modelViewMatrix.mat));
    glCheck(glUniformMatrix4fv(gd->view_matrix_uniform, 1, GL_FALSE, viewMatrix.mat));
    glCheck(glUniformMatrix4fv(gd->model_matrix_uniform, 1, GL_FALSE, modelMatrix.mat));

    glCheck(glUniform1i(gd->image_uniform, 0));

    glCheck(glDrawArrays(GL_TRIANGLES, 0, vertex_count));

    glCheck(glDisableVertexAttribArray (gd->vertex_attribute));
    glCheck(glDisableVertexAttribArray (gd->color_attribute));
    glCheck(glDisableVertexAttribArray (gd->tex_coord_attribute));
    glCheck(glUseProgram (GL_ZERO));

    glCheck(glBindTexture(GL_TEXTURE_2D, 0));

}

kmMat4 binocle_gd_create_model_view_matrix(float x, float y, float scale, float rotation) {
    float theta = rotation * (float)M_PI / 180.0f;
    float c = cosf(theta);
    float s = sinf(theta);

    kmMat4 m;
    m.mat[0] = c * scale;
    m.mat[1] = s * -scale;
    m.mat[2] = 0.0f;
    m.mat[3] = 0.0f;

    m.mat[4] = s * scale;
    m.mat[5] = c * scale;
    m.mat[6] = 0.0f;
    m.mat[7] = 0.0f;

    m.mat[8] = 0.0f;
    m.mat[9] = 0.0f;
    m.mat[10] = 1.0f;
    m.mat[11] = 0.0f;

    m.mat[12] = x;
    m.mat[13] = y;
    m.mat[14] = 0.0f;
    m.mat[15] = 1.0f;

    return m;
}

void binocle_gd_apply_gl_states() {
    glCheck(glDisable(GL_CULL_FACE));
    glCheck(glDisable(GL_DEPTH_TEST));

    glCheck(glEnable(GL_BLEND));
//    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
//    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
//    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

    glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
}

void binocle_gd_apply_viewport(kmAABB2 viewport) {
    glCheck(glViewport(viewport.min.x, viewport.min.y, viewport.max.x, viewport.max.y));
}

void binocle_gd_apply_blend_mode(const binocle_blend blend_mode) {
    glCheck(glBlendFunc(
            binocle_gd_factor_to_gl_constant(blend_mode.color_src_factor),
            binocle_gd_factor_to_gl_constant(blend_mode.color_dst_factor)));
    glCheck(glBlendEquation(binocle_gd_equation_to_gl_constant(blend_mode.color_equation)));
}

void binocle_gd_apply_shader(binocle_gd *gd, binocle_shader shader) {
    gd->vertex_attribute = glGetAttribLocation(shader.program_id, "vertexPosition");
    gd->tex_coord_attribute = glGetAttribLocation(shader.program_id, "vertexTCoord");
    gd->color_attribute = glGetAttribLocation(shader.program_id, "vertexColor");
    gd->normal_attribute = glGetAttribLocation(shader.program_id, "vertexNormal");
    gd->projection_matrix_uniform = glGetUniformLocation(shader.program_id, "projectionMatrix");
    //gd->model_view_matrix_uniform = glGetUniformLocation(shader.program_id, "modelViewMatrix");
    gd->view_matrix_uniform = glGetUniformLocation(shader.program_id, "viewMatrix");
    gd->model_matrix_uniform = glGetUniformLocation(shader.program_id, "modelMatrix");
    gd->image_uniform = glGetUniformLocation(shader.program_id, "tex0");
    glCheck(glUseProgram(shader.program_id));
}

void binocle_gd_apply_texture(binocle_texture texture) {
    glCheck(glActiveTexture(GL_TEXTURE0));
    glCheck(glBindTexture(GL_TEXTURE_2D, texture.tex_id));
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
}

GLuint binocle_gd_factor_to_gl_constant(binocle_blend_factor blend_factor) {
    switch (blend_factor) {
        case BINOCLE_BLEND_ZERO:
            return GL_ZERO;
        case BINOCLE_BLEND_ONE:
            return GL_ONE;
        case BINOCLE_BLEND_SRCCOLOR:
            return GL_SRC_COLOR;
        case BINOCLE_BLEND_ONEMINUSSRCCOLOR:
            return GL_ONE_MINUS_SRC_COLOR;
        case BINOCLE_BLEND_DSTCOLOR:
            return GL_DST_COLOR;
        case BINOCLE_BLEND_ONEMINUSDSTCOLOR:
            return GL_ONE_MINUS_DST_COLOR;
        case BINOCLE_BLEND_SRCALPHA:
            return GL_SRC_ALPHA;
        case BINOCLE_BLEND_ONEMINUSSRCALPHA:
            return GL_ONE_MINUS_SRC_ALPHA;
        case BINOCLE_BLEND_DSTALPHA:
            return GL_DST_ALPHA;
        case BINOCLE_BLEND_ONEMINUSDSTALPHA:
            return GL_ONE_MINUS_DST_ALPHA;
    }

    SDL_Log("Invalid value for Binocle::BlendMode::Factor! Fallback to Binocle::BlendMode::Zero.");
    return GL_ZERO;
}

GLuint binocle_gd_equation_to_gl_constant(binocle_blend_equation blend_equation) {
    switch (blend_equation) {
        case BINOCLE_BLEND_ADD:
            return GL_FUNC_ADD;
        case BINOCLE_BLEND_SUBTRACT:
            return GL_FUNC_SUBTRACT;
    }

    SDL_Log("Invalid value for Binocle::BlendMode::Equation! Fallback to Binocle::BlendMode::Add.");
    return GL_FUNC_ADD;
}
