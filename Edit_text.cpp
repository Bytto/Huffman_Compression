// Edit_text.cpp: 实现文件
//

#include "pch.h"
#include "HuffmanMFC.h"
#include "afxdialogex.h"
#include "Edit_text.h"


// Edit_text 对话框

IMPLEMENT_DYNAMIC(Edit_text, CDialogEx)

Edit_text::Edit_text(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG1, pParent)
	, FILE_NAME(_T(""))
{

}

Edit_text::~Edit_text()
{
}

void Edit_text::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, FILE_NAME);
}


BEGIN_MESSAGE_MAP(Edit_text, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT1, &Edit_text::OnEnChangeEdit1)
END_MESSAGE_MAP()


// Edit_text 消息处理程序
void Edit_text::OnEnChangeEdit1()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	UpdateData(true);
}