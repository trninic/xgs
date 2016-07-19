﻿/*
        xGS 3D Low-level rendering API

    Low-level 3D rendering wrapper API with multiple back-end support

    (c) livingcreative, 2015 - 2016

    https://github.com/livingcreative/xgs

    opengl/win/xGScontextplatform.h
        xGScontext - Windows OpenGL WGL context implementation
*/

#pragma once

#include "xGScontext.h"


namespace xGS
{

    class xGSdefaultcontext;


    class xGScontext : public xGScontextBase
    {
    public:
        xGScontext();
        ~xGScontext();

        GSerror Initialize();
        GSerror CreateRenderer(const GSrendererdescription &desc);
        GSbool DestroyRenderer();

        GSbool Display();

        GSsize RenderTargetSize() const;

    private:
        void setBitsSupport(GSuint &bitflags, int bits)
        {
            switch (bits) {
                case 2: bitflags |= b2; break;
                case 4: bitflags |= b4; break;
                case 8: bitflags |= b8; break;
                case 16: bitflags |= b16; break;
                case 24: bitflags |= b24; break;
                case 32: bitflags |= b32; break;
                case 64: bitflags |= b64; break;
                case 128: bitflags |= b128; break;
            }
        }

        GSbool EnumWGLPixelFormats(xGSdefaultcontext &dc);
        GSbool EnumDefaultPixelFormats(xGSdefaultcontext &dc);
        HGLRC CreateContext(HDC device);

        void CleanUp();

    private:
        GSbool                p_pixelformatWGL;
        GSbool                p_multisample;

        GSint                 p_devicepf;
        PIXELFORMATDESCRIPTOR p_devicepfd;
        HWND                  p_defaultwidget;
        HDC                   p_defaultdevice;
        HWND                  p_renderwidget;
        HDC                   p_renderdevice;
        HGLRC                 p_context;
    };

} // namespace xGS
