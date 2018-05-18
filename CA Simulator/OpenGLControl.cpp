#include "stdafx.h"
#include "OpenGLControl.h"
#include "Grid.h"
#include <math.h>
#include "Resource.h"
#include "Rule.h"
#include "Libs\glm\glm\glm.hpp"

OpenGLControl::OpenGLControl()
{
	mouseLastX = 0;
	mouseLastY = 0;
	zoom = -10;
	xPos = 0;
	yPos = 0;

	windowMaximized = false;

	windowHandle = nullptr;
	automata = nullptr;
}
OpenGLControl::~OpenGLControl()
{
	if (automata)
	{
		delete automata;
		automata = nullptr;
	}
}

void OpenGLControl::openGLCreate(CRect rect, CWnd* parent)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_OWNDC, NULL, (HBRUSH)GetStockObject(BLACK_BRUSH), NULL);

	CreateEx(0, className, _T("OpenGL"), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, rect, parent, 0);

	oldWindow = rect;
	originalRect = rect;

	windowHandle = parent;

	CRect windowRect;
	windowHandle->GetDlgItem(ID_OPENGL_PICTURE_CONTROL)->GetWindowRect(&windowRect);
	glViewport(0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top);
}

BEGIN_MESSAGE_MAP(OpenGLControl, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_TIMER()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

bool OpenGLControl::automataPaused(void)
{
	return automata->isPaused();
}

void OpenGLControl::pauseAutomata(bool value)
{
	automata->pause(value);
}
void OpenGLControl::resetAutomata(void)
{
	automata->resetAutomata();
}
void OpenGLControl::speedUpAutomata(void)
{
	if (automata)
	{
		automata->setTimeStep(automata->getTimeStep() / 1.5);
		displayTimer = SetTimer(1, automata->getTimeStep(), 0);
	}
}
void OpenGLControl::slowDownAutomata(void)
{
	if (automata)
	{
		automata->setTimeStep(automata->getTimeStep() * 1.5);
		displayTimer = SetTimer(1, automata->getTimeStep(), 0);
	}
}
void OpenGLControl::automataStep(void)
{
	automata->step();
}

void OpenGLControl::setNewCARule(string ruleString)
{
	if (automata)
	{
		//destroy timer - we do not want the grid updated while we are managing heap memory, in case the update or draw function attempts to access a deleted object
		//KillTimer(displayTimer);

		RuleType newRuleType = Rule::evaluateRuleType(ruleString);
		RuleType currentRuleType = automata->getRuleType();

		//if automata is compatible with rule, simply set the new rule
		if (newRuleType == currentRuleType)
		{
			automata->setNewRule(ruleString);
		}
		//otherwise, delete the current automata and create a new one to run the rule
		else
		{
			if (newRuleType == INVALID_RULE)
				return;

			delete automata;
			automata = nullptr;

			if (newRuleType == LIFE)
				automata = new LifeCA(ruleString);
			else if (newRuleType == WOLFRAM)
				automata = new WolframCA(ruleString);

			if (automata)
				automata->pause(true);
		}
	}
}
void OpenGLControl::runGeneticCA()
{
	if (automata)
		delete automata;

	automata = new GeneticCA();
}

void OpenGLControl::setWindowMaximizedFlag(bool flag)
{

}
bool OpenGLControl::getWindowMaximizedFlag(void)
{
	return windowMaximized;
}

void OpenGLControl::initOpenGL(void)
{
	static PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		32,	//bit depth
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		16,	//z-buffer depth
		0, 0, 0, 0, 0, 0, 0,
	};

	//get device context only once
	hdc = GetDC()->m_hDC;

	//pixel format
	m_nPixelFormat = ChoosePixelFormat(hdc, &pfd);
	SetPixelFormat(hdc, m_nPixelFormat, &pfd);

	//create OpenGL rendering context
	hrc = wglCreateContext(hdc);
	wglMakeCurrent(hdc, hrc);

	// Initialise GLEW library
	GLenum err = glewInit();

	// Ensure GLEW was initialised successfully before proceeding
	if (err == GLEW_OK)
		cout << "GLEW initialised okay\n";
	else
	{
		cout << "GLEW could not be initialised\n";
		throw;
	}

	//basic setup
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClearDepth(1.0);

	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);

	//depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	OnDraw(NULL);
}

void OpenGLControl::clickOnCell(CPoint point)
{
	//pause automata to prevent updates while user is drawing on the grid
	bool automataWasPaused = automata->isPaused();
	automata->pause(true);

	//get hold of OpenGL picture control's absolute window coordinates
	CRect windowRect;
	windowHandle->GetDlgItem(ID_OPENGL_PICTURE_CONTROL)->GetWindowRect(&windowRect);

	//establish vertical and horizontal length of OpenGL picture control
	int windowVerticalDist = windowRect.bottom - windowRect.top;
	int windowHorizontalDist = windowRect.right - windowRect.left;

	//set up arrays to store modelView and projection matrix data, and viewport size
	GLdouble modelViewMatrix[16];
	GLdouble projectionMatrix[16];
	GLint viewport[4];

	//query OpenGL to find values for current modelview matrix, projection matrix and viewport
	glGetDoublev(GL_MODELVIEW_MATRIX, modelViewMatrix);
	glGetDoublev(GL_PROJECTION_MATRIX, projectionMatrix);
	glGetIntegerv(GL_VIEWPORT, viewport);

	//create variables to hold x, y, z coordinate values of the near and far plane positions corresponding to the mouse position
	GLdouble nearX, nearY, nearZ;
	GLdouble farX, farY, farZ;

	//gluUnProject transforms window coordinates to OpenGL world coordinates
	//parameters: x, y and z window coordinates, model view matrix, projection matrix, viewport, variables to store object coordinates x, y and z
	//third parameter (for window z coordinate): 0 is mapped to near plane position, 1 is mapped to far plane position
	gluUnProject(point.x, windowVerticalDist - point.y, 0, modelViewMatrix, projectionMatrix, viewport, &nearX, &nearY, &nearZ);
	gluUnProject(point.x, windowVerticalDist - point.y, 1, modelViewMatrix, projectionMatrix, viewport, &farX, &farY, &farZ);

	//set up returned coordinate values as vectors
	glm::vec3 nearVec(nearX, nearY, nearZ);
	glm::vec3 farVec(farX, farY, farZ);

	//a vector going from near plane (at x, y mouse coords) to far plane (at x, y mouse coords)
	//we want to find the values of x and y where this line intersects the z = 0 plane
	glm::vec3 lineVec = farVec - nearVec;

	//a normal of the z = 0 plane (coming from the plane towards the screen)
	glm::vec3 planeNormal(0, 0, 1);

	//a randomly chosen point on the plane (as long as z = 0, point is on plane)
	glm::vec3 pointOnPlane(10, 10, 0);

	double d = - glm::dot(planeNormal, pointOnPlane);
	double planeEquation = glm::dot(planeNormal, nearVec) + d;
	double normalDotLine = glm::dot(planeNormal, lineVec);

	double intersectionPointXCoord = nearVec.x - ((lineVec.x * planeEquation) / normalDotLine);
	double intersectionPointYCoord = nearVec.y - ((lineVec.y * planeEquation) / normalDotLine);

	//convert opengl coordinates into grid coordinates
	int xCell = automata->findGridXCoord(intersectionPointXCoord);
	int yCell = automata->findGridYCoord(intersectionPointYCoord);

	automata->editGridCell(xCell, yCell);
	automata->drawGrid();

	SwapBuffers(hdc);

	automata->pause(automataWasPaused);
}

#pragma region Event handlers
//run when OpenGL window object is created
int OpenGLControl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	//create OpenGL context
	initOpenGL();

	//creation of CA object must be deferred until after OpenGL context has been created, otherwise the VAO will fail to be set up in the CA constructor
	automata = new LifeCA();
	automata->pause(true);

	return 0;
}

//called when OS wants to redraw some of application window
void OpenGLControl::OnPaint()
{
	//CPaintDC dc(this); // device context for painting
	ValidateRect(NULL);
}

//called when application view needs to be updated to display changes
void OpenGLControl::OnDraw(CDC* pDC)
{
	glLoadIdentity();
	glTranslatef(xPos, yPos, zoom); //this...

	//...was previously:
	//glTranslatef(0, 0, zoom);
	//glTranslatef(xPos, yPos, 0);
}

//called whenever timer object reports an interval has passed - used for rendering
void OpenGLControl::OnTimer(UINT_PTR nIDEvent) //note: in example code, type of nIDEvent was UINT (not UINT_PTR)
{
	if (nIDEvent == 1)
	{
		if (automata)
		{
			automata->drawGrid();
			SwapBuffers(hdc);
		}
	}

	CWnd::OnTimer(nIDEvent);
}

//handles window resizing
void OpenGLControl::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	if (cx <= 0 || cy <= 0 || nType == SIZE_MINIMIZED) //if window size change is due to window being minimised, do nothing
	{
		windowMaximized = false;
		return;
	}

	//get hold of OpenGL picture control's absolute window coordinates
	//CRect windowRect;
	//windowHandle->GetDlgItem(ID_OPENGL_PICTURE_CONTROL)->GetWindowRect(&windowRect);

	//temporary hacked solution to viewport being larger than necessary
	//code above does not work because this function is called before windowHandle pointer is passed in to OpenGLControl object
	glViewport(0, 0, 932, 545);

	//resize OpenGL viewport to match new window size (lower-left coords = (0, 0), width = cx, height = cy)
	//was previously (0, 0, cx, cy)

	glMatrixMode(GL_PROJECTION);

	glLoadIdentity(); //set current projection matrix to identity matrix
	gluPerspective(35, (float)cx / (float)cy, 0.01, 2000);

	glMatrixMode(GL_MODELVIEW);

	switch (nType)
	{
	case SIZE_MAXIMIZED:
	{	//causes debug assertion
		/*GetWindowRect(rect);

		MoveWindow(6, 6, cx - 14, cy - 14);

		GetWindowRect(rect);

		oldWindow = rect;*/

		break;
	}
	case SIZE_RESTORED:
	{
		if (windowMaximized)
		{
			GetWindowRect(rect);
			MoveWindow(oldWindow.left, oldWindow.top - 18, originalRect.Width() - 4, originalRect.Height() - 4);

			GetWindowRect(rect);
			oldWindow = rect;
		}

		break;
	}
	}
}

//handles mouse movement
void OpenGLControl::OnMouseMove(UINT nFlags, CPoint point)
{
	diffX = (int)(point.x - mouseLastX);
	diffY = (int)(point.y - mouseLastY);

	mouseLastX = (float)point.x;
	mouseLastY = (float)point.y;

	//left mouse button
	if (nFlags & MK_LBUTTON)
	{
		//set cells to state 1 by calling grid.setcellState(), choosing cell according to mouse coordinates
		clickOnCell(point);
	}
	//right mouse button
	/*else if (nFlags & MK_RBUTTON)
	{
	//don't want user to be able to zoom in so far they cannot see grid
	if (zoom - 0.2 * diffY > 0)
	zoom -= 0.1 * diffY;
	}
	else if (nFlags & MK_MBUTTON)
	{
	//used to move grid around when the middle button is held down while the mouse is being moved
	xPos += 0.05 * diffX;
	yPos -= 0.05 * diffY;

	if (automata)
	automata->drawGrid();
	}*/

	OnDraw(NULL);

	CWnd::OnMouseMove(nFlags, point);
}

//handles keyboard input
void OpenGLControl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_SPACE)
	{
		if (automata)
			automata->pause(!automata->isPaused());
	}
	else if (nChar == VK_ADD)
	{
		speedUpAutomata();
	}
	else if (nChar == VK_SUBTRACT)
	{
		slowDownAutomata();
	}

	//CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

//handles mouse wheel scrolling
BOOL OpenGLControl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	float rotation = 0.2 * zDelta;

	//if statement prevents user from zooming in/out so far they can no longer see grid
	if (zoom + 0.2f * rotation < 0.1f)
		zoom += 0.1f * rotation;

	//adjust camera transformations according to new zoom level
	OnDraw(NULL);

	if (automata)
	{
		if (!automata->isPaused())
		{
			automata->pause(true);
			automata->drawGrid();
			automata->pause(false);
		}
		else
		{
			automata->drawGrid();
		}
	}

	//display grid on screen
	SwapBuffers(hdc);

	return CWnd::OnMouseWheel(nFlags, zDelta, pt);
}

//handles left mouse click event (left mouse click toggles the state of the cell mouse cursor is hovering over)
void OpenGLControl::OnLButtonDown(UINT nFlags, CPoint point)
{
	//when the mouse is clicked over the OpenGL control, set keyboard focus to OpenGL control
	SetFocus();

	//when mouse is clicked, translate mouse window coordinate to CA grid coordinate and handle input to that cell
	clickOnCell(point);

	CWnd::OnLButtonDown(nFlags, point);
}
#pragma endregion