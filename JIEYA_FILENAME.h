#pragma once
#include "afxdialogex.h"


// JIEYA_FILENAME 对话框

class JIEYA_FILENAME : public CDialogEx
{
	DECLARE_DYNAMIC(JIEYA_FILENAME)

public:
	JIEYA_FILENAME(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~JIEYA_FILENAME();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG2 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString TEXT_NAME;
};
