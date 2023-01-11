#pragma once
#include "afxdialogex.h"


// Edit_text 对话框

class Edit_text : public CDialogEx
{
	DECLARE_DYNAMIC(Edit_text)

public:
	Edit_text(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~Edit_text();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeEdit1();
	CString FILE_NAME;
};
