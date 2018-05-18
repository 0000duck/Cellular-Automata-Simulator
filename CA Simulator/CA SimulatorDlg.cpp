// CA SimulatorDlg.cpp : implementation file

#include "stdafx.h"
#include "CA Simulator.h"
#include "CA SimulatorDlg.h"
#include "afxdialogex.h"
#include <string>
#include "Rule.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// CASimulatorDlg dialog
CASimulatorDlg::CASimulatorDlg(CWnd* pParent /*=NULL*/) : CDialogEx(CASimulatorDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CASimulatorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CASimulatorDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CASimulatorDlg::OnBnClickedOk)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON1, &CASimulatorDlg::OnBnClickedButton1)
	ON_WM_KEYDOWN()
	ON_WM_KEYDOWN()
	ON_BN_CLICKED(IDC_SET_RULE_BUTTON, &CASimulatorDlg::OnBnClickedSetRuleButton)
	ON_COMMAND(ID_FILE_NEWPATTERN, &CASimulatorDlg::OnFileNewPattern)
	ON_COMMAND_RANGE(ID_RULES_GAMEOFLIFE, ID_RULES_W110, &CASimulatorDlg::OnRuleMenuItems)
	ON_COMMAND(ID_PLAYBACK_PLAY, &CASimulatorDlg::OnPlaybackPlay)
	ON_COMMAND(ID_PLAYBACK_PAUSE, &CASimulatorDlg::OnPlaybackPause)
	ON_COMMAND(ID_PLAYBACK_FASTER, &CASimulatorDlg::OnPlaybackFaster)
	ON_COMMAND(ID_PLAYBACK_SLOWER, &CASimulatorDlg::OnPlaybackSlower)
	ON_COMMAND(ID_PLAYBACK_STEP, &CASimulatorDlg::OnPlaybackStep)
	ON_BN_CLICKED(IDC_BUTTON3, &CASimulatorDlg::OnPlaybackStep)
	ON_BN_CLICKED(IDC_BUTTON4, &CASimulatorDlg::OnPlaybackFaster)
	ON_BN_CLICKED(IDC_BUTTON5, &CASimulatorDlg::OnPlaybackSlower)
	ON_BN_CLICKED(IDC_BUTTON2, &CASimulatorDlg::OnPlaybackPlay)
	ON_BN_CLICKED(IDC_EVOLVE_BUTTON, &CASimulatorDlg::OnBnClickedEvolveButton)
	ON_COMMAND(ID_GUIDE, &CASimulatorDlg::OnGuide)
END_MESSAGE_MAP()

// CASimulatorDlg message handlers
BOOL CASimulatorDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	ShowWindow(SW_MAXIMIZE);
	openGLWindow.setWindowMaximizedFlag(true);

	CWnd* label = GetDlgItem(IDC_STATIC);
	label->SetWindowText(_T("Accepted rule formats:\n\n   Life - like rules - \"B.../S...\":\n\n\"...\" can be any-length combination of single-digit numbers between 0 and 8 e.g. B3/S23. \"B/S\", \"B/S...\" and \"B.../S\" also accepted.\n\n   Wolfram rules:\n\n\"W0\" - \"W255\""));
	UpdateData(FALSE);

	CRect rect;
	//setup edit control (text)

	//get size and position of picture control
	GetDlgItem(ID_OPENGL_PICTURE_CONTROL)->GetWindowRect(rect);

	//convert screen coordinates to client coordinates
	ScreenToClient(rect);

	//create OpenGLControl window
	openGLWindow.openGLCreate(rect, this);

	//setup timer used for managing periodic re-display of CA - this initial value will be changed later
	openGLWindow.displayTimer = openGLWindow.SetTimer(1, 1000, 0);

	return TRUE;  // return TRUE unless you set the focus to a control
}

void CASimulatorDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CASimulatorDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CASimulatorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CASimulatorDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}

void CASimulatorDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	switch (nType)
	{
	case SIZE_RESTORED:
	{
		if (openGLWindow.getWindowMaximizedFlag())
		{
			openGLWindow.OnSize(nType, cx, cy);
			openGLWindow.setWindowMaximizedFlag(false);
		}

		break;
	}
	case SIZE_MAXIMIZED:
	{
		openGLWindow.OnSize(nType, cx, cy);
		openGLWindow.setWindowMaximizedFlag(true);

		break;
	}
	}
}

void CASimulatorDlg::OnBnClickedButton1()
{
	openGLWindow.pauseAutomata(true);
}

void CASimulatorDlg::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_SPACE)
		openGLWindow.pauseAutomata(!openGLWindow.automataPaused());

	CDialogEx::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CASimulatorDlg::OnBnClickedSetRuleButton()
{
	//when button is clicked, get text from IDC_SET_RULE_EDIT text box
	CString text = '\0';
	std::string ruleString = "\0";
	((CEdit*)GetDlgItem(IDC_SET_RULE_EDIT))->GetWindowText(text);
	ruleString = CT2A(text.GetBuffer());

	if (ruleString[0] != '\0')
	{
		//validate rule
		RuleType type = Rule::evaluateRuleType(ruleString);

		if (type != INVALID_RULE)
			openGLWindow.setNewCARule(ruleString);
	}
}

void CASimulatorDlg::OnFileNewPattern()
{
	openGLWindow.resetAutomata();
}

//this is the event handler for every button in the "Rules" menu
void CASimulatorDlg::OnRuleMenuItems(UINT nID)
{
	string ruleString = "\n";

	//determine which button was clicked and set new rule string appropriately
	if (nID == ID_RULES_GAMEOFLIFE)
		ruleString = "B3/S23\n";
	else if (nID == ID_RULES_MAZE)
		ruleString = "B3/S12345\n";
	else if (nID == ID_RULES_SEEDS)
		ruleString = "B2/S\n";
	else if (nID == ID_RULES_REPLICATOR)
		ruleString = "B1357/S1357\n";
	else if (nID == ID_RULES_W30)
		ruleString = "W30";
	else if (nID == ID_RULES_W90)
		ruleString = "W90";
	else if (nID == ID_RULES_W110)
		ruleString = "W110";
	else
		return;

	openGLWindow.setNewCARule(ruleString);
}

void CASimulatorDlg::OnPlaybackPlay()
{
	openGLWindow.pauseAutomata(false);
}
void CASimulatorDlg::OnPlaybackPause()
{
	openGLWindow.pauseAutomata(true);
}

void CASimulatorDlg::OnPlaybackFaster()
{
	openGLWindow.speedUpAutomata();
}
void CASimulatorDlg::OnPlaybackSlower()
{
	openGLWindow.slowDownAutomata();
}

void CASimulatorDlg::OnPlaybackStep()
{
	openGLWindow.automataStep();
}

void CASimulatorDlg::OnBnClickedEvolveButton()
{
	openGLWindow.runGeneticCA();
}

void CASimulatorDlg::OnGuide()
{
	/*CSecondDlg Dlg;

	Dlg.DoModal();*/
}
