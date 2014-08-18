#include "GL/glew.h"

#include "vtkConeSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderWindow.h"
#include "vtkCamera.h"
#include "vtkActor.h"
#include "vtkRenderer.h"
#include "vtkShader2.h"
#include "vtkShaderProgram2.h"
#include "vtkShader2Collection.h"
#include "vtkSmartPointer.h"
#include "vtkOpenGLRenderWindow.h"
#include "vtkOpenGLProperty.h"

#include "vtkSaliencyPass.h"
#include "vtkCameraPass.h"
#include "vtkSequencePass.h"
#include "vtkOpaquePass.h"
#include "vtkRenderPassCollection.h"
#include "vtkVolumetricPass.h"
#include "vtkDefaultPass.h"
#include "vtkLightsPass.h"
#include "vtkClearZPass.h"

#include "vtkGenericDataObjectReader.h"

bool use_cone = false;

int main()
{
    // mapper, defined outside if statement
    vtkSmartPointer<vtkPolyDataMapper> coneMapper = vtkPolyDataMapper::New();

    // get source data
    if(!use_cone){
	std::string inputFilename = "btain.vtk";
 
	// read data
	vtkSmartPointer<vtkGenericDataObjectReader> reader = 
	    vtkSmartPointer<vtkGenericDataObjectReader>::New();
	reader->SetFileName(inputFilename.c_str());
	reader->Update();
	// polydata
	vtkSmartPointer<vtkPolyData> output = reader->GetPolyDataOutput();
	coneMapper->SetInputConnection(output->GetProducerPort());
    } else{
	vtkSmartPointer<vtkConeSource> cone = vtkConeSource::New();
	coneMapper->SetInputConnection( cone->GetOutputPort() );
    }

    // actor
    vtkSmartPointer<vtkActor> coneActor = vtkActor::New();
    coneActor->SetMapper( coneMapper );

    // renderer
    vtkSmartPointer<vtkRenderer> ren_l = vtkRenderer::New();
    ren_l->AddActor( coneActor );
    ren_l->SetBackground( 0.1, 0.2, 0.4 );
    vtkSmartPointer<vtkRenderer> ren_r = vtkRenderer::New();
    ren_r->AddActor( coneActor );
    ren_r->SetBackground( 0.1, 0.2, 0.4 );
 
   // render window
    double viewport_l[4] = {0.0, 0.0, 0.5, 1.0};
    double viewport_r[4] = {0.5, 0.0, 1.0, 1.0};
    vtkSmartPointer<vtkRenderWindow> renWin = vtkRenderWindow::New();
    renWin->SetSize(1280, 800);

    renWin->AddRenderer(ren_l); ren_l->SetViewport(viewport_l);
    renWin->AddRenderer(ren_r); ren_r->SetViewport(viewport_r);


    // Multipass Render
    // ////////////////////////
    // vtkOpaquePass *opaque=vtkOpaquePass::New();
    // vtkVolumetricPass *volume=vtkVolumetricPass::New();
    // vtkClearZPass* clearz=vtkClearZPass::New();
    vtkDefaultPass* defal=vtkDefaultPass::New();
    vtkLightsPass* lights=vtkLightsPass::New();

  vtkRenderPassCollection *passes=vtkRenderPassCollection::New();
//  passes->AddItem(opaque);
//  passes->AddItem(volume);
  passes->AddItem(defal);
  passes->AddItem(lights);

  vtkSequencePass *seq=vtkSequencePass::New();
  seq->SetPasses(passes);

  vtkCameraPass *cameraP=vtkCameraPass::New();
  cameraP->SetDelegatePass(seq);

  vtkSaliencyPass* saliencyP = vtkSaliencyPass::New();
  saliencyP->SetDelegatePass(cameraP);

    ren_l->SetPass(saliencyP);
    ren_r->SetPass(saliencyP);

    for (int i = 0; i < 360; ++i){
    	renWin->Render();
    	ren_l->GetActiveCamera()->Azimuth( 1 );
//	cameraP->translate(i, 0, 0);
//    	ren_r->GetActiveCamera()->Azimuth( 4 );
    }



    // // SDK 0.2 Style
    // // OVR::System::Init();
    // // {
    // // 	OVR::Ptr<OVR::DeviceManager> ovrManager = *OVR::DeviceManager::Create();
    // // 	OVR::Ptr<OVR::SensorDevice> ovrSensor = 
    // // 	    *ovrManager-> EnumerateDevices<OVR::SensorDevice>().CreateDevice();

    // // 	if (!ovrSensor) { 
    // // 	    std::cout << "Unable to detect Rift head tracker" << std::endl; 
    // // 	    return -1;
    // // 	}

    // // 	ovrFovPort eyeFov[2];
    // // 	ovrEyeRenderDesc eyeRenderDesc[2];

    // // 	eyeRenderDesc[0] = ovrHmd_GetRenderDesc(HMD, ovrEye_Left, eyeFov[0]);
    // // 	eyeRenderDesc[1] = ovrHmd_GetRenderDesc(HMD, ovrEye_Right, eyeFov[1]);
    // // 	// do OVR Stuff
    // // }
    // // OVR::System::Destroy();
    
    // SDK 0.3 Style: 
    // ovr_Initialize();
    // ovrHmd hmd = ovrHmd_Create(0);
    // ovrHmdDesc hmdDesc;
    // if(hmd){
    // 	ovrHmd_GetDesc(hmd, &hmdDesc);


    // Attempt at shaders... uses WIN32 stuff :(
    // // Initialize ovrEyeRenderDesc struct
    // ovrFovPort eyeFov[2];
	
    // ovrEyeRenderDesc eyeRenderDesc[2];
    // eyeRenderDesc[0] = ovrHmd_GetRenderDesc(hmd, ovrEye_Left, eyeFov[0]); 
    // eyeRenderDesc[1] = ovrHmd_GetRenderDesc(hmd, ovrEye_Right, eyeFov[1]);

	
    // //Generate distortion mesh for each eye 
    // for ( int eyeNum = 0; eyeNum < 2; eyeNum++ ){

    //     // Allocate & generate distortion mesh vertices. 
    //     ovrDistortionMesh meshData;
    //     unsigned int distortionCaps = hmdDesc.DistortionCaps;
	    

    //     // ... maybe the next line? TODO make it work for both eyes
    //     // NB they've changed sizei to ovrSizei with no documentation
    //     ovrSizei textureSize = ovrHmd_GetFovTextureSize(hmd, ovrEye_Left, 
    // 				 hmdDesc.DefaultEyeFov[0], 1.0f);

    //     // TODO: check viewport used here (viewport or eyeFov ?)
    //     ovrHmd_CreateDistortionMesh(
    // 	hmd, 
    // 	eyeRenderDesc[eyeNum].Eye, 
    // 	eyeRenderDesc[eyeNum].Fov, 
    // 	distortionCaps, 
    // 	&meshData);
    //     ovrHmd_GetRenderScaleAndOffset(
    // 	eyeRenderDesc[eyeNum].Fov, 
    // 	textureSize, 
    // 	eyeFov[eyeNum], 
    // 	(ovrVector2f*) distortionData.UVScaleOffset[eyeNum]);

    //     // Now parse the vertex data and create a render ready vertex buffer from it
    //     DistortionVertex * pVBVerts = (DistortionVertex*)OVR_ALLOC( sizeof(DistortionVertex) * meshData.VertexCount );
    //     DistortionVertex * v = pVBVerts;
    //     ovrDistortionVertex * ov = meshData.pVertexData; 

    //     for ( unsigned vertNum = 0; vertNum < meshData.VertexCount; vertNum++ ){
    // 	v->Pos.x = ov->Pos.x; 
    // 	v->Pos.y = ov->Pos.y;
    // 	v->TexR = (*(Vector2f*)&ov->TexR); 
    // 	v->TexG = (*(Vector2f*)&ov->TexG); 
    // 	v->TexB = (*(Vector2f*)&ov->TexB); 
    // 	v->Col.R = v->Col.G = v->Col.B = (OVR::UByte)( ov->VignetteFactor * 255.99f ); 
    // 	v->Col.A = (OVR::UByte)( ov->TimeWarpFactor * 255.99f ); 
    // 	v++; ov++;
    //     } 
	    
    //     //Register this mesh with the renderer
    //     DistortionData.MeshVBs[eyeNum] = *pRender->CreateBuffer(); 
    //     DistortionData.MeshVBs[eyeNum]->Data ( Buffer_Vertex, pVBVerts, sizeof(DistortionVertex) * meshData.VertexCount );
    //     DistortionData.MeshIBs[eyeNum] = *pRender->CreateBuffer(); 
    //     DistortionData.MeshIBs[eyeNum]->Data ( Buffer_Index, meshData.pIndexData, sizeof(unsigned short) * meshData.IndexCount );
    //     OVR_FREE ( pVBVerts ); 
    //     ovrHmd_DestroyDistortionMesh( &meshData );
    // }



    // Care! We're gonna kill the Rift after this point (so stop using it)
    // }
    // ovrHmd_Destroy(hmd);
    // ovr_Shutdown();


    return 0;
}


