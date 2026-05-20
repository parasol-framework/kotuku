/*********************************************************************************************************************

-CLASS-
Controller: Provides support for reading state-based game controllers.

Use the Controller class to read the state of game controllers that are recognised by the operating system.

Unlike analog devices that stream input commands (e.g. mice), gamepad controllers maintain a state that can be read
at any time.  The controller state is normally read at least once per frame, which can be achieved in a program's
inner loop, or in a separate timer.

On Linux, controllers are read through the `/dev/input/js*` joystick API.  The user running the application must have
read access to these device nodes.

Controller input management is governed by the @Display class.  The `GRAB_CONTROLLERS` flag must be defined in the
active Display's Flags field in order to ensure that controller input can be received.  Failure to do so may mean
that the Controller object appears to work but does not receive input.

-END-

*********************************************************************************************************************/

#include "defs.h"

#ifdef _WIN32
using namespace display;
#endif

#ifdef __linux__
extern ERR linuxReadController(int Port, double *Values, CON &Buttons);
extern ERR linuxGetControllerPorts(int &Value);
#endif

//********************************************************************************************************************

static ERR CONTROLLER_NewObject(objController *Self)
{
   Self->Port = -1;
   return ERR::Okay;
}

/*********************************************************************************************************************
-ACTION-
Query: Get the current controller state.
-END-
*********************************************************************************************************************/

static ERR CONTROLLER_Query(objController *Self)
{
#ifdef _WIN32
   if (auto error = winReadController(Self->Port, (double *)&Self->LeftTrigger, Self->Buttons); error IS ERR::Okay) {
      return ERR::Okay;
   }
   else return error;
#elif defined(__linux__)
   if (auto error = linuxReadController(Self->Port, (double *)&Self->LeftTrigger, Self->Buttons); error IS ERR::Okay) {
      return ERR::Okay;
   }
   else return error;
#else
   return ERR::NoSupport;
#endif
}

/*********************************************************************************************************************

-FIELD-
LeftTrigger: Left trigger value between 0.0 and 1.0.

-FIELD-
RightTrigger: Right trigger value between 0.0 and 1.0.

-FIELD-
LeftStickX: Left analog stick value for X axis, between -1.0 and 1.0.

-FIELD-
LeftStickY: Left analog stick value for Y axis, between -1.0 and 1.0.

-FIELD-
RightStickX: Right analog stick value for X axis, between -1.0 and 1.0.

-FIELD-
RightStickY: Right analog stick value for Y axis, between -1.0 and 1.0.

-FIELD-
Buttons: Button values expressed as bit-fields.

-FIELD-
Port: The port number assigned to the controller.

Set the port number to choose the controller that will be queried for state changes.  The default of -1 is used
to indicate the primary controller.  Fixed port numbers start from zero.  There is no guarantee that the existence
of a port means that a controller is connected to it.

It is acceptable to set the port number post-initialisation, so multiple controllers can be queried through one
interface at the cost of overwriting the previous state.  Check #TotalPorts if your program supports more than one
controller.

-FIELD-
TotalPorts: Reports the number of controller ports that should be scanned.

Port values range from zero to `TotalPorts - 1`.  Some platforms, including Linux, may expose sparse controller
indices, so an individual port in that range can fail to query if its device is not currently connected.

*********************************************************************************************************************/

static ERR CONTROLLER_GET_TotalPorts(extSurface *Self, int &Value)
{
#ifdef _WIN32
   if (glLastPort.load() >= 0) Value = glLastPort.load() + 1;
   else Value = 0;
   return ERR::Okay;
#elif defined(__linux__)
   return linuxGetControllerPorts(Value);
#endif

   return ERR::NoSupport;
}

//********************************************************************************************************************

#include "class_controller_def.c"

static const FieldArray clFields[] = {
   { "LeftTrigger",  FDF_DOUBLE|FDF_R },
   { "RightTrigger", FDF_DOUBLE|FDF_R },
   { "LeftStickX",   FDF_DOUBLE|FDF_R },
   { "LeftStickY",   FDF_DOUBLE|FDF_R },
   { "RightStickX",  FDF_DOUBLE|FDF_R },
   { "RightStickY",  FDF_DOUBLE|FDF_R },
   { "Buttons",      FDF_INT|FDF_R, nullptr, nullptr, &clControllerButtons },
   { "Port",         FDF_INT|FDF_RW },
   { "TotalPorts",   FDF_VIRTUAL|FDF_INT|FDF_R, CONTROLLER_GET_TotalPorts },
   END_FIELD
};

//********************************************************************************************************************

ERR create_controller_class(void)
{
   clController = objMetaClass::create::global(
      fl::BaseClassID(CLASSID::CONTROLLER),
      fl::ClassVersion(VER_CONTROLLER),
      fl::Name("Controller"),
      fl::Category(CCF::IO),
      fl::Actions(clControllerActions),
      fl::Fields(clFields),
      fl::Size(sizeof(objController)),
      fl::Path(MOD_PATH));

   return clController ? ERR::Okay : ERR::AddClass;
}
