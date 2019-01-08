#include "gfx.h"
#include "gui.h"
#include "main.h"
#include "homefinder.h"

static GHandle ghHomeFinderCompass;
static GHandle ghContainerHomeFinder;
static GWidgetInit wi;

void HomeFinderRender(){
	if( firstTimeRendered == false ){ // just create screen
		UIHomeFinerScreen();
		firstTimeRendered = true;
	}

	gwinSetIntCached(0, ghHomeFinderCompass, home_bearing, TRUE);
}


void UIHomeFinerScreen(){
	gwinWidgetClearInit(&wi);
	wi.customDraw = 0;
	wi.customParam = 0;
	wi.customStyle = 0;

	wi.g.show = TRUE;
	wi.g.width = 126;
	wi.g.height = 64;
	wi.g.y = 0;
	wi.g.x = 0;
	ghContainerHomeFinder = gwinContainerCreate(0, &wi, GWIN_CONTAINER_BORDER);
	wi.g.show = TRUE;
	wi.g.parent = ghContainerHomeFinder;
	wi.g.width = gwinGetInnerWidth(ghContainerHomeFinder);

	wi.g.y = 0;
	wi.g.x = 0;
	wi.g.height = 12;
	wi.text = "0";
	ghHomeFinderCompass = gwinLabelCreate(NULL, &wi);
	gwinLabelSetAttribute(ghHomeFinderCompass, 60, "Heading:");
}
