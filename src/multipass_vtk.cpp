#include "GL/glew.h"

#include "vtkConeSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderWindow.h"
#include "vtkCamera.h"
#include "vtkActor.h"
#include "vtkRenderer.h"
#include "vtkSmartPointer.h"
#include "vtkOpenGLRenderWindow.h"
#include "vtkOpenGLProperty.h"

#include "vtkSaliencyPass.h"
#include "vtkCameraPass.h"
#include "vtkSequencePass.h"
#include "vtkRenderPassCollection.h"
#include "vtkDefaultPass.h"
#include "vtkLightsPass.h"

#include "vtkGenericDataObjectReader.h"  // for reading btain.vtk

#include <vtkRenderWindowInteractor.h>   // for interactor
#include <vtkCallbackCommand.h>          // for callbacks
#include <time.h>

#include "riftclass.h"                    // for rift

bool use_cone = true;

void KeypressCallbackFunction (
  vtkObject* caller,
  long unsigned int vtkNotUsed(eventId),
  void* clientData,
  void* vtkNotUsed(callData) ){

    vtkRenderWindowInteractor *iren =
      static_cast<vtkRenderWindowInteractor*>(caller);
    Rift* rift = (Rift*) clientData;

    char* key = iren->GetKeySym();
    // care! GetKeySym returns things like 'space'!
    //    cout << "Pressed: " << key << endl;

    if('r' == key[0]) rift->ResetSensor();

}


class vtkTimerCallback : public vtkCommand{
public:
    static vtkTimerCallback *New(){
	vtkTimerCallback *cb = new vtkTimerCallback;
	cb->TimerCount = 0;
	return cb;
    }

    void Configure(
	vtkCamera* left,
	vtkCamera* right,
	vtkRenderWindow* window,
	Rift* rift_pointer
	){
	camera_l_ = left;
	camera_r_ = right;
	renderWindow_ = window;
	last_yaw = last_pitch = last_roll = 0.0;
	rift = rift_pointer;
	double camera_position[3];
	double eye_spacing = 0.8;
	camera_l_->GetPosition(camera_position);
	camera_position[0] += eye_spacing / 2;
	camera_r_->SetPosition(camera_position);
	camera_position[0] -= eye_spacing;
	camera_l_->SetPosition(camera_position);

	rift->ResetSensor();
//	double camera_focus[3];
//	// may need to Get, Set the FocalPoint(camera_focus)
    }

    virtual void Execute(vtkObject *vtkNotUsed(caller),
			 unsigned long eventId,
			 void *vtkNotUsed(callData)){

	if (vtkCommand::TimerEvent == eventId){
	    ++this->TimerCount;

	    rift->HeadPosition(yaw, pitch, roll);
	    // cout << "  y"; cout.width(5); cout << (int)yaw;
	    // cout << "  p"; cout.width(5); cout << (int) pitch;
	    // cout << "  r"; cout.width(5); cout << (int) roll;
	    // cout.width(5); cout << (int) (last_pitch - pitch);
	    // cout.width(5); cout << (int) (last_yaw - yaw);
	    // cout << endl;

	    // camera_r_->Azimuth(1);  camera_l_->Azimuth(1);

	    camera_r_->SetRoll( (double) - roll  );
	    camera_r_->Yaw(  (double) yaw - last_yaw  );
	    camera_r_->Pitch((double) pitch - last_pitch);

	    camera_l_->SetRoll( (double) - roll  );
	    camera_l_->Yaw(  (double) yaw - last_yaw  );
	    camera_l_->Pitch((double) pitch - last_pitch);

	    last_yaw = yaw; last_pitch = pitch; last_roll = roll;

	    renderWindow_->Render();
	}
    }
private:
    int TimerCount;
    vtkSmartPointer<vtkCamera> camera_l_;
    vtkSmartPointer<vtkCamera> camera_r_;
    vtkSmartPointer<vtkRenderWindow> renderWindow_;
    Rift* rift;

    float yaw, pitch, roll;
    float last_yaw, last_pitch, last_roll;

};

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

    Rift rift;

// Timed interactor for HMD
    vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor =
	vtkSmartPointer<vtkRenderWindowInteractor>::New();
    renderWindowInteractor->SetRenderWindow(renWin);
    renderWindowInteractor->Initialize();

    vtkSmartPointer<vtkTimerCallback> cb =
	vtkSmartPointer<vtkTimerCallback>::New();

    renderWindowInteractor->AddObserver(
	vtkCommand::TimerEvent,
	cb);
    int timerId = renderWindowInteractor->CreateRepeatingTimer(50);
    std::cout << "timerId: " << timerId << std::endl;

    renWin->Render();

    // touching the cameras before first render stops auto-scaling
    cb->Configure(ren_l->GetActiveCamera(),
		  ren_r->GetActiveCamera(),
		  renWin,
		  &rift);

// // Keypress interactor
    vtkSmartPointer<vtkCallbackCommand> keypressCallback =
	vtkSmartPointer<vtkCallbackCommand>::New();
    keypressCallback->SetCallback ( KeypressCallbackFunction );
    keypressCallback->SetClientData( &rift);

    renderWindowInteractor->AddObserver (
	vtkCommand::KeyPressEvent,
	keypressCallback );

    renderWindowInteractor->Start();

    return 0;
}
