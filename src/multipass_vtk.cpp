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

#include <vtkRenderWindowInteractor.h>

bool use_cone = true;

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
//    ren_l->SetBackground( 0.1, 0.2, 0.4 );
    vtkSmartPointer<vtkRenderer> ren_r = vtkRenderer::New();
    ren_r->AddActor( coneActor );
//    ren_r->SetBackground( 0.1, 0.2, 0.4 );
 
   // render window
    double viewport_l[4] = {0.0, 0.0, 0.5, 1.0};
    double viewport_r[4] = {0.5, 0.0, 1.0, 1.0};
    vtkSmartPointer<vtkRenderWindow> renWin = vtkRenderWindow::New();

    // size and shift window
    renWin->SetSize(1280, 800);
    renWin->SetPosition(1680, 0); 

    // alternative: full screen & mirror BUT sets to 1280 x 720!!
    // renWin->FullScreenOn();

    renWin->AddRenderer(ren_l); ren_l->SetViewport(viewport_l);
    renWin->AddRenderer(ren_r); ren_r->SetViewport(viewport_r);


    // Multipass Render
    // ////////////////////////
    // vtkOpaquePass *opaque=vtkOpaquePass::New();
    // vtkVolumetricPass *volume=vtkVolumetricPass::New();
    // vtkClearZPass* clearz=vtkClearZPass::New();

    // left
    vtkDefaultPass* defal_l=vtkDefaultPass::New();
    vtkLightsPass* lights_l=vtkLightsPass::New();

    vtkRenderPassCollection *passes_l=vtkRenderPassCollection::New();
    passes_l->AddItem(defal_l);
    passes_l->AddItem(lights_l);
    vtkSequencePass *seq_l=vtkSequencePass::New();
    seq_l->SetPasses(passes_l);
    vtkCameraPass *cameraP_l=vtkCameraPass::New();
    cameraP_l->SetDelegatePass(seq_l);
    vtkSaliencyPass* saliencyP_l = vtkSaliencyPass::New();
    saliencyP_l->SetDelegatePass(cameraP_l);


    // right
    vtkDefaultPass* defal_r=vtkDefaultPass::New();
    vtkLightsPass* lights_r=vtkLightsPass::New();
    vtkRenderPassCollection *passes_r=vtkRenderPassCollection::New();
    passes_r->AddItem(defal_r);
    passes_r->AddItem(lights_r);
    vtkSequencePass *seq_r=vtkSequencePass::New();
    seq_r->SetPasses(passes_r);
    vtkCameraPass *cameraP_r=vtkCameraPass::New();
    cameraP_r->SetDelegatePass(seq_r);
    vtkSaliencyPass* saliencyP_r = vtkSaliencyPass::New();
    saliencyP_r->SetDelegatePass(cameraP_r);


    ren_l->SetPass(saliencyP_l);
    ren_r->SetPass(saliencyP_r);

    // // use interactor
    // vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = 
    // 	vtkSmartPointer<vtkRenderWindowInteractor>::New();
    // renderWindowInteractor->SetRenderWindow(renWin);

    // // Begin mouse interaction
    // renderWindowInteractor->Start();

    for (int i = 0; i < 360; ++i){

    	renWin->Render();

	double camera_position[3];
	ren_l->GetActiveCamera()->GetPosition(camera_position);
	camera_position[0] += 0.2;
	ren_r->GetActiveCamera()->SetPosition(camera_position);

	// double camera_focus[3];
	// ren_l->GetActiveCamera()->GetFocalPoint(camera_focus);
	// camera_focus[0]    += 0.1;
	// ren_r->GetActiveCamera()->SetFocalPoint(camera_focus);

   	 ren_l->GetActiveCamera()->Azimuth( 1 );
   	 ren_r->GetActiveCamera()->Azimuth( 1 );
    }

    return 0;
}


