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

#ifndef PIC_GL_FILTERING_FILTER_CHANNEL_HPP
#define PIC_GL_FILTERING_FILTER_CHANNEL_HPP

#include "filtering/filter_luminance.hpp"
#include "gl/filtering/filter.hpp"

namespace pic {

/**
 * @brief The FilterGLChannel class
 */
class FilterGLChannel: public FilterGL
{
protected:
    /**
     * @brief InitShaders
     */
    void InitShaders();

    int channel;

    /**
     * @brief setChannel
     * @param channel
     */
    void setChannel(int channel)
    {
        if(channel > -1) {
            this->channel = channel;
        } else {
            this->channel = 0;
        }
    }

public:

    /**
     * @brief FilterGLChannel
     * @param channel
     */
    FilterGLChannel(int channel);

    /**
     * @brief Update
     * @param channel
     */
    void Update(int channel);

    /**
     * @brief Process
     * @param imgIn
     * @param imgOut
     * @return
     */
    ImageRAWGL *Process(ImageRAWGLVec imgIn, ImageRAWGL *imgOut);

    /**
     * @brief Execute
     * @param imgIn
     * @param imgOut
     * @param channel
     * @return
     */
    static ImageRAW *Execute(ImageRAWGL *imgIn, ImageRAWGL *imgOut, int channel = 0)
    {
        FilterGLChannel flt(channel);
        return flt.Process(SingleGL(imgIn), imgOut);
    }

    /**
     * @brief Test
     */
    static void Test()
    {
        ImageRAWGL imgIn(1, 512, 512, 3, IMG_GPU_CPU, GL_TEXTURE_2D);

        for(int i=0;i<imgIn.size();i+=3) {
            imgIn.data[i    ] = 1.0f;
            imgIn.data[i + 1] = 0.5f;
            imgIn.data[i + 2] = 0.25f;
        }

        imgIn.generateTexture2DGL(false);

        FilterGLChannel filter(0);
        ImageRAWGL *outR = filter.Process(SingleGL(&imgIn), NULL);

        filter.Update(1);
        ImageRAWGL *outG = filter.Process(SingleGL(&imgIn), NULL);

        filter.Update(2);
        ImageRAWGL *outB = filter.Process(SingleGL(&imgIn), NULL);

        outR->loadToMemory();
        outR->Write("channel_R.pfm");
        outG->loadToMemory();
        outG->Write("channel_G.pfm");
        outB->loadToMemory();
        outB->Write("channel_B.pfm");
    }
};

FilterGLChannel::FilterGLChannel(int channel) : FilterGL()
{
    setChannel(channel);

    InitShaders();
}

void FilterGLChannel::InitShaders()
{
    fragment_source = GLW_STRINGFY
                      (
    uniform sampler2D u_tex; \n
    uniform int channel; \n
    out     vec4 f_color; \n
    \n
    void main(void) {
        \n
        ivec2 coords = ivec2(gl_FragCoord.xy); \n
        vec3 color = texelFetch(u_tex, coords, 0).xyz; \n
        float output = color[channel]; \n
        f_color = vec4(output, output, output, 1.0); \n
    }
                      );

    filteringProgram.setup(glw::version("330"), vertex_source, fragment_source);

#ifdef PIC_DEBUG
    printf("[FilterGLChannel log]\n%s\n", filteringProgram.log().c_str());
#endif

    glw::bind_program(filteringProgram);
    filteringProgram.attribute_source("a_position", 0);
    filteringProgram.fragment_target("f_color", 0);
    filteringProgram.relink();
    glw::bind_program(0);

    Update(channel);
}

void FilterGLChannel::Update(int channel)
{
    setChannel(channel);

    glw::bind_program(filteringProgram);
    filteringProgram.uniform("u_tex", 0);
    filteringProgram.uniform("channel", channel);
    glw::bind_program(0);
}

ImageRAWGL *FilterGLChannel::Process(ImageRAWGLVec imgIn, ImageRAWGL *imgOut)
{
    if(imgIn.empty()) {
        return imgOut;
    }

    if(imgIn[0] == NULL) {
        return imgOut;
    }

    if(imgIn[0]->channels != 3) {
        return imgOut;
    }

    int w = imgIn[0]->width;
    int h = imgIn[0]->height;

    if(imgOut == NULL) {
        imgOut = new ImageRAWGL(1, w, h, 1, IMG_GPU, GL_TEXTURE_2D);
    }

    if(fbo == NULL) {
        fbo = new Fbo();
    }

    fbo->create(w, h, 1, false, imgOut->getTexture());

    //Rendering
    fbo->bind();
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);

    //Shaders
    glw::bind_program(filteringProgram);

    //Textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, imgIn[0]->getTexture());

    //Rendering aligned quad
    quad->Render();

    //Fbo
    fbo->unbind();

    //Shaders
    glw::bind_program(0);

    //Textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    return imgOut;
}

} // end namespace pic

#endif /* PIC_GL_FILTERING_FILTER_CHANNEL_HPP */

