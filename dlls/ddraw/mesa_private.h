/* MESA private include file
 * Copyright (c) 1998 Lionel ULMER
 *
 * This file contains all structures that are not exported
 * through d3d.h and all common macros.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __GRAPHICS_WINE_MESA_PRIVATE_H
#define __GRAPHICS_WINE_MESA_PRIVATE_H

#include "d3d_private.h"

#ifdef HAVE_OPENGL

#undef APIENTRY
#undef CALLBACK
#undef WINAPI

#define XMD_H /* This is to prevent the Xmd.h inclusion bug :-/ */
#include <GL/gl.h>
#include <GL/glx.h>
#ifdef HAVE_GL_GLEXT_H
# include <GL/glext.h>
#endif
#undef  XMD_H

#undef APIENTRY
#undef CALLBACK
#undef WINAPI

/* Redefines the constants */
#define CALLBACK    __stdcall
#define WINAPI      __stdcall
#define APIENTRY    WINAPI

/* X11 locking */

extern void (*wine_tsx11_lock_ptr)(void);
extern void (*wine_tsx11_unlock_ptr)(void);

/* As GLX relies on X, this is needed */
#define ENTER_GL() wine_tsx11_lock_ptr()
#define LEAVE_GL() wine_tsx11_unlock_ptr()

extern const GUID IID_D3DDEVICE_OpenGL;

typedef struct render_state {
    /* This is used for the device mode */
    GLenum src, dst;
    /* This is used for textures */
    GLenum mag, min;

    /* This is needed for the Alpha stuff */
    GLenum alpha_func;
    GLclampf alpha_ref;
    BOOLEAN alpha_blend_enable;

    /* This is needed for the stencil stuff */
    GLint stencil_ref;
    GLuint stencil_mask;
    GLenum stencil_func;
    BOOLEAN stencil_enable;
    GLenum stencil_fail, stencil_zfail, stencil_pass;
  
    /* This is needed for proper lighting */
    BOOLEAN lighting_enable, specular_enable;
    D3DMATERIALCOLORSOURCE color_diffuse, color_specular, color_ambient, color_emissive;

    /* This is needed to re-enable fogging when XYZRHW and XYZ primitives are mixed */
    BOOLEAN fog_on;
} RenderState;

typedef struct IDirect3DGLImpl
{
    struct IDirect3DImpl parent;
    DWORD free_lights;
    void (*light_released)(IDirect3DImpl *, GLenum light_num);
} IDirect3DGLImpl;

typedef struct IDirect3DLightGLImpl
{
    struct IDirect3DLightImpl parent;
    GLenum light_num;
} IDirect3DLightGLImpl;

/* This structure is used for the 'private' field of the IDirectDrawSurfaceImpl structure */
typedef struct IDirect3DTextureGLImpl
{
    GLuint tex_name;
    BOOLEAN loaded; /* For the moment, this is here.. Should be part of surface management though */

    /* Texture upload management */
    BOOLEAN initial_upload_done;
    BOOLEAN dirty_flag;

    /* This is for now used to override 'standard' surface stuff to be as transparent as possible */
    void (*final_release)(struct IDirectDrawSurfaceImpl *This);
    void (*lock_update)(IDirectDrawSurfaceImpl* This, LPCRECT pRect, DWORD dwFlags);
    void (*unlock_update)(IDirectDrawSurfaceImpl* This, LPCRECT pRect);
    void (*set_palette)(IDirectDrawSurfaceImpl* This, IDirectDrawPaletteImpl* pal);
} IDirect3DTextureGLImpl;

typedef enum {
    GL_TRANSFORM_NONE = 0,
    GL_TRANSFORM_ORTHO,
    GL_TRANSFORM_NORMAL,
    GL_TRANSFORM_VERTEXBUFFER
} GL_TRANSFORM_STATE;

typedef struct IDirect3DDeviceGLImpl
{
    struct IDirect3DDeviceImpl parent;
    
    GLXContext gl_context;

    /* The current render state */
    RenderState render_state;

    /* The last type of vertex drawn */
    GL_TRANSFORM_STATE transform_state;

    Display  *display;
    Drawable drawable;
} IDirect3DDeviceGLImpl;

/* This is for the OpenGL additions... */
typedef struct {
    struct IDirect3DVertexBufferImpl parent;

    DWORD dwVertexTypeDesc;
    D3DMATRIX world_mat, view_mat, proj_mat;
    LPVOID vertices;
} IDirect3DVertexBufferGLImpl;

/* All non-static functions 'exported' by various sub-objects */
extern HRESULT direct3d_create(IDirect3DImpl **obj, IDirectDrawImpl *ddraw);
extern HRESULT d3dtexture_create(IDirect3DImpl *d3d, IDirectDrawSurfaceImpl *surf, BOOLEAN at_creation, IDirectDrawSurfaceImpl *main_surf);
extern HRESULT d3dlight_create(IDirect3DLightImpl **obj, IDirect3DImpl *d3d, GLenum light_num);
extern HRESULT d3dexecutebuffer_create(IDirect3DExecuteBufferImpl **obj, IDirect3DImpl *d3d, IDirect3DDeviceImpl *d3ddev, LPD3DEXECUTEBUFFERDESC lpDesc);
extern HRESULT d3dmaterial_create(IDirect3DMaterialImpl **obj, IDirect3DImpl *d3d);
extern HRESULT d3dviewport_create(IDirect3DViewportImpl **obj, IDirect3DImpl *d3d);
extern HRESULT d3dvertexbuffer_create(IDirect3DVertexBufferImpl **obj, IDirect3DImpl *d3d, LPD3DVERTEXBUFFERDESC lpD3DVertBufDesc, DWORD dwFlags);
extern HRESULT d3ddevice_create(IDirect3DDeviceImpl **obj, IDirect3DImpl *d3d, IDirectDrawSurfaceImpl *surface);

/* Used for Direct3D to request the device to enumerate itself */
extern HRESULT d3ddevice_enumerate(LPD3DENUMDEVICESCALLBACK cb, LPVOID context) ;
extern HRESULT d3ddevice_enumerate7(LPD3DENUMDEVICESCALLBACK7 cb, LPVOID context) ;
extern HRESULT d3ddevice_find(IDirect3DImpl *d3d, LPD3DFINDDEVICESEARCH lpD3DDFS, LPD3DFINDDEVICERESULT lplpD3DDevice);

/* Used to upload the texture */
extern HRESULT gltex_upload_texture(IDirectDrawSurfaceImpl *This) ;

/* Used to set-up our orthographic projection */
extern void d3ddevice_set_ortho(IDirect3DDeviceImpl *This) ;

/* Common functions defined in d3dcommon.c */
void set_render_state(IDirect3DDeviceGLImpl* This,
		      D3DRENDERSTATETYPE dwRenderStateType, DWORD dwRenderState);
void store_render_state(D3DRENDERSTATETYPE dwRenderStateType, DWORD dwRenderState,
		        STATEBLOCK* lpStateBlock);
void get_render_state(D3DRENDERSTATETYPE dwRenderStateType, LPDWORD lpdwRenderState,
		      STATEBLOCK* lpStateBlock);
void apply_render_state(IDirect3DDeviceGLImpl* This, STATEBLOCK* lpStateBlock);

/* This structure contains all the function pointers to OpenGL extensions
   that are used by Wine */
typedef struct {
  void (*ptr_ColorTableEXT) (GLenum target, GLenum internalformat,
			     GLsizei width, GLenum format, GLenum type, const GLvoid *table);
} Mesa_DeviceCapabilities;

#endif /* HAVE_OPENGL */

#endif /* __GRAPHICS_WINE_MESA_PRIVATE_H */
