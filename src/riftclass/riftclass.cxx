#include "riftclass.h"

using namespace std; // already in a header :(
using namespace OVR;

Rift::Rift(){
    ovr_Initialize();
	
    this->hmd = ovrHmd_Create(0);

    if(!hmd) return;

    ovrHmd_GetDesc(hmd, &hmdDesc);

    ovrHmd_StartSensor(hmd, ovrSensorCap_Orientation| ovrSensorCap_YawCorrection | 
		       ovrSensorCap_Position, ovrSensorCap_Orientation);
}

Rift::~Rift(){
    ovrHmd_Destroy(hmd);
    ovr_Shutdown();
}

void Rift::Output()
{
    while(hmd){
	frameTiming = ovrHmd_BeginFrameTiming(hmd, 0); 
	ovrSensorState ss = ovrHmd_GetSensorState(hmd, frameTiming.ScanoutMidpointSeconds);

	if(ss.StatusFlags & (ovrStatus_OrientationTracked | ovrStatus_PositionTracked)){
	    ovrPosef pose = ss.Predicted.Pose;
	    Quatf quat = pose.Orientation;
	    float yaw, pitch, roll;
	    quat.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&yaw, &pitch, &roll);

	    cout << "yaw: " << RadToDegree(yaw)
		 << ", pitch: " << RadToDegree(pitch)
		 << ", roll: " << RadToDegree(roll)
		 << endl;
			
	    sleep(0.1);

	    ovrHmd_EndFrameTiming(hmd);
//	    if ('q' == getchar() ) exit(0);
	}
    }
}

bool Rift::HeadPosition(float& yaw,float& pitch,float& roll){
    frameTiming = ovrHmd_BeginFrameTiming(hmd, 0); 
    ovrSensorState ss = 
	ovrHmd_GetSensorState(hmd, frameTiming.ScanoutMidpointSeconds);

    if(ss.StatusFlags & (ovrStatus_OrientationTracked | ovrStatus_PositionTracked)){
	ovrPosef pose = ss.Predicted.Pose;
	Quatf quat = pose.Orientation;
	quat.GetEulerAngles<Axis_Y, Axis_X, Axis_Z>(&yaw, &pitch, &roll);
	yaw = RadToDegree(yaw);
	pitch = RadToDegree(pitch);
	roll= RadToDegree(roll);
	ovrHmd_EndFrameTiming(hmd);
	return true;
    }
    ovrHmd_EndFrameTiming(hmd);
    return false;
}

void Rift::ResetSensor(){
    // pass class member
    ovrHmd_ResetSensor(hmd);
}
