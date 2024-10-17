#include <stdio.h>
#include <math.h>
#include <windows.h>
#include <wingdi.h>
#define WINDOW_SIZE 1000
#define PROJECTION_TYPE 1 //0 for orthographic projection, 1 for weak perspective projection
#define SCALE 150 //scale of cube
#define DISTANCE 2 //distance from cube for weak perspective
#define PI 3.14159265358979323846

typedef struct Vector3{
	double x;
	double y;
	double z;
} Vector3;

typedef struct Vector2{
	double x;
	double y;
} Vector2;


//initializing vertices
Vector3 v[] = {
	[0]={-1,1,-1},
	[1]={1,1,-1},
	[2]={1,-1,-1},
	[3]={-1,-1,-1},
	[4]={-1,1,1},
	[5]={1,1,1},
	[6]={1,-1,1},
	[7]={-1,-1,1}
};

//projected vertices
Vector3 pv[8];



/*
to rotate about the:
    x axis: for each point, x is fixed, modify y and z
	y axis: modify x and z
	z axis: modify x and y
*/

//orthographic projection :: no perspective, so things far away (large z) look the same as things close (small z)
Vector2 orthographic_3Dto2D(Vector3 v){
	Vector2 returnVector={
		v.x*SCALE,
		v.y*SCALE
	};
	return returnVector;
}
//weak perspective projection :: simulates actual perspecive linearly
Vector2 wPerspective_3Dto2D(Vector3 v){
	double z=v.z+DISTANCE;
	Vector2 returnVector = {
		v.x*SCALE * (5/(DISTANCE+v.z+5)), //YOU MUST ADD 5 BECAUSE I SAID SO
		v.y*SCALE * (5/(DISTANCE+v.z+5)) //actually the reason you add 5 is because weak perspective projection calculates projected position based on (distance to screen)/("actual distance"), and we took distance to screen as 5
	};
	return returnVector;
}


//vector rotations :: this is just expanded matrix multiplication of 3D rotation matrices
double angle=0;
Vector3 rotateX(Vector3 v, double angle, int index){
	double Y=v.y;
	double Z=v.z;
	angle*=PI/180;

	//v->x does not change
	v.y=Y*cos(angle)-Z*sin(angle);
	v.z=Y*sin(angle)+Z*cos(angle);

	return v;
}
Vector3 rotateY(Vector3 v, double angle, int index){
	double X=v.x;
	double Z=v.z;
	angle*=PI/180;

	v.x=X*cos(angle)-Z*sin(angle);
	//v->y does not change
	v.z=X*sin(angle)+Z*cos(angle);

	return v;
}
Vector3 rotateZ(Vector3 v, double angle, int index){
	double X=v.x;
	double Y=v.y;
	angle*=PI/180;

	v.x=X*cos(angle)-Y*sin(angle);
	v.y=X*sin(angle)+Y*cos(angle);
	//v->z does not change

	return v;
}

void drawBetweenVertices(HDC hdc, Vector2 v1, Vector2 v2){
	MoveToEx(hdc,v1.x,v1.y,NULL); //move to point (v1.x, v1.y)
	LineTo(hdc,v2.x,v2.y); //draw to (v2.x, v2.y)
}






LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);//defining window procedure function. controls window behavior

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
	//every window needs to be associated with a window class. window class is a data structure used by OS

	//registering window class
	const char CLASS_NAME[]="Test window class"; //name of window class
	WNDCLASS wc = {}; //declaring the window class
	wc.lpfnWndProc=WindowProc; //procedure followed by window. for now, it is not defined, only declared
	wc.hInstance=hInstance; //is a parameter of windows entry point WinMain. identifies application instance
	wc.lpszClassName=(LPCSTR)CLASS_NAME;

	RegisterClass(&wc); //registering to OS


	//creating instance of window class wc
	HWND hwnd = CreateWindow( //hwnd = window handle. ID given to each window instance
		CLASS_NAME, //window class name
		"Rotating Cube", //window instance title
		(WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU), //window instance styling ... normally WS_OVERLAPPEDWINDOW but that allows for custom resizing WS_THICKFRAME, minimizing WS_MINIMIZEBOX, maximising WS_MAXIMIZEBOX, WHICH I DONT WANT!!!@*#(@!#&*(@!&#))
		(2560/2)-(WINDOW_SIZE/2), (1440/2)-(WINDOW_SIZE/2), WINDOW_SIZE, WINDOW_SIZE,//size of instance window .... top left x, top left y, width, height
		NULL,//parent instance window
		NULL,
		hInstance,
		NULL
	);

	if (!hwnd){
		MessageBox(NULL, "Failed to create window!", "Error", MB_OK | MB_ICONERROR);
		return 0;
	}
	
	ShowWindow(hwnd,nCmdShow);


	//message loop. this is necessary for all moving windows because it is what calls WindowProc
	MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }


    return 0;
}

	

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_PAINT:
        {	
			//get window context and create pen
			HDC window = GetDC(hwnd);
			HGDIOBJ blackPen=GetStockObject(BLACK_PEN);
			SelectObject(window,blackPen);
			
			//clear window
			HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255)); 
            RECT rect;
            GetClientRect(hwnd, &rect); //get rectangle size of window handle
            FillRect(window, &rect, hBrush); //fill window
            DeleteObject(hBrush);
			
			//setting origin (0,0) to centre of window
            SetViewportOrgEx(window, WINDOW_SIZE/2, WINDOW_SIZE/2, NULL);


			int i;

			//draw all edges based on choice of projection
			if (PROJECTION_TYPE==0){ //i know i should use an enum but im not bothered
				for (i=0; i<4; i++){
					drawBetweenVertices(window,orthographic_3Dto2D(pv[i]),orthographic_3Dto2D(pv[(i+1)%4])); //link vertices 0-1 1-2 2-3 3-0
					drawBetweenVertices(window,orthographic_3Dto2D(pv[i+4]),orthographic_3Dto2D(pv[(i+1)%4+4])); //link 4-5 5-6 6-7 7-4
					drawBetweenVertices(window,orthographic_3Dto2D(pv[i]),orthographic_3Dto2D(pv[i+4])); //link 0-4 1-5 2-6 3-7
				}
			} else if (PROJECTION_TYPE==1){
				for (i=0; i<4; i++){
					drawBetweenVertices(window,wPerspective_3Dto2D(pv[i]),wPerspective_3Dto2D(pv[(i+1)%4])); 
					drawBetweenVertices(window,wPerspective_3Dto2D(pv[i+4]),wPerspective_3Dto2D(pv[(i+1)%4+4]));
					drawBetweenVertices(window,wPerspective_3Dto2D(pv[i]),wPerspective_3Dto2D(pv[i+4])); 
				}
			}

			//rotate each vertex by multiplier*<angle> degrees
			for(i=0;i<8;i++){
				pv[i]=rotateX(v[i],1*angle,i);
				pv[i]=rotateY(pv[i],1*angle,i);
				pv[i]=rotateZ(pv[i],1*angle,i);
			}
			
			Sleep(10);
			angle+=1;

			DeleteObject(blackPen);
        }
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}