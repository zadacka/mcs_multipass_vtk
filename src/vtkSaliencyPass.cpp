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

bool verbose = false;

void check_uniforms(bool enabled){
    // if(enabled){  
    // 	// check that uniform locations were set
    // 	for(int i = 0; i != 4; i++){
    // 	    if(-1 == texShaded->input[i]){
    // 		std::cout << "Failed to set uniform location for input[" 
    // 			  << i << "]. "<< std::endl;
    // 		exit(1);
    // 	    }
    // 	}
    // }
    return;
}

void checkerror(){
    GLenum err;
    if ((err = glGetError()) != GL_NO_ERROR) {
        cerr << "OpenGL error: " << err << endl;
	
	std::string error;
	switch(err) {
	case GL_INVALID_OPERATION:      error="INVALID_OPERATION";      break;
	case GL_INVALID_ENUM:           error="INVALID_ENUM";           break;
	case GL_INVALID_VALUE:          error="INVALID_VALUE";          break;
	case GL_OUT_OF_MEMORY:          error="OUT_OF_MEMORY";          break;
	case GL_INVALID_FRAMEBUFFER_OPERATION:  error="INVALID_FRAMEBUFFER_OPERATION";  break;
	}
 
	std::cout << "GL_" << error.c_str() << std::endl;
	exit(1);
    }
    return;
}

void printMatrix(float matrix[16], std::string name = "matrix"){
    std::cout << std::endl;
    std::cout << name;
    for(int i = 0; i != 16; i++){
	if(0 == (i%4) ) std::cout << std::endl;
	std::cout << matrix[i] << " ";
    }
    std::cout << std::endl;
}

vtkCxxRevisionMacro(vtkSaliencyPass, "$Revision: 1.9 $");
vtkStandardNewMacro(vtkSaliencyPass);

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

    createAuxiliaryTexture(
	texRender, GENERATE_MIPMAPS | INTERPOLATED | GENERATE_FBO );
    createAuxiliaryTexture(
	texShaded, GENERATE_MIPMAPS | INTERPOLATED | GENERATE_FBO );

    // Gotta read and compile in order to set up the uniforms!
    texShaded->shader = shaderManager.loadfromFile((char*) "Distortion.vs", (char*) "Distortion.fs");

    texShaded->input[0]= glGetUniformLocation(
	texShaded->shader->GetProgramObject(),"Texture0");
    texShaded->input[1]= glGetUniformLocation(
	texShaded->shader->GetProgramObject(),"modelview");
    texShaded->input[2]= glGetUniformLocation(
	texShaded->shader->GetProgramObject(),"projection");
    texShaded->input[3]= glGetUniformLocation(
	texShaded->shader->GetProgramObject(),"offset");

    check_uniforms(false);

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
    // this is the same as the viewport size (and I'm not exactly sure why...)

    this->NumberOfRenderedProps=0;

    vtkRenderer *r=s->GetRenderer();
    r->GetTiledSizeAndOrigin(
	&this->ViewportWidth,
	&this->ViewportHeight,
	&this->ViewportX,
	&this->ViewportY);

    if(verbose){
	GLint m_viewport[4];
	glGetIntegerv( GL_VIEWPORT, m_viewport );
	cout << "Screen size is " 
	     << m_viewport[0] << ", "  
	     << m_viewport[1] << ", "  
	     << m_viewport[2] << ", "  
	     << m_viewport[3] << ", "  << endl;


	cout << "  w: " << size[0] 
	     << ", h: " << size[1] 
	     << ", vw:" << this->ViewportWidth 
	     << ", vh:" << this->ViewportHeight
	     << ", vx:" << this->ViewportX
	     << ", vy:" << this->ViewportY
	     << endl;
    }
  
    if(this->DelegatePass!=0)
    {

	int width;
	int height;
	int size[2];
	s->GetWindowSize(size);
	width  = size[0];
	height = size[1];
	int w = width;
	int h = height;
	m_height = h;
	m_width = w;
	init();
        
	if(w != m_old_width && h != m_old_height )
	{
	    createAuxiliaryTexture(
		texRender, GENERATE_MIPMAPS | INTERPOLATED | GENERATE_FBO );
	    createAuxiliaryTexture(
		texShaded, GENERATE_MIPMAPS | INTERPOLATED | GENERATE_FBO, true);

	    texShaded->input[0]= glGetUniformLocation(
		texShaded->shader->GetProgramObject(),"Texture0");
	    texShaded->input[1]= glGetUniformLocation(
		texShaded->shader->GetProgramObject(),"modelview");
	    texShaded->input[2]= glGetUniformLocation(
		texShaded->shader->GetProgramObject(),"projection");
	    texShaded->input[3]= glGetUniformLocation(
		texShaded->shader->GetProgramObject(),"offset");

	    int texwidth;
	    glGetTexLevelParameteriv(
		GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texwidth);
	    cout << "GL_TEXTURE_2D Width: " << texwidth << endl;

	    // check that uniform locations are *still* okay (caught a bug with this)
	    check_uniforms(false);

	    FramebufferObject::Disable();
	    m_old_width = w;
	    m_old_height = h;
	}
////////////////////////////////////////////////////////////////////////////////////
// Test
////////////////////////////////////////////////////////////////////////////////////

	// glEnable(GL_DEPTH_TEST); // clears the depth buffer, will draw anything closer
	// FramebufferObject::Disable();
	// this->DelegatePass->Render(s);


/////////////////////////////////////////////////////////////////////////////////////
// Custom rendering Shiz
////////////////////////////////////////////////////////////////////////////////////

	//render to my fbo 
	texRender->fbo->Bind();
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); // stops junk showing up

	glEnable(GL_DEPTH_TEST);
	this->DelegatePass->Render(s);
	// this->NumberOfRenderedProps+=this->DelegatePass->GetNumberOfRenderedProps();
	// // AZ: texRender should now contain scene as per VTK pipieline
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	// glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	FramebufferObject::Disable();

	// get matrices ready for vertex shader
	glMatrixMode(GL_MODELVIEW);  // switch to MODELVIEW
	glPushMatrix();              // push current to MODELVIEW stack
	glLoadIdentity();            // load something innocuous
	GLfloat modelview[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, modelview); 

	glMatrixMode(GL_PROJECTION); // switch to PROJECTION
	glPushMatrix();              // push current to PROJECTION stack
	glLoadIdentity();            // load something innocuous
	GLfloat projection[16];
	glGetFloatv(GL_PROJECTION_MATRIX, projection); 

	GLfloat offset[2] = {(GLfloat)ViewportX, (GLfloat) ViewportY};

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,  texRender->id);

	texShaded->shader->begin();    // does program->use, enables uniforms

	// // set all uniforms
	glUniform1i(texShaded->input[0], 0); // and pass that through as a uniform
	glUniformMatrix4fv(texShaded->input[1], 1, GL_FALSE, modelview);
	glUniformMatrix4fv(texShaded->input[2], 1, GL_FALSE, projection);
	glUniform2fv(texShaded->input[3], 1, offset);

	glScissor(ViewportX, ViewportY, ViewportWidth, ViewportHeight);
	glEnable(GL_SCISSOR_TEST);
	glEnable(GL_DEPTH_TEST);

	texRender->drawQuad();

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_DEPTH_TEST);

	// clean up matrix stack
	glMatrixMode(GL_PROJECTION);  glPopMatrix();                
	glMatrixMode(GL_MODELVIEW);   glPopMatrix();
	
	texShaded->shader->end();
    
//////////////////////////////// 

//     glEnable(GL_TEXTURE_2D); // THIS IS DEPRECATED in 4.0???
//     // shouldn't be needed since it is overriden by shader

//     glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

// //  glOrtho(0, w, 0, h, -1, 1);  // multiply current matrix (I) to apply clipping

	//   //////render
//    FramebufferObject::Disable();

//    glMatrixMode(GL_MODELVIEW);  // switch to MODELVIEW
//    glPushMatrix();              // push current to MODELVIEW stack
//    glLoadIdentity();            //


//    glMatrixMode(GL_PROJECTION); // switch to PROJECTION
//    glPushMatrix();              // push current to PROJECTION stack
//    glLoadIdentity();
// // //  glTranslatef(0.4, 0.4, 0);
// // //  glOrtho(0, w/2, 0, h/2, 0, -2);  // sets clipping (need to add offset?)


//    //  texShaded->shader->begin();    // does program->use, lets us do uniform stuff...

//    // texture:
//    glActiveTexture(GL_TEXTURE0);                 // texture unit 0 is active
//    glBindTexture(GL_TEXTURE_2D,  texRender->id); // bind texRender to tex unit 0, as a 2D tex
//    glUniform1i(texShaded->input[0], 0);          // and pass that through as a uniform

// //   GLfloat offset[2] = {(GLfloat)ViewportX, (GLfloat) ViewportY};
// //   glUniform2fv(texShaded->input[3], 1, offset);

//    GLfloat projection[16];
//    glGetFloatv(GL_PROJECTION_MATRIX, projection); 
//    glUniformMatrix4fv(texShaded->input[2], 1, GL_FALSE, projection);

//    GLfloat modelview[16];
//    glGetFloatv(GL_MODELVIEW_MATRIX, modelview); 
//    glUniformMatrix4fv(texShaded->input[1], 1, GL_FALSE, modelview);

// // //  glViewport(ViewportX, ViewportY, ViewportWidth, ViewportHeight);
// //   // glScissor(ViewportX, ViewportY, ViewportWidth, ViewportHeight);
// //   // glEnable(GL_SCISSOR_TEST); // limits the region of the buffer that is rendered to

//    glEnable(GL_DEPTH_TEST);

//    texShaded->drawQuad();

//    glDisable(GL_DEPTH_TEST);
// //   // glDisable(GL_SCISSOR_TEST);
// //   glBindTexture(GL_TEXTURE_2D, 0);



//    glMatrixMode(GL_PROJECTION); 
//    glPopMatrix();                // throw away top matrix from stack
//    glMatrixMode(GL_MODELVIEW);
//    glPopMatrix();                // throw away top matrix from stack

//    texShaded->shader->end();

////////////////////////////////////////////////////////////////////////////////

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

    texCurrent->imgWidth = m_width *2;//+ ViewportX;
    texCurrent->imgHeight = m_height + ViewportY;
    texCurrent->texWidth = m_width *2;//+ ViewportX;
    texCurrent->texHeight = m_height + ViewportY;

    cout << " m_width:  " << m_width
	 << " m_height: " << m_height
	 << " texW:  " << texCurrent->texWidth
	 << " texH:  " << texCurrent->texHeight
	 << " imgW:  " << texCurrent->imgWidth
	 << " imgH:  " << texCurrent->imgHeight
	 << endl;

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


