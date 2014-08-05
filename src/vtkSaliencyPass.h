#ifndef __vtkSaliencyPass_h
#define __vtkSaliencyPass_h

#include <GL/glew.h>
#include "vtkImageProcessingPass.h"

#include "glsl/glsl.h"
#include "fbo/glErrorUtil.h"
#include <fbo/framebufferObject.h>
#include <fbo/renderbuffer.h>
//#include <mitkStandaloneDataStorage.h>

//since not everything necessary is implemented in VTK we have to bypass the VTK textureObject
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


#define ONE_DIMENSIONAL (1 << 0) /* 0x01 */
#define GENERATE_MIPMAPS (1 << 1) /* 0x01 */
#define INTERPOLATED (1 << 2) /* 0x01 */
#define GENERATE_FBO (1 << 3) /* 0x01 */
#define FLIP_Y (1 << 4) /* 0x01 */

class vtkOpenGLRenderWindow;
class vtkDepthPeelingPassLayerList; // Pimpl
class vtkShaderProgram2;
class vtkShader2;
class vtkFrameBufferObject;
class vtkTextureObject;

class vtkSaliencyPass : public vtkImageProcessingPass
{
public:
  static vtkSaliencyPass *New();
  vtkTypeRevisionMacro(vtkSaliencyPass,vtkImageProcessingPass);
  void PrintSelf(ostream& os, vtkIndent indent);

  //BTX
  // Description:
  // Perform rendering according to a render state \p s.
  // \pre s_exists: s!=0
  virtual void Render(const vtkRenderState *s);
  //ETX

  // Description:
  // Release graphics resources and ask components to release their own
  // resources.
  // \pre w_exists: w!=0
  void ReleaseGraphicsResources(vtkWindow *w);

protected:
  // Description:
  // Default constructor. DelegatePass is set to NULL.
  vtkSaliencyPass();

  // Description:
  // Destructor.
  virtual ~vtkSaliencyPass();

  bool Supported;
  bool SupportProbed;

private:
  vtkSaliencyPass(const vtkSaliencyPass&);  // Not implemented.
  void operator=(const vtkSaliencyPass&);  // Not implemented.

  void createAuxiliaryTexture(TextureInfo *&texCurrent, unsigned char flags, bool resize = false);
  void showSaliency(const vtkRenderState *s);
  void init();

  TextureInfo* texRender;
  TextureInfo* texConspicuityMaps;

  TextureInfo* texFeatureMaps;
  TextureInfo* texShowMask;
  TextureInfo* texShowConspicuity;
  TextureInfo* texShowSaliency;
  TextureInfo* texFinal;
  float* weightsBuffer;
  float* saliencyValuesBuffer;
  TextureInfo* texInput;

  int m_width,m_height;
  int m_old_width,m_old_height;

  cwc::glShaderManager shaderManager;
  bool isInit;
  bool computeAverages;
  float* actualWeights;
  int ViewportX;
  int ViewportY;
  int ViewportWidth;
  int ViewportHeight;
};

#endif
