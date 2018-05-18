#pragma once

#include "Grid.h"
#include "afxwin.h"
#include <gl/gl.h>
#include <gl/glu.h>
#include "CA.h"

class OpenGLControl : public CWnd
{
public:
	//timer
	UINT_PTR displayTimer;

	OpenGLControl();
	virtual ~OpenGLControl();

	void openGLCreate(CRect rect, CWnd* parent);
	void initOpenGL(void);

	void setNewCARule(string ruleString);
	void runGeneticCA();

	void pauseAutomata(bool value);
	bool automataPaused(void);
	void resetAutomata(void);
	void speedUpAutomata(void);
	void slowDownAutomata(void);
	void automataStep(void);

	float getCATimeStep(void);
	void setCATimeStep(float timeStep);

	void setWindowMaximizedFlag(bool flag);
	bool getWindowMaximizedFlag(void);

	void clickOnCell(CPoint point);
private:
	//Window information
	CWnd* windowHandle;
	HDC hdc;
	HGLRC hrc;
	int m_nPixelFormat;
	CRect rect;
	CRect oldWindow;
	CRect originalRect;

	CA* automata;

	float mouseLastX, mouseLastY;
	float zoom;
	int diffX, diffY;
	float xPos, yPos; //not currently in use

	bool windowMaximized;
public:
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDraw(CDC* pDC);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
	
};