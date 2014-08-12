/*

PICCANTE
The hottest HDR imaging library!
http://vcg.isti.cnr.it/piccante

Copyright (C) 2014
Visual Computing Laboratory - ISTI CNR
http://vcg.isti.cnr.it
First author: Francesco Banterle

PICCANTE is free software; you can redistribute it and/or modify
under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation; either version 3.0 of
the License, or (at your option) any later version.

PICCANTE is distributed in the hope that it will be useful, but
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Lesser General Public License
( http://www.gnu.org/licenses/lgpl-3.0.html ) for more details.

*/

#ifndef PIC_UTIL_GL_FBO_HPP
#define PIC_UTIL_GL_FBO_HPP

#include <iostream>

namespace pic {

using namespace std;

void CheckFboStatus(GLenum fboStatus)
{
    switch(fboStatus) {
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        cerr << "FBO Incomplete: Attachment" << endl;
        break;

    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        cerr << "FBO Incomplete: Missing Attachment" << endl;
        break;

    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
        cerr << "FBO Incomplete: Dimensions" << endl;
        break;

    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
        cerr << "FBO Incomplete: Formats" << endl;
        break;

    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        cerr << "FBO Incomplete: Draw Buffer" << endl;
        break;

    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        cerr << "FBO Incomplete: Read Buffer" << endl;
        break;

    default:
        cerr << "Undefined FBO error" << endl;
        break;
    }
};

class Fbo
{
public:
    GLuint fbo;           // framebuffer object
    GLuint tex;           // we render to this texture
    GLuint depth;         // and to this depth buffer
    int width, height, frames;             // width and height of the framebuffer
    bool bDepth;

    Fbo();

    //all this functions must be called with an active OpenGL context!
    bool create(int width, int height);
    bool create(int width, int height, bool bDepth);
    bool create(int width, int height, int depth, bool bDepth, GLuint tex);

    void attachColorBuffer(GLuint tex, GLenum target, int slice = 0);

    void attachColorBuffer2(GLuint tex, GLenum target, int slice)
    {
        GLuint texWork = (tex == 0) ? this->tex : tex;

        switch(target) {
        case GL_TEXTURE_2D: {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                   texWork, 0);
        }
        break;

        case GL_TEXTURE_2D_ARRAY: {
            glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D_ARRAY, texWork, 0, slice);
        }
        break;

        case GL_TEXTURE_3D: {
            glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D,
                                   texWork, 0, slice);
        }
        break;
        }
    };

    void attachColorBuffer(GLuint tex);

    bool destroy();
    bool resize(int width, int height)
    {
        destroy();
        return create(width, height);
    }

    void bind();
    void bind(unsigned int i);
    void unbind();

    void bind2()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    }
    void unbind2()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    Fbo *clone();
};

Fbo::Fbo()
{
    depth = 0;
    fbo = 0;
    tex = 0;

    width = height = frames = 0;
};

Fbo *Fbo::clone()
{
    Fbo *ret = new Fbo();
    ret->create(width, height, frames, bDepth, 0);
    return ret;
};

bool Fbo::create(int width, int height)
{
    return create(width, height, false);
};

bool Fbo::create(int width, int height, bool bDepth)
{
    return create(width, height, 1, bDepth, 0);
};

bool Fbo::create(int width, int height, int frames, bool bDepth, GLuint tex)
{
    this->width = width;
    this->height = height;
    this->frames = frames;
    this->bDepth = bDepth;

    // FBO with one COLOR ATTACHMENT
    // setup texture (render target)
    if(tex == 0) {
        glGenTextures(1, &this->tex);

        if(frames == 1) {
            glBindTexture(GL_TEXTURE_2D, this->tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT,
                         0);
            glBindTexture(GL_TEXTURE_2D, 0);
        } else {
            glBindTexture(GL_TEXTURE_3D, this->tex);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, width, height, frames, 0, GL_RGBA,
                         GL_FLOAT, 0);
            glBindTexture(GL_TEXTURE_3D, 0);
        }
    } else {
        this->tex = tex;
    }

    // setup renderbuffer (depth buffer)
    //assert(glGenRenderbuffers);
    if(bDepth) {
        glGenRenderbuffers(1, &depth);
        glBindRenderbuffer(GL_RENDERBUFFER, depth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

    // setup FBO
    if(fbo == 0) {
        glGenFramebuffers(1, &fbo);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // attach color buffer (texture)
    if(frames == 1) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               this->tex, 0);
    } else {
        glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D,
                               this->tex, 0, 0);
    }

    // attach depth buffer (renderbuffer)
    if(bDepth) {
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                                  depth);
    }

#ifdef DEBUG_GL
    // check framebuffer status
    GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if(fboStatus != GL_FRAMEBUFFER_COMPLETE) {
        CheckFboStatus(fboStatus);
        glDeleteFramebuffers(1, &fbo);
        fbo = 0;
    }

#endif

    // unbind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return fbo != 0;
};

//attach a texture in the Fbo as color buffer
void Fbo::attachColorBuffer(GLuint tex, GLenum target, int slice)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLuint texWork = (tex == 0) ? this->tex : tex;

    switch(target) {
    case GL_TEXTURE_2D: {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               texWork, 0);
    }
    break;

    case GL_TEXTURE_3D: {
        glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D,
                               texWork, 0, slice);
    }
    break;
    }

#ifdef DEBUG_GL
    // check framebuffer status
    GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if(fboStatus != GL_FRAMEBUFFER_COMPLETE) {
        CheckFboStatus(fboStatus);
        glDeleteFramebuffers(1, &fbo);
        fbo = 0;
    }

#endif

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
};

void Fbo::attachColorBuffer(GLuint tex)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    if(tex == 0) {
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, this->tex, 0);
    } else {
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0);
    }

#ifdef DEBUG_GL
    // check framebuffer status
    GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if(fboStatus != GL_FRAMEBUFFER_COMPLETE) {
        CheckFboStatus(fboStatus);
        glDeleteFramebuffers(1, &fbo);
        fbo = 0;
    }

#endif

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
};

//Scattering
//

bool Fbo::destroy()
{
    if(tex != 0) {
        glDeleteTextures(1, &tex);
        tex = 0;
    }

    if(depth != 0) {
        glDeleteRenderbuffers(1, &depth);
        depth = 0;
    }

    if(fbo != 0) {
        glDeleteFramebuffers(1, &fbo);
        fbo = 0;
    }

    return true;
};

void Fbo::bind()
{
    if(!fbo) {
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
};

void Fbo::unbind()
{
    if(!fbo) {
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDrawBuffer(GL_BACK);
    glReadBuffer(GL_BACK);
};

} // end namespace pic

#endif /* PIC_UTIL_GL_FBO_HPP */

