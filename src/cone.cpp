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


int main()
{
    vtkSmartPointer<vtkConeSource> cone = 
	vtkSmartPointer<vtkConeSource>::New();

    vtkSmartPointer<vtkPolyDataMapper> coneMapper = 
	vtkSmartPointer<vtkPolyDataMapper>::New();
    coneMapper->SetInputConnection( cone->GetOutputPort() );

    vtkSmartPointer<vtkActor> coneActor = 
	vtkSmartPointer<vtkActor>::New();
    coneActor->SetMapper( coneMapper );

    vtkSmartPointer<vtkRenderer> ren= 
	vtkSmartPointer<vtkRenderer>::New();
    ren->AddActor( coneActor );

    vtkSmartPointer<vtkRenderWindow> renWin = 
	vtkSmartPointer<vtkRenderWindow>::New();
    renWin->AddRenderer( ren );


    int i;
    for (i = 0; i < 360; ++i)
    {
    	renWin->Render();
    	ren->GetActiveCamera()->Azimuth( 1 );
    }

    return 0;
}


