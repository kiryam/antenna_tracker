#include "stepper_tuning.h"

#include <stdio.h>

#include "gui.h"
#include "gfx.h"
#include "main.h"

#include "../tracking.h"
#include "../stepper.h"
#include "settings.h"


static GWidgetInit wi;
static GHandle ghServoBearingTuning;
static int16_t last_encoder_value;
static GHandle ghContainerServoTuning;
static GListener gl;
static GSourceHandle upHandle;
void StepperTuningRender();


static void gwStepperTuningEvent(void *param, GEvent *pe) {
	(void)param;
	if ( pe->type == GEVENT_TOGGLE ){
			switch( ((GEventToggle*)pe)->instance) {
			case BUTTON_ESC:
				ServoHandler();
				break;
			}
		}else if( pe->type == GEVENT_DIAL) {
			switch( ((GEventDial*)pe)->instance) {
			case 0:
				if ( ((GEventDial*)pe)->value != last_encoder_value ){
					if (((GEventDial*)pe)->value > last_encoder_value) {
						if( BearingTuning < 90 ){
							BearingTuning = BearingTuning + STEPPER_MIN_ANGILE;
						}
					} else {
						if (BearingTuning > -90 ) {
							BearingTuning = BearingTuning - STEPPER_MIN_ANGILE;
						}
					}
					last_encoder_value = ((GEventDial*)pe)->value;
				}
				break;
			}
		}
}

void StepperTuningCreate(){
	geventListenerInit(&gl);
	upHandle = ginputGetDial(0);
	geventAttachSource(&gl, upHandle, 0);

	GSourceHandle upHandle = ginputGetToggle(BUTTON_ESC);
	geventAttachSource(&gl, upHandle, GLISTEN_TOGGLE_ON);

	geventRegisterCallback(&gl, gwStepperTuningEvent, 0);

	gwinWidgetClearInit(&wi);

	wi.g.show = TRUE;
	wi.g.width = 126;
	wi.g.height = 64;
	wi.g.y = 0;
	wi.g.x = 0;
	ghContainerServoTuning = gwinContainerCreate(0, &wi, GWIN_CONTAINER_BORDER);
	wi.g.show = TRUE;
	wi.g.parent = ghContainerServoTuning;
	wi.g.width = gwinGetInnerWidth(ghContainerServoTuning);

	wi.g.y = 0;
	wi.g.x = 0;
	wi.g.height = 12;
	wi.text = "0";
	ghServoBearingTuning = gwinLabelCreate(NULL, &wi);
	gwinLabelSetAttribute(ghServoBearingTuning, 60, "BTuning:");
}

void StepperTuningDestroy(){
	UIDestroyContainerWithChilds(ghContainerServoTuning);
	ghContainerServoTuning = NULL;
	//geventDetachSourceListeners(upHandle);
	geventDetachSource(&gl, upHandle);
	vSemaphoreDelete(gl.waitqueue);
}

Page* CreateStepperTuningPage(){
	Page* page = pvPortMalloc(sizeof(Page));
	if( page == NULL ){
		return NULL;
	}
	page->Page = PAGE_SERVO_TUNING;
	page->Render = StepperTuningRender;
	page->Create = StepperTuningCreate;
	page->Destroy = StepperTuningDestroy;
	return page;
}

void StepperTuningRender(){
	char tmp[32] ={0};
	sprintf(tmp,"%f", BearingTuning);
	gwinSetText(ghServoBearingTuning, tmp, TRUE);
}
