#include "vtkSaliencyPass.h"
#include "vtkObjectFactory.h"
#include <assert.h>
#include "vtkRenderState.h"
#include "vtkRenderer.h"
#include "vtkgl.h"
#include "vtkFrameBufferObject.h"
#include "vtkTextureObject.h"
#include "vtkShaderProgram2.h"
#include "vtkShader2.h"
#include "vtkShader2Collection.h"
#include "vtkUniformVariables.h"
#include "vtkOpenGLRenderWindow.h"
#include "vtkTextureUnitManager.h"

// to be able to dump intermediate passes into png files for debugging.
// only for vtkSaliencyPass developers.
//#define VTK_GAUSSIAN_BLUR_PASS_DEBUG

#include "vtkPNGWriter.h"
#include "vtkImageImport.h"
#include "vtkPixelBufferObject.h"
#include "vtkPixelBufferObject.h"
#include "vtkImageExtractComponents.h"
#include "vtkCamera.h"
#include "vtkMath.h"

vtkCxxRevisionMacro(vtkSaliencyPass, "$Revision: 1.9 $");
vtkStandardNewMacro(vtkSaliencyPass);

//extern const char *vtkSaliencyPassShader_fs;


#include <stdlib.h>

using namespace std;
static char* readFile(const char *fileName) {
  char* text;

  if (fileName != NULL) {
    FILE *file = fopen(fileName, "rt");

    if (file != NULL) {
      fseek(file, 0, SEEK_END);
      int count = ftell(file);
      rewind(file);

      if (count > 0) {
        text = (char*)malloc(sizeof(char) * (count + 1));
        count = fread(text, sizeof(char), count, file);
        text[count] = '\0';
      }
      fclose(file);
    }
  }
  return text;
}

// ----------------------------------------------------------------------------
vtkSaliencyPass::vtkSaliencyPass()
{
  this->Supported=false;
  this->SupportProbed=false;
  this->isInit = false;
  computeAverages = false;
  actualWeights = new float[3];
}

// ----------------------------------------------------------------------------
vtkSaliencyPass::~vtkSaliencyPass(){}

// ----------------------------------------------------------------------------
void vtkSaliencyPass::init()
{
  if(this->isInit)
    return;

  const GLenum err = glewInit();
  if (GLEW_OK != err)
  {
    // Problem: glewInit failed, something is seriously wrong.
    printf("BufferedQmitkRenderWindow Error: glewInit failed with %s\n", glewGetErrorString(err));
  }

  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  createAuxiliaryTexture(texRender, GENERATE_MIPMAPS | INTERPOLATED | GENERATE_FBO );
  createAuxiliaryTexture(texShaded, GENERATE_MIPMAPS | INTERPOLATED | GENERATE_FBO );
  texShaded->input[0]= glGetUniformLocationARB(texShaded->shader->GetProgramObject(),"Texture0");
  texShaded->input[1]= glGetUniformLocationARB(texShaded->shader->GetProgramObject(),"modelview");
  texShaded->input[2]= glGetUniformLocationARB(texShaded->shader->GetProgramObject(),"projection");

  texShaded->shader= shaderManager.loadfromFile("Distortion.vs", "Distortion.fs");
  //texShaded->shader= shaderManager.loadfromFile("simple.vs", "simple.fs");
  //texShaded->shader= shaderManager.loadfromMemory(0, "void main(void){ gl_FragColor = vec4(1,0,0,1);}");
  
if (0 == texShaded->shader){
    std::cout << glGetString(GL_VERSION) << std::endl;
      std::cout << "Error Loading, compiling or linking shader" << std::endl;
      std::cout << "and that you're RUNNING in the right dir" << std::endl;
      exit(1);
  }
  FramebufferObject::Disable();

  this->isInit = true;

}


// ----------------------------------------------------------------------------
void vtkSaliencyPass::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

void vtkSaliencyPass::showSaliency(const vtkRenderState *s)
{

  assert("pre: s_exists" && s!=0);

  int size[2];
  s->GetWindowSize(size);

  this->NumberOfRenderedProps=0;

  vtkRenderer *r=s->GetRenderer();
  r->GetTiledSizeAndOrigin(
      &this->ViewportWidth,
      &this->ViewportHeight,
      &this->ViewportX,
      &this->ViewportY);

  if(this->DelegatePass!=0)
  {

    int width;
    int height;
    int size[2];
    s->GetWindowSize(size);
    width=size[0];
    height=size[1];
    int w=width;
    int h=height;
    m_height = h;
    m_width = w;
    init();

    if(w != m_old_width && h != m_old_height )
    {
	createAuxiliaryTexture(texRender, GENERATE_MIPMAPS | INTERPOLATED | GENERATE_FBO );
	createAuxiliaryTexture(texShaded, GENERATE_MIPMAPS | INTERPOLATED | GENERATE_FBO, true);
	texShaded->input[0]= glGetUniformLocationARB(texShaded->shader->GetProgramObject(),"Texture0");
	texShaded->input[1]= glGetUniformLocationARB(texShaded->shader->GetProgramObject(),"modelview");
	texShaded->input[2]= glGetUniformLocationARB(texShaded->shader->GetProgramObject(),"projectionview");
      FramebufferObject::Disable();
      m_old_width = w;
      m_old_height = h;
    }

    bool cAverages = true;
    int blurMask = 0.0;
    int modifyFocus = 1.0;
    int filterMethod = 0.0;
    int coherence = 1.0;
    int passes = 2.0;
    float levelsWeight = 1.0f/(passes+2.0f);
    bool showmap = false;


    //render to my fbo
    texRender->fbo->Bind();
    glEnable(GL_DEPTH_TEST);
    this->DelegatePass->Render(s);
    this->NumberOfRenderedProps+=this->DelegatePass->GetNumberOfRenderedProps();
    // AZ: texRender should now contain scene as per VTK pipieline

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D); // THIS IS DEPRECATED in 4.0???
    // shouldn't be needed since it is overriden by shader

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glPushMatrix();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, w, 0, h, 0, -2); 


    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

  //////render
  FramebufferObject::Disable();


  texShaded->shader->begin();
  
  float projection[16];
  float modelview[16];

  glGetFloatv(GL_PROJECTION_MATRIX, projection); 
  glGetFloatv(GL_MODELVIEW_MATRIX, modelview); 

  //let's me access modelviewmatrix within shader
  
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D,  texRender->id);

  glUniformMatrix4fv(texShaded->input[1], sizeof(modelview), GL_FALSE, &modelview[0]);
  glUniformMatrix4fv(texShaded->input[2], sizeof(projection), GL_FALSE, &projection[0]);

//  Check what OpenGL matrices are being given to the shaders...
  std::cout << std::endl;
  std::cout << "projection matrix: ";
  for(int i = 0; i != 16; i++){
      if(0 == (i%4) ) std::cout << std::endl;
      std::cout <<projection[i] << " ";
  }


  glUniform1iARB(texShaded->input[0], 0);        // ????
  texShaded->drawQuad();
  glBindTexture(GL_TEXTURE_2D, 0);

  glEnable(GL_DEPTH_TEST);
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  texShaded->shader->end();

}
  else
  {
    vtkWarningMacro(<<" no delegate.");
  }
}

// ----------------------------------------------------------------------------
// Description:
// Perform rendering according to a render state \p s.
// \pre s_exists: s!=0
void vtkSaliencyPass::Render(const vtkRenderState *s)
{
  showSaliency(s);
}

// ----------------------------------------------------------------------------
// Description:
// Release graphics resources and ask components to release their own
// resources.
// \pre w_exists: w!=0
void vtkSaliencyPass::ReleaseGraphicsResources(vtkWindow *w)
{
  assert("pre: w_exists" && w!=0);

  this->Superclass::ReleaseGraphicsResources(w);

}

void vtkSaliencyPass::createAuxiliaryTexture(TextureInfo *&texCurrent, unsigned char flags, bool resize)
{
  if(resize)
    glDeleteTextures(1,&texCurrent->id);

  if(!resize)
    texCurrent = new TextureInfo;

  // glActiveTexture(GL_TEXTURE0 + 1);

  texCurrent->imgWidth = m_width;
  texCurrent->imgHeight = m_height;
  texCurrent->texWidth = m_width;
  texCurrent->texHeight = m_height;

  texCurrent->format = GL_RGB;
  texCurrent->internalFormat = GL_RGB32F_ARB;
  texCurrent->u0 = (0);
  texCurrent->u1 = (texCurrent->imgWidth / (float)texCurrent->texWidth);

  if (flags&FLIP_Y)
  {
    texCurrent->v1 = (texCurrent->imgHeight / (float)texCurrent->texHeight);
    texCurrent->v0 = (0);
  }
  else
  {
    texCurrent->v0 = (texCurrent->imgHeight / (float)texCurrent->texHeight);
    texCurrent->v1 = (0);
  }

  glEnable(GL_TEXTURE_2D);
  glGenTextures(1, &texCurrent->id);
  glBindTexture(GL_TEXTURE_2D, texCurrent->id);
  glTexImage2D(GL_TEXTURE_2D, 0, texCurrent->internalFormat, texCurrent->texWidth, texCurrent->texHeight, 0,  texCurrent->format, GL_FLOAT, NULL);
  if (flags&GENERATE_MIPMAPS)
  {
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glGenerateMipmapEXT(GL_TEXTURE_2D);
  }
  else if (flags&INTERPOLATED)
  {
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  }
  else 
  {
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  }

  glBindTexture(GL_TEXTURE_2D, 0);
  glDisable(GL_TEXTURE_2D);

  if (flags&GENERATE_FBO)
  {
    if(!resize)
      texCurrent->fbo = new FramebufferObject;
    texCurrent->rbo = new Renderbuffer;
    texCurrent->rbo->Set( GL_DEPTH_COMPONENT24, texCurrent->texWidth, texCurrent->texHeight );
    texCurrent->fbo->Bind();
    texCurrent->fbo->AttachTexture(GL_TEXTURE_2D, texCurrent->id, GL_COLOR_ATTACHMENT0_EXT);
    texCurrent->fbo->AttachRenderBuffer(texCurrent->rbo->GetId(), GL_DEPTH_ATTACHMENT_EXT);
    texCurrent->fbo->IsValid();
  }

}


