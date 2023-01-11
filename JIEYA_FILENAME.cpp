// JIEYA_FILENAME.cpp: 实现文件
//

#include "pch.h"
#include "HuffmanMFC.h"
#include "afxdialogex.h"
#include "JIEYA_FILENAME.h"


// JIEYA_FILENAME 对话框

IMPLEMENT_DYNAMIC(JIEYA_FILENAME, CDialogEx)

JIEYA_FILENAME::JIEYA_FILENAME(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG2, pParent)
	, TEXT_NAME(_T(""))
{

}

JIEYA_FILENAME::~JIEYA_FILENAME()
{
}

void JIEYA_FILENAME::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, TEXT_NAME);
}


BEGIN_MESSAGE_MAP(JIEYA_FILENAME, CDialogEx)
END_MESSAGE_MAP()


// JIEYA_FILENAME 消息处理程序
