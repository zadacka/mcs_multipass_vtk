#include <vtkSmartPointer.h>
#include "vtkConeSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderWindow.h"
#include "vtkCamera.h"
#include "vtkActor.h"
#include "vtkRenderer.h"
#include <vtkRenderWindowInteractor.h>
#include <vtkCallbackCommand.h>

static void CameraModifiedCallback ( vtkObject* caller, long unsigned int eventId,
          void* clientData, void* callData );
//static void CameraModifiedCallback2 ( vtkObject* caller, long unsigned int eventId,
//          void* clientData, void* callData );

struct clientData{
    vtkCamera* camera;
    long unsigned int observerID;
    vtkCallbackCommand* observer;
};

int main()
{

  long unsigned int observerID;
  
  vtkConeSource *cone = vtkConeSource::New();
  cone->SetHeight( 3.0 );
  cone->SetRadius( 1.0 );
  cone->SetResolution( 10 );

  vtkPolyDataMapper *coneMapper = vtkPolyDataMapper::New();
  coneMapper->SetInputConnection( cone->GetOutputPort() );

  vtkActor *coneActor = vtkActor::New();
  coneActor->SetMapper( coneMapper );


  vtkSmartPointer<vtkCallbackCommand> observer1 = vtkSmartPointer<vtkCallbackCommand>::New();
  observer1->SetCallback (CameraModifiedCallback);
  clientData clientData1;

  vtkSmartPointer<vtkCallbackCommand> observer2 = vtkSmartPointer<vtkCallbackCommand>::New();
  observer2->SetCallback (CameraModifiedCallback);
  clientData clientData2;

  vtkRenderer *ren1= vtkRenderer::New();
  ren1->AddActor( coneActor );
  ren1->SetBackground( 0.1, 0.2, 0.4 );
  observerID = ren1->GetActiveCamera()->AddObserver(vtkCommand::ModifiedEvent, observer1);
  clientData1.observerID = observerID;
  

  vtkRenderer *ren2= vtkRenderer::New();
  ren2->AddActor( coneActor );
  ren2->SetBackground( 0.4, 0.1, 0.1 );
  ren2->GetActiveCamera()->AddObserver(vtkCommand::ModifiedEvent, observer2);
  clientData2.observerID = observerID;

  clientData1.camera = ren2->GetActiveCamera();
  clientData1.observer = observer2;

  clientData2.camera = ren1->GetActiveCamera();
  clientData2.observer = observer1;

  observer1->SetClientData(&clientData1); // know what the other camera is!

  observer2->SetClientData(&clientData2); // know what the other camera is!



  vtkRenderWindow *renWin = vtkRenderWindow::New();
  renWin->SetSize( 1280, 800 );
  //  renWin->FullScreenOn(); // might be useful later...

  double leftViewport[4] = {0.0, 0.0, 0.5, 1.0};
  double rightViewport[4] = {0.5, 0.0, 1.0, 1.0};
  renWin->AddRenderer( ren1 );
  ren1->SetViewport(leftViewport);
  renWin->AddRenderer( ren2 );
  ren2->SetViewport(rightViewport);

  // And one interactor
  vtkSmartPointer<vtkRenderWindowInteractor> interactor = 
      vtkSmartPointer<vtkRenderWindowInteractor>::New();
  interactor->SetRenderWindow(renWin);

  renWin->Render();
  interactor->Start();

  // int i;
  // for (i = 0; i < 360; ++i)
  //   {
  //   // render the image
  //   renWin->Render();
  //   // rotate the active camera by one degree
  //   ren1->GetActiveCamera()->Azimuth( 1 );
  //   }

  cone->Delete();
  coneMapper->Delete();
  coneActor->Delete();
  ren1->Delete();
  renWin->Delete();

  return 0;
}


static void CameraModifiedCallback(vtkObject* caller,
                                   long unsigned int observerID,
                                   void* clientDataIn,
				   void* vtkNotUsed(callData) )
{
    long unsigned int tempObserverID; 
    clientData* data = static_cast<clientData*>(clientDataIn);

    vtkCamera* camera = static_cast<vtkCamera*>(caller);
    vtkCamera* camera2 = data->camera;

    camera2->RemoveObserver(data->observerID);
    camera2->SetPosition(camera->GetPosition());
    camera2->SetFocalPoint(camera->GetFocalPoint());
    camera2->SetViewUp(camera->GetViewUp());
    tempObserverID = camera2->AddObserver(vtkCommand::ModifiedEvent, data->observer);
    data->observerID = tempObserverID;
}


// static void CameraModifiedCallback2(vtkObject* caller,
// 				    long unsigned int vtkNotUsed(eventId),
// 				    void* clientDataIn,
// 				    void* vtkNotUsed(callData) )
// {
//     long unsigned int tempObserverID; 
//     clientData* data = static_cast<clientData*>(clientDataIn);

//     vtkCamera* camera = static_cast<vtkCamera*>(caller);
//     vtkCamera* camera2 = data->camera;

//     camera2->RemoveObserver(data->observerID);
//     camera2->SetPosition(camera->GetPosition());
//     camera2->SetFocalPoint(camera->GetFocalPoint());
//     camera2->SetViewUp(camera->GetViewUp());
//     camera2->AddObserver(vtkCommand::ModifiedEvent, data->observer);

// }

// #include <vtkSmartPointer.h>
// #include "vtkConeSource.h"
// #include "vtkPolyDataMapper.h"
// #include "vtkRenderWindow.h"
// #include "vtkCamera.h"
// #include "vtkActor.h"
// #include "vtkRenderer.h"
// #include <vtkRenderWindowInteractor.h>
// #include <vtkCallbackCommand.h>

// static void KeypressCallbackFunction ( vtkObject* caller, long unsigned int eventId,
//           void* clientData, void* callData );

// int main()
// {
//   vtkConeSource *cone = vtkConeSource::New();
//   cone->SetHeight( 3.0 );
//   cone->SetRadius( 1.0 );
//   cone->SetResolution( 10 );

//   vtkPolyDataMapper *coneMapper = vtkPolyDataMapper::New();
//   coneMapper->SetInputConnection( cone->GetOutputPort() );

//   vtkActor *coneActor = vtkActor::New();
//   coneActor->SetMapper( coneMapper );

//   vtkRenderer *ren1= vtkRenderer::New();
//   ren1->AddActor( coneActor );
//   ren1->SetBackground( 0.1, 0.2, 0.4 );

//   vtkRenderer *ren2= vtkRenderer::New();
//   ren2->AddActor( coneActor );
//   ren2->SetBackground( 0.4, 0.1, 0.1 );

//   vtkRenderWindow *renWin = vtkRenderWindow::New();
//   renWin->SetSize( 1280, 800 );
//   //  renWin->FullScreenOn(); // might be useful later...

//   double leftViewport[4] = {0.0, 0.0, 0.5, 1.0};
//   double rightViewport[4] = {0.5, 0.0, 1.0, 1.0};
//   renWin->AddRenderer( ren1 );
//   ren1->SetViewport(leftViewport);

//   renWin->AddRenderer( ren2 );
//   ren2->SetViewport(rightViewport);

//   // Callback Command Stuff
//   vtkSmartPointer<vtkCallbackCommand> keypressCallback =
//     vtkSmartPointer<vtkCallbackCommand>::New();
//   keypressCallback->SetCallback(KeypressCallbackFunction );
//   // Allow the observer to access the sphereSource
//   keypressCallback->SetClientData(cone);

//   // And one interactor
//   vtkSmartPointer<vtkRenderWindowInteractor> interactor = 
//       vtkSmartPointer<vtkRenderWindowInteractor>::New();
//   interactor->SetRenderWindow(renWin);
//   interactor->AddObserver( vtkCommand::KeyPressEvent, keypressCallback );  //QVTKWidget::DragMoveEvent

//   renWin->Render();
//   interactor->Start();

//   // int i;
//   // for (i = 0; i < 360; ++i)
//   //   {
//   //   // render the image
//   //   renWin->Render();
//   //   // rotate the active camera by one degree
//   //   ren1->GetActiveCamera()->Azimuth( 1 );
//   //   }

//   cone->Delete();
//   coneMapper->Delete();
//   coneActor->Delete();
//   ren1->Delete();
//   renWin->Delete();

//   return 0;
// }


// void KeypressCallbackFunction(vtkObject* caller, long unsigned int vtkNotUsed(eventId), void* clientData, void* vtkNotUsed(callData) )
// {
//   // Prove that we can access the sphere source
//   vtkConeSource* coneSource =
//     static_cast<vtkConeSource*>(clientData);
//   std::cout << "Radius is " << coneSource->GetRadius() << std::endl;
// }
