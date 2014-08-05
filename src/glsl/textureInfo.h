#ifndef TEXTURE_INFO
#define TEXTURE_INFO

#include "glsl/glsl.h"
#include "fbo/framebufferObject.h"
#include "fbo/glErrorUtil.h"
#include "fbo/renderbuffer.h"

struct TextureInfo
{
  int imgWidth, imgHeight;
  int texWidth, texHeight;
  GLint format;
  GLuint id;
  GLuint internalFormat;
  int u0,u1;
  int v0,v1;
  FramebufferObject* fbo;
  Renderbuffer* rbo;
  cwc::glShader * shader;
  GLint input[32];

  void drawQuad()
  {
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex3f(-1, -1, 0);
    glTexCoord2f(1, 0); glVertex3f( 1, -1, 0);
    glTexCoord2f(1, 1); glVertex3f( 1,  1, 0);
    glTexCoord2f(0, 1); glVertex3f(-1,  1, 0);
    glEnd();
  }
};

#endif //TEXTURE_INFO