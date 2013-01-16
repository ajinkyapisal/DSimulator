// ChildSceneNodeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SceneEditor.h"
#include "ChildSceneNodeDlg.h"
#include "afxdialogex.h"


// CChildSceneNodeDlg dialog

IMPLEMENT_DYNAMIC(CChildSceneNodeDlg, CDialogEx)

CChildSceneNodeDlg::CChildSceneNodeDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CChildSceneNodeDlg::IDD, pParent)
{

	m_NodeName = _T("");
	x = 0;
	y = 0;
	z = 0;
}

CChildSceneNodeDlg::~CChildSceneNodeDlg()
{
}

void CChildSceneNodeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_NODE_NAME, m_NodeName);
	DDX_Text(pDX, IDC_X, x);
	DDX_Text(pDX, IDC_Y, y);
	DDX_Text(pDX, IDC_Z, z);
}


BEGIN_MESSAGE_MAP(CChildSceneNodeDlg, CDialogEx)
END_MESSAGE_MAP()


// CChildSceneNodeDlg message handlers
